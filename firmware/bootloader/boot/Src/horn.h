#ifndef __HORN_H__
#define __HORN_H__
#include <stdint.h>
#include "stm32f10x.h"
#include "dbg_cfg_app.h"

#define BUZZER_PORT	GPIOA
#define BUZZER_PIN	GPIO_BSRR_BS15

#define BUZZER_SET_OUTPUT		BUZZER_PORT->CRH	&= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15); BUZZER_PORT->CRH	|= (GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1)
#define BUZZER_SET_INPUT		BUZZER_PORT->CRH	&= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15); BUZZER_PORT->CRH	|= (GPIO_CRH_CNF15_0)
#define BUZZER_PIN_SET					BUZZER_PORT->BSRR = BUZZER_PIN
#define BUZZER_PIN_CLR					BUZZER_PORT->BRR = BUZZER_PIN
#define BUZZER_ON_TIME_DFG	(200 / TIMER_PERIOD) /*200ms */
#define BUZZER_OFF_TIME_DFG	(200 / TIMER_PERIOD) /*200ms */
#define BUZZER_TURN_ON	1
#define BUZZER_TURN_OFF 0
#define BUZZER_TURN_ON_TIMES	3

#define BuzzerDeInit()	BUZZER_SET_INPUT

typedef struct _BUZZ
{
    uint32_t buzzerOnTime;
		uint32_t buzzerOffTime;
		uint32_t buzzerCounter;
		uint8_t buzzerStatus;
		uint32_t buzzerTimes;
} buzzer_type;

extern buzzer_type	buzzerCtr;											
void BuzzerInit(void);
void BuzzerSetStatus(uint32_t tOn,uint32_t tOff,uint32_t status,uint32_t times);
void BuzzerCtl(void);

#endif


