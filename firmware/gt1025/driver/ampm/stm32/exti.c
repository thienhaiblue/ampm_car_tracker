/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/

#include "exti.h"
#include "low_power_mode.h"
#include "system_config.h"
#include "rtc.h"
#include "gps/gps.h"
#include "accelerometer_task.h"

extern uint32_t alarmFlag;
extern uint8_t sendStatus;
extern uint8_t statusSentFlag;
extern uint32_t RingCallProcess(uint16_t cnt,uint8_t c); 

void EXTI_Init(void)
{
//		//RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                     // enable clock for Alternate Function
		/*Congig use for EXTI0*/
		AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;	//Disable JTAG funtion, keep SWJ funtion for debug(only user when PB3,PB4,PB5,PA15 user for another funtion)
		AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0;   // clear used pin
		AFIO->EXTICR[1] &= ~AFIO_EXTICR1_EXTI1;   // clear used pin
		AFIO->EXTICR[2] &= ~AFIO_EXTICR1_EXTI2;   // clear used pin
		AFIO->EXTICR[3] &= ~AFIO_EXTICR1_EXTI3;    // clear used pin
			/*Use PB0 is ACCEL*/ 
		ACCEL_PIN_SET_INPUT; //set ACCELEROMETER is input with internal resistor pull-up
		EXTI->PR = EXTI_Line2;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PA;           // set pin to use
		EXTI->IMR       &= ~EXTI_Line2;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line2;             // mask event
    EXTI->RTSR      &= ~EXTI_Line2;            // clear rising edge
		EXTI->IMR       |= EXTI_Line2;             // mask interrupt
		EXTI->EMR       |= EXTI_Line2;             // mask event
    EXTI->FTSR      |= EXTI_Line2;            // set falling edge
		/* preemption = ACCL_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(EXTI2_IRQn, ((0x01<<3)| ACCL_PRIORITY));
		NVIC_EnableIRQ(EXTI2_IRQn);
	
		/*Use PB1 is MODEMRI_PRIORITY*/
		EXTI->PR = EXTI_Line8;
    AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI8_PA;           // set pin to use
		EXTI->IMR       &= ~EXTI_Line8;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line8;             // mask event
    EXTI->RTSR      &= ~EXTI_Line8;            // clear rising edge
		EXTI->IMR       |= EXTI_Line8;             // mask interrupt
		EXTI->EMR       |= EXTI_Line8;             // mask event
		EXTI->RTSR      &= ~EXTI_Line8;            // clear rising edge
    EXTI->FTSR      |= EXTI_Line8;            // set falling edge
		/* preemption = MODEMRI_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(EXTI9_5_IRQn, ((0x01<<3)| MODEMRI_PRIORITY));
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		
		
			/*Use PB3 is AVLIO1_PRIORITY*/
		EXTI->PR = EXTI_Line0;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA;           // set pin to use
		EXTI->IMR       &= ~EXTI_Line0;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line0;             // mask event
    EXTI->RTSR      &= ~EXTI_Line0;            // clear rising edge
		EXTI->IMR       |= EXTI_Line0;             // mask interrupt
		EXTI->EMR       |= EXTI_Line0;             // mask event

    EXTI->FTSR      |= EXTI_Line0;            // set falling edge
		/* preemption = AVLIO1_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(EXTI0_IRQn, ((0x01<<3)| AVLIO1_PRIORITY));
		NVIC_EnableIRQ(EXTI0_IRQn);
//		
		/*Use PB4 is AVLIO2_PRIORITY*/
		EXTI->PR = EXTI_Line4;
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;           // set pin to use
		EXTI->IMR       &= ~EXTI_Line4;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line4;             // mask event
    EXTI->RTSR      &= ~EXTI_Line4;            // clear rising edge
		EXTI->IMR       |= EXTI_Line4;             // mask interrupt
		EXTI->EMR       |= EXTI_Line4;             // mask event

    EXTI->FTSR      |= EXTI_Line4;            // set falling edge
		/* preemption = ACCL_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(EXTI4_IRQn, ((0x01<<3)| AVLIO2_PRIORITY));
		NVIC_EnableIRQ(EXTI4_IRQn);
////		
				/*Use PB5 is AVLIO3_PRIORITY*/
		EXTI->PR = EXTI_Line5;
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;           // set pin to use
		EXTI->IMR       &= ~EXTI_Line5;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line5;             // mask event
    EXTI->RTSR      &= ~EXTI_Line5;            // clear rising edge
		EXTI->IMR       |= EXTI_Line5;             // mask interrupt
		EXTI->EMR       |= EXTI_Line5;             // mask event

    EXTI->FTSR      |= EXTI_Line5;            // set falling edge
		/* preemption = ACCL_PRIORITY, sub-priority = 1 */
		NVIC_SetPriority(EXTI9_5_IRQn, ((0x01<<3)| AVLIO2_PRIORITY));
		NVIC_EnableIRQ(EXTI9_5_IRQn);

		// 		/*Congig use for EXTI4*/
// 		/*Use RTC Wakeup*/
		EXTI->PR = EXTI_Line17;
		EXTI->IMR       &= ~EXTI_Line17;             // mask interrupt
    EXTI->EMR       &= ~EXTI_Line17;             // mask event
		EXTI->IMR       |= EXTI_Line17;             // mask interrupt
		EXTI->EMR       |= EXTI_Line17;             // mask event
    EXTI->RTSR      |= EXTI_Line17;            // clear rising edge

}


void EXTI0_IRQHandler(void) 
{
	EXTI->PR = EXTI_Line0;
	sendStatus = 1;
	statusSentFlag = 0;
}

void EXTI1_IRQHandler(void)
{
		EXTI->PR = EXTI_Line1;
		
}

void EXTI2_IRQHandler(void) 
{
	EXTI->PR = EXTI_Line2; 
	alarmFlag |= ACCLEROMETER_FLAG;
	alarmFlag |= RFID_ACCELEROMETER_FLAG;
	if(accelerometerAlarmCnt == 0)
		accelerometerAlarmCnt = 10;
}

void EXTI3_IRQHandler(void)
{
		EXTI->PR = EXTI_Line3;
		
}

void EXTI4_IRQHandler(void)
{
		EXTI->PR = EXTI_Line4;
}

extern void Ampm_RingingProcess(void);
void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR & EXTI_Line5)
	{
		EXTI->PR = EXTI_Line5;
		
	}
	if(EXTI->PR & EXTI_Line8)
	{
		EXTI->PR = EXTI_Line8;
		Ampm_SetRinging();
	}
	
}




void USBWakeUp_IRQHandler(void)
{
	EXTI->PR = EXTI_Line18;
}



