/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AMPM_GSM_COMMON_H__
#define __AMPM_GSM_COMMON_H__

#include "ampm_gsm_io.h"

extern uint8_t csqBuff[16];
extern float csqValue;
extern uint8_t gsmSignal;
extern uint8_t ampm_ModemResetFlag;

extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_ATH;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CSQ;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CLIP;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGF_1;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGF_0;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CMGL;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CGACT;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_DIALUP;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CREG;
extern const AMPM_GSM_AT_CMD_PACKET_TYPE ampmAtCmd_AT_CHLD;
extern const AMPM_CMD_PROCESS_TYPE ampmCmdProcess_AT;
uint32_t Ampm_GsmGetCSQ(uint16_t cnt,uint8_t c);

#endif

