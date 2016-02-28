
#ifndef __FTP_H__
#define __FTP_H__
#include <stdint.h>
#include "tcp_ip_task.h"

TCP_STATE_TYPE FTP_Manage(void);
void FTP_Init(void);
void FTP_Reset(void);
uint8_t FTP_Task_IsIdle(void);
#endif /* __FTP_H__ */
