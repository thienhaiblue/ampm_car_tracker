/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include <stdio.h>
#include <string.h>
#include "tracker.h"
#include <math.h>
#include "system_config.h"
#include "gps/gps.h"
#include "db.h"
#include "rtc.h"
#include "lib/sys_time.h"
#include "adc_task.h"
#include "database_app.h"
#include "lib/sys_tick.h"
#include "alarm_task.h"
#include "diskio.h"
#include "mmc_ssp.h"
#include "ff.h"
#include "led.h"
#include "msc_init.h"
#include "ampm_gsm_main_task.h"

#define TRACKER(...)	//DbgCfgPrintf(__VA_ARGS__)

#define TICK_RATE			CONFIG_SYS_CLOCK
#define REMIND_INTERVAL		60*60				// VEHICLE_INFO report interval (sec)
#define OVERSPEED_INTERVAL	30					// sec
#define PARKING_INTERVAL	15

//#define USER_TEST_CODE	1			  /*user for test speed, GT402 will genarate internal speed to use*/
#define USER_SPEED_FOR_CAR_STATUS	 /*if not zezo then will use speed for detect car status is runing or stoping*/
																			/*else will use ACC for detect car status*/
#define USER_STOP_TIME_CONDITION	(15*60) /*sec*/
#define USER_START_TIME_CONDITION	(2*60) /*sec*/
//#define ENABLE_OVER_SPEED_WARNING

uint8_t trackerTaskRun = 0;
uint16_t batteryPercent;
extern uint32_t rtcCnt;
float mileage;
float an1Voltage = 0;
uint8_t trackerSaveLog;
uint32_t trackerTaskCnt = 0;
uint32_t adcChannel8Value = 0;
uint8_t newUpdateGps = 0;
uint32_t gpsGetCnt = 0;
uint8_t gpsFixedFlag = 0;


uint8_t  gpsGotWhenACC_OFF = 0;
uint8_t gpsTryCntWhenACC_OFF = 10;
uint16_t getGpsTimeout = 300;

uint32_t buzzStatus = 0;
uint32_t timeOverSpeedCnt = 0;
uint8_t speedRecordTimeOld = 0;
uint32_t timeEqualCnt = 0;
uint32_t startingTime = 0;

uint8_t sendStatus = 0;
uint8_t statusSentFlag = 0;

extern DATE_TIME fwUpdateDateHour;
extern DATE_TIME fwUpdatedLastDateHour;
extern uint32_t firmwareUpdateAt;
extern uint32_t flagFirmwareUpdateNow;

MSG_STATUS_RECORD	logSend;
MSG_STATUS_RECORD	*logSendPt = NULL;

uint8_t timeFixed = 0;

uint8_t newDayFlag = 0;
uint8_t newDriverSub = 0;

SPEED_RECORD speedRecord;

typedef enum _DriverWarningType{
	RESET_WARNING,
	ON_WARNING,
	OFF_WARNING,
}DriverWarningType;

DriverWarningType flagDrivingTimeWarning = RESET_WARNING;
DriverWarningType flagTotalDrivingTimeWarning = RESET_WARNING;

Timeout_Type tSpeedAlarmTimeout;
extern FATFS sdfs;

//printf report
uint8_t printReport = 0;
uint32_t flagPrint;
DATE_TIME printfReportTime;
uint8_t gotLastRecord = 0;
uint8_t carIsMoving = 0;
void TrackerInit(void)
{
	DB_InitLog(&logRecord);
}

void TRACKER_Pause(void)
{
	trackerTaskRun = 0;
}
void TRACKER_Resume(void)
{
	trackerTaskRun = 1;
}

uint8_t GetDataFromLog(void)
{
	if((ringLog.head != ringLog.tail))
	{
		if(logSendPt == NULL)
		{
			if(logSend.serverSent)
			{
				logSend.serverSent = 0;
				DB_RingLogNext();
			}
			if(DB_LoadNextLog(&logSend) == 0)
			{
				logSendPt = &logSend; 
				return 0;
			}
		}
	}
	return 1;
}

void TrackerTask(void)
{
	float currentSpeed = 0,gpsCurrentSpeed;
	static float speedSamples[5];
	uint32_t i;
	DATE_TIME timeTemp;
	uint8_t *pt;
	uint32_t gpsTimeSec;
//	if(trackerTaskRun)
	{
		trackerTaskCnt++;

		if(((sdfs.fs_type == 0
					|| (disk_memory_size == 0)
					|| (disk_sector_number_max == 0) 
					|| (disk_sector_size == 0)
				)
				&& ((trackerTaskCnt % 120) == 0))
		)
		{
			mscInit();
		}

		//GPS get info
		GPS_GetInfo();
		TIME_FromGps(&timeTemp,&nmeaInfo);
		gpsTimeSec = TIME_GetSec(&timeTemp);
		if(ACC_IS_ON)
		{
			gpsGotWhenACC_OFF = 0;
			getGpsTimeout = 300;
			gpsTryCntWhenACC_OFF = 10;
		}
		else
		{
			if(getGpsTimeout)
			{
				getGpsTimeout--;
			}
		}
		if((nmeaInfo.fix == 1 || nmeaInfo.fix == 2) 
		&& (nmeaInfo.utc.year >= 115) 
		&& (nmeaInfo.HDOP != 99.99)
		&& (nmeaInfo.VDOP != 99.99)
		&& (nmeaInfo.PDOP != 99.99)
		)
		{
			if(nmeaInfo.satinfo.inview)
			{
				if(UpdateRtcTime(gpsTimeSec))
					TIME_FromSec(&sysTime,gpsTimeSec);
			}
		}
		
		if((nmeaInfo.fix == 3) && 
				(nmeaInfo.utc.year >= 113) && 
				(nmeaInfo.sig > 0) && 
				(nmeaInfo.lat != 0) && 
				(nmeaInfo.lon != 0) && 
				(nmeaInfo.lat <= 90 || nmeaInfo.lat >= -90) &&
				(nmeaInfo.lon <= 180  || nmeaInfo.lon >= -180)
			)
		{
			if(gpsGetCnt++ >= 10)
			{
				gpsGetCnt = 20;
				if(!ACC_IS_ON)
				{
					gpsGotWhenACC_OFF = 1;
					getGpsTimeout = 0;
				}
				if(gpsFixedFlag == 0)//send a message to server when GPS is fixed
				{
					gpsFixedFlag = 1;
					trackerSaveLog = 1;
				}
				if(nmeaInfo.speed >= 5/*km*/ && ACC_IS_ON)
				{
					carIsMoving = 1;
				}
				newUpdateGps = 1;
				if(nmeaInfo.speed >= sysCfg.speedLimit && (CheckTimeout(&tSpeedAlarmTimeout) == SYSTICK_TIMEOUT))
				{
					InitTimeout(&tSpeedAlarmTimeout,SYSTICK_TIME_SEC(30));
					alarmFlag |= SPEED_ALARM_FLAG;
					trackerSaveLog = 1;
				}
				lastNmeaInfo = nmeaInfo;
				timeFixed = 1;
				if(UpdateRtcTime(gpsTimeSec)){
					TIME_FromSec(&sysTime,gpsTimeSec);
					TIME_FromSec(&localTime,gpsTimeSec + sysTimeZone);
				}
			}
		}
		else
		{
				gpsGetCnt = 0;
				if(gpsFixedFlag) // send last gps to server
				{
					trackerSaveLog = 1;
				}
		}
			

		//Printf report
		if(flagPrint == 1)
		{
		 flagPrint = 0;
		 printReport = DB_Print(&printfReportTime,1);
		 
		}
		else if(flagPrint == 2)
		{
		 flagPrint = 0;
		 printReport = DB_Print(&printfReportTime,2);
		}
		else
			printfReportTime = localTime;

//Save  gps location to FLASH
		if(trackerTaskCnt % 30 == 0)
		{
			if(lastNmeaInfo.fix == 3 && newUpdateGps)
			{
				newUpdateGps = 0;
				DB_SaveAll();
			}
		}

		// Speed calc
		gpsCurrentSpeed = nmeaInfo.speed;
		if(gpsCurrentSpeed < CONFIG_SPEED_STOP || nmeaInfo.fix < 3 || nmeaInfo.utc.year < 111 || nmeaInfo.sig < 1 || nmeaInfo.lat == 0 || nmeaInfo.lon == 0 || nmeaInfo.HDOP == 0)
		{
			gpsCurrentSpeed = 0;
		}
		speedSamples[sizeof speedSamples / sizeof speedSamples[0] - 1] = gpsCurrentSpeed;
		if(speedSamples[0] == 0 && speedSamples[sizeof speedSamples / sizeof speedSamples[0] - 1] == 0)
		{
			for(i=1;i<sizeof speedSamples / sizeof speedSamples[0] - 1;i++)
				speedSamples[i] = 0;
		}
		for(i=1;i<sizeof speedSamples / sizeof speedSamples[0]; i++)
			speedSamples[i-1] = speedSamples[i];
			
		gpsCurrentSpeed = speedSamples[0];
		
		currentSpeed = gpsCurrentSpeed;
		
		mileage += currentSpeed / 3600;

		// over speed warning
		//if(sysCfg.enableWarning)
		{
			if(currentSpeed > sysCfg.speedLimit)
			{
				buzzStatus = 1;
				if(timeOverSpeedCnt == 2) //0s
				{
					timeOverSpeedCnt++;
					if((timeOverSpeedCnt % 30) == 0) //30sec
					{
						speedRecord.overSpeedCnt++;
					}

					if(sysCfg.useGpsSpeed && nmeaInfo.fix >= 3)
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 3000, BUZZER_TURN_ON,10);	
						#endif
					}
					else
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 3000, BUZZER_TURN_ON,10);	
						#endif
					}
				}
				if(timeOverSpeedCnt == 10) //10s
				{
					if(sysCfg.useGpsSpeed && nmeaInfo.fix >= 3)
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 1500, BUZZER_TURN_ON,10);		
						#endif
					}
					else
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 1500, BUZZER_TURN_ON,10);		
						#endif
					}
				}
				if(timeOverSpeedCnt == 20) //20s
				{
					if(sysCfg.useGpsSpeed && nmeaInfo.fix >= 3)
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 300, BUZZER_TURN_ON,10);		//10 beeps, 300ms interval
						#endif
					}
					else
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,300, 300, BUZZER_TURN_ON,10);		// 10 beeps, 300ms interval
						#endif
					}
				}	
				if((timeOverSpeedCnt % 30)  == 0) //30s
				{
					if(sysCfg.useGpsSpeed && nmeaInfo.fix >= 3)
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,50, 50, BUZZER_TURN_ON,300);		//100 beeps, 50ms interval
						#endif
					}
					else
					{
						#ifdef ENABLE_OVER_SPEED_WARNING
						if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
							BuzzerSetStatus(&buzzer1Ctrl,50, 50, BUZZER_TURN_ON,300);		// 100 beeps, 50ms interval
						#endif
					}
				}	
			}
			else
			{
				if(buzzStatus) 
				{
					buzzStatus = 0;
					#ifdef ENABLE_OVER_SPEED_WARNING
					if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
						BuzzerSetStatus(&buzzer1Ctrl,50, 50, BUZZER_TURN_ON,3);
					#endif
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////
		// LOGGER
		if(localTime.year < 2015)
		{
			timeFixed = 0;
		}
		else //if(timeFixed)
		//if(0)
		{
			TIME_FromSec(&timeTemp,timeSave.value);
			if(timeTemp.mday != localTime.mday 
				|| timeTemp.month != localTime.month 
				|| timeTemp.year != localTime.year
			)
			{
				timeSave.value = TIME_GetSec(&localTime);
				DB_U32Save(&timeSave,TIME_SAVE_ADDR);
				newDayFlag = 1;
			}
			// new session
			if(newDriverSub)
			{
				timeSave.value = TIME_GetSec(&localTime);
				DB_U32Save(&timeSave,TIME_SAVE_ADDR);
				speedRecord.beginTime.hour = 0xff;
				speedRecord.beginTime.min = 0;
				speedRecord.beginTime.sec = 0;
				speedRecord.endTime.hour = localTime.hour;
				speedRecord.endTime.min = localTime.min;
				speedRecord.endTime.sec = localTime.sec;
				speedRecord.parkCnt = 0;
				speedRecord.overSpeedCnt = 0;
				speedRecord.overDrivingTimeCnt = 0;
				//speedRecord.lastOverSpeedOffset = 0;
				speedRecord.totalDrivingTimeCnt = 0;
				speedRecord.openDoorCnt = 0;
				speedRecord.closeDoorCnt = 0;
				speedRecord.offEngineCnt = 0;
				speedRecord.onEngineCnt = 0;
				speedRecord.drivingTime = 0;
				speedRecord.totalDrivingTime = 0;
				newDriverSub = 0;
			}
			// new day has come
			if(newDayFlag)
			{
				newDayFlag = 0;
				speedRecord.beginTime.hour = 0xff;
				speedRecord.beginTime.min = 0;
				speedRecord.beginTime.sec = 0;
				speedRecord.parkCnt = 0;
				speedRecord.openDoorCnt = 0;
				//speedRecord.lastOverSpeedOffset  = 0;
				speedRecord.overSpeedCnt = 0;
				speedRecord.overDrivingTimeCnt = 0;
				//speedRecord.totalDrivingTimeCnt = 0;
				speedRecord.openDoorCnt = 0;
				speedRecord.closeDoorCnt = 0;
				speedRecord.offEngineCnt = 0;
				speedRecord.onEngineCnt = 0;
				
				speedRecord.totalDrivingTime = 0;
				//if car start already
				if(speedRecord.drivingTime && localTime.hour != 0)
				{
					TIME_FromSec(&timeTemp,(TIME_GetSec(&localTime) - speedRecord.drivingTime));
					//If Car start in today
					if(timeTemp.mday == localTime.mday)
					{
						speedRecord.beginTime.hour = timeTemp.hour;
						speedRecord.beginTime.min = timeTemp.min;
						speedRecord.beginTime.sec = timeTemp.sec;
					}
					else //if car start in yesterday
					{
						speedRecord.beginTime.hour = 0;
						speedRecord.beginTime.min = 0;
						speedRecord.beginTime.sec = 0;
					}
				}
			}
			//get last record
//			if(gotLastRecord == 0)
//			{
//				gotLastRecord = 1;
//				GetLastRecord(&speedRecord,&localTime,sysCfg.driverIndex);
//			}

			#ifdef USER_START_TIME_CONDITION
			if((startingTime == 0) && speedRecord.drivingTime)
				startingTime = speedRecord.drivingTime + USER_START_TIME_CONDITION;
			#else
			if((startingTime == 0) && speedRecord.drivingTime)
				startingTime = speedRecord.drivingTime;
			#endif
			speedRecord.mileage = mileage;
			speedRecord.speed = currentSpeed;
			speedRecord.currentTime.hour = localTime.hour;
			speedRecord.currentTime.min = localTime.min;
			speedRecord.currentTime.sec = localTime.sec;
			speedRecord.lat = lastNmeaInfo.lat;
			speedRecord.lon = lastNmeaInfo.lon;

			speedRecord.status = ioStatus.din[0].bitNew | ioStatus.din[1].bitNew << 1 | ioStatus.din[2].bitNew << 2 | ioStatus.din[3].bitNew << 3;
			
			if(currentSpeed >= sysCfg.speedLimit)
			{
				timeOverSpeedCnt++;
				if((timeOverSpeedCnt % 30) == 0) //30sec
				{
					speedRecord.overSpeedCnt++;
					//speedRecord.lastOverSpeedOffset = 0xffffffff;
				}
				#ifdef USER_TEST_CODE
				if(timeOverSpeedCnt == 120)
					testSpeedTemp = 50;//km/h
				#endif
			}
			else
			{
				timeOverSpeedCnt = 0;
			}


			if((speedRecord.currentTime.sec != speedRecordTimeOld)/* && (speedRecord.beginTime.hour != 0xff)*/)
			{
				speedRecordTimeOld = speedRecord.currentTime.sec;
				DB_SaveSpeed(&speedRecord);
			}
			else
			{
				timeEqualCnt++; //no use
			}
			
			if(speedRecord.drivingTime)
			{
				speedRecord.endTime.hour = localTime.hour;
				speedRecord.endTime.min = localTime.min;
				speedRecord.endTime.sec = localTime.sec;
				if(speedRecord.beginTime.hour > 24 
					|| speedRecord.beginTime.hour > speedRecord.endTime.hour
					|| speedRecord.beginTime.hour < 0
				)
				{
					TIME_FromSec(&timeTemp,(TIME_GetSec(&localTime) - speedRecord.drivingTime));
					speedRecord.beginTime.hour = timeTemp.hour;
					speedRecord.beginTime.min = timeTemp.min;
					speedRecord.beginTime.sec = timeTemp.sec;
				}
			}
		}
		
		drivingTimeSave.value =  speedRecord.drivingTime;
		parkingTimeSave.value = speedRecord.parkingTime;
		// driving time calculation
#ifdef USER_SPEED_FOR_CAR_STATUS
		if((currentSpeed >= CONFIG_SPEED_STOP) || (ACC_IS_ON))
#else
		if(ACC_IS_ON) //ACC_ON
#endif
		{
#ifdef USER_START_TIME_CONDITION
			if(startingTime >= USER_START_TIME_CONDITION)
#endif
			{
				speedRecord.drivingTime++;
				speedRecord.totalDrivingTime++;
				speedRecord.parkingTime = 0;
				if(flagTotalDrivingTimeWarning == OFF_WARNING)
					flagTotalDrivingTimeWarning = RESET_WARNING;
				if(flagTotalDrivingTimeWarning == OFF_WARNING)
					flagDrivingTimeWarning = RESET_WARNING;
			}
#ifdef USER_START_TIME_CONDITION
			else
				startingTime++;
#endif
			
			//Driving time warning
			if(((speedRecord.drivingTime % sysCfg.drivingTimeLimit) == 0) && speedRecord.drivingTime)
			{
				if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
					BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 10000); 	// 60 beeps, 300 ms interval
				speedRecord.overDrivingTimeCnt++;
				flagDrivingTimeWarning = ON_WARNING;
			}
			if(((speedRecord.totalDrivingTime % sysCfg.totalDrivingTimeLimit) == 0) && speedRecord.totalDrivingTime)
			{
				if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
					BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 10000); 	// 60 beeps, 300 ms interval
				speedRecord.totalDrivingTimeCnt++;
				flagTotalDrivingTimeWarning = ON_WARNING;
			}
			
			if(flagTotalDrivingTimeWarning == ON_WARNING)
			{
				flagTotalDrivingTimeWarning = RESET_WARNING;
				if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
					BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 18000);//30min
			}
			if(flagDrivingTimeWarning == ON_WARNING)
			{
				flagDrivingTimeWarning = RESET_WARNING;
				if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
					BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 18000);//30mn
			}
		}
		else //ACC_OFF
		{
			speedRecord.parkingTime++;
#ifdef USER_STOP_TIME_CONDITION
			if(speedRecord.parkingTime >= USER_STOP_TIME_CONDITION)
#endif
			{
				speedRecord.drivingTime = 0;
				startingTime = 0;
			}
			//stop warning
			if(flagTotalDrivingTimeWarning != OFF_WARNING)
			{
				flagTotalDrivingTimeWarning = OFF_WARNING;
				BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_OFF, 10); 
			}
			if(flagDrivingTimeWarning != OFF_WARNING)
			{
				flagDrivingTimeWarning = OFF_WARNING;
				BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_OFF, 10); 
			}
		}

		if(trackerSaveLog)
		{
			trackerSaveLog = 0;
			logRecord.time = TIME_GetSec(&sysTime);
			logRecord.drvIndex = 0;
			logRecord.adc1Value = (uint16_t)(ADC1_10_Value*1000);
			logRecord.adc2Value = (uint16_t)(ADC1_11_Value*1000);
			logRecord.gpsLat = lastNmeaInfo.lat;
			logRecord.gpsLon = lastNmeaInfo.lon;
			logRecord.gpsHdop = lastNmeaInfo.HDOP;
			logRecord.cellLat = lastNmeaInfo.lat;
			logRecord.cellLon = lastNmeaInfo.lon;
			logRecord.cellHdop = lastNmeaInfo.HDOP;
			logRecord.gpsSpeed = currentSpeed;
			logRecord.pulseSpeed = 0;
			logRecord.gpsDir = lastNmeaInfo.direction;
			logRecord.drivingTime = 0;
			logRecord.totalDrivingTime = 0;
			logRecord.overSpeedCnt = 0;
			logRecord.parkCnt = 0;
			logRecord.openDoorCnt =  0;
			logRecord.onOffEngineCnt = 0;
			logRecord.parkingTime = 0;
			logRecord.mileage = mileage;
			logRecord.totalPulse = rtcCnt;
			logRecord.speedFreq = 0;
			logRecord.batteryPercent = batteryPercent;//ADC1_11_Value*100/4.2;
			logRecord.gsmSignal = (uint8_t)csqValue;
		
			
			logRecord.IOStatus = 0;
			logRecord.satinuse = lastNmeaInfo.satinfo.inuse;
			logRecord.IOStatus = 0;
			if(GET_AVLIO1_PIN)
				logRecord.IOStatus |= 1<<0; //ACC
			if(GET_AVLIO2_PIN)
				logRecord.IOStatus |= 1<<1;
			if(GET_AVLIO3_PIN)
				logRecord.IOStatus |= 1<<2;
			if(GET_AVLIO4_PIN)
				logRecord.IOStatus |= 1<<3;
			
			if((!ACC_IS_ON) && currentSpeed > 15/*km/h*/) //ACC
				logRecord.IOStatus |= 1<<4; //chuyen dong bat thuong
			//logRecordTemp = logRecord;
			//newestLog = logRecord;
			//newestLog.serverSent = 0;
			DB_SaveLog(&logRecord);
		}
	}
}

void TrackerGetNewData(void)
{
	trackerSaveLog = 1;
}
	

uint8_t TrackerCrc(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}


extern uint8_t tcpIpAnswerCmd;
uint16_t AddTrackerPacket(MSG_STATUS_RECORD *logRecordSend,char *data,uint16_t *len)
{
//	uint32_t u32Temp0,u32Temp1;
//	int32_t i32Temp0,i32Temp1;
	char *buff = data;
	char tmpBuf[32];
//	double fTemp;
	DATE_TIME time;
	buff[0] = 0;
	data[0] = 0;
	TIME_FromSec(&time,logRecordSend->time); 
	//device ID
	sprintf(tmpBuf, "{\"dev\":\"%s\",",sysCfg.imei);
	strcat(buff, tmpBuf);
	//YYMMDDHHMMSS
	sprintf(tmpBuf, "\"tim\":%02d%02d%02d%02d%02d%02d,",time.year%100, time.month, time.mday,time.hour, time.min, time.sec);
	strcat(buff, tmpBuf);
	//Operating time
	sprintf(tmpBuf, "\"old\":%d,",rtcCnt);
	strcat(buff, tmpBuf);
	//GPS lat
	if(logRecordSend->gpsLat){
		sprintf(tmpBuf, "\"lat\":%0.6f,",logRecordSend->gpsLat);
		strcat(buff, tmpBuf);
	}
	//GPS lon
	if(logRecordSend->gpsLon){
		sprintf(tmpBuf, "\"lon\":%0.6f,",logRecordSend->gpsLon);
		strcat(buff, tmpBuf);
	}
	//GPS speed
	if(logRecordSend->gpsSpeed){
		sprintf(tmpBuf, "\"vgp\":%0.2f,",logRecordSend->gpsSpeed);
		strcat(buff, tmpBuf);
	}
	//GPS Dir
	if(logRecordSend->gpsDir){
		sprintf(tmpBuf, "\"dir\":%0.3f,",logRecordSend->gpsDir);
		strcat(buff, tmpBuf);
	}
	//GPS HDop
	if(logRecordSend->gpsHdop){
		sprintf(tmpBuf, "\"hdp\":%0.2f,",logRecordSend->gpsHdop);
		strcat(buff, tmpBuf);
	}
	//GPS satinuse
	if(logRecordSend->satinuse){
		sprintf(tmpBuf,"\"sat\":%d,",logRecordSend->satinuse);
		strcat(buff,tmpBuf);
	}
	//mileage
	if(logRecordSend->mileage){
		sprintf(tmpBuf,"\"mil\":%.02f,",logRecordSend->mileage);
		strcat(buff,tmpBuf);
	}
//	batteryPercent
	if(logRecordSend->batteryPercent){
		sprintf(tmpBuf, "\"bat\":%d,",logRecordSend->batteryPercent);
		strcat(buff, tmpBuf);
	}
	//gsmSignal
	if(logRecordSend->gsmSignal){
		sprintf(tmpBuf, "\"sig\":%d,",logRecordSend->gsmSignal);
		strcat(buff, tmpBuf);
	}
	//ACC is ON
	if(logRecordSend->IOStatus & (1 << 0)) {
		sprintf(tmpBuf, "\"acc\":1,");
		strcat(buff, tmpBuf);
	}
	//Microwave is enable
	if(logRecordSend->IOStatus & (1 << 4)) {//microwave enable
		sprintf(tmpBuf, "\"miw\":1,");
		strcat(buff, tmpBuf);
	}
	//GPS is loss
	if(logRecordSend->gpsHdop > 10){ //no  GPS
		sprintf(tmpBuf, "\"gps\":1,");
		strcat(buff, tmpBuf);
	}
	//over speed
	if(logRecordSend->IOStatus & (1 << 6)){ 
		sprintf(tmpBuf, "\"spo\":1,");//over speed
		strcat(buff, tmpBuf);
	}
	//SOS is active now
	if(logRecordSend->IOStatus & (1 << 7)){ //SOS
		sprintf(tmpBuf, "\"sos\":1,");
		strcat(buff, tmpBuf);
	}
	//adc value 1
	if(logRecordSend->adc1Value){
		sprintf(tmpBuf, "\"an1\":%0.2f,",((float)logRecordSend->adc1Value)/1000);
		strcat(buff, tmpBuf);
	}
	//adc value 2
	if(logRecordSend->adc1Value){
		sprintf(tmpBuf, "\"an2\":%0.2f,",((float)logRecordSend->adc2Value)/1000);
		strcat(buff, tmpBuf);
	}
	//firmware
	sprintf(tmpBuf, "\"fwr\":\"%s\",",FIRMWARE_VERSION);
	strcat(buff, tmpBuf);
	//operating
	//firmware
	sprintf(tmpBuf, "\"opt\":0");
	strcat(buff, tmpBuf);
	
	if(logRecordSend->cameraID){
		sprintf(tmpBuf, ",\"cam\":%d",logRecordSend->cameraID);
		strcat(buff, tmpBuf);
	}
	if(tcpIpAnswerCmd)
	{
		sprintf(tmpBuf, ",\"cmd\":%d",tcpIpAnswerCmd);
		strcat(buff, tmpBuf);
		tcpIpAnswerCmd = 0;
	}
	
	strcat(buff, "}\r\n");
	*len = strlen(buff);

	return 0;
}

void PrintTrackerInfo(MSG_STATUS_RECORD *log)
{

}



