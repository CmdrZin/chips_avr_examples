/*
 * dev_solenoid.c
 *
 * Created: 6/12/2015 10:30:04 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "sysTimer.h"
// DEV_1MS_TIC,
#include "access.h"
// getCmdData(),

#include "dev_solenoid.h"

#define DEV_SOLENOID_1_PWM_DDR	DDRA
#define DEV_SOLENOID_1_PWM_PORT	PORTA
#define DEV_SOLENOID_1_PWM_P	PA0

#define DEV_SOLENOID_1_IN_DDR	DDRA
#define DEV_SOLENOID_1_IN_PORT	PORTA
#define DEV_SOLENOID_1_IN_P		PA1

#define DEV_SOLENOID_1_OUT_DDR	DDRA
#define DEV_SOLENOID_1_OUT_PORT	PORTA
#define DEV_SOLENOID_1_OUT_P	PA2

#define DEV_SOLENOID_2_PWM_DDR	DDRB
#define DEV_SOLENOID_2_PWM_PORT	PORTB
#define DEV_SOLENOID_2_PWM_P	PB2

#define DEV_SOLENOID_2_IN_DDR	DDRA
#define DEV_SOLENOID_2_IN_PORT	PORTA
#define DEV_SOLENOID_2_IN_P		PA3

#define DEV_SOLENOID_2_OUT_DDR	DDRA
#define DEV_SOLENOID_2_OUT_PORT	PORTA
#define DEV_SOLENOID_2_OUT_P	PA5

bool solenoid_1_swap_flag;			// reverse control pins
bool solenoid_2_swap_flag;

uint8_t solenoid_1_pulse_time;		// N * ms
uint8_t solenoid_2_pulse_time;

/*
 * Solenoid initialization.
 * Set IO for both solenoids.
 */
void dev_solenoid_init()
{
	solenoid_1_swap_flag = false;
	solenoid_2_swap_flag = false;

	solenoid_1_pulse_time = 20;		// N * ms
	solenoid_2_pulse_time = 20;

	// Reset default sets all IO LOW. This disables the H bridge driver.
	DEV_SOLENOID_1_PWM_PORT &= ~(1<<DEV_SOLENOID_1_PWM_P);
	DEV_SOLENOID_1_IN_PORT &= ~(1<<DEV_SOLENOID_1_IN_P);
	DEV_SOLENOID_1_OUT_PORT &= ~(1<<DEV_SOLENOID_1_OUT_P);
	
	DEV_SOLENOID_2_PWM_PORT &= ~(1<<DEV_SOLENOID_2_PWM_P);
	DEV_SOLENOID_2_IN_PORT &= ~(1<<DEV_SOLENOID_2_IN_P);
	DEV_SOLENOID_2_OUT_PORT &= ~(1<<DEV_SOLENOID_2_OUT_P);

	// Set IO as output
	DEV_SOLENOID_1_PWM_DDR |= (1<<DEV_SOLENOID_1_PWM_P);
	DEV_SOLENOID_1_IN_DDR |= (1<<DEV_SOLENOID_1_IN_P);
	DEV_SOLENOID_1_OUT_DDR |= (1<<DEV_SOLENOID_1_OUT_P);
	
	DEV_SOLENOID_2_PWM_DDR |= (1<<DEV_SOLENOID_2_PWM_P);
	DEV_SOLENOID_2_IN_DDR |= (1<<DEV_SOLENOID_2_IN_P);
	DEV_SOLENOID_2_OUT_DDR |= (1<<DEV_SOLENOID_2_OUT_P);
}

void dev_solenoid_service()
{
	// NO dynamic services at this time.
	return;
}


/* *** SOLENOID ACCESS FUNCTIONS *** */

/*
 * Command 0x01 - Pulse Solenoid 1
 * DEV 0x01
 */
void dev_solenoid_1_pulse()
{
	uint8_t count = solenoid_1_pulse_time;

	if (solenoid_1_swap_flag)
	{
		DEV_SOLENOID_1_OUT_PORT |= (1<<DEV_SOLENOID_1_OUT_P);		// AIN2 HIGH
		DEV_SOLENOID_1_IN_PORT &= ~(1<<DEV_SOLENOID_1_IN_P);		// AIN1 LOW
	}
	else
	{
		DEV_SOLENOID_1_IN_PORT |= (1<<DEV_SOLENOID_1_IN_P);			// AIN1 HIGH
		DEV_SOLENOID_1_OUT_PORT &= ~(1<<DEV_SOLENOID_1_OUT_P);		// AIN2 LOW
	}

	// Clear flag
	GPIOR0 &= ~(1<<DEV_1MS_TIC);
	// Wait for 1ms TIC to start pulse
	while( !(GPIOR0 & (1<<DEV_1MS_TIC)) );
	// Clear flag
	GPIOR0 &= ~(1<<DEV_1MS_TIC);
	// set pwm HIGH
	DEV_SOLENOID_1_PWM_PORT |= (1<<DEV_SOLENOID_1_PWM_P);
	// Wait for N 1ms TICs
	while(count != 0)
	{
		if( GPIOR0 & (1<<DEV_1MS_TIC) )
		{
			GPIOR0 &= ~(1<<DEV_1MS_TIC);
			--count;
		}
	}
	// set pwm LOW
	DEV_SOLENOID_1_PWM_PORT &= ~(1<<DEV_SOLENOID_1_PWM_P);
}

/*
 * Command 0x02 - Swap direction for Solenoid 1
 * DEV 0x02
 */
void dev_solenoid_1_toggle_swap()
{
	solenoid_1_swap_flag = !solenoid_1_swap_flag;
}

/*
 * Command 0x03 - Set Pulse time for Solenoid 1. N * ms.
 * DEV 0x03 MS
 */
void dev_solenoid_1_set_pulse()
{
	solenoid_1_pulse_time = getCmdData(2);
}


/*
 * Command 0x01 - Pulse Solenoid 2
 * DEV 0x01
 */
void dev_solenoid_2_pulse()
{
	uint8_t count = solenoid_2_pulse_time;

	if (solenoid_2_swap_flag)
	{
		DEV_SOLENOID_2_OUT_PORT |= (1<<DEV_SOLENOID_2_OUT_P);		// BIN2 HIGH
		DEV_SOLENOID_2_IN_PORT &= ~(1<<DEV_SOLENOID_2_IN_P);		// BIN1 LOW
	}
	else
	{
		DEV_SOLENOID_2_IN_PORT |= (1<<DEV_SOLENOID_2_IN_P);			// BIN1 HIGH
		DEV_SOLENOID_2_OUT_PORT &= ~(1<<DEV_SOLENOID_2_OUT_P);		// BIN2 LOW
	}

	// Clear flag
	GPIOR0 &= ~(1<<DEV_1MS_TIC);
	// Wait for 1ms TIC to start pulse
	while( !(GPIOR0 & (1<<DEV_1MS_TIC)) );
	// Clear flag
	GPIOR0 &= ~(1<<DEV_1MS_TIC);
	// set pwm HIGH
	DEV_SOLENOID_2_PWM_PORT |= (1<<DEV_SOLENOID_2_PWM_P);
	// Wait for N 1ms TICs
	while(count != 0)
	{
		if( GPIOR0 & (1<<DEV_1MS_TIC) )
		{
			GPIOR0 &= ~(1<<DEV_1MS_TIC);
			--count;
		}
	}
	// set pwm LOW
	DEV_SOLENOID_2_PWM_PORT &= ~(1<<DEV_SOLENOID_2_PWM_P);
}

/*
 * Command 0x02 - Swap direction for Solenoid 2
 * DEV 0x02
 */
void dev_solenoid_2_toggle_swap()
{
	solenoid_2_swap_flag = !solenoid_2_swap_flag;
}

/*
 * Command 0x03 - Set Pulse time for Solenoid 2. N * ms.
 * DEV 0x03 MS
 */
void dev_solenoid_2_set_pulse()
{
	solenoid_2_pulse_time = getCmdData(2);
}
