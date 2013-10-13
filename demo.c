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

/* blink()
 * delays 12*80 = 960ms
 * with visible pattern on LED
 */
void blink(void)
{
	unsigned int pat = 0b0111000111000;
	while (pat) 
	{
		RGB_LED(pat&2 ? 10:0, pat&1 ? 40:0, pat&4? 10:0 );
		delay( 80 );
		pat >>= 1;
	}	
}

// Main program
int main(void)
{
    short ax, ay, az;
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

    blink(); 

    // Welcome banner
    iprintf("\r\n\r\n====== Freescale Freedom FRDM-KL25Z\r\n");
    iprintf("\r\nBuilt: %s %s\r\n", __DATE__, __TIME__);
    iprintf("Ident, Count, Accel mag,pitch,roll Touch a,z *Chksum\r\n");
    
    for(;;) {
	blink();

	ax = accel_x();
	ay = accel_y();
	az = accel_z();
	force = magnitude( ax, ay, az);
	pitch = findArcsin( ax, force );
	roll  = findAngle( az, ay );
	force >>= 2;
        iprintf("$$HEX,%d,%4d,%3d,%3d,",seq++, force, pitch, roll);
        iprintf("%d,%d*%x\r\n", touch_data(9), touch_data(10), checksum);
    }
}
