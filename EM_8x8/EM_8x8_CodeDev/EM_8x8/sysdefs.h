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
 * sysdefs.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 */ 


#ifndef SYSDEFS_H_
#define SYSDEFS_H_

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif


/* General purpose struct for init() and service(), neither return values. */
/* Access only puts data into the output fifo to be read. Does not return data. */
typedef struct
{
	const uint16_t	id;
	void		(*function)();
} MOD_FUNCTION_ENTRY;

typedef struct
{
	const uint16_t	id;
	const MOD_FUNCTION_ENTRY*	cmd_table;		// address of the command table for the device ID.
} MOD_ACCESS_ENTRY;

typedef struct
{
	const uint8_t	c0;			// icon image elements
	const uint8_t	c1;
	const uint8_t	c2;
	const uint8_t	c3;
	const uint8_t	c4;
	const uint8_t	c5;
	const uint8_t	c6;
	const uint8_t	c7;
} ICON_DATA;

#endif /* SYSDEFS_H_ */