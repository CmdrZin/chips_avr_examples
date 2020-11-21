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

volatile static uint8_t cmp0;
volatile static uint8_t cmp1;
volatile static uint8_t cmp2;
volatile static uint8_t cmp3;
volatile static uint8_t cmp4;
volatile static uint8_t cmp5;

void TCA0servo_init()
{
	// Set CLK_PER to DIV16 and enable pre-scaler. Done in sysTime.c
//	CPU_CCP = CCP_IOREG_gc;		// unlock Change Protected Registers
//	CLKCTRL_MCLKCTRLB = CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm;	// DIV16 -> CLK+PER = 1 MHz (1us)

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
	// Set default positions to center.
    cmp0 = TCA0_SPLIT_LCMP0 = 125+63;
    cmp1 = TCA0_SPLIT_LCMP1 = 125+63;
    cmp2 = TCA0_SPLIT_LCMP2 = 125+63;
    cmp3 = TCA0_SPLIT_HCMP0 = 125+63;
    cmp4 = TCA0_SPLIT_HCMP1 = 125+63;
    cmp5 = TCA0_SPLIT_HCMP2 = 125+63;
	// Enable interrupt on underflow LUNF
	TCA0_SPLIT_INTCTRL = TCA_SPLIT_LUNF_bm;
	// Enable counter
	TCA0_SPLIT_CTRLA |= TCA_SPLIT_ENABLE_bm;
	// Set IO bit to OUTPUT
	PORTA_DIR |= PIN5_bm|PIN4_bm|PIN3_bm|PIN2_bm|PIN1_bm|PIN0_bm;	// PA0:5
}

/* Add position to base pulse value. position 0:125, PA:0 */
void TCAservo_setPWM0(uint8_t position)
{
    cmp0 = 125 + position;		// 	125 = 1ms offset at 8us
}

/* Add position to base pulse value. position 0:125, PA:1 */
void TCAservo_setPWM1(uint8_t position)
{
	cmp1 = 125 + position;		// 	125 = 1ms offset at 8us
}

/* Add position to base pulse value. position 0:125, PA:2 */
void TCAservo_setPWM2(uint8_t position)
{
	cmp2 = 125 + position;		// 	125 = 1ms offset at 8us
}

/* Add position to base pulse value. position 0:125, PA:3 */
void TCAservo_setPWM3(uint8_t position)
{
	cmp3 = 125 + position;		// 	125 = 1ms offset at 8us
}

/* Add position to base pulse value. position 0:125, PA:4 */
void TCAservo_setPWM4(uint8_t position)
{
	cmp4 = 125 + position;		// 	125 = 1ms offset at 8us
}

/* Add position to base pulse value. position 0:125, PA:5 */
void TCAservo_setPWM5(uint8_t position)
{
	cmp5 = 125 + position;		// 	125 = 1ms offset at 8us
}

ISR(TCA0_LUNF_vect)
{
	// Stop TCA0 timer.
	TCA0_SPLIT_CTRLA &= ~(TCA_SPLIT_ENABLE_bm);
	// Update CMPn registers while timer is stopped.
    TCA0_SPLIT_LCMP0 = cmp0;
    TCA0_SPLIT_LCMP1 = cmp1;
    TCA0_SPLIT_LCMP2 = cmp2;
	TCA0_SPLIT_HCMP0 = cmp3;
	TCA0_SPLIT_HCMP1 = cmp4;
	TCA0_SPLIT_HCMP2 = cmp5;
	
	TCA0_SPLIT_INTFLAGS = TCA_SPLIT_LUNF_bm;	// Clear interrupt
}
