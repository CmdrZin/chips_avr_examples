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
 */

/*
 * mod_em_service.h
 *
 * Created: 1/13/2016	0.01	ndp
 *  Author: Chip
 * revision: 2/01/2016	0.02	ndp	Added marquee support
 */ 


#ifndef MOD_EM_SERVICE_H_
#define MOD_EM_SERVICE_H_

#define MOD_EM_SERVICE_ID	0x20

#define MES_SET_ICON		0x01
#define MES_LOAD_ICON		0x02
#define MES_SEL_MARQUEE		0x40
#define MES_SET_KERNING		0x41
#define MES_SET_CONT_FLAG	0x42
#define MES_SET_RATE		0x43
#define MES_LOAD_MSERIES	0x44

#define MES_STORE_ICON		0x20

void mod_em_service_init();
void mod_em_service_service();

void mes_setIcon();
void mes_loadIcon();
void mes_storeIcon();

void mes_setAnimDelay();
void mes_setAnimSequence();

void mes_selectMarquee();
void mes_setKerningFlag();
void mes_setContinuousFlag();
void mes_setRate();
void mes_setIconSeries();


#endif /* MOD_EM_SERVICE_H_ */