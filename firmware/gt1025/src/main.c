/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "stm32f10x_iwdg.h"
#include "i2c.h"
#include "lib/ringbuf.h"
#include "lib/tick.h"
#include "lib/sys_tick.h"
#include "gps/gps.h"
#include "rtc.h"
#include "low_power_mode.h"
#include "sst25.h"
#include "tracker.h"
#include "lib/data_cmp.h"
#include "adc_task.h"
#include "sms_task.h"
#include "call_task.h"
#include "alarm_task.h"
#include "usb_config_task.h"
#include "accelerometer_task.h"
#include "mma7660.h"
#include "database_app.h"
#include "led.h"
#include "tcp_ip_task.h"
#include "buzzer.h"

#include "usbcore.h"
#include "usbhw.h"
#include "hiduser.h"
#include "cdcuser.h"
#include "mscuser.h"

#include "dtmf/dtmf_app.h"

#include "lib/encoding.h"
#include "lib/ampm_sprintf.h"
#include "ampm_gsm_main_task.h"
#include "camera.h"

#define MAIN_Info(...)  //DbgCfgPrintf(__VA_ARGS__)


#define SMS_TIME_CHECK 600



void mCentrelTest(void);
void AppRun(void);
uint8_t GSM_GPRS_Task(uint8_t *arg);
void TIMER2_Init(uint32_t pclk);
void SysSleep(void);
void GPS_TaskControl(void);
void SysInit(void);

uint8_t mainBuf[64] = "Công ty TNHH thương mại thiết bị điện tử AMPM";
uint32_t mainCnt = 0;
uint32_t timer2Cnt;
uint16_t sleepCnt = 0;
uint32_t reportInterVal = 0;
uint32_t smsTimeCheck = 0;
uint32_t camTakePicture = 0;

Timeout_Type tFwDownload;
Timeout_Type tResetApp;
Timeout_Type tAppTimeout;

uint8_t intervalReportWakeFlag = 0;

uint32_t alarmFlag;

enum{
	SYS_SLEEP_MODE,
	SYS_RUN_MODE
}sysStatus = SYS_RUN_MODE;


void HardFault_Handler(void)
{
	NVIC_SystemReset();
	while(1);
}

//recv SMS function
void Ampm_MainSmsRecvCallback(uint8_t *buf)
{
	CMD_CfgParse((char *)buf, 0,0);
	//Ampm_Sms_SendMsg(&smsUser,"0978779222",buf,strlen((char *)buf),SMS_TEXT_MODE,30,3);
}

int main(void)
{
	#ifndef DEBUG_MODE
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_64);
	IWDG_SetReload(3000); // 1/(40khz/prescaler/reload) =  3000*1.6 ms
	IWDG_ReloadCounter();
	IWDG_Enable();
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
	#endif
	SysInit();
	CFG_Load();
	TrackerInit();
	InitTimeout(&tFwDownload,SYSTICK_TIME_SEC(1));
	while(1)
	{
		AppRun();
	}
	return 0;
}


void SysInit(void)
{
	__enable_irq();
	SystemInit();
	TICK_Init(1);
	RCC->APB2ENR = (RCC_APB2ENR_AFIOEN |  /*enable clock for Alternate Function*/
								 RCC_APB2ENR_IOPAEN |   /* enable clock for GPIOA*/
								 RCC_APB2ENR_IOPBEN |	/*enable clock for GPIOB*/
								 RCC_APB2ENR_IOPCEN |/*enable clock for GPIOc*/ 									 
								 RCC_APB2ENR_IOPDEN	
								);   

	RCC->CFGR |= RCC_CFGR_USBPRE; // precale is 1 for usb clock
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;/*enable clock for USB*/

	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;  

	RTC_Init();
	EXTI_Init();

	OUTPUT1_SET_OUTPUT;
	OUTPUT1_PIN_CLR;
	OUTPUT2_SET_OUTPUT;
	OUTPUT2_PIN_CLR;
	PWR_EN_PIN_SET_OUTPUT;
	PWR_EN_PIN_CLR;
	GPS_EN_PIN_SET_OUTPUT;
	GPS_Enable(); //enable gps
	RI_PIN_SET_INPUT;
	IO_ToggleInit();
	TIMER2_Init(SystemCoreClock);
	BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 3);//300ms on/off , 3 times
	IO_Init();
	AccelerometerTaskInit(); 
	ADC_TaskInit();
	SST25_Init();
	DB_FlashMemInit();
	DB_Init();
/*GSM Init*/
 	USART1_Init(SystemCoreClock,__USART1_BAUDRATE);
/*GPS Init*/
 	GPSInit();
 	USART2_Init(SystemCoreClock/2,__USART2_BAUDRATE);
	USART3_Init(SystemCoreClock/2,__USART3_BAUDRATE);
	USART3_PutString("thienhaiblue@ampm.com.vn\r\n");
	USART3_PutString("130 Nguyen Minh Hoang\r\n");
	//camera init
	CAM_Init();
	UsbConfigTaskInit();
	/*USB Init*/
 	USB_Init();        	  // USB Initialization
	//USB Connect enable
	GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);                /* clear port PB13 */
  GPIOB->CRH |=  GPIO_CRH_MODE13_1;                /* PB13 General purpose output open-drain, max speed 50 MHz */
 	GPIOB->BRR  = GPIO_BRR_BR13; 
	USB_Connect(TRUE);    // USB Connect
}

void SysWakeUp(void)
{



}

void SysDeInit(void)
{
	
}


void SysSleep(void)
{

}



//Application
void AppRun(void)
{
	uint8_t arg = 0;
	LoadFirmwareFile();
	SMS_TaskInit();
	watchdogEnable[WTD_MAIN_LOOP] = 1;
	while(1)
	{
		if(CheckTimeout(&tResetApp) == SYSTICK_TIMEOUT)
			InitTimeout(&tResetApp,SYSTICK_TIME_SEC(3600*24));
		if(GSM_GPRS_Task(&arg) || (!ACC_IS_ON))
		{
			SysSleep();
		}
	}
}


uint8_t GSM_GPRS_Task(uint8_t *arg)
{
		uint16_t app1SecCnt = 0;
		uint8_t c;
		Timeout_Type tTrackerTask;
		
		Timeout_Type tApp_100MS;
		AMPM_GSM_Init((uint8_t *)sysCfg.gprsApn,(uint8_t *)sysCfg.gprsUsr,(uint8_t *)sysCfg.gprsPwd,vTcpIpTask,vTcpIpTaskInit);
		vTcpIpTaskInit();
		InitTimeout(&tAppTimeout,SYSTICK_TIME_SEC(120));
		InitTimeout(&tApp_100MS,SYSTICK_TIME_MS(100));
	
		while(CheckTimeout(&tResetApp) != SYSTICK_TIMEOUT)  
		{
				mainCnt++;
				watchdogFeed[WTD_MAIN_LOOP] = 0; 

				
				//GSM task
				AMPM_GSM_MainTask();
				//Tracker Load Log
				GetDataFromLog();
				//USB Task
				//Config and debug task
				UsbConfigTask();
				//Read ADC Value
				ADC_Task(rtcTimeSec);
				//RResetMcuTask
				ResetMcuTask();
				//Camera 
				while(RINGBUF_Get(&USART3_RxRingBuff,&c) == 0)
				{
					CAM_Input(c);
				}
				
				if(sysCfg.cameraInterval)
				{
					if(rtcTimeSec >= camTakePicture + sysCfg.cameraInterval)
					{
						if(CAM_TakeSnapshot(0xff) == 0)
						{
							camTakePicture = rtcTimeSec;
						}
					}
				}
				
				if(cameraSnapShotCnt)
				{
					CAM_TakeSnapshot(0xff);
				}
				CAM_Manage();
				//get IMEI
				if(flagGotIMEI && strstr((char *)sysCfg.imei,(char *)gsmIMEIBuf) == NULL)
				{
					flagGotIMEI = 0;
					strcpy((char *)sysCfg.imei,(char *)gsmIMEIBuf);
					CFG_Save();
				}

				//get CID
				if(flagGotSimCID && strstr((char *)sysCfg.ccid,(char *)gsmSimCIDBuf) == NULL)
				{
					flagGotSimCID = 0;
					strcpy((char *)sysCfg.ccid,(char *)gsmSimCIDBuf);
					CFG_Save();
				}
				
				//Log data process
				if((ACC_IS_ON && (rtcTimeSec - reportInterVal >= sysCfg.runReportInterval))
				|| (!ACC_IS_ON && (rtcTimeSec - reportInterVal >= sysCfg.stopReportInterval))
				)
				{
					reportInterVal = rtcTimeSec;
					TrackerGetNewData();
				}
				//1 sec tasks process
				if(CheckTimeout(&tTrackerTask) == SYSTICK_TIMEOUT)
				{
					app1SecCnt++;
					InitTimeout(&tTrackerTask,SYSTICK_TIME_SEC(1));
					//RING_Process();
					//IO Process fillter
					IO_Control(NULL);
					TrackerTask();
					if(app1SecCnt % 600 == 0)
						CFG_ReLoad();
				}
				// 0.1 sec process 
				if(CheckTimeout(&tApp_100MS) == SYSTICK_TIMEOUT)
				{
					InitTimeout(&tApp_100MS,SYSTICK_TIME_MS(100));
					Accelerometer_Task_MMA7660();
				}
				// ACC change status
				if( ioStatus.din[0].newUpdate	)
				{
					ioStatus.din[0].newUpdate = 0;
					TrackerGetNewData();
					sendStatus = 1;
					statusSentFlag = 0;
				}
				// firmware update process
//				if(FirmwareTask_IsBusy() 
//				|| (VoiceCallTask_GetPhase() != CALL_IDLE)
//				)
//				{
//					InitTimeout(&tAppTimeout,SYSTICK_TIME_SEC(120));
//				}
				//If all tasks is idle then go to sleep
//				if((!ACC_IS_ON) 
//				&& (!trackerSaveLog) 
//				&& ((logSendPt == NULL))
//				&& SmsCheckMessage_IsEmpty() 
//				&& VoiceCallTask_GetPhase() == CALL_IDLE
//				&& (!FirmwareTask_IsBusy())
//				&& gotAddrFlag == 0 
//				&& getAddrFlag == 0
//				&& (!Modem_IsRinging())
//				&& (!smsNewMessageFlag)
//				&& modemCmdMode
//				&& (modemIsBusy == 0)
//				&&  (accelerometerAlarmCnt == 0)
//				&& (CheckTimeout(&tFwDownload) == SYSTICK_TIMEOUT)
//				&& (!CallTask_IsBusy())
//				&& (!AlarmTask_IsBusy())
//				)
//				{
//					if(statusSentFlag || (sendStatus == 0 &&  statusSentFlag == 0))
//					{
//						statusSentFlag = 0;
//						if(GsmTask_GetPhase() == MODEM_SYS_COVERAGE_OK)
//								*arg = 1;
//							return 1;
//					}
//				}		
		}
		return 0;
}


void TIM2_IRQHandler(void)
{

	if(TIM2->SR & 1)
	{
		TIM2->SR = (uint16_t)~0x0001;
		timer2Cnt++;
		if(timer2Cnt % 50 == 0)
			IO_ToggleTask(timer2Cnt);
	}
}

void TIMER2_Init(uint32_t pclk)
{
		
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;                     // enable clock for TIM2
    TIM2->PSC = (uint16_t)(pclk/1000000) - 1;            // set prescaler
    TIM2->ARR = (uint16_t)(1000000*TIMER_PERIOD/1000 - 1);  //1ms          // set auto-reload
    TIM2->CR1 = 0;                                          // reset command register 1
    TIM2->CR2 = 0;                                          // reset command register 2
		TIM2->DIER = 1;                             
		NVIC_SetPriority (TIM2_IRQn,((0x01<<3) | TMR2_PRIORITY));
		NVIC_EnableIRQ(TIM2_IRQn);// enable interrupt
    TIM2->CR1 |= 1;                              // enable timer
}

