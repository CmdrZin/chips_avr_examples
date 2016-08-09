/*
 * mod_collection.c
 *
 * Created: 10/19/2015		0.01	ndp
 *  Author: Chip
 *
 * Measurement collection module. The general form can be used for any sensor scanning operation.
 * Data features are generated here. Peak, Power, Avg, Min, Max, etc.
 *
 * ADXL345 accelerometer.
 * Sample Rate: 200 Hz
 * Data: Two bytes Z-axis, (0x36)LSB (0x37)MSB
 * Processing:
 *		Combine two bytes into int16 val.
 *		if val < 0 then val = 0
 *		if val > peak then peak = val (retain peak for 30 sec then set peak = 0)
 *		
 *		
 */ 


#include <avr/io.h>

#include "sysTimer.h"
#include "mod_adxl345.h"
#include "ioLedsButtons.h"
#include "mod_UserIO.h"

#define MCO_PEAK_HOLD	100		// N * 1ms
#define MCO_SAMPLE_RATE	5		// N * 1ms..5ms -> 200Hz

int mco_LastVal;			// Last value collected.
int mco_Peak;				// current peak value.
int mco_DelayPeak;			// Delay time to hold peak value.
uint8_t mco_SampleDelay;	// Sample rate N * 1ms

uint8_t mco_buffer[4];		// data buffer

uint8_t mco_raw_count;

/*
 * Initialize User IO variables.
 */
void mco_init()
{
	mco_DelayPeak = MCO_PEAK_HOLD;
	mco_SampleDelay = MCO_SAMPLE_RATE;
}

/*
 * Sample ADXL
 * Record Last Value
 * Update Peak Value
 * Calculate Power Factor
 */
void mco_service()
{
	int temp;
	
	if( GPIOR0 & (1<<MCO_1MS_TIC) )
	{
		GPIOR0 &= ~(1<<MCO_1MS_TIC);
		
		if( --mco_SampleDelay == 0 )
		{
			mco_SampleDelay = MCO_SAMPLE_RATE;
			
			// Service ADXL
			// Read Z axis
			mod_adxl_readData( 0x36, mco_buffer );

			temp = ((int)mco_buffer[1]<<8) | mco_buffer[0];		// combine MSB with LSB
			if(temp < 0) temp = 0;
			
			mco_LastVal = temp;
//			mco_LastVal = 0x1234;

			if( mco_LastVal > mco_Peak )
			{
				mco_Peak = mco_LastVal;
				mco_DelayPeak = MCO_PEAK_HOLD;			// reset peak hold delay due to new value.
				idl_setPeakValue( mco_Peak );
				mco_raw_count = 20;
			}

#if 0
			if( mco_raw_count != 0 )
			{
				--mco_raw_count;
				// Dump raw data. Ten bytes at 38400 baud 8N1.
				muio_serial_raw();
			}
#endif

		}

		// Service Peak Delay
		if( mco_DelayPeak != 0 )
		{
			--mco_DelayPeak;

			if( mco_DelayPeak == 0)
			{
				mco_Peak = 10;
//				idl_setPeakValue( mco_Peak );
				idl_setPeakValue( 0 );
			}
		}

	}
}

/*
 * Get Current value
 */
int mco_GetLastVal()
{
	return( mco_LastVal );
}

/*
 * Get Peak value
 */
int mco_GetPeakVal()
{
	return( mco_Peak );
}
