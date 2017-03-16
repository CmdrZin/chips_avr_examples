/*
 * DemoApp1_ATtiny85.c
 *
 * Created: 1/23/2017 11:47:44 AM
 * Author : Chip
 */ 

#include <avr/io.h>
#include "mod_led.h"

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
		mod_led_on();
		for(int i=0; i<10000; ++i) {
			asm("nop");
		}

		mod_led_off();
		for(int i=0; i<10000; ++i) {
			asm("nop");
		}
    }
}

