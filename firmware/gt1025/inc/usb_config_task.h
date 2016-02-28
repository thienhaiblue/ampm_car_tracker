
#ifndef __USB_CONFIG_TASK_H__
#define __USB_CONFIG_TASK_H__

#include "app_config_task.h"

#define  USB_PACKET_SIZE		36

void UsbConfigTaskInit(void);
void UsbConfigTask(void);

#endif

