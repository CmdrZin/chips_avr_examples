/*
 * eeprom_util.c
 *
 * Created: 6/22/2015 12:44:32 PM
 *  Author: Chip
 *
 * Example code from ATtiny84 data sheet.
 *
 */

#include <avr/io.h>
#include <stdbool.h>

#include "eeprom_util.h"

#define	PARAM_TABLE		0x0020


/*
 * input:	ucAddress	EEPROM address 0x0000-0x01FF
 *			ucData		8bit data value
 */
void EEPROM_write(unsigned int ucAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set Programming mode */
	EECR = (0<<EEPM1)|(0<<EEPM0);				// 0 0: Erase and Write (3.4ms)
	/* Set up address and data registers */
	EEAR = ucAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}


/*
 * input:	ucAddress	EEPROM address 0x0000-0x01FF
 *
 * output:	data		8bit data value
 */
unsigned char EEPROM_read(unsigned int ucAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = ucAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	return EEDR;
}

/*
 * Search Parameter Table for Key and return Value if Key matched.
 * End of table is 00:00
 * input:	Key		8bit
 * output:	Value	16bit. Upper byte = 0x00 if valid, 0x01 if not. Lower byte = data.
 * NOTE: Unprogrammed EEPROM is 0xFF.
 */
uint16_t param_get_value(uint8_t key)
{
	bool result = false;
	bool search = true;
	uint16_t adrs;				// EEPROM address
	uint8_t tkey;
	uint8_t value = 0;
	uint16_t retvalue;

	adrs = PARAM_TABLE;	
	while( search )
	{
		tkey = EEPROM_read(adrs);
		if(tkey == 0xFF) {
			// End of table.
			search = false;
		} else {
			++adrs;							// set up for Value.
			// Check Key
			if(tkey == key) {
				// Get Value
				value = EEPROM_read(adrs);
				result = true;				// valid value.
				search = false;				// exit search.
			} else {
				++adrs;						// set up for next Key.
			}
		}
	}
	// Check results to set upper byte flag.
	if(result) {
		retvalue = value;		
	} else {
		retvalue = PARAM_NO_KEY;
	}

	return retvalue;
}

/*
 * Search parameter table for Key.
 *	Update Value if Key matches.
 *	Else store Key and Value after last table entry.
 * NOTE: Unprogrammed EEPROM is 0xFF.
 */
void param_update(uint8_t key, uint8_t value)
{
	uint16_t adrs;				// EEPROM address
	bool search = true;
	uint8_t tkey;

	adrs = PARAM_TABLE;
	while( search )
	{
		tkey = EEPROM_read(adrs);
		if(tkey == 0xFF) {
			// End of table.
			EEPROM_write(adrs, key);		//  Add Key.
			++adrs;
			EEPROM_write(adrs, value);		//  Add Value.
			++adrs;
			EEPROM_write(adrs, 0xFF);		//  Set end of table.
			search = false;
		} else {
			++adrs;							// set up for Value.
			// Check Key
			if(tkey == key) {
				// Update Value
				EEPROM_write(adrs, value);
				search = false;				// exit search.
			} else {
				++adrs;						// set up for next Key check.
			}
		}
	}
}
