/*
 * mod_em_service.c
 *
 * Created: 1/13/2016	0.01	ndp
 *  Author: Chip
 * revision: 2/01/2016	0.02	ndp	Added marquee support
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "led_display.h"
#include "sysTimer.h"
#include "access.h"
#include "flash_table.h"

extern const ICON_DATA icons[];

/* Local functions */
void marquee_init();
void marquee_service();


#define EM_TEST_DELAY	50			// N * 10ms

typedef enum EM_STATES { EM_TEST, EM_SINGLE, EM_ANIM, EM_MARQUEE } EM_STATE;

EM_STATE mes_state;

// Marquee support
uint8_t	mes_series[16];		// holds the icon list to be displayed.
uint8_t	mes_mkBuf[4][8];	// marquee buffer that is shifted and copied into the display buffer.
bool	mes_kerning;		// enable kerning of icons.
bool	mes_continuous;		// loops the series continuously.
uint8_t	mes_rate;			// scroll rate. N * FRS. Default = 1.
uint8_t mes_seriesLength;	// number of icons in marquee series.

uint8_t	mar_iconCount;		// current icon index
uint8_t	mar_shiftCount;		// current shift

/*
 * Initialize resources and variables for EM Service.
 */
void mod_em_service_init()
{
	// Set up variables
	mes_state = EM_TEST;
	mes_rate = 5;
	mes_kerning = false;
	mes_continuous = false;
	mes_seriesLength = 0;
	
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

				case EM_MARQUEE:
					// Marquee uses the display frame sync for timing.
					if( ld_getFramesDisplayed() >= mes_rate )
					{
						marquee_service();
					}
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

/* *** MARQUEE SUPPORT *** */
/*
 * Select Marquee display. Use current flags and rate settings.
 * CMD: F0 20 40
 */
void mes_selectMarquee()
{
	marquee_init();
	mes_state = EM_MARQUEE;
}

/*
 * Set Kerning flag
 * CMD: E1 20 41 KRN	Set Kerning flag. 0:OFF, 1:ON
 */
void mes_setKerningFlag()
{
	if( getMsgData(3) == 0)	
	{
		mes_kerning = false;
	}
	else
	{
		mes_kerning = true;
	}
}

/*
 * Set Continuous flag
 * CMD: E1 20 42 CON	Set Continuous scroll flag. 0:OFF, 1:ON
 */
void mes_setContinuousFlag()
{
	if( getMsgData(3) == 0)	
	{
		mes_continuous = false;
	}
	else
	{
		mes_continuous = true;
	}
}

/*
 * Set Scroll Rate
 * CMD: E1 20 43 RATE	Set scroll Rate. N * FRS. 01-FF.
 */
void mes_setRate()
{
	mes_rate = getMsgData(3);
}

/*
 * Limit of 15 steps in sequence due to message protocol.
 * CMD: ?? 44 R0 ... RN (max = 15)
 */
void mes_setIconSeries()
{
	uint8_t index;
	
	mes_seriesLength = getMsgData(0) & 0x0F;	// get length
	
	for( index=0; index < mes_seriesLength; ++index )
	{
		mes_series[index] = getMsgData(3+index);
	}
}


/*
 * Set up Marquee display. Use current flags and rate settings.
 */
void marquee_init()
{
	uint8_t i;
	
	if( mes_seriesLength == 0 ) return;
	
	mar_iconCount = 0;
	mar_shiftCount = 8;

	// Get first icon
	flash_copy8( (uint16_t)mes_series[0], icons, mes_mkBuf[0] );
	++mar_iconCount;

	if( mar_iconCount < mes_seriesLength )
	{
		// Copy next icon
		flash_copy8( (uint16_t)mes_series[1], icons, mes_mkBuf[1] );
		++mar_iconCount;
	}
	else
	{
		// or set to 0
		for( i=0; i<8; ++i )
		{
			mes_mkBuf[1][i] = 0;
		}
	}
}

/*
 * Called each time the display is ready to be updated.
 * Loads in a new frame to scroll the icons across the display.
 */
void marquee_service()
{
	uint8_t i;
	uint8_t data;
	
	// Copy mkBuf[] into display buffer.
	ld_directLoadIcon( mes_mkBuf[0] );
	
	// Shift mkBuf[1] into mkBuf[0]
	for(i=0; i<8; ++i)
	{
		data = mes_mkBuf[1][i];
		mes_mkBuf[1][i] <<= 1;
		mes_mkBuf[0][i] <<= 1;
		if( data & 0x80 )
		{
			mes_mkBuf[0][i] |= 0x01;
		}
	}
	
	if(--mar_shiftCount == 0)
	{
		if(mar_iconCount == mes_seriesLength)
		{
			mes_state = EM_SINGLE;
			ld_loadIcon(0, 0);
			return;							// EXIT
		}
		if(mar_iconCount < mes_seriesLength)
		{
			// Get next icon
			flash_copy8( (uint16_t)mes_series[mar_iconCount], icons, mes_mkBuf[1] );
			++mar_iconCount;
		}

		mar_shiftCount = 8;
	}
}
