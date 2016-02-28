/**
* \file
*         rf control
* \author
*         Nguyen Van Hai <hainv@ivx.vn>
*/
#include "rf_control.h"
#include "my_rf_settings.h"
#include "rtc.h"


#define LED_ON  	LED_PIN_SET
#define LED_OFF		LED_PIN_CLR


//----------------------------------------------------------------------------------
//  Constants used in this file
//----------------------------------------------------------------------------------
#define RADIO_MODE_TX        1
#define RADIO_MODE_RX        2

#define RX_OK                0
#define RX_LENGTH_VIOLATION  1
#define RX_CRC_MISMATCH      2
#define RX_FIFO_OVERFLOW     3
#define RX_START_CODE_FAIL   4
#define RX_PENDING           5

#define MY_MASTER_ADDR       0x10000000
#define MY_SLAVE1_ADDR        0x20000000
#define MY_SLAVE2_ADDR        0x30000000

#define PACKET_START_CODE   0xA5

#define CMD_KEY_OK                  0
#define CMD_GET_RANDOM_NUMBER       1

#define PACKET_TIME_OUT   10000
#define PACKET_TRY_AGAIN  3

#define PACKET_RANDUM_NUMBER_START_POSITION	4
#define PACKET_RANDUM_NUMBER_END_POSITION	8

uint32_t timeBuff[3] = {0};
uint32_t timeBuffCnt = 0;

enum{
	RF_WAIT_GET_RANDOM_NUMBER,
	RF_SENT_RANDOM_NUMBER,
	RF_WATI_RESPONSE_OK,
	RF_SENT_REPLY_OK
}RF_DataState = RF_WAIT_GET_RANDOM_NUMBER;


/* TypeDefs */
typedef union
{
	uint8_t  bytes[4];
	uint32_t lword;		
}T32_8;

typedef struct { 
  uint8_t data_length;
	uint8_t start_code;
	uint8_t dummy[2];
	T32_8 random_number;
  T32_8 source_addr;
  T32_8 destination_addr; 
	T32_8 data_command;
  T32_8 data;
  uint32_t crc;
} MY_STRUCT_TYPE;

typedef struct { 
  uint8_t code;
  uint8_t press;
} REMOTE_TYPE;

/* TypeDefs */
typedef union
{
	uint8_t  buff[sizeof(MY_STRUCT_TYPE)];
	MY_STRUCT_TYPE frame;		
}MY_PACKET_TYPE;

MY_PACKET_TYPE packet;

REMOTE_TYPE button;

uint32_t rfDriverStatus = RF_DEINIT;

//----------------------------------------------------------------------------------
//  Variables used in this file
//----------------------------------------------------------------------------------
static uint8_t txSendPacket(uint8_t* data, uint8_t length);
static uint8_t rxRecvPacket(MY_PACKET_TYPE *packet);
static uint32_t CheckSumCalc(uint8_t *data);
static void preparePacket(void);

uint32_t temp = 0;


uint8_t RF_ProcessInit(void)
{
		uint8_t  id;
    uint8_t  ver;
		SPI_Init(2,24000000);
    halRfResetChip();
    id  = halRfGetChipId();
    ver = halRfGetChipVer();
		//RfPrintf("\r\nid=%d,ver=%d\r\n",id,ver);
    // Put radio to sleep
    halRfStrobe(CC2500_SPWD);
    // Setup chip with register settings from SmartRF Studio
    halRfConfig(&myRfConfig, myPaTable, myPaTableLen);
    // Additional chip configuration for this example
    halRfWriteReg(CC2500_MCSM1,    0x00);   // No CCA, IDLE after TX and RX
    halRfWriteReg(CC2500_PKTCTRL0, 0x45);   // Enable data whitening and CRC
    halRfWriteReg(CC2500_PKTLEN, sizeof(MY_STRUCT_TYPE)*2);   // Max payload data length
		halRfWriteReg(CC2500_MCSM1, 0x3c);   // Stay Rx 
		//halRfStrobe(CC2500_SPWD);
		ForceToWORMode();
		rfDriverStatus = RF_INIT;
	return 0;
}

void ForceToRxMode(void)
{
	halRfStrobe(CC2500_SIDLE);
	halRfStrobe(CC2500_SFRX);
	halRfStrobe(CC2500_SRX);
}

void WOR_SetDefault(void)
{
	halRfWriteReg(CC2500_WOREVT1,0x87); //High Byte Event0 Timeout of 34666
	halRfWriteReg(CC2500_WOREVT0,0x6A); //Low Byte Event0 Timeout
	halRfWriteReg(CC2500_MCSM2,0x07); 
	halRfWriteReg(CC2500_WORCTRL,0xf8);
}
void ForceToWORMode(void)
{
	halRfWriteReg(CC2500_WOREVT1,0x87); //High Byte Event0 Timeout of 34666
	halRfWriteReg(CC2500_WOREVT0,0x6B); //Low Byte Event0 Timeout
	 /* WORCTRL.EVENT1 = 7 => T_EVENT1 = ~1.5 msec */
	halRfWriteReg(CC2500_MCSM2,0x00); //RX_TIME_RSSI  = 1 and RX_TIME = 6.
	/* WORCTRL.WOR_RES = 0 (EVENT0 = 34667, RX_TIME = 6)  =>  RX timeout  = 34667 * 0.0563 = ~1.96 msec */
	halRfWriteReg(CC2500_WORCTRL, (7 << 4) | 0);
	halRfStrobe(CC2500_SWOR);
}

void RF_ProcessDeInit(void)
{
	rfDriverStatus = RF_DEINIT;
}
//----------------------------------------------------------------------------------
//  void preparePacket(uint8_t id, uint8_t* data, uint8_t* length)
//
//  DESCRIPTION:
//    Set up a dummy packet, where the first byte contains the length of the payload
//    and the first byte of the payload contains a packet id.
//----------------------------------------------------------------------------------

static void preparePacket(void)
{
		packet.frame.start_code = PACKET_START_CODE; 
    packet.frame.data_length = sizeof(MY_STRUCT_TYPE)-1; //khong tinh 4 bytes dau
		packet.frame.random_number.lword = TICK_Get();
		packet.frame.data_command.lword = CMD_GET_RANDOM_NUMBER;
    packet.frame.source_addr.lword =  MY_MASTER_ADDR;
    packet.frame.destination_addr.lword  = MY_SLAVE1_ADDR; 
    packet.frame.data.lword = 0;    
}

static uint32_t CheckSumCalc(uint8_t *data)
{   uint32_t i,crc = 0;
	//cancel data length and crc
    for(i = 4 ; i < (sizeof(MY_STRUCT_TYPE)-4) ; i++ )
    {
      crc += data[i];
    }
    return crc;
}
/**/
static uint16_t Encode(uint8_t *data)
{ 
  uint8_t i,j;
  for(i = PACKET_RANDUM_NUMBER_END_POSITION,j = PACKET_RANDUM_NUMBER_START_POSITION; i < (sizeof(MY_STRUCT_TYPE)-4);i++)
  {
    data[i] += data[j];
    j++;
    if(j >= PACKET_RANDUM_NUMBER_END_POSITION) j = PACKET_RANDUM_NUMBER_START_POSITION;
  }
  return 0;
}
/**/
static uint16_t Decode(uint8_t *data)
{ 
  uint8_t i,j = 0;
	for(i = PACKET_RANDUM_NUMBER_END_POSITION,j = PACKET_RANDUM_NUMBER_START_POSITION; i < (sizeof(MY_STRUCT_TYPE)-4);i++)
  {
    data[i] -= data[j];
    j++;
    if(j >= PACKET_RANDUM_NUMBER_END_POSITION) j = PACKET_RANDUM_NUMBER_START_POSITION;
  }
  return 0;
}


static uint8_t txSendPacket(uint8_t* data, uint8_t length)
{
    Encode(data);
    // Write data to FIFO
    if(halRfWriteFifo(data, length) != 0x0f)
		{
				
			halRfStrobe(CC2500_SIDLE);
			halRfStrobe(CC2500_SFTX);
			if(halRfWriteFifo(data, length) != 0x0f)
				return 1;
		}		
    // Set radio in transmit mode
    halRfStrobe(CC2500_STX);
    return 0;
}

static uint8_t rxRecvPacket(MY_PACKET_TYPE *packet)
{
	uint8_t packet_status[2];
	uint8_t status;   
	status = RX_OK;   
	// Read first element of packet from the RX FIFO
	status = halRfReadFifo(packet->buff, 1);
	if ((status & CC2500_STATUS_STATE_BM) == CC2500_STATE_RX_OVERFLOW)
	{
			status = RX_FIFO_OVERFLOW;
	}
	else if (packet->frame.data_length != (sizeof(MY_STRUCT_TYPE) - 1))
	{
			status = RX_LENGTH_VIOLATION;
	}
	else
	{
			// Get payload
			halRfReadFifo((uint8_t *)&packet->frame.start_code, packet->frame.data_length);	
			// Get the packet status bytes [RSSI, LQI]
			halRfReadFifo(packet_status, 2);		
			// Check CRC
			if ((packet_status[1] & CC2500_LQI_CRC_OK_BM) != CC2500_LQI_CRC_OK_BM)
			{
					status = RX_CRC_MISMATCH;
			}
			else if(packet->frame.start_code == PACKET_START_CODE)
			{   
					Decode(packet->buff);
					if(packet->frame.crc != CheckSumCalc(packet->buff))
						status = RX_CRC_MISMATCH;
					else
					{
						status = RX_OK;
					}
			}
			 else
			{
					status = RX_START_CODE_FAIL;
			}
	}       
	WOR_SetDefault();
	ForceToRxMode();
// 	halRfStrobe(CC2500_SFRX);
// 	halRfStrobe(CC2500_SWOR);
	return(status);
}
uint32_t timeOld = 0,SOS_pressCnt = 0;
void RF_Process (void)
{uint32_t u32temp;
	LED_ON;
	sysEventFlag &= ~RF_RECIEVED_DATA_FLAG;
	switch(RF_DataState)
	{
		case RF_WAIT_GET_RANDOM_NUMBER:
			if (rxRecvPacket(&packet) == 0)
				if(packet.frame.destination_addr.lword == MY_MASTER_ADDR) 
					if(packet.frame.source_addr.lword == MY_SLAVE1_ADDR)
						if(packet.frame.data_command.lword == CMD_GET_RANDOM_NUMBER)
						{
							preparePacket();
							packet.frame.destination_addr.lword  = MY_SLAVE1_ADDR;
							packet.frame.data_command.lword = CMD_GET_RANDOM_NUMBER;
							packet.frame.crc = CheckSumCalc(packet.buff);
						// Send packet
							if(txSendPacket(packet.buff, sizeof(MY_STRUCT_TYPE)))
							{
								RF_DataState = RF_WAIT_GET_RANDOM_NUMBER;
							}
							else
							{
								RF_DataState = RF_SENT_RANDOM_NUMBER;
							}
						}
		break;
		case RF_SENT_RANDOM_NUMBER:					
				RF_DataState = RF_WATI_RESPONSE_OK;
				ForceToRxMode();
		break;
		case RF_WATI_RESPONSE_OK:
			if (rxRecvPacket(&packet) == 0)
				if(packet.frame.destination_addr.lword == MY_MASTER_ADDR) 
					if(packet.frame.source_addr.lword == MY_SLAVE1_ADDR)
						if(packet.frame.data_command.lword == CMD_KEY_OK)
						{
							switch(packet.frame.data.bytes[0])
							{
										case KEY_LOCK:
												ENGINE_TURN_OFF;
												sysEventFlag |= SECURITY_ON_FLAG;
												BuzzerSetStatus(100,100,BUZZER_TURN_ON,3);
										break;
// 										case KEY_UNLOCK:
// 												ENGINE_TURN_ON;
// 												sysEventFlag &= ~SECURITY_ON_FLAG;
// 												sysEventFlag &= ~THEFT_ALARM_FLAG;
// 												accelerometerFlagCnt = 0;
//  												BuzzerSetStatus(100,100,BUZZER_TURN_ON,2);
// 										break;
										case KEY_SOS:
												u32temp = RTC_GetCounter();
												if(u32temp > timeOld + sysCfg.SOSKeyPressPeriod)
												{
													SOS_pressCnt = 1;
													timeOld = u32temp;
												}
												else
												{
													SOS_pressCnt++;
												}
												if(SOS_pressCnt >= sysCfg.SOSKeyPressTimes)
													sysEventFlag |= SOS_FLAG;
												BuzzerSetStatus(100,100,BUZZER_TURN_ON,1);
											break;
										case KEY_HORN_ON:
												sysEventFlag |= HORN_ON_FLAG;
												BuzzerSetStatus(5000,10,BUZZER_TURN_ON,255);
										break;
										case KEY_HORN_OFF:
												sysEventFlag &= ~POWER_FAILS_FLAG;
												sysEventFlag &= ~SECURITY_ON_FLAG;
												sysEventFlag &= ~THEFT_ALARM_FLAG;
												accelerometerFlagCnt = 0;
												sysEventFlag &= ~HORN_ON_FLAG;
 												BuzzerSetStatus(100,100,BUZZER_TURN_ON,1);
										break;
										default:
										break;
							}
							preparePacket();
							packet.frame.destination_addr.lword  = MY_SLAVE1_ADDR;
							packet.frame.data_command.lword = CMD_KEY_OK;
							packet.frame.crc = CheckSumCalc(packet.buff);
						// Send packet
							txSendPacket(packet.buff, sizeof(MY_STRUCT_TYPE));
							RF_DataState = RF_SENT_REPLY_OK;	
						}
						else if(packet.frame.data_command.lword == CMD_GET_RANDOM_NUMBER)
						{	RF_DataState = RF_WAIT_GET_RANDOM_NUMBER;
								preparePacket();
								packet.frame.destination_addr.lword  = MY_SLAVE1_ADDR;
								packet.frame.data_command.lword = CMD_GET_RANDOM_NUMBER;
								packet.frame.crc = CheckSumCalc(packet.buff);
							// Send packet
								if(txSendPacket(packet.buff, sizeof(MY_STRUCT_TYPE)))
								{
									RF_DataState = RF_WAIT_GET_RANDOM_NUMBER;
								}
								else
								{
									RF_DataState = RF_SENT_RANDOM_NUMBER;
								}
						}
		break;
		case RF_SENT_REPLY_OK:
		default:
			RF_DataState = RF_WAIT_GET_RANDOM_NUMBER;
			ForceToWORMode();
			//sysEventFlag |= RF_RECIEVED_DATA_FLAG;
			temp++;
		break;
	}
	LED_OFF;     
}


void RF_GDO0_PIN_ISR(void)
{
	if(rfDriverStatus == RF_INIT)
		RF_Process();
}

