/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * main.c
 *
 * Created: 9/19/2018		0.01	ndp
 * Author : Chip
 *
 * USB Serial Tester 
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "io_led_button.h"
#include "serial.h"
#include "mod_comms.h"
#include "mod_audio.h"

void triggerBeepTone(uint8_t n);

#define B_ACK		IO_GREEN_BUTTON			// (0x01)
#define B_SNOOZE	IO_RED_BUTTON			// (0x02)

typedef enum pStates {S_NORMAL, S_PRECHECK } E_STATE;

char	mBuff[9];

E_STATE	eState;

int main(void)
{
	uint8_t		buttons;
	uint8_t		count = 0;

	long nextTime = st_millis() + 1000;	// set time reference.
	eState = S_PRECHECK;

	// turn OFF watchdog timer to reduce power.
	st_init_tmr0();					// set up timers
	mod_io_init();					// set up Button and LED pins.
	initUsart();					// set up USART mode 9600 8N1
	ma_init();						// set up audio

	sei();

	while (1)
	{
		mod_io_service();							// Scan buttons
		mod_comms_service();
		ma_service();

		// Main process loop
		switch(eState) {
			case S_NORMAL:
				buttons = mod_io_getButtons();
				if(buttons & B_ACK) {
					sendChar('A');
					sendChar(13);		// CR
					sendChar(10);		// LF
				}
				if(buttons & B_SNOOZE) {
					sendChar('S');
					sendChar(13);		// CR
					sendChar(10);		// LF
				}
				// Received characters are handled in mod_comms.c
				break;
					
			case S_PRECHECK:
				if(nextTime < st_millis()) {
					nextTime = nextTime + 1000;
					mod_io_setBlink(100, IO_RED_LED);
					if(++count > 3) {
						count = 0;
						eState = S_NORMAL;
					}
				}
				break;	

			default:
				eState = S_NORMAL;
				break;
				
		} // switch(eState)
	} // while()
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
