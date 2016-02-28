#ifndef __TICK_H__
#define __TICK_H__
#include "stm32f10x.h"
#include <stdint.h>
#include "core_cm3.h"
#include "system_stm32f10x.h"
#include "hw_config.h"

#define WTD_MAIN_LOOP	0
#define WTD_TIMER0_LOOP	1
#define WTD_TIMER1_LOOP	2

#define TICK_SECOND 1000

#define TIME_MS(x)	x
#define TIME_SEC(x)	(x*1000)

#define TIMEOUT		0
#define TIMEIN		1

#define WATCHDOG_NUM	3 /*MAIN + Timer0 + Timer1*/

typedef struct {
	uint32_t start_time; 		
	uint32_t timeout;
	uint32_t crc; 
} Timeout_Type;

typedef struct {
	uint8_t *buff;
	uint8_t index;
	uint8_t len;
}COMPARE_TYPE;

extern volatile uint32_t tickTimeSec;
extern volatile uint8_t tickTimeSecFlagUpdate;
extern volatile uint32_t watchdogFeed[WATCHDOG_NUM];
extern uint32_t watchdogEnable[WATCHDOG_NUM];
void TICK_Init(uint32_t timeMs);
void TICK_DeInit(void);
uint32_t TICK_Get(void);
uint32_t TICK_Get64(void);
void DelayMs(uint32_t ms);
void InitTimeout(Timeout_Type *t,uint32_t timeout);
uint32_t CheckTimeout(Timeout_Type *t);
void SYS_ClearWatchdog(void);
void InitFindData(COMPARE_TYPE *cmpData,uint8_t *data);
uint8_t FindData(COMPARE_TYPE *cmpData,uint8_t c);

#endif

