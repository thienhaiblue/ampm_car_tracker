#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "modem.h"
#include "lib/sys_tick.h"
#include "lib/data_cmp.h"
#include "at_command_parser.h"
#include "at_send_cmd_task.h"
#include "at_gsm_task.h"
#include "led.h"

uint32_t GSM_GetIMEI(uint16_t cnt,uint8_t c);
uint32_t GSM_GetCID(uint16_t cnt,uint8_t c);

const AT_CMD_PACKET_TYPE atGsmCmdStart[] = {
	"AT\r",NULL,NULL,"OK","ERROR",1,1000,100,
	"AT&K0\r",NULL,NULL,"OK","ERROR",1,1000,100,
	"ATI\r",NULL,NULL,"OK","ERROR",1,1000,100,
	"AT+UPSV=2\r",NULL,NULL,"OK","ERROR",1,1000,100,
	//"AT&D1\r",NULL,NULL,"OK","ERROR",1,1000,100,
	//"AT&C1\r",NULL,NULL,"OK","ERROR",1,1000,100,
	"AT+CGSN\r","+CGSN\r\r\n",GSM_GetIMEI,"OK","ERROR",1,1000,100,
	"AT+CMGF=1\r",NULL,NULL,"OK","ERROR",1,1000,100,
	"AT+CNMI=2,1,0,0,0\r",NULL,NULL,"OK","ERROR",1,2000,100,
	"AT+CLIP=1\r",NULL,NULL,"OK","ERROR",1,1000,100,
};


AT_CMD_LIST_TYPE atGsmCmdListStart[sizeof(atGsmCmdStart)/sizeof(AT_CMD_PACKET_TYPE)];

GSM_PHASE_TYPE gsm_TaskPhase;
Timeout_Type tGsmTaskPhaseTimeOut;
Timeout_Type tGsmTaskDelay;
Timeout_Type tGprsEnable;
uint8_t registerNetworkRetry = 0;
uint8_t flagGsmStatus = GSM_NO_REG_STATUS;

uint8_t *gprsApn;
uint8_t *gprsUsr;
uint8_t *gprsPwd;
uint8_t enableGprsRetryCnt = 0;
uint8_t gprsEnableFlag = 0;
uint8_t gprsDisable = 0;
uint8_t gprsEnableCmd = 0;

void GsmTask_InitApn(uint8_t *apn,uint8_t *usr,uint8_t *pwd)
{
	gprsApn = apn;
	gprsUsr = usr;
	gprsPwd = pwd;
}

void GsmTask_Init(void)
{
	modemIsBusy = 0; 
	modemIsBusySource = 0;
	gsm_TaskPhase = MODEM_TURN_ON;
	gprsEnableFlag  = 0;
	gprsDisable = 0;
	gprsEnableCmd = 0;
	modemCmdMode = 1;
	modem_IsOnline = 0;
	InitTimeout(&tGprsEnable,SYSTICK_TIME_SEC(5));
}

void GsmTask_SetPhase(GSM_PHASE_TYPE phase)
{
	gsm_TaskPhase = phase;
}

GSM_PHASE_TYPE GsmTask_GetPhase(void)
{
	return gsm_TaskPhase;
}



void Gsm_Task(void)
{	
	uint16_t i;
	
		
	switch(gsm_TaskPhase)
	{
		case MODEM_TURN_ON:
			flagGsmStatus = GSM_NO_REG_STATUS;
			flagGotIMEI = 0;
			modemIMEIPass = 0;
			MODEM_UartInit(115200);
			LedSetStatus(&led1Ctrl,50,50,LED_TURN_ON,0xffffffff);	
			POWER_PIN_SET_OUTPUT;
			RESET_PIN_SET_OUTPUT;
			DTR_PIN_SET_OUTPUT;
			DTR_PIN_CLR;
			RTS_PIN_SET_OUTPUT;
			RTS_PIN_CLR;
			RESET_PIN_SET;
			POWER_PIN_SET;    // Turn on GSM
			InitTimeout(&tGsmTaskDelay, SYSTICK_TIME_MS(5));
			gsm_TaskPhase = MODEM_TURN_ON1;
			break;
		case MODEM_TURN_ON1:
				POWER_PIN_CLR;      // Turn off GSM
				InitTimeout(&tGsmTaskDelay, SYSTICK_TIME_MS(200));
				gsm_TaskPhase = MODEM_TURN_ON2;
			break;
		case MODEM_TURN_ON2:
				POWER_PIN_SET;      // Turn off GSM
				// ---------- Turn on GPRS module ----------
				RESET_PIN_CLR;    // Reset GSM
				InitTimeout(&tGsmTaskDelay, SYSTICK_TIME_MS(200));
				gsm_TaskPhase = MODEM_TURN_ON3;
			break;
		case MODEM_TURN_ON3:
				RESET_PIN_SET;    // Start GSM module (Release reset)  
				InitTimeout(&tGsmTaskDelay, SYSTICK_TIME_SEC(5));
				gsm_TaskPhase = MODEM_SEND_AT;
			break;
		case MODEM_SEND_AT:
			if (CheckTimeout(&tGsmTaskDelay) == SYSTICK_TIMEOUT)
			{
					if(ModemCmdTask_SendCmd(NULL,NULL,100, modemOk, modemError, 1000, 2, "AT\r") == MODEM_RES_OK)
					{	
						InitTimeout(&tGsmTaskPhaseTimeOut, SYSTICK_TIME_SEC(3));
						gsm_TaskPhase = MODEM_CONFIG_0;
					}
			}
		break;
		case MODEM_CONFIG_0:
			//COMM_Puts("AT+IPR=115200\r");
			//COMM_Puts("AT+IPR=115200\r");
			InitTimeout(&tGsmTaskDelay, SYSTICK_TIME_SEC(2));
			gsm_TaskPhase = MODEM_CONFIG_1;
		break;
		case MODEM_CONFIG_1:
			if (ModemCmdTask_GetPhase() == MODEM_CMD_OK && CheckTimeout(&tGsmTaskDelay) == SYSTICK_TIMEOUT)
			{
				//MODEM_UartInit(115200);
				for(i = 0;i < sizeof(atGsmCmdStart)/sizeof(AT_CMD_PACKET_TYPE);i++)
				{
					atGsmCmdListStart[i].pt = (AT_CMD_PACKET_TYPE *)&atGsmCmdStart[i];
					ModemCmdAdd(&atGsmCmdListStart[i]);
				}
				gsm_TaskPhase = MODEM_REGISTRATION_NETWORK;
				InitTimeout(&tGsmTaskPhaseTimeOut, SYSTICK_TIME_SEC(30));
			}
			else if (ModemCmdTask_GetPhase() == MODEM_CMD_ERROR)
			{
				gsm_TaskPhase = MODEM_TURN_ON;
			}
			break;
		
		case MODEM_REGISTRATION_NETWORK:
			if(ModemCmdCheckList_IsEmpty() && (ModemCmdTask_GetPhase() == MODEM_CMD_OK))
			{
				ModemCmdTask_SendCmd(NULL,NULL,100, modemOk, "NOT USE", 3000, 20,"AT+COPS=0,2\r");
				InitTimeout(&tGsmTaskPhaseTimeOut, SYSTICK_TIME_SEC(60));
				gsm_TaskPhase = MODEM_WAIT_REGISTRATION_NETWORK;
			}
			else if (CheckTimeout(&tGsmTaskPhaseTimeOut) == SYSTICK_TIMEOUT || ModemCmdTask_GetPhase() == MODEM_CMD_ERROR) 
			{
				gsm_TaskPhase = MODEM_TURN_ON;
			}
		break;
		case MODEM_WAIT_REGISTRATION_NETWORK:
			if(ModemCmdCheckList_IsEmpty() && (ModemCmdTask_GetPhase() == MODEM_CMD_OK))
			{
				if(modemIMEIPass)
				{
					modemIMEIPass = 0;
					gsm_TaskPhase = MODEM_SYS_COVERAGE_OK;
				}
				else
				{
					gsm_TaskPhase = MODEM_TURN_ON;
				}
			}
			else if (CheckTimeout(&tGsmTaskPhaseTimeOut) == SYSTICK_TIMEOUT || ModemCmdTask_GetPhase() == MODEM_CMD_ERROR) 
			{
				gsm_TaskPhase = MODEM_TURN_ON;
			}
		break;
		case MODEM_SYS_COVERAGE_OK:
			break;
		default:
			gsm_TaskPhase = MODEM_TURN_ON;
			break;
	}
}
