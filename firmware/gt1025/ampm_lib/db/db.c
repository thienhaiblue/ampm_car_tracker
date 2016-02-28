/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "db.h"
#include "sst25.h"
#include "lib/sys_tick.h"

#define DATABASE_DBG(...)		//DbgCfgPrintf(__VA_ARGS__)

#define MAX_DISK_TRY		5


#define LOG_WRITE_BYTES 4
#define RINGLOG_MAX 	(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))
#define DB_U32_MAX_SIZE 	(4096 / sizeof(DB_U32))

extern DATE_TIME sysTime;

MSG_STATUS_RECORD	logRecord,newestLog;
LOG_RING_TYPE ringLog;
uint32_t flagNewLog = 1;
uint32_t flashInUse = 0;


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

uint16_t DbCalcCheckSum16(uint8_t *buff, uint16_t length)
{
	uint32_t i;
	uint16_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

void DB_InitLog(MSG_STATUS_RECORD *log)
{
	uint16_t u16temp;
	
	int32_t i;
	LOG_RING_TYPE logTemp;
	ringLog.head = 0;
	ringLog.tail = 0;
	ringLog.cnt = 0;
	ringLog.crc = 0;
	memset((uint8_t *)log,0,sizeof(MSG_STATUS_RECORD));
	if(sizeof(MSG_STATUS_RECORD) % LOG_WRITE_BYTES)
		while(1); //khai bao lai kieu bien
	for(i = 0;i < 4096;i += sizeof(LOG_RING_TYPE))
	{
		SST25_Read(LOG_POSITION_ADDR + i,(uint8_t *)&logTemp, sizeof(LOG_RING_TYPE));
		u16temp = DbCalcCheckSum16((uint8_t *)&logTemp,sizeof(LOG_RING_TYPE) - 2);
		if(u16temp == logTemp.crc
			&& logTemp.head < RINGLOG_MAX
			&& logTemp.tail < RINGLOG_MAX
			&& (logTemp.cnt == i)
		)
		{
			ringLog = logTemp;
			ringLog.cnt += sizeof(LOG_RING_TYPE);
		}
		else{
		 break;
		}
	}
	DB_LoadEndLog(log);
	if(DB_ParaCheckErr(log))
		memset((uint8_t *)log,0,sizeof(MSG_STATUS_RECORD));
	flashInUse = 0;
}


uint32_t DB_FloatToU32(double lf)
{
	uint32_t *u32pt;
	float f = lf,*tPt;
	tPt = &f;
	u32pt = (uint32_t *)tPt;
	return *u32pt;
}

float DB_U32ToFloat(uint32_t *u32)
{
	float *fpt;
	fpt = (float *)u32;
	return *fpt;
}

void DB_U32Load(DB_U32 *dbu32,uint32_t addr)
{
	int32_t i;
	DB_U32 db32_temp;
	uint16_t u16temp;
	flashInUse = 1;
	
	dbu32->oldValue = 0;
	dbu32->value = 0;
	dbu32->cnt = 0;
	dbu32->crc = 0;
	for(i = 0;i < DB_U32_MAX_SIZE*sizeof(DB_U32);i += sizeof(DB_U32))
	{
		SST25_Read(addr + i,(uint8_t *)&db32_temp, sizeof(DB_U32));
		u16temp = DbCalcCheckSum16((uint8_t *)&db32_temp, sizeof(DB_U32) - 2);
		if(u16temp == db32_temp.crc
			&& (db32_temp.cnt == i)
		)
		{
			*dbu32 = db32_temp;
			dbu32->cnt += sizeof(DB_U32);
		}
		else{
		 break;
		}
	}

	flashInUse = 0;
}

void DB_U32Save(DB_U32 *dbu32,uint32_t addr)
{
	DB_U32 dbu32T;
	flashInUse = 1; 
	if(dbu32->oldValue != dbu32->value)
	{
		dbu32->oldValue = dbu32->value;
		dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
		while(1)
		{
			if((dbu32->cnt +  sizeof(DB_U32)) >= (DB_U32_MAX_SIZE*sizeof(DB_U32)))
			{
					SST25_Erase(addr,block4k);
					dbu32->cnt = 0;
					dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
			}
			SST25_Write(addr + dbu32->cnt,(uint8_t *)dbu32,sizeof(DB_U32));
			SST25_Read(addr + dbu32->cnt,(uint8_t *)&dbu32T,sizeof(DB_U32));
			if(memcmp(&dbu32T, dbu32,sizeof(DB_U32)) != NULL)
			{
				SST25_Erase(addr,block4k);
				dbu32->cnt = 0;
				dbu32->crc = DbCalcCheckSum16((uint8_t *)dbu32,sizeof(DB_U32) - 2);
			}
			else
			{
				dbu32->cnt += sizeof(DB_U32);
				break;
			}
		}
	}
	flashInUse = 0;
}


void DB_RingLogReset(void)
{
	flashInUse = 1;
	SST25_Erase(LOG_POSITION_ADDR,block4k);
	ringLog.head = 0;
	ringLog.tail = 0;
	DB_RingLogSave();
	flashInUse = 0; 
}

void DB_RingLogNext(void)
{
	flashInUse = 1;
	if(ringLog.tail != ringLog.head)
	{
		ringLog.tail++;
		if(ringLog.tail >= RINGLOG_MAX)
			ringLog.tail = 0;
		DB_RingLogSave();
	}
	flashInUse = 0;
}



void DB_RingLogSave(void)
{
	LOG_RING_TYPE ringLogT;
	flashInUse = 1; 
	ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
	while(1)
	{
		if((ringLog.cnt +  sizeof(LOG_RING_TYPE) >=  4096) )
		{
			SST25_Erase(LOG_POSITION_ADDR,block4k);
			ringLog.cnt = 0;
			ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
		}
		SST25_Write(LOG_POSITION_ADDR + ringLog.cnt,(uint8_t *)&ringLog,sizeof(LOG_RING_TYPE));
		SST25_Read(LOG_POSITION_ADDR + ringLog.cnt,(uint8_t *)&ringLogT,sizeof(LOG_RING_TYPE));
		if(memcmp(&ringLogT, &ringLog,sizeof(LOG_RING_TYPE)) != NULL)
		{
			SST25_Erase(LOG_POSITION_ADDR,block4k);
			ringLog.cnt = 0;
			ringLog.crc = DbCalcCheckSum16((uint8_t *)&ringLog,sizeof(LOG_RING_TYPE) - 2);
		}
		else
		{
			ringLog.cnt += sizeof(LOG_RING_TYPE);
			break;
		}
	}
	flashInUse = 0;
}


uint32_t DB_LogFill(void)
{
	if(ringLog.head >= ringLog.tail)
	{
		return (ringLog.head - ringLog.tail);
	}
	else
	{
	   return(LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD) - ringLog.tail + ringLog.head);
	}
}

int8_t DB_LoadEndLog(MSG_STATUS_RECORD *log)
{

	uint32_t head = ringLog.head;
	MSG_STATUS_RECORD tLog;
	flashInUse = 1;
	while(1)
	{
		if(head == 0)
		{
			head = (LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD)) - 1;
		}
		else
		{
			head--;
		}
		if(head == ringLog.tail) break;
		SST25_Read(LOG_DATABASE_ADDR + head*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
		if(!DB_ParaCheckErr(&tLog))
		{
			*log = tLog;
			flashInUse = 0;
			return 0;
		}
	}
	flashInUse = 0;
	return 1;
}

uint8_t DB_ParaCheckErr(MSG_STATUS_RECORD *log)
{
//	DATE_TIME time;
//	time.hour = 0;
//	time.min = 0;
//	time.sec = 0;
//	time.mday = 19;
//	time.month = 3;
//	time.year = 2014;
//	if(log->time > TIME_GetSec(&time)
//	&& log->gpsLat > -90
//	&& log->gpsLat < 90
//	&& log->gpsLon > -180
//	&& log->gpsLon < 180
//	)
	if(1)
	{
		if(log->crc == DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1))
			return 0;
	}
	return 1;
}

int8_t DB_LoadLog(MSG_STATUS_RECORD *log,LOG_RING_TYPE *r)
{
	MSG_STATUS_RECORD tLog;
	if(r->tail == r->head) return 0xff;
	SST25_Read(LOG_DATABASE_ADDR + r->tail*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
	if(!DB_ParaCheckErr(&tLog))
	{
		*log = tLog;
		return 0;
	}
	else 
		return 0xff;
}

int8_t DB_LoadNextLog(MSG_STATUS_RECORD *log)
{
	uint32_t tail = ringLog.tail;
	flashInUse = 1;
	if(ringLog.tail == ringLog.head){
		flashInUse = 0;
		return 0xff;
	}
	while(DB_LoadLog(log,&ringLog) != 0)
	{
		ringLog.tail++;
		if(ringLog.tail >= RINGLOG_MAX)
			ringLog.tail = 0;
		if(ringLog.tail == ringLog.head){
			flashInUse = 0;
			return 0xff;
		}
	}
	if(tail != ringLog.tail)
		DB_RingLogSave();
	flashInUse = 0;
	return 0;
}

int8_t DB_SaveLog(MSG_STATUS_RECORD *log)
{
	MSG_STATUS_RECORD tLog;
	uint8_t tryCnt = 0,err = 0;
	uint32_t u32temp,tailSector = 0,headSector = 0,headSectorOld = 0,i;
	flashInUse = 1;
	while(1)
	{
		tryCnt++;
		if(tryCnt >= 10){
			flashInUse = 0;
			return 1;
		}
		tailSector = ringLog.tail * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		headSectorOld = ringLog.head * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		u32temp = ringLog.head;
		u32temp++;
		if(u32temp >= LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))	
		{
			u32temp = 0;
		}
		headSector = u32temp * sizeof(MSG_STATUS_RECORD) / SST25_SECTOR_SIZE;
		if(headSector != headSectorOld)
		{
			SST25_Erase(LOG_DATABASE_ADDR + headSector * SST25_SECTOR_SIZE,block4k);
		}
		if((headSector == tailSector) && (u32temp <= ringLog.tail))
		{
			tailSector++;
			ringLog.tail = tailSector * SST25_SECTOR_SIZE / sizeof(MSG_STATUS_RECORD);
			if((tailSector * SST25_SECTOR_SIZE) % sizeof(MSG_STATUS_RECORD))
				 ringLog.tail++;
			if(ringLog.tail >= LOG_DATA_SIZE_MAX / sizeof(MSG_STATUS_RECORD))
				ringLog.tail = 0;
		}

		log->crc = DbCalcCheckSum((uint8_t *)log,sizeof(MSG_STATUS_RECORD) - 1);
		
		for(i = 0;i < sizeof(MSG_STATUS_RECORD);i += LOG_WRITE_BYTES)
		{
			if(SST25_Write(LOG_DATABASE_ADDR + ringLog.head*sizeof(MSG_STATUS_RECORD) + i,(uint8_t *)log + i,LOG_WRITE_BYTES) == SST25_FAIL){
				err = 1;
				break;
			}
		}
		if(!err){
			SST25_Read(LOG_DATABASE_ADDR + ringLog.head*sizeof(MSG_STATUS_RECORD),(uint8_t *)&tLog,sizeof(MSG_STATUS_RECORD));
			ringLog.head = u32temp;
			if(!DB_ParaCheckErr(&tLog))
			{
				DB_RingLogSave();
				flashInUse = 0;
				return 0;
			}
		}
		else{
			err = 0;
			ringLog.head = u32temp;
		}
	}
	flashInUse = 0;
	return 1;
}



