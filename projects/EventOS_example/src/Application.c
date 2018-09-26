
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
void Application_receiveNewEvent( pvEventHandle pvEventHandler, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_receiveLight( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_createdEventCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_genericCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );

volatile portULONG msTicks; // counter for 1ms Applications
pvEventHandle event[15];

void SysTick_Handler(void)
{
	msTicks++;

	portULONG xVar = msTicks;

	//Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"Handling new Application interrupt\n");

	if( msTicks % 1000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: TICK");
		xEvent_publish(event[0], EVENT_PRIORITY_HIGH, NULL, 0);
		xEvent_publish(event[1], EVENT_PRIORITY_LOW, NULL, 0);
	}
	if( msTicks % 5000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: ETHERNET");
		xEvent_publish(event[2], EVENT_PRIORITY_MEDIUM, NULL, 0);
	}
	if( msTicks % 10000 == 0 )
	{
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Publishing new event: Application");
		xEvent_publish(event[4], EVENT_PRIORITY_MEDIUM, &xVar, sizeof(xVar));
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
	portCHAR evt[20];
	portCHAR i = 0;

	Application_initI2C();

	led2_init();
	light_enable();

	for(;i<10;i++) {
		sprintf((char*)evt, (const char*)"myevent%d", i+1);
		event[i] = uxEvent_createEvent( evt, strlen( (const char *)evt ) );
	}

	if(event[0] != NULL)
		xEvent_subscribe( Application_receiveNewEvent, event[0], NULL );
	if(event[1] != NULL)
		xEvent_subscribe( Application_receiveNewEvent, event[1], NULL );
	if(event[2] != NULL)
		xEvent_subscribe( Application_receiveNewEvent, event[2], NULL );
	if(event[3] != NULL)
		xEvent_subscribe( Application_receiveLight, event[3], NULL );
	if(event[4] != NULL)
		xEvent_subscribe( Application_createdEventCallback, event[4], NULL );

	for(i=5;i<10;i++) {
		if(event[i] != NULL)
			xEvent_subscribe(Application_genericCallback, event[i], NULL);
	}

	event[1] = NULL;

	event[1] = uxEvent_getEventHandler(("myevent2"), strlen("myevent2"));

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

void Application_receiveLight(pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize)
{
	int32_t* iLight = (int32_t*)pvPayload;

	Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Received light: %d", *iLight);
}

void Application_receiveNewEvent( pvEventHandle pvEventHandler, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE XPayloadSize )
{
	portBASE_TYPE xLight;

	if( pvEventHandler == event[0] ) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Systick)");
		xLight = light_read();
		xEvent_publish( event[3], EVENT_PRIORITY_MEDIUM, &xLight, sizeof( xLight ) );
		led2_invert();
	}
	else if( pvEventHandler == event[1] ) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Tick)");
	}
	else if( pvEventHandler == event[2] ) {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] Receiving new event from EventOS (Ethernet)");
	}
	else {
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_ERROR,"[app] Event not expected");
	}
}

void Application_createdEventCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize )
{
	int32_t* iValue = ( int32_t* ) pvPayload;
	int32_t iPosValue = 0;

	if(EventName != NULL)
		iPosValue = (rand())%10;

	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] xxxx Received data from created event: %d; Event name: [%s]", *iValue, EventName );

	if( iPosValue > 4 ) {
		if(event[iPosValue] != NULL) {
			xEvent_publish(event[iPosValue], EVENT_PRIORITY_LOW, (void*)iValue, sizeof(*iValue));
		}
	}
}

void Application_genericCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	int32_t* iValue = ( int32_t* ) pvPayload;

	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL,"[app] GENERIC FUNCTION Received data from created event: %d; Event name: [%s]", *iValue, EventName );
}

portULONG Application_getMsTicks(void)
{
	return msTicks;
}
