/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/
#ifndef __AMPM_GSM_SMS_H__
#define __AMPM_GSM_SMS_H__

#define SMS_MAX_RETRY	10

#define SMS_MSG_SENT	0
#define SMS_MSG_FAIL	1
#define SMS_NEW_MSG		2
#include <stdint.h>
typedef enum {
	SMS_PDU8_MODE = 0,
	SMS_PDU16_MODE,
	SMS_TEXT_MODE
	
}SMS_MODE_TYPE;

typedef struct{
	struct SMS_LIST_TYPE *next;
	uint8_t 	*phone;
	uint8_t 	*msg; 				//  maximun 159 character
	uint8_t len;
	uint8_t 	timeout; //second
	uint8_t timeoutReload;
	uint8_t 	tryNum;
	uint8_t flag;
	SMS_MODE_TYPE mode;
}SMS_LIST_TYPE;

extern void (*Ampm_SmsRecvCallback)(uint8_t *buf);
extern uint8_t smsTaskFailCnt;
extern uint8_t smsSender[];
void Ampm_SmsTask_Init(void (*callback)(uint8_t *buf));
uint32_t Ampm_SMS_Process(uint16_t cnt,uint8_t c);
uint32_t Ampm_SMS_URC_Incomming(uint16_t cnt,uint8_t c);
uint32_t Ampm_SMS_Process(uint16_t cnt,uint8_t c);
uint32_t Ampm_SMS_ReadMsg(uint16_t cnt,uint8_t c);
uint8_t Ampm_StartRecvSms(void);
uint8_t Ampm_RecvSms_IsFinished(void);
uint8_t Ampm_SmsTask(void);
uint16_t Ampm_Sms_SendMsg(SMS_LIST_TYPE *SMS,uint8_t *phone,uint8_t *msg,uint8_t len,SMS_MODE_TYPE mode,uint16_t timeout,uint8_t tryNum);
uint8_t Ampm_SmsCheckMessage_IsEmpty(void);
uint8_t Ampm_SmsTaskPeriodic_1Sec(void);
uint8_t Ampm_SendSmsPolling(const uint8_t *recipient, const uint8_t *data);
#endif


