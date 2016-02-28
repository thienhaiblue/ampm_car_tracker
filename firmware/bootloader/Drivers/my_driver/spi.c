
/**
* \file
*         spi driver
* \author
*         Nguyen Van Hai <hainv@ivx.vn>
*/
#include "spi.h"


void SPI_Init(uint8_t channel,uint32_t pclk1)
{
	if(channel == 2)
	{
		/* Clear BIDIMode, BIDIOE, RxONLY, SSM, SSI, LSBFirst, BR, MSTR, CPOL and CPHA bits */
		SPI2->CR1 &= SPI_CR1_CLEAR_Mask;
		SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSI | SPI_CR1_SSM;
		/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
		SPI2->I2SCFGR &= SPI_Mode_Select;		
	/*---------------------------- SPIx CRCPOLY Configuration --------------------*/
		/* Write to SPIx CRCPOLY */
		SPI2->CRCPR = 7;
		/* Enable the selected SPI peripheral */
		SPI2->CR1 |= SPI_CR1_SPE;
	}
	else if(channel == 1)
	{
			/* Clear BIDIMode, BIDIOE, RxONLY, SSM, SSI, LSBFirst, BR, MSTR, CPOL and CPHA bits */
		SPI1->CR1 &= SPI_CR1_CLEAR_Mask;
		SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSI | SPI_CR1_SSM;
		/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
		SPI1->I2SCFGR &= SPI_Mode_Select;		
	/*---------------------------- SPIx CRCPOLY Configuration --------------------*/
		/* Write to SPIx CRCPOLY */
		SPI1->CRCPR = 7;
		/* Enable the selected SPI peripheral */
		SPI1->CR1 |= SPI_CR1_SPE;
	}
}

uint8_t halSpiWriteByte(SPI_TypeDef* SPIx,uint8_t data)
{
	uint8_t rc;
	uint32_t timeOut = SPI_TIMEOUT;
	while(!(SPIx->SR & SPI_SR_TXE))
	{
			if(timeOut-- == 0) return 0xff;
	}
	/* Write in the DR register the data to be sent */
	SPIx->DR = data;
	while(!(SPIx->SR & SPI_SR_RXNE))
	{
			if(timeOut-- == 0) return 0xff;
	}
	rc = SPIx->DR;
	return rc;
}


