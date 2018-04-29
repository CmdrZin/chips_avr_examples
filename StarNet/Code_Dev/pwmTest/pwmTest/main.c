/*
 * pwmTest.c
 *
 * Created: 12/5/2016 3:10:26 PM
 * Author : Chip
 */ 

#include <avr/io.h>


int main(void)
{
	// Set up Timer0. Set OC0A at BOTTOM:Clear OC0A on OCR0A Match, Fast PWM(3)
	TCCR0A |= (1<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (1<<WGM00);
	TCCR0B |= (0<<WGM02) | (0<<CS12) | (0<<CS11) | (1<<CS10);	// Clk/1
	OCR0A = 0x40;			// 64/256 duty cycle
	// Set I/O for output
	DDRB |= (1<<PORTB3);	// set as output.

	// Set up Timer1. Set OC1A at BOTTOM:Clear OC1A on OCR1A Match, Fast PWM 8-bit(5)
	TCCR1A |= (1<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (1<<WGM10);
	TCCR1B |= (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);	// Clk/1
	OCR1A = 0x0040;
	// Set I/O for output
	DDRD |= (1<<PORTD5);	// set as output.

	// Set up Timer2. Set OC2A at BOTTOM:Clear OC2A on OCR2A Match, Fast PWM(3)
	TCCR2A |= (1<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (1<<WGM21) | (1<<WGM20);
	TCCR2B |= (0<<WGM22) | (0<<CS22) | (0<<CS21) | (1<<CS20);	// Clk/1
	OCR2A = 0x0040;
	// Set I/O for output
	DDRD |= (1<<PORTD7);	// set as output.


    /* Replace with your application code */
    while (1) 
    {
		asm("NOP");
    }
}

