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
 * ATmega4809_LEDblink
 *
 * Created: 11/7/2020 1:08:33 PM
 * Author : Chip
 */ 

#define PERIOD_EXAMPLE_VALUE    (0x0CB6)

#include <avr/io.h>
#include <avr/interrupt.h>
/*Using default clock 3.33MHz */

void TCA0_init(void);
void PORT_init(void);

void TCA0_init(void)
{
	/* enable overflow interrupt */
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	
	/* set Normal mode */
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
	
	/* disable event counting */
	TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
	
	/* set the period */
	TCA0.SINGLE.PER = PERIOD_EXAMPLE_VALUE;
	
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc         /* set clock source (sys_clk/256) */
	| TCA_SINGLE_ENABLE_bm;                /* start timer */
}

void PORT_init(void)
{
	/* set pin 0 of PORT A as output */
	PORTF.DIR |= PIN5_bm;
}

ISR(TCA0_OVF_vect)
{
	/* Toggle PIN 0 of PORT A */
	PORTF.OUTTGL = PIN5_bm;
	
	/* The interrupt flag has to be cleared manually */
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

int main(void)
{
	PORT_init();
	
	TCA0_init();
	
	/* enable global interrupts */
	sei();
	
	while (1)
	{
		;
	}
}