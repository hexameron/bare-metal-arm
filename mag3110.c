
//  mag3110.c -- Magnetometer support
//
//  Copyright (c) 2013 John Greb <github.com/hexameron>

// Based on example code from Freescale, released by Element 14.

#include "hal_i2c.h"
#include "common.h"

#define MAG_ADD		(0x0E << 1)
#define MAG_ID		(0xC4)

#define MAG_READY_REG   (0x00)
#define MAG_X_OUT	(0x01)
#define MAG_Y_OUT	(0x03)
#define MAG_Z_OUT	(0x05)
#define MAG_WHOAMI      (0x07)
#define MAG_TEMP	(0x00)
#define MAG_CTRL1	(0x10)
#define MAG_CTRL2	(0x11)

#define MAG_XYZ_READY	(1<<3)

char magReady;
void mag_init(void)
{
	hal_i2c_write(I2C1_B, MAG_ADD, MAG_CTRL1, 0);
        magReady = hal_i2c_read(I2C1_B, MAG_ADD, MAG_WHOAMI);
	if (MAG_ID != magReady ) return;

	// Setup autoresets and one-shot trigger
	hal_i2c_write(I2C1_B, MAG_ADD, MAG_CTRL2, 0x80);
	hal_i2c_write(I2C1_B, MAG_ADD, MAG_CTRL1, 0x1A);
}

short mag_compass(void)
{
	char dataready;
	short magX, magY, magZ;

        if (MAG_ID != magReady ) return 0;
	dataready = MAG_XYZ_READY & hal_i2c_read(I2C1_B, MAG_ADD, MAG_READY_REG);
	if (!dataready) return 0; 

	magX = hal_i2c_read(I2C1_B, MAG_ADD, MAG_X_OUT);
	magX <<= 8;
	magX += hal_i2c_read(I2C1_B, MAG_ADD, MAG_X_OUT + 1);

        magY = hal_i2c_read(I2C1_B, MAG_ADD, MAG_Y_OUT);
        magY <<= 8;
        magY += hal_i2c_read(I2C1_B, MAG_ADD, MAG_Y_OUT + 1);

        magZ = hal_i2c_read(I2C1_B, MAG_ADD, MAG_Z_OUT);
        magZ <<= 8;
        magZ += hal_i2c_read(I2C1_B, MAG_ADD, MAG_Z_OUT + 1);

	// using one-shot mode for minimum power, 128 oversample
	hal_i2c_write(I2C1_B, MAG_ADD, MAG_CTRL1, 0x1A);


	//TODO: Calculate magnetic compass heading
	return magnitude(magX, magY, magZ);
}

