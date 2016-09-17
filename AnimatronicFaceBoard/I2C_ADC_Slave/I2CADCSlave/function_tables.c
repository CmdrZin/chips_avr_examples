/*
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
	{ CORE_FUNCTIONS_ID, core_init },
	{ 0, 0}
};

/*
 * Used by service.c :: service_all()
 */
const MOD_FUNCTION_ENTRY mod_service_table[] PROGMEM =
{
	{ CORE_FUNCTIONS_ID, core_service },
	{ 0, 0}
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
	{ CORE_FUNCTIONS_ID, core_access },	// table to all functions supported by device debug.
	{ 0, 0 }
};
