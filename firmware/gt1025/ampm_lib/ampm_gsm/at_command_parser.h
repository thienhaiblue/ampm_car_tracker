/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AT_COMMAND_PARSER_H__
#define __AT_COMMAND_PARSER_H__
#include "stdio.h"
#include "string.h"
#include "lib/sparser.h"
#include "at_command_parser.h"
#include "ampm_gsm_sms.h"
#include "ampm_gsm_call.h"
#include "ampm_gsm_ring.h"

#define GPRS_DATA_MAX_LENGTH	1024

extern uint8_t ampm_AtCommandParserEnable;
extern char *commandSeparator;
extern STR_PARSER_Type AT_CmdParser;
extern const uint8_t terminateStr[7];
void AT_CmdProcessInit(void);
void AT_ComnandParser(char c);

void AT_CommandCtl(void);


#endif
