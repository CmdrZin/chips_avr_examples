/*
 * The MIT License
 *
 * Copyright 2020 CmdrZin.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * TCA6servo.c
 *
 * Created: 11/21/2020 6:28:53 PM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile static uint8_t dutys[6];

void TCA_6pwm_init()
{
	// Set CLK_PER to DIV16 and enable pre-scaler. Done in sysTime.c

	// Set TCA0 pre-scaler to DIV8 (clock -> 8us)
	TCA0_SINGLE_CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc;
	// Mode: Split
	TCA0_SPLIT_CTRLD = TCA_SPLIT_SPLITM_bm;
	// Waveform Mode: Output WO0:5 enable, Single Slope.
	TCA0_SPLIT_CTRLB = TCA_SPLIT_HCMP2EN_bm | TCA_SPLIT_HCMP1EN_bm | TCA_SPLIT_HCMP0EN_bm |
					   TCA_SPLIT_LCMP2EN_bm | TCA_SPLIT_LCMP1EN_bm | TCA_SPLIT_LCMP0EN_bm;
	// Set H and L periods to 2.4ms based on 8us clock.
    TCA0_SPLIT_LPER = 0xFF;
	TCA0_SPLIT_HPER = 0xFF;
	// Set default positions to middle (50%).
	for(int i=0; i<sizeof(dutys); i++) {
		dutys[i] = 128;
	}
	// Enable interrupt on underflow LUNF
	TCA0_SPLIT_INTCTRL = TCA_SPLIT_LUNF_bm;
	// Enable counter
	TCA0_SPLIT_CTRLA |= TCA_SPLIT_ENABLE_bm;
	// Set IO bit to OUTPUT
	PORTA_DIR |= PIN5_bm|PIN4_bm|PIN3_bm|PIN2_bm|PIN1_bm|PIN0_bm;	// PA0:5
}

/* Set duty cycle. */
void TCA_setPWM(uint8_t channel, uint8_t duty)
{
	if( channel < sizeof(dutys)) {
	    dutys[channel] = duty;		// 	128 = 50%
	}
}

ISR(TCA0_LUNF_vect)
{
	// Stop TCA0 timer.
	TCA0_SPLIT_CTRLA &= ~(TCA_SPLIT_ENABLE_bm);
	// Update CMPn registers while timer is stopped.
    TCA0_SPLIT_LCMP0 = dutys[0];
    TCA0_SPLIT_LCMP1 = dutys[1];
    TCA0_SPLIT_LCMP2 = dutys[2];
	TCA0_SPLIT_HCMP0 = dutys[3];
	TCA0_SPLIT_HCMP1 = dutys[4];
	TCA0_SPLIT_HCMP2 = dutys[5];
	// ReStart the timer
	TCA0_SPLIT_CTRLA |= TCA_SPLIT_ENABLE_bm;
	
	TCA0_SPLIT_INTFLAGS = TCA_SPLIT_LUNF_bm;	// Clear interrupt
}
