#include "system_config.h"
#include "tick.h"
#include "at_command_parser.h"
CONFIG_POOL sysCfg;




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
		saveFlag = 1;
		sysCfg.id[0] = 'Z';
		sysCfg.id[1] = 'T';
		sysCfg.id[2] = 'A';
		sysCfg.id[3] = '-';
		sysCfg.id[4] = '0';
		sysCfg.id[5] = '5';
		sysCfg.id[6] = 0;
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
		sysCfg.dServerUseIp = 1;
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
		strcpy((char *)sysCfg.priFserverName, DEFAULT_DSERVER_NAME);
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
		strcpy((char *)sysCfg.secFserverName, DEFAULT_DSERVER_NAME);
		saveFlag = 1;
	}
	
	// use IP or domain
	if(sysCfg.fServerUseIp != 0 && sysCfg.fServerUseIp != 1)
	{
		sysCfg.fServerUseIp = 1;
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
	

	if(sysCfg.enableWarning == 0xFFFF)
		sysCfg.enableWarning = DEFAULT_ENABLE_WARNING;
	
	
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
	if(sysCfg.prepaidAccount == 0xFFFFFFFF)
	{
		saveFlag = 1;
		sysCfg.prepaidAccount = 0;
	}
	if(sysCfg.featureSet == 0xFFFF)
	{
		saveFlag = 1;
		sysCfg.featureSet = 0;
		sysCfg.featureSet |= FEATURE_SUPPORT_BEEP_WARNING;
	}
	
	if(sysCfg.lastError == 0xFFFF)
	{
		sysCfg.lastError = 0;
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
		
	if(sysCfg.sleepTime == 0xFFFFFFFF)
		sysCfg.sleepTime = DEFAULT_SLEEP_TIME;
	
	if(sysCfg.moduleSupportFuntion == 0xFF)
		sysCfg.moduleSupportFuntion = DEFAULT_MODULE_SUPPORT;
	
	if(sysCfg.speedEngineOff == 0xFFFFFFFF)
		sysCfg.speedEngineOff = DEFAULT_SPEED_ENGINE_OFF;
	
	if(sysCfg.securityOn == 0xFF)
		sysCfg.securityOn = DEFAULT_SECURITY_ON_OFF;
	
	if(sysCfg.securityAction == 0xFF)
		sysCfg.securityAction = DEFAULT_SECURITY_ACTION;
	
	if(sysCfg.SOSKeyPressTimes == 0xFF)
		sysCfg.SOSKeyPressTimes = DEFAULT_SOS_KEY_PRESS_TIMES;
	
	if(sysCfg.SOSKeyPressPeriod == 0xFF)
		sysCfg.SOSKeyPressPeriod = DEFAULT_SOS_KEY_PRESS_PERIOD;
	
	if(sysCfg.powerLowWarningPeriod == 0xFF)
		sysCfg.powerLowWarningPeriod = DEFAULT_POWER_LOW_WARNING_PERIOD;
	
	if(sysCfg.batteryLowWarningPeriod == 0xFF)
		sysCfg.batteryLowWarningPeriod = DEFAULT_BATTERY_LOW_WARNING_PERIOD;
	
	if(sysCfg.powerOffDelayTime == 0xFFFFFFFF)
		sysCfg.powerOffDelayTime = DEFAULT_POWER_OFF_DELAY_TIME;
	if(sysCfg.serverSendPeriod == 0xFFFFFFFF)
		sysCfg.serverSendPeriod = DEFAULT_SERVER_SEND_PERIOD;

	if(saveFlag)
	{
		CFG_Save();
	}
}
void CFG_Save(void)
{
	int16_t i;
	uint32_t u32Temp;
	uint8_t *u8Pt;
	uint32_t *cfgdest;
	uint16_t  FlashStatus;
	
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
}
