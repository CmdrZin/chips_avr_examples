/*
 * initialize.c
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "initialize.h"
#include "sysdefs.h"
#include "sysTimer.h"
#include "function_tables.h"
#include "access.h"

#include "twiSlave.h"
#include "flash_table.h"

#include "core_functions.h"

void init_all()
{
	int	index;
	uint16_t deviceID;
	void (*func)();
	
	// ATmega164PA has an internal pull.
	
	st_init_tmr0();
	twiSlaveInit( SLAVE_ADRS );			// TODO: Pull this from EEPROM someday.
	access_init();

	/* *** Device initialization based on command_tables auto-generated based on devices used. *** */
	index = 0;
	while(1) {
		deviceID = flash_get_access_cmd(index, (MOD_FUNCTION_ENTRY*)mod_init_table);
		func = (void (*)())flash_get_access_func(index, (MOD_FUNCTION_ENTRY*)mod_init_table);
		if(deviceID != 0) {
			func();
		} else {
			break;
		}
		++index;
	}

	sei();					// Enable Interrupts.
	
	twiSlaveEnable();		// Enable I2C Slave interface.

	return;
}
