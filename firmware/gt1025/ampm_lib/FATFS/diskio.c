/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "flash_mem.h"
#include "mmc_ssp.h"
#include "lib/sys_time.h"

extern uint32_t  DbgCfgPrintf(const uint8_t *format, ...);
#define DISKIO_DBG(...)  //DbgCfgPrintf(__VA_ARGS__)

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	switch (pdrv) {
	case MMC :
		stat = MMC_disk_initialize(0);
		// translate the reslut code here
		return stat;
	case MCU_FLASH :
		//stat = MCU_flash_disk_initialize();
		// translate the reslut code here
		return stat;
	case SST25_FLASH:
		//stat = SST25_flash_disk_initialize();
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	switch (pdrv) {
		case MMC :
		stat = MMC_disk_status(0);

		// translate the reslut code here

		return stat;
	case MCU_FLASH :
		//stat = SST25_disk_status();
		// translate the reslut code here
		return stat;
	case SST25_FLASH :
		//stat = SST25_disk_status();
		// translate the reslut code here
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res = RES_OK;
	//DISKIO_DBG("\r\nDISKIO_%d:Read sector:%d\r\n",pdrv,sector);
	switch (pdrv) {
	case MMC :
		// translate the arguments here
		res = MMC_disk_read(0,buff, sector, count);
		// translate the reslut code here

		return res;
	case MCU_FLASH :
		// translate the arguments here
		//res = MCU_FlashRead(buff, sector, count);
		// translate the reslut code here
		return res;
	case SST25_FLASH :
		// translate the arguments here
		//res = SST25_disk_read(buff, sector, count);
		// translate the reslut code here
		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res = RES_OK;
	DISKIO_DBG("\r\nDISK%d:Wsector:%d\r\n",pdrv,sector);
	switch (pdrv) {
	case MMC :
		// translate the arguments here

		res = MMC_disk_write(0,buff, sector, count);

		// translate the reslut code here

		return res;
	case MCU_FLASH :
		// translate the arguments here
		//res = MCU_FlashWrite((BYTE *)buff, sector, count);
		// translate the reslut code here
		return res;
		case SST25_FLASH :
		// translate the arguments here
		//res = SST25_disk_write((BYTE *)buff, sector, count);
		// translate the reslut code here
		return res;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;

	switch (pdrv) {
	case MMC :
		// pre-process here

		res = MMC_disk_ioctl(0,cmd, buff);

		// post-process here

		return res;
	case MCU_FLASH :
		// pre-process here
		//res = MCU_flash_disk_ioctl(cmd, buff);
		// post-process here
		return res;
	case SST25_FLASH :
		// pre-process here
		//res = SST25_disk_ioctl(cmd, buff);
		// post-process here
		return res;
	}
	return RES_PARERR;
}
#endif

uint32_t get_fattime(void)
{
	DWORD v = 0;
	
	v = sysTime.year - 1980;
	v <<= 25;
	
	v |= (DWORD)sysTime.month << 21;
	v |= (DWORD)sysTime.mday << 16;
	v |= (DWORD)sysTime.hour << 11;
	v |= (WORD)sysTime.min << 5;
	v |= sysTime.sec >> 1;
	
	return v;
}
