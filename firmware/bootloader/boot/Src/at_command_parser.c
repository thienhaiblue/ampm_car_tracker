#include "at_command_parser.h"
#include "rtc.h"
#include "db.h"
#include "gps.h"
#include "db.h"
#include "sst25.h"
#define INFO(...)	DbgCfgPrintf(__VA_ARGS__)

#define smsScanf	sscanf

#define CMD_COUNT   (sizeof (AT_ProcessCmd) / sizeof (AT_ProcessCmd[0]))
#define AT_CMD_PARSER_SIZE	16

const uint8_t terminateStr[7] = "\r\nOK\r\n";

char GetCallerNumber(char c);
char RingCallProcess(char c);
char SMS_NewMessage(char c);
char SMS_Process(char c);
char GPRS_GetLength(char c);
char GPRS_GetData(char c);
char CLOSE_TCP_Process(char c);
char Parse(char *buff,uint8_t *smsSendBuff,uint32_t smsLenBuf);
char GPRS_CloseSocket(char c);
char GPRS_CloseSocket1(char c);
char GPRS_SocketControl(char c);
char SMS_ReadMsg(char c);
char CellLocateParser(char c);
char GPRS_CreateSocket(char c);
char CallBusy(char c);
char GPRS_GetDNS(char c);

/* Command definitions structure. */
typedef struct scmd {
   char cmdInfo[16];
   char (*func)(char c);
} SCMD;

static const SCMD AT_ProcessCmd[] = {
	"+CLIP: \"", GetCallerNumber,
	"RING" , RingCallProcess,
  "+CMGL: ", SMS_Process,
	//"+CMTI:", SMS_NewMessage,
	"+UUSORD: ", GPRS_GetLength,
	"+USORD: ", GPRS_GetData,
	"+UUSOCL: ", GPRS_CloseSocket1,
	"+USOWR: ", GPRS_CloseSocket,
	"+USOCTL: ",GPRS_SocketControl,
	"+CMGR: ",SMS_ReadMsg,
	"+USOCR: ",GPRS_CreateSocket,
	"BUSY",CallBusy,
	"NO ANSWER",CallBusy,
	"NO CARRIER",CallBusy,
	"+CMTI: \"SM\",",SMS_Process,
	"+CMTI: \"ME\",",SMS_Process,
	"+UDNSRN: \"",GPRS_GetDNS
	//"+UULOC: ",CellLocateParser
};

uint32_t cmdCnt[CMD_COUNT];
uint8_t numCmd;



enum{
	AT_CMD_NEW_STATE,
	AT_CMD_WAIT_FINISH,
	AT_CMD_FINISH
}AT_cmdState = AT_CMD_FINISH;

uint8_t callerPhoneNum[16];

uint8_t smsBuf[255];
uint8_t smsSender[18];
uint32_t smsCnt = 0,smsNewMessageFlag = 0,ringFlag = 0;
uint8_t inCalling = 0;

uint8_t smsUnreadBuff[32] = {0};
RINGBUF smsUnreadRingBuff;
uint8_t *gprsRecvPt;
uint32_t gprsRecvDataLen;
uint8_t gprsLengthBuff[SOCKET_NUM][GPRS_KEEP_DATA_INFO_LENGTH] = {0};
RINGBUF gprsRingLengthBuff[SOCKET_NUM];

uint32_t gprsDataOffset = 0,tcpSocketStatus[SOCKET_NUM] = {SOCKET_CLOSE,SOCKET_CLOSE};
uint32_t GPRS_dataUnackLength[SOCKET_NUM];
uint8_t socketMask[SOCKET_NUM];
uint8_t createSocketNow = 0;
uint32_t AT_cmdRecvLength;
uint32_t socketRecvEnable = 0;
uint32_t socketNo = 0;
uint32_t tcpTimeReCheckData[SOCKET_NUM] = {0xffffffff};
uint8_t DNS_serverIp[16];

void AT_CmdProcessInit(void)
{
	uint32_t i;
	RINGBUF_Init(&smsUnreadRingBuff,smsUnreadBuff,sizeof(smsUnreadBuff));
	for(i = 0;i < SOCKET_NUM; i++)
	{
		RINGBUF_Init(&gprsRingLengthBuff[i],gprsLengthBuff[i],GPRS_KEEP_DATA_INFO_LENGTH);
	}
}


char CallBusy(char c)
{
	inCalling = 0;
 return 0;
}

/*
	+UULOC: 27/09/2012,18:26:13.363,21.0213870,105.8091050,0,127,0,0 
*/


char CellLocateParser(char c)
{
	if(c != '\r')
	{
		
	}
	return 0;
}
/*
+CLIP: "0978779222",161,,,,0
*/
char GetCallerNumber(char c)
{
	if((c >= '0') && (c <= '9'))
		callerPhoneNum[AT_cmdRecvLength] = c;
	else 
	{
		ringFlag = 1;
		return 0;
	}
 return 0xff;
}

char RingCallProcess(char c)
{
	return 0;
}

char SMS_NewMessage(char c)
{
	smsNewMessageFlag = 1;
	return 0;
}
/*
AT+USOCTL=0,11 +USOCTL: 0,11,2501
*/
char GPRS_SocketControl(char c)
{
	static uint32_t length = 0;
	switch(AT_cmdRecvLength)
	{
		case 0:
				socketNo = RemaskSocket((c - '0'));
				length = 0;
				if(socketNo >= SOCKET_NUM)
				{
					return 0;
				}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			break;
		default:
			if(AT_cmdRecvLength >= 9) 
			{
				return 0;
			}
			if((c >= '0') && (c <= '9'))
			{
				length *= 10;
				length += c - '0';
			}
			else
			{
				GPRS_dataUnackLength[socketNo] = length;
				return 0;
			}
			break;
	}
	return 0xff;
}

char GPRS_CreateSocket(char c)
{
	if((c >= '0') && (c <= '9'))
	{
		socketMask[createSocketNow] = c - '0';
	}
	return 0;
}
/*+UUSOCL: */
char GPRS_CloseSocket1(char c)
{ uint8_t socketNo;
	socketNo = RemaskSocket((c - '0'));
	if(socketNo < SOCKET_NUM)
	{
		tcpSocketStatus[socketNo] = SOCKET_CLOSE;
	}
	return 0;
}
/*+USOWR: 0,0*/
char GPRS_CloseSocket(char c)
{
	switch(AT_cmdRecvLength)
	{
		case 0:
				socketNo = RemaskSocket((c - '0'));
				if(socketNo >= SOCKET_NUM)
				{
					return 0;
				}
			break;
		case 2:
				if(c == '0')
				{
					tcpSocketStatus[socketNo] = SOCKET_CLOSE;
				}
				return 0;
		default:
			break;
	}
	return 0xff;
}


/*   14,"REC READ","+84972046096","","12/07/26,11:10:17+28"   */
char SMS_Process(char c)
{ 
	static uint32_t length = 0;
	switch(AT_cmdRecvLength)
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
					smsCnt++;
					smsNewMessageFlag = 1;
					return 0;
				}	
		default:
			smsNewMessageFlag = 1;
			return 0;
	}
	return 0xff;
}


uint8_t RemaskSocket(uint8_t socket)
{
	uint8_t i;
	for(i = 0;i < SOCKET_NUM;i++)
	{
		if(tcpSocketStatus[i] == SOCKET_OPEN)
			if(socketMask[i] == socket)
				return i;
	}
	return 0xff;
}
/*				
				  \/
+UUSORD: 0,14
AT_cmdRecvBuff = 0,14
*/
char GPRS_GetLength(char c)
{
	static uint32_t length = 0;
	switch(AT_cmdRecvLength)
	{
		case 0:
				socketNo = RemaskSocket((c - '0'));
				if(socketNo >= SOCKET_NUM)
				{
					return 0;
				}
			break;
		case 2:
			if((c >= '0') && (c <= '9'))
				length = c - '0';
			else
				return 0;
			break;
		case 3:
			if((c >= '0') && (c <= '9'))
			{
				length *= 10;
				length += c - '0';
				RINGBUF_Put(&gprsRingLengthBuff[socketNo],(uint8_t)(length & 0x00ff));
				RINGBUF_Put(&gprsRingLengthBuff[socketNo],(uint8_t)((length >> 8) & 0x00ff));
				length = 0;
			}
			return 0;
		default:
			break;
	}
	return 0xff;
}


enum{
	GPRS_START_RECV_DATA,
	GPRS_GET_LENGTH,
	GPRS_PREPARE_GET_DATA,
	GPRS_GET_DATA,
	GPRS_FINISH
}dataPhase = GPRS_FINISH;

uint32_t GPRS_SendCmdRecvData(uint8_t socketNum,uint8_t *gprsBuff,uint16_t gprsBuffLen,uint32_t *dataLen)
{
	uint8_t buf[20];
	Timeout_Type tout;
	MODEM_Info("\r\nGSM->GPRS READ DATA\r\n");
	DelayMs(100);
	dataPhase = GPRS_START_RECV_DATA;
	gprsRecvDataLen = 0;
	gprsRecvPt = gprsBuff;
	socketRecvEnable = 1;
	sprintf((char *)buf, "AT+USORD=%d,%d\r",socketMask[socketNum],(GPRS_DATA_MAX_LENGTH-32));
	COMM_Puts(buf);
	InitTimeout(&tout,TIME_MS(1000));
	while(dataPhase != GPRS_FINISH)
	{
		if(CheckTimeout(&tout) == TIMEOUT)
		{
			return 1;
		}
	}
	socketRecvEnable = 0;
	*dataLen = gprsRecvDataLen;
	return 0;
}
uint32_t GPRS_CmdRecvData(uint8_t *gprsBuff,uint16_t gprsBuffLen,uint32_t *dataLen)
{
	static uint32_t timeRead[SOCKET_NUM] = {0};
	uint8_t c,buf[20];
	uint16_t len = 0;
	Timeout_Type tout;
	uint32_t socketNum;
	for(socketNum = 0;socketNum < SOCKET_NUM;socketNum++)
	{
		if(tcpSocketStatus[socketNum] == SOCKET_OPEN)
		{
			if((TICK_Get() - timeRead[socketNum]) >= tcpTimeReCheckData[socketNum]) //5SEC
			{
				len = (GPRS_DATA_MAX_LENGTH-32);
			}
			else if((RINGBUF_GetFill(&gprsRingLengthBuff[socketNum]) >= 2))
			{
				RINGBUF_Get(&gprsRingLengthBuff[socketNum],&c);
				len = c;
				RINGBUF_Get(&gprsRingLengthBuff[socketNum],&c);
				len |= (((uint16_t)c) << 8 & 0xff00);
			}	
			while(len)
			{
				timeRead[socketNum] = TICK_Get();
				DelayMs(100);
				if(len > (GPRS_DATA_MAX_LENGTH-32))
				{
					len -= (GPRS_DATA_MAX_LENGTH-32);
				}
				else
				{
					len = 0;
				}
				dataPhase = GPRS_START_RECV_DATA;
				gprsRecvDataLen = 0;
				gprsRecvPt = gprsBuff;
				socketRecvEnable = 1;
				MODEM_Info("\r\nGSM->GPRS READ DATA\r\n");
				sprintf((char *)buf, "AT+USORD=%d,%d\r",socketMask[socketNum],(GPRS_DATA_MAX_LENGTH-32));
				COMM_Puts(buf);
				InitTimeout(&tout,TIME_MS(3000));
				while(dataPhase != GPRS_FINISH)
				{
					if(CheckTimeout(&tout) == TIMEOUT)
					{
						 socketRecvEnable = 0;
						 return 0xff;
					}
				}
				socketRecvEnable = 0;
				*dataLen = gprsRecvDataLen;
				return socketNum;
			}
		}
		else
		{
			timeRead[socketNum] = TICK_Get();
		}
	}
	socketRecvEnable = 0;
	return 0xff;
}

char GPRS_GetDNS(char c)
{
	static uint8_t *pt;
	switch(AT_cmdRecvLength)
	{
		case 0:
				pt = DNS_serverIp;
				*pt = c;
				pt++;
			break;
		default:
				if((c == '"') || (pt >= DNS_serverIp + sizeof(DNS_serverIp)))
				{
					return 0;
				}
				else
				{
					*pt = c;
					pt++;
					*pt = 0;
				}
			break;
	}
	
	return 0xff;
}

/*
+USORD: 0,12,"thienhaiblue"
									 1,"
AT_cmdRecvBuff = 0,12,"thienhaiblue"
									 123,"
                 0,0,""
*/
char GPRS_GetData(char c)
{
	static uint32_t dataLen = 0;
	switch(AT_cmdRecvLength)
	{
		case 0:
				socketNo = RemaskSocket((c - '0'));
				if(socketNo >= SOCKET_NUM)
				{
					return 0;
				}
				dataPhase = GPRS_GET_LENGTH;
			break;
		case 1:
				if(c != ',') return 0;
			break;
		case 2:
			if((c > '0') && (c <= '9'))
				dataLen = c - '0';
			else
				return 0;
			break;
		default:
			switch(dataPhase)
			{
				case GPRS_GET_LENGTH:
					if((c >= '0') && (c <= '9'))
					{
						dataLen *= 10;
						dataLen += c - '0';
					}
					else if(c == ',')
						dataPhase = GPRS_PREPARE_GET_DATA;
					else 
						return 0;
					break;
				case GPRS_PREPARE_GET_DATA:
					if(c == '"')
					{
						dataPhase = GPRS_GET_DATA;
						gprsRecvDataLen = 0;
					}
					else 
						return 0;
					break;
				case GPRS_GET_DATA:
					if(socketRecvEnable)
					{
						gprsRecvPt[gprsRecvDataLen] = c;
						gprsRecvDataLen++;
					}
					dataLen--;
					if(dataLen == 0)
					{
						dataPhase = GPRS_FINISH;
						return 0;
					}
					break;
			}
			break;
	}
	return 0xff;
}
char CLOSE_TCP_Process(char c)
{
	//tcpStatus = TCP_CLOSED;
	return 0;
}

void AT_CommandCtl(void)
{	
	static uint32_t timeout = 0;
	uint8_t i;
	if(AT_cmdState == AT_CMD_FINISH) timeout = 0;
	else
	{
		timeout++;
		if(timeout >= TIME_SEC(3))
		{
			AT_cmdState = AT_CMD_FINISH;
			for(i = 0; i < CMD_COUNT;i++)
			{
				cmdCnt[i] = 0;
			}
		}
	}
}
void AT_ComnandParser(char c)
{
	uint32_t i;
	static uint32_t RingBuffInit = 0;
	if(RingBuffInit == 0)// int ring buff
	{
		RingBuffInit = 1;
		AT_CmdProcessInit();
	}
	switch(AT_cmdState)
	{
		case AT_CMD_FINISH:
			for(i = 0; i < CMD_COUNT;i++)
			{
					if(c == AT_ProcessCmd[i].cmdInfo[cmdCnt[i]])
					{
							cmdCnt[i]++;
							if(AT_ProcessCmd[i].cmdInfo[cmdCnt[i]] == '\0')
							{
									numCmd = i;
									AT_cmdState = AT_CMD_WAIT_FINISH;
									AT_cmdRecvLength = 0;
							}
					}
					else
					{
							cmdCnt[i] = 0;
					}
			}
		break;
		case AT_CMD_WAIT_FINISH:				
			if(AT_ProcessCmd[numCmd].func(c) == 0)
			{
				AT_cmdState = AT_CMD_FINISH;
				for(i = 0; i < CMD_COUNT;i++)
				{
					cmdCnt[i] = 0;
				}
			}
			AT_cmdRecvLength++;
			if(AT_cmdRecvLength >= GPRS_DATA_MAX_LENGTH)
			{
				AT_cmdState = AT_CMD_FINISH;
				for(i = 0; i < CMD_COUNT;i++)
				{
					cmdCnt[i] = 0;
				}
			}
			break;
		default:
			AT_cmdState = AT_CMD_FINISH;
			for(i = 0; i < CMD_COUNT;i++)
			{
				cmdCnt[i] = 0;
			}
			break;
	}	
}
/*+CMGR: "REC READ","+841645715282","","12/07/26,20:50:07+28"
"thienhailue"
*/
uint8_t flagSmsReadFinish = 0;
char SMS_ReadMsg(char c)
{ 
	static uint8_t comma = 0,getSmsDataFlag = 0;
	static uint8_t *u8pt;
	static uint8_t *u8pt1;
	static uint8_t *u8pt2;
	if(AT_cmdRecvLength == 0)
	{
		comma = 0;
		getSmsDataFlag = 0;
		u8pt = smsSender;
		u8pt2 = smsBuf;
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
		if((u8pt2 - smsBuf) >= sizeof(smsBuf))
		{		
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
// untested
void SMS_Manage(uint8_t *buff,uint32_t lengBuf)
{
	Timeout_Type t;
	uint8_t tmpBuf[32],c;	
	uint16_t smsLoc;
	// read the newly received SMS
	INFO("\r\nSMS->CHECK ALL\r\n");
	COMM_Puts("AT+CMGL=\"ALL\"\r");
	DelayMs(1000);
	while(RINGBUF_GetFill(&smsUnreadRingBuff) >=2)
	{
		watchdogFeed[WTD_MAIN_LOOP] = 0;
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsLoc = c;
		RINGBUF_Get(&smsUnreadRingBuff,&c);
		smsLoc |= (((uint16_t)c) << 8 & 0xff00);
		// read the newly received SMS
		INFO("\r\nSMS->READ SMS\r\n");
		flagSmsReadFinish = 0;
		sprintf((char *)tmpBuf, "AT+CMGR=%d\r", smsLoc);
		COMM_Puts(tmpBuf);
		InitTimeout(&t,3000);//1s
		while(!flagSmsReadFinish)
		{
			if(CheckTimeout(&t) == TIMEOUT) 
			{
				break;
			}
		}
		smsBuf[sizeof(smsBuf) - 1] = 0;
		INFO("\n\rSMS->PHONE:%s\n\r", smsSender);	
		INFO("\r\nSMS->DATA:%s\r\n",smsBuf);
		// delete just received SMS
		INFO("\n\rSMS->DELETE:%d\n\r",smsLoc);
		sprintf((char *)tmpBuf, "AT+CMGD=%d\r", smsLoc);
		COMM_Puts(tmpBuf);
		MODEM_CheckResponse("OK", 1000);	
		if(flagSmsReadFinish == 0) continue;
		Parse((char *)smsBuf,buff,lengBuf);
	}
}


char Parse(char *buff,uint8_t *smsSendBuff,uint32_t smsLenBuf)
{
	char *pt,*u8pt,tempBuff0[32],tempBuff1[16],flagCfgSave = 0;
	uint32_t i,t1,t2,t3,t4,len,error,flagResetDevice = 0;
	DATE_TIME t;
	uint8_t cmdOkFlag = 0;
	
	//check password
	if((strstr(buff,(char *)sysCfg.smsPwd) != NULL) || (strstr(buff,"ZOTA") != NULL))
	{
			INFO("\n\rSMS->PASSWORD OK\n\r");
	}
	else
	{
			INFO("\n\rSMS->PASSWORD FAILS\n\r");
			return 1;
	}
	//check monney
	pt = strstr(buff,"*101#");
	if(pt != NULL)
	{
		DelayMs(1000);
		COMM_Puts("ATD*101#\r");
		DelayMs(1000);
		if(!MODEM_CheckResponse("+CUSD:", 5000) == 0) return 1;
		DelayMs(1000);
		i = 0;
		while(RINGBUF_Get(&commBuf, &smsSendBuff[i])==0)
		{
			i++;
			if(i >= 160) break;
		}
		smsSendBuff[i] = 0;
		SendSMS(smsSender,smsSendBuff);
		cmdOkFlag = 1;
	}
	//check monney
	pt = strstr(buff,"*102#");
	if(pt != NULL)
	{
		DelayMs(1000);
		COMM_Puts("ATD*102#\r");
		DelayMs(1000);
		if(!MODEM_CheckResponse("+CUSD:", 5000) == 0) return 1;
		DelayMs(1000);
		i = 0;
		while(RINGBUF_Get(&commBuf, &smsSendBuff[i])==0)
		{
			i++;
			if(i >= 160) break;
		}
		smsSendBuff[i] = 0;
		SendSMS(smsSender,smsSendBuff);
		cmdOkFlag = 1;
	}
	//chagre monney
	pt = strstr(buff,"*100*");
	if(pt != NULL)
	{
		sprintf((char *)smsSendBuff,"ATD*100*%s\r",pt);
		DelayMs(1000);
		COMM_Puts(smsSendBuff);
		if(!MODEM_CheckResponse("+CUSD:", 10000) == 0) return 1;
		DelayMs(1000);
		i = 0;
		while(RINGBUF_Get(&commBuf, &smsSendBuff[i])==0)
		{
			i++;
			if(i >= 160) break;
		}
		smsSendBuff[i] = 0;
		SendSMS(smsSender,smsSendBuff);
		cmdOkFlag = 1;
	}
	
	__disable_irq();
	len = 0;
	//change password
// 	pt = strstr(buff,"ABCFG,0,");
// 	if(pt != NULL)
// 	{
// 		// compare with saved password here
// 		smsScanf(pt,"ABCFG,0,%s",tempBuff0);
// 		memcpy((char *)sysCfg.smsPwd,tempBuff0,sizeof(sysCfg.smsPwd));
// 		INFO("\n\rSMS->NEW PASSWORD:%s\n\r",sysCfg.smsPwd);
// 		len += sprintf((char *)&smsSendBuff[len], "npwr,%s\n", sysCfg.smsPwd);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	//change gprs password
// 	pt = strstr(buff,"ABCFG,1,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,1,%s",tempBuff0);
// 		memcpy((char *)sysCfg.gprsPwd,tempBuff0,sizeof(sysCfg.gprsPwd));
// 		INFO("\n\rSMS->NEW GPRS PASSWORD:%s\n\r",sysCfg.gprsPwd);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,1,%s\n", sysCfg.gprsPwd);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change gprs user
// 	pt = strstr(buff,"ABCFG,2,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,2,%s",tempBuff0);
// 		memcpy((char *)sysCfg.gprsUsr,tempBuff0,sizeof(sysCfg.gprsUsr));
// 		INFO("\n\rSMS->NEW GPRS USER:%s\n\r",sysCfg.gprsUsr);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,2,%s\n", sysCfg.gprsUsr);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change gprs apn
// 	pt = strstr(buff,"ABCFG,3,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,3,%s",tempBuff0);
// 		memcpy((char *)sysCfg.gprsApn,tempBuff0,sizeof(sysCfg.gprsApn));
// 		INFO("\n\rSMS->NEW GPRS APN:%s\n\r",sysCfg.gprsApn);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,3,%s\n", sysCfg.gprsApn);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change Server Name
// 	pt = strstr(buff,"ABCFG,4,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,4,%s",tempBuff0);
// 		memcpy((char *)sysCfg.priDserverName,tempBuff0,sizeof(sysCfg.priDserverName));
// 		sysCfg.dServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS DATA SERVER NAME:%s\n\r",sysCfg.priDserverName);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,4,%s\n", sysCfg.priDserverName);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 	}

// 	//change Server IP
// 	pt = strstr(buff,"ABCFG,5,");
// 	if(pt != NULL)
// 	{
// 		u8pt = (char *)sysCfg.priDserverIp;
// 		smsScanf(pt,"ABCFG,5,%d.%d.%d.%d:%s",&t1,&t2,&t3,&t4,tempBuff1);
// 		sysCfg.priDserverPort = atoi((char *)tempBuff1);
// 		u8pt[0] = t1;
// 		u8pt[1] = t2;
// 		u8pt[2] = t3;
// 		u8pt[3] = t4;		
// 		sysCfg.dServerUseIp = 1;
// 		INFO("\n\rSMS->NEW GPRS DATA SERVER IP:%d.%d.%d.%d:%d\n\r",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priDserverPort);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,5,%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priDserverPort);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 	}
// 	
// 	//change Server use IP
// 	pt = strstr(buff,"ABCFG,6,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,6,%s",tempBuff0);
// 		if(tempBuff0[0] == '1')
// 			sysCfg.dServerUseIp = 1;
// 		else
// 			sysCfg.dServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS DATA SERVER IP:%s\n\r",sysCfg.dServerUseIp);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,6,%d\n", sysCfg.dServerUseIp);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 	}
// 	
// 	//change Server Name
// 	pt = strstr(buff,"ABCFG,8,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,8,%s",tempBuff0);
// 		memcpy((char *)sysCfg.priFserverName,tempBuff0,sizeof(sysCfg.priFserverName));
// 		sysCfg.fServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS FIRMWARE SERVER NAME:%s\n\r",sysCfg.priFserverName);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,8,%s\n", sysCfg.priFserverName);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 	}
// 	//change Server IP
// 	pt = strstr(buff,"ABCFG,9,");
// 	if(pt != NULL)
// 	{
// 		u8pt = (char *)sysCfg.priFserverIp;
// 		smsScanf(pt,"ABCFG,9,%d.%d.%d.%d:%s",&t1,&t2,&t3,&t4,tempBuff1);
// 		sysCfg.priFserverPort = atoi((char *)tempBuff1);
// 		u8pt[0] = t1;
// 		u8pt[1] = t2;
// 		u8pt[2] = t3;
// 		u8pt[3] = t4;		
// 		sysCfg.fServerUseIp = 1;
// 		INFO("\n\rSMS->NEW GPRS FIRMWARE IP:%d.%d.%d.%d:%d\n\r",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priFserverPort);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,9,%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priFserverPort);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 	}
// 	
// 	//change Server use IP
// 	pt = strstr(buff,"ABCFG,10,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,10,%s",tempBuff0);
// 		if(tempBuff0[0] == '1')
// 			sysCfg.dServerUseIp = 1;
// 		else
// 			sysCfg.fServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS FIRMWARE SERVER IP:%s\n\r",sysCfg.fServerUseIp);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,10,%d\n", sysCfg.fServerUseIp);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 	}
// 	
// 	
// 	
// 	//Clear log
// 	pt = strstr(buff,"ABCFG,12,0");
// 	if(pt != NULL)
// 	{
// 		//__disable_irq();
// 		DB_RingLogReset();
// 		//__enable_irq();
// 		cmdOkFlag = 1;
// 		INFO("\n\rSMS->CLEAR LOG\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,12->OK\n");
// 	}


// 	//set default config
// 	pt = strstr(buff,"ABCFG,13");
// 	if(pt != NULL)
// 	{
// 		//__disable_irq();
// 		DB_RingLogReset();
// 		SST25_Erase(MILEAGE_ADDR,block4k);
// 		SST25_Erase(RESET_CNT_ADDR,block4k);
// 		DB_ResetCntLoad();
// 		DB_ResetCntSave();
// 		DB_MileageLoad();
// 		DB_MileageSave();
// 		memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
// 		CFG_Save();
// 		CFG_Load();
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 		tcpSocketStatus[2] = SOCKET_CLOSE;
// 		//__enable_irq();
// 		INFO("\n\rSMS->SET DEFAULT CONFIGURATION\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,13->OK\n");
// 		cmdOkFlag = 1;
// 	}
// 	//change device id
// 	pt = strstr(buff,"ABCFG,14,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,14,%s",tempBuff0);
// 		memcpy((char *)sysCfg.id,tempBuff0,sizeof(sysCfg.id));
// 		INFO("\n\rSMS->CHANGE DEVICE ID:%s\n\r",sysCfg.id);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,14,%s\n", sysCfg.id);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change plate no
// 	pt = strstr(buff,"ABCFG,15,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,15,%s",tempBuff0);
// 		memcpy((char *)sysCfg.plateNo,tempBuff0,sizeof(sysCfg.plateNo));
// 		INFO("\n\rSMS->CHANGE PLATE NO:%s\n\r",sysCfg.plateNo);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,15,%s\n", sysCfg.plateNo);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change interval time run
// 	pt = strstr(buff,"ABCFG,16,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,16,%[^/]/%s",tempBuff0,tempBuff1);
// 		sysCfg.runReportInterval = atoi((char *)tempBuff0);
// 		sysCfg.stopReportInterval = atoi((char *)tempBuff1);
// 		INFO("\n\rSMS->CHANGE INTERVAL TIME RUN:%d/%d\n\r",sysCfg.runReportInterval,sysCfg.stopReportInterval);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,16,%d/%d\n", sysCfg.runReportInterval,sysCfg.stopReportInterval);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}

// 	//set tyre rate
// 	pt = strstr(buff,"ABCFG,18,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,18,%s",tempBuff0);
// 		sysCfg.speedSensorRatio = atoi((char *)tempBuff0);
// 		INFO("\n\rSMS->CHANGE TYRE RATE:%d\n\r",sysCfg.speedSensorRatio);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,18,%d\n", sysCfg.speedSensorRatio);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//set beep warning enable
// 	pt = strstr(buff,"ABCFG,19");
// 	if(pt != NULL)
// 	{
// 		sysCfg.featureSet |= FEATURE_SUPPORT_BEEP_WARNING;
// 		INFO("\n\rSMS->BEEP WARNING ENABLE\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "BEEP ENABLE\n");
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//set beep warning disable
// 	pt = strstr(buff,"ABCFG,20");
// 	if(pt != NULL)
// 	{
// 		sysCfg.featureSet &= ~FEATURE_SUPPORT_BEEP_WARNING;
// 		INFO("\n\rSMS->BEEP WARNING DISABLE\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "BEEP DISABLE\n");
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//warning speed set
// 	pt = strstr(buff,"ABCFG,21,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,21,%s",tempBuff0);
// 		sysCfg.speedLimit = atoi((char *)tempBuff0);
// 		INFO("\n\rSMS->WARNING SPEED:%d\n\r",sysCfg.speedLimit);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,21,%d\n", sysCfg.speedLimit);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//get product info
// 	pt = strstr(buff,"ABCFG,22");
// 	if(pt != NULL)
// 	{
// 		INFO("\n\rSMS->GET PRODUCT INFO\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "TIME:%d:%d:%d\n",sysTime.hour,sysTime.min,sysTime.sec);
// 		len += sprintf((char *)&smsSendBuff[len], "IMEI:%s\n", sysCfg.imei);
// 		len += sprintf((char *)&smsSendBuff[len], "PLATE NO:%s\n", sysCfg.plateNo);
// 		len += sprintf((char *)&smsSendBuff[len], "FIRMWARE VESTION:%s\n", FIRMWARE_VERSION);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//get car status
// 	pt = strstr(buff,"ABCFG,23");
// 	if(pt != NULL)
// 	{
// 		INFO("\n\rSMS->GET CAR STATUS\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "TIME:%d:%d:%d\n",sysTime.hour,sysTime.min,sysTime.sec);
// 		//2+1
// 		if(GET_ACC_PIN)
// 			len += sprintf((char *)&smsSendBuff[len], "ACC:OFF\n");
// 		else
// 			len += sprintf((char *)&smsSendBuff[len], "ACC:ON\n");
// 		
// 		t1 = TICK_Get()/1000;
// 		len += sprintf((char *)&smsSendBuff[len],"TIME RUN:%02d:%02d:%02d",t1/3600,(t1%3600)/60,(t1%60));
// 		len += sprintf((char *)&smsSendBuff[len], "RESET CNT:%d\n",resetNum);
// 		if(tcpSocketStatus[0])
// 			len += sprintf((char *)&smsSendBuff[len], "D-SVR:CONNECT\n");
// 		else
// 			len += sprintf((char *)&smsSendBuff[len], "D-SVR:DISCONNECT\n");
// 		if(sysCfg.dServerUseIp)
// 		{
// 			if(sysCfg.curDServer == 0)
// 			{
// 				u8pt = (char *)sysCfg.priDserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priDserverPort);
// 			}
// 			else
// 			{
// 				u8pt = (char *)sysCfg.secDserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secDserverPort);
// 			}
// 			u8pt = (char *)sysCfg.priDserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR0:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priDserverPort);
// 			u8pt = (char *)sysCfg.secDserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR1:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secDserverPort);
// 		}
// 		else
// 		{
// 			if(sysCfg.curDServer == 0)
// 			{
// 				u8pt = (char *)sysCfg.priDserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%s:%d\n",sysCfg.priDserverName,sysCfg.priDserverPort);
// 			}
// 			else
// 			{
// 				u8pt = (char *)sysCfg.secDserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%s:%d\n",sysCfg.secDserverName,sysCfg.secDserverPort);
// 			}
// 			u8pt = (char *)sysCfg.priDserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR0:%s:%d\n",sysCfg.priDserverName,sysCfg.priDserverPort);
// 			u8pt = (char *)sysCfg.secDserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR1:%s:%d\n",sysCfg.secDserverName,sysCfg.secDserverPort);
// 		}
// 		
// 		
// 		if(tcpSocketStatus[1])
// 			len += sprintf((char *)&smsSendBuff[len], "F-SVR:CONNECT\n");
// 		else
// 			len += sprintf((char *)&smsSendBuff[len], "F-SVR:DISCONNECT\n");
// 		
// 		if(sysCfg.fServerUseIp)
// 		{
// 			if(sysCfg.curFServer == 0)
// 			{
// 				u8pt = (char *)sysCfg.priFserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priFserverPort);
// 			}
// 			else
// 			{
// 				u8pt = (char *)sysCfg.secFserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secFserverPort);
// 			}
// 			u8pt = (char *)sysCfg.priFserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR0:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priFserverPort);
// 			u8pt = (char *)sysCfg.secFserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR1:%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secFserverPort);
// 		}
// 		else
// 		{
// 			if(sysCfg.curFServer == 0)
// 			{
// 				u8pt = (char *)sysCfg.priFserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%s:%d\n",sysCfg.priFserverName,sysCfg.priFserverPort);
// 			}
// 			else
// 			{
// 				u8pt = (char *)sysCfg.secFserverIp;
// 				len += sprintf((char *)&smsSendBuff[len],"CURRENT SVR:%s:%d\n",sysCfg.secFserverName,sysCfg.secFserverPort);
// 			}
// 			u8pt = (char *)sysCfg.priFserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR0:%s:%d\n",sysCfg.priFserverName,sysCfg.priFserverPort);
// 			u8pt = (char *)sysCfg.secFserverIp;
// 			len += sprintf((char *)&smsSendBuff[len],"SVR1:%s:%d\n",sysCfg.secFserverName,sysCfg.secFserverPort);
// 		}

// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 	}
// 	//get cell id
// 	pt = strstr(buff,"ABCFG,24");
// 	if(pt != NULL)
// 	{
// 		INFO("\n\rSMS->GET CELL ID\n\r");
// 	}
// 	
// 	//get gps location
// 	pt = strstr(buff,"ABCFG,25");
// 	if(pt != NULL)
// 	{
// 		INFO("\n\rSMS->GET GPS LOCATION\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "TIME:%d:%d:%d\n",sysTime.hour,sysTime.min,sysTime.sec);
// 		len += sprintf((char *)&smsSendBuff[len], "SPEED:%f\n", lastGpsInfo.speed);
// 		len += sprintf((char *)&smsSendBuff[len], "HDOP:%f\n", lastGpsInfo.HDOP);
// 		len += sprintf((char *)&smsSendBuff[len],	"http://maps.google.com/maps?q=%.8f,%.8f\n",formatLatLng(lastGpsInfo.lat),formatLatLng(lastGpsInfo.lon));
// 	}
// 	
// 	//reset device
// 	flagResetDevice = 0;
// 	pt = strstr(buff,"ABCFG,40");
// 	if(pt != NULL)
// 	{
// 		flagResetDevice = 1;
// 		INFO("\n\rSMS->RESET DEVICE\\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "DEVICE RESETING...\n");
// 		cmdOkFlag = 1;
// 	}
// 	
// 	//change Server Name
// 	pt = strstr(buff,"ABCFG,41,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,41,%s",tempBuff0);
// 		memcpy((char *)sysCfg.secDserverName,tempBuff0,sizeof(sysCfg.secDserverName));
// 		sysCfg.dServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS DATA SERVER NAME:%s\n\r",sysCfg.secDserverName);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,41,%s\n", sysCfg.secDserverName);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 	}

// 	//change Server IP
// 	pt = strstr(buff,"ABCFG,42,");
// 	if(pt != NULL)
// 	{
// 		u8pt = (char *)sysCfg.secDserverIp;
// 		smsScanf(pt,"ABCFG,42,%d.%d.%d.%d:%s",&t1,&t2,&t3,&t4,tempBuff1);
// 		sysCfg.secDserverPort = atoi((char *)tempBuff1);
// 		u8pt[0] = t1;
// 		u8pt[1] = t2;
// 		u8pt[2] = t3;
// 		u8pt[3] = t4;		
// 		sysCfg.dServerUseIp = 1;
// 		INFO("\n\rSMS->NEW GPRS DATA SERVER IP:%d.%d.%d.%d:%d\n\r",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secDserverPort);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,42,%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secDserverPort);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[0] = SOCKET_CLOSE;
// 	}
// 	

// 	//change Server Name
// 	pt = strstr(buff,"ABCFG,43,");
// 	if(pt != NULL)
// 	{
// 		smsScanf(pt,"ABCFG,43,%s",tempBuff0);
// 		memcpy((char *)sysCfg.secFserverName,tempBuff0,sizeof(sysCfg.secFserverName));
// 		sysCfg.fServerUseIp = 0;
// 		INFO("\n\rSMS->NEW GPRS FIRMWARE SERVER NAME:%s\n\r",sysCfg.secFserverName);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,43,%s\n", sysCfg.secFserverName);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 	}
// 	//change Server IP
// 	pt = strstr(buff,"ABCFG,44,");
// 	if(pt != NULL)
// 	{
// 		u8pt = (char *)sysCfg.secFserverIp;
// 		smsScanf(pt,"ABCFG,44,%d.%d.%d.%d:%s",&t1,&t2,&t3,&t4,tempBuff1);
// 		sysCfg.secFserverPort = atoi((char *)tempBuff1);
// 		u8pt[0] = t1;
// 		u8pt[1] = t2;
// 		u8pt[2] = t3;
// 		u8pt[3] = t4;		
// 		sysCfg.fServerUseIp = 1;
// 		INFO("\n\rSMS->NEW GPRS FIRMWARE IP:%d.%d.%d.%d:%d\n\r",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secFserverPort);
// 		len += sprintf((char *)&smsSendBuff[len], "ABCFG,44,%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.secFserverPort);
// 		flagCfgSave = 1;
// 		cmdOkFlag = 1;
// 		tcpSocketStatus[1] = SOCKET_CLOSE;
// 	}
// 	

// 	//get firmware version
// 	pt = strstr(buff,"ABCFG,47");
// 	if(pt != NULL)
// 	{
// 		flagResetDevice = 1;
// 		INFO("\n\rSMS->RESET DEVICE\\n\r");
// 		len += sprintf((char *)&smsSendBuff[len], "VERSION:%s\n",FIRMWARE_VERSION);
// 		cmdOkFlag = 1;
// 	}

	//help
	pt = strstr(buff,"HELP");
	if((pt != NULL) && (cmdOkFlag == 0))
	{
		len = 0;
		INFO("\n\rSMS->HELP...\n\r");
		len += sprintf((char *)&smsSendBuff[len],"%s\n",sysCfg.smsPwd);
		len += sprintf((char *)&smsSendBuff[len],"ID\n");
		len += sprintf((char *)&smsSendBuff[len],"ABCFG,14,%s\n",sysCfg.id);
		len += sprintf((char *)&smsSendBuff[len],"IP:PORT\n");
		u8pt = (char *)sysCfg.priDserverIp;
		len += sprintf((char *)&smsSendBuff[len],"ABCFG,5,%d.%d.%d.%d:%d\n",u8pt[0],u8pt[1],u8pt[2],u8pt[3],sysCfg.priDserverPort);
		len += sprintf((char *)&smsSendBuff[len],"ON/OFF INTERVAL\n");
		len += sprintf((char *)&smsSendBuff[len],"ABCFG,16,%d/%d\n",sysCfg.runReportInterval,sysCfg.stopReportInterval);
		len += sprintf((char *)&smsSendBuff[len],"PLATE NO\n");
		len += sprintf((char *)&smsSendBuff[len],"ABCFG,15,%s\n",sysCfg.plateNo);
		len += sprintf((char *)&smsSendBuff[len],"SPEED LIMIT\n");
		len += sprintf((char *)&smsSendBuff[len],"ABCFG,21,%d\n",sysCfg.speedLimit);
	}
	if(len >= smsLenBuf)	len = smsLenBuf;
	__enable_irq();
	smsSendBuff[len] = 0;
	if(len >= 160)
	{
		for(i = 0;i < len; i += 160)
		{
			DelayMs(5000);
			SendSMS(smsSender,&smsSendBuff[i]);
		}
	}
	else
	{
		SendSMS(smsSender,smsSendBuff);
	}
	if(flagCfgSave)
	{
		CFG_Save();
	}
	if(flagResetDevice)
	{
		NVIC_SystemReset();
	}
	return 0;
}


