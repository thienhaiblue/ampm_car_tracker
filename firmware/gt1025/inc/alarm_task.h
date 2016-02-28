
#ifndef __ALARM_TASK_H__
#define __ALARM_TASK_H__

#include <stdint.h>
#include "lib/sys_tick.h"
extern uint32_t alarmFlag;

extern uint8_t gSensorAlarmTimes;
extern Timeout_Type tGetAddr;
void AlarmTaskInit(void);
void AlarmTask(uint32_t rtc);
uint8_t AlarmTask_IsBusy(void);
void SosHoldDetect(void);
void AlarmHoldDetect(void);
void MicrowaveDetect(void);

#endif
