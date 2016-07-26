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
 *
 * eeprom_util.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * Based on example code from ATtiny84 data sheet.
 *
 */ 


#ifndef EEPROM_UTIL_H_
#define EEPROM_UTIL_H_

/* *** EEPROM USAGE MAP *** */
#define	PARAM_TABLE			0x0020		// reserve first 32 bytes for special use???
#define	PARAM_TABLE_SIZE	0x0080		// carve out 128 bytes. Leave the rest for CAL tables and such.


#define PARAM_NO_KEY	0x0100		// No key found

void EEPROM_write(unsigned int ucAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int ucAddress);

uint16_t param_get_value(uint8_t key);
void param_update(uint8_t key, uint8_t value);

void param_set_end(void);
void param_clear_all(void);

#endif /* EEPROM_UTIL_H_ */