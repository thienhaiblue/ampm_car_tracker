
#include "reporter.h"
#include "config_pool.h"
#include "xtea.h"
#include "tick.h"



#define INFO(...) //PrintfDebug(__VA_ARGS__)


#define RECONNECT_TIMEOUT		10 * TICK_SECOND

volatile enum{
	INITIAL,
	CONNECT,
	CONNECTED,
	LOGGED_IN,
	REPORTING,
	WAIT_TIMEOUT_RECONNECT
}rpState = INITIAL;

uip_ipaddr_t *rpServerIp;
struct uip_conn *rpConn;
U32 rpTick;
static I16 rexmitCnt = 0;
I16 timedOutCnt = 0;

void REPORTER_Reset()
{
	rpConn = NULL;
	rpState = INITIAL;
}

void REPORTER_Manage()
{

}
void REPORTER_Callback()
{
	
}


