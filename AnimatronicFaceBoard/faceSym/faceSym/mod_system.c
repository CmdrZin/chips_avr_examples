/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * mod_system.c
 *
 * Created: 7/27/2016 10:54:23 AM
 *  Author: Chip
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mod_system.h"
#include "sysTimer.h"
#include "mod_led.h"
#include "adc_support.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "mod_pca9685.h"

char* getNameString( MSS_STATES state, char buf[] );
void mss_menuMode();
void mss_runMode();
void mss_manualMode();
uint8_t mss_getButtons();

#define MOD_ADC_I2C	0x32
#define MOD_LCD_I2C 0x5E

#define MSS_DELAY			10		// N * 1ms
#define MSS_RUN_DELAY		100		// N * 1ms
#define MSS_MANUAL_DELAY	25		// N * 1ms

#define BUTTON_GREEN	0x01
#define BUTTON_RED		0x02
#define BUTTON_YELLOW	0x04

typedef struct {
	uint32_t ktimeStamp;			// minimum 20ms steps. Maximum update rate.
	uint8_t	servo[16];
} SERVO_DATA;
SERVO_DATA	servoPos[8];		// 64 @ 20 bytes per entry -> 1280 bytes.
uint32_t seqStep;				// Duration between key frames.

MSS_STATES	mss_state;
uint8_t	mssDelay;
uint8_t	mss_count;

int currentAdc[16];			// latest ADC reading (-128 : 127)

char textBuf[18];

void mod_system_init()
{
	mss_state = MSS_IDLE;
	mssDelay = MSS_DELAY;
	mss_count = 0;
	seqStep = 100;				// N * ms
}

void mod_systey_setKeyFrame(uint8_t servoNum, uint8_t pos)
{
	servoPos[0].servo[0] = servoNum;
	servoPos[0].ktimeStamp = 500;
}

/*
 * System Service
 * Servos are on a 20ms period, so it doesn't doing good to try to update them faster than that.
 *
 */
void mod_system_service()
{
	static uint32_t lastTime = 0;
	uint32_t currentTime;
	char nameStr[10];
	
	currentTime = st_millis();
	
	if( currentTime > (lastTime + mssDelay) )
	{
		lastTime = currentTime;

		switch( mss_state )
		{
			case MSS_IDLE:
				// Check for BUTTON_GRN press. If pressed, go to MSS_RUN
				if(mss_count == 1) {
					// Send out banner.
					mod_stdio_print("The Face Is Up! ", 16);
				}
				if(++mss_count > 250)
				{
					mss_count = 0;
					mss_state = MSS_MENU_INIT;
					mssDelay = 1;
				}
				break;
				
			case MSS_MENU_INIT:
				// Set up Menu banner
				snprintf(textBuf, 17, " << %8s >> ", getNameString(MSS_MANUAL, nameStr));
				mod_stdio_print(textBuf, 16);
				mss_state = MSS_MENU;
				mssDelay = 50;			// N * 1ms
				break;
				
			case MSS_MENU:
				mss_menuMode();
				break;

			// MANUAL Mode has the servo track its associated control pot.
			case MSS_MANUAL:
				mss_state = MSS_IDLE;
				break;

			case MSS_PRGM:
				mss_state = MSS_IDLE;
				break;

			case MSS_RUN:
				mss_state = MSS_IDLE;
				break;
			
			case MSS_ERASE:
				mss_state = MSS_IDLE;
				break;

			case MSS_STEP:
				mss_state = MSS_IDLE;
				break;
			
			case MSS_FTIME:
				mss_state = MSS_IDLE;
				break;
			
			default:
				mss_state = MSS_IDLE;
				break;
		}
	}
}

char* getNameString( MSS_STATES state, char buf[] )
{
	switch( state )
	{
		case MSS_MANUAL:
			strncpy(buf, " MANUAL ", 9);
			break;
		
		case MSS_PRGM:
			strncpy(buf, "PROGRAM ", 9);
			break;
		
		case MSS_RUN:
			strncpy(buf, "  RUN   ", 9);
			break;
		
		case MSS_ERASE:
			strncpy(buf, " ERASE  ", 9);
			break;
		
		case MSS_STEP:
			strncpy(buf, "  STEP  ", 9);
			break;
		
		case MSS_FTIME:
			strncpy(buf, " F-TIME ", 9);
			break;
		
		default:
			strncpy(buf, " ?????? ", 9);
			break;
	}
	
	return( buf );
}

/*
 * Menu Mode - Call every 50ms
 * Read Buttons: GREEN=Cycle up list, RED=Select, YELLOW=Cycle down list
 * Menu list: MANUAL, PROGAM, RUN, ERASE, STEP, F-TIME
 */
void mss_menuMode()
{
	uint8_t	data;
	static MSS_STATES menuState = MSS_MANUAL;
	char nameStr[10];
	
	data = mss_getButtons();
	if(data > 7) return;			// 0x88 error

	switch( menuState )
	{
		case MSS_MANUAL:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_FTIME;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_MANUAL;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_PRGM;
			break;
			
		case MSS_PRGM:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_MANUAL;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_PRGM;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_RUN;
			break;
			
		case MSS_RUN:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_PRGM;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_RUN;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_ERASE;
			break;
		
		case MSS_ERASE:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_RUN;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_ERASE;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_STEP;
			break;
		
		case MSS_STEP:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_ERASE;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_STEP;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_FTIME;
			break;
		
		case MSS_FTIME:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_STEP;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_FTIME;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_MANUAL;
			break;
			
		default:
			menuState = MSS_MANUAL;
			break;
	}

	// Update is button pressed.
	if(data != 0) {
		snprintf(textBuf, 17, " << %8s >> ", getNameString(menuState, nameStr));
		mod_stdio_print(textBuf, 16);
	}
}

/*
 * Run Mode
 */
void mss_runMode()
{
	static uint8_t slow_cnt = 0;

			++slow_cnt;
			if( slow_cnt == 1) {
				mp_setPos(2, 0x0133+60);
				mod_led_on();

			}
			if( slow_cnt == 2) {
				mp_setPos(2, 0x0133-60);
				mod_led_off();
				slow_cnt = 0;
			}
}

/*
 * Manual Mode
 */
void mss_manualMode()
{
	int adc_data;
	char buf[8];
	uint8_t nbytes;

	for( uint8_t i=0; i<8; i++)
	{
		// Get ADC channels last reading.
		adc_data = (int)adc_support_readChan(i);			// 0 -> 255
		adc_data -= 128;									// center
		
		currentAdc[i] = adc_data;

		mp_setPos(i, MP_CENTER - currentAdc[i]);			// using i+8 controls second set of 8 servos.

		if(i == 0) {
			nbytes = snprintf(buf, 7, " %+04d ", adc_data);
			mod_stdio_printOffset(buf, nbytes-1, 3);
		}
	}

	// Read ADC Slave Ch 8-15
	buf[0] = 0x08;
	// Send packet.
	tim_write(MOD_ADC_I2C, (uint8_t*)buf, 1);
	
	// NOTE: Could use a time-out here of 5ms
	tim_read( MOD_ADC_I2C, 8 );
	for(int i=8; i<16; i++)
	{
		// Wait for data.
		while(!tim_hasData());
		// get data.
		adc_data = (int)tim_readData();
		adc_data -= 128;									// center

		currentAdc[i] =  adc_data;

		mp_setPos(i, MP_CENTER - currentAdc[i]);
	}
}

MSS_STATES mod_system_getState()
{
	return( mss_state );
}

/*
 * Get Button state from LCD controller
 * State is if the button was pressed since the last call, not the current raw state.
 * data = 00000YRG
 */
uint8_t mss_getButtons()
{
	char buf[6];
	uint8_t data;
	uint8_t tmp;

	buf[0] = 0x01;
	// Send packet.
	tim_write( MOD_LCD_I2C, (uint8_t*)buf, 1 );
	
	// NOTE: Could use a time-out here of 5ms
	tim_read( MOD_LCD_I2C, 1 );
	// Wait for data.
	while(!tim_hasData()) {
		 if( !tim_isBusy() ) return(0);		// error on bus while no data received. EXIT.
	}

	// get data.
	data = tim_readData();
	tmp = data;
	
	// DEBUG - Echo Buttons to LED
	for(int i=0; i<8; i++)
	{
		if(tmp & 0x80) {
			mod_led_on();
		} else {
			mod_led_off();
		}
		tmp = tmp << 1;
	}
	
	return(data);
}