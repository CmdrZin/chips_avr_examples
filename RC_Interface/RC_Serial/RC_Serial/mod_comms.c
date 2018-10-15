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
 *
 * Commands:
 * SYNC 0x00 STATE		Controls the LED. STATE = 0:OFF, 1:ON
 * SYNC 0x01 TT			Set Throttle to TT (0x00 Full Reverse..0x80 Stop..0xFF Full Forward)
 * SYNC 0x02 SS			Set Steering to SS (0x00 Full Left..0x80 Center..0xFF Full Right)
 * SYNC 0x10			Returns temperature data (ADCH ADCL '\n') once per second.
 * SYNC 0x11 			Returns Throttle and Steering data (TT ST '\n') every 20ms.
 *
 * txBuf[0] = 0x00 		always
 * txBuf[1] = ADCH		Left shifted..upper 8 bits
 * txBuf[2] = ADCL		Lower 2 bits
 * txBuf[3] = TROTTLE	One byte
 * txBuf[4] = STEERING	One byte
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysTimer.h"
#include "mod_comms.h"
#include "serial.h"
#include "mod_led.h"

#define SYNC 0x5A

void triggerBeepTone(uint8_t tone);		// in main.c

uint16_t dev_bin2BCD(uint8_t data);

// MC_MODE: MC_IDLE, MC_CMD, MC_DATA, MC_TEMP, MC_TT_ST, MC_SET_LED 
MC_MODE mc_rx_comm_mode = MC_IDLE;
MC_MODE mc_tx_comm_mode = MC_IDLE;

uint8_t mc_txBuf[6];

long mc_timeWait = 0;

void mod_comms_service()
{
	char		data;

	// Process Serial Input
	switch(mc_rx_comm_mode)
	{
		case MC_IDLE:
			if(isChar())
			{
				if( recvChar() == SYNC ) {
					mc_rx_comm_mode = MC_CMD;
				} // else ignore
			}
			break;
			
		case MC_CMD:
			if(isChar()) {
				data = recvChar();
				// Set up Command Process.
				switch(data) {
					case 0x00:
						mc_rx_comm_mode = MC_SET_LED;
						break;
						
					case 0x10:
						mc_tx_comm_mode = MC_TEMP;
						mc_rx_comm_mode = MC_IDLE;
						break;
						
					case 0x11:
						mc_tx_comm_mode = MC_TT_ST;
						mc_rx_comm_mode = MC_IDLE;
						break;
						
					default:
						break;
				}
			}
			break;
			
		case MC_SET_LED:
			if(isChar()) {
				data = recvChar();

				if(data == 0x00) {
					mod_led_off();
				}
				
				if(data == 0x01) {
					mod_led_on();
				}
				
				mc_rx_comm_mode = MC_IDLE;
			}
			break;

		default:
			mc_rx_comm_mode = MC_IDLE;
			break;
	}

	// Control Serial Output
	if( mc_timeWait < st_millis() ) {

		switch(mc_tx_comm_mode)
		{
			case MC_TEMP:
				mc_timeWait = st_millis() + 1000;		// set to 1 sec wait.
				sendChar(mc_txBuf[1]);
				sendChar(mc_txBuf[2]);
				sendChar(13);		// CR
				sendChar(10);		// LF
				break;
			
			case MC_TT_ST:
				mc_timeWait = st_millis() + 20;			// set to 20 msec wait.
				sendChar(mc_txBuf[3]);
				sendChar(mc_txBuf[4]);
				sendChar(13);		// CR
				sendChar(10);		// LF
				break;
				
			default:
				mc_tx_comm_mode = MC_TEMP;
				break;
		}
	}
}

// Set Transmit data register to value.
void mod_comm_setReg(int reg, uint8_t val)
{
	if(reg < sizeof(mc_txBuf)) {
		mc_txBuf[reg] = val;
	}
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
