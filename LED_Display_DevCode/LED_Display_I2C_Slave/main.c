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
* LED_Display_I2C_Slave.c
*
* Created: 6/29/2017	0.01	ndp
* Author : Chip
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sysTimer.h"
#include "twiSlave.h"
#include "io_4x27SegLeds.h"
#include "mod_4x2LedDisplay.h"

#define SLAVE_ADRS	0x58

uint8_t util_byte2bcd(uint8_t val);
uint32_t util_int2bcd(uint16_t val);

uint16_t count;
uint8_t delayTime;			// N * 10ms

int main(void)
{
	uint16_t converted;
	uint32_t converted2;
	
	count = 0;
	delayTime = 100;

	st_init_tmr0();
	twiSlaveInit( SLAVE_ADRS );
	mod_LD_init();

	sei();
	
	mod_LD_setMode(MLD_NORMAL);
	mod_LD_setDP(2, 1);
	mod_LD_setDP(5, 1);
	
	/* Replace with your application code */
	while (1)
	{
		mod_LD_service();
		// Check for TWI input and process it.
		
		if( GPIOR0 & (1<<SYS_10MS_TIC) )
		{
			GPIOR0 &= ~(1<<SYS_10MS_TIC);				// clear flag
			if( --delayTime == 0 )
			{
				delayTime = 100;
				++count;
				converted = util_byte2bcd((uint8_t)(count & 0x00FF));
				mod_LD_setDigit(2, (converted>>4) & 0x0F);
				mod_LD_setDigit(3, converted & 0x0F);

//				mod_LD_setDigit(7, count & 0x0F);
//				mod_LD_setDigit(6, (ISL_DECIMAL_POINT | (count>>4)) & 0x1F);

				converted2 = util_int2bcd(count);
				if(count < 1000) {
					mod_LD_setDigit(4, ISL_BLANK);			// leading zero blank
				} else {
					mod_LD_setDigit(4, (converted2>>12) & 0x0F);
				}
				mod_LD_setDigit(5, (converted2>>8) & 0x0F);
				mod_LD_setDigit(6, (converted2>>4) & 0x0F);
				mod_LD_setDigit(7, converted2 & 0x0F);
			}
		}
	}
}

/*
 * Convert a 16bit value into 4 digit BCD max 9999
 */
uint32_t util_int2bcd(uint16_t val)
{
	uint32_t vout = val / 1000;
	uint32_t vtemp;
		
	if( vout > 9 ) vout = 9;
	vout <<= 4;
	vtemp = val % 1000;
	vout |= vtemp / 100;
	vout <<= 4;
	vtemp = val % 100;
	vout |= vtemp / 10;
	vout <<= 4;
	vout |= val % 10;
	
	return vout;
}

/*
 * Convert an 8bit value to BCD max 99
 */
uint8_t util_byte2bcd(uint8_t val)
{
	uint8_t vout = val / 10;
	if( vout > 9 ) vout = 9;
	vout <<= 4;
	vout |= val % 10;
	
	return( vout );
}
