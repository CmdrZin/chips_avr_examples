/*
 * File:   io_ctrl.c
 * Author: Cmdrzin
 *
 * Created on August 8, 2024, 12:30 PM
 */


#include <avr/io.h>
#include "ioctrl.h"

#define LED_RED_PIN_BM PIN7_bm      // RED LED on PA7
#define AIN1_PIN    PIN4_bm         // AIN1 control
#define AIN2_PIN    PIN5_bm         // AIN2 control
#define BUMP_PIN    PIN6_bm         // Bump sensor input



/* *** Initialize the IO PORTS *** */
void init_io(void){
    /* set LED pins of PORT A as OUTPUT pins. The other bits are left as '0'
     * so that their associated pins will be INPUT pins. */
	PORTA.DIR |= LED_RED_PIN_BM;
    // Quad input pins PA2, PA3 default to INPUT
    // set control pins as OUTPUT
    PORTA.DIR |= AIN1_PIN | AIN2_PIN;
    // Bump pin defaults INPUT
}

/* toggle the RED LEDs state. */
void toggle_LED()
{
    PORTA_OUTTGL = LED_RED_PIN_BM;
}

void setLED(bool state)
{
    if(state) {
        PORTA_OUT |= LED_RED_PIN_BM;
    } else {
        PORTA_OUT &= ~LED_RED_PIN_BM;
    }
}

void setAN(bool stateAN1, bool stateAN2)
{
    uint8_t outPattern = 0;

    PORTA_OUT &= ~(AIN1_PIN | AIN2_PIN);        // clear pins to 0

    if(stateAN1) {
        outPattern |= AIN1_PIN;
    }
    if(stateAN2) {
        outPattern |= AIN2_PIN;
    }
    PORTA_OUT |= outPattern;
}

// +++ DEBUG SUPPORT +++
/*
 * Pulse LED ON n times.
 */
void pulseLED(uint8_t val)
{
	for(uint8_t i=0; i<val; ++i)
	{
		setLED(true);
		asm("nop");
		asm("nop");
		asm("nop");
		setLED(false);
		asm("nop");
		asm("nop");
		asm("nop");
	}
}
