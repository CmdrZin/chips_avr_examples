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
 * mod_comms.c
 *
 * Created: 1/31/2017 9:39:18 PM
 *  Author: Chip
 * revision:	09/19/2018		0.02	ndp		Hack to use for USBSerialTest
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysTimer.h"
#include "mod_comms.h"
#include "serial.h"
#include "io_led_button.h"

void triggerBeepTone(uint8_t tone);		// in main.c

uint16_t dev_bin2BCD(uint8_t data);

MC_MODE mc_comm_mode = MC_IDLE;

char mc_inBuff[24];			// input comm buffer
char mc_outBuff[48];		// output comm buffer

uint8_t mc_count = 0;

void mod_comms_service()
{
	char		data;

	switch(mc_comm_mode)
	{
		case MC_IDLE:
			if(isChar())
			{
				data = recvChar();
				// parse command
				switch(data)
				{
					case 'B':
						triggerBeepTone(2);
						break;
						
					case 'C':
						if(mc_count > 9) {
							mc_count = 0;
						}
						sendChar('@');
						sendChar(mc_count + '0');
						sendChar(13);		// CR
						sendChar(10);		// LF
						++mc_count;
						break;
						
					case 'G':
						mod_io_setBlink(100, IO_GREEN_LED);
						break;

					case 'R':
						mod_io_setBlink(100, IO_RED_LED);
						break;
						
					default:
						break;
				}
			}
			break;

		default:
			mc_comm_mode = MC_IDLE;
			break;
	}
}

MC_MODE mod_commGetMode()
{
	return mc_comm_mode;
}

/*
 * Binary (8bit) to ASCII BCD ('0''0'-'9''9')
 * Copy from dev_display.c
 * 
 */
uint16_t dev_bin2BCD(uint8_t data)
 {
	uint16_t result = 0;
	uint8_t temp = data;

	for(uint8_t i=0; i<8; i++) {
		// test bits
		result += ((result & 0x0F) > 4) ? 3 : 0;
		result += ((result & 0xF0) > 0x40) ? 0x30 : 0;
		// shift into result
		result <<= 1;
		result |= (temp & 0x80) ? 1 : 0;
		// shift data
		temp <<= 1;
	}

	temp = (result & 0x0F) + 0x30;				// add ASCII '0'
	result = (result << 4) + 0x3000;
	result &= 0xFF00;
	result += temp;

	return result;
 }
