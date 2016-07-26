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
 * Slave API Project
 *
 * org: 08/08/2015		0.01	ndp
 * author: Nels "Chip" Pearson
 *
 * Dependent on:
 *	module function files
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysdefs.h"

// Device prototypes
#include "core_functions.h"
#include "io_led_button.h"
#include "dev_display.h"


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
	{ DEV_DISPLAY_ID, dev_display_init },
	{ MOD_IO_LED_BUTTON_ID,	mod_io_init },
	{ CORE_FUNCTIONS_ID, core_init },
	{ 0, 0}
};

/*
 * Used by service.c :: service_all()
 */
const MOD_FUNCTION_ENTRY mod_service_table[] PROGMEM =
{
	{ DEV_DISPLAY_ID, dev_display_service },
	{ MOD_IO_LED_BUTTON_ID,	mod_io_service },
	{ CORE_FUNCTIONS_ID, core_service },
	{ 0, 0}
};

/*
 * Used by dev_access.c :: access_all() for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_io_access[] PROGMEM =
{
	{ 0x01, mod_io_getButtons },
	{ 0x10, mod_io_setGreenLed },
	{ 0x11, mod_io_setRedLed },
	{ 0x12, mod_io_setYellowLed },
	{ 0, 0 }
};

/*
 * Used by dev_access.c :: access_all() for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY dev_display_access[] PROGMEM =
{
	{ 0x01, dev_display_set_text },
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
 * This table is auto-generated from the configuration file.
 * Used by access.c :: access_all()
 */
const MOD_ACCESS_ENTRY mod_access_table[] PROGMEM =
{
	{ MOD_IO_LED_BUTTON_ID, mod_io_access },	// table to all functions supported by LED and buttons.
	{ DEV_DISPLAY_ID, dev_display_access },		// table to all functions supported by device display.
	{ CORE_FUNCTIONS_ID, core_access },			// table to all functions supported by device debug.
	{ 0, 0 }
};
