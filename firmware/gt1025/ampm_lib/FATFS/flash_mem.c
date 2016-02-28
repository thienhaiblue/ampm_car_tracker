
#include "flash_mem.h"
#include "diskio.h"
#include "string.h"
#include "ffconf.h"
#include "lib/sys_time.h"
extern uint32_t  DbgCfgPrintf(const uint8_t *format, ...);
#define FLASH_DBG(...)  //DbgCfgPrintf(__VA_ARGS__)
#define MSC_FLASH_SECTOR_SIZE	_MAX_SS

uint32_t flashFormatCnt = 0;


DSTATUS MCU_flash_disk_initialize(void)
{
	return 0;
}

DSTATUS SST25_flash_disk_initialize(void)
{
	SST25_Init();
	return 0;
}


DSTATUS SST25_disk_status(void)
{
	return 0;
}

DRESULT SST25_disk_read(BYTE *buff, DWORD sector, BYTE count)
{
	uint8_t trycnt = 3;
	while(trycnt--)
	{
		//if(SST25_Read(sector*MSC_FLASH_SECTOR_SIZE + SST25_MSC_FLASH_START,buff,MSC_FLASH_SECTOR_SIZE) == SST25_OK)
			return RES_OK;
	}
	return RES_ERROR;
}




DRESULT SST25_disk_write(BYTE *buff, DWORD sector, BYTE count)
{
//	uint32_t n,i,flashSectorNum,flashSectorAddr,offset;
	uint8_t trycnt = 3;
	FLASH_DBG("\r\nFLASH:Write sector:%d\r\n",sector);
	while(trycnt--)
	{
//		n = sector * MSC_FLASH_SECTOR_SIZE; //flash addr
//		flashSectorNum = (n / SST25_SECTOR_SIZE);
//		flashSectorAddr = flashSectorNum*SST25_SECTOR_SIZE;
//		SST25_Read(flashSectorAddr + SST25_MSC_FLASH_START,flashMemBuf,SST25_SECTOR_SIZE);
//		offset = sector * MSC_FLASH_SECTOR_SIZE - flashSectorAddr;
//		memcpy(&flashMemBuf[offset],buff,MSC_FLASH_SECTOR_SIZE);
//		if(SST25_Write(flashSectorAddr + SST25_MSC_FLASH_START,flashMemBuf,SST25_SECTOR_SIZE) == SST25_OK)
			return RES_OK;
	}
	return RES_ERROR;
}

DRESULT MCU_FlashRead(BYTE *buff, DWORD sector, BYTE count)
{

	return RES_OK;
}

DRESULT MCU_FlashWrite(BYTE *buff, DWORD sector, BYTE count)
{

		return RES_OK;
}



DRESULT MCU_flash_disk_ioctl(BYTE ctrl, void* buff)
{
	return RES_OK;
}

DRESULT SST25_disk_ioctl(BYTE ctrl, void* buff)
{
	DRESULT res;
	DWORD csize;
	
	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC:
				res = RES_OK;
			break;
			
		case GET_SECTOR_COUNT:			// get number of sectors on disk
				csize = (FLASH_SIZE_MAX - SST25_MSC_FLASH_START) / _MAX_SS;
				*(DWORD*)buff = (DWORD)csize;
				res = RES_OK;		
			break;
			
		case GET_SECTOR_SIZE : // get size of sectors on disk 
			*(WORD*) buff = _MAX_SS;
			res = RES_OK;
			break;
			
		case GET_BLOCK_SIZE : 		// get erase block size in units of sectors
				res = RES_OK;		
			break;
		default:
			res = RES_PARERR;	
	}	
	return res;
}

