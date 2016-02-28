#include "resolver.h"
#include "tcpip.h"
#include "lib/sys_tick.h"
#include "lib/sys_time.h"

#define RESOLVER_INTERVAL		(1800000) //1hour

U64 resolverTick;


enum{
	RSLV_INITIAL,
	RSLV_QUERY,
	RSLV_STANDBY,
}resolverState;
	
void RESOLVER_Reset(void)
{
	resolverState = RSLV_INITIAL;
	resolverTick = SysTick_Get();
}

void RESOLVER_Manage(void)
{
	uip_ipaddr_t dnsServerIp;
	switch(resolverState){
		case RSLV_INITIAL:
			resolv_init();			
			//uip_ipaddr(&dnsServerIp, 8, 8, 8, 8);
                        resolv_conf(&priDnsServerIp);
			resolv_conf(&dnsServerIp);
			resolverState = RSLV_QUERY;
		case RSLV_QUERY:
			resolverTick = SysTick_Get();
			resolverState = RSLV_STANDBY;
		case RSLV_STANDBY:
			if(rtcTimeSec - SysTick_Get() > RESOLVER_INTERVAL)
				resolverState = RSLV_QUERY;
			break;
			
		default:
			break;
	}
}
uip_ipaddr_t *RESOLVER_Lookup(const I8 *name)
{
	return (uip_ipaddr_t*)resolv_lookup(name);
}
void RESOLVER_Query(const I8 *name)
{
	resolv_query(name);
}
void resolv_found(I8 *name, uip_ipaddr_t *ipaddr)
{
	if(ipaddr != NULL){
		RESOLVER_Info("RESOLV: name: %s, address: %d.%d.%d.%d\n", name, ipaddr->u8[0], ipaddr->u8[1], ipaddr->u8[2], ipaddr->u8[3]);
	}
}

