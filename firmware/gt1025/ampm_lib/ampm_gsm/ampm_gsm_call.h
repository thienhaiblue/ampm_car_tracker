/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/

#ifndef __AMPM_GSM_CALL_H__
#define __AMPM_GSM_CALL_H__

#define PHONE_NO_LENGTH									16
#define SAME_NUMBER											1
#define DIFFERENT_NUMBER								0
#define PHONE_DIGIT_THRESHOLD						6

typedef enum{
	CALL_RESPONSE_NO_ERROR = 0,
	CALL_RESPONSE_MISS,
	CALL_RESPONSE_BUSY,
	CALL_RESPONSE_NO_ANSWER,
	CALL_RESPONSE_UNSUCCESS,
}CALL_RESPONSE;

typedef struct{
	CALL_RESPONSE error;
	uint8_t ringing;
	uint8_t inCalling;
	uint8_t busyFlag;
	uint8_t noAnswerFlag;
	uint8_t callUnsuccess;
	uint8_t incomingCall;
	uint8_t getIncomingNumber;
	uint8_t farHangUp;
	uint16_t extendedErrorCode;
	uint8_t pickUpCallFlag;
	uint8_t incomingCallPhoneNo[16];
	uint8_t ringingDetectFlag;
	uint8_t ringingCountDown;
	uint8_t ringingFlag;
	uint8_t missCall;
	uint8_t startCall;
	uint8_t autoAnswer;
	uint8_t autoTurnOffCall;
	uint8_t cancelCall;
	uint8_t answerCall;
	uint8_t answerCallRetry;
	uint8_t hangUpCallRetry;
	uint8_t startCallRetry;
	uint8_t startCallStatus;
	uint16_t calloutTalkingTimeout;
	uint16_t incommingCallTalkingTimeout;
}AT_CMD_CALL_TYPE;

extern AT_CMD_CALL_TYPE ampmCallPara;
extern uint8_t ampm_IncomingCallPhoneNo[16];
extern uint8_t ampm_GotIncomingNumberFlag;
uint8_t Ampm_ComparePhoneNumber_1(char* phone1, char* phone2,uint8_t digitThreshold);
uint8_t Ampm_ComparePhoneNumber(char* phone1, char* phone2);
#endif





