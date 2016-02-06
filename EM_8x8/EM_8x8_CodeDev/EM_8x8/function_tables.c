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
 * Slave API Project - EM 8x8 board
 *
 * org: 08/08/2015					0.01	ndp
 * author: Nels "Chip" Pearson
 * revision: 01/12/2016				0.02	ndp
 * revision: 02/01/2016				0.03	ndp	Added marquee support
 *
 * Dependent on:
 *	module function files
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysdefs.h"

// Device prototypes
#include "core_functions.h"
#include "mod_em_service.h"


/* *** Call Tables for INIT, SERVICE, and ACCESS *** */

/*
 * Used by intialize.c :: init_all()
 * Format:
 *  struct {
 *	  uint16_t	id;
 *	  void		(*function)();
 *	}
 */
const MOD_FUNCTION_ENTRY mod_init_table[] PROGMEM =
{

	{ MOD_EM_SERVICE_ID, mod_em_service_init },
	{ CORE_FUNCTIONS_ID, core_init },
	{ 0, 0}
};

/*
 * Used by service.c :: service_all()
 */
const MOD_FUNCTION_ENTRY mod_service_table[] PROGMEM =
{
	{ MOD_EM_SERVICE_ID, mod_em_service_service },
	{ CORE_FUNCTIONS_ID, core_service },
	{ 0, 0}
};

/*
 * Returned by mod_access_table[] for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_em_service_access[] PROGMEM =
{
	{ 0x01, mes_setIcon },
	{ 0x02, mes_loadIcon },
	{ 0x40, mes_selectMarquee},
	{ 0x41, mes_setKerningFlag },
	{ 0x42, mes_setContinuousFlag },
	{ 0x43, mes_setRate },
	{ 0x44, mes_setIconSeries },
	{ 0, 0 }
};

/*
 * Used by access.c :: access_all() for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY core_access[] PROGMEM =
{
	{ 0x02, core_get_version },
	{ 0x0A, core_get_build_time },
	{ 0x0B, core_get_build_date },
	{ 0, 0 }
};

/*
 * Used by access.c :: access_all()
 */
const MOD_ACCESS_ENTRY mod_access_table[] PROGMEM =
{
	{ MOD_EM_SERVICE_ID, mod_em_service_access },			// table to all functions supported by mod_em_service.
	{ CORE_FUNCTIONS_ID, core_access },						// table to all functions supported by device debug.
	{ 0, 0 }
};
