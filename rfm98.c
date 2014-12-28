/* (C) 2015 John Greb, MIT Lience */
#include "common.h"

// to write data set Address MSB High
#define REG_OPMODE	(0x01)
#define REG_RFM_TEMP	(0x3C)

#define xSLAVE (1<<0)
#define xCLOCK (1<<1)
#define xWRITE (1<<2)
#define xREAD  (1<<3)

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

uint8_t mode_on = 0;
// temp is not available in standby
/* returns 16 bits to help debug SPI */
short rfm98_temp()
{
	short temp;
	if (!mode_on) {
		temp = spi_rw(0x81, 0x0b); // transmit
		mode_on = 1;
	} else {
		temp = spi_rw(0x3c, 0x00); // read temp
	}
	return temp;
}
