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

/* blink()
 * delays 16*60 = 960ms
 * with visible pattern on LED
 */
void blink(void)
{
	unsigned int pat = 0b1100110011001100;
	while (pat) 
	{
		RGB_LED(pat&2 ? 30:0, pat&1 ? 60:0, pat&4? 40:0);
		delay(60);
		pat >>= 1;
	}	
}

// Main program
int main(void)
{
    char i;
    char *heap_end;
    unsigned short checksum = 0xdead;
    unsigned short seq = 100;

    // Initialize all modules
    uart_init(115200);
    accel_init();
    touch_init((1 << 9) | (1 << 10));       // Channels 9 and 10
    setvbuf(stdin, NULL, _IONBF, 0);        // No buffering
    // tests();

    blink(); 

    // Welcome banner
    iprintf("\r\n\r\n====== Freescale Freedom FRDM-KL25Z\r\n");

    getchar();

    iprintf("\r\nBuilt: %s %s\r\n", __DATE__, __TIME__);
    heap_end = _sbrk(0);
    iprintf("Reset code: 0x%02x 0x%02x\r\n", RCM_SRS0, RCM_SRS1);
    iprintf("Heap:  %p to %p (%d bytes used)\r\n", __heap_start, heap_end, 
                heap_end - (char *)__heap_start);
    iprintf("Stack: %p to %p (%d bytes used)\r\n", &i, __StackTop, 
                (char *)__StackTop - &i);
    iprintf("%d bytes free\r\n", &i - heap_end);

    iprintf("Ident, Count, Accel x,y,z, Touch a,z *Chksum\r\n");
    
    for(;;) {
        blink();
        iprintf("$$HEX,%d,%5d,%5d,%5d,",seq++, accel_x(), accel_y(), accel_z());
        iprintf("%3d,%3d*%x\r\n", touch_data(9), touch_data(10), checksum);
    }
}
