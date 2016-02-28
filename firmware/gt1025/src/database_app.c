/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "database_app.h"
#include "db.h"
#include "rtc.h"
#include "diskio.h"
#include "ff.h"
#include "hw_config.h"
#include "buzzer.h"
#include "lib/sys_tick.h"
#include "app_config_task.h"
#include "uart3.h"
#include "mmc_ssp.h"
#include "msc_init.h"
#include "led.h"
#define DATABASE_DBG(...)		//DbgCfgPrintf(__VA_ARGS__)

DB_U32 currentTimeSave;
DB_U32 beginTimeSave;
DB_U32 endTimeSave;
DB_U32 gpsLatSave;
DB_U32 gpsLonSave;
DB_U32 mileageSave;
DB_U32 drivingTimeSave;
DB_U32 parkingTimeSave;
DB_U32 overDrivingTimeCntSave_totalDrivingTimeCntSave;
DB_U32 totalDrivingTimeSave;
DB_U32 overSpeedCntSave_openDoorCntSave;
DB_U32 closeDoorCntSave_onEngineCntSave;
DB_U32 offEngineCntSave_parkCntSave;
DB_U32 statusSave_crcSave;

DB_U32 gpsHDOPSave;
DB_U32 gpsDirSave;

DB_U32 timeSave;
DB_U32 sysTimeSave;
extern SPEED_RECORD speedRecord;
extern float mileage;
uint32_t DB_SaveAll(void)
{
	uint8_t *pt;
	//Save driving time
	pt = (uint8_t *)&currentTimeSave.value;
	pt[0] = speedRecord.currentTime.hour;
	pt[1] = speedRecord.currentTime.min;
	pt[2] = speedRecord.currentTime.sec;
	
	pt = (uint8_t *)&beginTimeSave.value;
	pt[0] = speedRecord.beginTime.hour;
	pt[1] = speedRecord.beginTime.min;
	pt[2] = speedRecord.beginTime.sec;
	
	pt = (uint8_t *)&endTimeSave.value;
	pt[0] = speedRecord.endTime.hour;
	pt[1] = speedRecord.endTime.min;
	pt[2] = speedRecord.endTime.sec;

	gpsLatSave.value = DB_FloatToU32(lastNmeaInfo.lat);
	gpsLonSave.value = DB_FloatToU32(lastNmeaInfo.lon);
	mileageSave.value = DB_FloatToU32(mileage);
	drivingTimeSave.value = speedRecord.drivingTime;
	parkingTimeSave.value = speedRecord.parkingTime;
	overDrivingTimeCntSave_totalDrivingTimeCntSave.value = ((uint32_t)speedRecord.totalDrivingTimeCnt)<<16 | (uint32_t )speedRecord.overDrivingTimeCnt;
	totalDrivingTimeSave.value = speedRecord.totalDrivingTime;
	overSpeedCntSave_openDoorCntSave.value = (uint32_t)speedRecord.overSpeedCnt | ((uint32_t)speedRecord.openDoorCnt)<<16;
	closeDoorCntSave_onEngineCntSave.value = (uint32_t)speedRecord.closeDoorCnt | ((uint32_t)speedRecord.onEngineCnt)<<16;
	offEngineCntSave_parkCntSave.value = (uint32_t)speedRecord.offEngineCnt | ((uint32_t)speedRecord.parkCnt)<<16;
	pt = (uint8_t *)&statusSave_crcSave.value;
	pt[0] = speedRecord.status;
	pt[1] = speedRecord.crc;
	
	gpsHDOPSave.value = DB_FloatToU32(lastNmeaInfo.HDOP);
	gpsDirSave.value = DB_FloatToU32(lastNmeaInfo.direction);
	
	sysTimeSave.value = rtcTimeSec;
	
	DB_U32Save(&currentTimeSave,CURRENT_TIME_SAVE_ADDR);
	DB_U32Save(&beginTimeSave,BEGIN_TIME_SAVE_ADDR);
	DB_U32Save(&endTimeSave,END_TIME_SAVE_ADDR);
	DB_U32Save(&gpsLatSave,GPS_LAT_SAVE_ADDR);
	DB_U32Save(&gpsLonSave,GPS_LON_SAVE_ADDR);
	DB_U32Save(&mileageSave,MILEAGE_SAVE_ADDR);
	DB_U32Save(&drivingTimeSave,DRIVING_TIME_SAVE_ADDR);
	DB_U32Save(&parkingTimeSave,PARKING_TIME_SAVE_ADDR);
	DB_U32Save(&overDrivingTimeCntSave_totalDrivingTimeCntSave,OVER_DRIVING_TIME_SAVE_ADDR);
	DB_U32Save(&totalDrivingTimeSave,TOTAL_DRIVING_TIME_SAVE_ADDR);
	DB_U32Save(&overSpeedCntSave_openDoorCntSave,OVER_SPEED_CNT_SAVE_ADDR);
	DB_U32Save(&closeDoorCntSave_onEngineCntSave,CLOSE_DOOR_CNT_SAVE_ADDR);
	DB_U32Save(&offEngineCntSave_parkCntSave,ON_OFF_ENGINE_CNT_ADDR);
	DB_U32Save(&statusSave_crcSave,STATUS_CRC_SAVE_ADDR);

	DB_U32Save(&gpsHDOPSave,GPS_HDOP_SAVE_ADDR);
	DB_U32Save(&gpsDirSave,GPS_DIR_SAVE_ADDR);
	DB_U32Save(&timeSave,TIME_SAVE_ADDR);
	DB_U32Save(&sysTimeSave,SYSTEM_TIME_SAVE_ADDR);
	return  0;
}

uint32_t DB_FlashMemInit(void)
{
	float lat,lon;
	uint8_t *pt;
	DB_InitLog(&logRecord);
	
	DB_U32Load(&currentTimeSave,CURRENT_TIME_SAVE_ADDR);
	DB_U32Load(&beginTimeSave,BEGIN_TIME_SAVE_ADDR);
	DB_U32Load(&endTimeSave,END_TIME_SAVE_ADDR);
	DB_U32Load(&gpsLatSave,GPS_LAT_SAVE_ADDR);
	DB_U32Load(&gpsLonSave,GPS_LON_SAVE_ADDR);
	DB_U32Load(&mileageSave,MILEAGE_SAVE_ADDR);
	DB_U32Load(&drivingTimeSave,DRIVING_TIME_SAVE_ADDR);
	DB_U32Load(&parkingTimeSave,PARKING_TIME_SAVE_ADDR);
	DB_U32Load(&overDrivingTimeCntSave_totalDrivingTimeCntSave,OVER_DRIVING_TIME_SAVE_ADDR);
	DB_U32Load(&totalDrivingTimeSave,TOTAL_DRIVING_TIME_SAVE_ADDR);
	DB_U32Load(&overSpeedCntSave_openDoorCntSave,OVER_SPEED_CNT_SAVE_ADDR);
	DB_U32Load(&closeDoorCntSave_onEngineCntSave,CLOSE_DOOR_CNT_SAVE_ADDR);
	DB_U32Load(&offEngineCntSave_parkCntSave,ON_OFF_ENGINE_CNT_ADDR);
	DB_U32Load(&statusSave_crcSave,STATUS_CRC_SAVE_ADDR);

	DB_U32Load(&gpsHDOPSave,GPS_HDOP_SAVE_ADDR);
	DB_U32Load(&gpsDirSave,GPS_DIR_SAVE_ADDR);
	DB_U32Load(&timeSave,TIME_SAVE_ADDR);
	DB_U32Load(&sysTimeSave,SYSTEM_TIME_SAVE_ADDR);
	//update time
	if(sysTimeSave.value > rtcTimeSec)
	{
		if(UpdateRtcTime(sysTimeSave.value)){
			TIME_FromSec(&sysTime,sysTimeSave.value);
			TIME_FromSec(&localTime,sysTimeSave.value + sysTimeZone);
		}
	}
	
	//load driving time
	pt = (uint8_t *)&currentTimeSave.value;
	speedRecord.currentTime.hour = pt[0];
	speedRecord.currentTime.min = pt[1];
	speedRecord.currentTime.sec = pt[2];
	
	pt = (uint8_t *)&beginTimeSave.value;
	speedRecord.beginTime.hour = pt[0];
	speedRecord.beginTime.min = pt[1];
	speedRecord.beginTime.sec = pt[2];
	
	pt = (uint8_t *)&endTimeSave.value;
	speedRecord.endTime.hour = pt[0];
	speedRecord.endTime.min = pt[1];
	speedRecord.endTime.sec = pt[2];

	speedRecord.lat = *((float *)(&gpsLatSave.value));
	speedRecord.lon = *((float *)(&gpsLonSave.value));
	speedRecord.speed = 0;
	speedRecord.mileage = *((float *)(&mileageSave.value));
	speedRecord.drivingTime = drivingTimeSave.value;
	speedRecord.parkingTime = parkingTimeSave.value;
	speedRecord.overDrivingTimeCnt = overDrivingTimeCntSave_totalDrivingTimeCntSave.value & 0x0000ffff;
	speedRecord.totalDrivingTime = totalDrivingTimeSave.value;
	speedRecord.totalDrivingTimeCnt = overDrivingTimeCntSave_totalDrivingTimeCntSave.value>>16 & 0x0000ffff;
	speedRecord.overSpeedCnt = overSpeedCntSave_openDoorCntSave.value & 0x0000ffff;
	speedRecord.openDoorCnt = overSpeedCntSave_openDoorCntSave.value>>16 & 0x0000ffff;
	speedRecord.closeDoorCnt = closeDoorCntSave_onEngineCntSave.value & 0x0000ffff;
	speedRecord.onEngineCnt = closeDoorCntSave_onEngineCntSave.value>>16 & 0x0000ffff;
	speedRecord.offEngineCnt = offEngineCntSave_parkCntSave.value & 0x0000ffff;
	speedRecord.parkCnt = offEngineCntSave_parkCntSave.value>>16 & 0x0000ffff;
	pt = (uint8_t *)&statusSave_crcSave.value;
	speedRecord.status = pt[0];
	speedRecord.crc = pt[1];

	
	lat = *((float *)(&gpsLatSave.value));
	lon = *((float *)(&gpsLonSave.value));

	if(lat < 0)
		lastNmeaInfo.ns = 'S';
	else 
		lastNmeaInfo.ns = 'N';
	if(lon < 0)
		lastNmeaInfo.ew = 'W';
	else 
		lastNmeaInfo.ew = 'E';
	lastNmeaInfo.HDOP = *((float *)(&gpsHDOPSave.value));
	lastNmeaInfo.direction = *((float *)(&gpsDirSave.value));
	lastNmeaInfo.lon = lon;
	lastNmeaInfo.lat = lat;
	
	if((lastNmeaInfo.lat <= 90 || lastNmeaInfo.lat >= -90) 
		&& (lastNmeaInfo.lon <= 180  || lastNmeaInfo.lon >= -180)
		)
	{
		lastNmeaInfo.fix = 3;
		lastNmeaInfo.sig = 1;
	}
	else
	{
		lastNmeaInfo.lat = 0;
		lastNmeaInfo.lon = 0;
	}
	return 0;
}



FRESULT res;


extern uint32_t USB_Connected;
char DB_RemoveDir(char *name);

char DB_Init(void)
{
	#ifndef USE_SW_DBG
	// the saveFile system on disk
	if(ioStatus.din[3].bitNew )
	{
		if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
			BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 5);
		sdfs.fs_type = 0;
		return 0xff;
	}

//	SD_EN_PIN_SET_OUTPUT;
//	SD_EN_PIN_SET;
//	SysTick_DelayMs(500);
//	SD_EN_PIN_CLR;
//	SysTick_DelayMs(500);

	#else
	if(ioStatus.din[3].bitNew)
	{
		if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
			BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 5);
		sdfs.fs_type = 0;
		return 0xff;
	}
	#endif
	mscInit();

	if(sdfs.fs_type){
		BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 1);
		DATABASE_DBG("\r\nDATABASE-> SD CARD MOUNTED\r\n");
		return 0;
	}
	else
	{
		DATABASE_DBG("\r\nDATABASE-> SD CARD MOUNT FAILED\r\n");
		if(sysCfg.featureSet & FEATURE_SUPPORT_BEEP_WARNING)
			BuzzerSetStatus(&buzzer1Ctrl,50, 50,BUZZER_TURN_ON, 5);
		return 0xff;
	}
}

char DB_CreateHierarchy(void)
{
	DATE_TIME t;
	DIR dummy;
	char path[48];

	if(ioStatus.din[3].bitNew || sdfs.fs_type == 0)
	{
		sdfs.fs_type = 0;
		return 0xff;
	}
	
	// remove too old records
	t = localTime;
	
	TIME_AddSec(&t, -100*24*60*60);
	
	sprintf(path, "LOG/%d_%02d", t.year, t.month);
	
	DB_RemoveDir(path);
	// create directory structure for new records
	if((res = f_opendir(&dummy, "LOG")) == FR_NO_PATH)
	{
		if((res = f_mkdir("LOG")) != FR_OK)
			return 0xff;
	}
	if(res == FR_OK)
	{
	// create directory structure for new records
		sprintf(path, "LOG/%d_%02d", localTime.year, localTime.month);
		if((res = f_opendir(&dummy, path)) == FR_NO_PATH)
		{
			if((res = f_mkdir(path)) != FR_OK)
				return 0xff;
		}
	}
	if(res == FR_OK)
	{
		sprintf(path, "LOG/%d_%02d/%02d", localTime.year, localTime.month, localTime.mday);
		if((res = f_opendir(&dummy, path)) == FR_NO_PATH)
		{
			if((res = f_mkdir(path)) != FR_OK)
				return 0xff;
		}
		
	}
	if(res != FR_OK)
	{
		return 0xff;
	}
	return 0;
}


/**
* save speed log to db
*/
char currentPath[48];
uint8_t saveFileIsOpened = 0;
FIL saveFile;
uint8_t saveSpeedPause = 0;
uint8_t pauseTimeout = 180;
void DB_SaveSpeed_Pause(void)
{
	if(saveFileIsOpened)
	{
		saveFileIsOpened = 0;
		f_close(&saveFile);
	}
	saveSpeedPause = 1;
	pauseTimeout = 30;
}

void DB_SaveSpeed_Continue(void)
{
	saveSpeedPause = 0;
	pauseTimeout = 0;
}

char DB_SaveSpeed(SPEED_RECORD *r)
{
	char path[48];
	uint32_t i,offset;
	if(ioStatus.din[3].bitNew || sdfs.fs_type == 0)
	{
		sdfs.fs_type = 0;
		return 0xff;
	}
	if(saveSpeedPause && pauseTimeout){
		pauseTimeout--;
		return 0xff;
	}
	i = sprintf(path, "LOG/%d_%02d/%02d/%d_%d.log", localTime.year, localTime.month, localTime.mday, localTime.hour,sysCfg.driverIndex);
	if(memcmp(currentPath,path,i) != NULL)
	{
		strcpy(currentPath,path);
		DB_CreateHierarchy();
		//close file before open
		if(saveFileIsOpened){
			f_close(&saveFile);
		}
		if(f_open(&saveFile, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK)
		{
			strcpy(currentPath,path);
			DB_CreateHierarchy();
		}
		else
		{
			saveFileIsOpened = 1;
		}
	}

	if(saveFileIsOpened)
	{
		offset = (f_size(&saveFile) / sizeof(SPEED_RECORD)) * sizeof(SPEED_RECORD);
		if((res = f_lseek(&saveFile,offset)) == FR_OK)
		{
			r->crc = DbCalcCheckSum((uint8_t *)r,sizeof(SPEED_RECORD) - 1);
			if(f_write(&saveFile, r, sizeof(SPEED_RECORD), &i) == FR_OK && i == sizeof(SPEED_RECORD))
			{
				return 0;
			}
		}
	}
	else
	{
		if((res = f_open(&saveFile, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) != FR_OK)
		{
			strcpy(currentPath,path);
			DB_CreateHierarchy();
		}
		else
		{
			saveFileIsOpened = 1;
		}
	}
	
//	DATABASE_DBG("\r\nDATABASE->FAILED TO ACCESS FILE:%s\r\n", path);
	return 0xff;
}

char GetLastRecord(SPEED_RECORD *r,DATE_TIME *current,uint8_t driverIndex)
{
	FIL file;
	static uint32_t lastRecordGotFlag = 0;
	int32_t filSize = 0,len,i;
	char fname[48],crc;
	SPEED_RECORD recordLast;
	if(lastRecordGotFlag )	return 0xff;
	lastRecordGotFlag = 1;
	
	if(ioStatus.din[3].bitNew || sdfs.fs_type == 0)
	{
		sdfs.fs_type = 0;
		return 0xff;
	}
	
	for(i = current->hour; i >= 0; i--)
	{
		sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", current->year, current->month, current->mday, i,driverIndex);
		if((res = f_open(&file, fname, FA_READ | FA_OPEN_EXISTING)) == FR_OK)
		{
			filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
			if(filSize >= sizeof(SPEED_RECORD))
				while(filSize > 0)
				{
					filSize -= sizeof(SPEED_RECORD);
					f_lseek(&file, filSize);
					res = f_read(&file, (uint8_t *)&recordLast, sizeof(SPEED_RECORD), (UINT *)&len);
					if((res == FR_OK) && len == sizeof(SPEED_RECORD))
					{
						if(recordLast.currentTime.min <= current->min)
						{
							crc = DbCalcCheckSum((uint8_t *)&recordLast,sizeof(SPEED_RECORD) - 1);
							if(recordLast.crc == crc)
							{
								*r = recordLast;
								f_close(&file);
								return 0;
							}
						}
					}
					f_close(&file);
				}
		}
	}
	return 0xff;
}

char DB_RemoveDir(char *name)
{
	FILINFO finfo;
	DIR dir;
	char path[64];
	
	if(ioStatus.din[3].bitNew || sdfs.fs_type == 0)
	{
		sdfs.fs_type = 0;
		return 0xff;
	}
	
	res = f_opendir(&dir, name);
	
	if(res == FR_DISK_ERR) sdfs.fs_type = 0;
	if(res != FR_OK) return 0;
	
	while(1)
	{
		res = f_readdir(&dir, &finfo);
		if(res != FR_OK || finfo.fname[0] == 0) break;
		if(finfo.fname[0] == '.') continue;
		
		sprintf(path, "%s/%s", name, finfo.fname);
		if(finfo.fattrib & AM_DIR)
		{
			DB_RemoveDir(path);
		}
		else
		{
			f_unlink(path);
		}
	}
	
	f_unlink(name);
	
	return 0;
}



//Printf Report

int32_t DB__Print1(DATE_TIME *time);
int32_t DB__Print2(DATE_TIME *time);



char DB_Print(DATE_TIME *time,uint8_t printNo)
{
	char error = 0;
	int8_t res;
	watchdogEnable[WTD_MAIN_LOOP] = 0;
	if((time->mday > 31) && (time->mday <= 0)) error = 1;
	if((time->month > 12) && (time->month <= 0)) error = 1;
	if((time->year > 2099) && (time->year < 2012)) error = 1;
	if((time->hour > 24)) error = 1;
	if((time->min > 60)) error = 1;
	if((time->sec > 60)) error = 1;
	if(error) {
		watchdogEnable[WTD_MAIN_LOOP] = 1;
		return 0xff;
	}
	if(ioStatus.din[3].bitNew || sdfs.fs_type == 0)
	{
		sdfs.fs_type = 0;
		watchdogEnable[WTD_MAIN_LOOP] = 1;
		return 0xff;
	}
	USART3_Init(SystemCoreClock/2,9600); //PC and printer
	if(printNo == 1)
		res = DB__Print1(time);
	else if(printNo == 2)
		res = DB__Print2(time);
	USART3_Init(SystemCoreClock/2,__USART3_BAUDRATE);
	watchdogEnable[WTD_MAIN_LOOP] = 1;
	return res;
}


#define UART3_Puts USART3_PutString

int32_t DB__Print1(DATE_TIME *time)
{
	int32_t i,j;
	FIL file;
	DATE_TIME tTemp;
	int32_t filSize = 0;
	uint8_t crc;
	char fname[48],buf[sizeof(SPEED_RECORD)];
	SPEED_RECORD sr,finalRec,sr1;
	uint8_t driverIndex = 0xff,u8Temp;
	uint16_t u16Temp,u16Temp1;
	DATE_TIME current;	
	int32_t sb,recordTime,len;
	sb = TIME_GetSec(time);
	TIME_FromSec(&current, sb);
	u8Temp = 0;
	
	for(j = time->hour;j >= 0;j--)
		for(i = 0;i < 10;i++)
		{
			sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", time->year, time->month, time->mday,j,i);
			if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
			{
				filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
				filSize -= sizeof(SPEED_RECORD);
				f_lseek(&file, filSize);
				res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
				if((res == FR_OK) && len == sizeof(SPEED_RECORD))
				{
					sr = *(SPEED_RECORD *)&buf;
					crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
					if((sr.crc == crc) && (sr.currentTime.min >= u8Temp))
					{
						u8Temp = sr.currentTime.min;
						driverIndex = i;
						j = 0;
						break;
					}
				}
				f_close(&file);
			}
			f_close(&file);
		}
	f_close(&file);
	if(driverIndex == 0xff)
	{
		UART3_Puts("Khong tim thay du lieu\r\n");
		UART3_Puts("Xin hay chon thoi gian khac\r\n");
		return 0xff;
	}
	// print header
	UART3_Puts((char *)sysCfg.infoString);
	UART3_Puts("---------------------------\r\n");
	sprintf(buf,"BKS: %s\r\n",sysCfg.plateNo); 																UART3_Puts(buf);
	sprintf(buf,"LX: %s\r\n",sysCfg.driverList[driverIndex].driverName);			UART3_Puts(buf);
	sprintf(buf,"So GPLX:%s\r\n",sysCfg.driverList[driverIndex].licenseNo);		UART3_Puts(buf);
	sprintf(buf,"So se-ri GPLX: %s\r\n",&sysCfg.imei[8]);											UART3_Puts(buf);
	UART3_Puts("---------------------------\r\n");
	//end of header
	sprintf(buf,"Thoi diem in: %02d:%02d:%02d\r\n",time->hour,time->min,time->sec);						UART3_Puts(buf);
	sprintf(buf,"              %02d/%02d/%02d\r\n",time->mday,time->month,time->year - 2000);	UART3_Puts(buf);
	UART3_Puts("---------------------------\r\n");
	tTemp = *time;
	for(i = time->hour;i >= 0;i--)
	{
		sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", time->year, time->month, time->mday, i,driverIndex);
		if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
		{
			filSize = f_size(&file);
			filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
			while(filSize > 0)
			{
				filSize -= sizeof(SPEED_RECORD);
				if(filSize >= 0)
				{
					f_lseek(&file, filSize);
					res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
					if((res == FR_OK) && len == sizeof(SPEED_RECORD))
					{
						sr = *(SPEED_RECORD *)&buf;
						crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
						if((sr.crc == crc))
						{
							tTemp.hour = sr.currentTime.hour;
							tTemp.min = sr.currentTime.min;
							tTemp.sec = sr.currentTime.sec;
							recordTime = TIME_GetSec(&tTemp);
							if(recordTime <= sb) 
							{
									finalRec = sr;
									i = 0;
									break;
							}
						}
					}
				}
			}
		}
	}
	f_close(&file);
	sprintf(buf,"So lan vuot toc do:%d\r\n",finalRec.overSpeedCnt);						UART3_Puts(buf);
	if(finalRec.overSpeedCnt)
	{
		u16Temp = finalRec.overSpeedCnt;
		u16Temp1 = 0;
		sr1 = finalRec;
		for(i = finalRec.currentTime.hour;i >= 0;i--)
		{
			sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", current.year, current.month, current.mday, i,driverIndex);
			if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
			{
				filSize = f_size(&file);
				filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
				while(filSize > 0)
				{
					filSize -= sizeof(SPEED_RECORD);
					if(filSize >= 0)
					{
						f_lseek(&file, filSize);
						res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
						if((res == FR_OK) && len == sizeof(SPEED_RECORD))
						{
							sr = *(SPEED_RECORD *)&buf;
							crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
							if((sr.crc == crc))
							{
								tTemp.hour = sr.currentTime.hour;
								tTemp.min = sr.currentTime.min;
								tTemp.sec = sr.currentTime.sec;
								recordTime = TIME_GetSec(&tTemp);
								if(recordTime <= sb) 
								{
									if(u16Temp != sr.overSpeedCnt)
									{
										u16Temp--;
										u16Temp1++;
										sprintf(buf,"Lan %d: ",u16Temp1);																								UART3_Puts(buf);
										sprintf(buf, "%02d:%02d:%02d\r\n",tTemp.hour,tTemp.min,tTemp.sec);							UART3_Puts(buf);
										sprintf(buf, "%.06f,%.06f\r\n",formatLatLng(sr1.lat),formatLatLng(sr1.lon));		UART3_Puts(buf);
										sprintf(buf, "TDCP/TDTT: %.03d,%.03f km/h;\r\n",sysCfg.speedLimit,sr1.speed);		UART3_Puts(buf);
										if(u16Temp1 >= 5 || u16Temp == 0)
										{
											i = 0;
											break;
										}
									}
									else
									{
										sr1 = sr;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	f_close(&file);
	//so lan mo cua xe
	sprintf(buf,"So lan mo cua xe:%d\r\n",finalRec.openDoorCnt);						UART3_Puts(buf);
	if(finalRec.openDoorCnt)
	{
		u16Temp = finalRec.openDoorCnt;
		u16Temp1 = 0;
		for(i = finalRec.currentTime.hour;i >= 0;i--)
		{
			sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", time->year, time->month,time->mday, i,driverIndex);
			if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
			{
				filSize = f_size(&file);
				filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
				while(filSize > 0)
				{
					filSize -= sizeof(SPEED_RECORD);
					if(filSize >= 0)
					{
						f_lseek(&file, filSize);
						res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
						if((res == FR_OK) && len == sizeof(SPEED_RECORD))
						{
							sr = *(SPEED_RECORD *)&buf;
							crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
							if((sr.crc == crc))
							{
								tTemp.hour = sr.currentTime.hour;
								tTemp.min = sr.currentTime.min;
								tTemp.sec = sr.currentTime.sec;
								recordTime = TIME_GetSec(&tTemp);
								if(recordTime <= sb) 
								{
									if(u16Temp != sr.openDoorCnt)
									{
										u16Temp--;
										u16Temp1++;
										sprintf(buf,"Lan %d: ",u16Temp1);													UART3_Puts(buf);
										sprintf(buf,"%02d:%02d:%02d\r\n",sr.currentTime.hour,sr.currentTime.min,sr.currentTime.sec);		UART3_Puts(buf);
										sprintf(buf,"%.06f,%.06f\r\n",formatLatLng(sr1.lat),formatLatLng(sr1.lon));	UART3_Puts(buf);
										if(u16Temp1 >= 5 || (u16Temp == 0))
										{
											i = 0;
											break;
										}
									}
									else
									{
										sr1 = sr;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	f_close(&file);
	sprintf(buf,"---------------------------\r\n");
	UART3_Puts(buf);
	sprintf(buf,"LXLT: %02d %02d\r\n",finalRec.drivingTime/3600,(finalRec.drivingTime%3600)/60);
	UART3_Puts(buf);
	sprintf(buf,"LXTN: %02d %02d\r\n",finalRec.totalDrivingTime/3600,(finalRec.totalDrivingTime%3600)/60);
	UART3_Puts(buf);
	sprintf(buf,"---------------------------\r\n");
	UART3_Puts(buf);
	UART3_Puts("  \r\n");
	return 0;
}

int32_t DB__Print2(DATE_TIME *time)
{
	DATE_TIME tTemp;
	int32_t filSize = 0;
	char fname[48],buf[sizeof(SPEED_RECORD)];
	SPEED_RECORD sr;
	int32_t sb = TIME_GetSec(time),i,j,recordTime,len;
	uint8_t driverIndex = 0xff,u8Temp,crc;
	DATE_TIME current;	
	FIL file;
	TIME_FromSec(&current, sb);
	u8Temp = 0;
	for(j = time->hour;j >= 0;j--)
		for(i = 0;i < 10;i++)
		{
			sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", time->year, time->month, time->mday,j,i);
			if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
			{
				filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
				filSize -= sizeof(SPEED_RECORD);
				f_lseek(&file, filSize);
				res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
				if((res == FR_OK) && len == sizeof(SPEED_RECORD))
				{
					sr = *(SPEED_RECORD *)&buf;
					crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
					if((sr.crc == crc) && (sr.currentTime.min >= u8Temp))
					{
						u8Temp = sr.currentTime.min;
						driverIndex = i;
						j = 0;
						break;
					}
				}
				f_close(&file);
			}
			f_close(&file);
		}
	f_close(&file);
	if(driverIndex == 0xff)
	{
		UART3_Puts("Khong tim thay du lieu\r\n");
		UART3_Puts("Xin hay chon thoi gian khac\r\n");
		return 0xff;
	}
	// print header
	UART3_Puts((char *)sysCfg.infoString);
	UART3_Puts("---------------------------\r\n");
	sprintf(buf,"BKS: %s\r\n",sysCfg.plateNo); 																UART3_Puts(buf);
	sprintf(buf,"LX: %s\r\n",sysCfg.driverList[driverIndex].driverName);			UART3_Puts(buf);
	sprintf(buf,"So GPLX: %s\r\n",sysCfg.driverList[driverIndex].licenseNo);	UART3_Puts(buf);
	sprintf(buf,"So se-ri GPLX:%s\r\n",&sysCfg.imei[8]);												UART3_Puts(buf);
	UART3_Puts("---------------------------\r\n");
	//end of header
	sprintf(buf,"Thoi diem in: %02d:%02d:%02d\r\n",time->hour,time->min,time->sec);						UART3_Puts(buf);
	sprintf(buf,"              %02d/%02d/%02d\r\n",time->mday,time->month,time->year - 2000);	UART3_Puts(buf);
	UART3_Puts("---------------------------\r\n");
	u8Temp = 0;
	tTemp = *time;
	for(i = time->hour;i >= 0;i--)
	{
		sprintf(fname, "LOG/%d_%02d/%02d/%d_%d.log", time->year, time->month, time->mday, i,driverIndex);
		if((res = f_open(&file, fname, FA_READ|FA_OPEN_EXISTING)) == FR_OK)
		{
			filSize = f_size(&file);
			filSize = (f_size(&file) /sizeof(SPEED_RECORD))  * sizeof(SPEED_RECORD);
			while(filSize > 0)
			{
				filSize -= sizeof(SPEED_RECORD);
				if(filSize >= 0)
				{
					f_lseek(&file, filSize);
					res = f_read(&file, buf, sizeof(SPEED_RECORD), (UINT *)&len);
					if((res == FR_OK) && len == sizeof(SPEED_RECORD))
					{
						sr = *(SPEED_RECORD *)&buf;
						crc = DbCalcCheckSum((uint8_t *)buf,sizeof(SPEED_RECORD) - 1);
						if(sr.crc == crc)
						{
							tTemp.hour = sr.currentTime.hour;
							tTemp.min = sr.currentTime.min;
							tTemp.sec = sr.currentTime.sec;
							recordTime = TIME_GetSec(&tTemp);
							if(recordTime <= sb) 
							{
								u8Temp++;
								sprintf(buf,"%d: %.03f km/h\r\n",u8Temp,sr.speed);	UART3_Puts(buf);
								if(u8Temp >= 10)
								{
									i = 0;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	f_close(&file);
	sprintf(buf,"---------------------------\r\n");
	UART3_Puts(buf);
	UART3_Puts("  \r\n");
	return 0;
}


