#ifndef __DBG_CFG_APP_H__
#define __DBG_CFG_APP_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
//#include "typedef.h"

//#include "type.h"

#include "usbreg.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "hid.h"
#include "hiduser.h"
#include "stm32f10x.h"
#include "uart1.h"
#include "ringbuf.h"
#include "hw_config.h"
#include "accelerometer.h"
#include "horn.h"

typedef struct _ledDEF
{
    uint32_t ledOnTime;
		uint32_t ledOffTime;
		uint32_t ledCounter;
		uint8_t ledStatus;
} led_type;



extern led_type	ledCtr;	

#define TIMER_PERIOD	1	//ms

#define ENABLE_CLK_LED_PORT	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN
#define LED_PORT	GPIOA
#define LED_PIN	GPIO_BSRR_BS15
#define LED_SET_OUTPUT	LED_PORT->CRH	&= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15); LED_PORT->CRH	|= (GPIO_CRH_MODE15_0)
#define LED_SET_INPUT		LED_PORT->CRH	&= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15); LED_PORT->CRH	|= (GPIO_CRH_CNF15_0)
#define LED_PIN_SET					LED_PORT->BSRR = LED_PIN
#define LED_PIN_CLR					LED_PORT->BRR = LED_PIN
#define LED_ON_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define LED_OFF_TIME_DFG	(500 / TIMER_PERIOD) /*1s */
#define LED_TURN_ON	1
#define LED_TURN_OFF 0


#define LedSetStatus(x,y,z)		ledCtr.ledOnTime = x;\
															ledCtr.ledOffTime = y;\
															ledCtr.ledCounter = x + y;\
															ledCtr.ledStatus = z
															
#define LedInit()			LED_SET_OUTPUT;\
											LedSetStatus(LED_ON_TIME_DFG,LED_OFF_TIME_DFG,LED_TURN_OFF)
#define LedDeinit()	LED_SET_INPUT
											
#define LedCtr()	if(ledCtr.ledStatus == LED_TURN_ON) \
									{\
											if(ledCtr.ledCounter == 0) {ledCtr.ledCounter = ledCtr.ledOffTime + ledCtr.ledOnTime;LED_PIN_SET;}\
											ledCtr.ledCounter--; \
											if(ledCtr.ledCounter == ledCtr.ledOffTime) LED_PIN_CLR;	\
									}

									
#define USB_PACKET_SIZE		36
#define UART_PACKET_SIZE		36
#define FLASH_PACKET_SIZE		32
#define GPRS_PACKET_SIZE	516


typedef struct
{
	uint32_t packetNo;
	uint16_t packetLengthToSend;
	uint8_t  *dataPt;
} CFG_DATA_TYPE;

typedef enum{
	CFG_CMD_NEW_STATE,
	CFG_CMD_GET_LENGTH,
	CFG_CMD_GET_OPCODE,
	CFG_CMD_GET_DATA,
	CFG_CMD_CRC_CHECK,
	CFG_CMD_WAITING_SATRT_CODE
}CFG_CMD_STATE_TYPE;


typedef struct
{
	uint8_t start;
	uint16_t length;
	uint8_t opcode;
	uint8_t *dataPt;
	uint8_t crc;
} CFG_PROTOCOL_TYPE;

typedef struct
{
	CFG_CMD_STATE_TYPE state;
	uint16_t len;
	uint16_t lenMax;
	uint16_t cnt;
	uint8_t opcode;
	uint8_t crc;
} PARSER_PACKET_TYPE;

extern uint8_t smsSendMessageFlag;

extern uint8_t smsAddr[16];

extern uint8_t smsMsg[160];

extern uint32_t flagPrint;

extern uint32_t flagCalling;

extern uint32_t firmwareSendFileOffSetFlag;

extern uint32_t firmwareStatus;

extern uint32_t firmwareFileSize;

extern uint32_t firmwareFileOffSet;

extern uint8_t fSendBuff[64];

extern uint8_t fFlagSendData;

extern uint16_t fDataLength;

void LoadFirmwareFile(void);
uint8_t CfgParserPacket(PARSER_PACKET_TYPE *parserPacket,CFG_PROTOCOL_TYPE *cfgProtoRecv,uint8_t c);
uint8_t CfgProcessData(CFG_PROTOCOL_TYPE *cfgProtoRecv,CFG_PROTOCOL_TYPE *cfgProtoSend,uint8_t *cfgSendDataBuff,uint32_t maxPacketSize,uint8_t logSendEnable);
uint32_t  DbgCfgPrintf(const uint8_t *format, ...);
uint32_t DbgCfgPutChar(uint8_t c);
uint8_t CfgCalcCheckSum(uint8_t *buff, uint32_t length);

extern uint32_t enableUSB_Manager;
void GetInReport  (void);
void SetOutReport (void);
void TIMER2_Init(uint32_t pclk);
void USB_Manager(void);


#endif

