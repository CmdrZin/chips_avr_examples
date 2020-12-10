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
 * TCAservo.c
 *
 * Created: 11/15/2020 6:28:53 PM
 *  Author: Chip
 */ 

#include <avr/io.h>


void TCA0servo_init()
{
	// Set CLK_PER to DIV16 and enable prescaler
	CPU_CCP = CCP_IOREG_gc;		// unlock Change Protected Registers
	CLKCTRL_MCLKCTRLB = CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;	// DIV16 -> CLK+PER = 1 MHz (1us)

	// Set TCA0 prescaler to DIV8 (clock -> 8us)
	TCA0_SINGLE_CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc;
	// Mode: Output WO0: enable, Single Slope.
	TCA0_SINGLE_CTRLB = TCA_SINGLE_CMP0_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	// Set period to 20ms based on 8us clock.
    TCA0_SINGLE_PER = 2500;			// 2500 * 8us = 20ms
	// Set default position to center.
    TCA0_SINGLE_CMP0 = 125+67;
	// Enable Compare 0
	TCA0_SINGLE_CTRLC = TCA_SINGLE_CMP0OV_bm;
	// Enable counter
	TCA0_SINGLE_CTRLA |= TCA_SINGLE_ENABLE_bm;
}

/* Add position to base pulse value. */
void TCAservo_setPWM(uint8_t position)
{
	uint16_t val = 125;		// 1ms offset at 8us
	
    TCA0_SINGLE_CMP0 = val + position;
}
