/*
 * ImpactMon.c
 *
 * Created: 8/24/2015	0.01	ndp
 *  Author: Chip
 *
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "ioLedsButtons.h"


int main(void)
{
	st_init_tmr0();		// Set up timer
	io_init();			// Set up IO for LEDs and Buttons
	
	sei();				// Enable interrupts
	
    while(1)
    {
		io_service();
		

    }
}