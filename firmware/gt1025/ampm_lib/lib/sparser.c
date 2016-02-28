
#include "lib/sparser.h"


extern  uint32_t TICK_Get(void);

void Ampm_CmdAddToList(list_t cmdList, AMPM_CMD_LIST_TYPE *cmd)
{
	list_add(cmdList, cmd);
}

void Ampm_CmdResetList(list_t cmdList)
{
	list_init(cmdList);
}


uint8_t Ampm_CmdCheckList_IsEmpty(list_t cmdList)
{
	if(cmdList[0] == NULL)
		return 1;
	return 0;
}

uint8_t Ampm_CmdCheck_IsEmpty(AMPM_CMD_PROCESS_TYPE *cmdSend)
{
	if(cmdSend->next == NULL)
		return 1;
	return 0;
}

uint8_t Ampm_CmdTask_IsIdle(AMPM_CMD_PHASE_TYPE ampmCmdPhase)
{
	switch(ampmCmdPhase)
	{
		case AMPM_CMD_ERROR:
		case AMPM_CMD_OK:
			return 1;
		default:
			return 0;
	}
}

uint8_t Ampm_SendCmd(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
const AMPM_CMD_PROCESS_TYPE *cmd,
Timeout_Type *taskTimeout
)
{
	if(Ampm_CmdTask_IsIdle(*ampmCmdPhase))
	{
		*cmdSend = *cmd;
		InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
		*ampmCmdPhase = AMPM_CMD_SEND;
		return 1;
	}
	return 0;
}


void Ampm_SendCmdNow(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
const AMPM_CMD_PROCESS_TYPE *cmd,
Timeout_Type *taskTimeout
)
{
		*cmdSend = *cmd;
		InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
		*ampmCmdPhase = AMPM_CMD_SEND;
}

void Ampm_Cmd_Task(
AMPM_CMD_PHASE_TYPE *ampmCmdPhase,
AMPM_CMD_PROCESS_TYPE *cmdSend,
list_t cmdList, 
Timeout_Type *taskTimeout
)
{
	uint8_t c,res;
	AMPM_CMD_LIST_TYPE *cmd;
	switch(*ampmCmdPhase)
	{
		case AMPM_CMD_SEND:

				if(CheckTimeout(taskTimeout) == SYSTICK_TIMEOUT)
				{
						if(cmdSend->send_data_callback != NULL)
						{
							cmdSend->send_data_callback();
							InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->timeout));
							*ampmCmdPhase = AMPM_CMD_WAIT;
						}
						else
						{
							*ampmCmdPhase = AMPM_CMD_DELAY;
						}
				}
			break;
		case AMPM_CMD_RESEND:
			if (cmdSend->retryNum)
			{
				cmdSend->retryNum--;
				InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
				*ampmCmdPhase = AMPM_CMD_SEND;
			}
			else
			{
				*ampmCmdPhase = AMPM_CMD_GOTO_ERROR;				
			}
		break;
		case AMPM_CMD_GOTO_ERROR:		
				if(cmdSend->end_data_callback){
					if(cmdSend->end_data_callback(AMPM_CMD_ERROR))
					{	
						if(*ampmCmdPhase == AMPM_CMD_GOTO_ERROR)
						{
							*ampmCmdPhase = AMPM_CMD_ERROR;
						}
					}
				}else{
					*ampmCmdPhase = AMPM_CMD_ERROR;
				}
		break;
		case AMPM_CMD_WAIT:
			if(cmdSend->recv_data_callback == NULL)
			{
				*ampmCmdPhase = AMPM_CMD_DELAY;
				break;	
			}
			if(CheckTimeout(taskTimeout) == SYSTICK_TIMEOUT)
			{
				*ampmCmdPhase = AMPM_CMD_RESEND;
			}
			else
			{
					res = cmdSend->recv_data_callback();
					if(res == AMPM_CMD_DELAY)
					{
						InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
						*ampmCmdPhase = AMPM_CMD_DELAY;
					}
					else if(res == AMPM_CMD_RESEND)
					{
						*ampmCmdPhase = AMPM_CMD_RESEND;
							InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
					}
			}
			break;
		case AMPM_CMD_DELAY:	
			if(CheckTimeout(taskTimeout) == SYSTICK_TIMEOUT)
			{			
				if(cmdSend->end_data_callback){
					if(cmdSend->end_data_callback(AMPM_CMD_OK))
					{
							if(*ampmCmdPhase == AMPM_CMD_DELAY)
							{
								*ampmCmdPhase = AMPM_CMD_OK;
							}
					}
				}else{
					*ampmCmdPhase = AMPM_CMD_OK;
				}
			}
		break;
		
		case AMPM_CMD_ERROR:		
		case AMPM_CMD_OK:
			if(cmdSend->next)
			{
				
				*cmdSend = *((AMPM_CMD_PROCESS_TYPE *)cmdSend->next);
				InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
				*ampmCmdPhase = AMPM_CMD_SEND;
				break;
			}
			if(cmdList[0] != NULL)
			{
				if(*ampmCmdPhase != AMPM_CMD_ERROR)
				{
					cmd = list_pop(cmdList);
					if(cmd != NULL)
					{
						*cmdSend = *cmd->pt;
						InitTimeout(taskTimeout, SYSTICK_TIME_MS(cmdSend->delay));
						*ampmCmdPhase = AMPM_CMD_SEND;
					}
				}
				else
					list_init(cmdList);
			}
			break;
		default:
			*ampmCmdPhase = AMPM_CMD_ERROR;
			break;
	}
}

void StrParserInit(STR_PARSER_Type *process,uint8_t *numCharParsed,uint8_t strParserCntMax,uint16_t dataLengthMax)
{
	uint8_t i;
	process->state = STR_FINISH;
	process->dataRecvLength = 0;
	process->strParserCntMax = strParserCntMax;
	process->timeout = 0;
	process->timeoutSet = 10000;// 10sec
	process->dataRecvMaxLength = dataLengthMax;
	for(i = 0; i < process->strParserCntMax;i++)
	{
		numCharParsed[i] = 0;
	}
}

void StrParserCtl(uint32_t sysTimeMs,STR_PARSER_Type *process,uint8_t *numCharParsed)
{	
	uint8_t i;
	if(process->state == STR_FINISH) 
		process->timeout = sysTimeMs;
	else
	{
		if(sysTimeMs- process->timeout >= process->timeoutSet)
		{
			process->timeout = sysTimeMs;
			process->state = STR_FINISH;
			for(i = 0; i < process->strParserCntMax;i++)
			{
				numCharParsed[i] = 0;
			}
		}
	}
}

void StrComnandParser(uint32_t sysTimeMs,const STR_INFO_Type *info,STR_PARSER_Type *process,uint8_t *numCharParsed,uint8_t c)
{
	uint32_t i; 
	static uint8_t lastChar = 0;
	StrParserCtl(sysTimeMs,process,numCharParsed);
	switch(process->state)
	{
		case STR_FINISH:
			for(i = 0; i < process->strParserCntMax;i++)
			{
				if(c == info[i].command[numCharParsed[i]] || info[i].command[numCharParsed[i]] == '\t')
				{
						lastChar = c;
						numCharParsed[i]++;
						if(info[i].command[numCharParsed[i]] == '\0')
						{
								process->strInProcess = i;
								process->state = STR_WAIT_FINISH;
								process->dataRecvLength = 0;
						}
				}
				else if(lastChar == info[i].command[0])
				{
						numCharParsed[i] = 1;
				}
				else
					numCharParsed[i] = 0;
			}
		break;
		case STR_WAIT_FINISH:				
			if(info[process->strInProcess].callback(process->dataRecvLength,c) == 0)
			{
				process->state = STR_FINISH;
				//process->timeoutSet = 10000;
				for(i = 0; i < process->strParserCntMax;i++)
				{
					numCharParsed[i] = 0;
				}
			}
			process->dataRecvLength++;
			if(process->dataRecvLength >= process->dataRecvMaxLength)
			{
				process->state = STR_FINISH;
				for(i = 0; i <  process->strParserCntMax;i++)
				{
					numCharParsed[i] = 0;
				}
			}
			break;
		default:
			process->state = STR_FINISH;
			for(i = 0; i <  process->strParserCntMax;i++)
			{
				numCharParsed[i] = 0;
			}
			break;
	}	
}

