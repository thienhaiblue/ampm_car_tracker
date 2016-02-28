/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "led.h"
#include "buzzer.h"

IO_TOGGLE_TYPE	led1Ctrl;	
IO_TOGGLE_TYPE buzzer1Ctrl;
IO_TOGGLE_TYPE dout1Ctrl;
IO_TOGGLE_TYPE dout2Ctrl;

void IO_ToggleInit(void)			
{
	LED_SET_OUTPUT;
	BUZZER_SET_OUTPUT;
	OUTPUT1_SET_OUTPUT;
	OUTPUT2_SET_OUTPUT;
	IO_ToggleSetStatus(&led1Ctrl,IO_STATUS_ON_TIME_DFG,IO_STATUS_OFF_TIME_DFG,IO_TOGGLE_ENABLE,0xffffffff);
	IO_ToggleSetStatus(&buzzer1Ctrl,IO_STATUS_ON_TIME_DFG,IO_STATUS_OFF_TIME_DFG,IO_TOGGLE_DISABLE,0xffffffff);
	IO_ToggleSetStatus(&dout1Ctrl,IO_STATUS_ON_TIME_DFG,IO_STATUS_OFF_TIME_DFG,IO_TOGGLE_DISABLE,0xffffffff);
	IO_ToggleSetStatus(&dout2Ctrl,IO_STATUS_ON_TIME_DFG,IO_STATUS_OFF_TIME_DFG,IO_TOGGLE_DISABLE,0xffffffff);
}

void IO_ToggleSetStatus(IO_TOGGLE_TYPE *ioCtrl,uint32_t onTime,uint32_t offTime,uint32_t enable,uint32_t times)		
{
	if(ioCtrl->timesSave != times 
	|| ioCtrl->times == 0
	|| ioCtrl->enable != enable
	|| ioCtrl->onTime != onTime
	|| ioCtrl->offTime != offTime
	)
	{
		ioCtrl->onTime = onTime;
		ioCtrl->offTime = offTime;
		ioCtrl->counter = 0;
		ioCtrl->enable = enable;
		ioCtrl->times = times;
		ioCtrl->timesSave = times;
	}
}													


void IO_ToggleTask(uint32_t time)
{
		static uint64_t ioTimeNew = 0,ioTimeOld = 0;
		static uint32_t timeOld = 0;
		ioTimeNew += time - timeOld;
		IO_ToggleProcess(&led1Ctrl,ioTimeNew - ioTimeOld);
		IO_ToggleProcess(&buzzer1Ctrl,ioTimeNew - ioTimeOld);
		IO_ToggleProcess(&dout1Ctrl,ioTimeNew - ioTimeOld);
		IO_ToggleProcess(&dout2Ctrl,ioTimeNew - ioTimeOld);
		if(led1Ctrl.status == IO_STATUS_ON)
		{
			LED_PIN_SET;
		}
		else 
		{
			LED_PIN_CLR;
		}

		if(buzzer1Ctrl.status == IO_STATUS_ON)
		{
			InitPWM();
		}
		else 
		{
			DeInitPWM();
		}

		if(dout1Ctrl.status == IO_STATUS_ON)
		{
			//OUTPUT2_PIN_SET;
		}
		else 
		{
			//OUTPUT2_PIN_CLR;
		}

		if(dout2Ctrl.status == IO_STATUS_ON)
		{
			OUTPUT1_PIN_SET;
		}
		else 
		{
			OUTPUT1_PIN_CLR;
		}

		ioTimeOld = ioTimeNew; 
		timeOld = time;
}	
											
void IO_ToggleProcess(IO_TOGGLE_TYPE *ioCtrl, uint32_t times)	
{
	if(ioCtrl->enable == IO_TOGGLE_ENABLE) 
	{
			if(ioCtrl->counter > times)
				ioCtrl->counter -= times;
			else ioCtrl->counter = 0;
				
			if(ioCtrl->counter == 0) 
			{
				if(ioCtrl->times) 
				{
					ioCtrl->times--;
					ioCtrl->counter = ioCtrl->offTime + ioCtrl->onTime;
					ioCtrl->status = IO_STATUS_ON;
				}
				else
				{
					ioCtrl->enable = IO_TOGGLE_DISABLE;
					ioCtrl->status = IO_STATUS_OFF;
				}
			}
			
			if(ioCtrl->counter <= ioCtrl->offTime) 
				ioCtrl->status = IO_STATUS_OFF;
	}
	else
	{
		ioCtrl->status = IO_STATUS_OFF;
	}
}

