/*
* line_follow.c
*
* Created: 7/16/2016 10:23:11 PM
*  Author: Chip
*/


#define LF_MIDDLE	2000
#define MAX_MOTOR	127

#define P_COEF	10.0			// dividing
#define I_COEF	900.0			// dividing
#define D_COEF	1.0				// multiplying

typedef enum { LF_INIT, LF_CENTER, LF_STARTUP, LF_RUN, LF_STOP } LF_STATE;


/* Speed is -127 to +127. */
int	motor[4];

float		iError[4];
float		pErrorOld[4];

// Use Motor index 0-3 for motor[1:4]
int pid(int motor, uint16_t goal, uint16_t position)
{
	int	speed;
	int			error;

	float			pError;
	float			dError;
	float			pidError;
	
			// Pololu 3pi Robot User's Guide - 7.c pg 27 PID control (mostly)
			pError = (float)( ((int)position) - goal );
			dError = pError - pErrorOld;
			pidError = (int)( (pError/P_COEF) + (iError/I_COEF) + (dError*D_COEF) );

			iError += pError;
			pErrorOld = pError;

			error = (int)pidError;
			if( error > MAX_MOTOR )
			{
				error = MAX_MOTOR;
			}
			if( error < -MAX_MOTOR )
			{
				error = -MAX_MOTOR;
			}
			if( error < 0 )
			{
				speed = MAX_MOTOR + error;
			}
			else
			{
				speed = MAX_MOTOR - error;
			}

  return speed;
}
