
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

volatile portULONG msTicks; // counter for 1ms Applications
pvEventHandle temperatureEventHandler;
pvEventHandle uartEventHandler;
pvEventHandle lightEventHandler;
pvEventHandle logEventHandler;

void SysTick_Handler(void)
{
	msTicks++;

	if(msTicks%500 == 0) {
		led2_invert();
	}
	if(msTicks%10000 == 0) {
		xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, "[app] Publishing new event: Temperature LOW", 44);
		xEvent_publish(temperatureEventHandler, EVENT_PRIORITY_LOW, NULL, 0);
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
	NVIC_EnableIRQ(EINT3_IRQn);
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
		xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, "[app] Button Pressed!", 22);
		xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, "[app] Publishing new event: Temperature HIGH", 45);
		xEvent_publish(temperatureEventHandler, EVENT_PRIORITY_LOW, NULL, 0);
		xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, "[app] Publishing new event: Light", 33);
		xEvent_publish(lightEventHandler, EVENT_PRIORITY_HIGH, NULL, 0);
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

	logEventHandler = uxEvent_createEvent((portCHAR*)"Log", strlen((const char *)"log"));
	temperatureEventHandler = uxEvent_createEvent((portCHAR*)"Temperature", strlen((const char *)"Temperature"));
	lightEventHandler = uxEvent_createEvent((portCHAR*)"Light", strlen((const char *)"Light"));
	uartEventHandler = uxEvent_createEvent((portCHAR*)"Uart", strlen((const char *)"Uart"));

	xEvent_subscribe(Application_logCallback, logEventHandler, NULL);
	xEvent_subscribe(Application_temperatureCallback, temperatureEventHandler, NULL);
	xEvent_subscribe(Application_lightCallback, lightEventHandler, NULL);
	xEvent_subscribe(Application_uartCallback, uartEventHandler, NULL);

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

void Application_logCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	Log_print( LOG_FACILITY_USER_LEVEL_MESSAGES,LOG_SEVERITY_INFORMATIONAL, (const char*)pvPayload);
}

void Application_temperatureCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	uint32_t temp = temp_read();

	if(temp > 0) {
		portCHAR pcMyMsg[30];
		portLONG lMyMsgLen = 0;

		lMyMsgLen = sprintf(pcMyMsg, "[Temp] %lf C", temp/10.0);

		xEvent_publish(uartEventHandler, EVENT_PRIORITY_MEDIUM, pcMyMsg, lMyMsgLen);
		xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, pcMyMsg, lMyMsgLen);
	}
}

void Application_lightCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	int32_t light = light_read();

	portCHAR pcMyMsg[30];
	portLONG lMyMsgLen = 0;

	lMyMsgLen = sprintf(pcMyMsg, "[Light] %d L", light);

	xEvent_publish(uartEventHandler, EVENT_PRIORITY_MEDIUM, pcMyMsg, lMyMsgLen);
	xEvent_publish(logEventHandler, EVENT_PRIORITY_MEDIUM, pcMyMsg, lMyMsgLen);
}

void Application_uartCallback( pvEventHandle EventType, char* EventName, void* pvHandler, void* pvPayload, portBASE_TYPE xPayloadSize ) {
	UART_SendString(LPC_UART3, (uint8_t*)pvPayload);
}

portULONG Application_getMsTicks(void)
{
	return msTicks;
}
