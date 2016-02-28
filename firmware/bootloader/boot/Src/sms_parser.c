#include "sms_parser.h"

#define INFO(...)	DbgCfgPrintf(__VA_ARGS__)

#define smsScanf	sscanf

#define CMD_COUNT   (sizeof (AT_ProcessCmd) / sizeof (AT_ProcessCmd[0]))
#define AT_CMD_PARSER_SIZE	16



char SMS_Process(char c);
char GPRS_Process(char c);
char CLOSE_TCP_Process(char c);
char CLIP_Process(char c);
char Parse(char *buff);
char SMS_Incoming(char c);

uint8_t smsSendBuff[256];
uint8_t smsBuf[160];
uint8_t smsSender[18];

uint8_t callingNumber[16] = {0};

uint32_t smsCnt = 0;

uint8_t smsUnreadBuff[100] = {0};
RINGBUF smsUnreadRingBuff;

uint8_t gprsBuff[512] = {0};
RINGBUF gprsRingBuff;
uint32_t gprsDataLength = 0,gprsDataStatus = 0;


char AT_cmdRecvBuff[512];
uint32_t AT_cmdRecvLength;

void AT_CmdProcessInit(void)
{
	RINGBUF_Init(&smsUnreadRingBuff,smsUnreadBuff,sizeof(smsUnreadBuff));
	RINGBUF_Init(&gprsRingBuff,gprsBuff,sizeof(gprsBuff));
}



enum{
	AT_CMD_NEW_STATE,
	AT_CMD_WAIT_FINISH,
	AT_CMD_FINISH
}AT_cmdState = AT_CMD_FINISH;

/* Command definitions structure. */
typedef struct scmd {
   char cmdInfo[16];
   char (*func)(char c);
} SCMD;

static const SCMD AT_ProcessCmd[] = {
  "+CMGL: ", SMS_Process,
	"+IPD,", GPRS_Process,
	"CLOSE", CLOSE_TCP_Process,
	"+CLIP:",CLIP_Process,
	"+CMTI:",SMS_Incoming,
	"+PDP: DEACT" ,CLOSE_TCP_Process};


uint32_t cmdCnt[CMD_COUNT];
uint8_t numCmd;
/*+CMTI: "SM",7*/
char SMS_Incoming(char c)
{
	sysEventFlag |=  MODEM_SMS_FLAG;
	return 0;
}
/*+CLIP: "0978779222",129,"",,"",0*/
char CLIP_Process(char c)
{
	static uint8_t len;
	if(AT_cmdRecvLength <= 8)
	{
		memset(callingNumber,0,sizeof(callingNumber));
		len = 0;
	}
	else
	{
		if((c >= '0') && (c <= '9'))
		{
			callingNumber[len] = c;
			len++;
			if(len >= sizeof(callingNumber))
			{
				sysEventFlag |= MODEM_CALL_FLAG;
				tcpConnectionStatus = 0;
				return 0;
			}
		}
		else
		{
			sysEventFlag |= MODEM_CALL_FLAG;
			tcpConnectionStatus = 0;
			return 0;
		}
	}
	return 0xff;
}
/*+CMGL: 14,"REC READ","+84972046096","","12/07/26,11:10:17+28"*/
char SMS_Process(char c)
{ 
	char *buff;
	if(c == ',')
	{
		buff = &AT_cmdRecvBuff[7];
		if((buff[0] > '0') && (buff[0] <= '9')) 
		{
				c = buff[0] - '0';
				if((buff[1] >= '0') && (buff[1] <= '9'))
				{
					c *= 10;
					c += buff[1] - '0';
					if((buff[2] >= '0') && (buff[2] <= '9'))
					{
						c *= 10;
						c += buff[2] - '0';
					}
				}			
				RINGBUF_Put(&smsUnreadRingBuff,c);
				smsCnt++;
				return 0;	
		}
	}
	return 0xff;
}
/*	+IPD,5:thienhaiblue */
char GPRS_Process(char c)
{
	char *buff;
	uint32_t num = 0;
	if(gprsDataStatus == 0)
	{
		if((c ==  ':'))
		{
			buff = &AT_cmdRecvBuff[5];
			if((buff[0] > '0') && (buff[0] <= '9')) 
			{
					num = buff[0] - '0';
					if((buff[1] >= '0') && (buff[1] <= '9'))
					{
						num *= 10;
						num += buff[1] - '0';
						if((buff[2] >= '0') && (buff[2] <= '9'))
						{
							num *= 10;
							num += buff[2] - '0';
						}
					}
					gprsDataLength = num;
					gprsDataStatus = 1;
			}
		}
	}
	else
	{
		RINGBUF_Put(&gprsRingBuff,c);
		gprsDataLength--;
		if(gprsDataLength == 0) //get all data
		{
			return 0;
		}	
	}
	return 0xff;
}
char CLOSE_TCP_Process(char c)
{
	tcpConnectionStatus = 0;
	return 0;
}
void AT_ComnandParser(char c)
{
	uint32_t i;
	if(AT_cmdState == AT_CMD_FINISH)
	{
		switch(c)
		{
			case '+':
			case 'C': //CLOSE
					AT_cmdRecvLength = 0;
					gprsDataStatus = 0;
					numCmd = 0;
					AT_cmdState = AT_CMD_NEW_STATE;
					for(i = 0; i < CMD_COUNT;i++)
					{
						cmdCnt[i] = 0;
					}
			break;
			default:				
			break;
		}
	}
	switch(AT_cmdState)
	{
		case AT_CMD_NEW_STATE:
			AT_cmdRecvBuff[AT_cmdRecvLength] = c;
			for(i = 0; i < CMD_COUNT;i++)
			{
					if(AT_cmdRecvBuff[AT_cmdRecvLength] == AT_ProcessCmd[i].cmdInfo[cmdCnt[i]])
					{
							cmdCnt[i]++;
							if(AT_ProcessCmd[i].cmdInfo[cmdCnt[i]] == '\0')
							{
									numCmd = i;
									AT_cmdState = AT_CMD_WAIT_FINISH;
							}
					}
					else
					{
							cmdCnt[i] = 0;
					}
			}
			AT_cmdRecvLength++;
			if(AT_cmdRecvLength > AT_CMD_PARSER_SIZE)
			{
					AT_cmdState = AT_CMD_FINISH;
			}
		break;
		case AT_CMD_WAIT_FINISH:				
			AT_cmdRecvBuff[AT_cmdRecvLength] = c;
			AT_cmdRecvLength++;
			if(AT_cmdRecvLength >= sizeof(AT_cmdRecvBuff))
			{
				AT_cmdState = AT_CMD_FINISH;
			}
			if(AT_ProcessCmd[numCmd].func(c) == 0)
			{
				AT_cmdState = AT_CMD_FINISH;
			}
			break;
		default:
			AT_cmdState = AT_CMD_FINISH;
			break;
	}	
}
/*+CMGR: "REC READ","+841645715282","","12/07/26,20:50:07+28"
"thienhailue"
*/
// untested
void SMS_Manage(void)
{
	Timeout_Type t;
	uint8_t tmpBuf[32],smsLoc,len;	
	// read the newly received SMS
	INFO("\r\nSMS:Check all SMS unread\r\n");
	COMM_Puts("AT+CMGL=\"REC UNREAD\"\r");
	DelayMs(1000);
	while(RINGBUF_Get(&smsUnreadRingBuff,&smsLoc) == 0)
	{
		// read the newly received SMS
		INFO("\r\nSMS:Read the newly received SMS\r\n");
		sprintf((char *)tmpBuf, "AT+CMGR=%d\r", smsLoc);
		COMM_Puts(tmpBuf);
		if(!MODEM_Wait("+CMGR", 100)) return;
		if(!MODEM_Wait(",", 10)) return;
		if(!MODEM_Wait("\"", 10)) return;
		if(!MODEM_Gets(smsSender, sizeof smsSender, '\"')) return;
		if(!MODEM_Wait("\n", 10)) return;
		len = 0;
		InitTimeout(&t,1000);//1s
		while(RINGBUF_Get(&USART1_RxRingBuff,&smsBuf[len]) == 0)
		{
			if(((smsBuf[len-3]) == '\r') && ((smsBuf[len-2]) == '\n') && ((smsBuf[len-1]) == 'O') && ((smsBuf[len]) == 'K'))
				break;
			len++;
			if(len >= sizeof(smsBuf)) break;		
			if(CheckTimeout(&t))	return;
		}
		smsBuf[len] = '\0';
		INFO("\n\rSMS:%s\n\r", smsSender);	
		// delete just received SMS
		INFO("\n\rSMS:Delete SMS %d\n\r",smsLoc);
		sprintf((char *)tmpBuf, "AT+CMGD=%d\r", smsLoc);
		COMM_Puts(tmpBuf);
		MODEM_Wait("OK", 10);	
		Parse((char *)smsBuf);
	}
}









char Parse(char *buff)
{
	char *pt,tempBuff0[16],tempBuff1[16],i,flagCfgSave = 0;
	//check password
	pt = strstr(buff,"pwr,");
	if(pt != NULL)
	{
			// compare with saved password here
			smsScanf(pt,"pwr,%s",tempBuff0);
			if(strcmp(tempBuff0, (char *)sysCfg.smsPwd) != 0 && strcmp(tempBuff0, "IVXITRACKERMB") != 0) 
			{
					INFO("\n\rSMS:Password fails\n\r");
					return 1;
			}
			INFO("\n\rSMS:Password Ok\n\r");
	}
	else
	{
			return 1;
	}
	
	
	//check monney
	pt = strstr(buff,"*101#");
	if(pt != NULL)
	{
		COMM_Puts("ATD*101#\r");
		if(MODEM_Wait("+CUSD:", 50) == 0) return 1;
		DelayMs(1000);
		i = 0;
		while(RINGBUF_Get(&commBuf, &smsSendBuff[i])==0)
		{
			i++;
			if(i >= sizeof(smsSendBuff)) break;
		}
		MODEM_SendSMS(smsSender,smsSendBuff);
	}
	
	//chagre monney
	pt = strstr(buff,"*100*");
	if(pt != NULL)
	{
		sprintf((char *)smsSendBuff,"ATD*100*%s\r",pt);
		COMM_Puts(smsSendBuff);
		if(MODEM_Wait("+CUSD:", 100) == 0) return 1;
		DelayMs(1000);
		i = 0;
		while(RINGBUF_Get(&commBuf, &smsSendBuff[i])==0)
		{
			i++;
			if(i >= sizeof(smsSendBuff)) break;
		}
		MODEM_SendSMS(smsSender,smsSendBuff);
	}
	
	//change password
	pt = strstr(buff,"cpwr,");
	if(pt != NULL)
	{
		// compare with saved password here
		smsScanf(pt,"cpwr,%[^,],%s",tempBuff0,tempBuff1);
		if(strcmp(tempBuff0, (char *)sysCfg.smsPwd) != 0 && strcmp(tempBuff0, "IVXITRACKERMB") != 0) 
		{
					INFO("\n\rSMS:Change Password fails\n\r");
					return 1;
		}
		strcpy((char *)sysCfg.smsPwd,tempBuff1);
		INFO("\n\rSMS:New Password:%s\n\r",sysCfg.smsPwd);
		flagCfgSave = 1;
	}
	//change gprs password
	pt = strstr(buff,"gpwr,");
	if(pt != NULL)
	{
		smsScanf(pt,"gpwr,%s",tempBuff0);
		strcpy((char *)sysCfg.gprsPwd,tempBuff0);
		INFO("\n\rSMS:New GPRS Password:%s\n\r",sysCfg.gprsPwd);
		flagCfgSave = 1;
	}
	
	//change gprs user
	pt = strstr(buff,"gusr,");
	if(pt != NULL)
	{
		smsScanf(pt,"gusr,%s",tempBuff0);
		strcpy((char *)sysCfg.gprsUsr,tempBuff0);
		INFO("\n\rSMS:New GPRS user:%s\n\r",sysCfg.gprsUsr);
		flagCfgSave = 1;
	}
	
	//change gprs apn
	pt = strstr(buff,"gapn,");
	if(pt != NULL)
	{
		smsScanf(pt,"gapn,%s",tempBuff0);
		strcpy((char *)sysCfg.gprsApn,tempBuff0);
		INFO("\n\rSMS:New GPRS apn:%s\n\r",sysCfg.gprsApn);
		flagCfgSave = 1;
	}
	
	//change Server Name
	pt = strstr(buff,"svname,");
	if(pt != NULL)
	{
		smsScanf(pt,"svname,%s",tempBuff0);
		strcpy((char *)sysCfg.serverName,tempBuff0);
		sysCfg.serverUseIp = 0;
		INFO("\n\rSMS:New GPRS server name:%s\n\r",sysCfg.serverName);
		flagCfgSave = 1;
	}
	
	//change Server IP
	pt = strstr(buff,"svip,");
	if(pt != NULL)
	{
		smsScanf(pt,"svip,%s",tempBuff0);
		strcpy((char *)sysCfg.serverIp,tempBuff0);
		sysCfg.serverUseIp = 1;
		INFO("\n\rSMS:New GPRS server ip:%s\n\r",sysCfg.serverIp);
		flagCfgSave = 1;
	}
	
	//change Server Port
	pt = strstr(buff,"svport,");
	if(pt != NULL)
	{
		smsScanf(pt,"svport,%s",tempBuff0);
		strcpy((char *)sysCfg.serverPort,tempBuff0);
		INFO("\n\rSMS:New GPRS server port:%s\n\r",sysCfg.serverPort);
		flagCfgSave = 1;
	}
	//Set boss phone number
	pt = strstr(buff,"myphone,");
	if(pt != NULL)
	{
		smsScanf(pt,"myphone,%s",tempBuff0);
		strcpy((char *)sysCfg.bossPhoneNum,tempBuff0);
		INFO("\n\rSMS:New Boss Phone:%s\n\r",sysCfg.bossPhoneNum);
		flagCfgSave = 1;
	}
	
	//Phone list set
	pt = strstr(buff,"mylist,");
	if(pt != NULL)
	{
		smsScanf(pt,"mylist,%s",tempBuff0);
		if(SavePhoneList((uint8_t *)tempBuff0))
		{
				SavePhoneList((uint8_t *)tempBuff0);
		}
		INFO("\n\rSMS:New Phone in list:%s\n\r",tempBuff0);
		flagCfgSave = 1;
	}
	
	//Set sleep time
	pt = strstr(buff,"sleep,");
	if(pt != NULL)
	{
		smsScanf(pt,"sleep,%s",tempBuff0);
		sysCfg.sleepTime = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:Sleep Time:%d\n\r",sysCfg.sleepTime);
		flagCfgSave = 1;
	}
	
	//Set speed engine off
	pt = strstr(buff,"speo,");
	if(pt != NULL)
	{
		smsScanf(pt,"speo,%s",tempBuff0);
		sysCfg.speedEngineOff = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:Speed Engine off:%d\n\r",sysCfg.speedEngineOff);
		flagCfgSave = 1;
	}
	
	//Set security off
	pt = strstr(buff,"seon");
	if(pt != NULL)
	{
		ENGINE_TURN_OFF;
		sysEventFlag |= SECURITY_ON_FLAG;
		BuzzerSetStatus(100,100,BUZZER_TURN_ON,3);
		INFO("\n\rSMS:Security turn on\n\r");
		MODEM_SendSMS(sysCfg.bossPhoneNum,"BAT CHONG TROM!");
		flagCfgSave = 1;
	}
	
	//Set security off
	pt = strstr(buff,"seoff");
	if(pt != NULL)
	{
		sysEventFlag &= ~POWER_FAILS_FLAG;
		sysEventFlag &= ~SECURITY_ON_FLAG;
		sysEventFlag &= ~THEFT_ALARM_FLAG;
		sysEventFlag &= ~HORN_ON_FLAG;
		BuzzerSetStatus(100,100,BUZZER_TURN_ON,1);
		accelerometerFlagCnt = 0;
		INFO("\n\rSMS:Security turn off\n\r");
		MODEM_SendSMS(sysCfg.bossPhoneNum,"TAT CHONG TROM!");
		flagCfgSave = 1;
	}
	//Set warning turn off engine for security action
	pt = strstr(buff,"aeof");
	if(pt != NULL)
	{
		sysCfg.securityAction &= ~WARNING_TURN_OFF_ENGINE;
		INFO("\n\rSMS:Clear security action: TURN_OFF_ENGINE\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning turn off engine for security action
	pt = strstr(buff,"aeon");
	if(pt != NULL)
	{
		sysCfg.securityAction |= WARNING_TURN_OFF_ENGINE;
		INFO("\n\rSMS:Set security action: TURN_OFF_ENGINE\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning horn for security action
	pt = strstr(buff,"ahof");
	if(pt != NULL)
	{
		sysCfg.securityAction &= ~WARNING_HORN;
		INFO("\n\rSMS:Clear security action: WARNING_HORN\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning turn off engine for security action
	pt = strstr(buff,"ahon");
	if(pt != NULL)
	{
		sysCfg.securityAction |= WARNING_HORN;
		INFO("\n\rSMS:Set security action: WARNING_HORN\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning turn off engine for security action
	pt = strstr(buff,"asof");
	if(pt != NULL)
	{
		sysCfg.securityAction &= ~WARNING_SMS;
		INFO("\n\rSMS:Clear security action: WARNING_SMS\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning turn off engine for security action
	pt = strstr(buff,"ason");
	if(pt != NULL)
	{
		sysCfg.securityAction |= WARNING_SMS;
		INFO("\n\rSMS:Set security action: WARNING_SMS\n\r");
		flagCfgSave = 1;
	}
	
	//turn off engine for security action
	pt = strstr(buff,"tatmay");
	if(pt != NULL)
	{
		ENGINE_TURN_OFF;
		INFO("\n\rSMS:TAT MAY\n\r");
	}
	//turn off engine for security action
	pt = strstr(buff,"momay");
	if(pt != NULL)
	{
		ENGINE_TURN_ON;
		INFO("\n\rSMS:MO MAY\n\r");
	}
	
	//turn off engine for security action
	pt = strstr(buff,"tatcoi");
	if(pt != NULL)
	{
		sysEventFlag &= ~POWER_FAILS_FLAG;
		sysEventFlag &= ~SECURITY_ON_FLAG;
		sysEventFlag &= ~THEFT_ALARM_FLAG;
		accelerometerFlagCnt = 0;
		sysEventFlag &= ~HORN_ON_FLAG;
		BuzzerSetStatus(100,100,BUZZER_TURN_ON,1);
		INFO("\n\rSMS:TAT COI\n\r");
	}
	//turn off engine for security action
	pt = strstr(buff,"batcoi");
	if(pt != NULL)
	{
		sysEventFlag |= HORN_ON_FLAG;
		BuzzerSetStatus(5000,10,BUZZER_TURN_ON,255);
		INFO("\n\rSMS:BAT COI\n\r");
	}
	//Set warning turn off engine for security action
	pt = strstr(buff,"acoff");
	if(pt != NULL)
	{
		sysCfg.securityAction &= ~WARNING_CALL;
		INFO("\n\rSMS:Clear security action: WARNING_CALL\n\r");
		flagCfgSave = 1;
	}
	
	//Set warning turn off engine for security action
	pt = strstr(buff,"acon");
	if(pt != NULL)
	{
		sysCfg.securityAction |= WARNING_CALL;
		INFO("\n\rSMS:Set security action: WARNING_CALL\n\r");
		flagCfgSave = 1;
	}
	
	//Set sos times
	pt = strstr(buff,"sost,");
	if(pt != NULL)
	{
		smsScanf(pt,"sost,%s",tempBuff0);
		sysCfg.SOSKeyPressTimes = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:SOSKeyPressTimes:%d\n\r",sysCfg.SOSKeyPressTimes);
		flagCfgSave = 1;
	}
	
	//Set sos PERIOD
	pt = strstr(buff,"sosp,");
	if(pt != NULL)
	{
		smsScanf(pt,"sosp,%s",tempBuff0);
		sysCfg.SOSKeyPressPeriod = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:SOSKeyPressPeriod:%d\n\r",sysCfg.SOSKeyPressPeriod);
		flagCfgSave = 1;
	}
	//Reset system
	pt = strstr(buff,"reset");
	if(pt != NULL)
	{
		NVIC_SystemReset();
	}
	
	//Set power low warning period
	pt = strstr(buff,"powp,");
	if(pt != NULL)
	{
		smsScanf(pt,"powp,%s",tempBuff0);
		sysCfg.powerLowWarningPeriod = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:powerLowWarningPeriod:%d\n\r",sysCfg.powerLowWarningPeriod);
		flagCfgSave = 1;
	}
	
	//Set bat low warning period
	pt = strstr(buff,"battp,");
	if(pt != NULL)
	{
		smsScanf(pt,"battp,%s",tempBuff0);
		sysCfg.powerLowWarningPeriod = (uint32_t)atol(tempBuff0);
		INFO("\n\rSMS:batteryLowWarningPeriod:%d\n\r",sysCfg.batteryLowWarningPeriod);
		flagCfgSave = 1;
	}
	//set default config
	pt = strstr(buff,"defcfg");
	if(pt != NULL)
	{
		CFG_LoadDefault();
		INFO("\n\rSMS:Set default config\n\r");
		flagCfgSave = 1;
	}
	
	//set default config
	pt = strstr(buff,"gps");
	if(pt != NULL)
	{
		sysEventFlag |=  GPS_SMS_SEND_FLAG;	
		INFO("\n\rSMS:Send GPS location\n\r");
	}
	
	BuzzerSetStatus(100,100,BUZZER_TURN_ON,1);
	if(flagCfgSave)
	{
		CFG_Save();
	}
	return 0;
}

