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
    short t1, t2;
    short red, green, blue;
    short pitch, roll;
    unsigned short force;
    unsigned short checksum = 0xdead;
    unsigned short seq = 100;

    // Initialize all modules
    uart_init(115200);
    accel_init();
    touch_init((1 << 9) | (1 << 10));       // Channels 9 and 10
    setvbuf(stdin, NULL, _IONBF, 0);        // No buffering

    // Unused here, but necessary.
    heap_end = _sbrk(0);

    RGB_LED( 0, 40, 0 );
    delay( 100 );
    // Welcome banner
    iprintf("\r\n\r\n====== Freescale Freedom FRDM-KL25Z\r\n");
    iprintf("\r\nBuilt: %s %s\r\n", __DATE__, __TIME__);
    iprintf("Ident, Count, Accel mag,pitch,roll Touch l,r *Chksum\r\n");
    
    for(;;) {
	pat = 1 << 7;
	while (pat) {
		ax = accel_x();
		ay = accel_y();
		az = accel_z();
		force = magnitude( ax, ay, az );
		pitch = findArctan( ax, ay, az );
		roll  = findArctan( az, ay, 0 );
		force >>= 2;

		red = (force - 1000) >> 3;
		if (red < 0) red = -red;
		if (red > 80) red = 80;
		blue = pitch;
		if (blue < 0) blue = -blue;
		if (blue > 80) blue = 80;
		green = roll;
		if (green < 0) green = -green;
		if (green > 80) green = 80;

		t1 = (touch_data(9) + 4) >> 3;
		t2 = (touch_data(10) + 4) >> 3;
		if (t1 + t2 ) {
			blue = t1;
			green = t2;
		}	

		RGB_LED( red, green, blue );

		pat >>= 1;
		delay(120);
	}
        iprintf("$$HEX,%d,%4d,%3d,%3d,",seq++, force, pitch, roll);
        iprintf("%d,%d,*%x\r\n", t1, t2, checksum);
    }
}
