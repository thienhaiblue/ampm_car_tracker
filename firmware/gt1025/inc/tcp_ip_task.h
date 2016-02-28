#ifndef	__TCP_IP_TASK__H__
#define __TCP_IP_TASK__H__
#include <stdint.h>
#include "ppp.h"
#include "lib/sys_tick.h"
#define PPP_RESET_TIMEOUT	120
#define TCP_BUSY_TIMEOUT	30

typedef enum {
	INITIAL,
	CONNECT,
	CONNECTED,
	LOGGED_IN,
	REPORTING,
	WAIT_TIMEOUT_RECONNECT
}TCP_STATE_TYPE;

extern Timeout_Type tcpIpReset;
extern Timeout_Type tTcpDataIsBusy;
extern uint8_t tcpIpTryCnt;
uint8_t vTcpIpTask(void);
uint8_t vTcpIpTaskInit(void);

#endif

