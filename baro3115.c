
//  baro3115.c -- Barometer support
//
//  Copyright (c) 2013 John Greb <github.com/hexameron>

// Based on example code from Freescale, released by Element 14.

#include "hal_i2c.h"
#include "common.h"

#define BARO_ADD	(0x60 << 1)
#define BARO_ID         (0xc4)

#define BARO_A_OUT	(1)
#define BARO_T_OUT	(4)

#define BARO_CTRL1	(0x26)
#define BARO_STANDBY	(0xFE)
#define BARO_ACTIVE	(0x01)
#define PT_DATA_CFG_REG (0x13)
#define PP_OD12_MASK    (0x11)

// Altitude mode, with 16x oversample
#define ALT_MASK	(1<<7)
#define OVERSAMP_MASK	(4<<3)


// Wake Barometer from Standby
void turnOnBaro(void)
{
	char reg;
        reg = hal_i2c_read(I2C1_B, BARO_ADD, BARO_CTRL1);
	hal_i2c_write(I2C1_B, BARO_ADD, BARO_CTRL1, reg | BARO_ACTIVE);
}

// Put Barometer into standby
char turnOffBaro(void)
{
	char reg;
	reg = hal_i2c_read(I2C1_B, BARO_ADD, BARO_CTRL1);
	hal_i2c_write(I2C1_B, BARO_ADD, BARO_CTRL1, reg & BARO_STANDBY);
	return reg;
}

short baro_alt()
{
	short alt;
	char reg = BARO_A_OUT;

	// only reports alt to 32km, with 4 bits of fraction
	alt = hal_i2c_read(I2C1_B, BARO_ADD, reg++);
	alt <<= 8;
	alt += hal_i2c_read(I2C1_B, BARO_ADD, reg++);
	// alt <<= 8;
        // alt += hal_i2c_read(I2C1_B, BARO_ADD, reg++)
	// alt >>= 4;

	return alt;
}

short baro_temp()
{
	short temp;
	char reg = BARO_T_OUT;

	temp = hal_i2c_read(I2C1_B, BARO_ADD, reg++);
	// temp <<=8;
	// temp += hal_i2c_read(I2C1_B, BARO_ADD, reg++);
	// temp >> 4;

	return temp;
}

void baro_init(void)
{
	char reg = BARO_CTRL1;

	// Set altitude mode. For barometer mode remove ALT_MASK
	hal_i2c_write(I2C1_B, BARO_ADD, reg++, (ALT_MASK | OVERSAMP_MASK )); 
	hal_i2c_write(I2C1_B, BARO_ADD, reg++, 0);
	hal_i2c_write(I2C1_B, BARO_ADD, reg++, PP_OD12_MASK);
	hal_i2c_write(I2C1_B, BARO_ADD, reg++, 0);
	hal_i2c_write(I2C1_B, BARO_ADD, reg++, 0);
	hal_i2c_write(I2C1_B, BARO_ADD, PT_DATA_CFG_REG, 0x07);

	turnOnBaro();	
}

