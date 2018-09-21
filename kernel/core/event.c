
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
#include "AVLTree.h"

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
	portBASE_TYPE			xEventPriority;							/*< The priority of the event where 0 is the lowest priority. */
	ttag_nodeptr			pxEventType;
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
	ttag_nodeptr			pxEventType;
	pdEVENT_HANDLER_FUNCTION pdEventHandlerFunction;

	xListNode				xSubscriberListNode;					/*< List node used to place the event in the list. */
}evtSCB;

/* Lists for events and event handlers. --------------------*/
__PRIVATE_ xList pxEventsLists[ EVENT_PRIORITY_LAST ];	/*< Prioritized events. */
__PRIVATE_ xList pxSubscriberLists[ EVENT_TOTAL_EVENTS ];	/*< Event handler separated by event types. */
__PRIVATE_ portCHAR* pcEventNameList[ EVENT_TOTAL_EVENTS ];
__PRIVATE_ portHASH_TYPE pxHashList[ EVENT_TOTAL_EVENTS ];
__PRIVATE_ ttag_nodeptr ptagRoot;


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
__PRIVATE_ volatile portUBASE_TYPE uxNumberOfEventsCreated =		( portUBASE_TYPE ) EVENT_TYPE_LAST;


/*********************************************************
    private operations.
*********************************************************/

/*
 * Place the subscriber represented by pxSCB into the appropriate event queue for.
 * It is inserted at the head of the list.
 */
#define prvEvent_addSubscriberToList( pxSCB ) vList_insertHead( ( xList* ) &( pxSCB->pxEventType->Event.pxSubscriberList ), &( pxSCB->xSubscriberListNode ) );	\

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


__PRIVATE_ void prvEvent_initializeEventLists( void );
__PRIVATE_ void prvEvent_initializeSubscriberLists( ttag_nodeptr pxRoot );
__PRIVATE_ void prvEvent_initializeEventNameList( void );
__PRIVATE_ void prvEvent_initializeHashList( void );
__PRIVATE_ void prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, ttag_nodeptr pxEventType, void* pvSubscriber );
__PRIVATE_ void prvEvent_initializeECBVariables( evtECB* pxECB, ttag_nodeptr pxEventType, portUBASE_TYPE uxEventPriority );
__PRIVATE_ portHASH_TYPE xEvent_calculateHash( portCHAR* pcArray, portBASE_TYPE xArrayLenght );
__PRIVATE_ portBASE_TYPE prxEvent_checkEventType( ttag_nodeptr xEventType );
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

__PRIVATE_ void prvEvent_initializeSubscriberLists( ttag_nodeptr pxRoot )
{
	/* The initialization of this list should be done just for default events (see enum "tenuEventType" in "portmacro.h" file).
	 * This operation is performed automatically for events created at run-time (see function "uxEvent_createEvent").
	 */
	if(pxRoot == NULL) return;
	vList_initialize( ( xList* ) &( pxRoot->Event.pxSubscriberList ) );
	prvEvent_initializeSubscriberLists(pxRoot->ptagLeft);
	prvEvent_initializeSubscriberLists(pxRoot->ptagRight);

}

__PRIVATE_ void prvEvent_initializeEventNameList( void )
{
	portUBASE_TYPE uxEventType;

	/* The initialization is done for the entire list (for the default events and for those that can be created. */
	for( uxEventType = 0; uxEventType < EVENT_TOTAL_EVENTS; uxEventType++ )
	{
		pcEventNameList[ uxEventType ] = NULL;
	}
}

__PRIVATE_ void prvEvent_initializeHashList( void )
{
	portUBASE_TYPE uxEventType;

	/* The initialization is done for the entire list (for the default events and for those that can be created. */
	for( uxEventType = 0; uxEventType < EVENT_TOTAL_EVENTS; uxEventType++ )
	{
		pxHashList[ uxEventType ] = 0;
	}
}

__PRIVATE_ void prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, ttag_nodeptr pxEventType, void* pvSubscriber )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( pxEventType == NULL )
		return;

	pxSCB->pxEventType = pxEventType;
	pxSCB->pdEventHandlerFunction = pFunction;
	pxSCB->pvHandler = pvSubscriber;

	vList_initializeNode( &( pxSCB->xSubscriberListNode ) );

	/* Set the pxSCB as a link back from the xListNode.  This is so we can get
	back to	the containing SCB from a generic node in a list. */
	listSET_LIST_NODE_OWNER((xListNode*) &( pxSCB->xSubscriberListNode ), pxSCB );
}

__PRIVATE_ void prvEvent_initializeECBVariables( evtECB* pxECB, ttag_nodeptr pxEventType, portUBASE_TYPE uxEventPriority )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( pxEventType == NULL )
		return;

	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( uxEventPriority >= EVENT_PRIORITY_LAST )
	{
		uxEventPriority = EVENT_PRIORITY_LAST - 1;
	}

	pxECB->pxEventType = pxEventType;
	pxECB->xEventPriority = uxEventPriority;
	pxECB->pvPayload = NULL;
	pxECB->xPayloadSize = 0;

	/*Easier to access and run over the subscribers list*/
	pxECB->pxSubscriberList = (& pxEventType->Event.pxSubscriberList);

	vList_initializeNode( &( pxECB->xEventListNode ) );

	/* Set the pxECB as a link back from the xListNode.  This is so we can get
	back to	the containing SCB from a generic node in a list. */
	listSET_LIST_NODE_OWNER((xListNode*) &( pxECB->xEventListNode ), pxECB );
	/* Event lists are always in priority order. */
	listSET_LIST_NODE_VALUE( &( pxECB->xEventListNode ), ( portBASE_TYPE ) prxEvent_getProcessStamp());
}

/*
 * 			TODO
 * Insert a new hash algorithm
 */

__PRIVATE_ portHASH_TYPE xEvent_calculateHash( portCHAR* pcArray, portBASE_TYPE xArrayLenght )
{
	/* Random hashing implementation. Change it for the love of God. */
	if( pcArray == NULL ) return pdFAIL;
	if( xArrayLenght <= 0 ) return pdFAIL;

	portUBASE_TYPE xHash = 377767;
	portBASE_TYPE xIndex;

	for( xIndex = 0; xIndex < xArrayLenght; xIndex++ )
	{
		xHash *= pcArray[ xIndex ];
	}

	if( xHash == 0 )
	{
		xHash = 52341235;
	}

	return xHash;
}

/*
 * 			TODO
 * When the AVL tree is inserted, change this function to validade.
 */
__PRIVATE_ portBASE_TYPE prxEvent_checkEventType( ttag_nodeptr pxEventType )
{
	if(AVLTree_getHandler(ptagRoot, pxEventType->Event) == pxEventType) return pdTRUE;
	else return pdFAIL;
}

__PRIVATE_ void prvEvent_incrementProcessStamp( void )
{
	uxProcessStamp++;
}

/**
	This function initialize all the components of the system.
	It must be called before the scheduler starter function.

    @param void
    @return void
    @author samuelpereira7
    @date   07/08/2018
*/
void vEvent_initSystem( void )
{
	/* Start RTC */
	portSTART_RTC();

	/* Initalize the pointer to event tree with NULL*/
	ptagRoot = NULL;

	/* Initialize the event list before scheduler starts */
	prvEvent_initializeEventLists();

	/* Initialize the list of event names before scheduler starts */
	prvEvent_initializeEventNameList();

	/* Initialize the list of hash before scheduler starts */
	prvEvent_initializeHashList();
}

/**
	Event start scheduler. This is the method that starts the scheduler process.
	It must be called after the system initializer function.

    @param void
    @return void
    @author edielson
    @date   05/09/2014
*/
void vEvent_startScheduler( void )
{
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


/**
	This method creates new events. Such events can be used later on.

	@param     pcEventName: event name
			   uxNameLength: length of the event name
	@return portUBASE_TYPE - identifier (type) of the new event or pdFAIL in case of failure
	@author samuelpereira7/jponeticarvalho
	@date   18/09/2018
*/
portUBASE_TYPE uxEvent_createEvent( portCHAR* pcEventName, portUBASE_TYPE uxNameLength )
{
	if( pcEventName == NULL ) return pdFAIL;
	if( uxNameLength > EVENT_NAME_MAX_LEN ) return pdFAIL;
	if( uxNumberOfEventsCreated > EVENT_TOTAL_EVENTS) return pdFAIL;

	ttag_Event tagEvent;

	/* initializing the list of subscribers of the new event */
	vList_initialize( ( xList* ) &( tagEvent.pxSubscriberList ) );

	tagEvent.pcEventName = (portCHAR*) pvPortMalloc( uxNameLength + 1 );

	if( tagEvent.pcEventName != NULL )
	{
		/* saving the event name */
		strncpy( (portCHAR*) tagEvent.pcEventName, (portCHAR*)pcEventName, uxNameLength + 1 );

		tagEvent.xHash = xEvent_calculateHash( tagEvent.pcEventName, strlen((const char *)tagEvent.pcEventName) );

		if( tagEvent.xHash != pdFAIL )
		{
			/* saving the event into avl tree */
			ptagRoot = AVLTree_insertNode(ptagRoot, tagEvent);
			uxNumberOfEventsCreated++;
		}
		else
		{
			/* wild error, aborting */
			free( tagEvent.pcEventName );
			tagEvent.pcEventName = NULL;

			return NULL;
		}
	}
	else
	{
		return NULL;
	}

	return AVLTree_getHandler(ptagRoot, tagEvent);
}

/**
	This method searches for a an event by its name.
 	@param     pcEventName: event name
			   uxNameLength: length of the event name
	@return ttag_nodeptr - identifier (type) of the new event or NULL in case of failure
	@author jponeticarvalho
	@date   18/09/2018
*/
ttag_nodeptr uxEvent_getEventHandler( portCHAR* pcEventName, portUBASE_TYPE uxNameLength )
{
	if( pcEventName == NULL ) return NULL;
	if( uxNameLength > EVENT_NAME_MAX_LEN ) return NULL;

	ttag_nodeptr pxReturnValue = pdFAIL;
	ttag_Event tagxSearched;

	tagxSearched.xHash = xEvent_calculateHash( pcEventName, uxNameLength );
	strncpy( (portCHAR*) tagxSearched.pcEventName, (portCHAR*)pcEventName, uxNameLength );

	pxReturnValue = AVLTree_getHandler(ptagRoot, tagxSearched);

 	return pxReturnValue;
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
signed portBASE_TYPE xEvent_subscribe( pdEVENT_HANDLER_FUNCTION pvFunction, ttag_nodeptr pxEventType, void* pvSubscriber )
{
	if( pxEventType == NULL ) return pdFALSE;
	if( !pvFunction ) return pdFALSE;

	signed portBASE_TYPE xReturn = pdFALSE;

	evtSCB* pxNewSubscriber = (evtSCB*)pvPortMalloc(sizeof(evtSCB));
	if(pxNewSubscriber)
	{
		/*Initializing variables*/
		prvEvent_initializeSCBVariables( pxNewSubscriber, pvFunction, pxEventType, pvSubscriber );

		portDISABLE_INTERRUPTS();
		{
			/*Inserting new subscriber in the respective event list*/
			uxCurrentNumberOfSubscribers++;

			if( uxCurrentNumberOfSubscribers == ( portUBASE_TYPE ) 1 )
			{
				/* This is the first subscriber to be created so do the preliminary
				required initialization. */
				prvEvent_initializeSubscriberLists(ptagRoot);
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

signed portBASE_TYPE xEvent_publish( ttag_nodeptr pxEventType, portUBASE_TYPE uxPriority, void* pvPayload, portBASE_TYPE xPayloadSize )
{
	if( pxEventType == NULL ) return pdFALSE;
	if( uxPriority >= EVENT_PRIORITY_LAST ) return pdFALSE;

	portBASE_TYPE xStatus = pdFALSE;

	evtECB* pxNewEvent = (evtECB*)pvPortMalloc(sizeof(evtECB));
	if(pxNewEvent)
	{
		prvEvent_initializeECBVariables( pxNewEvent, pxEventType, uxPriority );

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
			if( ( pxSCB->pxEventType == pxCurrentECB->pxEventType ) && ( pxSCB->pdEventHandlerFunction ) )
			{
				pxSCB->pdEventHandlerFunction( pxSCB->pxEventType, (char*)pxSCB->pxEventType->Event.pcEventName, pxSCB->pvHandler, pxCurrentECB->pvPayload, pxCurrentECB->xPayloadSize ); //call event related function
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
