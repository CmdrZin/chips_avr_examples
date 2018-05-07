/*
 * GlowEyesType2.c
 *
 * Created: 4/5/2018 9:33:29 PM
 * Author : Chip
 */ 

#include <avr/io.h>

#include "ws2812_util.h"

#define NUM_OF_PIXS	2
#define BYTE_CHAIN	(NUM_OF_PIXS * 3)
char buf[8];

int main(void)
{
	int flag = 0;
	buf[0] = 0x00;		// Green
	buf[1] = 0x00;		// Red
	buf[2] = 0x00;		// Blue
	buf[3] = 0x00;		// Green
	buf[4] = 0x00;		// Red
	buf[5] = 0x00;		// Blue
	
    /* Replace with your application code */
    while (1) 
    {
#if 1
		ws2812_update((void*)buf, BYTE_CHAIN);

		// Simple delay
		for(int i=0; i<30000; i++) {
			asm("nop");
		}

		// Simple test
		if(flag == 0) {
			if(buf[0] != 255) {
				++buf[0];
			} else {
				if(buf[2] != 255) {
					++buf[2];
				} else {
					if(buf[1] != 255) {
						++buf[1];
					} else {
						flag = 1;
					}
				}
			}
		} else {
			if(buf[0] != 0) {
				--buf[0];
			} else {
				if(buf[2] != 0) {
					--buf[2];
				} else {
					if(buf[1] != 0) {
						--buf[1];
					} else {
						flag = 0;
					}
				}
			}
		}
#else
		DDRC |= (1<<PC6);
		PORTC |= (1<<PC6);
		for(int i=0; i<300; i++) {
			asm("nop");
		}
		PORTC &= ~(1<<PC6);
		for(int i=0; i<300; i++) {
			asm("nop");
		}
#endif
    }

	return(0);
}

