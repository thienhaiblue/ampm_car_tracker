#ifndef __MY_RF_SETTINGS_H__
#define __MY_RF_SETTINGS_H__
/* Chipcon */
/* Product = CC2500 */
/* Chip version = E */
/* Crystal accuracy = 10 ppm */
/* X-tal frequency = 26 MHz */
/* RF output power = 0 dBm */
/* RX filterbandwidth = 541.666667 kHz */
/* Phase = 1 */
/* Datarate = 249.938965 kbps */
/* Modulation = (7) MSK */
/* Manchester enable = (0) Manchester disabled */
/* RF Frequency = 2432.999908 MHz */
/* Channel spacing = 199.951172 kHz */
/* Channel number = 0 */
/* Optimization = Sensitivity */
/* Sync mode = (3) 30/32 sync word bits detected */
/* Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX */
/* CRC operation = (1) CRC calculation in TX and CRC check in RX enabled */
/* Forward Error Correction = (0) FEC disabled */
/* Length configuration = (1) Variable length packets, packet length configured by the first received byte after sync word. */
/* Packetlength = 255 */
/* Preamble count = (2)  4 bytes */
/* Append status = 1 */
/* Address check = (0) No address check */
/* FIFO autoflush = 0 */
/* Device address = 0 */
/* GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet */
/* GDO2 signal selection = (11) Serial Clock */


const HAL_RF_CONFIG myRfConfig = {
    0x09,   // FSCTRL1
    0x00,   // FSCTRL0
    0x5D,   // FREQ2
    0x93,   // FREQ1
    0xB1,   // FREQ0
    0x2D,   // MDMCFG4
    0x3B,   // MDMCFG3
    0x73,   // MDMCFG2
    0x22,   // MDMCFG1
    0xF8,   // MDMCFG0
    0x00,   // CHANNR
    0x01,   // DEVIATN
    0xB6,   // FREND1
    0x10,   // FREND0
    0x18,   // MCSM0
    0x1D,   // FOCCFG
    0x1C,   // BSCFG
    0xC7,   // AGCCTRL2
    0x00,   // AGCCTRL1
    0xB2,   // AGCCTRL0
    0xEA,   // FSCAL3
    0x0A,   // FSCAL2
    0x00,   // FSCAL1
    0x11,   // FSCAL0
    0x59,   // FSTEST
    0x88,   // TEST2
    0x31,   // TEST1
    0x0B,   // TEST0
    0x0B,   // IOCFG2
    0x06,   // IOCFG0
    0x04,   // PKTCTRL1
    0x05,   // PKTCTRL0
    0x00,   // ADDR
    22    // PKTLEN
};

const uint8_t myPaTable[] = {0xFE};
const uint8_t myPaTableLen = 1;

#endif
