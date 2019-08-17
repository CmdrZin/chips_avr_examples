/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
 * pwm_dc_motor.c
 *
 * Created: 10/29/2015		0.01	ndp
 *  Author: Chip
 * revised: 04/16/2019		0.10	ndp	for use by Motor Slave.
 *
 * PWM DC Motor Control
 *
 * Target: Motor Slave, 18.4320MHz, ATmega164P, TB6612FNG driver.
 *
 * Uses PWM Counters and direct output I/O pins OC0A, OC0B, OC1A, & OC1B.
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

#define PWM_M1_PORT		PORTB
#define PWM_M2_PORT		PORTB
#define PWM_M3_PORT		PORTD
#define PWM_M4_PORT		PORTD
#define PWM_M1_DDR		DDRB
#define PWM_M2_DDR		DDRB
#define PWM_M3_DDR		DDRD
#define PWM_M4_DDR		DDRD
#define PWM_M1_P		PB3			// OC0A output, TB6612 PWMA
#define PWM_M2_P		PB4			// OC0B output, TB6612 PWMB
#define PWM_M3_P		PD5			// OC1A output, TB6612 PWMA
#define PWM_M4_P		PD4			// OC1B output, TB6612 PWMB

#define PWM_STBY_PORT	PORTD
#define PWM_STBY_DDR	DDRD
#define PWM_STBY_P		PD7			// TB6612 STBY

#define PWM_CTL_M1_PORT	PORTA
#define PWM_CTL_M1_DDR	DDRA
#define PWM_CTL_M1_A1	PA0			// TB6612 AIN1
#define PWM_CTL_M1_A2	PA1			// TB6612 AIN2

#define PWM_CTL_M2_PORT	PORTA
#define PWM_CTL_M2_DDR	DDRA
#define PWM_CTL_M2_B1	PA2			// TB6612 BIN1
#define PWM_CTL_M2_B2	PA3			// TB6612 BIN2

#define PWM_CTL_M3_PORT	PORTA
#define PWM_CTL_M3_DDR	DDRA
#define PWM_CTL_M3_A1	PA4			// TB6612 AIN1
#define PWM_CTL_M3_A2	PA5			// TB6612 AIN2

#define PWM_CTL_M4_PORT	PORTA
#define PWM_CTL_M4_DDR	DDRA
#define PWM_CTL_M4_B1	PA6			// TB6612 BIN1
#define PWM_CTL_M4_B2	PA7			// TB6612 BIN2

/*
 * mod_pwm_dc_motot_init()
 *
 * Set up I/O and PWM counters
 *
 * OC1B		- Right motor
 * OC1A		- Left motor
 *
 * Set up Timer1 for 1ms PWM using 18.4320MHz CPU clock
 * PWM Mode 15, Top(ICR1) = 18432 (1ms), OCR1A/B set ON time. 0(OFF)->20000(max ON)
 * REDO for 8 bit PWM Timer0 and Timer1
 * Call this once after RESET.
 *
 * NOTE: Write H L, Read L H.
 */
void pwm_dc_motor_init()
{
// Set up Timer0 for PWM
	// Handled by st_init_tmr0().
// Set up Timer 1 for PWM
	TCCR1A |= (1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(1<<WGM11)|(0<<WGM10);	// Use OC1A & OC1B pins for PWM
	TCCR1B |= (1<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);						// WGM = 1110 for ICR1 as TOP..CPU div 1 = ~0.045us
	ICR1 = 18432;			// 1ms period

//  Set IO pins for output.
	PWM_M1_DDR |= (1<<PWM_M1_P)|(1<<PWM_M2_P);					// M1 and M2 PWM on same port.
	PWM_M3_DDR |= (1<<PWM_M3_P)|(1<<PWM_M4_P);					// M3 and M4 PWM on same port.

	// All control on PortA
	PWM_CTL_M1_DDR |= (1<<PWM_CTL_M1_A1)|(1<<PWM_CTL_M1_A2)|(1<<PWM_CTL_M2_B1)|(1<<PWM_CTL_M2_B2)|
					  (1<<PWM_CTL_M3_A1)|(1<<PWM_CTL_M3_A2)|(1<<PWM_CTL_M4_B1)|(1<<PWM_CTL_M4_B2);

	PWM_STBY_DDR |= (1<<PWM_STBY_P);

	// Uses Motor Number 1:4
	for(int i=1; i<5; i++)
	{
		pwm_stop(i);
		pwm_set_dir(i, PWM_CW);
		pwm_set_speed(i, 0);
	}
}

/*
 * Set Motor direction ( see TB6612 table )
 * Uses Motor Number 1:4
 */
void pwm_set_dir(uint8_t motor, PWM_DIR_CONTROL dir )
{
	switch( dir )
	{
		case PWM_CW:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);					// disable STBY
			switch(motor)
			{
				case 1:
					PWM_CTL_M1_PORT |= (1<<PWM_CTL_M1_A1);
					PWM_CTL_M1_PORT &= ~(1<<PWM_CTL_M1_A2);
					break;

				case 2:
					PWM_CTL_M2_PORT |= (1<<PWM_CTL_M2_B1);
					PWM_CTL_M2_PORT &= ~(1<<PWM_CTL_M2_B2);
					break;

				case 3:
					PWM_CTL_M3_PORT |= (1<<PWM_CTL_M3_A1);
					PWM_CTL_M3_PORT &= ~(1<<PWM_CTL_M3_A2);
					break;

				case 4:
					PWM_CTL_M4_PORT |= (1<<PWM_CTL_M4_B1);
					PWM_CTL_M4_PORT &= ~(1<<PWM_CTL_M4_B2);
					break;
			}
			break;
		
		case PWM_CCW:
			PWM_STBY_PORT |= (1<<PWM_STBY_P);
			switch(motor)
			{
				case 1:
					PWM_CTL_M1_PORT &= ~(1<<PWM_CTL_M1_A1);
					PWM_CTL_M1_PORT |= (1<<PWM_CTL_M1_A2);
					break;

				case 2:
					PWM_CTL_M2_PORT &= ~(1<<PWM_CTL_M2_B1);
					PWM_CTL_M2_PORT |= (1<<PWM_CTL_M2_B2);
					break;

				case 3:
					PWM_CTL_M3_PORT &= ~(1<<PWM_CTL_M3_A1);
					PWM_CTL_M3_PORT |= (1<<PWM_CTL_M3_A2);
					break;

				case 4:
					PWM_CTL_M4_PORT &= ~(1<<PWM_CTL_M4_B1);
					PWM_CTL_M4_PORT |= (1<<PWM_CTL_M4_B2);
					break;
			}
			break;
	}	
}

/*
 * Set PWM to STOP
 * Uses Motor Number 1:4
 */
void pwm_stop(uint8_t motor)
{
	PWM_STBY_PORT &= ~(1<<PWM_STBY_P);					// enable STBY
	switch(motor)
	{
		case 1:
			PWM_CTL_M1_PORT &= ~(1<<PWM_CTL_M1_A1);
			PWM_CTL_M1_PORT &= ~(1<<PWM_CTL_M1_A2);
			break;

		case 2:
			PWM_CTL_M2_PORT &= ~(1<<PWM_CTL_M2_B1);
			PWM_CTL_M2_PORT &= ~(1<<PWM_CTL_M2_B2);
			break;

		case 3:
			PWM_CTL_M3_PORT &= ~(1<<PWM_CTL_M3_A1);
			PWM_CTL_M3_PORT &= ~(1<<PWM_CTL_M3_A2);
			break;

		case 4:
			PWM_CTL_M4_PORT &= ~(1<<PWM_CTL_M4_B1);
			PWM_CTL_M4_PORT &= ~(1<<PWM_CTL_M4_B2);
			break;
	}
}

/*
 * Set PWM speed
 * Uses Motor Number 1:4
 */
void pwm_set_speed(uint8_t motor, uint8_t val )
{
	switch(motor)
	{
		case 1:
			OCR0A = val;
			break;

		case 2:
			OCR0B = val;
			break;

		case 3:
			// 16 bit register
			OCR1A = val << 8;
			break;

		case 4:
			// 16 bit register
			OCR1B = val << 8;
			break;
	}
}
