/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AMPM_GSM_IO_H__
#define __AMPM_GSM_IO_H__
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "lib/sparser.h"
#include "lib/data_cmp.h"
#include "lib/ampm_sprintf.h"
#include "comm.h"

#define AMPM_GSM_RES_FAILS 1
#define AMPM_GSM_RES_OK		0

extern const uint8_t *modemOk;
extern const uint8_t *modemError;

typedef struct{
	uint8_t *command;
	uint8_t (*sprintf)(uint8_t *buf);
	uint8_t *response;
  uint32_t (*callback)(uint16_t cnt,uint8_t c);
	uint8_t *resOk;
	uint8_t *resFails;
}AMPM_GSM_AT_CMD_PACKET_TYPE;



extern uint8_t gotoCmdModeFlag;
extern uint8_t gotoDataModeFlag;
extern list_t ampm_GSM_CmdList;
extern Timeout_Type ampm_GSM_TaskTimeout;
extern AMPM_CMD_PHASE_TYPE ampm_GSM_CmdPhase;
extern AMPM_CMD_PROCESS_TYPE ampmGSM_CmdSend;
extern uint8_t *ampm_GprsApn;

uint8_t Ampm_AtCmdCheckResponse(uint8_t *str,uint32_t t);
uint8_t Ampm_SetCmdOK(AMPM_CMD_PHASE_TYPE phase);
uint32_t Ampm_GsmSendCallback(void);
uint32_t Ampm_GsmRecvCallback(void);
void Ampm_GsmGotoCmdMode(void);
void Ampm_GsmGotoDataMode(void);
void Ampm_GsmSetApn(uint8_t *apn);
uint8_t Ampm_SendAtCmd(const AMPM_CMD_PROCESS_TYPE *cmd);
void Ampm_SendAtCmdNow(const AMPM_CMD_PROCESS_TYPE *cmd);
uint8_t Ampm_SendAtCheck_IsEmpty(void);
uint8_t Ampm_SendCommand(const uint8_t *resOk,const uint8_t *resFails,uint32_t timeout,uint8_t tryAgainNum,const uint8_t *format, ...);
uint8_t Ampm_CmdTask_SendCmd(uint32_t (*callback)(uint16_t cnt,uint8_t c),uint8_t *response,
	uint32_t delay, const uint8_t *resOk, const uint8_t *resFails, 
	uint32_t timeout, uint8_t tryAgainNum, const uint8_t *format, ...);

#endif

