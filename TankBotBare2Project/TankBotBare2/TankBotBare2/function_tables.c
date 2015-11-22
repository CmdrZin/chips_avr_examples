/*
 * Slave API Project - Tank Bot Bare ver 2 board
 *
 * org: 08/08/2015		0.01	ndp
 * author: Nels "Chip" Pearson
 * revision:	11/06/2015	0.02	ndp
 *
 * Dependent on:
 *	module function files
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sysdefs.h"

// Device prototypes
#include "core_functions.h"
#include "mod_led_status.h"
#include "mod_range_finder_US.h"
#include "mod_range_finder_IR.h"
#include "mod_dc_motor.h"


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
	{ MOD_LED_STATUS_ID, mod_led_status_init },
	{ MOD_RANGE_FINDER_US_ID, mod_range_finder_us_init },
	{ MOD_RANGE_FINDER_IR_ID, mrfir_init },
	{ MOD_DC_MOTOR_ID, mdm_init },
	{ CORE_FUNCTIONS_ID, core_init },
	{ 0, 0}
};

/*
 * Used by service.c :: service_all()
 */
const MOD_FUNCTION_ENTRY mod_service_table[] PROGMEM =
{
	{ MOD_LED_STATUS_ID, mod_led_status_service },
	{ MOD_RANGE_FINDER_US_ID, mod_range_finder_us_service },
	{ MOD_RANGE_FINDER_IR_ID, mrfir_service },
	{ MOD_DC_MOTOR_ID, mdm_service },
	{ CORE_FUNCTIONS_ID, core_service },
	{ 0, 0}
};

/*
 * Returned by mod_access_table[] for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_led_status_access[] PROGMEM =
{
	{ 0x01, mod_led_status_on },
	{ 0x02, mod_led_status_off },
	{ 0, 0 }
};

/*
 * Returned by mod_access_table[] for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_range_finder_us_access[] PROGMEM =
{
	{ 0x01, mod_range_finder_us_getRange },
	{ 0x02, mod_range_finder_us_getAllRange },
	{ 0x03, mod_range_finder_us_setMinimumRange },
	{ 0, 0 }
};

/*
 * Returned by mod_access_table[] for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_range_finder_ir_access[] PROGMEM =
{
	{ 0x01, mrfir_SendIR },
	{ 0x02, mrfir_SendAllIR },
	{ 0x03, mrfir_SetLimit },
	{ 0, 0 }
};

/*
 * Returned by mod_access_table[] for access functions specific to this device.
 * NOTE: This array has to be before the access table.
 */
const MOD_FUNCTION_ENTRY mod_dc_motor_access[] PROGMEM =
{
	{ 0x01, mdm_SetSpeed },
	{ 0x02, mdm_SetDirection },
	{ 0x03, mdm_ControlUpdate },
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
	{ MOD_LED_STATUS_ID, mod_led_status_access },			// table to all functions supported by mod_led_status.
	{ MOD_RANGE_FINDER_US_ID, mod_range_finder_us_access },	// table to all functions supported by mod_range_finder_us.
	{ MOD_RANGE_FINDER_IR_ID, mod_range_finder_ir_access },	// table to all functions supported by mod_range_finder_ir.
	{ MOD_DC_MOTOR_ID, mod_dc_motor_access },				// table to all functions supported by mod_dc_motor.
	{ CORE_FUNCTIONS_ID, core_access },						// table to all functions supported by device debug.
	{ 0, 0 }
};
