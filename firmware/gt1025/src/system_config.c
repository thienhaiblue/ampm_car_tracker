/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "system_config.h"
#include "lib/sys_tick.h"
#include <stdlib.h>
#include "rtc.h"

CONFIG_POOL sysCfg;
uint8_t sysUploadConfig = 2;

FW_UPDATE_TYPE firmware;
uint8_t smsEnableCar = 0;
uint8_t smsEnableCarTimeOut = 0;
uint8_t protectEnable = 0;
uint8_t vibrationProtectEnable = 0;

uint8_t sysResetMcuFlag = MCU_RESET_NONE;

extern uint32_t DB_SaveAll(void);

void ResetMcuSet(uint8_t resetType)
{
	sysResetMcuFlag = resetType;
}

void ResetMcuTask(void)
{
		static Timeout_Type tMcuResetTimeout;
		switch(sysResetMcuFlag)
		{
			case MCU_RESET_NONE:

			break;
			case MCU_RESET_IMMEDIATELY:
				DB_SaveAll();
				NVIC_SystemReset();
			break;
			case MCU_RESET_AFTER_10_SEC:
				InitTimeout(&tMcuResetTimeout,SYSTICK_TIME_SEC(10));
				sysResetMcuFlag = MCU_RESET_IS_WAITING;
			break; 
			case MCU_RESET_AFTER_30_SEC:
				InitTimeout(&tMcuResetTimeout,SYSTICK_TIME_SEC(30));	
				sysResetMcuFlag = MCU_RESET_IS_WAITING;
			break;
			case MCU_RESET_IS_WAITING:
				if(CheckTimeout(&tMcuResetTimeout) == SYSTICK_TIMEOUT)
				{
					DB_SaveAll();
					NVIC_SystemReset();
				}
			break;
			default:
				DB_SaveAll();
				NVIC_SystemReset();
				break;
		}
}


void CFG_ReLoad(void)
{
	memcpy(&sysCfg, (void*)CONFIG_AREA_START, sizeof(CONFIG_POOL));
}
void CFG_Load(void)
{
	uint32_t saveFlag = 0,u32Temp = 0;
	int16_t i;
	uint8_t *u8Pt;
	
	memcpy(&sysCfg, (void*)CONFIG_AREA_START, sizeof(CONFIG_POOL));
	u8Pt = (uint8_t *)&sysCfg;
	u32Temp = 0;
	for(i = 0;i < sizeof(CONFIG_POOL)-sizeof(sysCfg.crc);i++)
	{
		u32Temp += u8Pt[i];
	}
	if(u32Temp != sysCfg.crc)
	{
		memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
		saveFlag = 1;
	}
	if((uint8_t)sysCfg.imei[0] == 0xFF)
	{
		saveFlag = 1;
		sysCfg.imei[0] = 0;
	}

	if((uint8_t)sysCfg.id[0] == 0xFF)
	{
		strcpy((char *)sysCfg.id, DEFAULT_ID);
		saveFlag = 1;
	}
	
	if(sysCfg.fUserName[0] == 0xff)
	{
		strcpy((char *)sysCfg.fUserName, DEFAULT_F_USER);
		saveFlag = 1;
	}
	if(sysCfg.fPassword[0] == 0xff)
	{
		strcpy((char *)sysCfg.fPassword, DEFAULT_F_PASS);
		saveFlag = 1;
	}
	
	// load default SMS config password
	if((uint8_t)sysCfg.smsPwd[0] == 0xFF || sysCfg.smsPwd[0] == 0)
	{
		saveFlag = 1;
		strcpy((char *)sysCfg.smsPwd, DEFAULT_SMS_PWD);
	}
	
	if((uint8_t)sysCfg.whiteCallerNo[0] == 0xFF)
	{
		saveFlag = 1;
		strcpy((char *)sysCfg.whiteCallerNo, DEFAULT_BOST_NUMBER);
	}
	
	// load default GPRS config
	if((uint8_t)sysCfg.gprsApn[0] == 0xFF || sysCfg.gprsApn[0] == 0)
	{
		strcpy((char *)sysCfg.gprsApn, DEFAULT_GPSR_APN);
		strcpy((char *)sysCfg.gprsUsr, DEFAULT_GPRS_USR);
		strcpy((char *)sysCfg.gprsPwd, DEFAULT_GPRS_PWD);
		saveFlag = 1;
	}
	
	if(sysCfg.curDServer == 0xff)
	{
		sysCfg.curDServer = 0;
		saveFlag = 1;
	}
	//////////////////////////////////////////////
	// load default data server IP
	if((sysCfg.priDserverIp[0]==0xFFFF || sysCfg.priDserverIp[1]==0xFFFF))
	{
		((uint8_t*)(sysCfg.priDserverIp))[0] = DEFAULT_DSERVER_IP0;
		((uint8_t*)(sysCfg.priDserverIp))[1] = DEFAULT_DSERVER_IP1;
		((uint8_t*)(sysCfg.priDserverIp))[2] = DEFAULT_DSERVER_IP2;
		((uint8_t*)(sysCfg.priDserverIp))[3] = DEFAULT_DSERVER_IP3;
		
		sysCfg.priDserverPort = DEFAULT_DSERVER_PORT;
		saveFlag = 1;
	}
	
	// load default data server name
	if((uint8_t)sysCfg.priDserverName[0] == 0xFF)
	{
		strcpy((char *)sysCfg.priDserverName, DEFAULT_DSERVER_NAME);
		saveFlag = 1;
	}
	
	//////////////////////////////////////////////
	// load default data server IP
	if((sysCfg.secDserverIp[0]==0xFFFF || sysCfg.secDserverIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.secDserverIp))[0] = DEFAULT_DSERVER_IP0;
		((uint8_t*)(sysCfg.secDserverIp))[1] = DEFAULT_DSERVER_IP1;
		((uint8_t*)(sysCfg.secDserverIp))[2] = DEFAULT_DSERVER_IP2;
		((uint8_t*)(sysCfg.secDserverIp))[3] = DEFAULT_DSERVER_IP3;
		
		sysCfg.secDserverPort = DEFAULT_DSERVER_PORT;
		saveFlag = 1;
	}
	
	// load default data server name
	if((uint8_t)sysCfg.secDserverName[0] == 0xFF)
	{
		strcpy((char *)sysCfg.secDserverName, DEFAULT_DSERVER_NAME);
		saveFlag = 1;
	}
	// use IP or domain
	if(sysCfg.dServerUseIp != 0 && sysCfg.dServerUseIp != 1)
	{
		sysCfg.dServerUseIp = DEFAULT_DSERVER_USEIP;
		saveFlag = 1;
	}
	
	
	if(sysCfg.curFServer == 0xff)
	{
		sysCfg.curFServer = 0;
		saveFlag = 1;
	}
	/////////////////////////////////////////////
	// load default firmware server name
	if((sysCfg.priFserverIp[0]==0xFFFF || sysCfg.priFserverIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.priFserverIp))[0] = DEFAULT_FSERVER_IP0;
		((uint8_t*)(sysCfg.priFserverIp))[1] = DEFAULT_FSERVER_IP1;
		((uint8_t*)(sysCfg.priFserverIp))[2] = DEFAULT_FSERVER_IP2;
		((uint8_t*)(sysCfg.priFserverIp))[3] = DEFAULT_FSERVER_IP3;
		
		sysCfg.priFserverPort = DEFAULT_FSERVER_PORT;
		saveFlag = 1;
	}
	
	// load default firmware server name
	if((uint8_t)sysCfg.priFserverName[0] == 0xFF)
	{
		strcpy((char *)sysCfg.priFserverName, DEFAULT_FSERVER_NAME);
		saveFlag = 1;
	}
	
	
	/////////////////////////////////////////////
	// load default firmware server name
	if((sysCfg.secFserverIp[0]==0xFFFF || sysCfg.secFserverIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.secFserverIp))[0] = DEFAULT_FSERVER_IP0;
		((uint8_t*)(sysCfg.secFserverIp))[1] = DEFAULT_FSERVER_IP1;
		((uint8_t*)(sysCfg.secFserverIp))[2] = DEFAULT_FSERVER_IP2;
		((uint8_t*)(sysCfg.secFserverIp))[3] = DEFAULT_FSERVER_IP3;
		
		sysCfg.secFserverPort = DEFAULT_FSERVER_PORT;
		saveFlag = 1;
	}
	
	// load default firmware server name
	if((uint8_t)sysCfg.secFserverName[0] == 0xFF)
	{
		strcpy((char *)sysCfg.secFserverName, DEFAULT_FSERVER_NAME);
		saveFlag = 1;
	}
	
	// use IP or domain
	if(sysCfg.fServerUseIp != 0 && sysCfg.fServerUseIp != 1)
	{
		sysCfg.fServerUseIp = DEFAULT_FSERVER_USEIP;
		saveFlag = 1;
	}
	
	if(sysCfg.curInfoServer == 0xff)
	{
		sysCfg.curInfoServer = 0;
		saveFlag = 1;
	}
	/////////////////////////////////////////////
	// load default info server name
	if((sysCfg.priInfoServerIp[0]==0xFFFF || sysCfg.priInfoServerIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.priInfoServerIp))[0] = DEFAULT_INFO_SERVER_IP0;
		((uint8_t*)(sysCfg.priInfoServerIp))[1] = DEFAULT_INFO_SERVER_IP1;
		((uint8_t*)(sysCfg.priInfoServerIp))[2] = DEFAULT_INFO_SERVER_IP2;
		((uint8_t*)(sysCfg.priInfoServerIp))[3] = DEFAULT_INFO_SERVER_IP3;
		
		sysCfg.priInfoServerPort = DEFAULT_INFO_SERVER_PORT;
		saveFlag = 1;
	}
	

	
	
	/////////////////////////////////////////////
	// load default info server name
	if((sysCfg.secInfoServerIp[0]==0xFFFF || sysCfg.secInfoServerIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.secInfoServerIp))[0] = DEFAULT_INFO_SERVER_IP0;
		((uint8_t*)(sysCfg.secInfoServerIp))[1] = DEFAULT_INFO_SERVER_IP1;
		((uint8_t*)(sysCfg.secInfoServerIp))[2] = DEFAULT_INFO_SERVER_IP2;
		((uint8_t*)(sysCfg.secInfoServerIp))[3] = DEFAULT_INFO_SERVER_IP3;
		
		sysCfg.secInfoServerPort = DEFAULT_FSERVER_PORT;
		saveFlag = 1;
	}
	
	// load default info server name
	if((uint8_t)sysCfg.secInfoServerName[0] == 0xFF)
	{
		strcpy((char *)sysCfg.secInfoServerName, DEFAULT_INFO_SERVER_NAME);
		saveFlag = 1;
	}
	
	// use IP or domain
	if(sysCfg.infoServerUseIp != 0 && sysCfg.infoServerUseIp != 1)
	{
		sysCfg.infoServerUseIp = DEFAULT_INFO_SERVER_USEIP;
		saveFlag = 1;
	}
	
	
	// load default driver name
	if(sysCfg.driverIndex == 0xFF)
	{
		sysCfg.driverIndex = 0;
		saveFlag = 1;
	}
	
	for(i=0;i<CONFIG_MAX_DRIVERS;i++){
		if((uint8_t)sysCfg.driverList[i].driverName[0] == 0xFF){
			sysCfg.driverList[i].driverName[0] = 0;
			sysCfg.driverList[i].licenseNo[0] = 0;
			sysCfg.driverList[i].issueDate[0] = 0;
			sysCfg.driverList[i].expiryDate[0] = 0;
			sysCfg.driverList[i].phoneNo[0] = 0;
			saveFlag = 1;
		}
	}
	
	// load default Plate number
	if((uint8_t)sysCfg.plateNo[0] == 0xFF)
	{
		saveFlag = 1;
		sysCfg.plateNo[0] = 0;
	}
	// load default VIN
	if((uint8_t)sysCfg.vin[0] == 0xFF)
	{
		sysCfg.vin[0] = 0;
		saveFlag = 1;
	}
	// load default speed sensor ratio
	if(sysCfg.speedSensorRatio == 0 || sysCfg.speedSensorRatio == 0xFFFF)
	{
		saveFlag = 1;
		sysCfg.speedSensorRatio = DEFAULT_SPEED_SENSOR_RATIO;
	}
	if(sysCfg.useGpsSpeed == 0xffff)
	{
		saveFlag = 1;
		sysCfg.useGpsSpeed = 1;
	}
	
	// raw mode by default
	if(sysCfg.a1Mode == 0xFFFF){
		sysCfg.a1Mode = 0;
		sysCfg.a1LowerBound = 0;
		sysCfg.a1UpperBound = 1023;
		saveFlag = 1;
	}
	
	// raw mode by default
	if(sysCfg.a2Mode == 0xFFFF){
		sysCfg.a2Mode = 0;
		sysCfg.a2LowerBound = 0;
		sysCfg.a2UpperBound = 1023;
		saveFlag = 1;
	}
	if(sysCfg.curIServer == 0xff)
	{
		sysCfg.curIServer = 0;
		saveFlag = 1;
	}
	/////////////////////////////////////////////
	// load default image server name
	if((sysCfg.priIserverIp[0]==0xFFFF || sysCfg.priIserverIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.priIserverIp))[0] = DEFAULT_ISERVER_IP0;
		((uint8_t*)(sysCfg.priIserverIp))[1] = DEFAULT_ISERVER_IP1;
		((uint8_t*)(sysCfg.priIserverIp))[2] = DEFAULT_ISERVER_IP2;
		((uint8_t*)(sysCfg.priIserverIp))[3] = DEFAULT_ISERVER_IP3;
		
		sysCfg.priIserverPort = DEFAULT_ISERVER_PORT;
		saveFlag = 1;
	}
	//sysCfg.priIserverPort = DEFAULT_ISERVER_PORT;
	/////////////////////////////////////////////
	// load default image server name
	if((sysCfg.secIserverIp[0]==0xFFFF || sysCfg.secIserverIp[1]==0xFFFF)){
		((uint8_t*)(sysCfg.secIserverIp))[0] = DEFAULT_ISERVER_IP0;
		((uint8_t*)(sysCfg.secIserverIp))[1] = DEFAULT_ISERVER_IP1;
		((uint8_t*)(sysCfg.secIserverIp))[2] = DEFAULT_ISERVER_IP2;
		((uint8_t*)(sysCfg.secIserverIp))[3] = DEFAULT_ISERVER_IP3;
		
		sysCfg.secIserverPort = DEFAULT_ISERVER_PORT;
		saveFlag = 1;
	}
	
	// use IP or domain
	if(sysCfg.iServerUseIp != 0 && sysCfg.iServerUseIp != 1)
	{
		sysCfg.iServerUseIp = 1;
		saveFlag = 1;
	}
	
	if(sysCfg.numCameras == 0xFF)
	{
		saveFlag = 1;
		sysCfg.numCameras = 0;
	}
	//sysCfg.numCameras = 1;
	if(sysCfg.cameraCompression < 100 || sysCfg.cameraCompression > 250)
	{
		sysCfg.cameraCompression = 100;
		saveFlag = 1;
	}
	
	if(sysCfg.cameraWorkingStartTime >= 24)
	{
		sysCfg.cameraWorkingStartTime = 0;
		saveFlag = 1;
	}
	
	if(sysCfg.cameraWorkingStopTime > 24)
	{
		sysCfg.cameraWorkingStopTime = 24;
		saveFlag = 1;
	}
	
	if(sysCfg.cameraInterval == 0 || sysCfg.cameraInterval == 0xFFFF)
	{
		saveFlag = 1;
		sysCfg.cameraInterval = 0;
	}
	if(sysCfg.enableWarning == 0xFFFF)
	{
		sysCfg.enableWarning = DEFAULT_ENABLE_WARNING;
		saveFlag = 1;
	}
	
	if(sysCfg.cameraEvents == 0xFFFF)
		sysCfg.cameraEvents = CAMERA_INTTERVAL_SHOT;
	
		// load default speed limit
	if(sysCfg.speedLimit == 0 || sysCfg.speedLimit == 0xFFFF)
	{
		saveFlag = 1;
		sysCfg.speedLimit = DEFAULT_SPEED_LIMIT;
	}
		
	if(sysCfg.drivingTimeLimit == 0 || sysCfg.drivingTimeLimit == 0xFFFFFFFF)
	{
		saveFlag = 1;
		sysCfg.drivingTimeLimit = DEFAULT_DRIVING_TIME_LIMIT;
	}
		
	if(sysCfg.totalDrivingTimeLimit == 0 || sysCfg.totalDrivingTimeLimit == 0xFFFFFFFF)
	{
		saveFlag = 1;
		sysCfg.totalDrivingTimeLimit = DEFAULT_TOTAL_DRIVING_TIME_LIMIT;
	}
	if(sysCfg.baseMileage == 0xFFFFFFFF)
	{
		saveFlag = 1;
		sysCfg.baseMileage = 0;
	}

	if(sysCfg.featureSet == 0xFFFF)
	{
		saveFlag = 1;
		sysCfg.featureSet = DEFAULE_FEATURE;
	}
	
	if(sysCfg.lastError == 0xFFFF)
	{
		sysCfg.lastError = 0;
	}
	
	sysCfg.testMode = 0;
	
	if(sysCfg.loginKey[0] == 0xFFFFFFFF && sysCfg.loginKey[1] == 0xFFFFFFFF
		&& sysCfg.loginKey[2] == 0xFFFFFFFF && sysCfg.loginKey[3] == 0xFFFFFFFF)
	{
		sysCfg.loginKey[0] = DEFAULT_LOGIN_KEY0;
		sysCfg.loginKey[1] = DEFAULT_LOGIN_KEY1;
		sysCfg.loginKey[2] = DEFAULT_LOGIN_KEY2;
		sysCfg.loginKey[3] = DEFAULT_LOGIN_KEY3;
		saveFlag = 1;
	}
	// report interval
	if(sysCfg.runReportInterval == 0xFFFF)
	{
		sysCfg.runReportInterval = DEFAULT_REPORT_INTERVAL;
		saveFlag = 1;
	}
	// report interval
	if(sysCfg.stopReportInterval == 0xFFFF)
	{
		sysCfg.stopReportInterval = DEFAULT_REPORT_INTERVAL;
		saveFlag = 1;
	}
	
		if(sysCfg.sleepTimer == 0xFFFFFFFF)
	{
		sysCfg.sleepTimer = DEFAULT_SLEEP_TIMER;
		saveFlag = 1;
	}
	
	if(sysCfg.accountAlarmCheck >= 24)
	{
		sysCfg.accountAlarmCheck = 6;//6h AM
		saveFlag = 1;
	}
		
	if((uint8_t)sysCfg.upgradeTimeStamp[0] == 0xFF)
	{
		sysCfg.upgradeTimeStamp[0] = 0;
		saveFlag = 1;
	}
	if((uint8_t)sysCfg.infoString[0] == 0xFF)
	{
		saveFlag = 1;
		strcpy((char *)sysCfg.infoString, DEFAULT_INFO_STRING);
	}
	
	if(sysCfg.autoSecurityTime == 0xFFFFFFFF)
	{
		sysCfg.autoSecurityTime = DEFAULT_AUTO_ENABLE_SECURITY_TIME;
		saveFlag = 1;
	}
	
	if(sysCfg.speedEngineOff == 0xFFFFFFFF)
	{
		sysCfg.speedEngineOff = DEFAULT_SPEED_ENGINE_OFF;
		saveFlag = 1;
	}
	
	if(sysCfg.securityOn == 0xFF)
	{
		sysCfg.securityOn = DEFAULT_SECURITY_ON_OFF;
		saveFlag = 1;
	}
	
	if(sysCfg.securityAction == 0xFF)
	{
		sysCfg.securityAction = DEFAULT_SECURITY_ACTION;
		saveFlag = 1;
	}
	
	if(sysCfg.SOSKeyPressTimes == 0xFF)
	{
		sysCfg.SOSKeyPressTimes = DEFAULT_SOS_KEY_PRESS_TIMES;
		saveFlag = 1;
	}
	
	if(sysCfg.SOSKeyPressPeriod == 0xFF)
	{
		sysCfg.SOSKeyPressPeriod = DEFAULT_SOS_KEY_PRESS_PERIOD;
		saveFlag = 1;
	}
	
	if(sysCfg.powerLowWarningPeriod == 0xFFFFFFFF)
	{
		sysCfg.powerLowWarningPeriod = DEFAULT_POWER_LOW_WARNING_PERIOD;
		saveFlag = 1;
	}
	
	if(sysCfg.batteryLowWarningPeriod == 0xFFFFFFFF)
	{
		sysCfg.batteryLowWarningPeriod = DEFAULT_BATTERY_LOW_WARNING_PERIOD;
		saveFlag = 1;
	}
	
	if(sysCfg.powerLowWarningLevel == 0xFF)
	{
		sysCfg.powerLowWarningLevel = DEFAULT_POWER_LOW_WARNING_LEVEL;
		saveFlag = 1;
	}
	
	if(sysCfg.batteryLowWarningLevel == 0xFF)
	{
		sysCfg.batteryLowWarningLevel = DEFAULT_BATTERY_LOW_WARNING_LEVEL;
		saveFlag = 1;
	}
	
	
	if(sysCfg.powerOffDelayTime == 0xFFFFFFFF || sysCfg.powerOffDelayTime < 3600)
	{
		sysCfg.powerOffDelayTime = DEFAULT_POWER_OFF_DELAY_TIME;
		saveFlag = 1;
	}
	
	if(sysCfg.accountAlarmCheck >= 24)
	{
		sysCfg.accountAlarmCheck = 6;//6h AM
		saveFlag = 1;
	}

	if(saveFlag)
	{
		CFG_Save();
	}

}


uint8_t CFG_CheckSum(CONFIG_POOL *sysCfg)
{
	uint32_t u32Temp = 0;
	int16_t i;
	uint8_t *u8Pt;
	u8Pt = (uint8_t *)sysCfg;
	u32Temp = 0;
	for(i = 0;i < sizeof(CONFIG_POOL)-sizeof(sysCfg->crc);i++)
	{
		u32Temp += u8Pt[i];
	}
	if(u32Temp != sysCfg->crc)
	{
		return 0;
	}
	return 1;
}
void CFG_Save(void)
{
	int16_t i;
	uint32_t u32Temp;
	uint8_t *u8Pt;
	uint32_t *cfgdest;
	uint16_t  FlashStatus;

	
	if(memcmp((void *)CONFIG_AREA_START,&sysCfg,sizeof(CONFIG_POOL)) == NULL)
		return;
	__disable_irq();	
	
	u8Pt = (uint8_t *)&sysCfg;
	u32Temp = 0;
	for(i = 0;i < sizeof(CONFIG_POOL)-sizeof(sysCfg.crc);i++)
	{
		u32Temp += u8Pt[i];
	}
	sysCfg.crc = u32Temp;
	
	FLASH_Unlock();
	for(i=0; i <= sizeof(CONFIG_POOL)/ PAGE_SIZE; i++) 
	{
		FlashStatus = FLASH_ErasePage((uint32_t)(CONFIG_AREA_START + i*PAGE_SIZE));
		if(FLASH_COMPLETE != FlashStatus) 
		{
			FLASH_Lock();
			__enable_irq();
			return;
		}
	}
	cfgdest = (uint32_t*)&sysCfg;
	for(i=0; i < sizeof(CONFIG_POOL); i+=4) 
	{
		FlashStatus = FLASH_ProgramWord(CONFIG_AREA_START + i, *(uint32_t*)(cfgdest + i/4));
		if(*(uint32_t*)(cfgdest + i/4) != *(uint32_t*)(CONFIG_AREA_START + i)) //check wrote data
		{
			FLASH_Lock();
		__enable_irq();
			return;
		}
	}
	FLASH_Lock();

	__enable_irq();
	RTC_SetAlarm(RTC_GetCounter() + RTC_ALARM_INTERVAL);
}
