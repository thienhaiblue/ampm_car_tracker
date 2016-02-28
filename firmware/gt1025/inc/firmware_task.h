#ifndef _FIRMWARE_TASK_H_
#define _FIRMWARE_TASK_H_

#include "typedef.h"
#include "uip.h"
#include "resolver.h"
#include "lib/sys_tick.h"
#include "tcp_ip_task.h"



TCP_STATE_TYPE FIRMWARE_Manage(void);
void FIRMWARE_Reset(void);
void FIRMWARE_Callback(void);
void FIRMWARE_Init(uint32_t priority);
void FIRMWARE_Task(uint8_t *buff);
uint8_t FirmwareTask_IsBusy(void);

#endif
