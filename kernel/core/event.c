
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
#include "hash.h"

#include "Log.h"

/*********************************************************
    private constants.
*********************************************************/
#define CAST_EVENT_TYPE ttag_treeNodePtr

/*********************************************************
    private types.
*********************************************************/
typedef struct tag_Event {

	xList pxSubscriberList;	/*< Event handler separated by event types. */
	portCHAR *pcEventName;
	portHASH_TYPE xHash;

}ttag_EventType;


/*
 * Event control block.  A event control block (ECB) is allocated to each event,
 * and stores the parameters of the event.
 */
typedef struct evtEventControlBlock
{
	xListNode				xEventListNode;							/*< List node used to place the event in the list. */
	portBASE_TYPE			xEventPriority;							/*< The priority of the event where 0 is the lowest priority. */
	pvEventHandle			pvEventType;
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
	void* 						pvHandler;
	pvEventHandle				pvEventType;
	pdEVENT_HANDLER_FUNCTION 	pdEventHandlerFunction;
	xListNode					xSubscriberListNode;					/*< List node used to place the event in the list. */
}evtSCB;

/*********************************************************
    private attributes.
*********************************************************/
__PRIVATE_ evtECB* volatile pxCurrentECB = NULL;
/* Lists for events and event handlers. --------------------*/
__PRIVATE_ xList pxEventsLists[ EVENT_PRIORITY_LAST ];	/*< Prioritized events. */
__PRIVATE_ pvEventHandle ptagRoot;

/*********************************************************
    private variables.
*********************************************************/
__PRIVATE_ volatile portUBASE_TYPE uxCurrentNumberOfEvents 	= 		( portUBASE_TYPE ) 0;
__PRIVATE_ volatile portUBASE_TYPE uxCurrentNumberOfSubscribers =	( portUBASE_TYPE ) 0;
__PRIVATE_ volatile portUBASE_TYPE uxProcessStamp = 				( portUBASE_TYPE ) 0;
__PRIVATE_ volatile portUBASE_TYPE uxNumberOfEventsCreated =		( portUBASE_TYPE ) 0;


/*********************************************************
    private operations.
*********************************************************/

/*
 * Place the subscriber represented by pxSCB into the appropriate event queue for.
 * It is inserted at the head of the list.
 */
#define prvEvent_addSubscriberToList( pxSCB ) vList_insertHead( ( xList* ) &( ((ttag_EventType*)(((CAST_EVENT_TYPE)pxSCB->pvEventType)->pvPayload))->pxSubscriberList ), &( pxSCB->xSubscriberListNode ) );	\

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


__PRIVATE_ void 			prvEvent_initializeEventLists( void );
__PRIVATE_ void 			prvEvent_initializeSubscriberLists( pvEventHandle pxRoot );
__PRIVATE_ ttag_EventType* 	Event_AllocateEventType( portCHAR* pcEventName, portUBASE_TYPE uxNameLength );
__PRIVATE_ void 			prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, pvEventHandle pxEventType, void* pvSubscriber );
__PRIVATE_ void 			prvEvent_initializeECBVariables( evtECB* pxECB, pvEventHandle pvEventType, portUBASE_TYPE uxEventPriority );
__PRIVATE_ void 			prvEvent_incrementProcessStamp( void );
__PRIVATE_ portUBASE_TYPE 	prxEvent_getProcessStamp( void );
__PRIVATE_ void 			prvEvent_updateLifeTime( void );
__PRIVATE_ evtECB* 			prpEvent_getNextEvent( void );
__PRIVATE_ void 			prvEvent_increaseEventPriority( evtECB* pxECB );
__PRIVATE_ void 			prvEvent_decreaseEventPriority( evtECB* pxECB );
__PRIVATE_ portBASE_TYPE 	prxEvent_getEventPriority( evtECB* pxECB );
__PRIVATE_ void 			prvEvent_terminateEvent( evtECB* pxECB );
__PRIVATE_ void 			prvEvent_deleteECB( evtECB* pxECB );
__PRIVATE_ ttag_EventType* 	Event_DeallocateEventType( ttag_EventType* ptagEvent );

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

__PRIVATE_ void prvEvent_initializeSubscriberLists( pvEventHandle pvRoot )
{
	/* The initialization of this list should be done just for default events (see enum "tenuEventType" in "portmacro.h" file).
	 * This operation is performed automatically for events created at run-time (see function "uxEvent_createEvent").
	 */
	if(pvRoot == NULL) return;
	vList_initialize( ( xList* ) &( ( (ttag_EventType*) ( ( CAST_EVENT_TYPE )pvRoot )->pvPayload )->pxSubscriberList ) );

}

__PRIVATE_ ttag_EventType* Event_AllocateEventType( portCHAR* pcEventName, portUBASE_TYPE uxNameLength ) {

	ttag_EventType* ptagAlocatedEvent = (ttag_EventType*)pvPortMalloc(sizeof(ttag_EventType));

	if( ptagAlocatedEvent != NULL ) {
		/* initializing the list of subscribers of the new event */
		vList_initialize( ( xList* ) &( ptagAlocatedEvent->pxSubscriberList ) );

		ptagAlocatedEvent->pcEventName = (portCHAR*) pvPortMalloc( uxNameLength + 1 );

		if( ptagAlocatedEvent->pcEventName != NULL ) {
			/* saving the event name */
			strncpy( (portCHAR*) ptagAlocatedEvent->pcEventName, (portCHAR*)pcEventName, uxNameLength + 1 );

			ptagAlocatedEvent->xHash = xHash_calculateHash( ptagAlocatedEvent->pcEventName, strlen((const char *)ptagAlocatedEvent->pcEventName) );

			if( ptagAlocatedEvent->xHash == pdFAIL ) {
				/* wild error, aborting */
				vPortFree( ptagAlocatedEvent->pcEventName );
				ptagAlocatedEvent->pcEventName = NULL;
				vPortFree( ptagAlocatedEvent );
				return NULL;
			}
		}
		else {
			vPortFree( ptagAlocatedEvent );
			return NULL;
		}
	}

	return ptagAlocatedEvent;
}

__PRIVATE_ void prvEvent_initializeSCBVariables( evtSCB* pxSCB, pdEVENT_HANDLER_FUNCTION pFunction, pvEventHandle pvEventType, void* pvSubscriber )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( pvEventType == NULL )
		return;

	pxSCB->pvEventType = pvEventType;
	pxSCB->pdEventHandlerFunction = pFunction;
	pxSCB->pvHandler = pvSubscriber;

	vList_initializeNode( &( pxSCB->xSubscriberListNode ) );

	/* Set the pxSCB as a link back from the xListNode.  This is so we can get
	back to	the containing SCB from a generic node in a list. */
	listSET_LIST_NODE_OWNER((xListNode*) &( pxSCB->xSubscriberListNode ), pxSCB );
}

__PRIVATE_ void prvEvent_initializeECBVariables( evtECB* pxECB, pvEventHandle pvEventType, portUBASE_TYPE uxEventPriority )
{
	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( pvEventType == NULL )
		return;

	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( uxEventPriority >= EVENT_PRIORITY_LAST )
	{
		uxEventPriority = EVENT_PRIORITY_LAST - 1;
	}

	pxECB->pvEventType = pvEventType;
	pxECB->xEventPriority = uxEventPriority;
	pxECB->pvPayload = NULL;
	pxECB->xPayloadSize = 0;

	/*Easier to access and run over the subscribers list*/
	pxECB->pxSubscriberList = ( &( ((ttag_EventType*)(((CAST_EVENT_TYPE)pvEventType)->pvPayload))->pxSubscriberList ) );

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
	@return pvEventHandle - identifier (type) of the new event or NULL in case of failure
	@author samuelpereira7/jponeticarvalho
	@date   18/09/2018
*/
pvEventHandle uxEvent_createEvent( portCHAR* pcEventName, portUBASE_TYPE uxNameLength )
{
	if( pcEventName == NULL ) return pdFAIL;
	if( uxNameLength > EVENT_NAME_MAX_LEN ) return pdFAIL;
	if( uxNumberOfEventsCreated > EVENT_TOTAL_EVENTS) return pdFAIL;

	ttag_EventType* ptagEvent = NULL;
	CAST_EVENT_TYPE	pvNewEventHandler = NULL;

	portDISABLE_INTERRUPTS();
	{
		ptagEvent = Event_AllocateEventType(pcEventName, uxNameLength);


		if(ptagEvent != NULL) {
			/* saving the event into avl tree */
			ptagRoot = (pvEventHandle)AVLTree_insertNode((CAST_EVENT_TYPE)ptagRoot, (void*)ptagEvent, &(pvNewEventHandler));
			uxNumberOfEventsCreated++;
		}
		else
		{
			return NULL;
		}
	}
	portENABLE_INTERRUPTS();

	return pvNewEventHandler;
}

/**
	This method creates new events. Such events can be used later on.

	@param     pcEventName: event name
			   uxNameLength: length of the event name
	@return portUBASE_TYPE - identifier (type) of the new event or pdFAIL in case of failure
	@author jponeticarvalho
	@date   24/09/2018
*/
pvEventHandle uxEvent_deleteEvent( pvEventHandle pvNodeHandler )
{
	if( pvNodeHandler == NULL ) return pdFAIL;

	void* pvAux = NULL;

	portDISABLE_INTERRUPTS();
	{
		pvAux = ((CAST_EVENT_TYPE)pvNodeHandler)->pvPayload;
		ptagRoot = (pvEventHandle)AVLTree_removeSpecificNode((CAST_EVENT_TYPE)ptagRoot, &pvNodeHandler);
		pvAux = Event_DeallocateEventType((ttag_EventType*)(pvAux));
		uxNumberOfEventsCreated--;
	}
	portENABLE_INTERRUPTS();

	return pvNodeHandler;
}

/**
	This method searches for a an event by its name.
 	@param     pcEventName: event name
			   uxNameLength: length of the event name
	@return pvEventHandle - identifier (type) of the new event or NULL in case of failure
	@author jponeticarvalho
	@date   18/09/2018
*/
pvEventHandle uxEvent_getEventHandler( portCHAR* pcEventName, portUBASE_TYPE uxNameLength )
{
	if( pcEventName == NULL ) return NULL;
	if( uxNameLength > EVENT_NAME_MAX_LEN ) return NULL;

	pvEventHandle pxReturnValue = pdFAIL;
	ttag_EventType tagSearched;
	tagSearched.pcEventName = (portCHAR*)pvPortMalloc(uxNameLength+1);

	tagSearched.xHash = xHash_calculateHash( pcEventName, uxNameLength );
	strncpy( (portCHAR*) tagSearched.pcEventName, (portCHAR*)pcEventName, uxNameLength+1 );

	pxReturnValue = (pvEventHandle)AVLTree_getHandler((CAST_EVENT_TYPE)ptagRoot, (void*)(&tagSearched));

	vPortFree(tagSearched.pcEventName);

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
signed portBASE_TYPE xEvent_subscribe( pdEVENT_HANDLER_FUNCTION pvFunction, pvEventHandle pvEventType, void* pvSubscriber )
{
	if( pvEventType == NULL ) return pdFALSE;
	if( !pvFunction ) return pdFALSE;

	signed portBASE_TYPE xReturn = pdFALSE;

	evtSCB* pxNewSubscriber = (evtSCB*)pvPortMalloc(sizeof(evtSCB));
	if(pxNewSubscriber)
	{
		/*Initializing variables*/
		prvEvent_initializeSCBVariables( pxNewSubscriber, pvFunction, pvEventType, pvSubscriber );

		portDISABLE_INTERRUPTS();
		{
			if ( ( (ttag_EventType*) ( ( CAST_EVENT_TYPE )pvEventType )->pvPayload ) != NULL )
				/*Inserting new subscriber in the respective event list*/
				( ( (ttag_EventType*) ( ( CAST_EVENT_TYPE )pvEventType )->pvPayload )->pxSubscriberList ).uxNumberOfNodes++;

			if( ( ( (ttag_EventType*) ( ( CAST_EVENT_TYPE )pvEventType )->pvPayload )->pxSubscriberList ).uxNumberOfNodes == ( portUBASE_TYPE ) 1 )
			{
				/* This is the first subscriber to be created so do the preliminary
				required initialization. */
				prvEvent_initializeSubscriberLists(pvEventType);
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

signed portBASE_TYPE xEvent_publish( pvEventHandle pvEventType, portUBASE_TYPE uxPriority, void* pvPayload, portBASE_TYPE xPayloadSize )
{
	if( pvEventType == NULL ) return pdFALSE;
	if( uxPriority >= EVENT_PRIORITY_LAST ) return pdFALSE;

	portBASE_TYPE xStatus = pdFALSE;

	evtECB* pxNewEvent = (evtECB*)pvPortMalloc(sizeof(evtECB));
	if(pxNewEvent)
	{
		prvEvent_initializeECBVariables( pxNewEvent, pvEventType, uxPriority );

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
			if( ( pxSCB->pvEventType == pxCurrentECB->pvEventType ) && ( pxSCB->pdEventHandlerFunction ) )
			{
				pxSCB->pdEventHandlerFunction( (void*)pxSCB->pvEventType, (char*)( ((ttag_EventType*)(((CAST_EVENT_TYPE)pxSCB->pvEventType)->pvPayload))->pcEventName ), pxSCB->pvHandler, pxCurrentECB->pvPayload, pxCurrentECB->xPayloadSize ); //call event related function
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

__PRIVATE_ ttag_EventType* Event_DeallocateEventType( ttag_EventType* ptagEvent ) {

	if( ptagEvent == NULL ) return NULL;

	//TODO Implement Dealocater function to xList
	vList_deinitialize(&ptagEvent->pxSubscriberList);
	vPortFree( ptagEvent->pcEventName );
	ptagEvent->pcEventName = NULL;

	vPortFree(ptagEvent);
	ptagEvent = NULL;

	return ptagEvent;
}

portCHAR AVLTree_nodeCmp(void* pvFirst, void* pvSecond) {

	if( ((ttag_EventType*)pvFirst)->xHash > ((ttag_EventType*)pvSecond)->xHash ) {
		return BIGGER;
	}
	else if( ((ttag_EventType*)pvFirst)->xHash < ((ttag_EventType*)pvSecond)->xHash ) {
		return SMALLER;
	}
	else {
		if( strcmp( ((ttag_EventType*)pvFirst)->pcEventName,  ((ttag_EventType*)pvSecond)->pcEventName ) == 0 )
			return EQUALS;
		else
			return BIGGER;
	}

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
