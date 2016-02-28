/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AMPM_GSM_MAIN_TASK_H__
#define __AMPM_GSM_MAIN_TASK_H__
#include "at_command_parser.h"
#include "ampm_gsm_common.h"
#include "ampm_gsm_startup.h"
#include "ampm_gsm_dial_up.h"
#include "ampm_gsm_ring.h"
#include "ampm_gsm_sms.h"
#include "tcp_ip_task.h"
#include "ppp.h"

typedef enum{
	AMPM_GSM_MAIN_INIT_PHASE = 0,
	AMPM_GSM_MAIN_STARTUP_PHASE,
	AMPM_GSM_MAIN_SMS_RECV_PHASE,
	AMPM_GSM_MAIN_SMS_SEND_PHASE,
	AMPM_GSM_MAIN_VOICECALL_PHASE,
	AMPM_GSM_MAIN_VOICECALL_END_PHASE,
	AMPM_GSM_MAIN_DATACALL_PHASE,
	AMPM_GSM_MAIN_DIAL_UP_PHASE,
	AMPM_GSM_MAIN_GOTO_CMD_MODE_PHASE,
	AMPM_GSM_MAIN_GOTO_CMD_MODE_CHECK_PHASE,
	AMPM_GSM_MAIN_WAIT_GOTO_CMD_MODE_PHASE,
	AMPM_GSM_MAIN_GOTO_DATA_MODE_PHASE,
	AMPM_GSM_MAIN_WAIT_GOTO_DATA_MODE_PHASE,
	AMPM_GSM_MAIN_IDLE_PHASE
} AMPM_GSM_MAIN_PHASE_TYPE;
uint8_t Ampm_GsmIsWorking(void);
void AMPM_GSM_MainTask(void);
void AMPM_GSM_Init(uint8_t *apn, uint8_t *usr,uint8_t *pwr,uint8_t (*tcpIpCallback)(void),uint8_t (*tcpIpInit)(void));
#endif

