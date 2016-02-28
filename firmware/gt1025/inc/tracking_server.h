#ifndef _TRACKING_SVR_H_
#define _TRACKING_SVR_H_

#include "typedef.h"
#include "uip.h"
#include "resolver.h"
#include "lib/sys_tick.h"
#include "tcp_ip_task.h"


extern TCP_STATE_TYPE TRACKING_SVR_Manage(void);
extern void TRACKING_SVR_Reset(void);
extern void TRACKING_SVR_Callback(void);
extern void TRACKING_SVR_Init(uint32_t priority);

#endif
