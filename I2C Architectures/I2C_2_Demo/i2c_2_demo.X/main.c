/*
 * File:   main.c
 * Author: Cmdrzin
 *
 * Created on May 10, 2026
 * I2C Type 2 demo
 * ref: I2C Architecture.odt
 * 
 * Target: ATtiny814 (SOIC20)
 * LED: PA7
 * SCL: PB0
 * SDA: PB1
 */

#include <avr/io.h>
#include <avr/interrupt.h>          // to support the use of interrupts
#include "sysdefs.h"                // system wide defines.
#include "systime.h"                // access systime functions.
#include "ioctrl.h"
#include "twiRegSlave2.h"

#define TWI_DELAY		100UL        // update TWI slave data
#define SLAVE_ADRS		0x5E

// I2C communication buffers.
// rxBuff .. data received by WRITE
volatile uint8_t statusLED = 0;                  // manual set 0:Off .. 1:On
volatile uint16_t ledDelay = 500;                // LED toggle time (ms) .. 0: disabled
volatile uint8_t clearCount1 = 1;                // 1: clear count1 .. set to 0 after use
volatile uint8_t clearCount2 = 1;                // 1: clear count2 .. set to 0 after use
volatile uint8_t count1Dir = 0;                  // 0: inc .. 1: dec
volatile uint8_t count2Dir = 1;                  // 0: inc .. 1: dec
volatile int32_t setCount1 = 30000;              // preset count1 value

// txBuff .. data to send out by READ
uint8_t zeroBuff = 0;
uint8_t statusBuff = 0;
int32_t count1 = 0;
uint8_t count2 = 0;

uint8_t dummyBuff = 0;                          // no register assigned


COMM_BUFF rxtxBuffers[] = {
    { &statusLED, 1, &zeroBuff, 1 },            // rx 0x00   tx 0x00 
    { &ledDelay, 2, &statusBuff, 1 },           // rx 0x01   tx 0x01
    { &clearCount1, 1, &count1, 4},             // rx 0x02   tx 0x02
    { &clearCount2, 1, &count2, 1},             // rx 0x03   tx 0x03
    { &count1Dir, 1, &dummyBuff, 1},            // rx 0x04   tx N/A
    { &count2Dir, 1, &dummyBuff, 1},            // rx 0x05   tx N/A
    { &setCount1, 4, &dummyBuff, 1}             // rx 0x06   tx N/A
};
// NOTE: buffer size could be replace with sizeof(buffer)

int main(void) {
   	uint32_t ledTime = 0UL;
	uint32_t twiTime = 0UL;

    init_systime();             // set up TCA0 timer.
    init_io();                  // set up IO pins.
   	twiRegSlaveInit( SLAVE_ADRS, rxtxBuffers, sizeof(rxtxBuffers)/sizeof(COMM_BUFF) );  // 10);
    
    /* enable Global interrupts */
    sei();
   
    while (1) {
        // Blink LED if active
        if( ledDelay != 0 ) {
            if( ledTime < millis() ) {
                ledTime = millis() + ledDelay;
            
                toggle_LED();
            }
        }
        
        // Update system.
		if( twiTime < millis() ) {
			twiTime = millis() + TWI_DELAY;
            
            if( ledDelay == 0 ) {
                setLED( (statusLED == 1) );         // use manual control
            }
            
            // update data registers
            if( count1Dir == 0) {
        		++count1;
            } else {
                --count1;
            }
            
            if( count2Dir == 0) {
        		++count2;
            } else {
                --count2;
            }
            
            // Set status bit on overflow
            if( count1 == 0) {
                statusBuff |= 0x01;
            }

            if( count2 == 0) {
                statusBuff |= 0x02;
            }
            
            // Manual clear
            if( clearCount1 == 1 ) {
                clearCount1 = 0;
                count1 = 0;
                statusBuff &= ~0x01;
            }

            // Manual clear
            if( clearCount2 == 1 ) {
                clearCount2 = 0;
                count2 = 0;
                statusBuff &= ~0x02;
            }
		}
    }
}
