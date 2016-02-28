#include "stm32f10x.h"
#include "gsm_hwr.h"
#include "uart1.h"
#include "hw_config.h"
#include "lib/sys_tick.h"
#include "led.h"
uint8_t MODEM_PowerOn(void)
{
	LedSetStatus(&led1Ctrl,50,50,LED_TURN_ON,0xffffffff);	
	POWER_PIN_SET_OUTPUT;
	RESET_PIN_SET_OUTPUT;
	DTR_PIN_SET_OUTPUT;
	DTR_PIN_CLR;
	RTS_PIN_SET_OUTPUT;
	RTS_PIN_CLR;

	RESET_PIN_SET; 
	
	POWER_PIN_SET;    // Turn on GSM
	SysTick_DelayMs(5);             // Delay 5ms
	POWER_PIN_CLR;      // Turn off GSM
	SysTick_DelayMs(200);            // Delay 200ms
	POWER_PIN_SET;     // Turn on GSM

	// ---------- Turn on GPRS module ----------
	RESET_PIN_CLR;    // Reset GSM
	SysTick_DelayMs(200);           // Delay 200ms
	RESET_PIN_SET;    // Start GSM module (Release reset)  
	return 0;
}

void MODEM_Sleep(void)
{
	RTS_PIN_SET;
}
void MODEM_Wakeup(void)
{
	RTS_PIN_CLR;
}
void MODEM_UartInit(uint32_t baudrate)
{
	USART1_Init(SystemCoreClock,baudrate);
}


