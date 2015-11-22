/*
 * mod_dc_motor.c
 *
 * Created: 11/6/2015 11:18:03 AM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "sysTimer.h"
#include "mod_dc_motor.h"
#include "pwm_dc_motor.h"
#include "access.h"

#define MDM_MAX_WAIT	250

uint8_t mdm_wait;


/*
 * mdm_init()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * Set up variables.
 */
void mdm_init()
{
	mdm_wait = MDM_MAX_WAIT;
	pwm_dc_motor_init();
}


/*
 * mdm_service()
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * Manage motors. Check for updates within time limit.
 *
 */
void mdm_service()
{
	if( GPIOR0 & (1<<MTR_10MS_TIC) )
	{
		GPIOR0 &= ~(1<<MTR_10MS_TIC);
		
		if( --mdm_wait == 0 )
		{
			// Turn off motors. This wait time is reset with any motor command.
			pwm_set_left_dir(PWM_STOP);
			pwm_set_right_dir(PWM_STOP);
			pwm_set_left(PWM_L_STOP);
			pwm_set_right(PWM_R_STOP);
			mdm_wait = MDM_MAX_WAIT;
		}
	}
}

/*
 * Set Speed
 * HDR SDA_W ID 01 N DL DH
 */
void mdm_SetSpeed()
{
	MDM_MOTOR_SELECT select;
	uint16_t speed;
	
	select = getMsgData(3);

	speed = getMsgData(5);					// DH
	speed = (speed<<8) | getMsgData(4);		// DL
	
	switch( select )
	{
		case MDM_MOTOR_LEFT:
			pwm_set_left(speed);
			break;

		case MDM_MOTOR_RIGHT:
			pwm_set_right(speed);
			break;

		case MDM_MOTOR_BOTH:
			pwm_set_left(speed);
			pwm_set_right(speed);
			break;
			
		default:		// ignore
			break;
	}

	mdm_wait = MDM_MAX_WAIT;
}

/*
 * Set Direction
 * HDR SDA_W ID 03 N D
 */
void mdm_SetDirection()
{
	MDM_MOTOR_SELECT select;
	uint16_t dir;
	
	select = getMsgData(3);

	dir = getMsgData(4);
	
	switch( select )
	{
		case MDM_MOTOR_LEFT:
			pwm_set_left_dir(dir);
			break;

		case MDM_MOTOR_RIGHT:
			pwm_set_right_dir(dir);
			break;

		case MDM_MOTOR_BOTH:
			pwm_set_left_dir(dir);
			pwm_set_right_dir(dir);
			break;
			
		default:		// ignore
			break;
	}

	mdm_wait = MDM_MAX_WAIT;
}

/*
 * Set Update
 * HDR SDA_W ID 03
 * Send if no motor control command sent in the last 2 seconds.
 */
void mdm_ControlUpdate()
{
	mdm_wait = MDM_MAX_WAIT;
}
