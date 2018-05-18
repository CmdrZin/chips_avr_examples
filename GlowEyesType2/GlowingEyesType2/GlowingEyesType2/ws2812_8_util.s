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
 * ws2812_8_util.s			Uses 'C' preprocessor on source.
 *
 * Created: 5/17/2018	0.01	ndp
 *  Author: Chip
 *
 * Support utilities interfacing to the WS2812/SK6812 RGB LED device.
 * Target: Atmel devices with 8MHz clock. 125ns cycle.
 *
 */ 

/*
 * C_ASM_ICD
 * Can use r18:27, r30:31 in ASM without saving
 * X:r27,r26	Y:r29,r28	Z:r31,30	r1=0 always
 */ 
#include <avr/io.h>

#if 1					// Enable for ATtiny85 or 84A
WS2812_DDR	= DDRB
WS2812_PORT	= PORTB
WS2812_PIN	= PORTB0
#else					// Enable for ATmega164a or 328P series
WS2812_DDR	= DDRC
WS2812_PORT	= PORTC
WS2812_PIN	= PORTC6
#endif

    .section .text

 .global ws2812_8_update
/*
 * extern void ws2812_update( void* ptr, int count );
 *
 * Output count number of table RGB values to WS2812_PIN using NRZ
 * 0: 0.35us (7) H then 0.80us (16) L
 * 1: 0.70us (14) H then 0.60us (12) L
 * NOTE: Low time can be longer. * 
 * Leave last bit ending in H state
 * Reset is >80us L state
 *
 * r25:24 = table	MSB,LSB
 * r23:22 = number of entries
 * table has 3 bytes per entry. BGR values
 * returns 0 (void)r25:24
 *
 * ASSUMES a 125ns clock (8 MHz)
 */
ws2812_8_update:
	; move table pointer into X for indexing RAM
	mov		XH, r25		; set H byte
	mov		XL, r24		; set L byte
	; check for zero count
	mov		r24, r22
	or		r24, r23
	brne	wuSkip00	; EXIT if Zero
	ret
wuSkip00:
	;
	; DISABLE INTERRUPTS. This code is time critical.
	cli
	;
	; non-zero count..setup IO
	cbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN
	sbi		_SFR_IO_ADDR(WS2812_DDR), WS2812_PIN		; set as output
	; keep Low for >80us. Use 800 x 125ns = 100us.
	; load r24 with 200 (800/4) and count down to 0.
	ldi		r24, 200
wuLoop01:						; nominal 4 cycles..8
	nop							; (1)
	dec		r24					; (1)
	brne	wuLoop01			; (t.2) (f.1)
	;
	; Output loop. X->list. r23:22 -> count
	;
	ldi		r25, 8							; (1) load bit count
	;
	; Test bit
	ld		r24, X+							; (2) d = *X++   Get byte.
	rol		r24								; (1) move test bit into CY
	brcs	wuSkip01						; (f.1) (t.2) go to Output a 1.
	;																			
wuLoop06:									; aux entry.						Ex	b0	b1
	nop										; (1)								-	10	-
wuLoop04:									; aux entry.						Ex	b0	b1
	nop										; (1)								-	11	-
	; Output a 0 [0.30us or 0.35us]. H for 3 cycles (0.375us)
wuLoop02:
	sbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Always start H
	nop										; (1)									2
	; start 2nd phase.  L for [0.80us or 0.90us + 0.15us] 7 cycles (0.875us)
	cbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Set L for 7 cycles (0.875us) and do next check
	dec		r25								; (1) Dec bit count.				2	2	-	
	breq	wuSkip03						; (f.1) (t.2) Branch if done.		4	3	-
	; Finish phase 2.
wuSkip05:
	rol		r24								; (1) move test bit into CY.		-	4	-	13
	brcc	wuLoop02						; (f.1) (t.2) test bit. ouput a 0.	5	6	-	15
	rjmp	wuLoop03							; (2) output a 1					7	-	-	17	
	;
	; Test for Exit. Get to wuSkip05 as son as possible if continuing.
wuSkip03:
	; dec byte count and ckeck for 0.
	subi	r22, 1							; (1)								5	5	-
	sbc		r23, r1							; (1) r1 = 0 always					6	6	-
	breq	wuExit06						; (f.1) (t.2) Done. Leave line H.	8	7	-
	ldi		r25, 8							; (1) reload bit count				-	8	-
	ld		r24, X+							; (2) d = *X++   Get byte.			-	10	10
	rjmp	wuSkip05						; (2)								-	12	12
	;
wuSkip01:
	;  Output a 1 [0.70us or 0.60us]. H for 5 cycles (0.625us)
wuLoop03:
	sbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Always start H	Ex	b0	b1
	dec		r25								; (1) Dec bit count.				2	-	2	
	breq	wuSkip02						; (f.1) (t.2) Branch if done.		4	-	3
	; Not done yet. Finish 1st phase.
	nop										; (1)								-	-	4
	; start 2nd phase.  L for [0.60us] 5 cycles (0.625us)
wuLoop05:
	cbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Set L for 5 cycles (0.625us)	Ex	b0	b1
	nop										; (1)								-	-	2
	rol		r24								; (1) move test bit into CY.		-	-	3
	brcc	wuLoop02						; (f.1)(t.2)						-	5	4
	rjmp	wuLoop03						; (2)								-	-	6
	;
	; 8 bits sent. Check byte count. Load next if not 0. Get to wuLoop05 as soon as possible.
wuSkip02:
	; dec byte count and ckeck for 0.
	subi	r22, 1							; (1)								4	-	4
	sbc		r23, r1							; (1) r1 = 0 always					5	-	5
	breq	wuExit04						; (f.1) (t.2) Done. Leave line H.	7	-	6
	ldi		r25, 8							; (1) reload bit count				-	-	7
	ld		r24, X+							; (2) d = *X++   Get byte.			-	-	9
	rjmp	wuLoop05						; (2)								-	-	11
	;
	; Finish 1st of 1 and then do 2nd phase and exit.
wuExit04:
	cbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Set L for 5 cycles (0.625us)	Ex	b0	b1
	nop										; (1)								-	-	3
	nop										; (1)								-	-	4
wuExit06:
	sbi		_SFR_IO_ADDR(WS2812_PORT), WS2812_PIN		; (2) Leave line H.
wuExit:
	ret
