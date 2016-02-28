
#include <stdint.h>
#include <stdio.h>
#include "uip.h"
#include "tcpip.h"

#if UIP_TCP
/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  uint16_t port;
  void (*callback)(void);
};

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  struct process *p;
} s;
#endif

void tcpip_uipcall(void)
{
  uip_udp_appstate_t *ts;
#if UIP_UDP
  if(uip_conn != NULL) {
    ts = &uip_conn->appstate;
  } else {
    ts = &uip_udp_conn->appstate;
  }
#else /* UIP_UDP */
  ts = &uip_conn->appstate;
#endif /* UIP_UDP */
#if UIP_TCP
 {
   static unsigned char i;
   struct listenport *l;
   
   /* If this is a connection request for a listening port, we must
      mark the connection with the right process ID. */
   if(uip_connected()) {
     l = &s.listenports[0];
     for(i = 0; i < UIP_LISTENPORTS; ++i) {
       if(l->port == uip_conn->lport &&
	  l->callback != NULL) {
		ts->callback = l->callback;
		ts->state = NULL;
	 break;
       }
       ++l;
     }
   }
 }
#endif /* UIP_TCP */
  
  if(ts->callback != NULL) {
    ts->callback();
  }
}
//void tcpip_uipcall(void)
//{
//	if(uip_conn->appstate.callback)
//		uip_conn->appstate.callback();
//}

struct uip_conn *
tcp_connect(uip_ipaddr_t *ripaddr, uint16_t port,void (*callback)(void),void *appstate)
{
  struct uip_conn *c;
  
  c = uip_connect(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }
  c->appstate.callback = callback;
	c->appstate.state = appstate;
  return c;
}

struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, uint16_t port,void (*callback)(void),void *appstate)
{
  struct uip_udp_conn *c;
  uip_udp_appstate_t *s;
  c = uip_udp_new(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }
	s = &c->appstate;
  s->callback = callback;
	s->state = appstate;
  return c;
}

/*---------------------------------------------------------------------------*/
void tcp_listen(uint16_t port,void (*callback)(void))
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = port;
      l->callback = callback;
      uip_listen(port);
      break;
    }
    ++l;
  }
}

void tcp_unlisten(uint16_t port,void (*callback)(void))
{
  static unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == port &&
       l->callback == callback) {
      l->port = 0;
      uip_unlisten(port);
      break;
    }
    ++l;
  }
}

void tcp_attach(struct uip_conn *conn,void (*callback)(void),void *appstate)
{
  uip_tcp_appstate_t *s;
  s = &conn->appstate;
  s->callback = callback;
  s->state = appstate;
}





