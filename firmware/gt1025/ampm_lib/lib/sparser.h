

#ifndef __SPARSER_H__
#define __SPARSER_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib/list.h"
#include "lib/sys_tick.h"
/*String Info  definitions structure. */

typedef struct  {
	uint8_t *command;
	uint32_t (*callback)(uint16_t cnt,uint8_t c);
} STR_INFO_Type;

#define STR_PARSER_COUNT(x)   (sizeof (x) / sizeof (STR_INFO_Type))
typedef enum{
	AMPM_CMD_SEND = 0,
	AMPM_CMD_RESEND,
	AMPM_CMD_WAIT,
	AMPM_CMD_DELAY,
	AMPM_CMD_GOTO_ERROR,
	AMPM_CMD_ERROR,
	AMPM_CMD_OK
} AMPM_CMD_PHASE_TYPE;

typedef struct{
	void *next;
	void *appStruct;
	uint32_t (*send_data_callback)(void);
	uint32_t (*recv_data_callback)(void);
	uint8_t  (*end_data_callback)(AMPM_CMD_PHASE_TYPE phase);
	uint8_t retryNum;
	uint16_t timeout;/*cmd timeout in mS*/
	uint16_t delay;/*delay when retry: in mS*/
}AMPM_CMD_PROCESS_TYPE;

typedef struct{
	struct AMPM_CMD_LIST_TYPE *next;
	AMPM_CMD_PROCESS_TYPE *pt;
}AMPM_CMD_LIST_TYPE;



typedef enum{
	STR_NEW_STATE,
	STR_WAIT_FINISH,
	STR_FINISH
}STR_STATE_Type;


typedef struct  {
   STR_STATE_Type state;
	 uint16_t dataRecvLength;
	 uint16_t dataRecvMaxLength;
	 uint8_t strParserCntMax;
	 uint8_t strInProcess;
	 uint32_t timeout;
	 uint32_t timeoutSet;
} STR_PARSER_Type;

void StrParserInit(STR_PARSER_Type *process,uint8_t *numCharParsed,uint8_t strParserCntMax,uint16_t dataLengthMax);
void StrComnandParser(uint32_t sysTimeMs,const STR_INFO_Type *info,STR_PARSER_Type *process,uint8_t *numCharParsed,uint8_t c);
void Ampm_Cmd_Task(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
list_t cmdList, 
Timeout_Type *taskTimeout
);
uint8_t Ampm_SendCmd(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
const AMPM_CMD_PROCESS_TYPE *cmd,
Timeout_Type *taskTimeout
);
void Ampm_SendCmdNow(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
const AMPM_CMD_PROCESS_TYPE *cmd,
Timeout_Type *taskTimeout
);
uint8_t Ampm_CmdTask_IsIdle(AMPM_CMD_PHASE_TYPE ampmCmdPhase);
void Ampm_CmdAddToList(list_t cmdList, AMPM_CMD_LIST_TYPE *cmd);
void Ampm_CmdResetList(list_t cmdList);
uint8_t Ampm_CmdCheckList_IsEmpty(list_t cmdList);
uint8_t Ampm_CmdCheck_IsEmpty(AMPM_CMD_PROCESS_TYPE *cmdSend);

#endif
