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
 * mod_epLog.h
 *
 * Created: 3/10/2017 12:57:34 PM
 *  Author: Chip
 */ 


#ifndef MOD_EPLOG_H_
#define MOD_EPLOG_H_

#include <time.h>

typedef enum epTags { EPT_NONE, EPT_ACK, EPT_SNOOZE, EPT_NOACTION} EPT_TAG;

void me_init();
void me_store1(EPT_TAG tag, time_t event);
void me_store2(EPT_TAG tag, time_t event1, time_t event2);
void me_clear();
uint8_t me_getLog(uint8_t init, char* buff);

#endif /* MOD_EPLOG_H_ */