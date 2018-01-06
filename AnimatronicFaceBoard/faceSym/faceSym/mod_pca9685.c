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
 * mod_pca9685.c
 *
 * Created: 9/29/2016 2:01:42 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "mod_pca9685.h"
#include "twi_I2CMaster.h"

#define MP_ADRS		0x40		// I2C Address

uint8_t mp_buff[4];


/*
 * 
 * Call after setting up TWI I2C interface and enabling Interrupts.
 */
void mod_pca9685_init()
{
	while( tim_isBusy());					// Wait for last message to complete.

	// Set PreScale to 0x79 for 50 Hz .. Val = [ 25 MHz / (4096 x freq) ] -1 = 121 (0x79)
	mp_buff[0] = 0xFE;
	mp_buff[1] = 0x85;
	tim_write( MP_ADRS, mp_buff, 2);		// n_ON_L
	while( tim_isBusy());

	// Enable
	mp_buff[0] = 0x00;						// MODE 1
	mp_buff[1] = 0x00;
	tim_write( MP_ADRS, mp_buff, 2);		// n_ON_L
	while( tim_isBusy());

	// Set 0-15 to MP_MID (307)
	for( uint8_t i = 0; i < 16; i++ ) {
		mp_setPos( i, MP_CENTER );
	}
}

/*
 * Set Servo Channel Position
 */
void mp_setPos( uint8_t chan, uint16_t pos )
{
	while( tim_isBusy());					// Wait for last message to complete.

	mp_buff[0] = 0x06 + (chan<<2);
	mp_buff[1] = 0x01;
	tim_write( MP_ADRS, mp_buff, 2);		// n_ON_L
	while( tim_isBusy());

	++mp_buff[0];
	mp_buff[1] = 0x00;
	tim_write( MP_ADRS, mp_buff, 2);		// n_ON_H
	while( tim_isBusy());

	++mp_buff[0];
	mp_buff[1] = (uint8_t)(pos & 0x00FF);
	tim_write( MP_ADRS, mp_buff, 2);		// n_OFF_L
	while( tim_isBusy());

	++mp_buff[0];
	mp_buff[1] = (uint8_t)(pos>>8);
	tim_write( MP_ADRS, mp_buff, 2);		// n_OFF_H
	while( tim_isBusy());
}
