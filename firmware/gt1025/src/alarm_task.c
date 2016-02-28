
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "system_config.h"
#include "lib/sparser.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "lib/list.h"
#include "lib/encoding.h"
#include "gps/gps.h"

#include "sst25.h"
#include "adc_task.h"
#include "sms_task.h"
#include "call_task.h"
#include "exti.h"
#include "hw_config.h"
#include "rtc.h"
#include "app_config_task.h"
#include "accelerometer_task.h"

#include "led.h"

#include "ampm_gsm_main_task.h"

#define ALARM_Info(...) // DbgCfgPrintf(__VA_ARGS__)

#define ACC_OFF_TIME 3
#define GSENSOR_ALARM_TIME_INTERVAL 120
#define ACC_OFF_TIME_PROTECT	300

const uint8_t smsgSensorAlarmWarning[] = "Xe Chuyển động";

extern uint8_t carIsMoving;
extern uint8_t sosCarLockFlag;
SMS_LIST_TYPE smsUser1;
SMS_LIST_TYPE smsUser2;
SMS_LIST_TYPE smsUser3;


uint8_t alarmSmsBuf[160];

uint32_t alarmFlag = 0;


uint8_t gSensorAlarmTimes = 3;
uint8_t inProtectTime = 0;

uint8_t alarmCarLockFlag = 0;

uint8_t gSensorSmsSending = 0;

uint32_t time1Protect = 0;
uint32_t time2Protect = 0;
DATE_TIME timeProtect;

uint8_t tCarAccident = 0;

uint8_t tStopCheckGsensorTimeout = 0;

uint8_t tKeyAlarmPhaseWarningTimeout = 0;

uint16_t  tProtectOffTimeout = 0;
uint8_t keyAlarmTimes = 0;
uint8_t accOffTime = ACC_OFF_TIME;
uint8_t tCarLockTimeout = 0;
void StopAlarmKeyProtect(void);

void AlarmTaskInit(void)
{
	memset((uint8_t *)&smsUser1,NULL,sizeof(SMS_LIST_TYPE));
	memset((uint8_t *)&smsUser2,NULL,sizeof(SMS_LIST_TYPE));
	memset((uint8_t *)&smsUser3,NULL,sizeof(SMS_LIST_TYPE));
}

enum{
	ALARM_KEY_START = 0,
	ALARM_KEY_RUNING,
	ALARM_KEY_IDLE,
	ALARM_KEY_STOP
}keyPhaseAlarm = ALARM_KEY_STOP;

uint8_t AlarmTask_IsBusy(void)
{
	if(keyPhaseAlarm != ALARM_KEY_STOP
	||	buzzer1Ctrl.enable
	||	dout1Ctrl.enable
	||	dout2Ctrl.enable)
	{
		return 1;
	}
	return 0;
}

void AlarmTask(uint32_t rtc)
{
	uint8_t pdu2uniBuf[160];
	uint8_t smsLen;
	DATE_TIME timeTemp;
	static uint32_t time;
	if(time != rtc)
	{
		time = rtc;
		TIME_FromSec(&timeTemp,rtc + 25200);
		if(timeProtect.mday != timeTemp.mday
		|| timeProtect.month != timeTemp.month
		|| timeProtect.year != timeTemp.year
		)
		{
			timeProtect = timeTemp;
			timeProtect.hour = sysCfg.time1_hour;
			timeProtect.min = sysCfg.time1_min;
			time1Protect = TIME_GetSec(&timeProtect); 
			timeProtect.hour = sysCfg.time2_hour;
			timeProtect.min = sysCfg.time2_min;
			time2Protect = TIME_GetSec(&timeProtect);
		}

		if(rtc + 25200 >= time1Protect 
		&& rtc + 25200 <= time2Protect 
		&& (tProtectOffTimeout == 0)
		&& sysCfg.featureSet & FEATURE_CAR_PROTECT_ENABLE
		)
		{
			inProtectTime = 1;
		}
		else
		{
			inProtectTime = 0;
			if(keyPhaseAlarm != ALARM_KEY_STOP)
				StopAlarmKeyProtect();
		}
		
		if(ACC_IS_ON)
		{
			if(inProtectTime)
			{
				if(keyPhaseAlarm == ALARM_KEY_STOP)
				{
					keyPhaseAlarm = ALARM_KEY_START;
				}
			}
			else
			{
				tProtectOffTimeout = ACC_OFF_TIME_PROTECT;
				alarmCarLockFlag = 0;
			}
			accOffTime = ACC_OFF_TIME; 
		}
		else 
		{
			if(keyPhaseAlarm != ALARM_KEY_STOP)
			{
				StopAlarmKeyProtect();
				tProtectOffTimeout = 0;
			}
			if(accOffTime)
				accOffTime--;
		}
		
		if(tCarLockTimeout) tCarLockTimeout--;
		if(tProtectOffTimeout)	tProtectOffTimeout--;
		if(tKeyAlarmPhaseWarningTimeout)	tKeyAlarmPhaseWarningTimeout--;
		if(tStopCheckGsensorTimeout)	tStopCheckGsensorTimeout--;
		if(tCarAccident)	tCarAccident--;
	}
	
	switch(keyPhaseAlarm)
	{
		case ALARM_KEY_START:
			tKeyAlarmPhaseWarningTimeout = 0;
			keyAlarmTimes = 3;
			alarmCarLockFlag  = 1;
			tCarLockTimeout = 180;
			keyPhaseAlarm = ALARM_KEY_RUNING;
			smsLen = sprintf((char *)pdu2uniBuf,"Mở xe khi đang trong chế độ bảo vệ\nGọi điện để tắt chế độ bảo vệ");
			smsLen = utf8s_to_ucs2s((int16_t *)alarmSmsBuf,pdu2uniBuf);
			big2litel_endian((uint16_t *)alarmSmsBuf,unilen((uint16_t *)alarmSmsBuf));
			smsLen *= 2;
			Ampm_Sms_SendMsg(&smsUser1,sysCfg.userPhone1,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
			if(Ampm_ComparePhoneNumber((char *)sysCfg.userPhone2,(char *)sysCfg.userPhone1) == DIFFERENT_NUMBER
			&&	Ampm_ComparePhoneNumber((char *)sysCfg.userPhone2,(char *)sysCfg.userPhone3) == DIFFERENT_NUMBER
			)
			{
				Ampm_Sms_SendMsg(&smsUser2,sysCfg.userPhone2,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
			}
			if(Ampm_ComparePhoneNumber((char *)sysCfg.userPhone3,(char *)sysCfg.userPhone2) == DIFFERENT_NUMBER
			&&	Ampm_ComparePhoneNumber((char *)sysCfg.userPhone3,(char *)sysCfg.userPhone1) == DIFFERENT_NUMBER
			)
			{
				Ampm_Sms_SendMsg(&smsUser3,sysCfg.userPhone3,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
			}
		break;
		case ALARM_KEY_RUNING:
			if(tKeyAlarmPhaseWarningTimeout == 0)
			{
				if(keyAlarmTimes-- == 0)
					keyPhaseAlarm = ALARM_KEY_STOP;
				else
				{
					BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,60);
					IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,60);
					tKeyAlarmPhaseWarningTimeout = 60;
					keyPhaseAlarm = ALARM_KEY_IDLE;
				}
			}
		break;
		case ALARM_KEY_IDLE:
			if(tKeyAlarmPhaseWarningTimeout == 0)
			{
				BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
				IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
				tKeyAlarmPhaseWarningTimeout = 10;
				keyPhaseAlarm = ALARM_KEY_RUNING;
			}
		break;
		case ALARM_KEY_STOP:
			
		break;
	}

	if(alarmCarLockFlag || sosCarLockFlag)
	{
		if(tCarLockTimeout == 0)
		{
			BuzzerSetStatus(&dout2Ctrl,500,0,IO_TOGGLE_DISABLE,300);
		}
		else
		{
			BuzzerSetStatus(&dout2Ctrl,500,0,IO_TOGGLE_ENABLE,300);
		}
	}
	else
	{
		BuzzerSetStatus(&dout2Ctrl,500,0,IO_TOGGLE_DISABLE,10000);
	}
	
	if(buzzer1Ctrl.enable
		|| dout1Ctrl.enable
		|| dout2Ctrl.enable)
	{
		tStopCheckGsensorTimeout = 3;
	}

	if(alarmFlag & CAR_BE_TOWED_AWAY_FLAG || carIsMoving)
	{
		carIsMoving = 0;
		if(gSensorSmsSending)
		{
			alarmFlag &= ~CAR_BE_TOWED_AWAY_FLAG;
		}
		tCarLockTimeout = 180;
	}

	if(accOffTime)
	{
			alarmFlag &= ~ CAR_BE_TOWED_AWAY_FLAG;
			gSensorAlarmTimes = 2;
	}
	else
	{
	/*Car accident (G-Sensor more than 2G) */
		if(smsUser1.flag != SMS_NEW_MSG
		&& smsUser2.flag != SMS_NEW_MSG
		&& smsUser3.flag != SMS_NEW_MSG
		)
		{
			gSensorSmsSending = 0;
			if(alarmFlag & CAR_BE_TOWED_AWAY_FLAG)
			{
				if(tCarAccident == 0
				&& sysCfg.featureSet & FEATURE_CAR_MOVING_ALARM_ENABLE
				&& (tStopCheckGsensorTimeout == 0)
				)
				{
					BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,2);
					IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,2);
					
					getAddrFlag = 1;
					askAddrCnt = 0;
					tCarAccident = GSENSOR_ALARM_TIME_INTERVAL;
					
					ALARM_Info("ALARM:Car accident!\r\n");  
					alarmFlag &= ~CAR_BE_TOWED_AWAY_FLAG;
					gSensorAlarmTimes--;
					smsLen =sprintf((char *)pdu2uniBuf,"%s!\nhttp://maps.google.com/maps?q=%.6f,%.6f",smsgSensorAlarmWarning,lastGpsInfo.lat,lastGpsInfo.lon);
					smsLen = utf8s_to_ucs2s((int16_t *)alarmSmsBuf,pdu2uniBuf);
					big2litel_endian((uint16_t *)alarmSmsBuf,unilen((uint16_t *)alarmSmsBuf));
					smsLen *= 2;
					Ampm_Sms_SendMsg(&smsUser1,sysCfg.userPhone1,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
					if(Ampm_ComparePhoneNumber((char *)sysCfg.userPhone2,(char *)sysCfg.userPhone1) == DIFFERENT_NUMBER
					&&	Ampm_ComparePhoneNumber((char *)sysCfg.userPhone2,(char *)sysCfg.userPhone3) == DIFFERENT_NUMBER
					)
					{
						Ampm_Sms_SendMsg(&smsUser2,sysCfg.userPhone2,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
					}
					if(Ampm_ComparePhoneNumber((char *)sysCfg.userPhone3,(char *)sysCfg.userPhone2) == DIFFERENT_NUMBER
					&&	Ampm_ComparePhoneNumber((char *)sysCfg.userPhone3,(char *)sysCfg.userPhone1) == DIFFERENT_NUMBER
					)
					{
						Ampm_Sms_SendMsg(&smsUser3,sysCfg.userPhone3,(uint8_t *)alarmSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
					}
					addrSendToUser1Flag = 1;
					addrSendToUser2Flag = 1;
					addrSendToUser3Flag = 1;
					gSensorSmsSending = 1;
				}
				else
					alarmFlag &= ~CAR_BE_TOWED_AWAY_FLAG;
			}
		}
	}
}

uint8_t StopAlarmKey_IsBusy(void)
{
	if(keyPhaseAlarm != ALARM_KEY_STOP)
		return 1;
	return 0;
}

void StopAlarmKeyProtect(void)
{
	keyPhaseAlarm = ALARM_KEY_STOP;
	BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
	IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
	alarmCarLockFlag = 0;
}

