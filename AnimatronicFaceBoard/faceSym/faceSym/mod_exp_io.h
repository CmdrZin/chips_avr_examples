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
 * mod_exp_io.h
 *
 * Created: 1/19/2018	0.01	ndp
 *  Author: Chip
 */ 

#ifndef MOD_EXP_IO_H_
#define MOD_EXP_IO_H_

#define DEV_EXPIO_DDR		DDRB
#define DEV_EXPIO_PORT		PORTB
#define DEV_EXPIO_PIN0		PB0
#define DEV_EXPIO_PIN1		PB1
#define DEV_EXPIO_PIN2		PB2
#define DEV_EXPIO_PIN3		PB3

void mod_expio_init();
void mod_expio_set( uint8_t val );

#endif /* MOD_EXP_IO_H_ */
