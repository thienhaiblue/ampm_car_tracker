

#include <string.h>
#include <stddef.h>
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "uip_ppp.h"
#include "system_config.h"
#include "ftpc.h"
#include "resolver.h"
#include "tcp_ip_task.h"
#include "ff.h"
#include "diskio.h"
#include "tcpip.h"
#include "db.h"
#include "camera.h"

#define FTP_RECONNECT_TIMEOUT		30 * SYSTICK_SECOND

#define MAX_USERNAMELEN 16
#define MAX_PASSWORDLEN 16
#define MAX_HOSTNAMELEN 32


FIL ftpFile;

ftp_connection connection;

#define FTP_DBG(...)		DbgCfgPrintf(__VA_ARGS__)
TCP_STATE_TYPE ftpState = INITIAL;
uip_ipaddr_t ftpServerIpAddr;
uip_ipaddr_t *ftpServerIp;
uint16_t ftpServerPort;
uint32_t ftpSwitchServer = 0;
volatile int8_t ftpLoggedIn = 0;
uint32_t ftpTick;
char *filePath = "IMAGE";

uint8_t ftpTimesSendCnt;
uint8_t haveOnlyOneFileInDisk = 0;

uint8_t fileNameChangeCnt;

char *FTPGetUser(void);
char *FTPGetPass(void);
char *FTPGetFileName(void);
void FTPListFile(char *filename);

char ftpFileName[32] = "note.txt";
uint8_t ftpDirIsOk = 0;

enum {
	FTP_IDLE = 0,
	FTP_CHECK_BEFORE_SEND,
	FTP_WAIT_BEFORE_SEND,
	FTP_CREAT_DIR,
	FTP_GET_FILE,
	FTP_GET_FILE_WAIT,
	FTP_SEND_FILE,
	FTP_SEND_FILE_WAIT
}ftpProcessPhase = FTP_IDLE;

Timeout_Type ftpProcessTimeout;
Timeout_Type ftpCheckFilesTimeout;

MSG_STATUS_RECORD	ftplogRecord;

void FTP_Init(void)
{
	ftpState = INITIAL;
	ftpProcessPhase = FTP_IDLE;
}

void FTP_Reset(void)
{
	ftpState = INITIAL;
	ftpProcessPhase = FTP_IDLE;
}

uint8_t FTP_Task_IsIdle(void)
{
	if(ftpProcessPhase == FTP_IDLE)
		return 1;
	return 0;
}

TCP_STATE_TYPE FTP_Manage(void)
{ 
	uint16_t i;
	char *pt;
	uint32_t len;
	DATE_TIME time;
	FRESULT res;
	FILINFO fno;
	DIR dir;
	FIL fsFile;
	char buf[32];
	char buf1[32];
  char *fn;   /* This function is assuming non-Unicode cfg. */
	static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
	switch(ftpState)
	{
		case INITIAL:
			//ftpProcessPhase = FTP_IDLE;
			if(ftpProcessPhase != FTP_IDLE)
			{
				if(ftpSwitchServer == 0)	
				{
						ftpServerIp = (uip_ipaddr_t*)&sysCfg.priIserverIp;
						ftpServerPort = sysCfg.priIserverPort;
				}
				else
				{
					ftpServerIp = (uip_ipaddr_t*)&sysCfg.secIserverIp;
					ftpServerPort = sysCfg.secIserverPort;
				}
//				ftpServerIp = &ftpServerIpAddr;
//				ftpServerIp->u8[0] = 118;
//				ftpServerIp->u8[1] = 69;
//				ftpServerIp->u8[2] = 60;
//				ftpServerIp->u8[3] = 174;

				if(ftpServerIp == NULL) 
					break;
				else if(ftpServerIp->u8[0] == 0 
				&& ftpServerIp->u8[1]== 0 
				&& ftpServerIp->u8[2] == 0
				&& ftpServerIp->u8[3] == 0
				)
				{
					CFG_Load();
					break;
				}
				ftpLoggedIn = 0;
				FTP_DBG("\r\nFTP_SVR: Started, Server %d.%d.%d.%d:%d\r\n", ((uint8_t*)(ftpServerIp))[0], ((uint8_t*)(ftpServerIp))[1], 
						((uint8_t*)(ftpServerIp))[2], ((uint8_t*)(ftpServerIp))[3], 21);

				ftpTick = SysTick_Get();
				if(ftpc_connect(&connection,ftpServerIp, ftpServerPort) == NULL)
				{
					FTP_DBG("\r\nFTP_SVR: uip_connect returns NULL\r\n");
					ftpState = WAIT_TIMEOUT_RECONNECT;
					break;
				}
				//init handle for connection
				connection.fatFile = &ftpFile;
				connection.ftp_get_user = FTPGetUser;
				connection.ftp_get_pass = FTPGetPass;
				connection.ftp_list_file = FTPListFile;
				connection.ftp_get_file_name = FTPGetFileName;
				ftpState = CONNECT;
			}
		break;
		case CONNECT:
		case WAIT_TIMEOUT_RECONNECT:
			if(SysTick_Get() - ftpTick >= FTP_RECONNECT_TIMEOUT)
			{
				ftpTick = SysTick_Get();
				ftpState = INITIAL;
			}
			if(connection.socket_status == FTP_SOCKET_CONNECTED)
				ftpState = CONNECTED;
			break;
		case CONNECTED:
			//code here
				if(connection.socket_status == FTP_SOCKET_DISCONNECTED)
				{
					ftpState = INITIAL;
				}
//				if(CheckTimeout(&tFtpConnectionTimeout)){
//					InitTimeout(&tFtpConnectionTimeout,SYSTICK_TIME_SEC(30));
//					ftpState = INITIAL;
//				}
		break;
		default:
			
			break;
	}
	
	switch(ftpProcessPhase)
	{
		case FTP_IDLE:
			if(CheckTimeout(&ftpCheckFilesTimeout) == SYSTICK_TIMEOUT && (!CAM_IsBusy())){
				if(filePermissionDenied)
				{
					fileNameChangeCnt++;
					filePermissionDenied = 0;
					FTP_DBG("\r\n FTP_SEND_FILE\r\n");
					ftpProcessPhase = FTP_SEND_FILE;
					InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
					FTP_DBG("\r\nFTP:filePermissionDenied:%d\r\n",filePermissionDenied);
					break;
				}
				InitTimeout(&ftpCheckFilesTimeout,SYSTICK_TIME_SEC(10));
				FTP_DBG("\r\nCheck File...\r\n");
				res = f_opendir(&dir, filePath);                       /* Open the directory */
				if (res == FR_OK) {
					i = strlen(filePath);
					for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
            fn = *fno.lfname ? fno.lfname : fno.fname;
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
					} else {                                  /* It is a file. */
							FTP_DBG("\r\nFound->C:\\%s\\%s\r\n", filePath, fn);
								if((strstr(fn,".inf") != NULL
								&& strstr(fn,".jpg") != NULL)
								|| (strlen(fn) != 18) //file name is not 18 char
								)
								{
									FTP_DBG("\r\nDelete->C:\\%s\\%s.jpg\r\n", filePath, fn);
									sprintf(buf,"IMAGE/%s",fn);
									res = f_unlink(buf);
								}else{
									pt = strstr(fn,".inf");
									if(pt != NULL){
										pt[0] = '\0'; //remove .inf
										sprintf(buf,"%s.jpg",fn);//try to open .jpg file
										if(strcmp(ftpFileName,buf) == NULL)
										{
											FTP_DBG("\r\nThis file is processing\r\n");
											if(haveOnlyOneFileInDisk == 0){//If have any one file in this disk, Try to send it.
												haveOnlyOneFileInDisk = 1;
												continue; 
											}
										}
										haveOnlyOneFileInDisk = 0;
										sprintf(buf,"IMAGE/%s.jpg",fn);//try to open .jpg file
										if(f_open(&fsFile,buf, FA_READ | FA_OPEN_EXISTING) == FR_OK){//if can open then process inf file
											if(f_size(&fsFile) >= 200*1024){ //if filesize > 200kBytes detele it
												FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
												res = f_unlink(buf);
												f_close(&fsFile);//close .jpg file
												continue;
											}
											f_close(&fsFile);//close .jpg file
											sprintf(buf,"IMAGE/%s.inf",fn);//process inf file
											if(f_open(&fsFile,buf, FA_READ | FA_OPEN_EXISTING) == FR_OK){
												f_read(&fsFile,&ftplogRecord,sizeof(MSG_STATUS_RECORD),&len);
												if(ftplogRecord.crc == DbCalcCheckSum((uint8_t *)&ftplogRecord,sizeof(MSG_STATUS_RECORD) - 1)){
													FTP_DBG("\r\nFPT->Update File:C:\\%s\\%s.jpg\r\n", filePath, fn);
													sprintf(buf,"%s.jpg",fn);
													if(strcmp(buf,ftpFileName) != NULL)
													{
														fileNameChangeCnt = 0;
														strcpy(ftpFileName,buf);
													}
													if(ftpDirIsOk){
														FTP_DBG("\r\nFTP_SEND_FILE\r\n");
														ftpProcessPhase = FTP_SEND_FILE;
														InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
													}else{
														FTP_DBG("\r\nFTP_CHECK_BEFORE_SEND\r\n");
														ftpProcessPhase = FTP_CHECK_BEFORE_SEND;
														InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(30));
													}
												}else{//if ftplogRecord check sum is error
													FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
													res = f_unlink(buf);//delete .inf
													sprintf(buf,"IMAGE/%s.jpg",fn);//process inf file
													FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
													res = f_unlink(buf); //delete .jpg
												}
											}else{//if can't open .inf file detete it
												FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
												res = f_unlink(buf);
											}
										}else{//if can't open .jpg file then delete .inf file
											sprintf(buf,"IMAGE/%s.inf",fn);
											FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
											res = f_unlink(buf);//detelte .inf file
										}
										f_close(&fsFile);
										break;
									}else{
										pt = strstr(fn,".jpg");
										if(pt != NULL && strcmp(ftpFileName,fn) != NULL){
											TIME_FromSec(&time,cameraJpegFile.record.time); 
											sprintf(buf, "%d_%02d%02d%02d%02d%02d%02d.jpg",
												cameraJpegFile.record.cameraID,
												time.year%100,
												time.month,
												time.mday,
												time.hour, 
												time.min,
												time.sec);
												
											if(strcmp(fn,buf) != NULL){ //if this file is not writing by camera task
												pt[0] = '\0'; //remove .jpg
												sprintf(buf,"IMAGE/%s.inf",fn);//process .inf file
												if(f_open(&fsFile,buf, FA_READ | FA_OPEN_EXISTING) == FR_OK){
													f_read(&fsFile,&ftplogRecord,sizeof(MSG_STATUS_RECORD),&len);
													if(ftplogRecord.crc == DbCalcCheckSum((uint8_t *)&ftplogRecord,sizeof(MSG_STATUS_RECORD) - 1)){
														FTP_DBG("\r\nFPT->Update File:C:\\%s\\%s.jpg\r\n", filePath, fn);
														sprintf(buf,"%s.jpg",fn);
														if(strcmp(buf,ftpFileName) != NULL)
														{
															fileNameChangeCnt = 0;
															strcpy(ftpFileName,buf);
														}
														if(ftpDirIsOk){
															FTP_DBG("\r\nFTP_SEND_FILE\r\n");
															ftpProcessPhase = FTP_SEND_FILE;
															InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
														}else{
															FTP_DBG("\r\nFTP_CHECK_BEFORE_SEND\r\n");
															ftpProcessPhase = FTP_CHECK_BEFORE_SEND;
															InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(30));
														}
													}else{//if ftplogRecord check sum is error
														FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
														res = f_unlink(buf);//delete .inf
														sprintf(buf,"IMAGE/%s.jpg",fn);//process inf file
														FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
														res = f_unlink(buf); //delete .jpg
													}
											}else{//if can't open .inf file detete it
												FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
												res = f_unlink(buf);
												sprintf(buf,"IMAGE/%s.jpg",fn);//process inf file
												FTP_DBG("\r\nFPT->Delete:C:/%s\r\n", buf);
												res = f_unlink(buf); //delete .jpg
											}
											f_close(&fsFile);
										}else{
											FTP_DBG("\r\nThis file is writing by camera task\r\n");
											FTP_DBG("\r\nIs waiting for camera take picture:%d%%\r\n",cameraJpegFile.offset*100/cameraJpegFile.fileSize);
										}
									}
								}
							}
						}
					}
					f_closedir(&dir);
				}
			}
		break;
		case FTP_CHECK_BEFORE_SEND:
				if(ftpc_list(&connection,(char *)sysCfg.imei)){
					FTP_DBG("\r\nFTP_WAIT_BEFORE_SEND\r\n");
					ftpProcessPhase = FTP_WAIT_BEFORE_SEND;
					InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(30));
				}
				if(CheckTimeout(&ftpProcessTimeout) == SYSTICK_TIMEOUT){
					connection.type = TYPE_CLOSE;
					ftpProcessPhase = FTP_IDLE;
				}
		break;
		case FTP_WAIT_BEFORE_SEND:
			if(connection.dataconn.status != FTP_DATA_NONE){
				if(ftpDirIsOk){
					ftpProcessPhase = FTP_SEND_FILE;
					FTP_DBG("\r\nFTP_SEND_FILE\r\n");
					InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
				}
				else{
					FTP_DBG("\r\nFTP_CREAT_DIR\r\n");
					ftpProcessPhase = FTP_CREAT_DIR;
				}
			}
			if(CheckTimeout(&ftpProcessTimeout) == SYSTICK_TIMEOUT){
				connection.type = TYPE_CLOSE;
				ftpProcessPhase = FTP_IDLE;
			}
		break;
		case FTP_CREAT_DIR:
			if(ftpc_mkd(&connection,(char *)sysCfg.imei)){
				FTP_DBG("\r\n FTP_SEND_FILE\r\n");
				ftpProcessPhase = FTP_SEND_FILE;
				InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
			}
		break;
		case FTP_GET_FILE:
			if(ftpc_get(&connection,ftpFileName)){
				FTP_DBG("\r\n FTP_GET_FILE_WAIT\r\n");
				ftpProcessPhase = FTP_GET_FILE_WAIT;
				InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(30));
			}
		break;
		case FTP_GET_FILE_WAIT:
			if(connection.dataconn.status != FTP_DATA_NONE){
				FTP_DBG("\r\n FTP_IDLE\r\n");
				ftpProcessPhase = FTP_IDLE;
			}
			if(CheckTimeout(&ftpProcessTimeout) == SYSTICK_TIMEOUT){
				connection.type = TYPE_CLOSE;
				FTP_DBG("\r\n FTP_IDLE\r\n");
				ftpProcessPhase = FTP_IDLE;
			}
		break;
		case FTP_SEND_FILE:
			
			if(filePermissionDenied)
			{
				fileNameChangeCnt++;
				filePermissionDenied = 0;
				FTP_DBG("\r\n FTP_SEND_FILE\r\n");
				ftpProcessPhase = FTP_SEND_FILE;
				InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
				FTP_DBG("\r\nFTP:filePermissionDenied:%d\r\n",filePermissionDenied);
			}
			if(fileNameChangeCnt)
			{
				strcpy(buf1,ftpFileName);
				pt = strstr(buf1,".jpg");
				if(pt != NULL)
				{
					pt[0] = 0; //clear .jpg
					sprintf(buf,"%s_%d.jpg",buf1,fileNameChangeCnt);
				}
			}
			else
			{
				strcpy(buf,ftpFileName);
			}
			if(ftpc_put(&connection,buf))
			{
					ftpProcessPhase = FTP_SEND_FILE_WAIT;
					FTP_DBG("\r\n FTP_SEND_FILE_WAIT\r\n");
					InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(600));
			}
			if(CheckTimeout(&ftpProcessTimeout) == SYSTICK_TIMEOUT){
				//connection.type = TYPE_CLOSE;
				FTP_DBG("\r\n FTP_IDLE\r\n");
				ftpProcessPhase = FTP_IDLE;
			}
		break;
		case FTP_SEND_FILE_WAIT:
			if(filePermissionDenied)
			{
				fileNameChangeCnt++;
				filePermissionDenied = 0;
				FTP_DBG("\r\n FTP_SEND_FILE\r\n");
				ftpProcessPhase = FTP_SEND_FILE;
				InitTimeout(&ftpProcessTimeout,SYSTICK_TIME_SEC(120));
				FTP_DBG("\r\nFTP:filePermissionDenied:%d\r\n",filePermissionDenied);
				break;
			}
			if(connection.dataconn.status != FTP_DATA_NONE){
				FTP_DBG("\r\nFTP:Finish send a file\r\n");
				if(connection.dataconn.status == FTP_DATA_COMPLETED){
						FTP_DBG("FTP:RingLod:%d;\r\n",ringLog.head);
						DB_SaveLog(&ftplogRecord);
						FTP_DBG("FTP:DB_SaveLog(&ftplogRecord):%d;\r\n",ringLog.head);
						sprintf(buf,"IMAGE/%s",ftpFileName);
						res = f_unlink(buf);
						pt = strstr(buf,".jpg");
						if(pt){
							strcpy(pt,".inf");
							res = f_unlink(buf);
						}
				}
				InitTimeout(&ftpCheckFilesTimeout,SYSTICK_TIME_SEC(10));
				ftpProcessPhase = FTP_IDLE;
				FTP_DBG("\r\n FTP_IDLE\r\n");
			}
			if(CheckTimeout(&ftpProcessTimeout) == SYSTICK_TIMEOUT){
				//connection.type = TYPE_CLOSE;
				FTP_DBG("\r\n FTP_IDLE\r\n");
				InitTimeout(&ftpCheckFilesTimeout,SYSTICK_TIME_SEC(10));
				ftpProcessPhase = FTP_IDLE;
			}
			if(connection.state == STATE_CONNECTED){
				FTP_DBG("\r\n FTP_IDLE\r\n");
				InitTimeout(&ftpCheckFilesTimeout,SYSTICK_TIME_SEC(10));
				ftpProcessPhase = FTP_IDLE;
			}
		break;
	}

	return ftpState;
}



char *FTPGetUser(void)
{
	return (char *)sysCfg.fUserName;
}

void FTPListFile(char *filename)
{
	if(strcmp(filename,(char *)sysCfg.imei) == NULL)
	{
		ftpDirIsOk = 1;
	}
}

char *FTPGetPass(void)
{
	return (char *)sysCfg.fPassword;
}

char *FTPGetFileName(void)
{
	return (char *)ftpFileName;
}

