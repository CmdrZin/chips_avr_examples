/*
 * ImpactMonB.c
 *
 * Created: 10/31/2015		0.01	ndp
 *  Author: Chip
 *
 * Main for Impact Monitor B project.
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "io7SegLedsButtons.h"
#include "mod_UserIO.h"
#include "twi_I2CMaster.h"
#include "mod_adxl345.h"
#include "mod_collection.h"

int main(void)
{
	st_init_tmr0();		// Set up timer
	idl_init();			// Set up IO for LEDs and Buttons
	uio_init();			// Set up User IO variables
	tim_init();			// Initialize I2C interface
	mco_init();			// Initialize ADXL service
	
	sei();				// Enable interrupts
	
	while(1)
	{
#if 1
		idl_service();			// service LED multiplexed display.
		uio_service();			// service general display Modes.
		mco_service();			// service ADXL collection.
#else
//		DDRB = 0x83;
		PORTD = 0xFF;
		PORTB = 0x83;
#endif
	}
}
