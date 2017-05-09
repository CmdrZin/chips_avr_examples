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
 * mod_eventList.c
 *
 * Created: 2/2/2017 2:11:28 PM
 *  Author: Chip
 */ 
#include <avr/io.h>
#include <time.h>
#include <string.h>

#include "mod_eventList.h"
#include "sysTimer.h"

time_t	me_events[ME_EVENTS];
time_t	me_lastEvent = 0;

char	me_eventStr[24];

/*
 * Add event time to event list at index.
 */
void mod_eventList_add(uint8_t index, time_t event)
{
	if(index < ME_EVENTS) {
		me_events[index] = event;
	}
}


/*
 * Get event time at index from event list
 */
char* mod_eventList_get(uint8_t index)
{
	if(index < ME_EVENTS) {
		const time_t	event = me_events[index];
		const struct tm* timeStrut = localtime(&event);
		return isotime(timeStrut);
	} else {
		return strcpy(me_eventStr,"No Event");				// NULL terminated string.
	}
}

time_t me_getLastEvent()
{
	return	me_lastEvent;
}

/*
 * Check list for a time less than local time.
 */
char mod_eventList_check()
{
	time_t	localTime = st_getLocalTime();
	char tone;

	for(uint8_t i=0; i<ME_EVENTS; i++) {
		if( (me_events[i] != 0) && (me_events[i] < localTime) ) {
			const time_t* eHit = &me_events[i];
			tone = asctime(localtime(eHit))[18] - '0';
			// Save Event Time.
			me_lastEvent = me_events[i];
			// Zero Event from list.
			me_events[i] = 0;
			return tone;
		}
	}
	return 10;
}
