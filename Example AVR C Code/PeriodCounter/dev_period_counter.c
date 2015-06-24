/*
 * dev_period_counter.c
 *
 * Created: 6/22/2015 4:40:10 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "usiTwiSlave.h"

#include "dev_period_counter.h"

volatile uint8_t dev_period_overflow_cnt;	// inc by ISR

volatile uint8_t dev_period_overflow;		// set by ISR
volatile uint8_t dev_period_count_H;		// Loaded by ISR
volatile uint8_t dev_period_count_L;
volatile bool dev_period_count_flag;

uint8_t dev_period_overflow_last;
uint8_t dev_period_count_H_last;
uint8_t dev_period_count_L_last;

uint8_t dev_period_overflow_data;
uint8_t dev_period_count_H_data;
uint8_t dev_period_count_L_data;


/*
 * Period Counter initialization.
 * Uses ICP input pin to trigger counter.
 */
void dev_period_counter_init()
{
	dev_period_count_flag = false;
	dev_period_count_H_last = 0;
	dev_period_count_L_last = 0;
	dev_period_overflow_last = 0;
	
	dev_period_overflow_cnt = 0;

	dev_period_count_H_data = 0;
	dev_period_count_L_data = 0;
	dev_period_overflow_data = 0;

	// Initialize hardware
	TCCR1A = (0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10);
	// Use falling edge on ICP, CLK/8 (1 us), 
	TCCR1B = (0<<ICNC1)|(0<<ICES1)|(0<<WGM13)|(0<<WGM12)|(0<<CS12)|(1<<CS11)|(0<<CS10);
	// Enable ICP and OV interrupts
	TIMSK1 = (1<<ICIE1)|(0<<OCIE1B)|(0<<OCIE1A)|(1<<TOIE1);
	
	
// DEBUG ++
//	dev_period_count_H = 0x20;
//	dev_period_count_L = 0x10;
//	dev_period_overflow = 0x30;
//
//	dev_period_count_H_last = 0x60;
//	dev_period_count_L_last = 0x70;
//	dev_period_overflow_last = 0x50;
//
//	dev_period_count_flag = true;
// DEBUG --
}

void dev_period_counter_service()
{
	// Check flag and update count.
	if(dev_period_count_flag)
	{
		// Do 24 bit subtraction
		asm("lds	r21, dev_period_count_L_last");
		asm("lds	r22, dev_period_count_H_last");
		asm("lds	r20, dev_period_overflow_last");
		
		asm("lds	r24, dev_period_count_L");
		asm("lds	r25, dev_period_count_H");
		asm("lds	r23, dev_period_overflow");

		asm("sub	r24, r21");						// L - L_last
		asm("sbc	r25, r22");						// H - H_last
		asm("sbc	r23, r20");						// OV - OV_last
		
		//asm("brcc	dpcs01");						// CY=1 if last > current. Wrap-around det.
		//// Adjust for wrap-around.
		//asm("clr	r20");
		//asm("com	r24");
		//asm("com	r25");
		//asm("com	r23");
		//asm("adiw	r24, 1");
		//asm("adc	r23, r20");
		//asm("dpcs01:");

		asm("sts	dev_period_count_L_data, r24");
		asm("sts	dev_period_count_H_data, r25");
		asm("sts	dev_period_overflow_data, r23");
		// Update
		asm("lds	r24, dev_period_count_L");
		asm("lds	r25, dev_period_count_H");
		asm("lds	r23, dev_period_overflow");
		asm("sts	dev_period_count_L_last, r24");
		asm("sts	dev_period_count_H_last, r25");
		asm("sts	dev_period_overflow_last, r23");

		dev_period_count_flag = false;
	}

// DEBUG ++
//	dev_period_count_flag = true;		// data should go to 0
// DEBUG --
	return;
}


/* *** SOLENOID ACCESS FUNCTIONS *** */

/*
 * Command 0x01 - Read period count
 * DEV 0x01
 * Loads output FIFO with DEV dataH dataL
 */
void dev_period_counter_get()
{
	// Get last Period Count and put it into output fifo with device ID
	usiTwiTransmitByte( DEV_PERIOD_COUNTER_ID );
	usiTwiTransmitByte( dev_period_overflow_data );
	usiTwiTransmitByte( dev_period_count_H_data );
	usiTwiTransmitByte( dev_period_count_L_data );
}

ISR(SIG_INPUT_CAPTURE1)
{
	dev_period_count_L = ICR1L;
	dev_period_count_H = ICR1H;
	dev_period_overflow = dev_period_overflow_cnt;
	dev_period_count_flag = true;
}

/*
 * Increment Overflow Count
 * 
 */
ISR(TIM1_OVF_vect)
{
	++dev_period_overflow_cnt;
}