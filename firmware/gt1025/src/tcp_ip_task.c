/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"
#include "ppp.h"
#include "resolv.h"
#include "resolver.h"
#include "tracking_server.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"
#include "ftp/ftpc.h"
#include "ftp/ftp.h"
#include "firmware_task.h"

uint32_t gprsDataSending = 0;
uint32_t tcpCnt = 0;
Timeout_Type tcpIpReset;
Timeout_Type tTcpDataIsBusy;
uint8_t tcpIpTryCnt;
extern uint8_t mainBuf[512];

uint8_t vTcpIpTaskInit(void)
{
	TRACKING_SVR_Init(1);
	FIRMWARE_Init(1);
	FTP_Init();
	InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
	InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(TCP_BUSY_TIMEOUT));
	return 0;
}

uint8_t vTcpIpTask(void)
{	
		if(CheckTimeout(&tcpIpReset) == SYSTICK_TIMEOUT || tcpIpTryCnt >= 5)
		{
			tcpIpTryCnt  = 0;
			InitTimeout(&tcpIpReset,SYSTICK_TIME_SEC(PPP_RESET_TIMEOUT));
			FTP_Reset();
			TRACKING_SVR_Reset();
			FIRMWARE_Reset();
			return 0xff;
		}
		FTP_Manage();
		TRACKING_SVR_Manage();
		FIRMWARE_Manage();
		FIRMWARE_Task(mainBuf);
		return 0;
}
