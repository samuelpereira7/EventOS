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

#ifndef EVENT_H_
#define EVENT_H_

#include "portable.h"
#include "list.h"
#include "AVLTree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * MACROS AND DEFINITIONS
 *----------------------------------------------------------*/

#define evnKERNEL_VERSION_NUMBER "V1.0.1"

/**
 * event. h
 *
 * Type by which event handlers are referenced.  For example, a call to Event_subscribe
 * returns (via a pointer parameter) an pEventHandle variable that can then
 * be used as a parameter to vEvent_unsubscribe to unsubscribe the handler.
 *
 */
typedef void* pvEventHandle;

#define EVENT_RUNTIME_EVENTS	10

#define EVENT_TOTAL_EVENTS     	( EVENT_RUNTIME_EVENTS )

#define EVENT_NAME_MAX_LEN		20

typedef enum enu_Priorities
{
	EVENT_PRIORITY_HIGH = 0,
	EVENT_PRIORITY_MEDIUM,
	EVENT_PRIORITY_LOW,

	/*Must be the last one*/
	EVENT_PRIORITY_LAST
}tenuEventPriority;

/* Defines the prototype to which event handler functions must conform.
 * Parameters: event handler, event name, subscribe handler, event payload, size of event payload */
typedef void ( *pdEVENT_HANDLER_FUNCTION )( pvEventHandle, char*, void*, void*, portBASE_TYPE );

/*********************************************************
    Public Operations
*********************************************************/
void					vEvent_initSystem( void );
void 					vEvent_startScheduler( void );
pvEventHandle			uxEvent_createEvent( portCHAR* pcEventName, portUBASE_TYPE uxNameLength );
pvEventHandle			uxEvent_deleteEvent( pvEventHandle pvNodeHandler );
pvEventHandle			uxEvent_getEventHandler( portCHAR* pcEventName, portUBASE_TYPE uxNameLength );
signed portBASE_TYPE 	xEvent_subscribe( pdEVENT_HANDLER_FUNCTION pvFunction, pvEventHandle pvEventType, void* pvSubscriber );
signed portBASE_TYPE	xEvent_publish( pvEventHandle pvEventType, portUBASE_TYPE uxPriority, void* pvPayload, portBASE_TYPE xPayloadSize );
portCHAR*  				pxEvent_getVersion( void );

/*-----------------------------------------------------------
 * SCHEDULER INTERNALS AVAILABLE FOR PORTING PURPOSES
 * documented in EventOS.h
 *----------------------------------------------------------*/
void 					vEvent_processEvents( void );

#ifdef __cplusplus
}
#endif

#endif /* EVENT_H_ */
