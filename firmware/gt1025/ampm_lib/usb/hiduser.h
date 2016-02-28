/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    HIDUSER.H
 *      Purpose: HID Custom User Definitions
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing 
 *      else gives you the right to use this software.
 *
 *      Copyright (c) 2005-2009 Keil Software.
 *---------------------------------------------------------------------------*/

#ifndef __HIDUSER_H__
#define __HIDUSER_H__
#include <stdint.h>
#include "lib/ringbuf.h"

/* HID Number of Reports */
#define HID_REPORT_NUM      1
#define HID_REPORT_NUM      1
/* HID Data In/Out Endpoint Address */
#define HID_DATA_IN       0x81
#define HID_DATA_OUT      0x01

extern RINGBUF HID_TxRingBuff;
extern RINGBUF HID_RxRingBuff;
extern uint8_t HID_DepInEmpty;
extern uint8_t HID_TxBuff[128];
extern uint8_t HID_RxBuff[64];

/* HID Global Variables */
extern uint8_t HID_Protocol;
extern uint8_t HID_IdleTime[HID_REPORT_NUM];

/* HID Requests Callback Functions */
extern uint32_t HID_GetReport   (void);
extern uint32_t HID_SetReport   (void);
extern uint32_t HID_GetIdle     (void);
extern uint32_t HID_SetIdle     (void);
extern uint32_t HID_GetProtocol (void);
extern uint32_t HID_SetProtocol (void);
extern  void USB_HID_Init(void);

#endif  /* __HIDUSER_H__ */
