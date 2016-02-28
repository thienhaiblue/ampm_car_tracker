#include "horn.h"
#include "hw_config.h"


uint32_t timer3Cnt;
uint32_t hornOnFlag = 0;
void BuzzerSetStatus(uint32_t tOn,uint32_t tOff,uint32_t status,uint32_t times)
{
		buzzerCtr.buzzerOnTime = tOn;
		buzzerCtr.buzzerOffTime = tOff;
		buzzerCtr.buzzerTimes = times + 1;
		buzzerCtr.buzzerStatus = status;
}


void BuzzerCtl(void)
{
	if((buzzerCtr.buzzerStatus == BUZZER_TURN_ON) && buzzerCtr.buzzerTimes) 
		{
				if(buzzerCtr.buzzerCounter)	
				{
					buzzerCtr.buzzerCounter--;
				}
				if(buzzerCtr.buzzerCounter > (buzzerCtr.buzzerOffTime + buzzerCtr.buzzerOnTime))
				{
					buzzerCtr.buzzerCounter = 0;
				}
				if(buzzerCtr.buzzerCounter == 0) 
				{
					buzzerCtr.buzzerCounter = buzzerCtr.buzzerOffTime + buzzerCtr.buzzerOnTime;
					hornOnFlag = 1;
					if(buzzerCtr.buzzerTimes)
					{
						buzzerCtr.buzzerTimes--;
					}
					if(buzzerCtr.buzzerTimes == 0)	
					{
						buzzerCtr.buzzerStatus = BUZZER_TURN_OFF;
						hornOnFlag = 0;
					}
				}
				if(buzzerCtr.buzzerCounter == buzzerCtr.buzzerOffTime)
				{				
					hornOnFlag = 0;
				}
		}
		else 
		{
			hornOnFlag = 0;
		}
}

void TIMER3_Init(uint32_t pclk)
{
		//RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;                     // enable clock for TIM2
    TIM3->PSC = (uint16_t)(pclk/1000000) - 1;            // set prescaler
    TIM3->ARR = 20;  //2kHz          // set auto-reload
    TIM3->CR1 = 0;                                          // reset command register 1
    TIM3->CR2 = 0;                                          // reset command register 2
		TIM3->DIER = 1;                             
		NVIC_SetPriority (TIM3_IRQn,((0x01<<3) | 0));
		NVIC_EnableIRQ(TIM3_IRQn);// enable interrupt
    TIM3->CR1 |= 1;                              // enable timer
}


void BuzzerInit(void)
{
	BUZZER_SET_OUTPUT;
	BuzzerSetStatus(BUZZER_ON_TIME_DFG,BUZZER_OFF_TIME_DFG,BUZZER_TURN_OFF,BUZZER_TURN_ON_TIMES);
	TIMER3_Init(SystemCoreClock);
}

uint32_t run_wao = 0;
void TIM3_IRQHandler(void)
{
	TIM3->SR = (uint16_t)~0x0001;
	if(hornOnFlag)
	{
		timer3Cnt++;
		if((timer3Cnt % 500) == 0)
		{
			run_wao++;
		}
		if(run_wao > 30)	
		{
			run_wao = 20;
		}
		if((timer3Cnt % run_wao) > (run_wao/2))		BUZZER_PIN_SET;
		else								BUZZER_PIN_CLR;
	}
	else
	{
		BUZZER_PIN_CLR;
	}
}

// uint32_t run_wao = 0;
// void TIM3_IRQHandler(void)
// {
// 	TIM3->SR = (uint16_t)~0x0001;
// 	timer3Cnt++;
// 	if((timer3Cnt % 500) == 0)
// 	{
// 		run_wao++;
// 	}
// 	if(run_wao > 50)	
// 	{
// 		run_wao = 40;
// 	}
// 	if((timer3Cnt % run_wao) > (run_wao/2))		BUZZER_PIN_SET;
// 	else								BUZZER_PIN_CLR;
// }


