#include "common.h"

void spi_init(void) {
	// Enable clock network to SPI0
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;
	
	// configure output crossbar
	PORTD_PCR0 = PORT_PCR_MUX(2);  // PCS
	PORTD_PCR1 = PORT_PCR_MUX(2);  // SCK
	PORTD_PCR2 = PORT_PCR_MUX(2);  // MOSI
	PORTD_PCR3 = PORT_PCR_MUX(2);  // MISO
	
	// configure gpio address select
	/* here */
	
	// Config registers, turn on SPI0 as master
	// Enable chip select
	SPI0_C1 = 0x52;
	SPI0_C2 = 0x10;
	SPI0_BR = 0x00;
}

uint8_t spi_status(void) {
	return SPI0_S;
}

// Write out all characters in supplied buffer to register at address
void spi_write(uint8_t* p, uint8_t size, uint8_t addr) {
	uint8_t i;
	// set SPI line to output (BIDROE = 1)
	//SPI0->C2 |= 0x04;
	for (i = 0; i < size; ++i) {
		// poll until empty
		while ((spi_status() & 0x20) != 0x20);
		SPI0_D = p[i];
	}
}

// Read size number of characters into buffer p from register at address
void spi_read(uint8_t* p, uint8_t size, uint8_t addr) {
	uint8_t i;
	// set SPI line to input (BIDROE = 0)
	//SPI0->C2 &= 0xF7;
	for (i = 0; i < size; ++i) {
		// poll until full
		SPI0_D = 0x00;
		while ((spi_status() & 0x80) != 0x80);
		p[i] = SPI0_D;
	}
}
