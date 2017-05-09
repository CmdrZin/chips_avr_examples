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
#include <avr/pgmspace.h>

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

#define M_TWOSECTIME	200
#define M_200MSTIME		20

typedef enum pStates {S_NORMAL, S_PRECHECK, S_EVENT_AWAKE, S_EVENT_ASLEEP, S_SNOOZE_AWAKE, S_SNOOZE_ASLEEP} E_STATE;
//#define S_ACK		10
//#define S_SNOOZE	20

const char version_date[] PROGMEM = {"0.51 2017-MAY-08 17:00:00"};	// svn 6

bool goToSleep;
uint16_t awakeCount;

char	mBuff[9];
char	mCycle = 0;

E_STATE	eState;
bool	preCheck;

int main(void)
{
	char		eTone = 0;
	uint8_t		buttons;
	uint8_t		oneMinTime = 1;
	uint16_t	tenSecTime = 1;
	uint8_t		tenMinTime = 1;
	uint8_t		tenMinCount = 1;
	bool		ledToggle = false;
	bool		skipSleep;

	// turn OFF watchdog timer to reduce power.
	wdt_disable();
	st_init_tmr0();					// set up timers
	st_init_tmr2();
	mod_io_init();					// set up Button and LED pins.
	initUsart();					// set up USART mode 9600 8N1
	ma_init();						// set up audio
	me_init();						// set up EEPROM logging

	sei();

	preCheck = true;				// Self Check mode.

	// set up SLEEP
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	goToSleep = true;
	awakeCount = M_TWOSECTIME;		// N * 10ms
	eState = S_PRECHECK;			// Start in PreCheck mode

	while (1)
	{
		mod_io_service();							// Scan buttons
		mod_comms_service();
		ma_service();
#if 1
		if(ma_isPlaying()) {
			skipSleep = true;
		} else {
			skipSleep = false;
		}

		if(goToSleep && !skipSleep) {
			// Don't go to sleep until Tune/Beep finishes.
			cli();
			sleep_enable();
			sei();
			sleep_cpu();						// sleep for 8s based on Timer2 setup.
			// wait for Timer2 interrupt to wake up.
			// Decide which LED(s) to blink on wake up.
			switch(eState) {
				case S_NORMAL:
					mod_io_setBlink(5, IO_GREEN_LED);
					break;
					
				case S_PRECHECK:
					if(ledToggle) {
						mod_io_setBlink(5, IO_GREEN_LED);
					} else {
						mod_io_setBlink(5, IO_RED_LED);
					}
					ledToggle = !ledToggle;
					break;
					
				case S_SNOOZE_AWAKE:
				case S_SNOOZE_ASLEEP:
					mod_io_setBlink(5, IO_RED_LED);
					break;
					
				case S_EVENT_AWAKE:
				case S_EVENT_ASLEEP:
					mod_io_setBlink(5, IO_BOTH_LED);
					break;
			}
		}

		// stay awake if comm activity
		if(mod_commGetMode() != MC_IDLE) awakeCount = 1000;		// reset timeout to longer.

		// Main process loop
		if(!skipSleep) {
			// Only run this block if coming out of Sleep. Simplifies logic.
			switch(eState) {
				case S_NORMAL:
					// Allow for LED blink after waking. AND 2 sec comms window.
					goToSleep = false;
					if( GPIOR0 & (1<<DEV_10MS_TIC) )
					{
						GPIOR0 &= ~(1<<DEV_10MS_TIC);
						if(--awakeCount == 0) {
							awakeCount = M_TWOSECTIME;
							goToSleep = true;		// sleep the 6 seconds left.
						}
					}

					eTone = mod_eventList_check();	// returns seconds ones value on hit, else returns 10
					if( (eTone < 0) || (eTone > 9) ) {
						// No Event
					} else {
						// Valid Tone/Tune select is 0->9
						triggerBeepTone(eTone);
						
						// Init timers and counters
						oneMinTime = 10;		// based on ten second timer
						awakeCount = M_TWOSECTIME;		// based on 10ms TIC
						tenMinTime = 75;		// based on 8 second sleep.
						tenMinCount = 3;		// maximum number of 10 min waits
						tenSecTime = 1000;		// uses 10ms TIC
						goToSleep = false;		// use manual time control
						eState = S_EVENT_AWAKE;
					}
					break;
					
				case S_PRECHECK:
					// Waiting for PreCheck ACK. Check for seconds after waking, then go back to sleep.
					goToSleep = false;
					if( GPIOR0 & (1<<DEV_10MS_TIC) )
					{
						GPIOR0 &= ~(1<<DEV_10MS_TIC);
						if(--awakeCount == 0) {
							awakeCount = M_TWOSECTIME;
							goToSleep = true;		// sleep the 6 seconds left.
						}
					}
					
					// ACK is checked for 2 seconds after wake up.
					buttons = mod_io_getButtons();
					if(buttons & B_ACK) {
						ma_clearAudio();
						triggerBeepTone(0);
//						triggerBeepTone(4);
						eState = S_NORMAL;
						goToSleep = true;
					}
					break;	

				case S_EVENT_AWAKE:
					// under manual control, not sleeping
					goToSleep = false;
					// Processing EVENT Trigger
					// ACK or Snooze button can be pressed an ANY time.
					buttons = mod_io_getButtons();
					if(buttons & B_ACK) {
						ma_clearAudio();
						triggerBeepTone(0);
						me_store1(EPT_ACK, me_getLastEvent());			// Store Event as ACK'd.
						eState = S_NORMAL;
						goToSleep = true;
						break;
					}

					if(buttons & B_SNOOZE) {
						ma_clearAudio();
						triggerBeepTone(2);
						eState = S_SNOOZE_ASLEEP;
						// oneMin and twoSec already set up.
						tenMinTime = 75;				// based on 8 second sleep.
						goToSleep = true;
						break;
					}

					// Play Tune every 10 seconds
					// After 1 minute. go to sleep for 10 minutes. repeat 3 times.
					if( GPIOR0 & (1<<DEV_10MS_TIC) )
					{
						GPIOR0 &= ~(1<<DEV_10MS_TIC);
						if(--tenSecTime == 0) {
							// Ten second timeout. Play tune/tone every 10 seconds.
							tenSecTime = 1000;
							triggerBeepTone(eTone);			// replay tune.
							
							if(--oneMinTime == 0) {
								// One minute timeout
								oneMinTime = 10;
								// No response. Go to sleep for 10 minutes.
								awakeCount = 200;
								tenMinTime = 75;		// uses 8 sec sleep timer
								eState = S_EVENT_ASLEEP;
								goToSleep = true;
							}
						}
					}
					break;					

				case S_EVENT_ASLEEP:
					// allow 2 seconds to detect ACK or Snooze button, then go to sleep for the remaining 6 seconds.
					if(awakeCount != 0) {
						goToSleep = false;
						// ACK or Snooze button can be pressed within 2 seconds after awake.
						buttons = mod_io_getButtons();
						if(buttons & B_ACK) {
							ma_clearAudio();
							triggerBeepTone(0);
							me_store1(EPT_ACK, me_getLastEvent());			// Store Event as ACK'd.
							eState = S_NORMAL;
							goToSleep = true;
							break;
						}

						if(buttons & B_SNOOZE) {
							ma_clearAudio();
							triggerBeepTone(2);
							eState = S_SNOOZE_ASLEEP;
							// Wait until Snooze ends or is ACK'd to store to Log.
							goToSleep = true;				// Go back to sleep while in Snooze
							break;
						}

						// Manual timer control for 2 seconds
						if( GPIOR0 & (1<<DEV_10MS_TIC) )
						{
							GPIOR0 &= ~(1<<DEV_10MS_TIC);
							if(--awakeCount == 0) {
								goToSleep = true;			// leave manual control. Sleep for last 6 seconds.
							}
						}
					} else {
						// awakeCount == 0 to get here
						awakeCount = M_TWOSECTIME;			// to trigger above section
						// decrement time every 8 seconds and counts on wake up
						if(--tenMinTime == 0) {
							tenMinTime = 75;
							// ten minute time out
							if(--tenMinCount == 0) {
								tenMinCount = 3;
								// gone through 3 ten minute periods. Log it.
								me_store1(EPT_NOACTION, me_getLastEvent());
								eState = S_NORMAL;
								goToSleep = true;
								break;
							} else {
								// Wake up again after 10 minutes.
								tenSecTime = 1;				// immediate trigger of tone.
								eState = S_EVENT_AWAKE;
								goToSleep = true;
								break;
							}
						}
						goToSleep = false;					// to get to above section on next loop
					}
					break;
				
				case S_SNOOZE_AWAKE:
					// Only Snooze for 10 min. No button checks.
					// Check ACK for 2 seconds after waking.
					goToSleep = false;		// use manual control
					// ACK is checked for 1 minute after 8s wakeup while playing tune.
					buttons = mod_io_getButtons();
					if(buttons & B_ACK) {
						ma_clearAudio();
						triggerBeepTone(0);
						me_store2(EPT_SNOOZE, me_getLastEvent(), st_getLocalTime());			// Store Event as Snooze then ACK'd.
						eState = S_NORMAL;
						goToSleep = true;
						break;
					}

					// Play tune every 10 seconds
					// After 1 minute. go to sleep for 10 minutes.
					if( GPIOR0 & (1<<DEV_10MS_TIC) )
					{
						GPIOR0 &= ~(1<<DEV_10MS_TIC);
						if(--tenSecTime == 0) {
							// Ten second timeout. Play tune/tone every 10 seconds.
							tenSecTime = 1000;
							triggerBeepTone(eTone);			// replay tune.
							
							if(--oneMinTime == 0) {
								// One minute timeout
								oneMinTime = 10;
								// No response. Go to sleep for 10 minutes.
								awakeCount = M_TWOSECTIME;
								tenMinTime = 75;		// uses 8 sec sleep timer
								eState = S_SNOOZE_ASLEEP;
								goToSleep = true;
							}
						}
					}
					break;

				case S_SNOOZE_ASLEEP:
					// ACK is checked for 2 seconds after 8s wakeup.
					buttons = mod_io_getButtons();
					if(buttons & B_ACK) {
						ma_clearAudio();
						triggerBeepTone(0);
						me_store2(EPT_SNOOZE, me_getLastEvent(), st_getLocalTime());			// Store Event as Snooze then ACK'd.
						eState = S_NORMAL;
						goToSleep = true;
						break;
					}

					// Allow for ACK check and LED blink for 2s after waking, then go back to sleep.
					goToSleep = false;
					if( GPIOR0 & (1<<DEV_10MS_TIC) )
					{
						GPIOR0 &= ~(1<<DEV_10MS_TIC);
						if(--awakeCount == 0) {
							awakeCount = M_TWOSECTIME;

							// Stay asleep for 10 seconds more
							// decrement time and counts on wake up
							if(--tenMinTime == 0) {
								// ten minute time out
								tenMinTime = 75;
								triggerBeepTone(eTone);	// beep/tune again after 10 min
								eState = S_SNOOZE_AWAKE;
							}
							goToSleep = true;		// sleep the 7.8 seconds left.
						}
					}
					break;
				
				default:
					eState = S_NORMAL;
					break;
				
			} // switch(eState)
		} // if(!skipSleep)
#else
		if(goToSleep) {
			if(!ma_isPlaying()) {
				// Don't go to sleep until Tune/Beep finishes.
				cli();
				sleep_enable();
				sei();
				sleep_cpu();						// sleep for 8s based on Timer2 setup.
				// wait for intr to wake up.
				if(!eTempEvent) {
					eTone = mod_eventList_check();	// returns seconds ones value on hit, else returns 10
					if(eTone != 10) {
						eState = 1;					// Event time hit.
					} else {
						eState = 0;
					}
				}
				// Use eState to further process.
				goToSleep = false;
				if(eState == S_SNOOZE) {
					mod_io_setBlink(5, IO_RED_LED);				// N * 1ms
				} else {
					if(ledToggle || !preCheck) {
						ledToggle = false;
						mod_io_setBlink(5, IO_GREEN_LED);			// N * 1ms
					} else {
						ledToggle = true;
						mod_io_setBlink(5, IO_RED_LED);				// N * 1ms
					}
				}
			}
		} else {
			if(preCheck) {
				if( GPIOR0 & (1<<DEV_10MS_TIC) )
				{
					GPIOR0 &= ~(1<<DEV_10MS_TIC);
					if( --awakeCount == 0)
					{
						awakeCount = 200;
						goToSleep = true;
					}
				}
				buttons = mod_io_getButtons();
				if(buttons & B_ACK) {
					triggerBeepTone(0);
//					triggerBeepTone(4);
					eTempEvent = false;
					goToSleep = true;
					eState = 0;
					preCheck = false;
				}

			} else  if( (eState == 0) && !goToSleep ) {
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
							ma_clearAudio();
							eState = S_ACK;
							break;
						}
						if(buttons & B_SNOOZE) {
							ma_clearAudio();
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
							triggerBeepTone(0);
							me_store2(EPT_SNOOZE, me_getLastEvent(), st_getLocalTime());			// Store Event as Snooze then ACK'd.
							eTempEvent = false;
							goToSleep = true;
							eState = 0;
							break;
						}

						// Stay awake for at least 2s
						if(!goToSleep) {							// just woke up
							if( GPIOR0 & (1<<DEV_10MS_TIC) )
							{
								GPIOR0 &= ~(1<<DEV_10MS_TIC);
								if( --awakeCount == 0)
								{
									awakeCount = 200;
									goToSleep = true;
								}
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
									oneMinTime = 30;		// based on two second timer
									twoSecTime = 200;		// based on 10ms TIC
									eState = 2;
								}
							}
						}
						break;

					case S_ACK:
						triggerBeepTone(0);
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
#endif
	} // while()
}


void setSelfCheck()
{
	preCheck = true;
}

/*
 * Configure Beep or Tune to play.
 */
void triggerBeepTone(uint8_t tone)
{
	switch(tone) {
		case 0:
		ma_setService(MA_SERVICE_BEEP);
		ma_setBeepParam(MA_BEEP_4k, 250, 2);
		break;

		case 1:
		ma_setService(MA_SERVICE_BEEP);
		ma_setBeepParam(MA_BEEP_5k, 250, 3);
		break;

		case 2:
		ma_setService(MA_SERVICE_BEEP);
		ma_setBeepParam(MA_BEEP_3k, 250, 3);
		break;

		case 3:
		ma_setService(MA_SERVICE_SCORE6);
		break;

		case 4:
		ma_setService(MA_SERVICE_SCORE7);
		break;

		case 5:
		ma_setService(MA_SERVICE_SCORE2);
		break;

		case 6:
		ma_setService(MA_SERVICE_SCORE3);
		break;

		case 7:
		ma_setService(MA_SERVICE_SCORE4);
		break;

		case 8:
		ma_setService(MA_SERVICE_SCORE5);
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
