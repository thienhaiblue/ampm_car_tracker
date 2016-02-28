

#include "DTMF.h"                 // global definitions
#include "stdio.h"
#include "stdint.h"
#include "lib/ringbuf.h"
#include "adc_task.h"
#include "hw_config.h"
#include "dtmf_app.h"
#include "led.h"

RINGBUF DTMF_ringBuff;
uint8_t DTMF_buf[32];

/*  DTMF Digit encoding */
static char DTMFchar[16] = {
  '1', '2', '3', 'A', 
  '4', '5', '6', 'B', 
  '7', '8', '9', 'C', 
  '*', '0', '#', 'D', 
};

struct DTMF dail1 = { 0, DTMF_DATA_DEEP, 0 };  // DTMF info of one input

uint8_t DEMF_Enable = 0;



void TPM3_DeInit(void)
{
	RCC->APB1ENR &=~ RCC_APB1ENR_TIM3EN;                     // disnable clock for TIM3
	NVIC_DisableIRQ(TIM3_IRQn);
}

//uint32_t toggle = 0;
void TIM3_IRQHandler(void)
{
	if(TIM3->SR & 1)
	{
//		toggle ^= 1;
//		if(toggle)
//			LED_PIN_SET;
//		else
//			LED_PIN_CLR;
		TIM3->SR = (uint16_t)~0x0001;
		dail1.AInput[dail1.AIindex & (DTMFsz-1)] = ADC1ConvertedValue[1];
		dail1.AIindex++;
		
	}
}

void TPM3_Init(uint32_t pclk)
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;                     // enable clock for TIM3
    TIM3->PSC = (uint16_t)(pclk/1000000) - 1;            // set prescaler
    TIM3->ARR = (uint16_t)(1000000/8000 - 1);  //8kHz         // set auto-reload
    TIM3->CR1 = 0;                                          // reset command register 1
    TIM3->CR2 = 0;                                          // reset command register 2
		TIM3->DIER = 1;                             
		NVIC_SetPriority (TIM3_IRQn,((0x01<<3) | 1));
		NVIC_EnableIRQ(TIM3_IRQn);// enable interrupt
    TIM3->CR1 |= 1;                              // enable timer
}

void DTMF_DeInit(void)
{
	DEMF_Enable = 0;
	TPM3_DeInit();
}

void DTMF_Init(void)
{
	RINGBUF_Init(&DTMF_ringBuff,DTMF_buf,sizeof(DTMF_buf));
	TPM3_Init(SystemCoreClock);
	DEMF_Enable = 1;
}


char DTMF_Task(void)
{
	uint8_t c;
	if(DEMF_Enable)
	{
		if (dail1.AIindex >= dail1.AIcheck)  
		{
			DTMF_Detect (&dail1);
		}
		if (dail1.new)  
		{
			c = DTMFchar[dail1.digit & 0x0F];
			RINGBUF_Put(&DTMF_ringBuff,c);
			//RINGBUF_Get(&DTMF_ringBuff,&c);
			dail1.new = 0;                              // digit taken
			return c;		
		}
	}
	return 0;
}
