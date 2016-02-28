#ifndef __RF_CONTROL_H__
#define __RF_CONTROL_H__

#include "type.h"
#include "spi.h"
#include "hal_rf.h"
#include <cc2500.h>
#include <stdlib.h>
#include "tick.h"
#include "dbg_cfg_app.h"

#define FUN_KEY0	1
#define FUN_KEY1	2

#define RF_INIT	1
#define RF_DEINIT	0


#define KEY_LOCK	1
//#define KEY_UNLOCK	2
#define KEY_HORN_ON	4
#define KEY_HORN_OFF	8
#define KEY_SOS 2

extern uint32_t tcpConnectionStatus;
extern uint32_t rfDriverStatus;

void RF_GDO0_PIN_ISR(void);
void RF_Process (void);
uint8_t RF_ProcessInit(void);
void RF_ProcessDeInit(void);
void ForceToRxMode(void);
void ForceToWORMode(void);
#endif
