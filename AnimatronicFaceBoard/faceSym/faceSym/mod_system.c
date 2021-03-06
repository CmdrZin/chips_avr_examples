/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * Revised: 1/09/2018	ndp		0.10	For use with faceAnim system
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "mod_system.h"
#include "sysTimer.h"
#include "mod_led.h"
#include "adc_support.h"
#include "twi_I2CMaster.h"
#include "mod_stdio.h"
#include "mod_pca9685.h"

// DEBUG++
#include "mod_exp_io.h"
// DEBUG--


char* getNameString( MSS_STATES state, char buf[] );
void mss_menuMode();
void mss_prgmInit();
void mss_prgmService();
void mss_runInit();
void mss_runService();
void mss_manualMode();
void mss_zeroMode();
uint8_t mss_getButtons();

#define MOD_LCD_I2C 0x5E

#define MSS_DELAY			10		// N * 1ms
#define MSS_RUN_DELAY		100		// N * 1ms
#define MSS_MANUAL_DELAY	50		// N * 1ms

#define BUTTON_GREEN	0x01
#define BUTTON_RED		0x02
#define BUTTON_YELLOW	0x04

#define MAX_FRAMES		8
const uint8_t mss_calTable[][16];		// MUST be declared before use.


typedef struct {
	uint32_t timeStamp;			// minimum 20ms steps. Maximum update rate.
	uint8_t servo[16];
} SERVO_DATA;
SERVO_DATA	servoPos[MAX_FRAMES];	// 64 @ 20 bytes per entry -> 1280 bytes.
uint32_t seqStep;				// Duration between key frames.

static uint8_t runIndex;		// also used in Program as frame index.
static uint32_t runStartTime;	// also used in Program as start time.
static bool runLoop;
static uint32_t moveTo;

MSS_STATES menuState = MSS_MANUAL;
MSS_STATES	mss_state;
uint8_t	mssDelay;
uint8_t	mss_count;

uint8_t currentAdc[16];			// latest ADC reading (-128 : 127)

char textBuf[20];

void mod_system_init()
{
	char data;
	
	menuState = MSS_MANUAL;
	mss_state = MSS_IDLE;
	mssDelay = MSS_DELAY;
	mss_count = 0;
	seqStep = 2000;				// N * ms

	// Zero ADC array
	for(int i=0; i<16; i++) {
		currentAdc[i] = 0;
	}

	// Zero RUN array
	for(int i=0; i<MAX_FRAMES; i++) {
		servoPos[i].timeStamp = 0;
		for(int y=0; y<16; y++) {
			servoPos[i].servo[y] = 0;
		}
	}
	
	// TEST - PRELOAD array
	for(int i=0; i<6; i++) {
		servoPos[i].timeStamp = (i * seqStep) + 1;
		for(int y=0; y<16; y++) {
			data = pgm_read_byte(&mss_calTable[i][y]);
			servoPos[i].servo[y] = data;
		}
	}
}

void mod_systey_setKeyFrame(uint8_t servoNum, uint8_t pos)
{
	servoPos[0].servo[0] = servoNum;
	servoPos[0].timeStamp = 500;
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
				snprintf(textBuf, 17, " << %8s >> ", getNameString(menuState, nameStr));
				mod_stdio_print(textBuf, 16);
				mss_state = MSS_MENU;
				mssDelay = 50;			// N * 1ms
				break;
				
			case MSS_MENU:
				mss_menuMode();
				break;

			// MANUAL Mode has the servo track its associated control pot.
			case MSS_MANUAL:
				mssDelay = MSS_MANUAL_DELAY;		// N * 1ms
				mss_manualMode();
				break;

			case MSS_PRGM_MENU:
			// Set up Menu banner
				mod_stdio_print("REC   EXIT  #:00", 16);
				mss_state = MSS_PRGM_INIT;
				mssDelay = 100;			// N * 1ms
				break;
			
			case MSS_PRGM_INIT:
				mss_prgmInit();
				break;
			
			case MSS_PRGM:
				mss_prgmService();
				break;

			case MSS_RUN_MENU:
				// Set up Menu banner
				mod_stdio_print("ONCE  EXIT  LOOP", 16);
				mss_state = MSS_RUN_INIT;
				mssDelay = 50;			// N * 1ms
				break;
			
			case MSS_RUN_INIT:
				mss_runInit();
				break;
			
			case MSS_RUN:
				mss_runService();
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
				
			case MSS_ZERO:
				mss_zeroMode();
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
		
		case MSS_RUN_MENU:
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
		
		case MSS_ZERO:
			strncpy(buf, "  ZERO  ", 9);
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
	char nameStr[10];
	
	data = mss_getButtons();
	if(data > 7) return;			// 0x88 error

	switch( menuState )
	{
		case MSS_MANUAL:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_ZERO;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_MANUAL;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_PRGM;
			break;
			
		case MSS_PRGM:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_MANUAL;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_PRGM_MENU;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_RUN_MENU;
			break;
			
		case MSS_RUN_MENU:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_PRGM;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_RUN_MENU;
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_ERASE;
			break;
		
		case MSS_ERASE:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_RUN_MENU;
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
			if((data & BUTTON_YELLOW) == BUTTON_YELLOW) menuState = MSS_ZERO;
			break;
			
		case MSS_ZERO:
			if((data & BUTTON_GREEN) == BUTTON_GREEN) menuState = MSS_FTIME;
			if((data & BUTTON_RED) == BUTTON_RED) mss_state = MSS_ZERO;
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
 * Program Init Mode
 */
void mss_prgmInit()
{
	runIndex = 0;
	runStartTime = 1;				// non-zero time. Zero is end of sequence.
	mss_state = MSS_PRGM;
}

/*
 * Program Mode
 * NOTE: When building the servoPos array, the timeStamp starts at 0 and F-Time is added to each step.
 * CAUTION: Large movement can occur if LAST frame and FIRST frame are not the same.
 */
void mss_prgmService()
{
	uint8_t data;
	uint8_t nbytes;
	
	for( uint8_t i=0; i<8; i++)
	{
		// Get ADC channels last reading.
		currentAdc[i] = adc_support_readChan(i);			// 0 -> 255
		mp_setPos(i, (MP_CENTER + currentAdc[i]) - 128);		// using i+8 controls second set of 8 servos.
	}

	for(int i=8; i<16; i++)
	{
#if 0
		// set data.
		currentAdc[i] = 128;			// ADC Center
#else
		// get data.
		currentAdc[i] = adc_support_readChan(i);
#endif
		mp_setPos(i, (MP_CENTER + currentAdc[i]) - 128);		// using i+8 controls second set of 8 servos.
	}
	data = mss_getButtons();
	if(data > 7) return;			// 0x88 error

	if(data != 0) {
		if( ((data & BUTTON_GREEN) == BUTTON_GREEN) && (runIndex < MAX_FRAMES) ) {
			// Copy adc_data into servo[]
			for(int i=0; i<16; i++)
			{
				servoPos[runIndex].servo[i] = currentAdc[i];
			}
			++runIndex;

			// Update display
			nbytes = snprintf(textBuf, 17, "REC   EXIT #:%02d  ", runIndex);
			mod_stdio_print(textBuf, nbytes-1);

			// Update parameters
			runStartTime = runStartTime + seqStep;
		} else if((data & BUTTON_RED) == BUTTON_RED) {
			mss_state = MSS_MENU_INIT;
		}
	}
}

/*
 * Run Init Mode
 */
void mss_runInit()
{
	uint8_t data;
	
	data = mss_getButtons();
	if(data > 7) return;			// 0x88 error
	
	if(data != 0) {
		if((data & BUTTON_GREEN) == BUTTON_GREEN) {
			runLoop = false;
			mss_state = MSS_RUN;
		} else 	if((data & BUTTON_YELLOW) == BUTTON_YELLOW) {
			runLoop = true;
			mss_state = MSS_RUN;
		} else if((data & BUTTON_RED) == BUTTON_RED) {
			mss_state = MSS_MENU_INIT;
		}
		runIndex = 0;
		runStartTime = st_millis();
		moveTo = 0;
	}
}

/*
 * Run Mode
 * NOTE: When building the servoPos array, the timeStamp starts at 0 and F-Time is added to each step.
 * CAUTION: Large movement can occur if LAST frame and FIRST frame are not the same.
 */
void mss_runService()
{
	uint8_t data;
	uint32_t frameTime;

	frameTime = st_millis() - runStartTime;

	if(frameTime >= moveTo) {
		// Check for last Frame AFTER allowing move to new position.
		if((runIndex == MAX_FRAMES) || (servoPos[runIndex].timeStamp == 0)) {
			if(runLoop == true) {
				// Reset parameters and rerun.
				runIndex = 0;
				runStartTime = st_millis();
				moveTo = 0;
			} else {
				mss_state = MSS_RUN_MENU;
			}
			return;						// EXIT even though should never execute next block.
										// timeStamp is still half a seqStep away.
		}

		if(frameTime >= servoPos[runIndex].timeStamp ) {		// are we there yet?
			moveTo = frameTime + seqStep;					// allow time to move to new position.
			// Update servos
			for(int i=0; i<16; i++) {
				mp_setPos(i, (MP_CENTER + servoPos[runIndex].servo[i]) - 128);
				
				// TEST DELAY
//				frameTime = st_millis();
//				while( frameTime != st_millis());
			}
			snprintf(textBuf, 17, "FRAME: %2d      ", runIndex);
			mod_stdio_print(textBuf, 16);

			++runIndex;
		}
	}
	
	// Check for EXIT request
	data = mss_getButtons();
	if( data != 0 ) {
		mss_state = MSS_RUN_MENU;	// Press ANY button to STOP.
	}
}

/*
 * Manual Mode
 */
void mss_manualMode()
{
	uint8_t nbytes;
	uint8_t data;

	for( uint8_t i=0; i<8; i++)
	{
		// Get ADC channels last reading.
		currentAdc[i] = adc_support_readChan(i);			// 0 -> 255
		mp_setPos(i, (MP_CENTER + currentAdc[i]) - 128);		// using i+8 controls second set of 8 servos.

		if(i == 0) {
			nbytes = snprintf(textBuf, 17, "SERVO[1]: %+03d   ", (int)currentAdc[i] - 128);
			mod_stdio_print(textBuf, nbytes);
		}
	}

	// Update Servos
	for(int i=8; i<16; i++)
	{
		currentAdc[i] = adc_support_readChan(i);			// 0 -> 255
		mp_setPos(i, (MP_CENTER + currentAdc[i]) - 128);		// using i+8 controls second set of 8 servos.
	}

	// Check for EXIT request
	data = mss_getButtons();
	if((data & BUTTON_RED) == BUTTON_RED) {
		mss_state = MSS_MENU_INIT;	// Press ANY button to STOP.
	}
}

void mss_zeroMode()
{
	for(int i=0; i<16; i++) {
		mp_setPos(i, MP_CENTER);
	}
	mss_state = MSS_MENU_INIT;	// Press ANY button to STOP.
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
	uint8_t data;
	uint8_t tmp;
	uint32_t dTime;

	textBuf[0] = 0x01;
	// Send packet.
	tim_write( MOD_LCD_I2C, (uint8_t*)textBuf, 1 );

	dTime = st_millis() + 1;
	while(dTime > st_millis());		// wait 1 ms
	
	// NOTE: Could use a time-out here of 5ms
	tim_read( MOD_LCD_I2C, 1 );
	// Wait for read to finish
	while( tim_isBusy());

	// get data.
	data = tim_readData();
	tmp = data;
	// ERROR CATCH
	if(data > 7) data = 0;
	
	// DEBUG - Echo Buttons to LED
	for(int i=0; i<8; i++)
	{
		if((tmp & 0x80) == 0x80) {
			mod_led_on();
		} else {
			mod_led_off();
		}
		tmp = tmp << 1;
	}
	mod_led_off();

	return(data);
}

/*
 * Servo position check table.
 * Timestamp is set by the calling process.
 * Value are the same as ADC readings..0:255 with 128 as center.
 */
const uint8_t mss_calTable[][16] PROGMEM = {
	{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	{ 200,  50,  50,  50, 128, 128, 128, 128, 128, 128,  50,  50, 128, 128, 128, 128},
	{  50,  50, 200,  50, 128, 128, 128, 128, 128, 128,  50, 200, 128, 128, 128, 128},
	{  50, 200, 200, 200, 128, 128, 128, 128, 128, 128, 200, 200, 128, 128, 128, 128},
	{ 200, 200,  50, 200, 128, 128, 128, 128, 128, 128,  50, 200, 128, 128, 128, 128},
	{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
	{ 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}
};
