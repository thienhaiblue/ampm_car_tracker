#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <time.h>
#include "gps.h"




#define TIME_ZONE			(+7)

typedef struct __attribute__((packed)){
	int16_t year;			// year with all 4-digit (2011)
	int8_t month;			// month 1 - 12 (1 = Jan)
	int8_t mday;			// day of month 1 - 31
	int8_t wday;			// day of week 1 - 7 (1 = Sunday)
	int8_t hour;			// hour 0 - 23
	int8_t min;				// min 0 - 59
	int8_t sec;				// sec 0 - 59
}DATE_TIME;

extern DATE_TIME sysTime;

int8_t TIME_AddSec(DATE_TIME *t, int32_t sec);
int32_t TIME_GetSec(DATE_TIME *t);
int8_t TIME_FromSec(DATE_TIME *t, int32_t sec);
int8_t TIME_FromGps(DATE_TIME *t, nmeaINFO *nmeaInfo);
int8_t TIME_FromGsm(DATE_TIME *t, DATE_TIME *time);


#endif

