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
 * mod_exp_io.c
 *
 * Created: 1/19/2018 11:09:47 AM
 *  Author: Chip
 */ 

#include <avr/io.h>

#include "mod_exp_io.h"

void mod_expio_init()
{
	DEV_EXPIO_DDR |= (1<<DEV_EXPIO_PIN3)|(1<<DEV_EXPIO_PIN2)|(1<<DEV_EXPIO_PIN1)|(1<<DEV_EXPIO_PIN0);			// set HIGH for output
	mod_expio_set(0);
	return;
}

/*
 * Set Exp IO Port
 */
void mod_expio_set( uint8_t val )
{
	DEV_EXPIO_PORT = val;

	return;
}
