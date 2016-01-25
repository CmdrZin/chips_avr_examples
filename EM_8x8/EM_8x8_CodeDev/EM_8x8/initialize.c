/*
 * initialize.c
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * revision: 1/13/2016	0.02	ndp		add ATmega48P -> 328P reset code.
 * revision: 1/17/2016	0.03	ndp		get SLAVE_ADRS from ia_getAddress()
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "initialize.h"
#include "sysdefs.h"
#include "i2c_address.h"
#include "sysTimer.h"
#include "function_tables.h"
#include "access.h"

#include "twiSlave.h"
#include "flash_table.h"
#include "eeprom_util.h"

#include "core_functions.h"

void init_all()
{
	int	index;
	uint16_t deviceID;
	void (*func)();
	
	/* *** Core initialization operations that are part of every build. *** */
#if defined(__AVR_ATtiny24__) | defined(__AVR_ATtiny44__) | defined(__AVR_ATtiny84__) \
	| defined(__AVR_ATtiny24A__) | defined(__AVR_ATtiny44A__) | defined(__AVR_ATtiny84A__)
	DDRB &= ~(1<<PB3);			// set RESET line as INPUT.
	PORTB |= (1<<PB3);			// enable RESET line pull-up.
#endif
#if defined(__AVR_ATmega48P__) | defined(__AVR_ATtiny88P__) | defined(__AVR_ATtiny168PA__)| defined(__AVR_ATtiny328P__)
	DDRC &= ~(1<<PC6);			// set RESET line as INPUT.
	PORTC |= (1<<PC6);			// enable RESET line pull-up.
#endif
// TODO: Add ATmega164P and 328P RESET pull-up code.

//	param_set_end();			// called in case of corruption. Could do CKSUM test on param table.
//	param_clear_all();
	
	st_init_tmr0();
	twiSlaveInit( ia_getAddress() );
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
