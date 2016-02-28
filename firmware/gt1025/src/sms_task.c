/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ampm_gsm_sms.h"
#include "lib/sparser.h"
#include "lib/sys_tick.h"
#include "lib/list.h"
#include "lib/encoding.h"
#include "ampm_gsm_main_task.h"
#include "system_config.h"
#include "tracker.h"
#include "database_app.h"
#include "app_config_task.h"

#define smsScanf	sscanf

#define INFO(...)	//DbgCfgPrintf(__VA_ARGS__)
#define SMS_TASK_DBG(...)	//PrintfDebug(__VA_ARGS__)

#define SMS_RETRY_TIMES	3

const uint8_t smsReplyOk[] = "Thành Công!";
const uint8_t addrString[] = "@ADDR:";

uint8_t smsReplyBuf[32];

uint8_t findCarFlag = 0;

uint8_t askAddrCnt = 0;
uint8_t getAddrFlag = 0;
uint8_t gotAddrFlag = 0;

SMS_LIST_TYPE replySms;
SMS_LIST_TYPE forwardSms;
uint8_t guestPhoneNumber[16];
uint8_t forwardToNumber[16];
uint8_t lastRecvSmsPhone[16];
uint8_t replySmsBuf[160];
uint8_t forwardSmsBuf[160];

uint8_t myAddr[160];
uint8_t myAddrLen = 0;

uint8_t smsSendBuff[256];
uint8_t * FindAllPhone(uint8_t *phone);


extern Timeout_Type tFwDownload;

void SMS_TaskInit(void)
{
	memset((uint8_t *)&replySms,NULL,sizeof(SMS_LIST_TYPE));
}


uint16_t UniConfigParser(uint16_t *in, uint16_t *out)
{
	uint8_t u8temp;
	uint16_t *u16pt;
	uint8_t buf[32];
	uint16_t len = 0;
	
	utf8s_to_ucs2s((int16_t *)buf,(uint8_t *)addrString);
	u16pt = uni_srtsrt((uint16_t *)in,(uint16_t *)buf); 
	if (u16pt != 0)
	{
			askAddrCnt = 0;
			getAddrFlag = 0;
			gotAddrFlag = 1;
			u8temp = unilen((uint16_t *)&u16pt[6]);
			u8temp *= 2;
			if(u8temp && u8temp < sizeof(myAddr))
			{
				memset(myAddr,0,sizeof(myAddr));
				memcpy(myAddr,&u16pt[6],u8temp);
				big2litel_endian((uint16_t *)myAddr,unilen((uint16_t *)myAddr));
			}
	}
	return len;
}


uint16_t CMD_CfgParse(char *cmdBuff, uint8_t pwdCheck,uint8_t sourceCmd)
{
	uint8_t pdu2uniBuf[256];
	char *buff, *pt;
	uint8_t *resPhone = 0;
	uint32_t  t1,t2,t3,t4,t5,t6;
	uint8_t smsLen = 0,len = 0;
	char tempBuff0[34],tempBuff1[16];
	uint8_t flagCfgSave = 0;
	uint8_t cmdOk = 0;
	uint8_t passOk = 0;
	uint8_t error;
	DATE_TIME t;
	uint16_t i;
	buff = cmdBuff;
	smsLen = utf8s_to_ucs2s((int16_t *)smsReplyBuf,(uint8_t *)smsReplyOk);
	big2litel_endian((uint16_t *)smsReplyBuf,unilen((uint16_t *)smsReplyBuf));
	smsLen *= 2;
	
	pt = strstr(buff,"SMS,");
	if(pt == NULL)
	{
		for(t1 = 0; t1 < strlen(buff);t1++)
		{
				if(islower(buff[t1]))
				 buff[t1] = toupper(buff[t1]);
		}
	}
	if(smsSender[0] == '+'){
			strcpy((char *)guestPhoneNumber,(char *)&smsSender[1]);
	}else{
		strcpy((char *)guestPhoneNumber,(char *)smsSender);
	}
	
	pt = strstr(buff,"DANGNHAP");
	if(pt != NULL)
	{
		smsLen =	sprintf((char *)pdu2uniBuf,"Không thành công!");
		smsLen = utf8s_to_ucs2s((int16_t *)replySmsBuf,pdu2uniBuf);
		big2litel_endian((uint16_t *)replySmsBuf,unilen((uint16_t *)replySmsBuf));
		smsLen *= 2;
		for(i = 0;i < CONFIG_MAX_DRIVERS;i++)
		{
			/*+84978779222 <-> 0978779222*/
			if(Ampm_ComparePhoneNumber((char *)&sysCfg.driverList[i].phoneNo[1],(char *)&guestPhoneNumber) != NULL)
			{
				newDriverSub = 1;
				sysCfg.driverIndex = i;
				CFG_Save();
				break;
			}
		}
		Ampm_Sms_SendMsg(&replySms,guestPhoneNumber,(uint8_t *)smsReplyBuf,smsLen,SMS_PDU16_MODE,30000,3);
		return 0;
	}
	
	pt = strstr(buff,"IN1:");
	if(pt != NULL)
	{
		INFO("\n\rSMS->PRINT REPORT START ...\n\r");
		// compare with saved password here
		smsScanf(pt,"IN1:%d%d%d%d%d%d",&t1,&t2,&t3,&t4,&t5,&t6);
		t.mday = t1;
		t.month = t2;
		t.year = t3 + 2000;
		t.hour = t4;
		t.min = t5;
		t.sec = t6;
		error = 0;
		if((t.mday > 31) || (t.mday <= 0)) error = 1;
		if((t.month > 12) || (t.month <= 0)) error = 1;
		if((t.year > 2099) || (t.year < 2012)) error = 1;
		if((t.hour > 24)) error = 1;
		if((t.min > 60)) error = 1;
		if((t.sec > 60)) error = 1;
		if(error)
		{
			smsLen = sprintf((char *)pdu2uniBuf, "Sai cấu trúc!\r\nIN1: dd mm yy hh mm ss");
		}
		else
		{
			len = sprintf((char *)&smsSendBuff[0], "Dang in...");
			len += sprintf((char *)&smsSendBuff[len], "\nTIME:%d/%d/%d %d:%d:%d",t1,t2,t3,t4,t5,t6);
			Ampm_SendSmsPolling(guestPhoneNumber,smsSendBuff);
			printfReportTime = t;
			if(DB_Print(&printfReportTime,1) == 0xff)
			{
				smsLen = sprintf((char *)pdu2uniBuf, "In không thành công!");
			}
			else
			{
				smsLen = sprintf((char *)pdu2uniBuf, "In thành công");
			}
		}
		smsLen = utf8s_to_ucs2s((int16_t *)replySmsBuf,pdu2uniBuf);
		big2litel_endian((uint16_t *)replySmsBuf,unilen((uint16_t *)replySmsBuf));
		smsLen *= 2;
		Ampm_Sms_SendMsg(&replySms,guestPhoneNumber,(uint8_t *)replySmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
		INFO("\n\rSMS->PRINT REPORT FINISH\n\r");
		return 0;
	}
	
	pt = strstr(buff,"IN2:");
	if(pt != NULL)
	{
		INFO("\n\rSMS->PRINT REPORT START ...\n\r");
		// compare with saved password here
		smsScanf(pt,"IN2:%d%d%d%d%d%d",&t1,&t2,&t3,&t4,&t5,&t6);
		t.mday = t1;
		t.month = t2;
		t.year = t3 + 2000;
		t.hour = t4;
		t.min = t5;
		t.sec = t6;
		error = 0;
		if((t.mday > 31) || (t.mday <= 0)) error = 1;
		if((t.month > 12) || (t.month <= 0)) error = 1;
		if((t.year > 2099) || (t.year < 2012)) error = 1;
		if((t.hour > 24)) error = 1;
		if((t.min > 60)) error = 1;
		if((t.sec > 60)) error = 1;
		if(error)
		{
			smsLen = sprintf((char *)pdu2uniBuf, "Sai cấu trúc!\r\nIN1: dd mm yy hh mm ss");
		}
		else
		{
			len = sprintf((char *)&smsSendBuff[0], "Dang in...");
			len += sprintf((char *)&smsSendBuff[len], "\nTIME:%d/%d/%d %d:%d:%d",t1,t2,t3,t4,t5,t6);
			Ampm_SendSmsPolling(guestPhoneNumber,smsSendBuff);
			printfReportTime = t;
			if(DB_Print(&printfReportTime,2) == 0xff)
			{
				smsLen = sprintf((char *)pdu2uniBuf, "In không thành công!");
			}
			else
			{
				smsLen = sprintf((char *)pdu2uniBuf, "In thành công");
			}
		}
		smsLen = utf8s_to_ucs2s((int16_t *)replySmsBuf,pdu2uniBuf);
		big2litel_endian((uint16_t *)replySmsBuf,unilen((uint16_t *)replySmsBuf));
		smsLen *= 2;
		Ampm_Sms_SendMsg(&replySms,guestPhoneNumber,(uint8_t *)replySmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
		INFO("\n\rSMS->PRINT REPORT FINISH\n\r");
		return 0;
	}
	
	if(pwdCheck)
	{
		pt = strstr(buff,"12345678");
		if(pt != NULL)
		{
			// compare with saved password here
			passOk = 1;
			INFO("\n\rSMS->PASSWORD OK\n\r");
		}

		pt = strstr(buff,"THIENHAIBLUE");
		if(pt != NULL)
		{
			// compare with saved password here
			passOk = 1;
			INFO("\n\rSMS->PASSWORD OK\n\r");
		}
	}
	else
	{
		passOk = 1;
	}
	len = strlen((char *)forwardToNumber);
	if(len < 6)
		len = 3;
	else
		len = 6;
	if(forwardToNumber[0] 
	&& Ampm_ComparePhoneNumber_1((char *)forwardToNumber,(char *)smsSender,len))
	{
		forwardToNumber[0] = 0;
		strcpy((char *)replySmsBuf,buff);
		replySmsBuf[sizeof(replySmsBuf) - 1] = 0;
		smsLen = strlen((char *)replySmsBuf);

		replySmsBuf[smsLen] = 0;
		Ampm_Sms_SendMsg(&replySms,lastRecvSmsPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,3);
		return 0xff;
	}
	
	if(!passOk)
	{
		resPhone = FindAllPhone(smsSender);
		if(resPhone ==  NULL && passOk == 0)
		{
			return 0xff;
		}
	}
	else
	{
		resPhone = guestPhoneNumber;
	}
	
	pt = strstr(buff,"SMS,");
	if(pt != NULL)
	{
		strcpy((char *)lastRecvSmsPhone,(char *)resPhone);
		smsScanf(pt,"SMS,%[^, :\t\n\r#]",forwardToNumber);

		strcpy((char *)forwardSmsBuf,(pt + strlen((char *)forwardToNumber) + 5));
		smsLen = strlen((char *)forwardSmsBuf);
		Ampm_Sms_SendMsg(&forwardSms,(uint8_t *)forwardToNumber,(uint8_t *)forwardSmsBuf,smsLen,SMS_TEXT_MODE,30000,3);
		//cmdOk = 1;
	}
	
	//check monney
	pt = strstr(buff,"*101#");
	if(pt != NULL)
	{
		Ampm_SendCommand("+CUSD:",modemError,5000,1,"ATD*101#;\r");
		//Ampm_SendCommand(modemOk,modemError,2000,1,NULL);
		SysTick_DelayMs(1000);
		t1 = 0;
		while(RINGBUF_Get(commRxRingBuf, &replySmsBuf[t1])==0)
		{
			if(t1 < 16 && (replySmsBuf[t1] == '"'))
				t1 = 0;
			else
				t1++;
			if(t1 >= sizeof(replySmsBuf)) break;
		}
		while(t1)
		{
			if(replySmsBuf[t1] == '"')
			{
				break;
			}
			t1--;
		}
		replySmsBuf[t1] = 0;
		Ampm_Sms_SendMsg(&replySms,resPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,3);
	}

	//check monney
	pt = strstr(buff,"*102#");
	if(pt != NULL)
	{
		Ampm_SendCommand("+CUSD:",modemError,5000,1,"ATD*102#;\r");
		//Ampm_SendCommand(modemOk,modemError,2000,1,NULL);
		SysTick_DelayMs(1000);
		t1 = 0;
		while(RINGBUF_Get(commRxRingBuf, &replySmsBuf[t1])==0)
		{
			if(t1 < 16 && (replySmsBuf[t1] == '"'))
				t1 = 0;
			else
				t1++;
			if(t1 >= sizeof(replySmsBuf)) break;
		}
		while(t1)
		{
			if(replySmsBuf[t1] == '"')
			{
				break;
			}
			t1--;
		}
		replySmsBuf[t1] = 0;
		Ampm_Sms_SendMsg(&replySms,resPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,3);
	}

	//check monney
	pt = strstr(buff,"*100*");
	if(pt != NULL)
	{
		for(len = 0;len < 32;len++)
		{
			if(pt[len] != '*' && pt[len] != '#' && (pt[len] < '0' || pt[len] > '9'))
			{
				pt[len] = 0;
				break;
			}
		}
		Ampm_SendCommand("+CUSD: ","ERROR",10000,1,"AT+CUSD=1,\"%s\",15\r",pt);
		SysTick_DelayMs(5000);
		t1 = 0;
		while(RINGBUF_Get(commRxRingBuf, &replySmsBuf[t1])==0)
		{
			if(t1 < 16 && (replySmsBuf[t1] == '"'))
				t1 = 0;
			else
				t1++;
			if(t1 >= sizeof(replySmsBuf)) break;
		}
		while(t1)
		{
			if(replySmsBuf[t1] == '"')
			{
				break;
			}
			t1--;
		}
		replySmsBuf[t1] = 0;
		Ampm_Sms_SendMsg(&replySms,resPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,3);
	}
	
	
	//warning speed set
	pt = strstr(buff,"CFG,21,");
	if(pt != NULL)
	{
		smsScanf(pt,"CFG,21,%s",tempBuff0);
		sysCfg.speedLimit = atoi((char *)tempBuff0);
		INFO("\n\rSMS->WARNING SPEED:%d\n\r",sysCfg.speedLimit);
		flagCfgSave = 1;
		cmdOk = 1;
	}
	//reset device
	pt = strstr(buff,"CFG,40");
	if(pt != NULL)
	{
		ResetMcuSet(MCU_RESET_AFTER_30_SEC);
		INFO("\n\rSMS->RESET DEVICE\\n\r");
		cmdOk = 1;
	}
	
	//CHECK IMEI
	pt = strstr(buff,"IMEI");
	if(pt != NULL)
	{
		sprintf((char *)replySmsBuf,"IMEI:%s",sysCfg.imei);
		smsLen = strlen((char *)replySmsBuf);
		Ampm_Sms_SendMsg(&replySms,resPhone,(uint8_t *)replySmsBuf,smsLen,SMS_TEXT_MODE,30000,SMS_RETRY_TIMES);
	}
	
	pt = strstr(buff, "FWUPDATE");
	if (pt != NULL)
	{
			InitTimeout(&tFwDownload,SYSTICK_TIME_SEC(600)); //wait for 10min
			cmdOk = 1;
	}
	
	if(Ampm_ComparePhoneNumber_1((char *)smsSender,(char *)sysCfg.whiteCallerNo,6) ==  SAME_NUMBER || pwdCheck == 0)
	{
		
		
		//change gprs password
		pt = strstr(buff,"CFG,1,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,1,%s",tempBuff0);
			memcpy((char *)sysCfg.gprsPwd,tempBuff0,sizeof(sysCfg.gprsPwd));
			INFO("\n\rSMS->NEW GPRS PASSWORD:%s\n\r",sysCfg.gprsPwd);
			len += sprintf((char *)&smsSendBuff[len], "CFG,1,%s\n", sysCfg.gprsPwd);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		
		//change gprs user
		pt = strstr(buff,"CFG,2,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,2,%s",tempBuff0);
			memcpy((char *)sysCfg.gprsUsr,tempBuff0,sizeof(sysCfg.gprsUsr));
			INFO("\n\rSMS->NEW GPRS USER:%s\n\r",sysCfg.gprsUsr);
			len += sprintf((char *)&smsSendBuff[len], "CFG,2,%s\n", sysCfg.gprsUsr);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		
		//change gprs apn
		pt = strstr(buff,"CFG,3,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,3,%s",tempBuff0);
			memcpy((char *)sysCfg.gprsApn,tempBuff0,sizeof(sysCfg.gprsApn));
			INFO("\n\rSMS->NEW GPRS APN:%s\n\r",sysCfg.gprsApn);
			len += sprintf((char *)&smsSendBuff[len], "CFG,3,%s\n", sysCfg.gprsApn);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//change Server Name
		pt = strstr(buff,"CFG,4,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,4,%[^, :\t\n\r#]:%d",tempBuff0,&t1);
			memcpy((char *)sysCfg.priDserverName,tempBuff0,sizeof(sysCfg.priDserverName));
			sysCfg.priDserverPort = t1;
			INFO("\n\rSMS->NEW GPRS DATA SERVER NAME:%s\n\r",sysCfg.priDserverName);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//change Server Name
		pt = strstr(buff,"CFG,5,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,5,%[^, :\t\n\r#]:%d",tempBuff0,&t1);
			memcpy((char *)sysCfg.secDserverName,tempBuff0,sizeof(sysCfg.secDserverName));
			sysCfg.secDserverPort = t1;
			INFO("\n\rSMS->NEW GPRS DATA SERVER NAME:%s\n\r",sysCfg.secDserverName);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//Clear log
		pt = strstr(buff,"CFG,12,0");
		if(pt != NULL)
		{
			DB_RingLogReset();
			INFO("\n\rSMS->CLEAR LOG\n\r");
			len += sprintf((char *)&smsSendBuff[len], "CFG,12->OK\n");
			cmdOk = 1;
		}
		//set default config	
		pt = strstr(buff, "CFG,13");
		if (pt != NULL)
		{
			DB_RingLogReset();
			strcpy((char *)tempBuff0, (char *)sysCfg.id);
			memset((void*)&sysCfg, 0xFF, sizeof sysCfg);
			CFG_Save();
			CFG_Load();
			strcpy((char *)sysCfg.id ,(char *)tempBuff0);
			CFG_Save();
			ResetMcuSet(MCU_RESET_AFTER_30_SEC);
			cmdOk = 1;
		}
		//change device id
		pt = strstr(buff,"CFG,14,");
		if(pt != NULL)
		{
			INFO("\n\rSMS->CHANGE DEVICE ID:%s\n\r",sysCfg.id);
			smsScanf(pt,"CFG,14,%s",tempBuff0);
			memcpy((char *)sysCfg.id,tempBuff0,sizeof(sysCfg.id));
			flagCfgSave = 1;
			cmdOk = 1;
		}
		
		//change plate no
		pt = strstr(buff,"CFG,15,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,15,%s",tempBuff0);
			memcpy((char *)sysCfg.plateNo,tempBuff0,sizeof(sysCfg.plateNo));
			INFO("\n\rSMS->CHANGE PLATE NO:%s\n\r",sysCfg.plateNo);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		
		//change interval time run
		pt = strstr(buff,"CFG,16,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,16,%[^/]/%s",tempBuff0,tempBuff1);
			sysCfg.runReportInterval = atoi((char *)tempBuff0);
			sysCfg.stopReportInterval = atoi((char *)tempBuff1);
			INFO("\n\rSMS->CHANGE INTERVAL TIME RUN:%d/%d\n\r",sysCfg.runReportInterval,sysCfg.stopReportInterval);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//set tyre rate
		pt = strstr(buff,"CFG,18,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,18,%s",tempBuff0);
			sysCfg.speedSensorRatio = atoi((char *)tempBuff0);
			INFO("\n\rSMS->CHANGE TYRE RATE:%d\n\r",sysCfg.speedSensorRatio);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//warning speed set
		pt = strstr(buff,"CFG,21,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,21,%s",tempBuff0);
			sysCfg.speedLimit = atoi((char *)tempBuff0);
			INFO("\n\rSMS->WARNING SPEED:%d\n\r",sysCfg.speedLimit);
			flagCfgSave = 1;
			cmdOk = 1;
		}
		//CHANGE PHONE BOSS
		pt = strstr(buff,"CFG,60,");
		if(pt != NULL)
		{
			smsScanf(pt,"CFG,60,%s",tempBuff0);
			memcpy((char *)sysCfg.whiteCallerNo,tempBuff0,sizeof(sysCfg.whiteCallerNo));
			INFO("\n\rSMS->BOSS PHONE:%s\n\r",sysCfg.whiteCallerNo);
			flagCfgSave = 1;
			cmdOk = 1;
		}
	}
		
	if(resPhone && cmdOk)
	{
		Ampm_Sms_SendMsg(&replySms,resPhone,(uint8_t *)smsReplyBuf,smsLen,SMS_PDU16_MODE,30000,3);
		strcpy((char *)smsSendBuff,"\r\nOK\r\n");
	}
	
	if(flagCfgSave)
	{
		CFG_Save();
	}

	return 0xff;
}


uint8_t * FindAllPhone(uint8_t *phone)
{
		if(Ampm_ComparePhoneNumber((char *)phone,(char *)"0978779222") != NULL)
		{
			return "0978779222";
		}

		return NULL;
}
