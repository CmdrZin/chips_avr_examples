/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * eeprom_util.c
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * Example code from ATtiny84 data sheet to access EEPROM.
 * Parameter code added to read and write KEY:VALUE parameters.
 *
 * TODO: Update to handle more than just 8 bit VALUE objects.
 */

#include <avr/io.h>
#include <stdbool.h>

#include "eeprom_util.h"


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
	while( search && (adrs < PARAM_TABLE+PARAM_TABLE_SIZE+1) )
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
			}
			++adrs;						// set up for next Key check.
		}
	}
}

/*
 * Set end of parameter table.
 */
void param_set_end()
{
	EEPROM_write(PARAM_TABLE+PARAM_TABLE_SIZE, 0xFF);		//  NO VALUE
	EEPROM_write(PARAM_TABLE+PARAM_TABLE_SIZE+1, 0xFF);		//  NO VALUE
}


/*
 * Clear parameter table
 */
void param_clear_all()
{
	uint16_t adrs = PARAM_TABLE;
	
	while(adrs < PARAM_TABLE+PARAM_TABLE_SIZE)
	{
		EEPROM_write(adrs, 0xFF);		//  NO VALUE
		++adrs;
	}
}
