#include "common.h"
#if 0
/* C1 Bit Fields */
#define SPI_C1_LSBFE_MASK                        0x1u
#define SPI_C1_SSOE_MASK                         0x2u
#define SPI_C1_CPHA_MASK                         0x4u
#define SPI_C1_CPOL_MASK                         0x8u
#define SPI_C1_MSTR_MASK                         0x10u
#define SPI_C1_SPTIE_MASK                        0x20u
#define SPI_C1_SPE_MASK                          0x40u
#define SPI_C1_SPIE_MASK                         0x80u
/* C2 Bit Fields */
#define SPI_C2_SPC0_MASK                         0x1u
#define SPI_C2_SPISWAI_MASK                      0x2u
#define SPI_C2_RXDMAE_MASK                       0x4u
#define SPI_C2_BIDIROE_MASK                      0x8u
#define SPI_C2_MODFEN_MASK                       0x10u
#define SPI_C2_TXDMAE_MASK                       0x20u
/* Flags */
#define SPI_S_SPTEF_MASK                         0x20u
#define SPI_S_SPMF_MASK                          0x40u
#define SPI_S_SPRF_MASK                          0x80u
#endif

void spi_init(void) {
	// Enable clock network to SPI0
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM_SCGC4 |= SIM_SCGC4_SPI0_MASK;
	
	// configure output crossbar
	PORTD_PCR0 = PORT_PCR_MUX(2);  // PCS
//	PORTD_PCR1 = PORT_PCR_MUX(2);  // SCK - conflicts with blue LED
	PORTC_PCR5 = PORT_PCR_MUX(2);  // ALT SCK
	PORTD_PCR2 = PORT_PCR_MUX(2);  // 2=MOSI, 4=MISO
	PORTD_PCR3 = PORT_PCR_MUX(2);  // 2=MISO, 4=MOSI
	
	// configure gpio address select
	/* here */
	
	// Config registers, turn on SPI0 as master
	// Enable chip select
	// Use low baudrate for prototyping
	SPI0_C1 = 0x52; // (SPE + MSTR + SSOE)
	SPI0_C2 = 0x10; // (MODFEN/SSOE)
	SPI0_BR = 0x04; // Baudrate divider 2<<n => 32
}

uint8_t spi_status(void) {
	return SPI0_S;
}

// Clear To Send
void spiCTS()
{
	while ( !(spi_status() & SPI_S_SPTEF_MASK) )
		__asm("nop");
}

// Data Ready
void spiDR()
{
	while ( !(spi_status() & SPI_S_SPRF_MASK) )
		__asm("nop");
}

// Write out all characters in supplied buffer to register at address
void spi_write(uint8_t* p, uint8_t size, uint8_t addr) {
	uint8_t i;

	spiCTS();
	SPI0_D = addr;
	for (i = 0; i < size; ++i) {
		spiCTS();
		SPI0_D = p[i];
	}
	spiCTS();
}

// read and/or write one byte to/from address
uint8_t spi_single(uint8_t addr, uint8_t data)
{
	uint8_t tmp;

	spiCTS();       // check last transmit has ended
	SPI0_D = addr;  // send request

	spiCTS();	// double buffered => instant-ready
	tmp = SPI0_D;	// clear receive buffer
	SPI0_D = data;  // queue data

	spiDR();
	tmp = SPI0_D;	// discard first read

	spiDR();
	tmp = SPI0_D;	// get actual data
	return tmp;
}

// Read size number of characters into buffer p from register at address
void spi_read(uint8_t* p, uint8_t size, uint8_t addr)
{
	uint8_t i;

	spiCTS();	// check last transmit has ended
	SPI0_D = addr;	// send request

	spiCTS();	// double buffered - first write should be quick
	p[0] = SPI0_D;  // clear receive buffer
	SPI0_D = 0;     // request data

	spiDR();
	p[0] = SPI0_D;	// discard first data
	SPI0_D = 0;	// keep channel open

	for (i = 0; i < size; ++i)
	{
		spiDR();
		p[i] = SPI0_D;	// read actual data
		SPI0_D = 0;	// request more data
	}
}
