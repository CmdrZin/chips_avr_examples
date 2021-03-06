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
#include "mod_UserIO.h"
#include "twi_I2CMaster.h"
#include "mod_adxl345.h"
#include "mod_serial.h"
#include "mod_collection.h"

int main(void)
{
	st_init_tmr0();		// Set up timer
	io_init();			// Set up IO for LEDs and Buttons
	uio_init();			// Set up User IO variables
	tim_init();			// Initialize I2C interface
//	mod_adxl_init();	// Initialize ADXL
	mod_serial_init();	// Initialize Serial port
	mco_init();			// Initialize ADXL service
	
	sei();				// Enable interrupts
	
    while(1)
    {
		idl_service();			// service LED bar graph for multiplexed display update.
		uio_service();			// service general display Modes.
		mco_service();			// service ADXL collection.
    }
}
