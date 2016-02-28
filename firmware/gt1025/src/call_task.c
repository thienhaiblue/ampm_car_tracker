
#include "system_config.h"
#include "lib/sparser.h"
#include "lib/sys_tick.h"
#include "lib/list.h"
#include "sst25.h"
#include "gps/gps.h"
#include "adc_task.h"
#include "sms_task.h"
#include "lib/encoding.h"
#include "alarm_task.h"
#include "led.h"
#include "dtmf/dtmf_app.h"
#include "accelerometer_task.h"

#include "ampm_gsm_main_task.h"

SMS_LIST_TYPE locationSms;
void SOS_Stop(void);
enum{
	SOS_START = 0,
	SOS_RUNING,
	SOS_IDLE,
	SOS_STOP
}sosPhase = SOS_STOP;

uint8_t sosTimes = 0;

const uint8_t smsCallReportLocation[] = "Vị Trí Hiện Tại";

uint8_t locationAskSmsBuf[160];
uint8_t thatIsYourBoss = 0;
uint8_t tWarningTurmOffCar = 0;
uint8_t tSosPhaseWarningTimeout = 0;
extern uint16_t tProtectOffTimeout;
uint8_t sosCarLockFlag = 0;
extern uint8_t StopAlarmKey_IsBusy(void);
extern uint8_t tCarLockTimeout;
uint8_t CallTask_IsBusy(void)
{
	if(sosPhase != SOS_STOP)
	{
		return 1;
	}
	return 0;
}

void CallTask(uint32_t rtc)
{
		uint8_t pdu2uniBuf[256],c;
		uint16_t smsLen = 0;
		static uint32_t time = 0;

		if(rtc != time)
		{
			time = rtc;

			if(tSosPhaseWarningTimeout) tSosPhaseWarningTimeout--;
			if(tWarningTurmOffCar)	tWarningTurmOffCar--;
		}

		if(VoiceCallTask_GetPhase() == INCOMING_CALL)
		{
			if(atCmdCallParser.getIncomingNumber
				 && locationSms.flag != SMS_NEW_MSG
			)
			{
				if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone1)
				|| ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone2)
				|| ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone3)
				)
				{
					if((!ACC_IS_ON || StopAlarmKey_IsBusy()) && (!CallTask_IsBusy()))
					{
						tProtectOffTimeout = 300;
						if(!StopAlarmKey_IsBusy())
						{
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,2);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,2);
						}
						else
						{
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,2);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,2);
							atCmdCallParser.cancelCall = 1;
						}
					}
					else
					{
						if(StopAlarmKey_IsBusy())
						{
							tProtectOffTimeout = 300;
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,2);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,2);
						}
						atCmdCallParser.autoAnswer = 1;
					}
					thatIsYourBoss = 1;
					atCmdCallParser.farHangUp = 0;
				}
				else
				{
					atCmdCallParser.cancelCall = 1;
					atCmdCallParser.autoAnswer = 0;
					atCmdCallParser.farHangUp = 0;
					thatIsYourBoss = 0;
				}		
			}
		}
		else if(VoiceCallTask_GetPhase() == IN_CALLING
		&& thatIsYourBoss
		)
		{
			if(RINGBUF_Get(&DTMF_ringBuff,&c) == 0)
			{
					switch(c)
					{
						case '1':
								BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,1);
								IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,1);
							break;
						case '5':
							alarmFlag |=  CAR_LOCK_CMD_FLAG;
							tWarningTurmOffCar = 30;
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,30);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,30);
							atCmdCallParser.cancelCall = 1;
							protectEnable = 1;
							smsLen =	sprintf((char *)pdu2uniBuf,"Bạn đã bật chế độ bảo vệ");
							smsLen = utf8s_to_ucs2s((int16_t *)locationAskSmsBuf,pdu2uniBuf);
							big2litel_endian((uint16_t *)locationAskSmsBuf,unilen((uint16_t *)locationAskSmsBuf));
							smsLen *= 2;
							if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone1) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone1,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone2) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone2,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone3) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone3,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}

						break;
						case '6':

						break;
						case '8':
							protectEnable = 0;
							atCmdCallParser.cancelCall = 1;
							SOS_Stop();
							StopAlarmKeyProtect();
							smsLen =	sprintf((char *)pdu2uniBuf,"Bạn đã tắt chế độ bảo vệ");
							smsLen = utf8s_to_ucs2s((int16_t *)locationAskSmsBuf,pdu2uniBuf);
							big2litel_endian((uint16_t *)locationAskSmsBuf,unilen((uint16_t *)locationAskSmsBuf));
							smsLen *= 2;
							if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone1) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone1,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone2) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone2,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone3) != NULL)
							{
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone3,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,1);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,1);
						break;
						case '9':
							alarmFlag |=  CAR_FIND_FLAG;
							BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,10);
							IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,10);
							atCmdCallParser.cancelCall = 1;
							getAddrFlag = 1;
							askAddrCnt = 0;
							smsLen =sprintf((char *)pdu2uniBuf,"%s!\nhttp://maps.google.com/maps?q=%.6f,%.6f",smsCallReportLocation,lastGpsInfo.lat,lastGpsInfo.lon);
							smsLen = utf8s_to_ucs2s((int16_t *)locationAskSmsBuf,pdu2uniBuf);
							big2litel_endian((uint16_t *)locationAskSmsBuf,unilen((uint16_t *)locationAskSmsBuf));
							smsLen *= 2;

							if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone1) != NULL)
							{
								addrSendToUser1Flag = 1;
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone1,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone2) != NULL)
							{
								addrSendToUser2Flag = 1;
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone2,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
							else if(ComparePhoneNumber((char *)atCmdCallParser.incomingCallPhoneNo,(char *)sysCfg.userPhone3) != NULL)
							{
								addrSendToUser3Flag = 1;
								Ampm_Sms_SendMsg(&locationSms,sysCfg.userPhone3,(uint8_t *)locationAskSmsBuf,smsLen,SMS_PDU16_MODE,30000,3);
							}
						break;
					}
			}
		}

		if(alarmFlag & CAR_LOCK_CMD_FLAG
		&& (tWarningTurmOffCar == 0)
		)
		{
				alarmFlag &=  ~CAR_LOCK_CMD_FLAG;
				sosCarLockFlag = 1;
				tCarLockTimeout = 180;
				sosPhase = SOS_START;
		}

		switch(sosPhase)
		{
			case SOS_START:
				tSosPhaseWarningTimeout = 0;
				sosTimes = 3;
				sosPhase = SOS_RUNING;
			break;
			case SOS_RUNING:
				if(tSosPhaseWarningTimeout == 0)
				{
					BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
					IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
					tSosPhaseWarningTimeout = 10;
					sosPhase = SOS_IDLE;
				}
			break;
			case SOS_IDLE:
				if(tSosPhaseWarningTimeout == 0)
				{
					if(sosTimes-- == 0)
						sosPhase = SOS_STOP;
					else
					{
						BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_ENABLE,60);
						IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_ENABLE,60);
						tSosPhaseWarningTimeout = 60;
						sosPhase = SOS_RUNING;
					}
				}
			break;
			case SOS_STOP:
				
			break;
		}

}


void SOS_Stop(void)
{
		sosPhase = SOS_STOP;
		alarmFlag &=  ~CAR_LOCK_CMD_FLAG;
		BuzzerSetStatus(&buzzer1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
		IO_ToggleSetStatus(&dout1Ctrl,500,500,IO_TOGGLE_DISABLE,30);
		sosCarLockFlag = 0;
}

void CALL_TaskInit(void)
{
	
}
