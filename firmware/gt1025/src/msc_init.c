/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "msc_init.h"
#include "sst25.h"
#include "lib/sys_tick.h"
extern uint32_t  DbgCfgPrintf(const uint8_t *format, ...);
#define PrintfDebug(...)  //DbgCfgPrintf(__VA_ARGS__)

extern const char *usb_driver;

FATFS sdfs;

FRESULT scan_files (
    char* path        /* Start node to be scanned (also used as work area) */
);

void mscInit(void)
{
	uint32_t flagWriteDriverFile = 0,res = 0;
	uint8_t buf[128];//,buf1[4096];
	FIL filFile;
	DIR dummy;

	if((FRESULT)disk_initialize(MMC) == FR_OK)
	{
		f_mount(&sdfs,"MMC:",1);
	}
	flagWriteDriverFile = 0;
	if(f_open(&filFile,"DRIVER.inf",  FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		PrintfDebug("\r\n Opened File DRIVER.inf on MMC=>Ok\r\n");
	}
	else
		flagWriteDriverFile = 1;
	f_close(&filFile);
//	if(flagWriteDriverFile)
//	{
//		PrintfDebug("\r\n Opened File DRIVER.inf on MMC=>Fail\r\n");
//		PrintfDebug("\r\n Start Formart MMC DISK\r\n");
//		SysTick_DelayMs(2000);
//		if(f_mkfs("MMC:", 0, 1) == FR_OK)
//		{
//			PrintfDebug("\r\n OK\r\n");
//			if(f_open(&filFile,"DRIVER.inf", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
//			{
//				f_write(&filFile,"thienhaiblue\r\n",14,&res);
//			}
//		}
//		else 
//		{
//			PrintfDebug("\r\n Fail\r\n");
//		}
//		f_close(&filFile);
//	}
	// create directory structure for new records
	if((res = f_opendir(&dummy, "IMAGE")) == FR_NO_PATH)
	{
		f_mkdir("IMAGE");
	}
	PrintfDebug("\r\n Sys Load File\r\n");
	scan_files("IMAGE");
}


FRESULT scan_files (
    char* path        /* Start node to be scanned (also used as work area) */
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                sprintf(&path[i], "/%s", fn);
                res = scan_files(path);
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                PrintfDebug("C:\\%s\\%s\r\n", path, fn);
            }
        }
        f_closedir(&dir);
    }
    return res;
}
		
		
		

