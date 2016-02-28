#include "tracker.h"
#include <math.h>
#include "ringbuf.h"
#include "system_config.h"
#include "gps.h"
#include "db.h"
#include "packet_queue.h"
#include "adc.h"
#include "at_command_parser.h"
#include "modem.h"

#define GET_POWER_PIN	0
#define GET_SOS_PIN	0
#define GET_DOOR_INPUT_PIN	0

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
#define ENABLE_OVER_SPEED_WARNING

#define lastNmeaInfo lastGpsInfo

uint32_t timer0Cnt = 0;

uint32_t remindInterval = 0;
uint16_t rpSampleCount = 0;


uint32_t gpsTimeSec = 0;
uint32_t gsmTimeSec = 0;


DATE_TIME timeTemp;

volatile float currentSpeed;
float gpsBigSpeed = 0;
RINGBUF trackerRb;
uint8_t trackerBuf[512];			// big buffer to hold unsent packets
MSG_STATUS_RECORD	logRecord,logRecordSend;
uint32_t startingTime = 0;


typedef enum _DriverWarningType{
	RESET_WARNING,
	ON_WARNING,
	OFF_WARNING,
}DriverWarningType;

DriverWarningType flagDrivingTimeWarning = RESET_WARNING;
DriverWarningType flagTotalDrivingTimeWarning = RESET_WARNING;

uint32_t timeOverSpeedCnt = 0;

float an1Voltage = 0;
float an2Voltage = 0;


uint32_t serverSendDataFlag = 0;

uint32_t pulseCnt = 0;



TrackerStatusType  trackerStatus;

char tmpBuf[32];
char pkGpsData[256];
uint16_t packetNo = 0;
uint8_t speedRecordTimeOld = 0;
uint32_t timeEqualCnt = 0;
uint32_t buzzStatus = 0;
float testSpeedTemp;
void TrackerInit(void)
{
	
	RINGBUF_Init(&trackerRb, trackerBuf, sizeof trackerBuf);
	
	trackerStatus.newDriverSub = 1;
}

void TrackerTask(void) // Timer0 interrupt subroutine 
{
	float currentSpeed = 0,gpsCurrentSpeed,pulseCurrentSpeed,fHDOPTemp;
	static float speedSamples[5];
	uint32_t i;
	uint8_t flagSendStatus = 0;
	timer0Cnt++;
	//watchdogFeed
	watchdogFeed[WTD_TIMER0_LOOP] = 0;
	//GPS get info
	GPS_GetInfo();
	TIME_FromGps(&timeTemp,&nmeaInfo);
	gpsTimeSec = TIME_GetSec(&timeTemp);
	TIME_FromGsm(&timeTemp,&cellLocate.time);
	gsmTimeSec = TIME_GetSec(&timeTemp);
	if((nmeaInfo.fix > 1) || (nmeaInfo.sig >= 1))
	{
		tickTimeSecFlagUpdate = 1;
	}
	else if(tickTimeSec < gsmTimeSec)
	{
		tickTimeSecFlagUpdate = 2;	
	}
	TIME_FromSec(&sysTime,tickTimeSec);

	// Speed
	gpsCurrentSpeed = nmeaInfo.speed;
	if(gpsCurrentSpeed < CONFIG_SPEED_STOP || nmeaInfo.fix < 3 || nmeaInfo.utc.year < 111 || nmeaInfo.sig < 1 || nmeaInfo.lat == 0 || nmeaInfo.lon == 0 || nmeaInfo.HDOP == 0)
		gpsCurrentSpeed = 0;
		
	speedSamples[sizeof speedSamples / sizeof speedSamples[0] - 1] = gpsCurrentSpeed;
	
	if(speedSamples[0] == 0 && speedSamples[sizeof speedSamples / sizeof speedSamples[0] - 1] == 0)
	{
		for(i=1;i<sizeof speedSamples / sizeof speedSamples[0] - 1;i++)
			speedSamples[i] = 0;
	}
	
	for(i=1;i<sizeof speedSamples / sizeof speedSamples[0]; i++)
		speedSamples[i-1] = speedSamples[i];
		
	gpsCurrentSpeed = speedSamples[0];
	
	if(sysCfg.useGpsSpeed)
		currentSpeed = gpsCurrentSpeed;
	else
		currentSpeed = pulseCurrentSpeed;
	
		////////////////////////////////////////////////////////////
		// ASCII protocol
		///////////////////////////////////////////////////////////
	
	rpSampleCount++;
	if(GET_ACC_PIN) //ACC OFF
		i = sysCfg.stopReportInterval;
	else
		i = sysCfg.runReportInterval;

		if((tcpSocketStatus[0] == SOCKET_CLOSE) || (RINGBUF_GetFill(&trackerRb) >= (sizeof(trackerBuf) - 128)))
		{
			if( serverSendDataFlag || flagSendStatus|| ((rpSampleCount % i) == 0))
			{
				serverSendDataFlag = 0;
				flagSendStatus = 0;
				logRecord.currentTime = sysTime;
				logRecord.drvIndex = sysCfg.driverIndex;
				logRecord.adc1Value = 0;
				logRecord.adc2Value = 0;
				
				if((nmeaInfo.sig >= 1) && (nmeaInfo.mlon != 0) && (nmeaInfo.mlat != 0))
				{
						logRecord.ew = nmeaInfo.ew;
						logRecord.ns = nmeaInfo.ns;
						logRecord.lat = nmeaInfo.mlat;
						logRecord.lon = nmeaInfo.mlon;
						logRecord.hdop = nmeaInfo.HDOP;
				}
				else if((tcpSocketStatus[0] == SOCKET_OPEN) && (cellLocate.lon != 0) && (cellLocate.lat != 0))
				{
					logRecord.ew = cellLocate.ew;
					logRecord.ns = cellLocate.ns;
					logRecord.lat = cellLocate.lat;
					logRecord.lon = cellLocate.lon;
					logRecord.hdop = cellLocate.uncertainty;
				}
				else if((lastGpsInfo.mlon != 0) && (lastGpsInfo.mlat != 0))
				{
					logRecord.ew = lastGpsInfo.ew;
					logRecord.ns = lastGpsInfo.ns;
					logRecord.lat = lastGpsInfo.mlat;
					logRecord.lon = lastGpsInfo.mlon;
					logRecord.hdop = lastGpsInfo.HDOP;	
				}
				else
				{
					logRecord.ew = 'E';
					logRecord.ns = 'N';
					logRecord.lat = 0;
					logRecord.lon = 0;
					logRecord.hdop = 99.99;
			  }
				
				logRecord.speed = currentSpeed;
				logRecord.dir = lastNmeaInfo.direction;
				logRecord.drivingTime = 0;
				logRecord.totalDrivingTime = 0;
				logRecord.overSpeedCnt = 0;
				logRecord.parkCnt =0;
				logRecord.openDoorCnt = 0;
				logRecord.onOffEngineCnt = 0;
				logRecord.parkingTime = 0;
				logRecord.IOStatus = 0;
				#ifdef CUSTOMER_ADA
					logRecord.speedFreq = (uint16_t)(speedFreq*100);
				#endif
				
				// 11
				if(0)
					logRecord.IOStatus |= 1<<11;
				// 10
				if(0)
					logRecord.IOStatus |= 1<<10;
				//9
				if((GET_ACC_PIN) && currentSpeed > 15/*km/h*/) //ACC
					logRecord.IOStatus |= 1<<9;
				//8
				if(nmeaInfo.sig < 1)
					logRecord.IOStatus |= 1<<8;
				//7
				if(trackerStatus.overSpeed)
					logRecord.IOStatus |= 1<<7;
				//6
				//5
				if(!GET_DOOR_INPUT_PIN)
					logRecord.IOStatus |= 1<<5;
				//4
				if(!GET_POWER_PIN)
					logRecord.IOStatus |= 1<<4;
				//3
				if(GET_SOS_PIN)
					logRecord.IOStatus |= 1<<3;
				//2+1
				if(!GET_ACC_PIN)
					logRecord.IOStatus |= 3<<1;
				//0
				#ifdef TAMBI
				if(speedRecord.drivingTime)
					logRecord.IOStatus |= 1;
				#endif
				
				DB_SaveLog(&logRecord);
			}
		}
		else 
		{
			if(!(serverSendDataFlag || flagSendStatus || ((rpSampleCount % i) == 0)))
			{
				while(RINGBUF_GetFill(&trackerRb) <= (sizeof(trackerBuf)/2))
				{
					//if(0)
					if(DB_LoadLog(&logRecordSend) == 0)
					{
						pkGpsData[0] = 0;
						strcat(pkGpsData, "$POS,");
						strcat(pkGpsData, (char *)sysCfg.id);
						sprintf(tmpBuf, ",%03d",logRecordSend.drvIndex);
						strcat(pkGpsData, tmpBuf);
						//strcat(pkGpsData, (char *)sysCfg.licenseNo);
						// 14 bytes timestamp
						sprintf(tmpBuf, ",%02d%02d%02d.00", logRecordSend.currentTime.hour, logRecordSend.currentTime.min, logRecordSend.currentTime.sec);
						strcat(pkGpsData, tmpBuf);
						
						sprintf(tmpBuf, ",%02d%02d%02d",logRecordSend.currentTime.mday, logRecordSend.currentTime.month, logRecordSend.currentTime.year%100);
						strcat(pkGpsData, tmpBuf);
						if((logRecordSend.lon != 0) && (logRecordSend.lat != 0))
						{
							sprintf(tmpBuf, ",%.06f,%c,%.06f,%c", logRecordSend.lon, logRecordSend.ew, logRecordSend.lat, logRecordSend.ns);
						}
						else
						{
							sprintf(tmpBuf, ",0.000000,E,0.000000,N");	
						}
						strcat(pkGpsData, tmpBuf);
						
						sprintf(tmpBuf, ",%.01f,%.01f", logRecordSend.speed,logRecordSend.dir);
						strcat(pkGpsData, tmpBuf);
						// 11
						if(logRecordSend.IOStatus & (1 << 11))
							strcat(pkGpsData, ",1");
						else
							strcat(pkGpsData, ",0");
						// 10
						if(logRecordSend.IOStatus & (1 << 10))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//9
						if(logRecordSend.IOStatus & (1 << 9))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//8
						if(logRecordSend.IOStatus & (1 << 8))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//7
						if(logRecordSend.IOStatus & (1 << 7))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//6
						strcat(pkGpsData, "0");
						//5
						#if defined CUSTOMER_OK_9108TW || defined CUSTOMER_ADA
						if(logRecordSend.IOStatus & (1 << 5))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						#else
						if(logRecordSend.IOStatus & (1 << 5))
							strcat(pkGpsData, "0");
						else
							strcat(pkGpsData, "1");
						#endif
						//4
						if(logRecordSend.IOStatus & (1 << 4))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//3
						if(logRecordSend.IOStatus & (1 << 3))
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						//2+1
						if(logRecordSend.IOStatus & (3 << 1))
							strcat(pkGpsData, "00");
						else
							strcat(pkGpsData, "11");
						//0
						#ifdef TAMBI
						if(logRecordSend.IOStatus & 1)
							strcat(pkGpsData, "1");
						else
							strcat(pkGpsData, "0");
						#endif
						//so lan mo cua
						sprintf(tmpBuf,",%d",logRecordSend.openDoorCnt);
						strcat(pkGpsData,tmpBuf);
						//thoi gian lai xe lien tuc va tong thoi gian lai xe trong ngay
						sprintf(tmpBuf, ",%02d%02d",logRecordSend.drivingTime/3600,(logRecordSend.drivingTime%3600)/60);
						strcat(pkGpsData, tmpBuf);
						sprintf(tmpBuf, ",%02d%02d",logRecordSend.totalDrivingTime/3600,(logRecordSend.totalDrivingTime%3600)/60);
						strcat(pkGpsData, tmpBuf);
						
						sprintf(tmpBuf, ",%.02f", logRecordSend.hdop);
						strcat(pkGpsData, tmpBuf);
						
						// 8 bytes adc
						an1Voltage = (float)logRecordSend.adc1Value * 3.3 / 4096;
						an2Voltage = (float)logRecordSend.adc2Value * 3.3 / 4096;
						an1Voltage = an1Voltage;
						an2Voltage = an2Voltage;
						sprintf(tmpBuf, ",%.02f,%.02f", an1Voltage, an2Voltage);
						strcat(pkGpsData, tmpBuf);
						//strcat(pkGpsData, ",2.2,2.0");
						// 8 bytes mileage
						sprintf(tmpBuf, ",%08d", ((uint32_t)(logRecordSend.mileage + (float)sysCfg.baseMileage)*10));
						strcat(pkGpsData, tmpBuf);
						#ifdef TAMBI
						//so lan qua toc do trong ngay
						sprintf(tmpBuf,",%d",logRecordSend.overSpeedCnt);
						strcat(pkGpsData,tmpBuf);
						//so lan dug do xe trong ngay
						sprintf(tmpBuf,",%d",logRecordSend.parkCnt);
						strcat(pkGpsData,tmpBuf);
						//so lan tat mo khoa xe trong ngay
						sprintf(tmpBuf,",%d",logRecordSend.onOffEngineCnt);
						strcat(pkGpsData,tmpBuf);
						//thoi gian dung do tai thoi dien dang dung do
						sprintf(tmpBuf,",%d",logRecordSend.parkingTime);
						strcat(pkGpsData,tmpBuf);
						strcat(pkGpsData, "\r\n");
						#endif
						#ifdef CUSTOMER_ADA
						//so lan mo cua
						sprintf(tmpBuf,",%.02f",logRecordSend.speedFreq);
						strcat(pkGpsData,tmpBuf);
						#endif
						APQUEUE_Put(&trackerRb, (uint8_t*)pkGpsData);
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				flagSendStatus = 0;
				serverSendDataFlag = 0;
				
				pkGpsData[0] = 0;
				strcat(pkGpsData, "$POS,");
				strcat(pkGpsData, (char *)sysCfg.id);
				sprintf(tmpBuf, ",%03d",sysCfg.driverIndex);
				strcat(pkGpsData, tmpBuf);
				//strcat(pkGpsData, (char *)sysCfg.licenseNo);
				// 14 bytes timestamp
				sprintf(tmpBuf, ",%02d%02d%02d.00", sysTime.hour, sysTime.min, sysTime.sec);
				strcat(pkGpsData, tmpBuf);
				
				sprintf(tmpBuf, ",%02d%02d%02d",sysTime.mday, sysTime.month, sysTime.year%100);
				strcat(pkGpsData, tmpBuf);
				if((nmeaInfo.sig >= 1) && (nmeaInfo.mlon != 0) && (nmeaInfo.mlat != 0))
				{
						sprintf(tmpBuf, ",%.06f,%c,%.06f,%c", nmeaInfo.mlon, nmeaInfo.ew, nmeaInfo.mlat, nmeaInfo.ns);
						fHDOPTemp = nmeaInfo.HDOP;
				}
				else if((tcpSocketStatus[0] == SOCKET_OPEN) && (cellLocate.lon != 0) && (cellLocate.lat != 0))
				{
					sprintf(tmpBuf, ",%.06f,%c,%.06f,%c", cellLocate.lon,cellLocate.ew, cellLocate.lat, cellLocate.ns);
					fHDOPTemp = cellLocate.uncertainty;
				}
				else if((lastGpsInfo.mlon != 0) && (lastGpsInfo.mlat != 0))
				{
					sprintf(tmpBuf, ",%.06f,%c,%.06f,%c", lastGpsInfo.mlon, lastGpsInfo.ew, lastGpsInfo.mlat, lastGpsInfo.ns);
					fHDOPTemp = lastGpsInfo.HDOP;
					
				}
				else
				{
					sprintf(tmpBuf, ",0.000000,E,0.000000,N");	
					fHDOPTemp = 99.99;
			  }
				strcat(pkGpsData, tmpBuf);
				
				sprintf(tmpBuf, ",%.01f,%.01f", currentSpeed,lastNmeaInfo.direction);
				strcat(pkGpsData, tmpBuf);
				// 11
				if(0)
					strcat(pkGpsData, ",0");
				else
					strcat(pkGpsData, ",1");
				// 10
				if(0)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//9
				if((GET_ACC_PIN) && currentSpeed > 15/*km/h*/) //ACC
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//8
				if(nmeaInfo.sig < 1)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//7
				if(trackerStatus.overSpeed)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//6
				strcat(pkGpsData, "0");
				//5
				#if defined CUSTOMER_OK_9108TW || defined CUSTOMER_ADA
				if(GET_DOOR_INPUT_PIN)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				#else
				if(!GET_DOOR_INPUT_PIN)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				#endif
				//4
				if(!GET_POWER_PIN)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//3
				if(GET_SOS_PIN)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				//2+1
				if(GET_ACC_PIN)
					strcat(pkGpsData, "00");
				else
					strcat(pkGpsData, "11");
				//0
				#ifdef TAMBI
				if(speedRecord.drivingTime)
					strcat(pkGpsData, "1");
				else
					strcat(pkGpsData, "0");
				#endif
				//so lan mo cua
				sprintf(tmpBuf,",%d",0);
				strcat(pkGpsData,tmpBuf);
				//thoi gian lai xe lien tuc va tong thoi gian lai xe trong ngay
				sprintf(tmpBuf, ",%02d%02d",0,0);
				strcat(pkGpsData, tmpBuf);
				sprintf(tmpBuf, ",%02d%02d",0,0);
				strcat(pkGpsData, tmpBuf);
				
				sprintf(tmpBuf, ",%.02f", fHDOPTemp );
				strcat(pkGpsData, tmpBuf);
				
				// 8 bytes adc
				an1Voltage = 0;
				an2Voltage = 0;
				sprintf(tmpBuf, ",%.02f,%.02f", an1Voltage, an2Voltage);
				strcat(pkGpsData, tmpBuf);
				//strcat(pkGpsData, ",2.2,2.0");
				// 8 bytes mileage
				sprintf(tmpBuf, ",%08d", ((uint32_t)((mileage + (float)sysCfg.baseMileage)*10)));
				strcat(pkGpsData, tmpBuf);
				#ifdef TAMBI
				//so lan qua toc do trong ngay
				sprintf(tmpBuf,",%d",speedRecord.overSpeedCnt);
				strcat(pkGpsData,tmpBuf);
				//so lan dug do xe trong ngay
				sprintf(tmpBuf,",%d",speedRecord.parkCnt);
				strcat(pkGpsData,tmpBuf);
				//so lan tat mo khoa xe trong ngay
				sprintf(tmpBuf,",%d",speedRecord.onEngineCnt + speedRecord.offEngineCnt);
				strcat(pkGpsData,tmpBuf);
				//thoi gian dung do tai thoi dien dang dung do
				sprintf(tmpBuf,",%d",speedRecord.parkingTime);
				strcat(pkGpsData,tmpBuf);
				strcat(pkGpsData, "\r\n");
				#endif
				
				#ifdef CUSTOMER_ADA
				//so lan mo cua
				sprintf(tmpBuf,",%.02f",speedFreq);
				strcat(pkGpsData,tmpBuf);
				#endif
				APQUEUE_Put(&trackerRb, (uint8_t*)pkGpsData);
			}
		}
}

uint16_t CalculateCRC(const int8_t* buf, int16_t len)
{
	uint16_t crc = 0xFFFF;
	int16_t pos, i;
	
	for(pos = 0; pos < len; pos++){
		crc ^= buf[pos];          // XOR byte into least sig. byte of crc
	
		for(i = 8; i != 0; i--){    // Loop over each bit
			if ((crc & 0x0001) != 0){      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;  
}

//calculate haversine distance for linear distance
float haversine_km(float lat1, float long1, float lat2, float long2)
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6367 * c;

    return d;
}


