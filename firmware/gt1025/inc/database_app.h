
#ifndef __DATABASE_APP_H__
#define __DATABASE_APP_H__
#include <stdint.h>
#include "db.h"





extern DB_U32 currentTimeSave;
extern DB_U32 beginTimeSave;
extern DB_U32 endTimeSave;
extern DB_U32 gpsLatSave;
extern DB_U32 gpsLonSave;
extern DB_U32 mileageSave;
extern DB_U32 drivingTimeSave;
extern DB_U32 parkingTimeSave;
extern DB_U32 overDrivingTimeCntSave_totalDrivingTimeCntSave;
extern DB_U32 totalDrivingTimeSave;
extern DB_U32 overSpeedCntSave_openDoorCntSave;
extern DB_U32 closeDoorCntSave_onEngineCntSave;
extern DB_U32 offEngineCntSave_parkCntSave;
extern DB_U32 statusSave_crcSave;

extern DB_U32 gpsHDOPSave;
extern DB_U32 gpsDirSave;

extern DB_U32 timeSave;

typedef struct __attribute__((packed))
{
	int8_t hour;			// hour 0 - 23
	int8_t min;				// min 0 - 59
	int8_t sec;				// sec 0 - 59
}RECORD_TIME;

typedef struct __attribute__((packed)){
	RECORD_TIME currentTime;
	RECORD_TIME beginTime;
	RECORD_TIME endTime;
	double lat;
	double lon;
	double speed;
	double mileage;
	uint32_t drivingTime;
	uint32_t parkingTime;
	uint16_t overDrivingTimeCnt;
	uint32_t totalDrivingTime;
	uint16_t totalDrivingTimeCnt;
	uint16_t overSpeedCnt;
	uint16_t openDoorCnt;
	uint16_t closeDoorCnt;
	uint16_t onEngineCnt;
	uint16_t offEngineCnt;
	uint16_t parkCnt;
	uint8_t status;
	uint8_t crc;
}SPEED_RECORD;

uint32_t DB_SaveAll(void);
uint32_t DB_FlashMemInit(void);
char DB_Init(void);
char DB_CreateHierarchy(void);
char DB_SaveSpeed(SPEED_RECORD *r);
char GetLastRecord(SPEED_RECORD *r,DATE_TIME *current,uint8_t driverIndex);
char DB_RemoveDir(char *name);
char DB_Print(DATE_TIME *time,uint8_t printNo);
void DB_SaveSpeed_Pause(void);
void DB_SaveSpeed_Continue(void);

#endif

