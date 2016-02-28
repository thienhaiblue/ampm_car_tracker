/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "buzzer.h"
#include "stm32f10x.h"
#include "hw_config.h"
#include <stdint.h>
#include "hw_config.h"


#define INIT	1
#define DEINIT	0

uint32_t pwmStatus = DEINIT;
//--------------------------------------------
void InitPWM(void)
{
	if(pwmStatus == DEINIT)
	{
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;                     // enable clock for TIM2
    TIM4->PSC = 47;            // set prescaler
    TIM4->ARR = 178;  //Us          // set auto-reload
    TIM4->CR1 = 0;                                          // reset command register 1
    TIM4->CR2 = 0;                                          // reset command register 2
		TIM4->DIER = 1;                             
		NVIC_SetPriority (TIM4_IRQn,((0x01<<3) | BUZZER_PROORITY));
		NVIC_EnableIRQ(TIM4_IRQn);// enable interrupt
    TIM4->CR1 |= 1;   // enable timer
		BUZZER_SET_OUTPUT;
		BUZZER_PIN_CLR;
	} 
	pwmStatus = INIT;
}

//--------------------------------------------
void DeInitPWM(void)
{
	if(pwmStatus == INIT)
	{
		TIM4->CR1 = 0;                                          // reset command register 1
    TIM4->CR2 = 0;                                          // reset command register 2
		NVIC_DisableIRQ(TIM4_IRQn);// enable interrupt
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;                     // enable clock for TIM4     
		BUZZER_PIN_CLR;
		BUZZER_SET_INPUT;
	}
	pwmStatus = DEINIT;
}


uint32_t timer3Cnt = 0;
void TIM4_IRQHandler(void)
{
	static uint32_t tog = 0;
	timer3Cnt++;
	if(TIM4->SR & 1)
	{
		TIM4->SR = (uint16_t)~0x0001;
			tog ^= 1;
		if(tog)
			BUZZER_PIN_CLR;
		else
			BUZZER_PIN_SET;
	}
}




