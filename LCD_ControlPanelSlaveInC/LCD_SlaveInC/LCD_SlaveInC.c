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
 * LCD_SlaveInC.c
 *
 * Created: 7/20/2016 12:15:48 PM
 *  Author: Chip
 *
 * This is the main() file for the Core Slave API Middleware.
 * It contains the initialization call and the task scheduler loop.
 * Other than DEBUG code, no changes should be needed to this file.
 */ 


#include <avr/io.h>

#include "initialize.h"
#include "service.h"
#include "access.h"

// DEBUG ++
#include "twiSlave.h"
// DEBUG --

/*
 * main()
 *
 * This is a sequential scheduler that services all available tasks
 * and then checks for a complete message and forwards it to the 
 * addressed task access process.
 *
 * Each task manages their own time slice.
 *
 */
int main(void)
{
	init_all();

	while(1)
	{
		service_all();
		
		access_all();

#if 0
		// DEBUG ++
			if(!twiDataInReceiveBuffer()) {
				twiStuffRxBuf( 0xF0 );
				twiStuffRxBuf( 0xFE );
				twiStuffRxBuf( 0x02 );
			}
		// DEBUG --
#endif
	}

}
