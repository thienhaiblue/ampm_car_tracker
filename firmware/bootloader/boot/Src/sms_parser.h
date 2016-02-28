#ifndef __SMS_PARSER_H__
#define __SMS_PARSER_H__
#include "modem.h"
#include "uart1.h"
#include "stdio.h"
#include "string.h"
#include "ringbuf.h"
#include "system_config.h"
#include "gps.h"
#include "exti.h"

extern uint8_t smsSender[18];

extern uint8_t callingNumber[16];

extern RINGBUF smsUnreadRingBuff;

extern RINGBUF gprsRingBuff;

void AT_CmdProcessInit(void);
void AT_ComnandParser(char c);
void SMS_Manage(void);
#endif
