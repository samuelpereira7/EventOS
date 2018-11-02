
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
#include "temp.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"


/*********************************************************
    private operations.
*********************************************************/
#define PIN_DEFINITION_BUTTON_PORT			0
#define PIN_DEFINITION_BUTTON_BIT_VALUE		4
#define PIN_DEFINITION_BUTTON_DIR			0

void Application_init( void );
void Application_initI2C( void );
void Application_initSysTick( void );
portULONG Application_getMsTicks( void );
void Application_logCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_uartCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_lightCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );
void Application_temperatureCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize );

volatile portULONG msTicks=0; // counter for 1ms Applications
volatile portULONG ulPressedTime;
pvEventHandle temperatureEventHandler = NULL;
pvEventHandle uartEventHandler = NULL;
pvEventHandle lightEventHandler = NULL;
pvEventHandle logEventHandler = NULL;

void SysTick_Handler(void)
{
	msTicks++;

	if(msTicks%500 == 0) {
		led2_invert();
	}
	if(msTicks%10000 == 0)
	{
		Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[app] Publishing new event: Light LOW" );
		uint32_t light = light_read();
		xEvent_publish(lightEventHandler, EVENT_PRIORITY_LOW, &light, sizeof(light));
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

void Application_initButton() {
	GPIO_SetDir(PIN_DEFINITION_BUTTON_PORT, 1<<PIN_DEFINITION_BUTTON_BIT_VALUE, PIN_DEFINITION_BUTTON_DIR);
	GPIO_IntCmd(PIN_DEFINITION_BUTTON_PORT, 1<<PIN_DEFINITION_BUTTON_BIT_VALUE, 1);
	GPIO_IntCmd(PIN_DEFINITION_BUTTON_PORT, 1<<PIN_DEFINITION_BUTTON_BIT_VALUE, 0);
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_SetPriority(EINT3_IRQn, 1);
}

void Application_initUart() {
	PINSEL_CFG_Type PinCfg;
	UART_CFG_Type uartCfg;

	/* Initialize UART3 pin connect */
	PinCfg.Funcnum = 2;//2;
	PinCfg.Pinnum = 0;//0;
	PinCfg.Portnum = 0;//0;
	PINSEL_ConfigPin(&PinCfg);

	PinCfg.Funcnum = 2;//2;
	PinCfg.Pinnum = 1;//0;
	PinCfg.Portnum = 0;//0;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate 	= 115200;
	uartCfg.Databits	= UART_DATABIT_8;
	uartCfg.Parity 		= UART_PARITY_NONE;
	uartCfg.Stopbits 	= UART_STOPBIT_1;

	UART_Init(LPC_UART3, &uartCfg);
	UART_TxCmd(LPC_UART3, ENABLE);
}

void EINT3_IRQHandler(void)
{

	if(GPIO_GetIntStatus(PIN_DEFINITION_BUTTON_PORT, PIN_DEFINITION_BUTTON_BIT_VALUE, 1)) {
		ulPressedTime = Application_getMsTicks();
	}
	else if(GPIO_GetIntStatus(PIN_DEFINITION_BUTTON_PORT, PIN_DEFINITION_BUTTON_BIT_VALUE, 0)) {
		portULONG ulReleaseTime = Application_getMsTicks();
		portULONG ulTimeStamp = ulReleaseTime - ulPressedTime;

		if( ulTimeStamp >= 5000 && ulTimeStamp < 10000 ) {
			if( temperatureEventHandler != NULL ) {
				Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[SYS] Deleting Temperature event" );
				temperatureEventHandler = uxEvent_deleteEvent( temperatureEventHandler );
			}
		}
		else if( ulTimeStamp >= 10000 ) {
			if( temperatureEventHandler == NULL ) {
				Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[SYS] Creating Temperature event" );
				temperatureEventHandler = uxEvent_createEvent((portCHAR*)"Temperature", strlen((const char *)"Temperature"));
				xEvent_subscribe(Application_temperatureCallback, temperatureEventHandler, NULL);
			}
		}
		else {
			uint32_t light = light_read();
			uint32_t temp  = temp_read();
			Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[app] Button Pressed!" );
			Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[app] Publishing new event: Temperature" );
			xEvent_publish( temperatureEventHandler, EVENT_PRIORITY_HIGH, &temp, sizeof(temp) );
			Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, "[app] Publishing new event: Light HIGH" );
			xEvent_publish( lightEventHandler, EVENT_PRIORITY_HIGH, &light, sizeof(light) );
		}

	}
	GPIO_ClearInt(PIN_DEFINITION_BUTTON_PORT, 1<<PIN_DEFINITION_BUTTON_BIT_VALUE);

}

void Application_new( void )
{
	Application_initI2C();
	Application_initUart();
	Application_initButton();

	led2_init();
	light_enable();
	temp_init(Application_getMsTicks);

	lightEventHandler = uxEvent_createEvent((portCHAR*)"Light", strlen((const char *)"Light"));

	xEvent_subscribe(Application_lightCallback, lightEventHandler, NULL);

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
	else
	{
		NVIC_SetPriority(SysTick_IRQn, 0);
	}
	return;
}

void Application_delete( void )
{

}

void Application_temperatureCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	portCHAR pcMyMsg[30];
	portLONG lMyMsgLen = 0;
	uint32_t* temp = (uint32_t*)pvPayload;

	lMyMsgLen = sprintf(pcMyMsg, "[Temp] %lf C", (*temp)/10.0);

	UART_SendString(LPC_UART3, (uint8_t*)pcMyMsg);
	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, pcMyMsg);
}

void Application_lightCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	portCHAR pcMyMsg[30];
	portLONG lMyMsgLen = 0;
	uint32_t* light = ((uint32_t*)pvPayload);

	lMyMsgLen = sprintf(pcMyMsg, "[Light] %d L", *light);

	UART_SendString(LPC_UART3, pcMyMsg);
	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, pcMyMsg);
}

portULONG Application_getMsTicks(void)
{
	return msTicks;
}
