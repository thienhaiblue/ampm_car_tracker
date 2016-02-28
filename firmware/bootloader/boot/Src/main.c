#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "usbreg.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "hid.h"
#include "hiduser.h"
#include "system_stm32f10x.h"
#include "sst25.h"
#include "stm32f10x_flash.h"
#include "flashprog.h"

#define SPI_SD_CS_PORT	GPIOA
#define SPI_SD_CS_PIN					GPIO_BSRR_BS1
#define SPI_SD_CS_SET_OUTPUT		SPI_SD_CS_PORT->CRL	&= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1); SPI_SD_CS_PORT->CRL	|= (GPIO_CRL_MODE1_0 | GPIO_CRL_MODE1_1)
#define SPI_SD_CS_SET_INPUT		SPI_SD_CS_PORT->CRL	&= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1); SPI_SD_CS_PORT->CRL	|= (GPIO_CRL_CNF1_0)
#define SPI_SD_CS_PIN_SET			SPI_SD_CS_PORT->BSRR = SPI_SD_CS_PIN
#define SPI_SD_CS_PIN_CLR			SPI_SD_CS_PORT->BRR = SPI_SD_CS_PIN

#define SPI_FL_CS_PORT	GPIOA
#define SPI_FL_CS_PIN					GPIO_BSRR_BS4
#define SPI_FL_CS_SET_OUTPUT		SPI_FL_CS_PORT->CRL	&= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4); SPI_FL_CS_PORT->CRL	|= (GPIO_CRL_MODE4_0 | GPIO_CRL_MODE4_1)
#define SPI_FL_CS_SET_INPUT		SPI_FL_CS_PORT->CRL	&= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4); SPI_FL_CS_PORT->CRL	|= (GPIO_CRL_CNF4_0)
#define SPI_FL_CS_PIN_SET			SPI_FL_CS_PORT->BSRR = SPI_FL_CS_PIN
#define SPI_FL_CS_PIN_CLR			SPI_FL_CS_PORT->BRR = SPI_FL_CS_PIN

#define USB_rxBuff	HID_receivedBuf

#define USER_FLASH_START		0x08002800

uint8_t buff[4096];
uint8_t flashBuff[PAGE_SIZE];
uint32_t fileSize;
uint32_t fileCrc;
uint32_t packetNo;
uint32_t firmwareFileOffSet;
uint32_t firmwareFileSize;
uint8_t USB_txBuff[64];
uint8_t usbRecv = 0;
uint32_t memAddr = 0;
uint32_t flashCheckSum;
void SysDeInit(void);
void HardFault_Handler(void)
{
		NVIC_SystemReset();
}

void SetOutReport (void)
{
	usbRecv = 1;
}

void GetInReport  (void)
{

}

uint8_t CfgCalcCheckSum(uint8_t *buff, uint32_t length)
{
	uint32_t i;
	uint8_t crc = 0;
	for(i = 0;i < length; i++)
	{
		crc += buff[i];
	}
	return crc;
}

typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;

void execute_user_code(void)
{
	SysDeInit();
	if (((*(vu32*)USER_FLASH_START) & 0x2FFF0000 ) == 0x20000000)
    { /* Jump to user application */
			//SCB->VTOR = USER_FLASH_START;
      JumpAddress = *(vu32*) (USER_FLASH_START + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(vu32*) USER_FLASH_START);
      Jump_To_Application();
    }    
}

void SysInit(void)
{
	//__disable_irq();
	SystemInit();
	RCC->APB2ENR = (RCC_APB2ENR_AFIOEN |  /*enable clock for Alternate Function*/
								 RCC_APB2ENR_IOPAEN |   /* enable clock for GPIOA*/
								 RCC_APB2ENR_IOPBEN |	/*enable clock for GPIOB*/
								 RCC_APB2ENR_IOPCEN | /*enable clock for GPIOc*/ 									 
								 RCC_APB2ENR_SPI1EN	/*enable clock for SPI1*/
								);     
	RCC->CFGR |= RCC_CFGR_USBPRE; // precale is 1 for usb clock
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;/*enable clock for USB*/
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;	//Disable JTAG funtion, keep SWJ funtion for debug(only user when PB3,PB4,PB5,PA15 user for another funtion)
	
	//SPI1 Init
	GPIOA->CRL 	 &= ~(GPIO_CRL_MODE4 | GPIO_CRL_CNF4 | 
										GPIO_CRL_MODE5 | GPIO_CRL_CNF5 | 
										GPIO_CRL_MODE6 | GPIO_CRL_CNF6 | 
										GPIO_CRL_MODE7 | GPIO_CRL_CNF7); // Clear PB4,PB5, PB6, PB7          
	GPIOA->CRL   |=  	GPIO_CRL_MODE4_0 | GPIO_CRL_MODE4_1; // PB4 General purpose output push-pull								
	GPIOA->CRL   |=   GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1; // PB5 Alternate function output push-pull
	GPIOA->CRL   |=   GPIO_CRL_CNF6_1 | GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1; // PB6 Alternate function output push-pull
	GPIOA->CRL   |=   GPIO_CRL_CNF7_1 | GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1; // PB7 Alternate function output push-pull
	
	SPI_SD_CS_SET_OUTPUT;
	SPI_SD_CS_PIN_SET;
	__enable_irq();
}

void SysDeInit(void)
{
	//default reset
	__disable_irq();
	NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
	
	GPIOA->CRH = 0x88844444;
	GPIOA->CRL = 0x44444444;
	
	GPIOB->CRH = 0x44444444;
	GPIOB->CRL = 0x44444444;
	
	GPIOC->CRH = 0x44444444;
	GPIOC->CRL = 0x44444444;
	
	RCC->APB1ENR = 0;
	RCC->APB2ENR = 0;
	USB_Connect(FALSE);
	__enable_irq();
}

void Delay(uint32_t i)
{
	while(i--);
}

int main(void)
{
	uint32_t *u32Pt = (uint32_t *)buff;
	uint8_t *u8pt,u8temp;
	int32_t i = 3000000,j = 0;
	uint32_t u32temp,timeOut = 20000000;
	uint8_t tryNum,exit;
	SysInit();
	while(i--);
	//USB Connect enable
	GPIOB->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13);                /* clear port PB13 */
  GPIOB->CRH |=  GPIO_CRH_MODE13_1;                /* PB13 General purpose output open-drain, max speed 50 MHz */
 	GPIOB->BRR  = GPIO_BRR_BR13; 
/*USB Init*/
 	USB_Init();        	  // USB Initialization
  USB_Connect(TRUE);    // USB Connect
	i = 10000000;
//	DelayMs(1000);
	while(i && (USB_Connected == 0))
	{
		i--;
	}
	FLASH_Unlock();
	if(USB_Connected)
		while(timeOut)
		{
			timeOut--;
			//if(USB_Connected)
			//	timeOut = 3000000;
			 if((usbRecv))
			 {
						usbRecv = 0;
						USB_ReadEP(0x01,USB_rxBuff);
						u8temp = CfgCalcCheckSum((uint8_t *)&USB_rxBuff[4],USB_rxBuff[1]);
						if((USB_rxBuff[0] == 0xCA) && (u8temp == USB_rxBuff[USB_rxBuff[1] + 4]))
						{
							for(i = 0;i < 64;i++)
								USB_txBuff[i] = 0;
							switch(USB_rxBuff[3])
							{
								case 0x12:
									//timeOut = 20000000;
									u32Pt = (uint32_t *)&USB_rxBuff[4];
									packetNo = *u32Pt;
									if(packetNo == 0xA5A5A5A5)
									{
										firmwareFileOffSet = 0;
										u32Pt = (uint32_t *)&USB_rxBuff[8];
										firmwareFileSize = *u32Pt;
										USB_txBuff[1] = 12; //length
										USB_txBuff[3] = 0x12; //opcode
										
										USB_txBuff[4] = USB_rxBuff[4];
										USB_txBuff[5] = USB_rxBuff[5];
										USB_txBuff[6] = USB_rxBuff[6];
										USB_txBuff[7] = USB_rxBuff[7];
										//Max file size
										USB_txBuff[8] = USB_rxBuff[8];
										USB_txBuff[9] = USB_rxBuff[9];
										USB_txBuff[10] = USB_rxBuff[10];
										USB_txBuff[11] = USB_rxBuff[11];
										//Max packet size
										USB_txBuff[12] = 32; //max packet size
									}
									else
									{
										timeOut = 20000000;
										if(packetNo == firmwareFileOffSet)
										{
											firmwareFileOffSet += (USB_rxBuff[1] - 4);
											u32temp = firmwareFileOffSet % PAGE_SIZE;
											for(i = 0;i < (USB_rxBuff[1] - 4);i++)
											{
												flashBuff[(packetNo % PAGE_SIZE) + i] = USB_rxBuff[i + 8];
											}						
											if(u32temp == 0)
											{
												myFMC_Erase(USER_FLASH_START + firmwareFileOffSet - PAGE_SIZE);
												u32Pt = (uint32_t *)flashBuff;
												FMC_ProgramPage(USER_FLASH_START + firmwareFileOffSet - PAGE_SIZE,u32Pt);
												for(i = 0;i < PAGE_SIZE; i++)
													flashBuff[i] = 0xff;
											}
											else if(firmwareFileOffSet >= firmwareFileSize)
											{
												i = (firmwareFileOffSet / PAGE_SIZE)*PAGE_SIZE;
												myFMC_Erase(USER_FLASH_START + i);
												u32Pt = (uint32_t *)flashBuff;
												FMC_ProgramPage(USER_FLASH_START + i,u32Pt);
											}
										}
										packetNo = firmwareFileOffSet;
										if(firmwareFileOffSet >= firmwareFileSize)
										{
											timeOut = 1000000;
											firmwareFileOffSet = 0;
											packetNo = 0x5A5A5A5A;
										}
										u8pt = (uint8_t *)&packetNo;
										USB_txBuff[1] = 4; //length
										USB_txBuff[3] = 0x12; //opcode
										USB_txBuff[4] = u8pt[0];
										USB_txBuff[5] = u8pt[1];
										USB_txBuff[6] = u8pt[2];
										USB_txBuff[7] = u8pt[3];
									}
								break;
								case 0xA5:
									FLASH_Lock();
									execute_user_code();
								break;
							default:
								USB_txBuff[1] = 4; //length
								USB_txBuff[3] = 0x33; //opcode
								USB_txBuff[4] = 0xA5;
								USB_txBuff[5] = 0xA5;
								USB_txBuff[6] = 0xA5;
								USB_txBuff[7] = 0xA5;
								break;
						}
						USB_txBuff[0] = 0xCA;
						USB_txBuff[USB_txBuff[1] + 4] = CfgCalcCheckSum((uint8_t *)&USB_txBuff[4],USB_txBuff[1]);
						USB_WriteEP (0x81,USB_txBuff,64);
						}
					USB_rxBuff[0] = 0;
			 }
		}
	u32Pt = (uint32_t *)buff;
	SST25_Init();
	SST25_Read(FIRMWARE_INFO_ADDR + FIRMWARE_STATUS_OFFSET*4,buff,4);
	if(*u32Pt != 0x5A5A5A5A)	
	{
		execute_user_code();
	}
	tryNum = 10;
	exit = 0;
	while(!exit && tryNum--)
	{
		SST25_Read(FIRMWARE_INFO_ADDR + FIRMWARE_CRC_OFFSET*4,buff,4);
		fileCrc = 	*u32Pt;
		SST25_Read(FIRMWARE_INFO_ADDR + FIRMWARE_FILE_SIZE_OFFSET*4,buff,4);
		fileSize = 	*u32Pt;
		if(fileSize < FIRMWARE_MAX_SIZE)
		{
			flashCheckSum = 0;
			for(i = 0; i < fileSize;i += PAGE_SIZE)
			{
				SST25_Read(i + FIRMWARE_BASE_ADDR,buff, PAGE_SIZE);
				for(j = 0 ; j < PAGE_SIZE;j++)
				{
					if(i + j < fileSize)
						flashCheckSum += buff[j];
					else
						break;
				}
				myFMC_Erase(USER_FLASH_START + i);
				FMC_ProgramPage(USER_FLASH_START + i,u32Pt);
				SST25_Read(i + FIRMWARE_BASE_ADDR,buff, PAGE_SIZE);
				memAddr = USER_FLASH_START + i;
				if(memcmp(buff, (void*)memAddr , PAGE_SIZE) != NULL)
					break;
			}
			if(flashCheckSum == fileCrc) 
			{
					SST25_Read(BASE_FIRMWARE_INFO_ADDR + FIRMWARE_STATUS_OFFSET*4,buff,4);
					if(*u32Pt != 0x5A5A5A5A)	
					{
						tryNum = 10;
						while(tryNum--)
						{
							flashCheckSum = 0;
							for(i = 0; i < fileSize;i += 256)
							{
								SST25_Read(i + FIRMWARE_BASE_ADDR,buff, 256);
								SST25_Write(i + BASE_FIRMWARE_BASE_ADDR,buff, 256);
								SST25_Read(i + BASE_FIRMWARE_BASE_ADDR,flashBuff, 256);
								if(memcmp(buff,flashBuff,256) != NULL)
								{
										break;
								}
							}
							if(i >= fileSize)
								for(i = 0; i < fileSize;i += 256)
								{
										SST25_Read(i + BASE_FIRMWARE_BASE_ADDR,buff, 256);
										for(j = 0 ; j < 256;j++)
										{
											if(i + j < fileSize)
												flashCheckSum += buff[j];
											else
												break;
										}
								}
							if(flashCheckSum == fileCrc)
							{
								SST25_Read(FIRMWARE_INFO_ADDR,buff, 256);
								SST25_Write(BASE_FIRMWARE_INFO_ADDR,buff, 256);
								SST25_Read(BASE_FIRMWARE_INFO_ADDR,flashBuff, 256);
								if(memcmp(buff,flashBuff,256) == NULL)
								{
										break;
								}
							}
						}
					}
					exit = 1;
			}
		}
 }
	if(exit == 0 && tryNum == 0xff)
	{
			tryNum = 10;
			exit = 0;
			while(!exit && tryNum--)
			{
				SST25_Read(BASE_FIRMWARE_INFO_ADDR + FIRMWARE_CRC_OFFSET*4,buff,4);
				fileCrc = 	*u32Pt;
				SST25_Read(BASE_FIRMWARE_INFO_ADDR + FIRMWARE_FILE_SIZE_OFFSET*4,buff,4);
				fileSize = 	*u32Pt;
				if(fileSize < FIRMWARE_MAX_SIZE)
				{
					flashCheckSum = 0;
					for(i = 0; i < fileSize;i += PAGE_SIZE)
					{
						SST25_Read(i + BASE_FIRMWARE_BASE_ADDR,buff, PAGE_SIZE);
						for(j = 0 ; j < PAGE_SIZE;j++)
						{
								if(i + j < fileSize)
									flashCheckSum += buff[j];
								else
									break;
						}
						myFMC_Erase(USER_FLASH_START + i);
						FMC_ProgramPage(USER_FLASH_START + i,u32Pt);
						SST25_Read(i + BASE_FIRMWARE_BASE_ADDR,buff, PAGE_SIZE);
						memAddr = USER_FLASH_START + i;
						if(memcmp(buff, (void*)memAddr , PAGE_SIZE) != NULL)
							break;
					}
					if(flashCheckSum == fileCrc)
					{
							exit = 1;
					}
				}
		 }
	}	

	SST25_Erase(FIRMWARE_INFO_ADDR,block4k);
	execute_user_code();
	while(1)
	{
	}
}

