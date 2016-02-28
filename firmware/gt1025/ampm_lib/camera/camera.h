#ifndef __CAMERA_H__
#define __CAMERA_H__
#include <stdint.h>
#include "db.h"

extern uint32_t pictureTime;
extern uint8_t cameraID;
extern uint8_t cameraSnapShotCnt;
#define CAMERA_DATA_SIZE	512

typedef struct __attribute__((packed)){
	MSG_STATUS_RECORD record;
	
	uint32_t fileSize;			// file size
	uint32_t offset;				// offset of the chunk
	uint32_t len;				// size of the chunk
	uint8_t data[CAMERA_DATA_SIZE];	// data chunk
	uint32_t crc;
}CAMERA_PAKET_TYPE;


extern uint8_t cameraShotFlag;
extern CAMERA_PAKET_TYPE cameraJpegFile;
uint32_t CAM_crc(uint8_t *buff, uint32_t length);
void CAM_Init(void);
void CAM_Input(uint8_t c);
void CAM_Manage(void);
uint8_t CAM_IsBusy(void);
void CAM_ChangeId(uint8_t id, uint8_t newId);
uint8_t CAM_TakeSnapshot(uint16_t cameras);
void CAM_Stop(void);
void CAM_Start(void);

#endif
