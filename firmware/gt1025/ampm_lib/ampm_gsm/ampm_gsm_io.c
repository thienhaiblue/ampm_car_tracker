/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_io.h"
#include "lib/ampm_sprintf.h"

const uint8_t *modemOk = "OK";
const uint8_t *modemError = "ERROR";


AMPM_GSM_AT_CMD_PACKET_TYPE *ampm_GSM_AtCmd;

uint8_t *ampm_GprsApn = "internet";
void *ampm_GSM_CmdList_list = NULL; 
list_t ampm_GSM_CmdList = (list_t)&ampm_GSM_CmdList_list;
AMPM_CMD_PHASE_TYPE ampm_GSM_CmdPhase;
AMPM_CMD_PROCESS_TYPE ampmGSM_CmdSend;
Timeout_Type ampm_GSM_TaskTimeout;


AMPM_GSM_AT_CMD_PACKET_TYPE ampmCommonAppStruct;
//AMPM_CMD_LIST_TYPE ampmCommonCmdList;
//AMPM_CMD_PROCESS_TYPE ampmCommonCmd;
uint8_t  ampmAtCmdBuf[256];

COMPARE_TYPE ampmGSM_CmdOk, ampmGSM_CmdFails;

STR_PARSER_Type ampm_AtCallBackParser;
uint8_t ampm_CmdParsedChar[1] =  {0};
STR_INFO_Type ampm_AtProcessCmd[1];

uint8_t gotoCmdModeFlag = 0;
uint8_t gotoDataModeFlag = 0;

void Ampm_GsmGotoCmdMode(void)
{
	if(gotoCmdModeFlag == 0)
	{
		gotoCmdModeFlag = 1;
	}
}

void Ampm_GsmGotoDataMode(void)
{
	if(gotoDataModeFlag == 0)
	{
		gotoDataModeFlag = 1;
	}
}

void Ampm_GsmSetApn(uint8_t *apn)
{
	ampm_GprsApn = apn;
}


uint8_t Ampm_SendAtCheck_IsEmpty(void)
{
	return Ampm_CmdCheck_IsEmpty(&ampmGSM_CmdSend);
}


uint8_t Ampm_AtCmdCheckResponse(uint8_t *str,uint32_t t)
{
	COMPARE_TYPE cmp;
	uint8_t c;
	InitFindData(&cmp,str);
	while(t--)
	{
		SysTick_DelayMs(1);
		if(RINGBUF_Get(commRxRingBuf,&c) == 0)
		{
			if(FindData(&cmp,c) == 0)
			{
				return 0;
			}
		}
	}
	return 0xff;
}

uint8_t Ampm_SendCommand(const uint8_t *resOk,const uint8_t *resFails,uint32_t timeout,uint8_t tryAgainNum,const uint8_t *format, ...)
{
	static  uint8_t  buffer[128];
	COMPARE_TYPE cmp1,cmp2;
	Timeout_Type tOut;
	uint8_t c;
        #if defined ( __ICCARM__ )
        va_list     vArgs;
        #elif defined (__CC_ARM)
         __va_list     vArgs;
        #endif
	
	
	va_start(vArgs, format);
	vsprintf((char *)&buffer[0], (char const *)format, vArgs);
	va_end(vArgs);
	COMM_Putc(0x1A);
	COMM_Putc(0x1A);
	if (tryAgainNum == 0)
	{
		tryAgainNum = 1;
	}
	while (tryAgainNum)
	{
		SysTick_DelayMs(100);
		InitFindData(&cmp1, (uint8_t *)resOk);
		InitFindData(&cmp2, (uint8_t *)resFails);
		InitTimeout(&tOut, SYSTICK_TIME_MS(timeout));
		while(RINGBUF_Get(commRxRingBuf, &c)==0);
		MODEM_Info("\r\nGSM->%s\r\n", buffer);
		COMM_Puts(buffer);
		while(CheckTimeout(&tOut))
		{
			if(RINGBUF_Get(commRxRingBuf, &c) == 0)
			{
				if(FindData(&cmp1, c) == 0)
				{
					MODEM_Info("\r\nGSM->%s OK\r\n",buffer);
					return 0;
				}
				if(FindData(&cmp2, c) == 0)
				{
					break;
				}
			}
		}
		tryAgainNum--;
	}
	MODEM_Info("\r\nGSM->%s FAILS\r\n",buffer);
	return 0xff;
}

uint8_t Ampm_SetCmdOK(AMPM_CMD_PHASE_TYPE phase)
{
	ampm_GSM_CmdPhase = AMPM_CMD_OK;
	return 0;
}

uint8_t Ampm_SendAtCmd(const AMPM_CMD_PROCESS_TYPE *cmd)
{
	return Ampm_SendCmd(&ampm_GSM_CmdPhase,&ampmGSM_CmdSend,cmd,&ampm_GSM_TaskTimeout);
}

void Ampm_SendAtCmdNow(const AMPM_CMD_PROCESS_TYPE *cmd)
{
	Ampm_SendCmdNow(&ampm_GSM_CmdPhase,&ampmGSM_CmdSend,cmd,&ampm_GSM_TaskTimeout);
}

uint8_t Ampm_CmdTask_SendCmd(uint32_t (*callback)(uint16_t cnt,uint8_t c),uint8_t *response,
	uint32_t delay, const uint8_t *resOk, const uint8_t *resFails, 
	uint32_t timeout, uint8_t tryAgainNum, const uint8_t *format, ...)
{
		uint8_t c;
   va_list     vArgs;

	if(Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase))
	{
		while(RINGBUF_Get(commTxRingBuf, &c)==0);
		if(format != NULL)
		{
			va_start(vArgs, format);
			ampm_vsprintf((char *)&ampmAtCmdBuf[0], (char const *)format, vArgs);
			va_end(vArgs);
		} 
		else
		{
			ampmAtCmdBuf[0] = 0;
		}
		ampmCommonAppStruct.response = response;
		ampmCommonAppStruct.callback = callback;
		ampmCommonAppStruct.resOk = (uint8_t *)resOk;
		ampmCommonAppStruct.resFails = (uint8_t *)resFails;
		ampmCommonAppStruct.command = ampmAtCmdBuf;
		ampmCommonAppStruct.sprintf = NULL;
                ampmGSM_CmdSend.next = NULL;
		ampmGSM_CmdSend.appStruct = &ampmCommonAppStruct;
		ampmGSM_CmdSend.delay = delay;
		ampmGSM_CmdSend.timeout = timeout;
		ampmGSM_CmdSend.retryNum = tryAgainNum;
		ampmGSM_CmdSend.end_data_callback = NULL;
		ampmGSM_CmdSend.send_data_callback = Ampm_GsmSendCallback;
		ampmGSM_CmdSend.recv_data_callback = Ampm_GsmRecvCallback;
		//ampmCommonCmdList.pt = &ampmCommonCmd;
		InitTimeout(&ampm_GSM_TaskTimeout, SYSTICK_TIME_MS(ampmGSM_CmdSend.delay));
		ampm_GSM_CmdPhase = AMPM_CMD_SEND;
		return AMPM_GSM_RES_OK;
	}
	return AMPM_GSM_RES_FAILS;
}

uint32_t Ampm_GsmSendCallback(void)
{
	uint8_t c;
	ampm_GSM_AtCmd = (AMPM_GSM_AT_CMD_PACKET_TYPE *)ampmGSM_CmdSend.appStruct;
	while(RINGBUF_Get(commRxRingBuf, &c)==0);
	if(ampm_GSM_AtCmd->sprintf)
	{
		ampm_GSM_AtCmd->sprintf(ampmAtCmdBuf);
		ampmAtCmdBuf[255] = 0;
		COMM_Puts(ampmAtCmdBuf);
	}
	else
	{
		COMM_Puts(ampm_GSM_AtCmd->command);
	}
	InitFindData(&ampmGSM_CmdOk, (uint8_t *)ampm_GSM_AtCmd->resOk);
	InitFindData(&ampmGSM_CmdFails, (uint8_t *)ampm_GSM_AtCmd->resFails);
	if(ampm_GSM_AtCmd->callback != NULL && ampm_GSM_AtCmd->response != NULL)
	{
		StrParserInit(&ampm_AtCallBackParser,ampm_CmdParsedChar,1,1024);
		ampm_AtCallBackParser.timeoutSet = ampmGSM_CmdSend.timeout;
		ampm_AtProcessCmd[0].callback  = ampm_GSM_AtCmd->callback;
		ampm_AtProcessCmd[0].command  = ampm_GSM_AtCmd->response;
	}
	return 1;
}

uint32_t Ampm_GsmRecvCallback(void)
{
	uint8_t c;
	while (RINGBUF_Get(commRxRingBuf, &c) == 0)
	{
		AT_CMD_Debug("%c",c);
		if ((c != 0x1A) && (c != 0x1B))
		{
			if(ampm_GSM_AtCmd->callback != NULL && ampm_GSM_AtCmd->response != NULL)
			{
				StrComnandParser(SysTick_Get(),ampm_AtProcessCmd,&ampm_AtCallBackParser,ampm_CmdParsedChar,c);
			}
			if (FindData(&ampmGSM_CmdOk, c) == 0)
			{
				return AMPM_CMD_DELAY;
			}
			else if (FindData(&ampmGSM_CmdFails, c) == 0)
			{
				return AMPM_CMD_RESEND;
			}
		}
	}
	return AMPM_CMD_WAIT;
}








