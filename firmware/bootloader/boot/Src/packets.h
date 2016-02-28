#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>
#include "system_config.h"
#include "ringbuf.h"

typedef struct __attribute__((packed)){
	uint16_t size;					// = sizeof (PACKET_GPS_INFO)
	uint8_t type;					// = 0x00
	uint8_t year;					// year since 2000
	uint8_t month;					// 1 - 12
	uint8_t mday;					// 1 - 31
	uint8_t hour;					// 0 - 23
	uint8_t minute;					// 0 - 59
	uint8_t sec;						// 0 - 59
	uint8_t status;					// bit0: D0 status: 1/0 = engine ON/OFF
								// bit1: D1 status: 1/0 = air conditioner ON/OFF
								// bit2: D2 status: 1/0 = door CLOSED/OPENED
								// bit3: D3 status: 
								// bit4: abrupt acceleration
								// bit5: abrupt braking
								// bit7-6: 	10 - on main power, backup battery full (or no battery present)
								//			11 - on mainpower, backup battery charging
								//			00 - on backup battery, main power lost
								//			01 - on backup battery, main power lost, battery low
	float latitude;				// (-180 - +180)
	float longitude;			// (-90 - +90)
	float altitude;
	float speed;				// km/h
	float heading;				// 0 - 360
	float mileage;
	uint16_t analog1;				// A1: 0 - 4095
	uint16_t analog2;				// A2: 0 - 4095
	uint16_t pulse1;					// P1: 0 - 4095
	uint16_t pulse2;					// P2: 0 - 4095
	
	int8_t id[17];					// tracker id
	int8_t dummy;					// do not use
	
	int16_t lastError;				// last error code
	uint16_t snapshotMap;
	
	float hdop;					//hdop
	uint8_t sig;
	uint8_t fix;
	uint8_t satInUse;
	uint8_t satInView;
	
}PACKET_GPS_INFO;
extern uint8_t trackerBuff[512];
extern RINGBUF trackerRb;
void CreatePacket(void);


#endif
