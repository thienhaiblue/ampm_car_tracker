#ifndef __AT_COMMAND_PARSER_H__
#define __AT_COMMAND_PARSER_H__
#include "modem.h"
#include "uart1.h"
#include "stdio.h"
#include "string.h"
#include "ringbuf.h"
#include "system_config.h"
#include "gps.h"
#include "uart1.h"

// #define SET_NEW_PASSWORD										"ABCFG,0,"
// #define CHANGE_GPRS_PASS										"ABCFG,1,"
// #define CHANGE_GPRS_USER										"ABCFG,2,"
// #define CHANGE_GPRS_APN										"ABCFG,3,"
// #define CHANGE_DATA_SERVER_NAME						"ABCFG,4,"
// #define CHANGE_DATA_SERVER_IP_AND_PORT			"ABCFG,5,xxx.xxx.xxx.xxx:xxxxx"  
// #define CHANGE_DATA_SERVER_USE_IP					"ABCFG,6,"
// #define CHANGE_FIRMWARE_SERVER_NAME				"ABCFG,8,"
// #define CHANGE_FIRMWARE_SERVER_IP_AND_PORT					"ABCFG,9,xxx.xxx.xxx.xxx:xxxxx"
// #define CHANGE_FIRMWARE_SERVER_USE_IP			"ABCFG,10,"
// #define CLEAR_LOG													"ABCFG,12"
// #define SET_DEFAULT_FACTORY								"ABCFG,13,"
// #define SET_DEVICE_ID											"ABCFG,14,"
// #define SET_PLATE_NO												"ABCFG,15,"
// #define SET_ON_OFF_INTERVAL								"ABCFG,16,xx/xx"   //timeon/timeoff
// #define SET_TYRE_RATE											"ABCFG,18,"
// #define SET_BEEP_ENABLE										"ABCFG,19"
// #define SET_BEEP_DISABLE										"ABCFG,20"
// #define SET_WARNING_SPEED									"ABCFG,21,"
// #define GET_PRODUCT_INFO										"ABCFG,22"
// #define GET_STATUS													"ABCFG,23"
// #define GET_CELL_ID												"ABCFG,24"
// #define GET_GPS_LOCATION										"ABCFG,25"


#define SOCKET_NUM		3
#define GPRS_DATA_MAX_LENGTH	600
#define GPRS_KEEP_DATA_INFO_LENGTH	16
#define SOCKET_CLOSE	0
#define SOCKET_OPEN 	1

extern uint8_t socketMask[SOCKET_NUM];

extern uint8_t createSocketNow;

extern uint32_t smsNewMessageFlag,ringFlag,tcpSocketStatus[SOCKET_NUM];

extern uint32_t tcpTimeReCheckData[SOCKET_NUM];

extern uint8_t DNS_serverIp[16];

extern uint8_t callerPhoneNum[16];

extern uint8_t inCalling;

extern uint8_t smsSender[18];

extern RINGBUF smsUnreadRingBuff;

extern RINGBUF gprsRingBuff;

extern uint32_t GPRS_dataUnackLength[SOCKET_NUM];

uint8_t RemaskSocket(uint8_t socket);
void AT_CmdProcessInit(void);
void AT_ComnandParser(char c);
void SMS_Manage(uint8_t *buff,uint32_t lengBuf);
uint32_t GPRS_CmdRecvData(uint8_t *gprsBuff,uint16_t gprsBuffLen,uint32_t *dataLen);
uint32_t GPRS_SendCmdRecvData(uint8_t socketNum,uint8_t *gprsBuff,uint16_t gprsBuffLen,uint32_t *dataLen);
void AT_CommandCtl(void);
#endif
