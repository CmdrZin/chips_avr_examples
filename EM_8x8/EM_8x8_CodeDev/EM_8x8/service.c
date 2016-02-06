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
 * service.c
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 */ 

#include <avr/io.h>

#include "function_tables.h"

#include "flash_table.h"

/*
 * Calls each service using a look-up branch table.
 * The service checks its associated TIC bit and calls its service function if its bit is set.
 * The TIC bits are defined in the sysTimer.h file.
 */

void service_all()
{
	int	index = 0;
	uint16_t deviceID;
	void (*func)();

	while(1) {
		deviceID = flash_get_access_cmd(index, (MOD_FUNCTION_ENTRY*)mod_service_table);
		func = (void (*)())flash_get_access_func(index, (MOD_FUNCTION_ENTRY*)mod_service_table);

		if(deviceID != 0) {
			func();
			} else {
			break;
		}
		++index;
	}
}