/*
 * pwm_support.c
 *
 * ref1: ATmega164P_datasheet.pdf Section 13, pg. 111-138
 * NOTE:	16bit write sequence is High then Low
 *			16bit read sequence is Low then High
 *
 * Use OC1A (PD5): Right side
 * Use OC1B (PD4): Left side
 *
 * Use 20ms total period for Timer1
 * Use 1/20 (1.0ms) for CCW
 * Use 3/40 (1.5ms) for STOP
 * Use 1/10 (2.0ms)for CW
 *
 * Created: 7/26/2016 2:27:19 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "pwm_support.h"

uint16_t speed_left;
uint16_t speed_right;


/*
 * Using Timer1 in PWM mode 14.
 * 20 MHz	 0.05us		  3.2768ms
 *  /8		 0.40us		 26.2144ms
 *  /64		 3.20us		209.7152ms -> CS12:0 = 011
 *  /256	12.80us		838.8608ms
 *
 * CCW	= 312 (1.0ms)
 * STOP = 469 (1.5ms)
 * CW	= 625 (2.0ms)
 */
void pwm_support_init()
{
	TCCR1A |= (1<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
	TIMSK1 |= (1<<TOIE1);					// enable interrupt on overflow.
//	ICR1 = 6250;		// 20ms period.
	ICR1 = 3125;		// 10ms period.
	DDRD |= (1<<PD5) | (1<<PD4);			// Set as outputs.

	speed_left = PWM_L_STOP;
	speed_right = PWM_R_STOP;
//	pwm_setMotor( PWM_L, PWM_L_STOP );
//	pwm_setMotor( PWM_R, PWM_R_STOP );
}

// Using Fast PWM mode
// OCR1A sets the width of the ON pulse
// OCR1B sets the width of the ON pulse
// ICR1 (TOP) sets the duration of the period.

void pwm_setMotor( uint8_t motor, uint16_t speed )
{
	if( motor == PWM_L )
	{
		speed_left = speed;
	}
	else
	{
		speed_right = speed;
	}
}

/*
 * Return Motor setting
 */
uint16_t pwm_getMotor( uint8_t motor )
{
	uint16_t val;
	
	val = (motor == PWM_L) ? speed_left : speed_right;
	
	return( val );
}

/*
 * Update motor control on interrupt. Double buffers control.
 */
ISR(TIMER1_OVF_vect)
{
	OCR1B = speed_left;
	OCR1A = speed_right;
}
