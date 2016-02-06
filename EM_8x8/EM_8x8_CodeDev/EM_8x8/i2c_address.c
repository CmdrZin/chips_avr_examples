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
 * i2c_address.c
 *
 * Created: 1/17/2016 10:07:38 AM
 *  Author: Chip
 *
 * This module supports external jumpers to adjust the I2C address.
 */ 

#include <avr/io.h>

#include "i2c_address.h"

#define I2C_ADRS_DDR	DDRC
#define I2C_ADRS_PORT	PORTC
#define I2C_ADRS_PIN	PINC

#define I2C_ADRS_D0		PORTC0
#define I2C_ADRS_D1		PORTC1
#define I2C_ADRS_D2		PORTC2
#define I2C_ADRS_D3		PORTC3


/*
 * This function returns the I2C address modified by the jumpers.
 * Change as needed to match hardware.
 */
uint8_t ia_getAddress()
{
	uint8_t adrs;
	
	// Default is INPUT
	// Set Pull-Ups and wait for a bit to stabilize
	for(adrs = 0; adrs<20; ++adrs)
	{
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D3);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D2);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D1);
		I2C_ADRS_PORT |= (1<<I2C_ADRS_D0);
	}

	// Inverted to make jumpers 1 if inserted and 0 if not.
	adrs = ~(I2C_ADRS_PIN);
	//  Mask to match pins used.
	adrs &= (1<<I2C_ADRS_D3)|(1<<I2C_ADRS_D2)|(1<<I2C_ADRS_D1)|(1<<I2C_ADRS_D0);
		
	return( SLAVE_ADRS + adrs );	// TODO: Pull this from EEPROM someday.
}
