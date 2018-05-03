/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * mod_pwm.c
 *
 * Created: 10/24/2017		0.01	ndp
 *  Author: Chip
 * Revised: 04/30/2018		0.10	ndp stripped down for simple LED glow control.
 *
 * PWM control using Timer0
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define DDR_PWM0A	DDRB
#define DDR_PWM0B	DDRB

#define	PORT_PWM0A	PORTB
#define PORT_PWM0B	PORTB

#define PWM0A		PB0
#define PWM0B		PB1

// Use static to restrict scope to this module.
static volatile long	totalMilliseconds;		// total millisecond since POR.

void mod_pwm_init()
{
	// sysTimer support.
	totalMilliseconds = 0L;
	
	// Set up I/O pins as OUTPUT.
	DDR_PWM0A |= (1<<PWM0A);
	DDR_PWM0B |= (1<<PWM0B);
	
	// Set up Timer0
	OCR0A = 0;						// Initial value (OFF)
	OCR0B = 0;
	// Reset on OC0x match. Mode=3. TOP=0xFF. clk/64. Use Inverted output for DEMO board.
	TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (0<<WGM02)|(0<<CS02)|(1<<CS01)|(1<<CS00);		// 8us clock @ 8MHz. ~2ms overflow. 
	TIMSK = (1<<TOIE0);										// Enable OV interrupt.
}

// Set brightness for LED 1
// Val = 0 (OFF) ... 255 (MAX ON)
void mod_pwm1_set(int8_t val)
{
	OCR0A = val;
}

// Set brightness for LED 2
// Val = 0 (OFF) ... 255 (MAX ON)
void mod_pwm2_set(int8_t val)
{
	OCR0B = val;
}

void mod_pwn_disable()
{
	DDR_PWM0A &= ~(1<<PWM0A);
	DDR_PWM0B &= ~(1<<PWM0B);
}
// SYSTEM TIME taken care of here since Timer0 is used for PWM.

/*
 * Return milliseconds from Power-On-Reset.
 */
long st_millis()
{
	uint32_t temp;
	
	cli();
	temp = totalMilliseconds;
	sei();
	
	return temp;
}

/*
 * Timer0 OV interrupt service.
 * Called each 2.56ms
 *
 */
ISR(TIM0_OVF_vect)
{
	++totalMilliseconds;
	++totalMilliseconds;
}
