/**
* \file
*         tick driver
* \author
*         Nguyen Van Hai <hainv@ivx.vn>
*/

#include <stdio.h>
#include <string.h>
#include "tick.h"


//#define __NVIC_PRIO_BITS 4

#define GetCurrentTime  TICK_Get
#define MAIN_LOOP_TIME_MAX 120000 //120sec
#define TIMER0_LOOP_TIME_MAX 60000 //60sec
#define TIMER1_LOOP_TIME_MAX 60000 //60sec

volatile uint32_t tickCounter32 = 0;
volatile uint64_t tickCounter64 = 0;

volatile uint32_t tickTimeSec = 0;
volatile uint8_t tickTimeSecFlagUpdate = 0;

uint32_t watchdogEnable[WATCHDOG_NUM] = {0};
volatile uint32_t watchdogCnt[WATCHDOG_NUM] = {0};
volatile uint32_t watchdogFeed[WATCHDOG_NUM] = {0};
const uint32_t watchdogCntValueMax[WATCHDOG_NUM] = {MAIN_LOOP_TIME_MAX,TIMER0_LOOP_TIME_MAX,TIMER1_LOOP_TIME_MAX};


extern void AT_CommandCtl(void);

void TICK_Init(uint32_t timeMs) 
{
  SysTick->LOAD  = ((SystemCoreClock/1000)*timeMs - 1);      // set reload register
	/* preemption = TICK_PRIORITY, sub-priority = 1 */
	NVIC_SetPriority(SysTick_IRQn, ((0x01<<3)| TICK_PRIORITY));
  SysTick->VAL   =  0;                                          // clear  the counter
  SysTick->CTRL = 0x07;                       
} // end

void TICK_DeInit(void)
{
	SysTick->CTRL = 0x00; 
}


//interrupt
void SysTick_Handler(void)
{
	uint32_t temp;
	tickCounter32++;
	tickCounter64++;
	if((tickCounter64 % 1000) == 0)	tickTimeSec++;
	//SYS_ClearWatchdog();
	temp = tickCounter32 % WATCHDOG_NUM;
	if(watchdogEnable[temp])
	{
		//check watchdog value for funtion
		watchdogCnt[temp] += WATCHDOG_NUM;
		if(watchdogCnt[temp] >= watchdogCntValueMax[temp]) 
		{
			NVIC_SystemReset();
		}
		if(watchdogFeed[temp] == 0)
		{
			watchdogCnt[temp] = 0;
			watchdogFeed[temp] = 1;
		}
	}
	else	watchdogCnt[temp] = 0;

	AT_CommandCtl();
}



uint32_t TICK_Get(void)
{
	return tickCounter32;
}

uint32_t TICK_Get64(void)
{
   return tickCounter64;
}

void DelayMs(uint32_t ms)
{
   	uint32_t currentTicks = TICK_Get();
		while(TICK_Get()- currentTicks < ms);
}



void InitTimeout(Timeout_Type *t,uint32_t timeout)
{
	t->start_time = GetCurrentTime();
	t->timeout = timeout;
	t->crc = t->start_time + t->timeout;
}

uint32_t CheckTimeout(Timeout_Type *t)
{
	uint32_t u32temp,u32temp1;
	u32temp = t->start_time + t->timeout;
	if(u32temp != t->crc) NVIC_SystemReset();
	u32temp = GetCurrentTime();
	t->crc = t->start_time + t->timeout;
	if(u32temp >= t->start_time)
		u32temp1 = u32temp - t->start_time;
	else
		u32temp1 = (0xFFFFFFFF - t->start_time) + u32temp;
	if(u32temp1 >= t->timeout) return 0;
	return (t->timeout - u32temp1);
}

void TimerDelayms(uint32_t time)
{
	uint32_t start_time,current_time;
	start_time = GetCurrentTime();
	while(1)
	{
		current_time = GetCurrentTime();
		if(current_time >= start_time)
		{
			if((current_time - start_time) >= time) break;
		}
		else
		{
			if(((0xFFFFFFFF - start_time) + current_time) >= time) break;
		}
	}
}

void InitFindData(COMPARE_TYPE *cmpData,uint8_t *data)
{
	cmpData->buff = data;
	cmpData->index = 0;
	cmpData->len = strlen((char *)data);
}
uint8_t FindData(COMPARE_TYPE *cmpData,uint8_t c)
{
	if(cmpData->buff[cmpData->index] == c) cmpData->index++;
	else cmpData->index = 0;
	
	if(cmpData->index >= cmpData->len) return 0;
	
	return 0xff;
}

