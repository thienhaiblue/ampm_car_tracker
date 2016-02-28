
#include "rfid.h"
#include "system_config.h"
#include "buzzer.h"
#include "database_app.h"
#include "alarm_task.h"
#include "led.h"

uint8_t rfidTimeout = 0;
uint8_t flagRfidTag = 0;
uint8_t rfidTagId[32];


enum {
	RFID_INIT,
	RFID_CHECK_OLD_STATE,
	RFID_WAIT_ACC_ON,
	RFID_WAIT_TAG,
	RFID_TAG_OK,
	RFID_TAG_FAIL,
	RFID_CHECK_ACCELEROMETER,
	RFID_WAIT_ACC_OFF
}rfidPhase = RFID_INIT;

void RFID_Input(char c)
{
	static char len;
	uint8_t i,j,ok = 0;
	//if(rfidPhase == RFID_WAIT_TAG)
	{
		switch(c)
		{
			case 0x02:
				len = 0;
			break;
			case 0x0D:
			case 0x0A:
			case 0x03:
				if(len < CONFIG_SIZE_RFID_PASS) 
				{
					for(j = 0;j < CONFIG_SIZE_RFID_USER_MAX;j++)
					{
						if(strstr((char *)sysCfg.rfidPass[j],ADD_RFID_TAG_ID) != NULL)
						{
							memcpy(sysCfg.rfidPass[j],rfidTagId,len);
							CFG_Save();
						}
						ok = 1;
						for(i = 0;i < 16 && i < len;)
						{
							if(rfidTagId[i] != sysCfg.rfidPass[j][i])
							{
								ok = 0;
								break;
							}
							rfidTagId[i] = 0;
							i++;
						}
						if(ok)
						{
							flagRfidTag = 1;
							break;
						}
					}
					len = 0;
				}
			break;
			default:
				if(len < 16)
				{
					if(c >= '0' &&  c <= '9')
					{
						rfidTagId[len] = c;
						len++;
					}
					else
					{
						len = 0;
					}
				}
				else
					len = 0;
			break;
		}
	}
}


void RFID_Task(void)
{
	if(sysCfg.featureSet & FEATURE_RFID)
	{
		if(rfidTimeout) rfidTimeout--;
		if(!ACC_IS_ON && rfidPhase != RFID_WAIT_ACC_ON)	
		{
			rfidPhase = RFID_WAIT_ACC_ON;
			BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_OFF, 500);//200ms on/off , 1 times
		}
		
		switch(rfidPhase)
		{
			case RFID_INIT:
				rfidTimeout = 3;
				rfidPhase = RFID_CHECK_OLD_STATE;
			break;
			case RFID_CHECK_OLD_STATE:
				if(rfidTimeout == 0)
				{
					if(ACC_IS_ON)
					{
						if(rfidState.value == RFID_TAG_FAIL)
						{
							OUTPUT1_PIN_SET; //turn off engine
							rfidPhase = RFID_TAG_FAIL;
							BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 500);//200ms on/off , 1 times
							rfidTimeout = 30; //30sec
						}
						else
							rfidPhase = RFID_WAIT_ACC_OFF;
					}
					else
						rfidPhase = RFID_WAIT_ACC_ON;
				}
			break;
			case RFID_WAIT_ACC_ON:
				if(ACC_IS_ON)
				{
					flagRfidTag = 0;
					OUTPUT1_PIN_SET; //turn off engine
					rfidPhase = RFID_WAIT_TAG;
					OUTPUT2_PIN_SET; //enable RFID reader
					rfidTimeout = 5; //5 secont
				}
			break;
			case RFID_WAIT_TAG:
				if(flagRfidTag == 1 || smsEnableCar == 1)
				{
					smsEnableCar = 0;
					flagRfidTag = 0;
					OUTPUT2_PIN_CLR;//disable RFID reader
					rfidPhase = RFID_TAG_OK;
				}
				if(rfidTimeout == 0)
				{
					OUTPUT1_PIN_SET; //turn off engine
					rfidPhase = RFID_TAG_FAIL;
					rfidState.value = rfidPhase;
					BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 500);//200ms on/off , 1 times
					rfidTimeout = 180; //180sec
				}
			break;
				
			case RFID_TAG_OK:
				BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 1);//200ms on/off , 1 times
				OUTPUT1_PIN_CLR; //turn on engine
				rfidPhase = RFID_WAIT_ACC_OFF;
				rfidState.value = rfidPhase;
			break;
				
			case RFID_TAG_FAIL:
				if(rfidTimeout == 0)
				{
					rfidPhase = RFID_CHECK_ACCELEROMETER;
					alarmFlag &= ~RFID_ACCELEROMETER_FLAG;
					OUTPUT2_PIN_CLR;//disable RFID reader
					OUTPUT1_PIN_CLR; //turn on engine
				}
				if(flagRfidTag == 1 || smsEnableCar == 1)
				{
					flagRfidTag = 0;
					OUTPUT2_PIN_CLR;//disable RFID reader
					rfidPhase = RFID_TAG_OK;
				}
			break;
				
			case RFID_CHECK_ACCELEROMETER:
				if(alarmFlag & RFID_ACCELEROMETER_FLAG)
				{
					BuzzerSetStatus(&buzzer1Ctrl,100, 100,BUZZER_TURN_ON, 500);//200ms on/off , 1 times
					flagRfidTag = 0;
					OUTPUT2_PIN_SET;//disable RFID reader
					alarmFlag &= ~RFID_ACCELEROMETER_FLAG;
					OUTPUT1_PIN_SET; //turn off engine
					rfidPhase = RFID_TAG_FAIL;
					rfidTimeout = 30; //30sec
				}
				if(flagRfidTag == 1 || smsEnableCar == 1)
				{
					smsEnableCar = 0;
					flagRfidTag = 0;
					OUTPUT2_PIN_CLR;//disable RFID reader
					rfidPhase = RFID_TAG_OK;	
				}
			break;
			
			case RFID_WAIT_ACC_OFF:
				if(!ACC_IS_ON)
				{
					rfidPhase = RFID_WAIT_ACC_ON;
					OUTPUT1_PIN_CLR; //turn on engine
				}
			break;
			default:
				rfidPhase = RFID_WAIT_ACC_ON;
			break;
		}
	}
}
