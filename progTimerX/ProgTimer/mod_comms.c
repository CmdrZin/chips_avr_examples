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
 */ 
#include <avr/io.h>

#include "sysTimer.h"
#include "mod_comms.h"
#include "serial.h"
#include "mod_parse.h"
#include "mod_eventList.h"
#include "mod_epLog.h"

// In main.c
void setSelfCheck();

uint16_t dev_bin2BCD(uint8_t data);

MC_MODE mc_comm_mode = MC_IDLE;

char mc_inBuff[24];			// input comm buffer
char mc_outBuff[48];		// output comm buffer

uint8_t mc_numChar = 0;

void mod_comms_service()
{
	char	data;
	char*	ptr;
	uint16_t	val;
	time_t	eTime;
	uint8_t tag;

	switch(mc_comm_mode)
	{
		case MC_IDLE:
			if(isChar())
			{
				data = recvChar();
				// parse command
				switch(data)
				{
					// Go into Pre-Use Self-Check
					case 'C':
						setSelfCheck();
						sendChar('C');
						sendChar('-');
						sendChar('O');
						sendChar('K');
						sendChar(10);		// LF
						sendChar(13);		// CR
						break;

					// Dump LOG
					case 'D':
						mc_comm_mode = MC_SEND_LOG;
						break;

					// Erase LOG
					case 'E':
						me_clear();
						sendChar('E');
						sendChar('-');
						sendChar('O');
						sendChar('K');
						sendChar(10);		// LF
						sendChar(13);		// CR
						break;

					// Start of List entry..L!2017-09-23 12:34:56
					case 'L':
						mc_comm_mode = MC_RECV_LIST;
						mc_numChar = 0;
						break;

					// Read back List
					case 'R':
						mc_comm_mode = MC_SEND_LIST;
						break;

					// Set Local Time..S2017-02-07 11:23:45
					case 'S':
						mc_comm_mode = MC_RECV_LOCAL;
						mc_numChar = 0;
						break;

					// Read Local Time
					case 'T':
						mc_comm_mode = MC_SEND_LOCAL;
						break;

					default:
						sendChar('?');
						sendChar(data);		// echo back unknown command.
						sendChar(10);		// LF
						sendChar(13);		// CR
						break;
				}
			}
			break;

		case MC_RECV:
			break;

		case MC_RECV_LIST:
			if(isChar())
			{
				data = recvChar();
				mc_inBuff[mc_numChar] = data;
				if(++mc_numChar == 20)
				{
					mc_inBuff[mc_numChar] = 0;			// add NULL
					ptr = mc_outBuff;
					// parse Event time and add to List in RAM.
					data = mc_inBuff[0];				// save index
					eTime = mod_parse_event(&mc_inBuff[1]);
					if(eTime != 0) {
						mod_eventList_add(data, eTime);
						*ptr++ = 'O';
						*ptr++ = 'K';
					} else {
						*ptr++ = '?';
					}
					*ptr++ = 10;		// LF
					*ptr++ = 13;		// CR
					*ptr = 0;

					mc_comm_mode = MC_SEND;		// NOTE: Could lose incoming data is Host does not wait for OK or ?.
				}
			}
			break;

		case MC_RECV_LOCAL:
			if(isChar())
			{
				data = recvChar();
				mc_inBuff[mc_numChar] = data;
				if(++mc_numChar == 19)
				{
					mc_inBuff[mc_numChar] = 0;			// add NULL
					ptr = mc_outBuff;
					// parse Local time.
					eTime = mod_parse_event(mc_inBuff);
					if(eTime != 0) {
						st_setLocalTime(eTime);
						*ptr++ = 'O';
						*ptr++ = 'K';
					} else {
						*ptr++ = '?';
					}
					*ptr++ = 10;		// LF
					*ptr++ = 13;		// CR
					*ptr = 0;

					mc_comm_mode = MC_SEND;		// NOTE: Could lose incoming data is Host does not wait for OK or ?.
				}
			}
			break;

		case MC_SEND:
			ptr = mc_outBuff;
			while( (data = *ptr++) != 0)
			{
				sendChar(data);
			}
			mc_comm_mode = MC_IDLE;
			break;

		case MC_SEND_LIST:
			for(uint8_t i=0; i<ME_EVENTS; i++) {
//			for(uint8_t i=30; i<36; i++) {
				val = dev_bin2BCD(i);
				ptr = mc_outBuff;
				*ptr++ = val >> 8;
				*ptr++ = val & 0x00FF;
				*ptr++ = ' ';
				*ptr = 0;
				ptr = mc_outBuff;
				while( (data = *ptr++) != 0)
				{
					sendChar(data);
				}

				ptr = mod_eventList_get(i);
				while( (data = *ptr++) != 0)
				{
					sendChar(data);
				}
				sendChar(10);
				sendChar(13);
			}
			mc_comm_mode = MC_IDLE;
			break;

		case MC_SEND_LOCAL:
			ptr = st_getLocalAscii();
			while( (data = *ptr++) != 0)
			{
				sendChar(data);
			}
			sendChar(10);
			sendChar(13);
			mc_comm_mode = MC_IDLE;
			break;

		case MC_SEND_LOG:
			tag = me_getLog(0, mc_outBuff);

			while(tag != 0) {
				ptr = mc_outBuff;
				while( (data = *ptr++) != 0)
				{
					sendChar(data);
				}
				sendChar(10);
				sendChar(13);
				// Next LOG entry.
				tag = me_getLog(1, mc_outBuff);
			}
			mc_comm_mode = MC_IDLE;
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
