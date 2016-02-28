#include "db.h"
#include "sst25.h"
#define DATABASE_DBG(...)		//DbgCfgPrintf(__VA_ARGS__)

float formatLatLng(float v);


typedef struct __attribute__((packed)){
	uint32_t head;
	uint32_t tail;
	uint8_t 	crc;
}LOG_RING_TYPE;

LOG_RING_TYPE ringLog;
uint32_t logPositionCnt = 0;
uint32_t flagNewLog = 1;

float mileage;
uint32_t mileageCnt = 0;

uint32_t resetNum;
uint32_t resetCnt = 0;


void DB_ResetCntLoad(void)
{
	uint8_t u8temp,buf[5];
	int32_t i;
	uint32_t *u32pt;
	resetNum = 0;
	resetCnt = 0;
	u32pt = (uint32_t *)buf;
	for(i = 4090;i >= 0;i -= 5)
	{
		SST25_Read(RESET_CNT_ADDR + i,buf,5);
		if(*u32pt != 0xffffffff)
		{
			u8temp = DbCalcCheckSum(buf,4);
			if(resetCnt == 0)
				resetCnt = i;
			if(buf[4] == u8temp)
			{
				memcpy((uint8_t *)&resetNum,buf,4);
				break;
			}
		}
	}
}

void DB_ResetCntSave(void)
{
	uint8_t buff[5];
	resetNum++;
	memcpy(buff,(uint8_t *)&resetNum,4);
	buff[4] = DbCalcCheckSum((uint8_t *)buff,4);
	SST25_Write(RESET_CNT_ADDR + resetCnt,buff,4);
	SST25_Write(RESET_CNT_ADDR + resetCnt + 4,&buff[4],1);
	resetCnt += 5;
	if(resetCnt >= 4095)
	{
		SST25_Erase(RESET_CNT_ADDR,block4k);
		resetCnt = 0;
	}
}


void DB_MileageLoad(void)
{
	uint8_t u8temp,buf[5];
	int32_t i;
	uint32_t *u32pt;
	mileage = 0;
	mileageCnt = 0;
	u32pt = (uint32_t *)buf;
	for(i = 4090;i >= 0;i -= 5)
	{
		SST25_Read(MILEAGE_ADDR + i,buf,5);
		if(*u32pt != 0xffffffff)
		{
			u8temp = DbCalcCheckSum(buf,4);
			if(mileageCnt == 0)
				mileageCnt = i;
			if(buf[4] == u8temp)
			{
				memcpy((uint8_t *)&mileage,buf,4);
				break;
			}
		}
	}
}

void DB_MileageSave(void)
{
	uint8_t buff[5];
	memcpy(buff,(uint8_t *)&mileage,4);
	buff[4] = DbCalcCheckSum((uint8_t *)buff,4);
	SST25_Write(MILEAGE_ADDR + mileageCnt,buff,4);
	SST25_Write(MILEAGE_ADDR + mileageCnt + 4,&buff[4],1);
	mileageCnt += 5;
	if(mileageCnt >= 4095)
	{
		SST25_Erase(MILEAGE_ADDR,block4k);
		mileageCnt = 0;
	}
}

void DB_InitLog(void)
{
	uint8_t u8temp;
	int32_t i;
	LOG_RING_TYPE logTemp;
	ringLog.head = 0;
	ringLog.tail = 0;
	ringLog.crc = 0;
	logPositionCnt = 0;
	for(i = 4095 - sizeof(LOG_RING_TYPE)  ;i >= 0;i -= sizeof(LOG_RING_TYPE))
	{
		SST25_Read(LOG_POSITION_ADDR + i,(uint8_t *)&logTemp, sizeof(LOG_RING_TYPE));
		if(logTemp.head != 0xffffffff)
		{
			u8temp = DbCalcCheckSum((uint8_t *)&logTemp,sizeof(LOG_RING_TYPE) - 1);
			if(logPositionCnt == 0)
				logPositionCnt = i;
			if(logTemp.crc == u8temp)
			{
				ringLog = logTemp;
				break;
			}
		}
	}
}


void DB_RingLogReset(void)
{
	ringLog.head = 0;
	ringLog.tail = 0;
	DB_RingLogSave();
}
void DB_RingLogSave(void)
{
	ringLog.crc = DbCalcCheckSum((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 1);
	SST25_Write(LOG_POSITION_ADDR + logPositionCnt,(uint8_t *)&ringLog.head,sizeof(ringLog.head));
	logPositionCnt += sizeof(ringLog.head);
	SST25_Write(LOG_POSITION_ADDR + logPositionCnt ,(uint8_t *)&ringLog.tail,sizeof(ringLog.tail));
	logPositionCnt += sizeof(ringLog.tail);
	SST25_Write(LOG_POSITION_ADDR + logPositionCnt ,(uint8_t *)&ringLog.crc,sizeof(ringLog.crc));
	logPositionCnt += sizeof(ringLog.crc);
	if(logPositionCnt >= 4095)
	{
		SST25_Erase(LOG_POSITION_ADDR,block4k);
		logPositionCnt = 0;
	}
		
}

int8_t DB_LoadLog(MSG_STATUS_RECORD *log)
{
	uint8_t u8temp;
	
	if(ringLog.tail >= ringLog.head) return 0xff;
	
	SST25_Read(LOG_DATABASE_ADDR + ringLog.tail*64,(uint8_t *)log,sizeof(MSG_STATUS_RECORD));
	u8temp = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1);
	ringLog.tail++;
	DB_RingLogSave();
	if(log->crc == u8temp)
	{
		return 0;
	}
	else 
	{
		return 0xff;
	}
}

int8_t DB_SaveLog(MSG_STATUS_RECORD *log)
{
	uint8_t buff[64];
	uint32_t u32temp,tailSector = 0,headSector = 0;
	tailSector = ringLog.tail*64 / 4096;
	u32temp = ringLog.head;
	u32temp++;
	if(u32temp >= 12800)	
	{
		u32temp = 0;
	}
	headSector = u32temp*64/4096;
	if((headSector == tailSector) && (u32temp <= ringLog.tail))
	{
		return 0xff;
	}
	else
	{
		log->crc = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1);
		memcpy((char *)buff,(char *)log,sizeof(MSG_STATUS_RECORD));
		SST25_Write(LOG_DATABASE_ADDR + ringLog.head*64,buff,64);
		ringLog.head = u32temp;
		DB_RingLogSave();
	}
	return 0;
}

uint8_t DbCalcCheckSum(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}
