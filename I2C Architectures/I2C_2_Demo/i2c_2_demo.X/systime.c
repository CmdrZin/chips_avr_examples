/*
 * File:   systime.c
 * Author: Cmdrzin
 *
 * Created on February 24, 2026
 * Uses RTC for ~1ms tic
 */


#include <avr/io.h>
#include "systime.h"

volatile static uint32_t totalMilliseconds;	// local variable

/* RTC interrupt service routine. */
ISR(RTC_PIT_vect)
{
   	++totalMilliseconds;
    RTC.PITINTFLAGS = RTC_PI_bm;   // clear the interrupt flag
}

/* Return the total number of milliseconds since the project started. */
uint32_t millis(void)
{
	uint32_t temp;			// make a holder for the counter.
    uint8_t sreg_save;
    
    // Save SREG, disable interrupts, do code, restore SREG.
    // If interrupts were enabled, they will be re-enabled.
    sreg_save = SREG;
    cli();
  	temp = totalMilliseconds;	// get a copy while interrupts are disabled.
    SREG = sreg_save;

	return temp;			// return a 'clean' copy of the counter.
}

/* *** RTC Configuration for PIT 1ms tic *** */
void init_systime(void)
{
    // Configure the Clock CLK_RTC
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;  // 32.768kHz Internal Crystal Oscillator (INT32K)
    while( RTC.PITSTATUS > 0);          // Wait for PIT to synchronize with system.

    // Enable PIT interrupt, set PERIOD, and enable PIT
    RTC.PITINTCTRL |= RTC_PI_bm;
    RTC.PITCTRLA = RTC_PERIOD_CYC32_gc | RTC_PITEN_bm;
}
