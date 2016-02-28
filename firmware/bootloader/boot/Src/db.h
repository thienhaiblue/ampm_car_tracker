#ifndef __DB_H__
#define __DB_H__

#include <stdint.h>
#include "sys_time.h"
#include "system_config.h"


/*
* log db layout:
* year_month/mday/hour.log		speed log every second
*/

typedef struct __attribute__((packed)){
	DATE_TIME currentTime; //8 thoi gian hien tai
	float lat;//12 kinh do
	float lon;//16 vi do
	float hdop;//20 sai so trong don vi met
	float speed;//24 toc do
	float dir;//28 huong di chuyen
	float mileage; //32 so km hien thi tren dong ho do cua xe
	float speedFreq;//36 tan so (dung cho cam bien xang)
	uint32_t drivingTime; //38  thoi gian lai xe lien tuc
	uint32_t totalDrivingTime;//42 tong thoi gian lai xe trong ngay
	uint16_t IOStatus;//46  Trang thai cua cac IO nhu DOOR, AIRCON,Engine
	uint16_t adc1Value;//48 Gia tri cua cam bien 1
	uint16_t adc2Value;//50 Gia tri cua cam bien 2
	uint16_t overSpeedCnt;//52 Toc do gioi han cua xe
	uint16_t parkCnt;//54 so dan dung do xe
	uint16_t openDoorCnt;//56 so lan dong mo cua xe
	uint16_t onOffEngineCnt;//58 so lan tat mo may xe
	uint16_t parkingTime;//60 thoi gian dau xe 
	uint8_t drvIndex;//61 ID cua tai xe
	uint8_t ew;//62 huong 'E' hoac "W'
	uint8_t ns;//63 huong 'N' hoac 'S'
	uint8_t crc;//64
}MSG_STATUS_RECORD;



typedef struct __attribute__((packed))
{
	uint32_t sysResetTimes;
	uint32_t sysHaltFailTimes;
}SYSTEM_STATUS_TYPE;

extern uint32_t resetNum;
extern float mileage;

uint8_t DbCalcCheckSum(uint8_t *buff, uint32_t length);

//flash log
void DB_InitLog(void);
int8_t DB_SaveLog(MSG_STATUS_RECORD *log);
int8_t DB_LoadLog(MSG_STATUS_RECORD *log);
void DB_RingLogReset(void);
void DB_RingLogSave(void);
void DB_ResetCntLoad(void);
void DB_ResetCntSave(void);

//mileage
void DB_MileageLoad(void);
void DB_MileageSave(void);

#endif

