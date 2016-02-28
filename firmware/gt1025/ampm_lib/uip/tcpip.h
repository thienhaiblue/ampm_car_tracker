
#ifndef __TCP_IP_H__
#define __TCP_IP_H__

#include "uipopt.h"
#include "uip.h"

void tcpip_uipcall(void);
struct uip_conn *tcp_connect(uip_ipaddr_t *ripaddr, uint16_t port,void (*callback)(void),void *appstate);
struct uip_udp_conn *udp_new(const uip_ipaddr_t *ripaddr, uint16_t port,void (*callback)(void),void *appstate);
void tcp_attach(struct uip_conn *conn,void (*callback)(void),void *appstate);
#define tcp_markconn(conn, callback,appstate) tcp_attach(conn,callback, appstate)
void tcp_unlisten(uint16_t port,void (*callback)(void));
void tcp_listen(uint16_t port,void (*callback)(void));
#endif
