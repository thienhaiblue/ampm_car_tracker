#ifndef __SMS_TASK_H__
#define __SMS_TASK_H__

#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "lib/ringbuf.h"
#include "ampm_gsm_main_task.h"


extern SMS_LIST_TYPE replySms;

extern const uint8_t smsReplyOk[];

extern uint8_t replySmsBuf[160];
extern uint8_t firmwarePhoneSender[16];
extern uint8_t myAddr[160];
extern uint8_t getAddrFlag;
extern uint8_t gotAddrFlag;
extern uint8_t askAddrCnt;
extern uint8_t addrSendToUser1Flag;
extern uint8_t addrSendToUser2Flag;
extern uint8_t addrSendToUser3Flag;
extern uint8_t addrSendToBoss1Flag;
extern uint8_t addrSendToBoss2Flag;
extern uint8_t addrSendToBoss3Flag;
extern uint8_t findCarFlag;

void SMS_Manage(uint8_t *buff,uint32_t lengBuf);
void SMS_TaskInit(void);
void RequestTableTask(uint32_t rtc);
uint16_t UniConfigParser(uint16_t *in, uint16_t *out);
uint16_t CMD_CfgParse(char *cmdBuff, uint8_t pwdCheck,uint8_t sourceCmd);

#endif
