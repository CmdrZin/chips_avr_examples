/*
 * mod_UserIO.c
 *
 * Created: 8/30/2015 7:30:47 PM
 *  Author: Chip
 */ 


#include <avr/io.h>

#include "sysTimer.h"
#include "mod_UserIO.h"
#include "io7SegLedsButtons.h"
#include "twi_I2CMaster.h"
#include "mod_adxl345.h"
#include "mod_collection.h"

#define MUIO_DISPLAY_DELAY	10		// N * 100ms
#define MUIO_TEST_DELAY		2		// N * 100ms
#define MUIO_NORMAL_DELAY	10		// N * 100ms
#define MUIO_RANGE_DELAY	30		// N * 100ms


uint8_t muio_stateDelay;

uint8_t	muio_StatisLEDs;			// b1:POWER, b0:PEAK	1:ON, 0:OFF
uint8_t	muio_PowerVal;				// Value to display as solid bar up to value. 1:20, 0:OFF
uint16_t muio_PeakVal;				// Value to display as PEAK.
uint16_t muio_PeakDisplay;			// Converted to BCD.

uint8_t muio_count;
MUIO_DISPLAY_MODE muio_mode;
uint8_t muio_modeState;

uint8_t muio_normalState;
uint8_t muio_normalDelay;
uint8_t muio_decimalPattern;

uint8_t muio_rangeValue;
uint8_t muio_rangeState;
uint8_t muio_rangeDelay;

bool buttonFlag;

uint8_t muio_buffer[8];

int	muio_DisplayDelay;				// Display switch delay		N * 10ms

/*
 * Initialize User IO variables.
 */
void uio_init()
{
	muio_StatisLEDs = 0;
	muio_PowerVal = 0;
	muio_PeakVal = 0;

	muio_count = 0;
	muio_mode = MUIO_TEST;
	muio_rangeValue = 1;

	muio_modeState = 0;
	muio_DisplayDelay = MUIO_TEST_DELAY;
	
	muio_normalState = 0;
}

/*
 * Call all of the sub-services to update.
 * Each sub-service manages their own display delay.
 */
void uio_service()
{
	uint8_t scan;
	
	if( GPIOR0 & (1<<DEV_100MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_100MS_TIC);
		
		switch( muio_mode )
		{
			case MUIO_TEST:
				muio_testMode();
				break;

			case MUIO_RAW_SERIAL:
				break;
				
			case MUIO_NORMAL:
				muio_normal();
				break;
				
			case MUIO_RANGE:
				muio_range();
				break;
				
			default:
				muio_mode = MUIO_TEST;
				break;
		}

		// Service activity decimal points
		if( --muio_normalDelay == 0 )
		{
			if( muio_decimalPattern != 0 )
			{
				muio_decimalPattern >>= 1;				// shift right
			}
			else
			{
				muio_decimalPattern = 0x08;
			}
			idl_setDecimalPoints( muio_decimalPattern );
			muio_normalDelay = MUIO_NORMAL_DELAY;	// n * 100ms
		}

		// Buttons scan every 100ms. b1:UP, b0:DN
		scan = idl_scanButtons();
		if( scan == 0 ) {
			buttonFlag = false;
		} else {
			// NOTE: This can be changed to check for BOTH pressed.
			if( !buttonFlag && (scan == 0x02)) {		// UP
				if( (muio_rangeValue < 4) && (muio_mode == MUIO_RANGE) ) {
					++muio_rangeValue;
					mod_adxl_setRange( muio_rangeValue );
				}
				buttonFlag = true;
				muio_mode = MUIO_RANGE;
				muio_rangeState = 1;
			}
			if( !buttonFlag && (scan == 0x01)) {		// DN
				if( (muio_rangeValue > 1) && (muio_mode == MUIO_RANGE) ) {
					--muio_rangeValue;
					mod_adxl_setRange( muio_rangeValue );
				}
				buttonFlag = true;
				muio_mode = MUIO_RANGE;
				muio_rangeState = 1;
			}
		}
	}
}

/*
 * Set current display mode.
 */
void uio_setMode( MUIO_DISPLAY_MODE mode )
{
	muio_mode = mode;
}

/*
 * Called at 100ms TIC
 */
void muio_testMode()
{
	uint16_t temp;
	
	if( muio_stateDelay != 0 )
	{
		--muio_stateDelay;
	}
	else
	{
		switch( muio_modeState )
		{
			case 0:
				muio_count = 0;
				idl_setValue( (uint16_t)muio_count );
				idl_setMode( IDL_DL1 );
				muio_modeState = 1;
				muio_stateDelay = MUIO_TEST_DELAY;	// n * 100ms
				break;
			
			case 1:
				if( ++muio_count <= 9)
				{
					idl_setValue( (uint16_t)muio_count );
				}
				else
				{
					muio_modeState = 2;
					muio_count = 0;
					idl_setValue( (uint16_t)muio_count );
				}
				muio_stateDelay = MUIO_TEST_DELAY;	// n * 100ms
				break;

			case 2:
				if( ++muio_count <= 9)
				{
					temp = (muio_count) << 8;
					idl_setValue( temp );
				}
				else
				{
					muio_modeState = 0;
					muio_mode = MUIO_NORMAL;
					muio_normalState = 1;
					muio_decimalPattern = 0x0F;
					mod_adxl_init();	// Initialize ADXL
				}
				muio_stateDelay = MUIO_TEST_DELAY;	// n * 100ms
				break;

			default:
				muio_modeState = 0;
				break;
		}
	}
}

/*
 * Val is a 10bit signed value [-512 -- +511]
 */
void muio_setPeak( uint16_t val )
{
	uint16_t temp;
	uint16_t acc;
	
	muio_PeakVal = val;
	
	// Convert to 2 byte BCD 0-99%. val/512 * 100. (val * 100) / 512. (val * 25) / 128.
	temp = (val << 2) + val;				// x5
	acc = (temp << 2) + temp;				// x5
	acc >>= 7;								// /128

	temp = acc % 10;
	acc = acc / 10;
	
	muio_PeakDisplay = (acc << 8) | temp;
}

/*
 * Called at 100ms TIC
 */
void muio_normal()
{
	switch( muio_normalState )
	{
		case 1:
			idl_setMode(IDL_DL1);
			idl_setValue( 0x1010 );				// BLANK = 0x10
			idl_setDecimalPoints( muio_decimalPattern );
			muio_normalDelay = MUIO_NORMAL_DELAY;	// n * 100ms
			muio_normalState = 2;
			break;
			
		case 2:
			// mod_collection will set this to 0 if no peak to display.
			// Peak value is controlled by mod_collection.c
			if( muio_PeakVal != 0 )
			{
				idl_setValue( muio_PeakDisplay );
			}
			else
			{
				idl_setValue( 0x1010 );				// BLANK = 0x10
			}
			break;
	}

}

/*
 * Called at 100ms TIC
 */
void muio_range()
{
	switch( muio_rangeState )
	{
		// Set up controls
		case 1:
			idl_setValue( 0x1400 | (uint16_t)muio_rangeValue );		// rN
			muio_rangeDelay = MUIO_RANGE_DELAY;		// n * 100ms
			muio_rangeState = 2;
			break;

		// Time out delay
		case 2:
			if( --muio_rangeDelay == 0 )
			{
				muio_mode = MUIO_NORMAL;
				muio_normalState = 1;
				muio_setPeak(0);			// clear old value
				break;
			}
	}
}

void muio_powerMode()
{
	// Nothing to do for now.
}
