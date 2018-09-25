
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event.h"

#include "Application.h"
#include "Log.h"

#include "leds.h"
#include "light.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"


/*********************************************************
    private operations.
*********************************************************/

void Application_init( void );
void Application_initI2C( void );
void Application_initSysTick( void );
void Application_receiveNewEvent( ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_receiveLight( ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_createdEventCallback( ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );

volatile portULONG msTicks; // counter for 1ms Applications
ttag_nodeptr event1;
ttag_nodeptr event2;
ttag_nodeptr event3;
ttag_nodeptr event4;
ttag_nodeptr event5;

void SysTick_Handler(void)
{
	msTicks++;

	ttag_nodeptr eventHandler;
	portULONG xVar = msTicks;

	//Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"Handling new Application interrupt\n");

	if( msTicks % 1000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: TICK");
		xEvent_publish(event1, EVENT_PRIORITY_HIGH, NULL, 0);
		xEvent_publish(event2, EVENT_PRIORITY_LOW, NULL, 0);
	}
	if( msTicks % 5000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: ETHERNET");
		xEvent_publish(event3, EVENT_PRIORITY_MEDIUM, NULL, 0);
	}
	if( msTicks % 10000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: Application");
		xEvent_publish(event5, EVENT_PRIORITY_MEDIUM, &xVar, sizeof(xVar));
	    msTicks = 0;
	}
}



void Application_initI2C(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}


void Application_new( void )
{
	char* evt1 = "myevent1"; //systick
	char* evt2 = "myevent2"; //tick
	char* evt3 = "myevent3";
	char* evt4 = "myevent4";
	char* evt5 = "myevent5";
	ttag_nodeptr id_out;

	Application_initI2C();

	led2_init();
	light_enable();

	event1 = uxEvent_createEvent( evt1, strlen( evt1 ) );
	event2 = uxEvent_createEvent( evt2, strlen( evt2 ) );
	event3 = uxEvent_createEvent( evt3, strlen( evt3 ) );
	event4 = uxEvent_createEvent( evt4, strlen( evt4 ) );
	event5 = uxEvent_createEvent( evt5, strlen( evt5 ) );
	xEvent_subscribe( Application_receiveNewEvent, event1, NULL );
	xEvent_subscribe( Application_receiveNewEvent, event2, NULL );
	xEvent_subscribe( Application_receiveNewEvent, event3, NULL );
	xEvent_subscribe( Application_receiveLight, event4, NULL );
	xEvent_subscribe( Application_createdEventCallback, event5, NULL );

	led2_on();

	Application_initSysTick();
}


void Application_initSysTick( void )
{
	// Setup Systick Timer to interrupt at 1 msec intervals
	if (SysTick_Config(SystemCoreClock/1000))
	{
	    while (1);  // Capture error
	}
	return;
}

void Application_delete( void )
{

}

void Application_receiveLight(ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize)
{
	int32_t* iLight = (int32_t*)pvPayload;

	Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Received light: %d", *iLight);
}

void Application_receiveNewEvent(ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize)
{
	portBASE_TYPE xLight;

	if(EventType == event1) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Systick)");
		xLight = light_read();
		xEvent_publish( event4, EVENT_PRIORITY_MEDIUM, &xLight, sizeof( xLight ) );
		led2_invert();
	}
	else if(strcmp(EventType->Event.pcEventName, "myevent2" ) == 0) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Tick)");
	}
	else if(strcmp(EventType->Event.pcEventName, "myevent3" ) == 0) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Ethernet)");
	}
	else {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_ERROR,"[app] Event not expected");
	}
}

void Application_createdEventCallback( ttag_nodeptr EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize )
{
	int32_t* iValue = ( int32_t* ) pvPayload;

	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] xxxx Received data from created event: %d; Event name: [%s]", *iValue, EventName );
}

portULONG Application_getMsTicks(void)
{
	return msTicks;
}
