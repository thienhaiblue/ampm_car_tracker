#include "packets.h"
#include "system_config.h"
#include "gps.h" 
#include "packet_queue.h"
#include "sys_time.h"

#define  PACKET_TYPE_GPS_INFO			0x00

uint8_t trackerBuff[512];
RINGBUF trackerRb;

PACKET_GPS_INFO pkGpsInfo;

void CreatePacket(void)
{
	static uint32_t ringBufInitFlag = 0;
	if(!ringBufInitFlag)
	{
		ringBufInitFlag = 1;
		RINGBUF_Init(&trackerRb,trackerBuff,sizeof(trackerBuff));
	}
	pkGpsInfo.size = sizeof(PACKET_GPS_INFO);
	pkGpsInfo.type = PACKET_TYPE_GPS_INFO;
	memcpy(pkGpsInfo.id,sysCfg.imei,sizeof(sysCfg.imei));
	pkGpsInfo.year = sysTime.year - 2000;
	pkGpsInfo.month = sysTime.month;
	pkGpsInfo.mday = sysTime.mday;
	pkGpsInfo.hour = sysTime.hour;
	pkGpsInfo.minute = sysTime.min;
	pkGpsInfo.sec = sysTime.sec;
	
	pkGpsInfo.latitude = lastGpsInfo.lat;
	pkGpsInfo.longitude = lastGpsInfo.lon;
	pkGpsInfo.altitude = lastGpsInfo.elv;
	pkGpsInfo.speed = lastGpsInfo.speed;
	pkGpsInfo.heading = lastGpsInfo.direction;
	pkGpsInfo.mileage = 0;
	
	pkGpsInfo.status = 0;

	pkGpsInfo.sig = gpsInfo.sig;
	pkGpsInfo.fix = gpsInfo.fix;
	pkGpsInfo.hdop = gpsInfo.HDOP;
	pkGpsInfo.satInUse = gpsInfo.satinfo.inuse;
	pkGpsInfo.satInView = gpsInfo.satinfo.inview;

	pkGpsInfo.analog1 = 0;
	
	pkGpsInfo.analog2 = 0;
	
	pkGpsInfo.pulse1 = 0;
	pkGpsInfo.pulse2 = 0;
	PQUEUE_Put(&trackerRb,(uint8_t *)&pkGpsInfo);
	
}


