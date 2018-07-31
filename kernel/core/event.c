
/*
    EventOS V1.0 - Copyright (C) 2014 Edielson Prevato Frigieri.

       This file is part of the EventOS distribution.

    EventTOS is free software; you can redistribute it and/or modify it under
    the terms of the MIT License (MIT).
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "EventOS.h"
#include "event.h"

#include "Log.h"

/*********************************************************
    private constants.
*********************************************************/

/*********************************************************
    private types.
*********************************************************/

/*
 * Event control block.  A event control block (ECB) is allocated to each event,
 * and stores the parameters of the event.
 */
typedef struct evtEventControlBlock
{
	xListNode				xEventListNode;							/*< List node used to place the event in the list. */
	portBASE_TYPE			xEventPriority;						/*< The priority of the event where 0 is the lowest priority. */
	portBASE_TYPE			xEventType;
	signed char				pcEventName[configMAX_EVENT_NAME_LEN];	/*< Descriptive name given to the event when created.  Facilitates debugging only. */

	xList*					pxSubscriberList;						/*< Pointer to the list of event handlers >*/
	void*					pvPayload;
	portBASE_TYPE			xPayloadSize;
} evtECB;

/*
 * Event handler control block.  A event handler control block (HCB) is allocated to each event subscriber,
 * and stores the parameters of the event handler.
 */
typedef struct evtSubscriber
{
	void* pvHandler;
	portBASE_TYPE			xEventType;
	pdEVENT_HANDLER_FUNCTION pdEventHandlerFunction;

	xListNode				xSubscriberListNode;					/*< List node used to place the event in the list. */
}evtSCB;

/* Lists for events and event handlers. --------------------*/
__PRIVATE_ xList pxEventsLists[ EVENT_PRIORITY_LAST ];	/*< Prioritized events. */
__PRIVATE_ xList pxSubscriberLists[ EVENT_TYPE_LAST ];	/*< Event handler separated by event types. */


/*********************************************************
    private attributes.
*********************************************************/
__PRIVATE_ evtECB* volatile pxCurrentECB = NULL;

/*********************************************************
    private variables.
*********************************************************/
__PRIVATE_ volatile portUBASE_TYPE uxCurrentNumberOfEvents 	= 		( portUBASE_TYPE ) 0;
__PRIVATE_ volatile portUBASE_TYPE uxCurrentNumberOfSubscribers =	( portUBASE_TYPE ) 0;
__PRIVATE_ volatile portUBASE_TYPE uxProcessStamp = 				( portUBASE_TYPE ) 0;


/*********************************************************
    private operations.
*********************************************************/

/*
 * Place the subscriber represented by pxSCB into the appropriate event queue for.
 * It is inserted at the head of the list.
 */
#define prvEvent_addSubscriberToList( pxSCB ) vList_insertHead( ( xList* ) &( pxSubscriberLists[ pxSCB->xEventType ] ), &( pxSCB->xSubscriberListNode ) );	\

/*
 * Place the event represented by pxECB into the appropriate event priority queue.
 * It is inserted at the right position in the list, considering the deadline set on ulNodeValue.
 */
#define prvEvent_addEventToList( pxECB ) vList_insert( ( xList* ) &( pxEventsLists[ pxECB->xEventPriority ] ), &( pxECB->xEventListNode ) );	\

/*
 * Place the event represented by pxECB into the appropriate event priority queue.
 * It is inserted at the right position in the list, considering the deadline set on ulNodeValue.
 */
#define prvEvent_removeEventFromList( pxECB ) vList_remove( &( pxECB->xEventListNode ) );	\


__PRIVATE_ void prvEvent_deleteECB( evtECB* pxECB );
__PRIVATE_ void prvEvent_initializeEventLists( void );
__PRIVATE_ void prvEvent_initializeSubscriberLists( void );
__PRIVATE_ void prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, portBASE_TYPE ulEventType, void* pvSubscriber );
__PRIVATE_ void prvEvent_initializeECBVariables( evtECB* pxECB, portUBASE_TYPE uxEventType, portUBASE_TYPE uxEventPriority );
__PRIVATE_ void prvEvent_incrementProcessStamp( void );
__PRIVATE_ portUBASE_TYPE prxEvent_getProcessStamp( void );
__PRIVATE_ void prvEvent_updateLifeTime( void );
__PRIVATE_ evtECB* prpEvent_getNextEvent( void );
__PRIVATE_ void prvEvent_increaseEventPriority( evtECB* pxECB );
__PRIVATE_ void prvEvent_decreaseEventPriority( evtECB* pxECB );
__PRIVATE_ portBASE_TYPE prxEvent_getEventPriority( evtECB* pxECB );
__PRIVATE_ void prvEvent_terminateEvent( evtECB* pxECB );
__PRIVATE_ void prvEvent_deleteECB( evtECB* pxECB );

/*********************************************************
    Operations implementation
*********************************************************/


__PRIVATE_ portUBASE_TYPE prxEvent_getProcessStamp( void )
{
	return uxProcessStamp;
}

__PRIVATE_ void prvEvent_initializeEventLists( void )
{
	portUBASE_TYPE uxPriority;

	for( uxPriority = 0; uxPriority < EVENT_PRIORITY_LAST; uxPriority++ )
	{
		vList_initialize( ( xList* ) &( pxEventsLists[ uxPriority ] ) );
	}
}

__PRIVATE_ void prvEvent_initializeSubscriberLists( void )
{
	unsigned portBASE_TYPE ulEventType;

	for( ulEventType = 0; ulEventType < EVENT_TYPE_LAST; ulEventType++ )
	{
		vList_initialize( ( xList* ) &( pxSubscriberLists[ ulEventType ] ) );
	}
}

__PRIVATE_ void prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, portBASE_TYPE xEventType, void* pvSubscriber )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( xEventType >= EVENT_TYPE_LAST )
	{
		xEventType = EVENT_TYPE_LAST - 1;
	}

	pxSCB->xEventType = xEventType;
	pxSCB->pdEventHandlerFunction = pFunction;
	pxSCB->pvHandler = pvSubscriber;

	vList_initializeNode( &( pxSCB->xSubscriberListNode ) );

	/* Set the pxSCB as a link back from the xListNode.  This is so we can get
	back to	the containing SCB from a generic node in a list. */
	listSET_LIST_NODE_OWNER((xListNode*) &( pxSCB->xSubscriberListNode ), pxSCB );
}

__PRIVATE_ void prvEvent_initializeECBVariables( evtECB* pxECB, portUBASE_TYPE uxEventType, portUBASE_TYPE uxEventPriority )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( uxEventType >= EVENT_TYPE_LAST )
	{
		uxEventType = EVENT_TYPE_LAST - 1;
	}

	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( uxEventPriority >= EVENT_PRIORITY_LAST )
	{
		uxEventType = EVENT_PRIORITY_LAST - 1;
	}

	pxECB->xEventType = uxEventType;
	pxECB->xEventPriority = uxEventPriority;
	pxECB->pvPayload = NULL;
	pxECB->xPayloadSize = 0;

	/*Easier to access and run over the subscribers list*/
	pxECB->pxSubscriberList = (& pxSubscriberLists[ uxEventType ]);

	vList_initializeNode( &( pxECB->xEventListNode ) );

	/* Set the pxECB as a link back from the xListNode.  This is so we can get
	back to	the containing SCB from a generic node in a list. */
	listSET_LIST_NODE_OWNER((xListNode*) &( pxECB->xEventListNode ), pxECB );
	/* Event lists are always in priority order. */
	listSET_LIST_NODE_VALUE( &( pxECB->xEventListNode ), ( portBASE_TYPE ) prxEvent_getProcessStamp());
}

__PRIVATE_ void prvEvent_incrementProcessStamp( void )
{
	uxProcessStamp++;
}


/**
	Event start scheduler. This is the method that starts the scheduler process.

    @param void
    @return void
    @author edielson
    @date   05/09/2014
*/
void vEvent_startScheduler( void )
{
	/* Start RTC */
	portSTART_RTC();

	/* Initialize the event list before scheduler start */
	prvEvent_initializeEventLists();

	/* Setting up the system for sleep mode in the specific. */
	if( xPortStartScheduler() )
	{
		/* Should not reach here as if the scheduler is running the
		function will not return. */
	}
	else
	{
		/* Error */
	}
}

/*
	Event subscribe. Function subscribes a event handler to receive notifications about as specific event

    @param pvFunction: function to be called for event notification
    	   ulEventType: type of event to be notified
    	   pvSubscriber: event handler object
    @return void
    @author Edielson
    @date   15/09/2017
*/
signed portBASE_TYPE xEvent_subscribe (pdEVENT_HANDLER_FUNCTION pFunction, portBASE_TYPE ulEventType, void* pvSubscriber)
{
	if(ulEventType >= (portBASE_TYPE)EVENT_TYPE_LAST) return pdFALSE;
	if(!pFunction) return pdFALSE;

	signed portBASE_TYPE xReturn = pdFALSE;

	evtSCB* pxNewSubscriber = (evtSCB*)pvPortMalloc(sizeof(evtSCB));
	if(pxNewSubscriber)
	{
		/*Initializing variables*/
		prvEvent_initializeSCBVariables(pxNewSubscriber,pFunction,ulEventType,pvSubscriber);

		portDISABLE_INTERRUPTS();
		{
			/*Inserting new subscriber in the respective event list*/
			uxCurrentNumberOfSubscribers++;

			if( uxCurrentNumberOfSubscribers == ( portUBASE_TYPE ) 1 )
			{
				/* This is the first subscriber to be created so do the preliminary
				required initialization. */
				prvEvent_initializeSubscriberLists();
			}
			prvEvent_addSubscriberToList( pxNewSubscriber );

			xReturn = pdPASS;
		}
		portENABLE_INTERRUPTS();
	}
	//EventOS_printLog((portCHAR*)"[subscribe] Event: %d", eEvent);
	return xReturn;
}
/*
	EventOS publish. Publishing events in a particular queue according to its priority

    @param - void* pxEventSlot
    @return void
    @author samuel/amanda
    @date   22/09/2014
*/

signed portBASE_TYPE xEvent_publish (portBASE_TYPE xEventType, portBASE_TYPE xPriority, void* pvPayload, portBASE_TYPE xPayloadSize)
{
	if (xEventType >= EVENT_TYPE_LAST) return pdFALSE;
	if (xPriority >= EVENT_PRIORITY_LAST) return pdFALSE;

	portBASE_TYPE xStatus = pdFALSE;

	evtECB* pxNewEvent = (evtECB*)pvPortMalloc(sizeof(evtECB));
	if(pxNewEvent)
	{
		prvEvent_initializeECBVariables( pxNewEvent, xEventType, xPriority );

		if( pvPayload != NULL )
		{
			pxNewEvent->pvPayload = malloc( xPayloadSize * sizeof( portCHAR ) );
			memcpy( pxNewEvent->pvPayload, pvPayload, xPayloadSize );
			pxNewEvent->xPayloadSize = xPayloadSize;

			//Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[EventOS] Malloc: %p", pxNewEvent->pvPayload );
		}

		portDISABLE_INTERRUPTS();
		{
			uxCurrentNumberOfEvents++;
			prvEvent_addEventToList( pxNewEvent );
			xStatus = pdPASS;
			//EventOS_printLog((portCHAR*)"[publish] Event: %d / Priority: %d", pxNew->pxEvent->xHeader.eEvent, pxNew->pxEvent->xHeader.ePriority);
		}
		portENABLE_INTERRUPTS();
		portGENERATE_EVENT();
	}
	return xStatus;
}

/*
	Process event. This is the method that processes the events in the queue.

    @param void
    @return void
    @author Edielson
    @date   15/09/2017
*/

void vEvent_processEvents (void)
{
	static evtSCB* pxSCB;

	pxCurrentECB = prpEvent_getNextEvent();

	while( pxCurrentECB )
	{
		//EventOS_printLog((portCHAR*)"[process] Event: %d / Priority: %d", pEvent->xHeader.eEvent,pEvent->xHeader.ePriority);

		/*take the first subscriber from the sub list related to the event*/
		listGET_OWNER_OF_NEXT_NODE( pxSCB, ( xList* ) pxCurrentECB->pxSubscriberList );
		while(pxSCB)
		{
			if( ( pxSCB->xEventType == pxCurrentECB->xEventType ) && ( pxSCB->pdEventHandlerFunction ) )
			{
				pxSCB->pdEventHandlerFunction( pxSCB->xEventType, pxSCB->pvHandler, pxCurrentECB->pvPayload, pxCurrentECB->xPayloadSize ); //call event related function
			}
			/*take the next subscriber from the sub list related to the event*/
			listGET_OWNER_OF_NEXT_NODE( pxSCB, ( xList* ) pxCurrentECB->pxSubscriberList );
		}
		/*discard event after processed by all subscribers*/
		prvEvent_terminateEvent( pxCurrentECB );
		prvEvent_updateLifeTime();
		pxCurrentECB = prpEvent_getNextEvent();
	}
}

/*
	Event update lifetime. This function check event lifetime and updates the priority

    @param void
    @return void
    @author Gabriel
    @date   05/01/2017
*/
__PRIVATE_ void prvEvent_updateLifeTime (void)
{
	portBASE_TYPE ulPriority = EVENT_PRIORITY_HIGH;
	evtECB* pxECB = NULL;

	prvEvent_incrementProcessStamp();

	while(ulPriority < EVENT_PRIORITY_LAST)
	{
		if( listIS_EMPTY((xList*) &( pxEventsLists[ ulPriority ] ) ) )
		{
			ulPriority++;
		}
		else
		{
			pxECB = listGET_OWNER_OF_HEAD_ENTRY((xList*)&( pxEventsLists[ ulPriority ] ) );

			if(((portBASE_TYPE)(prxEvent_getProcessStamp() - listGET_LIST_ITEM_VALUE( &( pxECB->xEventListNode )))) >= configEVENTOS_LIFE_TIME)
			{
				//EventOS_printLog((portCHAR*)"[update] Event: %d / Priority: %d to Priority: %d", pxEventListAux->pxEvent->xHeader.eEvent,pxEventListAux->pxEvent->xHeader.ePriority, (pxEventListAux->pxEvent->xHeader.ePriority - 1));

				portDISABLE_INTERRUPTS();
				{
					/* Remove from lower priority list */
					prvEvent_removeEventFromList( pxECB );

					/* Update ctrl variables */
					prvEvent_increaseEventPriority(pxECB);
					listSET_LIST_NODE_VALUE( &( pxECB->xEventListNode), prxEvent_getProcessStamp() );

					/* Insert from higher priority list */
					prvEvent_addEventToList( pxECB );
				}
				portENABLE_INTERRUPTS();
			}
			else ulPriority++;
		}
	}
}

/*
	EventOS dequeue event. This is the method that dequeue the events in the publish queue.

    @param tx_EventNode* pxEventList
    @return void
    @author Amanda/Samuel
    @date   20/10/2014
*/
__PRIVATE_ evtECB* prpEvent_getNextEvent( void )
{
	portBASE_TYPE xPriority = EVENT_PRIORITY_HIGH;
	evtECB* pxECB = NULL;

	/*check the event list with highest priority that is not empty*/
	while( ( xPriority < EVENT_PRIORITY_LAST ) && ( pxECB == NULL ) )
	{
		pxECB = (evtECB*) listGET_OWNER_OF_HEAD_ENTRY( ( xList* ) &( pxEventsLists[ xPriority ] ) );
		xPriority++;
	}

	return pxECB;
}

/*
	EventOS dequeue event. This is the method that dequeue the events in the publish queue.

    @param tx_EventNode* pxEventList
    @return void
    @author Amanda/Samuel
    @date   20/10/2014
*/
__PRIVATE_ void prvEvent_increaseEventPriority( evtECB* pxECB )
{
	if(!pxECB) return;
	pxECB->xEventPriority--;
	if(pxECB->xEventPriority < EVENT_PRIORITY_HIGH)
	{
		pxECB->xEventPriority = (portBASE_TYPE) EVENT_PRIORITY_HIGH;
	}
}

/*
	EventOS dequeue event. This is the method that dequeue the events in the publish queue.

    @param tx_EventNode* pxEventList
    @return void
    @author Amanda/Samuel
    @date   20/10/2014

*/
__PRIVATE_ void prvEvent_decreaseEventPriority(evtECB* pxECB)
{
	if(!pxECB) return;
	pxECB->xEventPriority++;
	if(pxECB->xEventPriority >= EVENT_PRIORITY_LAST)
	{
		pxECB->xEventPriority = (portBASE_TYPE) EVENT_PRIORITY_LAST-1;
	}
}

/*
	EventOS dequeue event. This is the method that dequeue the events in the publish queue.

    @param tx_EventNode* pxEventList
    @return void
    @author Amanda/Samuel
    @date   20/10/2014

*/
__PRIVATE_ portBASE_TYPE prxEvent_getEventPriority(evtECB* pxECB)
{
	if(!pxECB) return EVENT_PRIORITY_LAST;
	return (portBASE_TYPE)pxECB->xEventPriority;
}

__PRIVATE_ void prvEvent_terminateEvent( evtECB* pxECB )
{
	portDISABLE_INTERRUPTS();
	{
		vList_remove( &( pxECB->xEventListNode ) );
		--uxCurrentNumberOfEvents;
	}
	portENABLE_INTERRUPTS();
	prvEvent_deleteECB( pxECB );
}

__PRIVATE_ void prvEvent_deleteECB( evtECB* pxECB )
{
	/* Free up the memory allocated by the scheduler for the task.  It is up to
	the task to free any memory allocated at the application level. */
	if( pxECB->pvPayload != NULL )
	{
		//Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[EventOS] Free: %p", pxECB->pvPayload );

		vPortFree( pxECB->pvPayload );
		pxECB->pvPayload = NULL;
	}

	vPortFree( pxECB );
	pxECB = NULL;
}


/**
	Get event scheduler version

    @param void
    @return pointer to the version string
    @author edielsonpf
    @date   19/09/2017
*/
portCHAR* pxEvent_getVersion(void)
{
	return (portCHAR*) evnKERNEL_VERSION_NUMBER;
}
