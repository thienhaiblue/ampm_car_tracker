


#ifndef __DTMF_APP_H__
#define __DTMF_APP_H__
#include "lib/ringbuf.h"

extern RINGBUF DTMF_ringBuff;

extern void DTMF_DeInit(void);
extern char DTMF_Task(void);
extern void DTMF_Init(void);


#endif
