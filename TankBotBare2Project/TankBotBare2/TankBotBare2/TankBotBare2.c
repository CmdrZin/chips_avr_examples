/*
 * TankBotBare2.c
 *
 * Created: 8/13/2015	0.01	ndp
 *  Author: Chip
 *
 * This is the main() file for the Tank Bot Bare ver 2 board.
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
				twiStuffRxBuf( 0xE1 );
				twiStuffRxBuf( 0x01 );
				twiStuffRxBuf( 0x01 );
				twiStuffRxBuf( 6 );
			}
		// DEBUG --
#endif
	}

}
