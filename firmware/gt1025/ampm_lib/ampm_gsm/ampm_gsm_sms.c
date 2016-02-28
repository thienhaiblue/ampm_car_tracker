/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"
#include "ampm_gsm_sms.h"
#include "lib/list.h"
#include "sms/pdu.h"
#include "at_command_parser.h"


#define INFO(...)	//PrintfDebug(__VA_ARGS__)

#define smsScanf	sscanf



uint32_t Ampm_GetServiceCenterNumber(uint16_t cnt,uint8_t c);

LIST(smsList);

Timeout_Type tSmsTaskTimeOut;
SMS_LIST_TYPE *smsMsg;

uint8_t smsNewMessageFlag = 0;
uint8_t smsUnreadBuff[32] = {0};
RINGBUF smsUnreadRingBuff;

uint8_t smsSender[16];
uint8_t smsDataBuf[256];
uint8_t pdu_len_except_smsc;
int16_t pdu_len;

void (*Ampm_SmsRecvCallback)(uint8_t *buf);

uint8_t smsCancelAll = 0;
uint8_t flagSmsReadFinish = 0;
uint8_t serviceCenterNumber[16];
uint8_t getServiceCenterNumber = 0;
uint8_t smsTaskFailCnt = 0;
uint8_t readTryCnt = 0;
uint16_t smsIndex = 0;



void Ampm_SmsAddMsg(SMS_LIST_TYPE *msg);
uint8_t Ampm_GsmSmsRecvCallback_CMGR(uint8_t *buf);
uint8_t Ampm_GsmSmsRecvCallback_CMGD(uint8_t *buf);
uint8_t Ampm_GsmSmsRecv0(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmSmsRecv1(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmSmsRecv2(AMPM_CMD_PHASE_TYPE phase);

uint8_t Ampm_GsmSmsSendCallback_PreCMGS(uint8_t *buf);
uint8_t Ampm_GsmSmsSendCallback_CMGF(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmSmsSendCallback_CSCA(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmSmsSendMessageCallback(uint8_t *buf);
uint8_t Ampm_GsmSmsSendCallback_CMGS(uint8_t *buf);
uint8_t Ampm_GsmSmsSent(AMPM_CMD_PHASE_TYPE phase);
uint8_t Ampm_GsmCREG(AMPM_CMD_PHASE_TYPE phase);

uint8_t smsTryCnt = 0;

const AMPM_GSM_AT_CMD_PACKET_TYPE smsRecvCmd_AT_CMGR = {NULL,Ampm_GsmSmsRecvCallback_CMGR,NULL,NULL,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE smsRecvCmd_AT_CMGD = {NULL,Ampm_GsmSmsRecvCallback_CMGD,NULL,NULL,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_end	= {
		NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_5	= {
		NULL,(void *)&smsRecvCmd_AT_CMGD,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsRecv1,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_4	= {
		(void *)&smsRecvCmdProcess_5,(void *)&smsRecvCmd_AT_CMGR,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsRecv2,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_3	= {
		(void *)&smsRecvCmdProcess_4,(void *)&ampmAtCmd_AT_CMGL,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsRecv1,1,1000,1000};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_2	= {
		(void *)&smsRecvCmdProcess_3,(void *)&ampmAtCmd_AT_CMGF_1,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsRecv0,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_1	= {
		(void *)&smsRecvCmdProcess_2,(void *)&ampmAtCmd_AT_CLIP,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsRecvCmdProcess_0	= {
		(void *)&smsRecvCmdProcess_1,(void *)&ampmAtCmd_AT_CSQ,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

//send SMS
const AMPM_GSM_AT_CMD_PACKET_TYPE smsSendCmd_AT_CSCA = {"AT+CSCA?\r",NULL,"+CSCA: \"",Ampm_GetServiceCenterNumber,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE smsSendCmd_AT_CMGS = {NULL,Ampm_GsmSmsSendCallback_CMGS,NULL,NULL,"\r\r\n>","ERROR"};//delay = 500,timeout = 3000,retry = 1
const AMPM_GSM_AT_CMD_PACKET_TYPE smsSendMessage = {NULL,Ampm_GsmSmsSendMessageCallback,NULL,NULL,"OK","ERROR"};//delay = 500,timeout = 3000,retry = 1

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_end	= {
		NULL,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_6	= {
		NULL,(void *)&ampmAtCmd_AT_CREG,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmCREG,1,3000,100};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_5	= {
		(void *)&smsSendCmdProcess_end,(void *)&smsSendMessage,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsSent,1,10000,100};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_4	= {
		(void *)&smsSendCmdProcess_5,(void *)&smsSendCmd_AT_CMGS,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,5000,500};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_3	= {
		(void *)&smsSendCmdProcess_4,(void *)&smsSendCmd_AT_CSCA,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,NULL,1,5000,500};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_2	= {
		(void *)&smsSendCmdProcess_3,(void *)&ampmAtCmd_AT_CMGF_0,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsSendCallback_CSCA,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_1	= {
		(void *)&smsSendCmdProcess_4,(void *)&ampmAtCmd_AT_CMGF_1,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsSendCallback_CSCA,1,1000,100};

const AMPM_CMD_PROCESS_TYPE smsSendCmdProcess_0	= {
		(void *)&smsSendCmdProcess_1,(void *)&ampmAtCmd_AT,Ampm_GsmSendCallback,Ampm_GsmRecvCallback,Ampm_GsmSmsSendCallback_CMGF,1,1000,100};


uint8_t Ampm_GsmSmsSendCallback_CMGF(AMPM_CMD_PHASE_TYPE phase)
{
	if(smsMsg->mode == SMS_TEXT_MODE)
	{
		Ampm_SendAtCmdNow(&smsSendCmdProcess_1);
	}
	else
	{
		Ampm_SendAtCmdNow(&smsSendCmdProcess_2);
	}
	return 1;
}

uint8_t Ampm_GsmSmsSendCallback_CSCA(AMPM_CMD_PHASE_TYPE phase)
{
	if(smsMsg->msg != NULL && smsMsg->phone != NULL)
	{
		if(smsMsg->mode != SMS_TEXT_MODE)
		{
			getServiceCenterNumber = 0;
			Ampm_SendAtCmdNow(&smsSendCmdProcess_3);
		}
	}
	else
	{
		Ampm_SendAtCmdNow(&smsSendCmdProcess_end);
	}
	return 1;
}

uint8_t Ampm_GsmSmsSent(AMPM_CMD_PHASE_TYPE phase)
{
	if(phase == AMPM_CMD_OK)
	{
		smsMsg->phone = NULL;
		smsMsg->msg = NULL;
		smsMsg->flag = SMS_MSG_SENT;
		Ampm_SendAtCmdNow(&smsSendCmdProcess_end);
		smsTaskFailCnt = 0;
	}
	else
	{
		COMM_Putc(0x1B);
		smsTaskFailCnt++;
		Ampm_SendAtCmdNow(&smsSendCmdProcess_6);
	}
	return 1;
}

uint8_t Ampm_GsmCREG(AMPM_CMD_PHASE_TYPE phase)
{
	if(phase == AMPM_CMD_ERROR)
	{
		smsTaskFailCnt++;
	}
	return 1;
}

uint8_t Ampm_GsmSmsSendMessageCallback(uint8_t *buf)
{
	uint16_t i;
	if(smsMsg->mode == SMS_TEXT_MODE)
	{
		ampm_sprintf((char *)buf,"%s%c",smsMsg->msg,0x1A/*Send Ctl-Z*/);
	}
	else
	{
		for (i = 0; i < pdu_len; ++i) 
		{	
			ampm_sprintf((char *)&buf[i*2], "%02X", smsDataBuf[i]);
		}
		buf[i*2] = 0x1A;// Send Ctl-Z
		buf[i*2 + 1] = 0;
	}
	return 1;
}

uint8_t Ampm_GsmSmsSendCallback_CMGS(uint8_t *buf)
{
	uint8_t res = 0;
	if((smsMsg->msg[0] != NULL || smsMsg->mode != SMS_TEXT_MODE) && smsMsg->phone[0] != NULL)
	{
		if(smsMsg->mode == SMS_TEXT_MODE)
		{
			res = ampm_sprintf((char *)buf,"AT+CMGS=\"%s\"\r",smsMsg->phone);
		}
		else
		{
			if(getServiceCenterNumber && serviceCenterNumber[0] != NULL)
			{
				if(smsMsg->mode == SMS_PDU16_MODE)
					pdu_len = pdu16_encode((const char *)serviceCenterNumber,(const char *)smsMsg->phone,( uint16_t *)smsMsg->msg,smsMsg->len,smsDataBuf,sizeof(smsDataBuf));
				else
					pdu_len = pdu_encode((const char *)serviceCenterNumber,(const char *)smsMsg->phone,(const char*)smsMsg->msg,smsDataBuf,sizeof(smsDataBuf));
				if(pdu_len < 0) //"error encoding to PDU"
				{
					smsMsg->phone = NULL;
					smsMsg->msg = NULL;
					smsMsg->flag = SMS_MSG_FAIL;
					smsTaskFailCnt++;
					Ampm_SendAtCmdNow(&smsSendCmdProcess_end);
					return 0;
				}
				pdu_len_except_smsc = pdu_len - 1 - smsDataBuf[0];
				res =  ampm_sprintf((char *)buf,"AT+CMGS=%d\r",pdu_len_except_smsc);
			}
		}
		Ampm_SendAtCmdNow(&smsSendCmdProcess_4);
		return res;
	}
	else
	{
		smsMsg->phone = NULL;
		smsMsg->msg = NULL;
		smsMsg->flag = SMS_MSG_SENT;
		Ampm_SendAtCmdNow(&smsSendCmdProcess_end);
		smsTaskFailCnt++;
	}
	return 0;
}



uint8_t Ampm_GsmSmsRecvCallback_CMGR(uint8_t *buf)
{
	InitTimeout(&tSmsTaskTimeOut, SYSTICK_TIME_SEC(3));
	return (uint8_t )ampm_sprintf((char *)buf,"AT+CMGR=%d\r",smsIndex);
}

uint8_t Ampm_GsmSmsRecvCallback_CMGD(uint8_t *buf)
{
	return (uint8_t )ampm_sprintf((char *)buf,"AT+CMGD=%d\r",smsIndex);
}

uint8_t Ampm_GsmSmsRecv0(AMPM_CMD_PHASE_TYPE phase)
{
	uint8_t c;
	/*send:  NULL,Ampm_GsmSmsRecvCallback0,NULL,NULL,"OK","ERROR",//delay = 500,timeout = 3000,retry = 1*/
	if(RINGBUF_GetFill(&smsUnreadRingBuff) >=2)
	{
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsIndex = c;
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsIndex |= (((uint16_t)c) << 8 & 0xff00);
		if(smsIndex >= 1000)
		{
			Ampm_SendAtCmdNow(&smsRecvCmdProcess_5);
		}
		else
		{
			readTryCnt = 3;
			Ampm_SendAtCmdNow(&smsRecvCmdProcess_4);
		}
	}
	else
	{
		Ampm_SendAtCmdNow(&smsRecvCmdProcess_3);
	}
	return 1;
}

uint8_t Ampm_GsmSmsRecv1(AMPM_CMD_PHASE_TYPE phase)
{
	uint8_t c;
	/*send:  NULL,Ampm_GsmSmsRecvCallback0,NULL,NULL,"OK","ERROR",//delay = 500,timeout = 3000,retry = 1*/
	if(RINGBUF_GetFill(&smsUnreadRingBuff) >=2)
	{
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsIndex = c;
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsIndex |= (((uint16_t)c) << 8 & 0xff00);
		if(smsIndex >= 1000)
		{
			Ampm_SendAtCmdNow(&smsRecvCmdProcess_5);
		}
		else
		{
			readTryCnt = 3;
			Ampm_SendAtCmdNow(&smsRecvCmdProcess_4);
		}
	}
	else{
		Ampm_SendAtCmdNow(&smsRecvCmdProcess_end);
	}
	return 1;
}

uint8_t Ampm_GsmSmsRecv2(AMPM_CMD_PHASE_TYPE phase)
{
	if(flagSmsReadFinish || (CheckTimeout(&tSmsTaskTimeOut) == SYSTICK_TIMEOUT))
	{
		if(flagSmsReadFinish)
		{
			Ampm_SmsRecvCallback(smsDataBuf);
			Ampm_SendAtCmdNow(&smsRecvCmdProcess_5);
		}
		else
		{
			if(readTryCnt) 
			{
				readTryCnt--;
				COMM_Putc(0x1B);
				InitTimeout(&tSmsTaskTimeOut, SYSTICK_TIME_SEC(3));
				Ampm_SendAtCmdNow(&smsRecvCmdProcess_4);
			}
			else
			{
				Ampm_SendAtCmdNow(&smsRecvCmdProcess_5);
			}
		}
		return 1;
	}
	return 0;
}


uint8_t Ampm_StartRecvSms(void)
{
		return Ampm_SendAtCmd(&smsRecvCmdProcess_0);
}

uint8_t Ampm_RecvSms_IsFinished(void)
{
	if(Ampm_SendAtCheck_IsEmpty() && Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase)){
		return 1;
	}
	return 0;
}


void Ampm_SmsTask_Init(void (*callback)(uint8_t *buf))
{
	Ampm_SmsRecvCallback = callback;
	list_init(smsList);
	getServiceCenterNumber = 0;
	RINGBUF_Init(&smsUnreadRingBuff,smsUnreadBuff,sizeof(smsUnreadBuff));
}

uint8_t Ampm_SmsTaskPeriodic_1Sec(void)
{
	uint8_t res = 0;
	SMS_LIST_TYPE *msg;
	if(list_length(smsList) == 0)
			return 0;
	msg = list_head(smsList);
	while(msg != NULL)
	{
		if(msg->flag == SMS_NEW_MSG
			&& msg->msg != NULL 
			&& msg->phone != NULL 
			&& msg->tryNum
		)
		{
			if(msg->timeout){ msg->timeout--;}
			else {res = 1;}
		}
		else
		{
				list_remove(smsList,msg);
				msg = list_item_next(msg);
		}
		msg = list_item_next(msg);
	}
	return res;
}

uint8_t Ampm_SmsTask(void)
{
	static uint8_t checkCnt  = 0;
	static SMS_LIST_TYPE *msg = NULL;
	if(Ampm_SendAtCheck_IsEmpty() 
	&& Ampm_CmdTask_IsIdle(ampm_GSM_CmdPhase))
	{
		checkCnt++;
		if(checkCnt >= 10)
		{
			checkCnt = 0;
			return 1;
		}
		if(list_length(smsList))
		{
			msg = list_head(smsList);
			while(msg != NULL)
			{
				checkCnt = 0;
				if(msg->mode == SMS_TEXT_MODE)
				{
					if(msg->msg[0] == 0){
						msg->msg = NULL;
					}
				}
				else if(msg->len == 0)
				{
					msg->msg = NULL;
				}
				
				if(msg->flag == SMS_NEW_MSG
					&& msg->msg != NULL 
					&& msg->phone != NULL 
				)
				{
					if(msg->timeout == 0)
					{
						msg->timeout = msg->timeoutReload;
						if(msg->tryNum){msg->tryNum--;}
						smsMsg = msg;
						Ampm_SendAtCmdNow(&smsSendCmdProcess_0);
					}
					else
					{
						msg = list_item_next(msg);
					}
				}
				else
				{
					list_remove(smsList,msg);
					msg = list_item_next(msg);
				}
			}
		}
		else
		{
				return 1;
		}
	}
	return 0;
}
//Check SMS need to send out
uint8_t Ampm_SmsCheckMessage_IsEmpty(void)
{
	if(list_length(smsList))
		return 0;
//	SMS_LIST_TYPE *sms;
//	sms = list_head(smsList);
//	while(sms != NULL)
//	{
//		if(sms->flag == SMS_NEW_MSG && sms->msg != NULL && sms->phone != NULL)
//		{
//			return 0;
//		}
//		sms = list_item_next(sms);
//	}
	return 1;
}

void Ampm_SmsAddMsg(SMS_LIST_TYPE *msg)
{
	list_add(smsList, msg);
}


uint16_t Ampm_Sms_SendMsg(SMS_LIST_TYPE *SMS,uint8_t *phone,uint8_t *msg,uint8_t len,SMS_MODE_TYPE mode,uint16_t timeout,uint8_t tryNum)
{
		SMS->msg = msg;
		SMS->len = len;
		SMS->phone = phone;
		SMS->timeout = 0;
		SMS->timeoutReload = timeout;
		SMS->tryNum = tryNum;
		if(mode != SMS_TEXT_MODE && (len == 0))
		{
				SMS->flag = SMS_MSG_SENT;
				return 0xff;
		}
		if(SMS->phone[0] != NULL)
			SMS->flag = SMS_NEW_MSG;
		else
		{
			SMS->flag = SMS_MSG_SENT;
			return 0xff;
		}
		SMS->mode = mode;
		Ampm_SmsAddMsg(SMS);
		return 0;
}


/*+CMGR: "REC READ","+841645715282","","12/07/26,20:50:07+28"
"thienhailue"
*/

uint32_t Ampm_SMS_ReadMsg(uint16_t cnt,uint8_t c)
{ 
	static uint8_t comma = 0,getSmsDataFlag = 0;
	static uint8_t *u8pt;
	static uint8_t *u8pt1;
	static uint8_t *u8pt2;
	if(cnt == 0)
	{
		comma = 0;
		getSmsDataFlag = 0;
		u8pt = smsSender;
		u8pt2 = smsDataBuf;
		u8pt1 = (uint8_t *)terminateStr;
		return 0xff;
	}
	if(c == ',') 
	{
		comma++;
	}
	
	if(getSmsDataFlag)
	{
		if(c == *u8pt1)
		{
			u8pt1++;
			if(*u8pt1 == '\0')
			{
				if(u8pt2 - smsDataBuf > sizeof(terminateStr))
				{
					u8pt2 = u8pt2 - sizeof(terminateStr);
				}
				*u8pt2 = 0;
				flagSmsReadFinish = 1;
				return 0;
			}
		}
		else
		{
			u8pt1 = (uint8_t *)terminateStr;
			if(c == *u8pt1) u8pt1++;
		}
		if((u8pt2 - smsDataBuf) >= sizeof(smsDataBuf))
		{
			if(u8pt2 - smsDataBuf > sizeof(terminateStr))
			{
				u8pt2 = u8pt2 - sizeof(terminateStr);
			}
			*u8pt2 = 0;
			flagSmsReadFinish = 1;
			return 0;
		}
		*u8pt2 = c;
		 u8pt2++;
	}
	else
	{
		switch(comma)
		{
			case 0:
				break;
			case 1:
				if((u8pt - smsSender) >= sizeof(smsSender))
				{
					smsSender[0] = 0;
					return 0;
				}
				if(((c >= '0') && (c <= '9')) || (c == '+'))
				{
					*u8pt = c;
					u8pt++;
					*u8pt = 0;
				}
				break;
			default:
				break;
		}
	}
	if(c == '\n')
	{
		getSmsDataFlag = 1;
	}
	return 0xff;
}


/*
+CSCA: "+886931000099",145
*/
uint32_t Ampm_GetServiceCenterNumber(uint16_t cnt,uint8_t c)
{
	static uint8_t len = 0;
	if(cnt == 0)
		len = 0;
	if((c >= '0') && (c <= '9') || (c == '+'))
	{
			if(c != '+')
			{
			 serviceCenterNumber[len] = c;
			 len++;
			 serviceCenterNumber[len] = '\0';
			}
	}
	else 
	{
		if(c == '"' && len >= 2)
		{
			getServiceCenterNumber = 1;
		}
		return 0;
	}
 return 0xff;
}

/*   14,"REC READ","+84972046096","","12/07/26,11:10:17+28"   */
/*
AT+CMGL="ALL"
+CMGL: 1,"REC READ","+84978779222",,"14/05/26,08:39:57+28"
*88:88#  
*/


/*+CMTI: "ME",1*/
uint32_t Ampm_SMS_URC_Incomming(uint16_t cnt,uint8_t c)
{ 
	static uint32_t length = 0;
	switch(cnt)
	{
		case 0:
				if((c >= '0') && (c <= '9'))
				{
					length = c - '0';
					break;
				}
				return 0;
		case 1:
		case 2:
		case 3:
				if((c >= '0') && (c <= '9'))
				{
					length *= 10;
					length += c - '0';
					break;
				}
				else
				{
					RINGBUF_Put(&smsUnreadRingBuff,(uint8_t)(length & 0x00ff));
					RINGBUF_Put(&smsUnreadRingBuff,(uint8_t)((length >> 8) & 0x00ff));
					return 0;
				}
		default:
			return 0;
	}
	return 0xff;
}



/*   14,"REC READ","+84972046096","","12/07/26,11:10:17+28"   */
/*
AT+CMGL="ALL"
+CMGL: 1,"REC READ","+84978779222",,"14/05/26,08:39:57+28"
*88:88#  
*/
uint32_t Ampm_SMS_Process(uint16_t cnt,uint8_t c)
{ 
	static uint32_t length = 0;
	switch(cnt)
	{
		case 0:
				Ampm_SetAtCmti();
				if((c >= '0') && (c <= '9'))
				{
					length = c - '0';
					break;
				}
				return 0;
		case 1:
		case 2:
		case 3:
				if((c >= '0') && (c <= '9'))
				{
					length *= 10;
					length += c - '0';
					break;
				}
				else
				{
					RINGBUF_Put(&smsUnreadRingBuff,(uint8_t)(length & 0x00ff));
					RINGBUF_Put(&smsUnreadRingBuff,(uint8_t)((length >> 8) & 0x00ff));
					return 0;
				}	
		default:
			return 0;
	}
	return 0xff;
}


uint8_t Ampm_SendSmsPolling(const uint8_t *recipient, const uint8_t *data)
{
	uint8_t buf[32];
	uint8_t i,c;
	SysTick_DelayMs(100);
	while(RINGBUF_Get(commRxRingBuf, &c)==0);
	sprintf((char *)buf, "AT+CMGS=\"%s\"\r", recipient);
	COMM_Puts(buf);
	if(Ampm_AtCmdCheckResponse("> ", 10000))
	{
		return 0xff;
	}
	
	for(i = 0;i < 160;i++)
	{
		if(data[i] == '\0')
			break;
		//DbgCfgPutChar(data[i]);
		COMM_Putc(data[i]);
	}
	COMM_Putc(0x1A);
	
	if(Ampm_AtCmdCheckResponse("OK", 10000)){
		return 0xff;
	}
	return 0;
}




