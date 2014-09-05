
//  mag8700.c -- Magnetometer support
//
//  Copyright (c) 2014 John Greb <github.com/hexameron>

// Based on example code from Freescale, released by Element 14.

#include "hal_i2c.h"
#include "common.h"
#include <stdio.h>

// Magnetic calibration needs to be done after board assembly
// Offsets can be programmed into chip, or added in code.
#define MAG_CAL_X (200)
#define MAG_CAL_Y (50)
#define MAG_CAL_Z (900)

#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_WHOAMI_VAL 0xC7

// I2C address depends on wiring: [1c,1d,1e,1f]
#define MAG_ADD		(0x1D << 1)
#define MAG_ID		(FXOS8700CQ_WHOAMI_VAL)

#define MAG_READY_REG   (0x32)
#define MAG_X_OUT	(0x33)
#define MAG_Y_OUT	(0x35)
#define MAG_Z_OUT	(0x37)
#define MAG_WHOAMI      (FXOS8700CQ_WHOAMI)
#define MAG_TEMP	(0x51)
#define MAG_CTRL1	(FXOS8700CQ_M_CTRL_REG1)
#define MAG_CTRL2	(FXOS8700CQ_M_CTRL_REG2)


// MAG_CTRL1 manages Acc/Mag on/off as well as oversample rate.
#define MAG_SETTING	(0x2f)
#define MAG_XYZ_READY	(1<<3)

char magReady;
void mag_init(void)
{
	hal_i2c_write(I2C0_B, MAG_ADD, MAG_CTRL1, 0x3);
        magReady = hal_i2c_read(I2C0_B, MAG_ADD, MAG_WHOAMI);
	if (MAG_ID != magReady ) return;

	// Setup autoresets and one-shot trigger in raw mode
	hal_i2c_write(I2C0_B, MAG_ADD, MAG_CTRL2, 0x90);
	hal_i2c_write(I2C0_B, MAG_ADD, MAG_CTRL1, MAG_SETTING);
}

// Forward is away from touchpad, towards USB
// Pitch is up at front
// Roll is righthand up
// tan(yaw angle) = (mz * sin(roll) – my * cos(roll)) /
//      (mx * cos(pitch) +  mz * cos(roll) * sin(pitch))

// Measure Mag Field, transform by attitude and calculate North
short mag_compass(short pitch, short roll)
{
	char dataready;
	short magX, magY, magZ;
        short angle1, angle2, result;
        short sin_pitch, sin_roll, cos_pitch, cos_roll;

        if (MAG_ID != magReady ) return 0;
	dataready = hal_i2c_read(I2C0_B, MAG_ADD, MAG_READY_REG);
	if (!(dataready & MAG_XYZ_READY)) return 0;

	magX = hal_i2c_read(I2C0_B, MAG_ADD, MAG_X_OUT);
	magX <<= 8;
	magX += hal_i2c_read(I2C0_B, MAG_ADD, MAG_X_OUT + 1);
	magX -= MAG_CAL_X;

        magY = hal_i2c_read(I2C0_B, MAG_ADD, MAG_Y_OUT);
        magY <<= 8;
        magY += hal_i2c_read(I2C0_B, MAG_ADD, MAG_Y_OUT + 1);
	magY -= MAG_CAL_Y;

        magZ = hal_i2c_read(I2C0_B, MAG_ADD, MAG_Z_OUT);
        magZ <<= 8;
        magZ += hal_i2c_read(I2C0_B, MAG_ADD, MAG_Z_OUT + 1);
	magZ -= MAG_CAL_Z;

	//iprintf("%d,%d,%d.\n\r",magX,magY,magZ);

	// using one-shot mode for minimum power: check oversample rate.
	hal_i2c_write(I2C0_B, MAG_ADD, MAG_CTRL1, MAG_SETTING);

	magX >>= 4; // 12 bits down to 8. Will overflow near large magnets
	magY >>= 4; // 7 bit sine table gives no headroom
	magZ >>= 4;

	sin_pitch = sine(pitch);
	cos_pitch = cosine(pitch);
	sin_roll = sine(roll);
	cos_roll = cosine(roll);

	angle1 = magZ * sin_roll - magY * cos_roll;
	angle2 = ((magZ * cos_roll) >> 7) * sin_pitch + magX * cos_pitch;

	result = 180 - findArctan(angle2, angle1, 0);
	return result;
}

short mag_temp()
{
	return hal_i2c_read(I2C0_B, MAG_ADD, MAG_TEMP);
}


