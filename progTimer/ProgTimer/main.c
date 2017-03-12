/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * ProgTimer.c
 *
 * Created: 1/30/2017		0.01	ndp
 * Author : Chip
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "sysTimer.h"
#include "io_led_button.h"
#include "serial.h"
#include "mod_comms.h"
#include "mod_audio.h"
#include "mod_eventList.h"
#include "mod_epLog.h"

void triggerBeepTone(uint8_t n);

#define B_ACK		IO_GREEN_BUTTON			// (0x01)
#define B_SNOOZE	IO_RED_BUTTON			// (0x02)

#define S_ACK		10
#define S_SNOOZE	20


bool goToSleep;
uint16_t awakeCount;

char	mBuff[9];
char	mCycle = 0;

uint8_t	eState;
bool	eSelfCheck;

int main(void)
{
	char	eTone = 0;
	uint8_t	buttons;
	uint8_t oneMinTime = 1;
	uint8_t	twoSecTime = 1;
	uint8_t tenMinTime = 1;
	uint8_t tenMinCount = 1;
	bool	eTempEvent;
	bool	ledToggle = false;

	// turn OFF watchdog timer to reduce power.
	wdt_disable();
	st_init_tmr0();					// set up timers
	st_init_tmr2();
	mod_io_init();					// set up Button and LED pins.
	initUsart();					// set up USART mode 9600 8N1
	ma_init();						// set up audio
	me_init();						// set up EEPROM logging

	sei();

	eSelfCheck = true;				// Self Check mode.

	// set up SLEEP
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	goToSleep = true;
	awakeCount = 200;				// N * 10ms
	eState = 0;						// No Event
	eTempEvent = false;

	while (1)
	{
		mod_io_service();
		mod_comms_service();
		ma_service();

		if(goToSleep) {
			if(!ma_isPlaying()) {
				cli();
				sleep_enable();
				sei();
				sleep_cpu();						// sleep for 8s based on Timer2 setup.
				// wait for intr to wake up.
				if(!eTempEvent) {
					eTone = mod_eventList_check();
					if(eTone != 10) {
						eState = 1;
					} else {
						eState = 0;
					}
				}
				// Use eState to further process.
				goToSleep = false;
				if(ledToggle || !eSelfCheck) {
					ledToggle = false;
					mod_io_setBlink(5, IO_GREEN_LED);			// N * 1ms
				} else {
					ledToggle = true;
					mod_io_setBlink(5, IO_RED_LED);				// N * 1ms
				}
			}
		} else {
			if(eSelfCheck) {
				// TODO: Self Check UI
				if( GPIOR0 & (1<<DEV_10MS_TIC) )
				{
					GPIOR0 &= ~(1<<DEV_10MS_TIC);
					if( --awakeCount == 0)
					{
						awakeCount = 50;
						goToSleep = true;
					}
				}
				buttons = mod_io_getButtons();
				if(buttons & B_ACK) {
					triggerBeepTone(5);
					eTempEvent = false;
					goToSleep = true;
					eState = 0;
					eSelfCheck = false;
				}

			} else  if(eState == 0) {
				// Normal 2 second awake checks
				// Stay awake for at least 2s
				if( GPIOR0 & (1<<DEV_10MS_TIC) )
				{
					GPIOR0 &= ~(1<<DEV_10MS_TIC);
					if( --awakeCount == 0)
					{
						awakeCount = 200;
						goToSleep = true;
					}
				}
				// stay awake if comm activity
				if(mod_commGetMode() != MC_IDLE) awakeCount = 1000;		// reset timeout to longer.
			} else {
				// Process Event UI
				buttons = mod_io_getButtons();

				switch(eState) {
					// Event Triggered
					case 1:
						oneMinTime = 30;		// based on two second timer
						twoSecTime = 200;		// based on 10ms TIC
						tenMinTime = 75;				// based on 8 second sleep.
						tenMinCount = 3;
						triggerBeepTone(eTone);
						eState = 2;
						break;

					// Waiting for any button - 1 minute
					case 2:
						if(buttons & B_ACK) {
							eState = S_ACK;
							break;
						}
						if(buttons & B_SNOOZE) {
							eState = S_SNOOZE;
							// oneMin and twoSec already set up.
							tenMinTime = 75;				// based on 8 second sleep.
							eTempEvent = true;				// Use sleep timer for long No Action wait.
							goToSleep = true;
							break;
						}
						// Check 1 minute timeout
						if( GPIOR0 & (1<<DEV_10MS_TIC) )
						{
							GPIOR0 &= ~(1<<DEV_10MS_TIC);

								if( --twoSecTime == 0) {
									twoSecTime = 200;
									triggerBeepTone(eTone);

									if( --oneMinTime == 0) {
										oneMinTime = 30;
										tenMinTime = 75;				// based on 8 second sleep.
										eTempEvent = true;				// Use sleep timer for long No Action wait.
										goToSleep = true;
										eState = 3;
									}
								}
						}
						break;

					// Waiting 10 min - Uses 8 second sleep time
					case 3:
						if( --tenMinTime == 0 ) {
							tenMinTime = 75;

							if( --tenMinCount == 0 ) {
								tenMinCount = 3;
								// No response in 3x ten minute waits.
								me_store1(EPT_NOACTION, me_getLastEvent());
								eTempEvent = false;
								goToSleep = true;
							} else {
								eState = 2;
							}
						}
						break;

					// Waiting 10 min in Snooze mode
					case S_SNOOZE:
						if(buttons & B_ACK) {
							triggerBeepTone(5);
							me_store2(EPT_SNOOZE, me_getLastEvent(), st_getLocalTime());			// Store Event as Snooze then ACK'd.
							eTempEvent = false;
							goToSleep = true;
							eState = 0;
							break;
						}

						// Stay awake for at least 2s
						if( GPIOR0 & (1<<DEV_10MS_TIC) )
						{
							GPIOR0 &= ~(1<<DEV_10MS_TIC);
							if( --awakeCount == 0)
							{
								awakeCount = 200;
								goToSleep = true;
							}
						}

						if(goToSleep) {
							if( --tenMinTime == 0 ) {
								tenMinTime = 75;

								if( --tenMinCount == 0 ) {
									tenMinCount = 3;
									// No response in 3x ten minute waits.
									eTempEvent = false;
								} else {
									// DON'T go back to Sleep. Going to do one min beeps.
									goToSleep = false;
									eState = 2;
								}
							}
						}
						break;

					case S_ACK:
						triggerBeepTone(5);
						me_store1(EPT_ACK, me_getLastEvent());			// Store Event as ACK'd.
						eTempEvent = false;
						goToSleep = true;
						eState = 0;
						break;

					default:
						break;
				}
			}
		}
	}
}


void setSelfCheck()
{
	eSelfCheck = true;
}

void triggerBeepTone(uint8_t tone)
{
	switch(tone) {
		case 0:
		ma_setService(MA_SERVICE_BEEP);
		ma_setBeepParam(MA_BEEP_4k, 250, 2);
		break;

		case 5:
		ma_setService(MA_SERVICE_SCORE2);
		break;

		case 9:
		ma_setService(MA_SERVICE_SCORE1);
		break;

		default:
		ma_setService(MA_SERVICE_BEEP);
		ma_setBeepParam(MA_BEEP_4k, 250, 1);
		break;
	}

}