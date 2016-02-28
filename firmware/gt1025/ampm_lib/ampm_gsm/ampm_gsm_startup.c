/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/


#include "ampm_gsm_common.h"
#include "ampm_gsm_startup.h"

uint32_t Ampm_GSM_GetIMEI(uint16_t cnt,uint8_t c);
uint32_t Ampm_GSM_GetSimCID(uint16_t cnt,uint8_t c);
uint32_t Ampm_GSM_GetATI(uint16_t cnt,uint8_t c);
uint8_t gsmSimCIDBuf[32];
uint8_t flagGotSimCID = 0;
uint8_t modemCIDPass = 0;
uint8_t flagServerUpdateSimCID = 0;

uint8_t gsmIMEIBuf[18];
uint8_t flagGotIMEI = 0;
uint8_t newIMEI_IsUpdate = 0;
uint8_t modemIMEIPass = 0;



Timeout_Type tAmpm_GsmStartupTaskDelay;


uint8_t *ampm_AtGsmCmdStart_pt;

typedef enum{
	AMPM_GSM_STARTUP_INIT_PHASE = 0,
	AMPM_GSM_STARTUP_TURN_ON,
	AMPM_GSM_STARTUP_TURN_ON1,
	AMPM_GSM_STARTUP_GET_CMD_PHASE,
	AMPM_GSM_STARTUP_IDLE_PHASE
} AMPM_GSM_STARTUP_PHASE_TYPE;

AMPM_GSM_STARTUP_PHASE_TYPE ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;

typedef struct{
	uint32_t cmdType;
	uint8_t cmd[64];
	uint8_t res[32];
	uint32_t (*callback)(uint16_t cnt,uint8_t c);
	uint8_t ok[32];
	uint8_t err[32];
	uint32_t tryCnt;
	uint32_t timeout;
	uint32_t delay;
}AMPM_GSM_AT_SATRTUP;

AMPM_GSM_AT_SATRTUP at_cmd;
#define AMPM_GSM_BAUDRATE_SIZE	2
uint8_t ampm_gsm_baudrate_index = 0;
uint32_t gsmBaudrateSaved = 0;
const uint32_t ampm_gsm_baudrate_list[AMPM_GSM_BAUDRATE_SIZE] = {115200,230400};


uint8_t AMPM_GSM_Startup_Init(void)
{
	gsmBaudrateSaved = 0;
	ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;
	InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_MS(100));
	return 0;
}

uint8_t AMPM_GSM_Startup(const uint8_t *cmdList)
{
	uint32_t callback,uartBaud;
	switch(ampm_GSM_StartupPhase)
	{
        case AMPM_GSM_STARTUP_INIT_PHASE:
								if(gsmBaudrateSaved)
								{
									uartBaud = gsmBaudrateSaved;
								}
								else
								{
									ampm_gsm_baudrate_index %= AMPM_GSM_BAUDRATE_SIZE;
									uartBaud = ampm_gsm_baudrate_list[ampm_gsm_baudrate_index];
								}
                MODEM_UartInit(uartBaud);
								
                MODEM_MOSFET_Off();
                InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_SEC(2));
                ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_TURN_ON;
								AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_TURN_ON \r\n");
        break;
        case AMPM_GSM_STARTUP_TURN_ON:
                if (CheckTimeout(&tAmpm_GsmStartupTaskDelay) == SYSTICK_TIMEOUT)
                {
									MODEM_MOSFET_On();
									MODEM_DTR_Clr();
									MODEM_RESET_Set();
									MODEM_POWER_Set();
									InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_MS(1000));
									ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_TURN_ON1;
									AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_TURN_ON1 \r\n");
                }
        break;
        case AMPM_GSM_STARTUP_TURN_ON1:
                if (CheckTimeout(&tAmpm_GsmStartupTaskDelay) == SYSTICK_TIMEOUT)
                {
									MODEM_POWER_Set();
									ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_GET_CMD_PHASE;
									ampm_AtGsmCmdStart_pt = (uint8_t *)cmdList;
									InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_MS(1));
									ampm_GSM_CmdPhase = AMPM_CMD_OK;
									AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_CMD_OK \r\n");
                }
        break;
        case AMPM_GSM_STARTUP_GET_CMD_PHASE:
                if (CheckTimeout(&tAmpm_GsmStartupTaskDelay) == SYSTICK_TIMEOUT)
                {
                        if(ampm_GSM_CmdPhase == AMPM_CMD_OK)
                        {
                                memset(&at_cmd,0,sizeof(AMPM_GSM_AT_SATRTUP));
                                ampm_AtGsmCmdStart_pt = (uint8_t *)strstr((char *)ampm_AtGsmCmdStart_pt,"@@");
                                ampm_AtGsmCmdStart_pt += 2;
                                at_cmd.cmdType = ampm_AtGsmCmdStart_pt[0];
                                if(ampm_AtGsmCmdStart_pt == NULL)
                                {
                                  ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;
																	AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_INIT_PHASE \r\n");
																	ampm_gsm_baudrate_index++;
																	InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_SEC(5));
                                   break;
                                }
                                ampm_AtGsmCmdStart_pt++;
                               
                                InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_MS(1));
                                switch(at_cmd.cmdType)
                                {
                                        case '0':
																					
																				case 'G':
                                        sscanf((const char *)ampm_AtGsmCmdStart_pt,"$%[^$]$%[^$]$%d$%[^$]$%[^$]$%d$%d$%d\r\n",
                                                                                at_cmd.cmd,
                                                                                at_cmd.res,
                                                                                &callback,
                                                                                at_cmd.ok,
                                                                                at_cmd.err,
                                                                                &at_cmd.tryCnt,
                                                                                &at_cmd.timeout,
                                                                                &at_cmd.delay);
                                        if(at_cmd.delay == 0) at_cmd.delay = 300;
                                        if(at_cmd.timeout == 0) at_cmd.timeout = 1000;
																				switch(callback)
																				{
																					case 0:
																						at_cmd.callback = NULL;
																					break;
																					case 1:
																						at_cmd.callback = Ampm_GSM_GetIMEI;
																					break;
																					case 2:
																						at_cmd.callback = Ampm_GSM_GetSimCID;
																					break;
																					case 3:
																						at_cmd.callback = Ampm_GSM_GetATI;
																					break;
																					case 4:
																						at_cmd.callback = Ampm_GsmGetCSQ;
																					break;
																					default:
																						at_cmd.callback = NULL;
																					break;
																				}
																				Ampm_CmdTask_SendCmd(
																												at_cmd.callback,
																												at_cmd.res,
																												at_cmd.delay, 
																												at_cmd.ok, at_cmd.err, 
																												at_cmd.timeout, 
																												at_cmd.tryCnt,
																												at_cmd.cmd) ;
																				if(at_cmd.cmdType == 'G'){
																					ampmGSM_CmdSend.end_data_callback = Ampm_SetCmdOK;
																				}
                                        break;
                                        case '1':
                                                MODEM_POWER_Clr();
                                        break;
                                        case '2':
                                                MODEM_POWER_Set();
                                        break;
                                        case '3':
                                        sscanf((const char *)ampm_AtGsmCmdStart_pt,"$%d",&at_cmd.delay);
                                        if(at_cmd.delay >= 10000)
                                                        at_cmd.delay = 10000;
                                        InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_MS(at_cmd.delay));
                                        break;
                                        case '4':
                                        MODEM_RESET_Clr();
                                        break;
                                        case '5':
                                        MODEM_RESET_Set();
                                        break;
                                        case 'D':
                                        sscanf((const char *)ampm_AtGsmCmdStart_pt,"$%[^$]$\r\n",
                                                                                at_cmd.cmd);
                                        Ampm_CmdTask_SendCmd(
                                                                NULL,
                                                                NULL,
                                                                100, 
                                                                "OK", "ERROR", 
                                                                1000, 
                                                                3,
                                                                at_cmd.cmd) ;
                                        break;
																				case 'F':
                                        sscanf((const char *)ampm_AtGsmCmdStart_pt,"$%[^$]$\r\n",
                                                                                at_cmd.cmd);
                                        Ampm_CmdTask_SendCmd(
                                                                NULL,
                                                                NULL,
                                                                100, 
                                                                "OK", "ERROR", 
                                                                1000, 
                                                                1,
                                                                at_cmd.cmd) ;
																				ampmGSM_CmdSend.end_data_callback = Ampm_SetCmdOK;
                                        break;
                                        case 'S':
																					gsmBaudrateSaved = ampm_gsm_baudrate_list[ampm_gsm_baudrate_index];
																					ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_IDLE_PHASE;
																					AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_IDLE_PHASE \r\n");
																					break;
                                        default:
																					ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;
                                        break;
                                }
                        }else if (ampm_GSM_CmdPhase == AMPM_CMD_ERROR){
															ampm_gsm_baudrate_index++;
                              ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;
															AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_INIT_PHASE \r\n");
															InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_SEC(5));
                        }
                }
        break;
        case AMPM_GSM_STARTUP_IDLE_PHASE:
             if(modemIMEIPass && modemCIDPass)
              {
                  modemIMEIPass = 0;
                  modemCIDPass = 0;
                  return 1;
              }
              ampm_GSM_StartupPhase = AMPM_GSM_STARTUP_INIT_PHASE;
							AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:AMPM_GSM_STARTUP_INIT_PHASE \r\n");
							InitTimeout(&tAmpm_GsmStartupTaskDelay, SYSTICK_TIME_SEC(5));
              return 0;
        default:
						ampm_ModemResetFlag = 1;
             return 0;
	}
	return 0;
}




uint32_t Ampm_GSM_GetIMEI(uint16_t cnt,uint8_t c)
{
	if(cnt >= 18)
		return 0;
	if(c >= '0' && c <= '9')
		gsmIMEIBuf[cnt] = c;
	else
	{
		AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:Got IMEI:%s \r\n",gsmIMEIBuf);
		flagGotIMEI = 1;
		modemIMEIPass = 1;
		return 0;
	}
	return 0xff;
}

uint32_t Ampm_GSM_GetSimCID(uint16_t cnt,uint8_t c)
{
	if(cnt >= 30)
            return 0;
	if(c >= '0')
		gsmSimCIDBuf[cnt] = c;
	else
	{
		AMPM_GSM_LIB_DBG("\r\n AMPM_GSM_INIT:Got CID:%s \r\n",gsmSimCIDBuf);
		flagGotSimCID = 1;
		modemCIDPass = 1;
		return 0;
	}
	return 0xff;
}






