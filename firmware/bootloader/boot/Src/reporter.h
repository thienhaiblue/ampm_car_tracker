#ifndef _REPORTER_H_
#define _REPORTER_H_

#include "typedef.h"
#include "uip.h"
#include "resolver.h"

extern void REPORTER_Manage(void);
extern void REPORTER_Reset(void);
extern void REPORTER_Callback(void);
extern void REPORTER_Init(uint32_t priority);
extern void vReporterTask(void *pvParameters);

#endif
