/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * mod_dc_motor.c
 *
 * Created: 11/6/2015 11:18:03 AM
 *  Author: Chip
 * revised: 04/16/2019		0.10	ndp	for use by Motor Slave.
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "mod_dc_motor.h"
#include "pwm_dc_motor.h"
#include "mod_quad_decode.h"
//
#include "serial.h"
//
char bin2hex(char val);


#define MDM_SERVICE_WAIT	100L			// N * ms wait

// ref: Pololu 3pi Robot User's Guide - 7.c pg 27 PID control (mostly)
#define P_COEF		1.0			// dividing 10.0
#define I_COEF		900.0			// dividing
#define D_COEF		1.0				// multiplying
#define MAX_MOTOR	255				// absolute value for 8bit PWM max. Direction controlled from +/- error.

bool mdm_disableAuto;				// Switch OFF is giving manual Speed/Direction commands. ON when Position commands received.
long mdm_service_time;

// Motor tables
uint8_t nextPositionHB[4];			// Save first byte of Position message.
uint16_t nextPosition[4];

uint16_t currentPosition[4];

/*
 * mdm_init()
 *
 * Set up variables.
 */
void mdm_init()
{
	mdm_disableAuto = false;
	mdm_service_time = MDM_SERVICE_WAIT;
	pwm_dc_motor_init();
}

/*
 * mdm_service()
 *
 * Manage motors. Call continuously. Check for updates within time limit.
 *
 */
void mdm_service()
{
	int speed;
	uint8_t direction;

	if(!mdm_disableAuto)
	{
		if( mdm_service_time < st_millis() )
		{
			// Update Motor Control PWM/Control with nextPosition and currentPosistion.
			for(int i=0; i<4; i++)
			{
				currentPosition[i] = mqd_getPosition(i);
				speed = mdm_move( i, nextPosition[i], currentPosition[i] );
				// Assume that + is CW and - is CCW
				direction = (speed < 0) ? PWM_CCW : PWM_CW;
				// Use abs(speed)
				if(speed < 0)
				{
					speed = -speed;
				}
				// Adjust index 0:3 to motor number 1:4.
				pwm_set_speed(i+1, speed);
				pwm_set_dir(i+1, direction);
			}
		}
	}
}

// Manual Speed set. motor 1:4.
void mdm_SetSpeed(uint8_t motor, uint8_t speed)
{
	// Ignore invalid motor numbers.
	if( (motor > 0) && (motor < 5) ) {
		mdm_disableAuto = true;					// don't let Auto service override manual command.

		pwm_set_speed(motor, speed);
	}
}

// Manual Direction Set. motor 1:4.
void mdm_SetDirection(uint8_t motor, uint8_t direction)
{
	PWM_DIR_CONTROL dir;

	// Ignore invalid motor numbers.
	if( (motor > 0) && (motor < 5) ) {
		mdm_disableAuto = true;					// don't let Auto service override manual command.

		dir = ( ( direction == 0 ) ? PWM_CW : PWM_CCW );
		pwm_set_dir(motor, dir);
	}
}


/*
 * Set Position High Byte.
 * Motor values: 1,2,3,4 so use MV-1 and ignore result if <= 0.
 */
void mdm_SetPositionHB(uint8_t motor, uint8_t data)
{
	uint8_t m;

	// Ignore invalid motor numbers.
	if( (motor > 0) && (motor < 5) ) {
		m = motor - 1;
		nextPositionHB[m] = data;
	}
}

/*
 * Set Position Low Byte.
 * Motor values: 1,2,3,4 so use MV-1 and ignore result if <= 0.
 */
void mdm_SetPositionLB(uint8_t motor, uint8_t data)
{
	uint8_t m;

	mdm_disableAuto = false;				// let Auto service control system.

	// Ignore invalid motor numbers.
	if( (motor > 0) && (motor < 5) ) {
		m = motor - 1;
		nextPosition[m] = ( ((uint16_t)nextPositionHB[m]) << 8 ) + data;
	}
}


/*
 * Simple PID controller to adjust speed and move to next position.
 * motorIndex:	Motor array index: 0:3 derived from Motor number: 1:4.
 * endPosition:	target position to move to.
 * position:	current motor position. 0x0000:0xFFFF
 *
 * error:		speed value +/- 255
 */
float mdm_move(uint8_t motorIndex, float endPosition, float position)
{
	static float pErrorOld[4] = {0.0, 0.0, 0.0, 0.0};			// last error value

	float pError;			// Proportional error
	float iError = 0;		// Integration error
	float dError;			// Differential error

	float pidError;			// Total PID error
	int error;				// final error value for PWM speed.

	// Calculate the Proportional error from where we are to where we want to be.
	pError = (float)( position - endPosition );
	// Accumulated the Differential errors
	dError = pError - pErrorOld[motorIndex];
	// Run the PID equation.
	pidError = (int)( (pError/P_COEF) + (iError/I_COEF) + (dError*D_COEF) );

	// Accumulate the Integration error.
	iError += pError;
	// Update the Proportional error history.
	pErrorOld[motorIndex] = pError;

	// Limit the error value that is sent to the motor PWM control.
	error = (int)pidError;
	if( error > MAX_MOTOR )
	{
		error = MAX_MOTOR;
	}
	if( error < -MAX_MOTOR )
	{
		error = -MAX_MOTOR;
	}

	return( error );
}


// Motor HOME operation
void mdm_motorHome(uint8_t motor)
{
	uint8_t motorIndex;

	// Ignore invalid motor numbers.
	if( (motor > 0) && (motor < 5) ) {
		// Switch to motorIndex.
		motorIndex = motor - 1;
		// check limit switch
		// move a little away then back until limit detected.
		// move back slow until no limit then set as HOME.
		mqd_setHome(motorIndex);
	}
}
