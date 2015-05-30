/*
 * dev_keypad4x4.c
 *
 * Created: 5/22/2015 3:44:38 PM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "sysTimer.h"
// DEV_KP_10MS_TIC,

#include "usiTwiSlave.h"
// usiTwiTransmitByte(),

#include "dev_keypad4x4.h"
// DEV_KEYPAD_ID,

void dev_kp_setup_delay();

// ROW can be on any IO pin.
#define DEV_KP_ROW0_DDR  DDRB
#define DEV_KP_ROW0_PORT PORTB
#define DEV_KP_ROW0_PIN  PINB0

#define DEV_KP_ROW1_DDR  DDRB
#define DEV_KP_ROW1_PORT PORTB
#define DEV_KP_ROW1_PIN  PINB1

#define DEV_KP_ROW2_DDR  DDRB
#define DEV_KP_ROW2_PORT PORTB
#define DEV_KP_ROW2_PIN  PINB2

#define DEV_KP_ROW3_DDR  DDRA
#define DEV_KP_ROW3_PORT PORTA
#define DEV_KP_ROW3_PIN  PINA7

// COLS have to be on bits 0-3 of ONE port since they are read in as a group.
#define DEV_KP_COLS_DDR  DDRA
#define DEV_KP_COLS_PORT PORTA
#define DEV_KP_COLS_PINS PINA

uint8_t dev_kp_scan_count;				// number of scans so far.
uint8_t dev_kp_delay_count;				// delay count so far.

uint8_t dev_kp_scans;					// number of scans to do. Settable by command.
uint8_t dev_kp_scan_delay;				// delay between scans. N * 10ms. Settable by command.

uint8_t dev_kp_data_keys7_0;			// 0=up..1=down
uint8_t dev_kp_data_keysF_8;

uint8_t dev_kp_temp[4];					// cols of row0

/*
 * Initialize hardware for 4x4 keypad scanning.
 */
void dev_keypad4x4_init()
{
	// ROW DDR is set up during scan.

	DEV_KP_COLS_DDR &= 0xF0;						// set 0-3 as inputs
	DEV_KP_COLS_PORT |= 0x0F;						// enable pull-ups
	
	// set defaults
	dev_kp_scans = 3;								// OR successive scans.
	dev_kp_scan_count = dev_kp_scans;
	
	dev_kp_scan_delay = 1;							// N * 10ms.
	dev_kp_delay_count = dev_kp_scan_delay;
	
	// Clear temps. They collect 0s.
	dev_kp_temp[0] = 0xFF;
	dev_kp_temp[1] = 0xFF;
	dev_kp_temp[2] = 0xFF;
	dev_kp_temp[3] = 0xFF;

	return;
}

/*
 * Service keypad scan.
 * ROW pins can be any where. COL pins must be 0-3 on one PORT.
 * 1MS_TIC could be used to scan faster.
 */
void dev_keypad4x4_service()
{
	if( GPIOR0 & (1<<DEV_KP_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<DEV_KP_10MS_TIC);
		// check delay
		if( --dev_kp_delay_count == 0 )
		{
			dev_kp_delay_count = dev_kp_scan_delay;
			// 15us setup delay before reading line causes service to be ~65us.

			DEV_KP_ROW0_DDR |= (1<<DEV_KP_ROW0_PIN);		// enable pin
			DEV_KP_ROW0_PORT  &= ~(1<<DEV_KP_ROW0_PIN);		// clear ROW0
			// wait 15us
			dev_kp_setup_delay();
			dev_kp_temp[0] &= DEV_KP_COLS_PINS;				// read COLS
			DEV_KP_ROW0_DDR &= ~(1<<DEV_KP_ROW0_PIN);		// disable pin

			DEV_KP_ROW1_DDR |= (1<<DEV_KP_ROW1_PIN);		// enable pin
			DEV_KP_ROW1_PORT  &= ~(1<<DEV_KP_ROW1_PIN);		// clear ROW1
			// wait 15us
			dev_kp_setup_delay();
			dev_kp_temp[1] &= DEV_KP_COLS_PINS;				// read COLS
			DEV_KP_ROW1_DDR &= ~(1<<DEV_KP_ROW1_PIN);		// disable pin
			
			DEV_KP_ROW2_DDR |= (1<<DEV_KP_ROW2_PIN);		// enable pin
			DEV_KP_ROW2_PORT  &= ~(1<<DEV_KP_ROW2_PIN);		// clear ROW2
			// wait 15us
			dev_kp_setup_delay();
			dev_kp_temp[2] &= DEV_KP_COLS_PINS;				// read COLS
			DEV_KP_ROW2_DDR &= ~(1<<DEV_KP_ROW2_PIN);		// disable pin

			DEV_KP_ROW3_DDR |= (1<<DEV_KP_ROW3_PIN);		// enable pin
			DEV_KP_ROW3_PORT  &= ~(1<<DEV_KP_ROW3_PIN);		// clear ROW3
			// wait 15us
			dev_kp_setup_delay();
			dev_kp_temp[3] &= DEV_KP_COLS_PINS;				// read COLS
			DEV_KP_ROW3_DDR &= ~(1<<DEV_KP_ROW3_PIN);		// disable pin

			if( --dev_kp_scan_count == 0 )
			{
				dev_kp_scan_count = dev_kp_scans;		// reset count
				// process cols into key pattern
				dev_kp_data_keys7_0 = (~dev_kp_temp[0]) & 0x0F;				// ROWS use pins 0-3..ASSUME keys 3-0
				dev_kp_data_keys7_0 |= ( ((~dev_kp_temp[1]) & 0x0F) << 4 );	// ASSUMES key 7-4
				dev_kp_data_keysF_8 = (~dev_kp_temp[2]) & 0x0F;				// ROWS use pins 0-3..ASSUME keys 11-8
				dev_kp_data_keysF_8 |= ( ((~dev_kp_temp[3]) & 0x0F) << 4 );	// ASSUMES key 15-12
				// Clear temps.
				dev_kp_temp[0] = 0xFF;
				dev_kp_temp[1] = 0xFF;
				dev_kp_temp[2] = 0xFF;
				dev_kp_temp[3] = 0xFF;
			}
		}
	}
	return;
}


void dev_kp_setup_delay()
{
	uint8_t count;
	
	for( count=0; count < 20; count++ )
	{
		asm("nop");
	}
}


void dev_keypad4x4_get_data()
{
	usiTwiTransmitByte( DEV_KEYPAD_ID );
	usiTwiTransmitByte( dev_kp_data_keysF_8 );
	usiTwiTransmitByte( dev_kp_data_keys7_0 );
	return;
}


void dev_keypad4x4_set_scan_1()
{
	dev_keypad4x4_set_scan_num( 1 );
}

void dev_keypad4x4_set_scan_2()
{
	dev_keypad4x4_set_scan_num( 2 );
}

void dev_keypad4x4_set_scan_3()
{
	dev_keypad4x4_set_scan_num( 3 );
}

void dev_keypad4x4_set_scan_4()
{
	dev_keypad4x4_set_scan_num( 4 );
}

void dev_keypad4x4_set_scan_5()
{
	dev_keypad4x4_set_scan_num( 5 );
}

void dev_keypad4x4_set_scan_6()
{
	dev_keypad4x4_set_scan_num( 6 );
}

void dev_keypad4x4_set_scan_7()
{
	dev_keypad4x4_set_scan_num( 7 );
}

void dev_keypad4x4_set_scan_8()
{
	dev_keypad4x4_set_scan_num( 8 );
}

void dev_keypad4x4_set_scan_9()
{
	dev_keypad4x4_set_scan_num( 9 );
}

void dev_keypad4x4_set_scan_10()
{
	dev_keypad4x4_set_scan_num( 10 );
}

void dev_keypad4x4_set_scan_11()
{
	dev_keypad4x4_set_scan_num( 11 );
}

void dev_keypad4x4_set_scan_12()
{
	dev_keypad4x4_set_scan_num( 12 );
}

void dev_keypad4x4_set_scan_13()
{
	dev_keypad4x4_set_scan_num( 13 );
}

void dev_keypad4x4_set_scan_14()
{
	dev_keypad4x4_set_scan_num( 14 );
}

void dev_keypad4x4_set_scan_15()
{
	dev_keypad4x4_set_scan_num( 15 );
}

void dev_keypad4x4_set_scan_num( uint8_t scans )
{
	dev_kp_scans = scans;
}


void dev_keypad4x4_set_delay_1()
{
	dev_keypad4x4_set_scan_delay( 1 );
}

void dev_keypad4x4_set_delay_2()
{
	dev_keypad4x4_set_scan_delay( 2 );
}

void dev_keypad4x4_set_delay_3()
{
	dev_keypad4x4_set_scan_delay( 3 );
}

void dev_keypad4x4_set_delay_4()
{
	dev_keypad4x4_set_scan_delay( 4 );
}

void dev_keypad4x4_set_delay_5()
{
	dev_keypad4x4_set_scan_delay( 5 );
}

void dev_keypad4x4_set_delay_6()
{
	dev_keypad4x4_set_scan_delay( 6 );
}

void dev_keypad4x4_set_delay_7()
{
	dev_keypad4x4_set_scan_delay( 7 );
}

void dev_keypad4x4_set_delay_8()
{
	dev_keypad4x4_set_scan_delay( 8 );
}

void dev_keypad4x4_set_delay_9()
{
	dev_keypad4x4_set_scan_delay( 9 );
}

void dev_keypad4x4_set_delay_10()
{
	dev_keypad4x4_set_scan_delay( 10 );
}

void dev_keypad4x4_set_delay_11()
{
	dev_keypad4x4_set_scan_delay( 11 );
}

void dev_keypad4x4_set_delay_12()
{
	dev_keypad4x4_set_scan_delay( 12 );
}

void dev_keypad4x4_set_delay_13()
{
	dev_keypad4x4_set_scan_delay( 13 );
}

void dev_keypad4x4_set_delay_14()
{
	dev_keypad4x4_set_scan_delay( 14 );
}

void dev_keypad4x4_set_delay_15()
{
	dev_keypad4x4_set_scan_delay( 15 );
}

void dev_keypad4x4_set_scan_delay(uint8_t delay )
{
	dev_kp_scan_delay = delay;
}
