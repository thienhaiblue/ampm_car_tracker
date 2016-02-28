#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <stdint.h>
#include "ringbuf.h"


#define	M_PI		3.14159265358979323846	/* pi */
#define d2r (M_PI / 180.0)


typedef struct {
	int din1:1;
	int din2:1;
	int din3:1;
	int din4:1;
	int overSpeed:1;
	volatile int newDriver:1;
	volatile int newDriverSub:1;
	volatile int idle:1;
}TrackerStatusType;

extern TrackerStatusType  trackerStatus;

extern uint32_t serverSendDataFlag;
extern RINGBUF trackerRb;

void TrackerInit(void);
void TrackerTask(void);
uint16_t CalculateCRC(const int8_t* buf, int16_t len);
float haversine_km(float lat1, float long1, float lat2, float long2);

#endif

