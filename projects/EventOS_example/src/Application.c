
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
void Application_receiveNewEvent( portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_receiveLight( portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_createdEventCallback( portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );

volatile portULONG msTicks; // counter for 1ms Applications

void SysTick_Handler(void)
{
	msTicks++;

	long id;
	portULONG xVar = msTicks;

	//Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"Handling new Application interrupt\n");

	if( msTicks % 1000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: TICK");
		//xEvent_publish(EVENT_SYS_TICK, EVENT_PRIORITY_LOW, NULL, 0);
		id = uxEvent_getEventID( "myevent3", strlen( "myevent3" ) );
		xEvent_publish( id, EVENT_PRIORITY_HIGH, &xVar, sizeof( xVar ) );
		//xEvent_publish(EVENT_SYS_ETHERNET, EVENT_PRIORITY_MEDIUM, NULL, 0);
		//xEvent_publish(EVENT_APP_TEMPERATURE, EVENT_PRIORITY_MEDIUM, NULL, 0);
		//xEvent_publish(EVENT_SYS_UART, EVENT_PRIORITY_MEDIUM, NULL, 0);
		//xEvent_publish(EVENT_SYS_USB, EVENT_PRIORITY_MEDIUM, NULL, 0);
		//xEvent_publish(EVENT_APP_TEMPERATURE, EVENT_PRIORITY_MEDIUM, NULL, 0);
	}
	if( msTicks % 5000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: ETHERNET");
		xEvent_publish(EVENT_SYS_ETHERNET, EVENT_PRIORITY_MEDIUM, NULL, 0);
	}
	if( msTicks % 10000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: Application");
		xEvent_publish(EVENT_SYS_SYSTICK, EVENT_PRIORITY_HIGH, NULL, 0);

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
	char* evt1 = "myevent1";
	char* evt2 = "myevent2";
	char* evt3 = "myevent3";
	char* evt4 = "myevent4";
	long id_in;
	long id_out;

	Application_initI2C();

	led2_init();
	light_enable();

	id_in = uxEvent_createEvent( evt1, strlen( evt1 ) );
	id_in = uxEvent_createEvent( evt2, strlen( evt2 ) );
	id_in = uxEvent_createEvent( evt3, strlen( evt3 ) );
	id_in = uxEvent_createEvent( evt4, strlen( evt4 ) );

	id_out = uxEvent_getEventID( evt3, strlen( evt1 ) );

	if( id_out != pdFAIL )
	{
		xEvent_subscribe( Application_createdEventCallback, id_out, NULL );
	}

	xEvent_subscribe( Application_receiveNewEvent, EVENT_SYS_TICK, NULL );
	xEvent_subscribe( Application_receiveNewEvent, EVENT_SYS_SYSTICK, NULL );
	xEvent_subscribe( Application_receiveNewEvent, EVENT_SYS_ETHERNET, NULL );
	xEvent_subscribe( Application_receiveLight, EVENT_APP_LIGHT, NULL );

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

void Application_receiveLight(portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize)
{
	int32_t* iLight = (int32_t*)pvPayload;

	Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Received light: %d", *iLight);
}

void Application_receiveNewEvent(portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize)
{
	portBASE_TYPE xLight;

	switch (EventType)
	{
		case EVENT_SYS_SYSTICK:
			Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Systick)");
			xLight = light_read();
			xEvent_publish( EVENT_APP_LIGHT, EVENT_PRIORITY_MEDIUM, &xLight, sizeof( xLight ) );
		    led2_invert();
			break;
		case EVENT_SYS_TICK:
			Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Tick)");
			break;
		case EVENT_SYS_ETHERNET:
			Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Ethernet)");
			break;
		default:
			Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_ERROR,"[app] Event not expected");
			break;
	}
}

void Application_createdEventCallback( portBASE_TYPE EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize )
{
	int32_t* iValue = ( int32_t* ) pvPayload;

	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] xxxx Received data from created event: %d; Event name: [%s]", *iValue, EventName );
}

portULONG Application_getMsTicks(void)
{
	return msTicks;
}
