/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbdesc.c
 * Purpose: USB Descriptors
 * Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC microcontroller devices only. Nothing else
 *      gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.20 Changed string descriptor handling
 *          V1.00 Initial Version
 *---------------------------------------------------------------------------*/
#include "usb.h"
#include "cdc.h"
#include "msc.h"
#include "hid.h"
#include "usbcfg.h"
#include "usbdesc.h"



/* HID Report Descriptor */
const uint8_t HID_ReportDescriptor[] = {
  HID_UsagePageVendor(0x00),
  HID_Usage(0x01),
  HID_Collection(HID_Application),
	HID_UsageMin(1),
	HID_UsageMax(0x40),
	HID_LogicalMin(0),
	HID_LogicalMax(0x40),
	HID_ReportCount(0x40),
	HID_ReportSize(0x08),
	HID_Input(HID_Data),
	HID_UsageMin(0),
	HID_UsageMax(0x40),
	HID_Output(HID_Data),
  HID_EndCollection,
};


const uint16_t HID_ReportDescSize = sizeof(HID_ReportDescriptor);



/* USB Standard Device Descriptor */
const uint8_t USB_DeviceDescriptor_0[] = {
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 2.20 */          /* bcdUSB */
  0xEF,                              /* bDeviceClass */
  0x02,                              /* bDeviceSubClass */
  0x01,                              /* bDeviceProtocol */
  USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
  WBVAL(0x2047),                     /* idVendor */
  WBVAL(0x0301),                     /* idProduct */
  WBVAL(0x0000), /* 2.00 */          /* bcdDevice */
  0x01,                              /* iManufacturer */
  0x02,                              /* iProduct */
  0x03,                              /* iSerialNumber */
  0x01                               /* bNumConfigurations */
};




uint8_t *USB_DeviceDescriptor = (uint8_t *)USB_DeviceDescriptor_0;

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t USB_ConfigDescriptor_0[] = {
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(                             /* wTotalLength */
    USB_CONFIGUARTION_DESC_SIZE +
    USB_INTERFACE_DESC_SIZE     +
    HID_DESC_SIZE               +
    USB_ENDPOINT_DESC_SIZE			+
		USB_ENDPOINT_DESC_SIZE
  ),
  0x01,                              /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x00,                              /* iConfiguration */
  USB_CONFIG_POWERED_MASK /*|*/       /* bmAttributes */
/*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(500),          /* bMaxPower */
/* Interface 0, Alternate Setting 0, HID Class */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  USB_HID_IF_NUM,                              /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x02,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_HUMAN_INTERFACE,  /* bInterfaceClass */
  HID_SUBCLASS_NONE,                 /* bInterfaceSubClass */
  HID_PROTOCOL_NONE,                 /* bInterfaceProtocol */
  0,                              /* iInterface */
/* HID Class Descriptor */
/* HID_DESC_OFFSET = 0x0012 */
  HID_DESC_SIZE,                     /* bLength */
  HID_HID_DESCRIPTOR_TYPE,           /* bDescriptorType */
  WBVAL(0x0111), /* 1.00 */          /* bcdHID */
  0x00,                              /* bCountryCode */
  0x01,                              /* bNumDescriptors */
  HID_REPORT_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(HID_REPORT_DESC_SIZE),       /* wDescriptorLength */
/* Endpoint, HID Interrupt In */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_IN(1),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0x01,          /* 32ms */          /* bInterval */
/* Terminator */
/* Endpoint, HID Interrupt OUT */
  USB_ENDPOINT_DESC_SIZE,            /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_OUT(1),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
  WBVAL(0x0040),                     /* wMaxPacketSize */
  0x01,          /* 32ms */          /* bInterval */
/* Terminator */
  0                                  /* bLength */
};



uint8_t *USB_ConfigDescriptor = (uint8_t *)USB_ConfigDescriptor_0;

/* USB String Descriptor (optional) */
const uint8_t USB_StringDescriptor[] = {
/* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
/* Index 0x01: Manufacturer */
  (9*2 + 2),                        /* bLength (13 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'A',0,
  'M',0,
  'B',0,
  'O',0,
  '-',0,
  'T',0,
  'E',0,
  'C',0,
  'H',0,
/* Index 0x02: Product */
  (12*2 + 2),                        /* bLength ( 17 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'A',0,
  'M',0,
  'B',0,
  'O',0,
  '-',0,
  'G',0,
  'T',0,
  '4',0,
  '0',0,
  '0',0,
  'V',0,
  'N',0,
/* Index 0x03: Serial Number */
  (1*2 + 2),                        /* bLength (12 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  '0',0,
/* Index 0x04: Configuration String */
  (8*2 + 2),                        /* bLength ( 17 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'A',0,
  'M',0,
  'B',0,
  'O',0,
  ' ',0,
  'U',0,
  'S',0,
  'B',0,
/* Index 0x05: Interface 0, Alternate Setting 0 */
  (13*2 + 2),                        /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'M',0,
  'S',0,
  'C',0,
  ' ',0,
	'I',0,
  'n',0,
  't',0,
  'e',0,
	'r',0,
  'f',0,
  'a',0,
  'c',0,
	'e',0,
/* Index 0x06: Interface 0, Alternate Setting 0 */
  (13*2 + 2),                        /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'M',0,
  'S',0,
  'C',0,
  ' ',0,
	'I',0,
  'n',0,
  't',0,
  'e',0,
	'r',0,
  'f',0,
  'a',0,
  'c',0,
	'e',0,
/* Index 0x07: Interface 0, Alternate Setting 0 */
  (30*2 + 2),                        /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
	'G',0,
  'T',0,
  '4',0,
  '0',0,
  '0',0,
  'V',0,
  'N',0,
  ' ',0,
  'V',0,
  'i',0,
  'r',0,
  't',0,
	'u',0,
  'a',0,
  'l',0,
  ' ',0,
	'C',0,
  'O',0,
  'M',0,
  ' ',0,
	'P',0,
	'o',0,
	'r',0,
	't',0,
	' ',0,
	'(',0,
	'C',0,
	'D',0,
	'C',0,
	')',0,
/* Index 0x06: Interface 0, Alternate Setting 0 */
  (13*2 + 2),                        /* bLength (4 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'H',0,
  'I',0,
  'D',0,
  ' ',0,
	'I',0,
  'n',0,
  't',0,
  'e',0,
	'r',0,
  'f',0,
  'a',0,
  'c',0,
	'e',0,
};
