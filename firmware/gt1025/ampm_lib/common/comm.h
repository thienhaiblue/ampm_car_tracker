
#ifndef __COMM_H__
#define __COMM_H__
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lib/ringbuf.h"


extern uint32_t  DbgCfgPrintf(const uint8_t *format, ...);

#define PPP_Info(...)		DbgCfgPrintf(__VA_ARGS__)
#define PPP_Debug(...)	DbgCfgPrintf(__VA_ARGS__)
#define AT_CMD_Debug(...)	DbgCfgPrintf(__VA_ARGS__)
#define UIP_PPP_Info(...) DbgCfgPrintf(__VA_ARGS__)
#define AMPM_GSM_LIB_DBG(...) DbgCfgPrintf(__VA_ARGS__)
#define MODEM_Info(...)		DbgCfgPrintf(__VA_ARGS__)


extern RINGBUF USART1_TxRingBuff;
extern RINGBUF USART1_RxRingBuff;


extern RINGBUF *commTxRingBuf;
extern RINGBUF *commRxRingBuf;

extern uint8_t USART1_PutChar(uint8_t ch);
extern void USART1_PutString (uint8_t *s);
extern void COMM_Putc(uint8_t c);
extern void COMM_Puts(uint8_t *s);

extern uint8_t COMM_CarrierDetected(void);
extern void MODEM_UartInit(uint32_t baudrate);
extern void MODEM_DTR_Set(void);
extern void MODEM_DTR_Clr(void);
extern void MODEM_MOSFET_On(void);
extern void MODEM_MOSFET_Off(void);
extern void MODEM_POWER_Set(void);
extern void MODEM_POWER_Clr(void);
extern void MODEM_RESET_Set(void);
extern void MODEM_RESET_Clr(void);

#endif
