/*
 * mod_em_service.c
 *
 * Created: 1/13/2016 1:02:42 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "led_display.h"
#include "sysTimer.h"
#include "access.h"

#define EM_TEST_DELAY	50			// N * 10ms

typedef enum EM_STATES { EM_TEST, EM_SINGLE, EM_ANIM } EM_STATE;

EM_STATE mes_state;


/*
 * Initialize resources and variables for EM Service.
 */
void mod_em_service_init()
{
	// Set up variables
	mes_state = EM_TEST;

	// Set up hardware
	ld_init();

	// Load default Icon into display buffer.
	ld_loadIcon( 0, 0 );
}

/*
 * Periodic service support.
 * Called from 'fast' main loop.
 */
void mod_em_service_service()
{
	static uint8_t delay = EM_TEST_DELAY;
	static uint16_t iconCount = 0;
		
	if( GPIOR0 & (1<<DEV_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_1MS_TIC);
		ld_service();					// support LED display

		if( GPIOR0 & (1<<DEV_10MS_TIC) )
		{
			// ALL states use 10ms TIC. Wouldn't see and changes faster than 40ms anyway.
			GPIOR0 &= ~(1<<DEV_10MS_TIC);

			switch( mes_state )
			{
				case EM_TEST:
					if( --delay == 0 )
					{
						delay = EM_TEST_DELAY;
						ld_loadIcon( iconCount, 0 );
						if( ++iconCount == 100 )				// set to last defined icon
						{
							iconCount = 0;
							ld_loadIcon( iconCount, 0 );		// reset to default
							mes_state = EM_SINGLE;
						}
					}
				
					break;
					
				case EM_SINGLE:
					// Services I2C messages by access calls.
					break;
					
				case EM_ANIM:
				
					break;
			}
		}
	}
}

/*
 * Set Icon number to display.
 * Icon number comes from I2C message.
 * CMD: 01 N
 *		N = Icon index
 */
void mes_setIcon()
{
	uint8_t icon;

	icon = getMsgData(3);		// get Icon number

	// load icon into display buffer.
	ld_loadIcon( (uint16_t)icon, 0 );
}

/*
 * Loads a User Icon pattern of 8 bytes to display.
 * Icon pattern comes from I2C message.
 * CMD: 02 R0 R1 R2 R3 R4 R5 R6 R7
 *		Rx = Row data pattern
 */
void mes_loadIcon()
{
	uint8_t buff[8];

	mes_state = EM_SINGLE;			// switch to Single is User loads an Icon to display.

	for(int i=0; i<8; ++i)
	{
		buff[i] = getMsgData(3+i);
	}

	// load icon into display buffer.
	ld_directLoadIcon( buff );
}

/*
 * Loads a User Icon pattern of 8 bytes into FLASH or EEPROM.
 * Icon pattern comes from I2C message.
 * CMD: 02 LOC R0 R1 R2 R3 R4 R5 R6 R7
 *		LOC: 0xC0:0xFF	Up to 64 User Icon can be stored. Reserve space for them.
 *		Rx = Row data pattern
 */
void mes_storeIcon()
{
//	uint8_t icon;

//	icon = getMsgData(3);		// get Icon number

	// load data into FLASH or EEPROM.
	// TODO:
}

void mes_setAnimDelay()
{
	
}

/*
 * Limit of 15 steps in sequence due to message protocol.
 */
void mes_setAnimSequence()
{
//	uint8_t led;
		
//	led = getMsgData(3);		// get LED
	
}
