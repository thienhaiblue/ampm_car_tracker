/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "usb_config_task.h"
#include "usbcore.h"
#include "usbhw.h"
#include "hiduser.h"
#include "cdcuser.h"

uint8_t usbRecv = 0;
uint8_t PrevXferComplete;
uint32_t enableUSB_Manager = 0;

CFG_PROTOCOL_TYPE USB_ProtoRecv;
CFG_PROTOCOL_TYPE USB_ProtoSend;
uint8_t USB_DataBuff[USB_PACKET_SIZE];
PARSER_PACKET_TYPE USB_parserPacket;
void CfgUSBSendData(CFG_PROTOCOL_TYPE *data);

void UsbConfigTask(void)
{
	uint8_t buff[65],len;
	uint8_t c;
	if (USB_Connected)
	{
		//USB HID
		while(RINGBUF_Get(&HID_RxRingBuff,&c) == 0)
		{
			if(CfgParserPacket(&USB_parserPacket,&USB_ProtoRecv,c) == 0)
					break;
		}
		CfgProcessData(&USB_ProtoRecv,&USB_ProtoSend,USB_DataBuff,USB_parserPacket.lenMax - 4,1);
		CfgUSBSendData(&USB_ProtoSend);

		if(HID_DepInEmpty)
		{
			len = 0;
			while((RINGBUF_Get(&HID_TxRingBuff,&c) == 0))
			{
				buff[len] = c;
				len++;
				if(len >= 64) break;
			}
			if(len)
			{
				HID_DepInEmpty = 0;
				USB_WriteEP (HID_DATA_IN,buff,64);
			}
		}
	}
}


void UsbConfigTaskInit(void)
{
	USB_HID_Init();
	AppConfigTaskInit();
	USB_ProtoRecv.dataPt = USB_DataBuff;
	USB_parserPacket.state = CFG_CMD_WAITING_SATRT_CODE;
	USB_parserPacket.lenMax = USB_PACKET_SIZE;
	
}

void CfgUSBSendData(CFG_PROTOCOL_TYPE *data)
{
	uint8_t i;
	if(data->length)
	{
		RINGBUF_Put(&HID_TxRingBuff,data->start);
		RINGBUF_Put(&HID_TxRingBuff,((uint8_t *)&data->length)[0]);
		RINGBUF_Put(&HID_TxRingBuff,((uint8_t *)&data->length)[1]);
		RINGBUF_Put(&HID_TxRingBuff,data->opcode);
		for(i = 0;i < data->length;i++)
			RINGBUF_Put(&HID_TxRingBuff,data->dataPt[i]);
		RINGBUF_Put(&HID_TxRingBuff,data->crc);
		data->length = 0;
	}
}
