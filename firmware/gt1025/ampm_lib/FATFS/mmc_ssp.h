
#ifndef __MMC_SSP_H__
#define __MMC_SSP_H__

#include "integer.h"
#include "diskio.h"

extern DWORD disk_sector_number_max;
extern DWORD disk_sector_size;
extern DWORD disk_memory_size; 


DSTATUS MMC_disk_initialize (
	BYTE drv		/* Physical drive number (0) */
);
DSTATUS MMC_disk_status (
	BYTE drv		/* Physical drive number (0) */
);
DRESULT MMC_disk_read (
	BYTE drv,		/* Physical drive number (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
);
DRESULT MMC_disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
);
DWORD MMC_disk_multiple_write_start(DWORD sector,DWORD count);
DWORD MMC_disk_multiple_write_stop(void);
DRESULT MMC_disk_multiple_write (const BYTE *buff);
DWORD MMC_disk_multiple_read_start(DWORD sector);
DWORD MMC_disk_multiple_read_stop(void);
DRESULT MMC_disk_multiple_read (BYTE *buff);
DRESULT MMC_disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
);
DRESULT  MMC_disk_info(void);

#endif
