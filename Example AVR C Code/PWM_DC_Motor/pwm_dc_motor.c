/*
 * pwm_dc_motor.c
 *
 * Created: 10/29/2015		0.01	ndp
 *  Author: Chip
 *
 * PWM DC Motor Control
 *
 * Target: Tank Bot Bare, 20MHz, ATmega164P, TB6612 driver.
 *
 * Dependencies
 *	sysTimer.asm
 *
 * PWM servo driver. 1.0ms(Fwd) - 1.5ms (Stop) - 2.0ms (Rev) pulse at 40ms period.
 * Uses PWM Counters and direct output I/O pins OC1A & OC1B.
 *
 */

#include <avr/io.h>

#include "pwm_dc_motor.h"


uint8_t pwm_flags;
uint16_t pwm_val_right;
uint16_t pwm_val_left;

/*
 * TB6612 Control
 * IN1 IN2 PWM STBY  OUT1 OUT2  Mode
 *  H   H  H/L  H     L    L    Short break
 *
 *  L   H   H   H     L    H    CCW
 *  L   H   L   H     L    L    Short break
 *
 *  H   L   H   H     H    L    CW
 *  H   L   L   H     L    L    Short break
 *
 *  L   L   H   H     OFF(Hi-Z) Stop
 *
 * H/L H/L H/L  L     OFF(Hi-Z) Standby
 */

#define PWM_PORT		PORTD
#define PWM_DDR			DDRD
#define PWM_R_P			PD4			// OC1B output, TB6612 PWMB
#define PWM_L_P			PD5			// OC1A output, TB6612 PWMA

#define PWM_STBY_PORT	PORTD
#define PWM_STBY_DDR	DDRD
#define PWM_STBY_P		PD3			// TB6612 STBY

#define PWM_CTL_R_PORT	PORTB
#define PWM_CTL_R_DDR	DDRB
#define PWM_CTL_R_A1	PB4			// TB6612 AIN1
#define PWM_CTL_R_A2	PB3			// TB6612 AIN2

#define PWM_CTL_L_PORT	PORTD
#define PWM_CTL_L_DDR	DDRD
#define PWM_CTL_L_B1	PD6			// TB6612 BIN1
#define PWM_CTL_L_B2	PD7			// TB6612 BIN2


/*
 * mod_pwm_dc_motot_init()
 *
 * Set up I/O and PWM counters
 *
 * OC1B		- Right motor
 * OC1A		- Left motor
 *
 * Set up Timer1 for 1ms PWM using 20MHz CPU clock
 * PWM Mode 15, Top(ICR1) = 20000 (1ms), OCR1A/B set ON time. 0(OFF)->20000(max ON)
 * Call this once after RESET.
 *
 * NOTE: Write H L, Read L H.
 */
void pwm_dc_motor_init()
{
// Set up Timer 1
	TCCR1A |= (1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(1<<WGM11)|(0<<WGM10);	// Use OC1A & OC1B pins for PWM
	TCCR1B |= (1<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);						// WGM = 1110 for ICR1 as TOP..CPU div 1 = 0.050us
	ICR1 = 20000;			// 1ms period

//  Set IO pins for output.
	PWM_DDR |= (1<<PWM_R_P)|(1<<PWM_L_P);
	PWM_CTL_L_DDR |= (1<<PWM_CTL_L_B1)|(1<<PWM_CTL_L_B2);
	PWM_CTL_R_DDR |= (1<<PWM_CTL_R_A1)|(1<<PWM_CTL_R_A2);
	PWM_STBY_DDR |= (1<<PWM_STBY_P);

	pwm_set_left_dir(PWM_STOP);
	pwm_set_right_dir(PWM_STOP);
	pwm_stop_right();
	pwm_stop_left();
}

/*
 * Set Right side direction ( see TB6612 table )
 */
void pwm_set_right_dir( PWM_DIR_CONTROL dir )
{
	switch( dir )
	{
		case PWM_STOP:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_R_PORT &= ~(1<<PWM_CTL_R_A1);
			PWM_CTL_R_PORT &= ~(1<<PWM_CTL_R_A2);
			break;
		
		case PWM_FWD:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_R_PORT |= (1<<PWM_CTL_R_A1);
			PWM_CTL_R_PORT &= ~(1<<PWM_CTL_R_A2);
			break;
		
		case PWM_REV:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_R_PORT &= ~(1<<PWM_CTL_R_A1);
			PWM_CTL_R_PORT |= (1<<PWM_CTL_R_A2);
			break;
	}	
}

/*
 * Set PWM RIGHT to STOP
 */
void pwm_stop_right()
{
	pwm_val_right = PWM_R_STOP;
	pwm_set_right( pwm_val_right );
}

/*
 * Add PWM rate RIGHT
 * NOTE: No range check
 */
void pwm_add16_right( uint16_t val )
{
	pwm_val_right += val;
	pwm_set_right( pwm_val_right );
}

/*
 * Subtract PWM rate RIGHT
 * NOTE: No range check
 */
void pwm_sub16_right( uint16_t val )
{
	pwm_val_right -= val;
	pwm_set_right( pwm_val_right );
}

/*
 * Set PWM rate RIGHT and Output
 * NOTE: No range check
 */
void pwm_set_right( uint16_t val )
{
	pwm_val_right = val;
	OCR1B = pwm_val_right;
}

/*
 * Set Left side direction ( see TB6612 table )
 */
void pwm_set_left_dir( PWM_DIR_CONTROL dir )
{
	switch( dir )
	{
		case PWM_STOP:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_L_PORT &= ~(1<<PWM_CTL_L_B1);
			PWM_CTL_L_PORT &= ~(1<<PWM_CTL_L_B2);
			break;
		
		case PWM_FWD:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_L_PORT |= (1<<PWM_CTL_L_B1);
			PWM_CTL_L_PORT &= ~(1<<PWM_CTL_L_B2);
			break;
		
		case PWM_REV:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			PWM_CTL_L_PORT &= ~(1<<PWM_CTL_L_B1);
			PWM_CTL_L_PORT |= (1<<PWM_CTL_L_B2);
			break;
	}	
}

/*
 * Set PWM LEFT to STOP
 */
void pwm_stop_left()
{
	pwm_val_left = PWM_L_STOP;
	pwm_set_left( pwm_val_left );
}

/*
 * Add PWM rate LEFT
 * NOTE: No range check
 */
void pwm_add16_left(uint16_t val )
{
	pwm_val_left += val;
	pwm_set_left( pwm_val_left );
}

/*
 * Subtract PWM rate LEFT
 * NOTE: No range check
 */
void pwm_sub16_left( uint16_t val )
{
	pwm_val_left -= val;
	pwm_set_left( pwm_val_left );
}

/*
 * Set PWM rate LEFT
 * Rate is N * 4.77us
 *
 * input:	R17	- Data
 */
void pwm_set_left( uint16_t val )
{
	pwm_val_left = val;
	OCR1A = pwm_val_left;
}
