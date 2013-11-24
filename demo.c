//
// demo.c -- Simple demonstration program
//
//  Copyright (c) 2012-2013 Andrew Payne <andy@payne.org>
//  Copyright (c) 2013-2014 John Greb <github.com/hexameron>
//

#include <stdio.h>
#include "freedom.h"
#include "common.h"

extern char *_sbrk(int len);
static char *heap_end;

// Main program
int main(void)
{
    unsigned int pat;
    short ax, ay, az;
    short lpitch = 0;
    short lroll = 0;
    short t1, t2, temp;
    short red, green, blue;
    short pitch, roll;
    short compass;
    unsigned short force, pressure;
    unsigned short checksum = 0xdead;
    unsigned short seq = 100;

    // Initialize all modules
    uart_init(115200);
    hal_i2c_init(I2C1_BASE_PTR);	// Setup I2C for EVK
    accel_init();
    baro_init();
    mag_init();
    touch_init((1 << 9) | (1 << 10));       // Channels 9 and 10
    setvbuf(stdin, NULL, _IONBF, 0);        // No buffering

    // Unused here, but necessary.
    heap_end = _sbrk(0);

    RGB_LED( 0, 40, 0 );
    delay( 100 );
    // Welcome banner
    iprintf("\r\n\r\n====== Freescale Freedom FRDM-KL25Z\r\n");
    iprintf("\r\nBuilt: %s %s\r\n", __DATE__, __TIME__);
    iprintf("Ident, Count, force,pitch,roll, mag field, pressure,temp *Chksum\r\n");

    ax = ay = az = 0;
    for(;;) {
	pat = 1 << 7;
	while (pat) {
		delay(120 );
		accel_read();
		ax = accel_x();
		ay = accel_y();
		az = accel_z();
		force = magnitude( ax, ay, az );
		pitch = findArctan( ax, ay, az );
		roll  = findArctan( az, ay, 0 );
		force += (force >> 1) + (force >> 4);
		force >>= 6; // force as percentage of 1G

		red = (force - 100);
		if (red < 0) red = -red;
		blue = pitch;
		if (blue < 0) blue = -blue;
		green = roll >> 1;
		if (green < 0) green = -green;

		t1 = (touch_data(9) + 4) >> 3;
		t2 = (touch_data(10) + 4) >> 3;
		if (t1 + t2 ) {
			blue = t1;
			green = t2;
		}	
		if (green > 80) green = 80;
		if (blue > 80) blue = 80;
		if (red > 80) red = 80;

		RGB_LED( red, green, blue );

		pat >>= 1;
	}

	pressure = get_pressure();
	temp = baro_temp();

	// Magnetometer is in one-shot mode so we need to use the accelerometer
	// readings from the previous loop.
	compass = mag_compass(lpitch, lroll);
	lpitch = pitch;
	lroll = roll;

	iprintf("$$HEX,%d,%3d,%3d,%3d,",seq++, force, pitch, roll);
	iprintf("%3d,%d0,%d,*%x\r\n", compass, pressure, temp, checksum);
    }
}
