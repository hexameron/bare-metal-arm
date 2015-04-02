/* (C) 2015 John Greb, MIT Lience */
#include "common.h"

// to write data set Address MSB High
#define REG_FIFO 0
#define REG_OPMODE 0x1
#define REG_BITRATE_MSB 0x2
#define REG_BITRATE_LSB 0x3
#define REG_FDEV_MSB 0x4
#define REG_FDEV_LSB 0x5
#define REG_FRF_MSB 0x6
#define REG_FRF_MID 0x7
#define REG_FRF_LSB 0x8
#define REG_PA_CONFIG 0x9
#define REG_PA_RAMP 0xA
#define REG_OCP 0xB
#define REG_LNA 0xC
#define REG_RX_CONFIG 0xD
#define REG_RSSI_CONFIG 0xE
#define REG_RSSI_COLLISION 0xF
#define REG_RSSI_THRESH 0x10
#define REG_RSSI_VALUE_FSK 0x11
#define REG_RX_BW 0x12
#define REG_AFC_BW 0x13
#define REG_OOK_PEAK 0x14
#define REG_OOK_FIX 0x15
#define REG_AVG 0x16
#define RES17 0x17
#define RES18 0x18
#define RES19 0x19
#define REG_AFC_FEI 0x1A
#define REG_AFC_MSB 0x1B
#define REG_AFC_LSB_FSK 0x1C
#define REG_FEI_MSB_FSK 0x1D
#define REG_FEI_LSB_FSK 0x1E
#define REG_PREAMBLE_DETECT 0x1F
#define REG_RX_TIMEOUT1 0x20
#define REG_RX_TIMEOUT2 0x21
#define REG_RX_TIMEOUT3 0x22
#define REG_RX_DELAY 0x23
#define REG_OSC 0x24
#define REG_PREAMBLE_MSB_FSK 0x25
#define REG_PREAMBLE_LSB_FSK 0x26
#define REG_SYNC_CONFIG 0x27
#define REG_SYNC_VALUE1 0x28
#define REG_SYNC_VALUE2 0x29
#define REG_SYNC_VALUE3 0x2A
#define REG_SYNC_VALUE4 0x2B
#define REG_SYNC_VALUE5 0x2C
#define REG_SYNC_VALUE6 0x2D
#define REG_SYNC_VALUE7 0x2E
#define REG_SYNC_VALUE8 0x2F
#define REG_PACKET_CONFIG1 0x30
#define REG_PACKET_CONFIG2 0x31
#define REG_PAYLOAD_LENGTH_FSK 0x32
#define REG_NODE_ADRS 0x33
#define REG_BROADCAST_ADRS 0x34
#define REG_FIFO_THRESH 0x35
#define REG_SEQ_CONFIG1 0x36
#define REG_SEQ_CONFIG2 0x37
#define REG_TIMER_RESOL 0x38
#define REG_TIMER1_COEF 0x39
#define REG_TIMER2_COEF 0x3A
#define REG_IMAGE_CAL 0x3B
#define REG_TEMP 0x3C
#define REG_LOW_BAT 0x3D
#define REG_IRQ_FLAGS1 0x3E
#define REG_IRQ_FLAGS2 0x3F
#define REG_DIO_MAPPING1 0x40
#define REG_DIO_MAPPING2 0x41
#define REG_VERSION 0x42
#define REG_PLL_HOP 0x44
#define REG_TCXO 0x4B
#define REG_PA_DAC 0x4D
#define REG_FORMER_TEMP 0x5B
#define REG_BITRATE_FRAC 0x5D
#define REG_AGC_REF 0x61
#define REG_AGC_THRESH1 0x62
#define REG_AGC_THRESH2 0x63
#define REG_AGC_THRESH3 0x64
#define REG_PLL 0x70

// SPI
#define xSLAVE (1<<0)
#define xCLOCK (1<<1)
#define xWRITE (1<<2)
#define xREAD  (1<<3)

void rfm98_config(void); // forward declaration
void rfm98_init(void)
{
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	// bitbang spio pins
	GPIOD_PSOR  = 7;	//set 0-2 high
	GPIOD_PDDR |= 7;	//set 0-2 as output
	GPIOD_PDDR &=~8;	//set  3  as input
	PORTD_PCR0 = PORT_PCR_MUX(1);  // 2=PCS
	PORTD_PCR1 = PORT_PCR_MUX(1);  // 2=SCK - conflicts with blue LED
	PORTD_PCR2 = PORT_PCR_MUX(1);  // 2=MOSI, 4=MISO
	PORTD_PCR3 = PORT_PCR_MUX(1);  // 2=MISO, 4=MOSI

	GPIOD_PCOR = xCLOCK; // clock low

	rfm98_config();	
}

void pause(void) 
{
	short i;
	for (i = 0; i < 50; i++ )
		__asm("nop");
}
short spi_rw(uint8_t address, uint8_t data)
{
	short i, j, result = 0;
	uint16_t send = ((uint16_t)address << 8) + data;

	GPIOD_PCOR = xCLOCK; // clock low
	pause();
	GPIOD_PCOR = xSLAVE; // slave select
	
	for (i = 15; i >= 0; i--) {  
		pause();
		j = send >> i;
		if (j & 1)
			GPIOD_PSOR = xWRITE; // MOSI
		else
			GPIOD_PCOR = xWRITE; // !MOSI
		pause();
		GPIOD_PSOR = xCLOCK; // clock high
		j = (GPIOD_PDIR >> 3 ) & 1;
		result |= j << i;
		pause();
		GPIOD_PCOR = xCLOCK; // clock low
	}
	pause();
	GPIOD_PSOR = xSLAVE; // slave unselect
	pause();
	GPIOD_PSOR = xCLOCK; // clock high
	return result;
}

short rfm_temp = 0;
// temp is not available in standby
short rfm98_temp(void)
{
	// rfm_temp = spi_rw(0x3c, 0x00);
	return rfm_temp;
}

void rfm98_config(void)
{
	spi_rw(0x81, 1); // standby
	delay(250);

	//bitrate = 600, divided to 200
	spi_rw(0x80|REG_BITRATE_LSB,0x56);
	spi_rw(0x80|REG_BITRATE_MSB,0xD0);
	spi_rw(0x80|REG_FDEV_LSB, 7); // deviation / 62 Hz , min 600Hz
	spi_rw(0x80|REG_FDEV_MSB, 0);
	spi_rw(0x80|REG_PREAMBLE_LSB_FSK, 8);
	// spi_rw(0x80|REG_PREAMBLE_MSB_FSK, 0;

	//spi_rw(0x80|REG_SYNC_CONFIG, 0x13); // default 32 bits
	spi_rw(0x80|REG_SYNC_VALUE1, 0x03); // 2 start bits
	spi_rw(0x80|REG_SYNC_VALUE2, 0xff); //
	spi_rw(0x80|REG_SYNC_VALUE3, 0xff); // 8 stop bits
	spi_rw(0x80|REG_SYNC_VALUE4, 0xff); //

	//spi_rw(0x80|REG_PACKET_CONFIG1, 80);
	//spi_rw(0x80|REG_PACKET_CONFIG2, 40);

	// 434.400 MHz
	spi_rw(0x80|REG_FRF_MSB, 0x6c);
	spi_rw(0x80|REG_FRF_MID, 0x99);
	spi_rw(0x80|REG_FRF_LSB, 0x99);

	spi_rw(0x80|REG_PA_RAMP, 0x23); // 500us shaping
	spi_rw(0x80|REG_PA_CONFIG, 0x82); // low power
}

// 7n3 * 3 bits = 33 bit
uint32_t char2rtty(char c)
{
	short i, b;
	uint32_t group = 0xf8; // 2 extra stop bits + start bit
	c |= 0x80; // true stop bit
	for (i = 0; i < 8; i++) {
		group <<= 3;
		b = c & 1;			// LSB one bit in
		group |= b | (b<<1) | (b<<2);	// MSB 3 bits out
		c >>= 1;
	}
	return group;
}

void rfm98_transmit(char* message)
{
	int i;
	uint32_t group;

	spi_rw(0x80|REG_IRQ_FLAGS1, 0xff);
	spi_rw(0x80|REG_IRQ_FLAGS2, 0xff);

	spi_rw(0x80, 0x03); // start bit
	spi_rw(0x80, 0xff);
	spi_rw(0x80, 0xff); // stop bits
	spi_rw(0x80, 0xff);
	spi_rw(0x81, 0x43); // start transmitting

	for (i = 0; i < 64; i++) {
		if (0 == message[i]) break;

		// fifo threshold is 16 bytes, 128 bits, 200ms at 600 baud
		while ( spi_rw(REG_IRQ_FLAGS2, 0) & (1<<5) )
			delay(50);	// buffer full enough

		group = char2rtty(message[i]);
		spi_rw(0x80, (group >>24) & 0xff);
		spi_rw(0x80, (group >>16) & 0xff);
		spi_rw(0x80, (group >> 8) & 0xff);
		spi_rw(0x80, (group >> 0) & 0xff);
	}
	rfm_temp = spi_rw(0x3c, 0x00);

	while (!( spi_rw(REG_IRQ_FLAGS2, 0) & (1<<6) ))
		delay(50);	// buffer still not empty
	delay(50);
	spi_rw(0x81, 0x01); // stop transmitting
}
