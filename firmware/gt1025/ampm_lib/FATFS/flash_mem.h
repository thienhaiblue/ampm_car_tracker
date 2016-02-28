

#ifndef __FLASH_MEM_H__
#define __FLASH_MEM_H__
#include "diskio.h"
#include <stdint.h>
#include "sst25.h"

extern uint8_t flashMemBuf[4096];

DSTATUS SST25_flash_disk_initialize(void);
DSTATUS MCU_flash_disk_initialize(void);
DSTATUS SST25_disk_status(void);
DRESULT SST25_disk_read(BYTE *buff, DWORD sector, BYTE count);
DRESULT SST25_disk_write(BYTE *buff, DWORD sector, BYTE count);
uint32_t get_fattime(void);
DRESULT SST25_disk_ioctl(BYTE ctrl, void* buff);
DRESULT MCU_flash_disk_ioctl(BYTE ctrl, void* buff);
DRESULT MCU_FlashWrite(BYTE *buff, DWORD sector, BYTE count);
DRESULT MCU_FlashRead(BYTE *buff, DWORD sector, BYTE count);

#endif
