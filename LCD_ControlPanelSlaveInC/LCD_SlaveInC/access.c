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
 * access.c
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * revision: 8/13/2015	0.02	ndp		make getMsgData() global.
 *
 * This is the message header processor for I2C messages.
 *
 * Command Format
 *   The command format uses a three byte HEADER plus an optional fifteen bytes of DATA.
 *   The HEADER format in LEN MOD CMD where
 *     LEN   b3:0 = number of additional bytes of data.
 *           b7:4 = bit negation of b3:0 for validity check.
 *     MOD   The ID of module being addressed. 01:FE
 *     CMD   The command being sent to the module. 01:FE
 *     DATA  Additional data associated with the command. 00:FF
 *   NOTE: For MOD and CMD, the values 00 and FF are reserved and can not be used.
 *
 */ 

#include <avr/io.h>

#include "sysdefs.h"
#include "function_tables.h"
#include "twiSlave.h"
#include "flash_table.h"


#define ACCESS_MSG_BUFF_SIZE 20
static uint8_t accMsgBuff[ACCESS_MSG_BUFF_SIZE];		// copy of command string.
static uint8_t accMsgIndex;					// index reset to 0 after command process.
static uint8_t accMsgSize;						// expected total length of message.
static MOD_FUNCTION_ENTRY* accFuncTable;		// Command table for Device of current Message.

/*
 * Get message data.
 */
uint8_t getMsgData( uint8_t index )
{
	if( index < ACCESS_MSG_BUFF_SIZE ) {
		return accMsgBuff[index];
	} else {
		return(0);					// ERROR
	}
}

/*
 * Initialize GLOBAL variables use by access_all().
 */
void access_init()
{
	accMsgIndex = 0;
	accMsgSize = 0;
	accFuncTable = 0;
}

/* Service incoming I2C message. */
void access_all()
{
	uint8_t temp;
	uint8_t index;
	bool search;
	uint8_t mod;
	uint8_t cmd;
	bool scan;
	
	void (*func)() = 0;

	/* Check for I2C message. */
	if(twiDataInReceiveBuffer())
	{
		accMsgBuff[accMsgIndex] = twiReceiveByte();
		if( ++accMsgIndex >= ACCESS_MSG_BUFF_SIZE )
		{
			accMsgIndex = 0;					// ERROR..too many bytes.
			accMsgSize = 0;
			accFuncTable = 0;
			while( twiDataInReceiveBuffer() )
			{
				// Flush input buffer.
				(void)twiReceiveByte();
			}
		}

		// Valid message being received? ~LEN.7:4 == LEN.3:0
		if ( accMsgIndex == 1)
		{
			accMsgSize = getMsgData(0);
			// NOTE: Compiler was treating as 16bit using r25:24
			temp = ~accMsgSize;
			temp >>= 4;
			temp &= 0x0F;
			accMsgSize &= 0x0F;
			if( temp != accMsgSize )
			{
				accMsgIndex = 0;		// ERROR..size check failed.
				accMsgSize = 0;
			}
			else
			{
				accMsgSize += 3;		// ALL messages are a minimum of three bytes.
			}
		}

		// Message check. ALL messages are at least three bytes long. LEN MOD CMD
		if ( accMsgIndex == 3)
		{
			// Three bytes received. Should be a LEN MOD CMD. Check for a MOD match.
			search = true;
			index = 0;
			while(search)
			{
				mod = flash_get_mod_access_id(index);

				if ( mod == 0)
				{
					// End of list. No match.
					search = false;
					accMsgIndex = 0;
					accMsgSize = 0;
					accFuncTable = 0;
				}
				else if ( mod == accMsgBuff[1] )
				{
					search = false;
					// Get the function table for this module.
					accFuncTable = flash_get_mod_function_table(index);
				}
				++index;
			}
		} // end if == 3

		// Process command now?
		if ( (accMsgIndex == accMsgSize) && (accMsgIndex != 0) && (accFuncTable != 0) )
		{
			scan = true;
			index = 0;
			while(scan)
			{
				cmd = flash_get_access_cmd(index, accFuncTable);
				func = (void (*)())flash_get_access_func(index, accFuncTable);
				
				if ( accMsgBuff[2] == cmd )
				{
					func();
					accMsgIndex = 0;
					accMsgSize = 0;
					accFuncTable = 0;
					scan = false;
				}
				++index;
			}
		}
	} // end if recv data
}
