/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 *
 * mod_epLog.c
 *
 * Created: 3/10/2017 12:57:11 PM
 *  Author: Chip
 */ 
#include <avr/io.h>

#include "mod_epLog.h"
#include "eeprom_util.h"

uint16_t	me_NextAdrs;
uint16_t	me_NextAdrsOut;			// Different so Log can be read while Events are still active.

void me_init()
{
	me_NextAdrs = 0;
}

// Save TAG and Event time into EEPROM.
// Ignored if no room left.
void me_store1(EPT_TAG tag, time_t event)
{
	if(me_NextAdrs < 0x03F0) {
		EEPROM_write(me_NextAdrs++, (uint8_t)tag);
		EEPROM_write(me_NextAdrs++, (uint8_t)((event>>24) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event>>16) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event>>8) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)(event & 0x00FF));
	}
}

// Overload function.
void me_store2(EPT_TAG tag, time_t event1, time_t event2) {
	if(me_NextAdrs < 0x03F0) {
		EEPROM_write(me_NextAdrs++, (uint8_t)tag);
		EEPROM_write(me_NextAdrs++, (uint8_t)((event1>>24) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event1>>16) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event1>>8) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)(event1 & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event2>>24) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event2>>16) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)((event2>>8) & 0x00FF));
		EEPROM_write(me_NextAdrs++, (uint8_t)(event2 & 0x00FF));
	}
}

// Clear EEPROM to 0xFF
void me_clear()
{
	for(uint16_t i=0; i<0x03FF; i++) {
		EEPROM_write(i, 0xFF);
	}
}


// Return 0: Invalid tag else return tag.
uint8_t me_getLog(uint8_t init, char* buff) {
	EPT_TAG	tag;
	time_t	event;
	
	char* buffer = buff;
	char* ptr;

	if(init == 0) {
		me_NextAdrsOut = 0;
	}

	tag = EEPROM_read(me_NextAdrsOut++);
	if( (tag == EPT_ACK) || (tag == EPT_NOACTION) ) {
		*buffer++ = '0' + tag;					// '1' or '3'
		*buffer++ = ' ';
		event = EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);

		const struct tm* timeStrut = localtime(&event);
		ptr = isotime(timeStrut);
		while(*ptr != 0) {
			*buffer++ = *ptr++;
		}
	} else if(tag == EPT_SNOOZE) {
		*buffer++ = '0' + tag;					// '2'
		*buffer++ = ' ';
		// Event time
		event = EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);

		const struct tm* timeStrut = localtime(&event);
		ptr = isotime(timeStrut);
		while(*ptr != 0) {
			*buffer++ = *ptr++;
		}
		// Snooze ACK time
		*buffer++ = ' ';
		event = EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);
		event = (event<<8) + EEPROM_read(me_NextAdrsOut++);

		const struct tm* timeStrut2 = localtime(&event);
		ptr = isotime(timeStrut2);
		while(*ptr != 0) {
			*buffer++ = *ptr++;
		}
	} else {
		// Invalid tag
		return 0;
	}

	*buffer++ = 10;			// LF
	*buffer++ = 13;			// CR
	*buffer = 0;

	return tag;
}
