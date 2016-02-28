#ifndef __LED__H__
#define __LED__H__


#include <stdint.h>
#include "hw_config.h"





typedef struct _ledDEF
{
    uint32_t onTime;
		uint32_t offTime;
		int32_t counter;
		uint8_t status;
		uint8_t enable;
		uint32_t times;
		uint32_t timesSave;
} IO_TOGGLE_TYPE;


extern IO_TOGGLE_TYPE	led1Ctrl,buzzer1Ctrl;	
extern IO_TOGGLE_TYPE dout1Ctrl;
extern IO_TOGGLE_TYPE dout2Ctrl;

#define TIMER_PERIOD	1	//ms

#define IO_STATUS_ON	1
#define IO_STATUS_OFF 0

#define IO_STATUS_ON_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define IO_STATUS_OFF_TIME_DFG	(500 / TIMER_PERIOD) /*1s */

#define IO_TOGGLE_ENABLE	1
#define IO_TOGGLE_DISABLE 0

//Led define
#define LED_PORT	GPIOB
#define LED_PIN	GPIO_BSRR_BS14
#define LED_SET_OUTPUT	LED_PORT->CRH	&= ~(GPIO_CRH_MODE14 | GPIO_CRH_CNF14); LED_PORT->CRH	|= (GPIO_CRH_MODE14_0)
#define LED_SET_INPUT		LED_PORT->CRH	&= ~(GPIO_CRH_MODE14 | GPIO_CRH_CNF14); LED_PORT->CRH	|= (GPIO_CRH_CNF14_0)
#define LED_PIN_CLR					LED_PORT->BSRR = LED_PIN
#define LED_PIN_SET					LED_PORT->BRR = LED_PIN

#define 	LED_ON	IO_STATUS_ON
#define 	LED_OFF IO_STATUS_OFF
#define  	LED_TURN_ON IO_TOGGLE_ENABLE
#define  	LED_TURN_OFF  IO_TOGGLE_DISABLE

#define LedSetStatus IO_ToggleSetStatus


#define 	BUZZER_ON			IO_STATUS_ON
#define 	BUZZER_OFF 		IO_STATUS_OFF
#define  	BUZZER_TURN_ON 		IO_TOGGLE_ENABLE
#define  	BUZZER_TURN_OFF  	IO_TOGGLE_DISABLE


#define BuzzerSetStatus IO_ToggleSetStatus


void IO_ToggleProcess(IO_TOGGLE_TYPE *ledCtr,uint32_t times);
void IO_ToggleSetStatus(IO_TOGGLE_TYPE *ledCtr,uint32_t onTime,uint32_t offTime,uint32_t enable,uint32_t times);
void IO_ToggleInit(void);
void IO_ToggleTask(uint32_t time);

#endif

