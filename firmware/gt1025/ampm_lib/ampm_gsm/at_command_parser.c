/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "at_command_parser.h"
#include "lib/sys_tick.h"

#define INFO(...)	//DbgCfgPrintf(__VA_ARGS__)
uint32_t Ampm_URC_CLIP_Process(uint16_t cnt,uint8_t c);
uint32_t Ampm_URC_RING_Process(uint16_t cnt,uint8_t c);

uint8_t ampm_AtCommandParserEnable = 0;

static const STR_INFO_Type AT_ProcessCmd[] = {
	"+CLIP: \"", Ampm_URC_CLIP_Process,
	"RING" , Ampm_URC_RING_Process,
  "+CMGL: ", Ampm_SMS_URC_Incomming,
	"+CMGR: ",Ampm_SMS_ReadMsg,
	"+CMTI: \"SM\",",Ampm_SMS_Process,
	"+CMTI: \"ME\",",Ampm_SMS_Process,
	"+CMTI: \"MT\",",Ampm_SMS_Process,
};



char *commandSeparator = " ,:\r\n";
const uint8_t terminateStr[7] = "\r\nOK\r\n";
uint8_t cmdCnt[STR_PARSER_COUNT(AT_ProcessCmd)];
STR_PARSER_Type AT_CmdParser;

 
/*
+CLIP: "0978779222",161,,,,0
*/
uint32_t Ampm_URC_CLIP_Process(uint16_t cnt,uint8_t c)
{
	static uint8_t incomingCallPhoneNo[16];
	if((c >= '0') && (c <= '9'))
	{
		 incomingCallPhoneNo[cnt] = c;
		 incomingCallPhoneNo[cnt+1] = '\0';
	}
	else 
	{
		if(c == '"' && cnt >= 2)
		{
			memcpy(ampm_IncomingCallPhoneNo,incomingCallPhoneNo,sizeof(incomingCallPhoneNo));
			ampm_GotIncomingNumberFlag = 1;
		}
		Ampm_SetAtClip();
		return 0;
	}
	if(cnt > 14)
	{
		Ampm_SetAtClip();
		return 0;
	}
	return 0xff;
}

uint32_t Ampm_URC_RING_Process(uint16_t cnt,uint8_t c)
{
	Ampm_SetAtRing();
	return 0;
}

void AT_CmdProcessInit(void)
{
	StrParserInit(&AT_CmdParser,cmdCnt,STR_PARSER_COUNT(AT_ProcessCmd),1024);
}

void AT_ComnandParser(char c)
{
	static uint32_t RingBuffInit = 0;
	if(RingBuffInit == 0)// int ring buff
	{
		RingBuffInit = 1;
		AT_CmdProcessInit();
	}
	if(ampm_AtCommandParserEnable){
		StrComnandParser(SysTick_Get(),AT_ProcessCmd,&AT_CmdParser,cmdCnt,c);
	}
}

