/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "camera.h"
#include "system_config.h"
#include "lib/sys_tick.h"
#include "uart3.h"
#include "lib/sys_time.h"
#include "hw_config.h"
#include "ff.h"
#include "diskio.h"
#include "adc_task.h"
#include "app_config_task.h"
#define INFO(...)	DbgCfgPrintf(__VA_ARGS__)


#define RS485_DIR		
#define CAM_TIME_OUT	SYSTICK_SECOND * 3
#define CAM_INTERVAL	SYSTICK_SECOND * sysCfg.cameraInterval * 60
#define CAM_DELAYED_START	TICK_SECOND * 10
#define CAM_MAX_GET_DATA_RETRY		10
#define CAM_MAX_SNAPSHOT_RETRY		10

uint32_t pictureTime = 0;
uint8_t cameraID = 0;
uint8_t cameraSnapShotCnt = 0;

typedef struct{
	uint32_t time;
	uint32_t size;
	uint16_t packetCount;
}SNAPSHOT_INFO;

SNAPSHOT_INFO ssInfo[4];

uint8_t cmdSetCompression[] = {'U', 'Q', 
						0,		/* camera ID */
						200,	/* compression value, higher value, lower quality */
						'#'};
						
uint8_t cmdSnapshot[] = {'U', 'H', 
						0, 		/* camera ID */
						'3',	/* resolution = 640x480 */
						0, 2, 	/* packet size = 512 */
						'#'};

uint8_t cmdGetPacket[] = {'U', 'E', 
						0, 		/* camera ID */
						1, 0, 	/* packet ID */
						'#'};

uint8_t cmdSetBaudRate[] = {'U', 'I', 
							0, 		/* camera ID */
						'0', 	/* baud 9600 */
						'#'};
						
uint8_t cmdChangeId[] = {'U', 'D',
					0xff,			/* current ID */
					0,			/* new ID */
					'#'};

volatile enum{
	WAIT_HEADER,
	GET_CMD,
	GET_CAM_ID,
	GET_CMD_DATA,
	GET_IMG_PKG_ID0,
	GET_IMG_PKG_ID1,
	GET_IMG_DATA_LEN0,
	GET_IMG_DATA_LEN1,
	GET_IMG_DATA,
	GET_IMG_CRC0,
	GET_IMG_CRC1
}camState;

volatile enum{
	INITIAL,
	TAKE_SNAPSHOT,
	WAIT_RESPONSE,
	GET_PACKET,
	WAIT_PACKET,
	GOT_PACKET,
	WAIT_ENQUEUED,
	NEXT_CAMERA,
}camManState;


uint8_t cameraShotFlag = 0;
uint8_t camCurrentId = 0;
uint8_t camRespondedCmd = 0;
uint8_t camRespondedId = 0;
//uint8_t camData[CAMERA_DATA_SIZE];
uint16_t camRespondedDataLen,camRespondedCrc;
uint16_t camSnapshotMap;
uint8_t camStopFlag = 0;

volatile uint16_t camRespondedPacketId;
volatile uint16_t camRespondedPacketLen;
volatile uint8_t camAcked;
volatile uint8_t camGetDataRetryCnt;
volatile uint32_t camTick;
volatile uint32_t camIntervalTick;
extern float mileage;
extern float csqValue;
static void SendCmd(uint8_t *buf, uint16_t len);
static void ProcessCmd(void);

Timeout_Type tCamSnapShot;

CAMERA_PAKET_TYPE cameraJpegFile;

DATE_TIME camTime;

uint32_t CAM_crc(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint32_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

uint16_t CAM_crc16(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint16_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

void CAM_Init(void)
{
	uint8_t i;
	//RS485_DIR = 0;
	camCurrentId = 0;
	
	#if CONFIG_USE_ID_IN_PACKETS == 1
	strcpy((char *)cameraJpegFile.id, (char *)sysCfg.imei);
	#endif
	
	cmdSnapshot[0] = 'U';
	cmdSnapshot[1] = 'H';
	cmdSnapshot[3] = '3';
	cmdSnapshot[4] = 0;
	cmdSnapshot[5] = 2;
	cmdSnapshot[6] = '#';
	
	cmdGetPacket[0] = 'U';
	cmdGetPacket[1] = 'E';
	cmdGetPacket[3] = 1;
	cmdGetPacket[4] = 0;
	cmdGetPacket[5] = '#';
	
	cmdChangeId[0] = 'U';
	cmdChangeId[1] = 'D';
	cmdChangeId[2] = 0xff;
	cmdChangeId[3] = 0;
	cmdChangeId[4] = '#';
	SysTick_DelayMs(1000);

	cmdSetCompression[0] = 'U';
	cmdSetCompression[1] = 'Q';
	cmdSetCompression[3] = 100;
	cmdSetCompression[4] = '#';
	for(i=0;i<sysCfg.numCameras;i++)
	{
		cmdSetCompression[2] = i;
		SendCmd(cmdSetCompression, sizeof cmdSetCompression);
		SysTick_DelayMs(1000);
	}
	InitTimeout(&tCamSnapShot,SYSTICK_TIME_SEC(30));
	camTick = SysTick_Get();
	sysCfg.numCameras = 1;
}
uint8_t CAM_TakeSnapshot(uint16_t cameras)
{
	if(camManState != INITIAL) return 0xff;
	
	INFO("CAM: Snapshotting\r\n");
	memset(ssInfo, 0, sizeof ssInfo);
	camSnapshotMap = cameras;
		
	memset(&cameraJpegFile,0,sizeof(CAMERA_PAKET_TYPE));

	cameraJpegFile.record.time = rtcTimeSec;
	cameraJpegFile.record.cameraID = cameras + 1;
	cameraJpegFile.record.time = TIME_GetSec(&sysTime);
	cameraJpegFile.record.adc1Value = (uint16_t)(ADC1_10_Value*1000);
	cameraJpegFile.record.adc2Value = (uint16_t)(ADC1_11_Value*1000);
	cameraJpegFile.record.gpsLat = lastNmeaInfo.lat;
	cameraJpegFile.record.gpsLon = lastNmeaInfo.lon;
	cameraJpegFile.record.gpsHdop = lastNmeaInfo.HDOP;
	cameraJpegFile.record.gpsSpeed = nmeaInfo.speed;
	cameraJpegFile.record.gpsDir = lastNmeaInfo.direction;
	cameraJpegFile.record.mileage = mileage;
	cameraJpegFile.record.batteryPercent = ADC1_11_Value*100/4.2;
	cameraJpegFile.record.gsmSignal = (uint8_t)csqValue;

	cameraJpegFile.record.IOStatus = 0;
	cameraJpegFile.record.satinuse = lastNmeaInfo.satinfo.inuse;
	cameraJpegFile.record.IOStatus = 0;
	if(GET_AVLIO1_PIN)
		cameraJpegFile.record.IOStatus |= 1<<0; //ACC
	if(GET_AVLIO2_PIN)
		cameraJpegFile.record.IOStatus |= 1<<1;
	if(GET_AVLIO3_PIN)
		cameraJpegFile.record.IOStatus |= 1<<2; 
	if(GET_AVLIO4_PIN)
		cameraJpegFile.record.IOStatus |= 1<<3; 


	cameraJpegFile.fileSize = 0;			// file size
	cameraJpegFile.offset = 0;				// offset of the chunk
	cameraJpegFile.len = 0;				// size of the chunk
	
	camManState = TAKE_SNAPSHOT;
	return 0;
}

uint8_t CAM_IsBusy(void)
{
	if(camManState != INITIAL)
		return 1;
	return 0;
}
uint32_t camTimeCmt = 0;
uint32_t fre_sect;
uint8_t f_close_flag = 0;
FIL camFile;
void CAM_Manage(void)
{	
	
	FATFS *fs;
	char buf[256];
	DWORD fre_clust;
	uint32_t i,j;
	FRESULT res;
	DATE_TIME t;
	
	if(camStopFlag == 0){
		camTimeCmt++;
		switch(camManState){
			case INITIAL:
				if(f_close_flag)
				{
					f_close_flag = 0;
					f_close(&camFile);
				}
//				if(CheckTimeout(&tCamSnapShot) == SYSTICK_TIMEOUT){
//					InitTimeout(&tCamSnapShot,SYSTICK_TIME_SEC(60));
//					if(CAM_TakeSnapshot(0xff) == 0){
//						cameraShotFlag = 0;
//					}
//				}
				break;
			case TAKE_SNAPSHOT:
				if(camCurrentId >= sysCfg.numCameras){
					camManState = INITIAL;
					break;
				}
				
				if(!((1<<camCurrentId) & camSnapshotMap)){
					camManState = WAIT_RESPONSE;
					break;
				}
				
				cmdSnapshot[2] = camCurrentId;
				SendCmd(cmdSnapshot, sizeof cmdSnapshot);
				
				camTick = SysTick_Get();
				camState = WAIT_HEADER;
				camManState = WAIT_RESPONSE;
				break;
			
			case WAIT_RESPONSE:
				if(SysTick_Get() - camTick >= SYSTICK_SECOND){
					if(++camCurrentId >= sysCfg.numCameras){
						camCurrentId = 0;
						cmdGetPacket[3] = 1;
						camManState = GET_PACKET;
					}else{
						camManState = TAKE_SNAPSHOT;
					}
				}
				break;
				
			case GET_PACKET:
				if(ssInfo[camCurrentId].size == 0){
					camManState = NEXT_CAMERA;
					break;
				}
				
				cmdGetPacket[2] = camCurrentId;
				SendCmd(cmdGetPacket, sizeof cmdGetPacket);
				camTick = SysTick_Get();
				camManState = WAIT_PACKET;
				break;
			
			case WAIT_PACKET:
				if(SysTick_Get() - camTick > CAM_TIME_OUT){
					if(++camGetDataRetryCnt >= CAM_MAX_GET_DATA_RETRY){
						camManState = NEXT_CAMERA;
					}else{
						INFO("CAM: GET_PACKET timedout %d\r\n", camGetDataRetryCnt);
						camManState = GET_PACKET;
					}
				}
				break;
				
			case GOT_PACKET:
			//	f_close_flag = 0;
				//YYMMDDHHMMSS
				
				//sprintf(buf,"IMAGE/%d_%d.jpg",cameraJpegFile.record.cameraID,cameraJpegFile.record.time);
				if(cameraJpegFile.offset < 512){
					TIME_FromSec(&camTime,cameraJpegFile.record.time); 
					sprintf(buf, "IMAGE/%d_%02d%02d%02d%02d%02d%02d.jpg",
					cameraJpegFile.record.cameraID,
					camTime.year%100,
					camTime.month,
					camTime.mday,
					camTime.hour, 
					camTime.min,
					 camTime.sec);
					res = f_getfree("0:", &fre_clust, &fs);
					if(res== FR_OK){
						fre_sect = fre_clust * fs->csize; 
						if(fre_sect * _MAX_SS > cameraJpegFile.fileSize + 10240/*10k*/){
							res = f_open(&camFile,buf, FA_WRITE | FA_CREATE_ALWAYS);
							f_close_flag = 1;
						}
					}
				}

				if(cameraJpegFile.offset >= f_size(&camFile) && f_close_flag)
				{
					if(f_lseek(&camFile,cameraJpegFile.offset) == FR_OK)
					{
						f_write(&camFile, cameraJpegFile.data,cameraJpegFile.len, &i);
					}
				}
				

				camGetDataRetryCnt = 0;
				cmdGetPacket[3]++;
				camManState = WAIT_ENQUEUED;
				break;
				
			case WAIT_ENQUEUED:
				if(camRespondedPacketId >= ssInfo[camCurrentId].packetCount){
					if(f_close_flag)
					{
						f_close_flag = 0;
						f_close(&camFile);
					}
					//had a picture already
					camManState = NEXT_CAMERA;
					if(fre_sect * _MAX_SS > cameraJpegFile.fileSize + 10240/*10k*/){
						//sprintf(buf,"IMAGE/%d_%d.inf",cameraJpegFile.record.cameraID,cameraJpegFile.record.time);
						sprintf(buf, "IMAGE/%d_%02d%02d%02d%02d%02d%02d.inf",
						cameraJpegFile.record.cameraID,
						camTime.year%100,
						camTime.month,
						camTime.mday,
						camTime.hour, 
						camTime.min,
						 camTime.sec);
						if(f_open(&camFile,buf, FA_WRITE | FA_CREATE_ALWAYS) ==FR_OK)
						{
							cameraJpegFile.record.crc = DbCalcCheckSum((uint8_t *)&cameraJpegFile.record,sizeof(MSG_STATUS_RECORD) - 1);
							f_write(&camFile,&cameraJpegFile.record,sizeof(MSG_STATUS_RECORD), &j);
						}
						f_close(&camFile);
					}
					cameraJpegFile.record.time = rtcTimeSec;
				}
				else
					camManState = GET_PACKET;
				break;
				
			case NEXT_CAMERA:
				if(cameraSnapShotCnt) cameraSnapShotCnt--;
				if(f_close_flag)
					{
						f_close_flag = 0;
						f_close(&camFile);
					}
				if(++camCurrentId >= sysCfg.numCameras){
					camCurrentId = 0;
					camManState = INITIAL;
				}else{
					cmdGetPacket[2] = camCurrentId;
					cmdGetPacket[3] = 1;
					camManState = GET_PACKET;
				}
				camGetDataRetryCnt = 0;
				break;
				
			default:
				break;
		}
	}
}

void CAM_Stop(void)
{
	camStopFlag = 1;
}

void CAM_Start(void)
{
	camStopFlag = 0;
}

void CAM_ChangeId(uint8_t id, uint8_t newId)
{
	INFO("CAM: Change ID from %d to %d\r\n", id, newId);
	cmdChangeId[2] = id;
	cmdChangeId[3] = newId;
	SendCmd(cmdChangeId, sizeof cmdChangeId);
	SysTick_DelayMs(1000);
	SendCmd(cmdChangeId, sizeof cmdChangeId);
	SysTick_DelayMs(1000);
}
void CAM_Input(uint8_t c)
{
	static uint16_t CamCrc = 0;
	if(camStopFlag == 0)
	switch(camState){
		case WAIT_HEADER:
			if(c == 'U')
			{
				CamCrc = c;
				camState = GET_CMD;
			}
			break;
			
		case GET_CMD:
			CamCrc += c;
			camRespondedCmd = c;
			camRespondedDataLen = 0;
			camState = GET_CAM_ID;
			break;
			
		case GET_CAM_ID:
			CamCrc += c;
			camRespondedId = c;
			// image data
			if(camRespondedCmd == 'F'){
				camState = GET_IMG_PKG_ID0;
			}else{
				camRespondedDataLen = 0;
				camState = GET_CMD_DATA;
			}
			break;
			
		case GET_CMD_DATA:
			if(c == '#'){
				ProcessCmd();
				camState = WAIT_HEADER;
			}else{
				cameraJpegFile.data[camRespondedDataLen++] = c;
			}
			break;
			
		case GET_IMG_PKG_ID0:
			CamCrc += c;
			camRespondedPacketId = c;
			camState = GET_IMG_PKG_ID1;
			break;
			
		case GET_IMG_PKG_ID1:
			CamCrc += c;
			camRespondedPacketId |= ((uint16_t)c)<<8;
			camState = GET_IMG_DATA_LEN0;
			break;
			
		case GET_IMG_DATA_LEN0:
			CamCrc += c;
			camRespondedPacketLen = c;
			camState = GET_IMG_DATA_LEN1;
			break;
			
		case GET_IMG_DATA_LEN1:
			CamCrc += c;
			camRespondedPacketLen |= ((uint16_t)c)<<8;
			camRespondedPacketLen += 2;
			camRespondedDataLen = 0;
			camState = GET_IMG_DATA;
			break;
			
		case GET_IMG_DATA:
			if((camRespondedDataLen < CAMERA_DATA_SIZE))
			{
				cameraJpegFile.data[camRespondedDataLen] = c;
				CamCrc += c;
			}
			camRespondedDataLen++;
			if(camRespondedDataLen >= camRespondedPacketLen - 2)
				camState = GET_IMG_CRC0; 
			break;
			
		case GET_IMG_CRC0:
			camRespondedCrc = c;
			camState = GET_IMG_CRC1;
			break;
		case GET_IMG_CRC1:
			camRespondedCrc |= ((uint16_t)c)<<8;
			if(CamCrc == camRespondedCrc)
			{
				// prepair the packet
				CamCrc = 0;
				cameraJpegFile.record.cameraID = camRespondedId + 1;
				cameraJpegFile.len = camRespondedDataLen;
				cameraJpegFile.offset = (uint32_t)(camRespondedPacketId - 1)*CAMERA_DATA_SIZE;
				cameraJpegFile.fileSize = ssInfo[camRespondedId].size;
				
				cameraJpegFile.record.time = ssInfo[camRespondedId].time;
				camManState = GOT_PACKET;
				INFO("CAM: Got packet from CAM %d, ID: %d / %d, len:%d\r\n", camRespondedId, camRespondedPacketId, ssInfo[camCurrentId].packetCount, camRespondedDataLen);
			}
			else
				camManState = GET_PACKET;
			camState = WAIT_HEADER;
				break;
			default:
				camManState = GOT_PACKET;
				camState = WAIT_HEADER;
				break;
	}
}

static void SendCmd(uint8_t *buf, uint16_t len)
{
	camAcked = 0;
	//RS485_DIR = 1;
	while(len--){
		USART3_PutChar(*buf++);
	}
	//RS485_DIR = 0;
}
static void ProcessCmd()
{
	switch(camRespondedCmd){
		case 'R':			// snapshot info
			memcpy(&(ssInfo[camRespondedId].size), cameraJpegFile.data, 4);
			ssInfo[camRespondedId].packetCount = *((uint16_t*)(cameraJpegFile.data + 4));
			ssInfo[camRespondedId].time = rtcTimeSec;


			INFO("CAM: id=%d, snapshot size=%lu, packets=%d\r\n", camRespondedId, ssInfo[camRespondedId].size, ssInfo[camRespondedId].packetCount);
			break;
			
		case 'C':
			SendCmd((uint8_t*)&camRespondedId, 1);
			break;
			
		case 'H':
		case 'E':
		case 'D':
		case 'Q':
			camAcked = 1;
			break;
		case '?':
			camAcked = 0xff;
			break;
	}
}
