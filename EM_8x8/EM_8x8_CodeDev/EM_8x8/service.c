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