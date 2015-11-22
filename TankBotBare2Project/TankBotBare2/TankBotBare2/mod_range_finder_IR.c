/*
 * mod_range_finder_IR.c
 *
 * Created: 10/20/2015		0.01	ndp
 *  Author: Nels "Chip" Pearson
 *
 * Optical IR Range Sensor Service
 *
 * Target: Tank Bot Bare Board, 20MHz, ATmega164P
 *
 * This service supports four IR GP2Y0A41SK0F range sensors.
 * The sensors use power control to help reduce power use which is 20ma per sensor.
 *
 * Uses PORT A
 *
 * Dependencies
 *   sysTimers
 *
 */

#include <avr/io.h>

#include "mod_range_finder_IR.h"
#include "sysTimer.h"
#include "mod_adc.h"
#include "access.h"
#include "twiSlave.h"
#include "mod_led_status.h"


#define MRFIR_LR_LED		10-8	// D12
#define MRFIR_LF_LED		11-8	// D13
#define MRFIR_RF_LED		9-8		// D14
#define MRFIR_RR_LED		8-8		// D14

typedef enum { MRFIR_WAIT_IDLE, MRFIR_WAIT_LEFT_R, MRFIR_WAIT_LEFT_F, MRFIR_WAIT_RIGHT_F, MRFIR_WAIT_RIGHT_R } MRFIR_STATE;

#define IR_LED_WAIT_DELAY	30		// N * 10ms
#define IR_LED_ON_TIME	5			// N * 10ms

// IO Pin definitions .. IR Range uses PORTA.0:7
#define MRFIR_DDR	DDRA
#define MRFIR_PORT	PORTA

#define IR_LEFT_FRONT_SIG	PORTA0
#define IR_LEFT_REAR_SIG	PORTA1
#define IR_RIGHT_REAR_SIG	PORTA2
#define IR_RIGHT_FRONT_SIG	PORTA3

#define IR_LEFT_FRONT_CTRL	PORTA4
#define IR_LEFT_REAR_CTRL	PORTA5
#define IR_RIGHT_REAR_CTRL	PORTA6
#define IR_RIGHT_FRONT_CTRL	PORTA7


uint8_t mrf_ir_range[4];				// 10cm = 255..30cm = 20	0:LF 1:LR 2:RR 3:RF
uint8_t mrf_ir_limit[4];				// 10cm = 255..30cm = 20	0:LF 1:LR 2:RR 3:RF

MRFIR_STATE mrf_ir_state;
uint8_t mrf_ir_delay;					// N * 10ms


/*
 * Initialize Optical Range Sensor
 *
 */
void mrfir_init()
{
	// Disable all
	MRFIR_PORT |= (1<<IR_LEFT_REAR_CTRL);
	MRFIR_PORT |= (1<<IR_LEFT_FRONT_CTRL);
	MRFIR_PORT |= (1<<IR_RIGHT_FRONT_CTRL);
	MRFIR_PORT |= (1<<IR_RIGHT_REAR_CTRL);

	// Set as Output
	MRFIR_DDR |= (1<<IR_LEFT_REAR_CTRL);
	MRFIR_DDR |= (1<<IR_LEFT_FRONT_CTRL);
	MRFIR_DDR |= (1<<IR_RIGHT_FRONT_CTRL);
	MRFIR_DDR |= (1<<IR_RIGHT_REAR_CTRL);

	mrf_ir_state = MRFIR_WAIT_IDLE;
	mrf_ir_delay = IR_LED_WAIT_DELAY;

	// Set default range limits
	mrf_ir_limit[0] = 240;
	mrf_ir_limit[1] = 240;
	mrf_ir_limit[2] = 240;
	mrf_ir_limit[3] = 240;

	// Init ADC
	mod_adc_init();
}

/*
 * mrfir_service()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * Four ADC channels for IR distance.
 * Four I/O lines for sensor power control.
 * Set up ADC to trigger after LED turn-on. 30ms cycle.
 * Cycle through sensors.
 *
 * Process
 * 0. Wait IDLE 300ms then Light Left Front LED for 30ms
 * 1. Wait 30ms then Sample Left Front Detector, Light Left Rear LED for 30ms
 * 2. Wait 30ms then Sample Left Rear Detector, Light Right Front LED for 30ms
 * 3. Wait 30ms then Sample Right Front Detector, Light Right Rear LED for 30ms
 * 4. Wait 30ms then Sample Right Rear Detector, Set IDLE delay for 300ms
 *
 */
void mrfir_service()
{
	if( GPIOR0 & (1<<MIR_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<MIR_10MS_TIC);
		
		if( --mrf_ir_delay == 0 )
		{
			switch( mrf_ir_state )
			{
				case MRFIR_WAIT_IDLE:
					MRFIR_PORT &= ~(1<<IR_LEFT_REAR_CTRL);						// turn ON LR LED
					mrf_ir_state = MRFIR_WAIT_LEFT_R;
					mrf_ir_delay = IR_LED_ON_TIME;
					break;
					
				case MRFIR_WAIT_LEFT_R:
					// Trigger ADC
					mrf_ir_range[MRFIR_LEFT_R] = mod_adc_read(MRFIR_LEFT_R);	// save range data

					mls_setRangeStatus(MRFIR_LR_LED, (mrf_ir_range[MRFIR_LEFT_R] > mrf_ir_limit[MRFIR_LEFT_R]) );	// set status LED

					MRFIR_PORT |= (1<<IR_LEFT_REAR_CTRL);						// turn OFF LR LED
					MRFIR_PORT &= ~(1<<IR_LEFT_FRONT_CTRL);						// turn ON LF LED
					mrf_ir_state = MRFIR_WAIT_LEFT_F;
					mrf_ir_delay = IR_LED_ON_TIME;
					break;

				case MRFIR_WAIT_LEFT_F:
					// Trigger ADC
					mrf_ir_range[MRFIR_LEFT_F] = mod_adc_read(MRFIR_LEFT_F);	// save range data

					mls_setRangeStatus(MRFIR_LF_LED, (mrf_ir_range[MRFIR_LEFT_F] > mrf_ir_limit[MRFIR_LEFT_F]) );	// set status LED

					MRFIR_PORT |= (1<<IR_LEFT_FRONT_CTRL);						// turn OFF LF LED
					MRFIR_PORT &= ~(1<<IR_RIGHT_FRONT_CTRL);					// turn ON RF LED
					mrf_ir_state = MRFIR_WAIT_RIGHT_F;
					mrf_ir_delay = IR_LED_ON_TIME;
					break;

				case MRFIR_WAIT_RIGHT_F:
					// Trigger ADC
					mrf_ir_range[MRFIR_RIGHT_F] = mod_adc_read(MRFIR_RIGHT_F);	// save range data

					mls_setRangeStatus(MRFIR_RF_LED, (mrf_ir_range[MRFIR_RIGHT_F] > mrf_ir_limit[MRFIR_RIGHT_F]) );	// set status LED

					MRFIR_PORT |= (1<<IR_RIGHT_FRONT_CTRL);						// turn OFF RF LED
					MRFIR_PORT &= ~(1<<IR_RIGHT_REAR_CTRL);						// turn ON RR LED
					mrf_ir_state = MRFIR_WAIT_RIGHT_R;
					mrf_ir_delay = IR_LED_ON_TIME;
					break;

				case MRFIR_WAIT_RIGHT_R:
					// Trigger ADC
					mrf_ir_range[MRFIR_RIGHT_R] = mod_adc_read(MRFIR_RIGHT_R);	// save range data

					mls_setRangeStatus(MRFIR_RR_LED, (mrf_ir_range[MRFIR_RIGHT_R] > mrf_ir_limit[MRFIR_RIGHT_R]) );	// set status LED

					MRFIR_PORT |= (1<<IR_RIGHT_REAR_CTRL);						// turn OFF RR LED
					mrf_ir_delay = IR_LED_WAIT_DELAY;
					mrf_ir_state = MRFIR_WAIT_IDLE;
					break;
			}
		}
	}
}

/*
 * Send IR Range Data
 * E1 ID 01 N
 * N: 0-3
 */
void mrfir_SendIR()
{
	uint8_t sensor;
	
	sensor = getMsgData(3);
	if( sensor > 3) sensor = 0;

	twiTransmitByte( MOD_RANGE_FINDER_IR_ID );
	twiTransmitByte( mrf_ir_range[sensor] );

}

/*
 * Send ALL IR Range Data
 */
void mrfir_SendAllIR()
{
//	static uint8_t count = 0;
//	mrf_ir_range[1] = ++count;
	
	twiTransmitByte( MOD_RANGE_FINDER_IR_ID );
	twiTransmitByte( mrf_ir_range[0] );
	twiTransmitByte( mrf_ir_range[1] );
	twiTransmitByte( mrf_ir_range[2] );
	twiTransmitByte( mrf_ir_range[3] );
}

/*
 * Set Range Limit
 */
void mrfir_SetLimit()
{
	uint8_t sensor;
	uint8_t limit;
	
	sensor = getMsgData(3);
	if( sensor > 3) sensor = 0;

	limit = getMsgData(4);
	// Set data
	mrf_ir_limit[ sensor ] = limit;
}
