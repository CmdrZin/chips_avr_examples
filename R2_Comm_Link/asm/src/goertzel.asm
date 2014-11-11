/*
 * Goertzel Tone Detector Processes
 *
 * org: 7/29/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include goertzel.asm
 *
 * General:
 * 	R3 - Free (1)
 * 	R0:2 & R16:R23 - Used by Fractional Math library (avr_201.asm)
 *  R4:R15 - Used by Goertzel Algorythm. Must be left alone.
 *	R24:27 - Free (4)
 *	R28:R31 - Used by 8 Tone process. Must be left alone if processing 8 tones.
 *
 * Resources;
 * 		gortz_loop		1 byte
 *
 * NOTES:
 * 	Farctional rep
 *		01111111 11111111	 0.99994
 *		01000000 00000000	 0.50000000000000
 *		00100000 00000000	 0.25000000000000
 *		00010000 00000000	 0.12500000000000
 *		00001000 00000000	 0.06250000000000
 *		00000100 00000000	 0.03125000000000
 *		00000010 00000000	 0.01562500000000
 *
 * 		00000001 00000000	 0.00781250000000
 * 		00000000 10000000	 0.00390625000000
 * 		00000000 01000000	 0.00195312500000
 * 		00000000 00100000	 0.00097653250000
 * 		00000000 00010000	 0.00048828125000
 * 		00000000 00001000	 0.00024414062500
 * 		00000000 00000100	 0.00012207031250
 * 		00000000 00000010	 0.00006103515625
 *
 *		00000000 00000000	 0.00000000000000
 *
 *		11111111 11111110	-0.00006103515625
 *
 *		10000000 00000001	-0.99994
 *		10000000 00000000	-1.00000
 *
 *	ADC>>6..MSB is Sign Bit (inverted)
 *		00000001 11111111		+511 = +1.22v		 0.99997
 *
 *		00000001 11110100		+500 = +1.00v		 0.9765625
 *
 *		00000000 00000001		  +1 = +0.04v		 0.001953125
 *		00000000 00000000		   0 =  0.00v		 0.00000
 *		11111111 11111111		  -1 = -0.04v		-0.001953125
 *
 *		11111110 00001100		-500 = -1.00v		-0.9765625
 *
 *		11111110 00000000		-512 = -1.28v		-1.00000
 */ 

.def	xnH		= R15	; input data sample (->b9:b3)
.def	xnL		= R14	; input data sample (b2:b0<-)
.def	wnH		= R13	; w(n).H
.def	wnL		= R12	; w(n).L
.def	wn1H	= R11	; w(n-1).H
.def	wn1L	= R10	; w(n-1).L
.def	wn2H	= R9	; w(n-2).H
.def	wn2L	= R8	; w(n-2).L
.def	h1H		= R7	; h1.H
.def	h1L		= R6	; h1.L

.DSEG
gortz_loop:			.BYTE	1		; Goertzel Loop Count (64)
gortz_tone_loop:	.BYTE	1		; Tone index (8)
gortz_det:			.BYTE	1		; detect flags..MSB:Tone 8 .. LSB:Tone 1
gortz_lev:			.BYTE	1		; detection threshold level

.CSEG


/*
 * Goertzel Initialization - 1 Tone
 * input reg:	none
 * output reg:	none
 *
 * Called once from main()
 *
 */
goertzel_init:
	clr		wnH				; w(n)   = 0.0
	clr		wnL
	clr		wn1H			; w(n-1) = 0.0
	clr		wn1L
	clr		wn2H			; w(n-2) = 0.0
	clr		wn2L
;
;	clr		R16
;	sts		gortz_det, R16	; clear detect flags
;
	ldi		R16, 0x20
	sts		gortz_lev, R16	; set detection threshold level
;
; Load coef since 8 tones will need it pre-set from FLASH.
; NOTE: This should come from SRAM.
	ldi		R17, 1			; Tone 4
	call	goertzel_get_h1
;
; Set Loop count
	ldi		R16, 64
	sts		gortz_loop, R16
;
	ldi		R16, SCF_NO_TONE_IN		; disable Tone Detect Processing
	sts		slave_flags, R16
;
	ret

/*
 * Use Tone Index to retrieve h1 from table
 * input reg:	R17 = index
 * output reg:	h1 = 16bit coef (hiH,hiL)
 */
goertzel_get_h1:
	ldi		ZH, high(h1_table<<1)	; Initialize Z pointer
	ldi		ZL, low(h1_table<<1)
	lsl		R17					; double for 16 bit address entries
;
	clr		R16
	add		ZL, R17
	adc		ZH, R16
;
	lpm		h1L, Z+			; LSB, MSB
	lpm		h1H, Z
;
	ret

/*
 * Goertzel Tone Detector - 1 Tone
 * input reg:	adc_buffH, adc_buffL
 * output reg:	R17= Tone # (0-7)
 * 				R18 = 1 if valid
 *
 * Called every 125us (8kps) (i.e. each ADC sample)
 *
 * w(n) = h1*w(n-1) - w(n-2) + x(n)		h1 = 2cos(2PIm/64)		m=4,6,7,8,9,10,11,13
 * Clocks: 
 *
 * resources:	R0:2, R4:15, R16:23, R28:31
 *	r23:r20			A & B - inputs to math
 *	r19:r16			C - output of math	C = A * B
 * IIR code usage (1 & 8 tones)
 *	r15	x(n).H		input data sample (b9:b8)
 *	r14	x(n).L		input data sample (b7:b0)
 *	r13	w(n).H
 *	r12	w(n).L
 *	r11	w(n-1).H
 *	r10	w(n-1).L
 *	r9	w(n-2).H
 *	r8	w(n-2).L
 *	r7	h1.H
 *	r6	h1.L
 *
 * NOTE: ADC data adjusted values (div by 64) between +0.99997 to -1.0000 (+1.22v -> -1.28v)
 */
goertzel_1_tone:
// get sample..already adjusted if needed..ADH -> x(n).H, ADL -> x(n).L
	lds		r16, adc_buffH
	mov		xnH, r16
	lds		r16, adc_buffL
	mov		xnL, r16
// propigate last data
// w(n-1).H -> w(n-2).H, w(n-1).L -> w(n-2).L
	mov		wn2H, wn1H
	mov		wn2L, wn1L
// w(n).H -> w(n-1).H, w(n).L -> w(n-1).L
	mov		wn1H, wnH
	mov		wn1L, wnL
// calc w(n) = h1*w(n-1) - w(n-2) + x(n)
; w(n) = 0.5*h1*w(n-1)*2
	mov		r23, h1H		; h1.H -> r23, h1.L -> r22
	mov		r22, h1L
	mov		r21, wn1H		; w(n-1).H -> r21, w(n-1) -> r20
	mov		r20, wn1L
	call	fmuls16x16_32	; results in r19:r16
; adjust because using h1 divided by 2.
	lsl		r17
	rol		r18
	rol		r19
; w(n) = w(n) + x(n) = h1*w(n-1) + x(n)
	mov		wnL, r18
	mov		wnH, r19
	add		wnL, xnL
	adc		wnH, xnH
;	w(n) = w(n) - w(n-2) =  h1*w(n-1) - w(n-2) + x(n)
	sub		wnL, wn2L
	sbc		wnH, wn2H
; Check if done.
	lds		r16, gortz_loop
	dec		r16
	sts		gortz_loop, r16
	brne	g1t_skip10
; Check magnitude and save results.
	mov		r17, wnH			; get last calc
	lds		r16, gortz_lev
	cp		r16, r17			; check level
	brge	g1t_skip08
	ldi		r16, 0b00000001		; Tone detected
	rjmp	g1t_skip09
g1t_skip08:
	ldi		r16, 0b00000000		; no Tone
g1t_skip09:
	sts		gortz_det, r16
;
	call	goertzel_init			; clears results and overwrites gortz_loop.
;
	ldi		R17, 1					; testing for index 1 (Tone 4)
	lds		R18, gortz_det
;
g1t_skip10:
	ret								; EXIT
;

; already divided by 2 because some values were outside +/- 1.0
; shift product << 1 to restore.
; Use same index as for freq
h1_table:	
.dw		0x6A6D	; tone 3  750 Hz
.dw		0x5A82	; tone 4 1000 Hz
.dw		0x471C	; tone 5
.dw		0x30FB
.dw		0x18F8
.dw		0x0000
.dw		0xE707
.dw		0xCF04	; tone 10


// Bring in Fractional mat code.
.include "avr201.asm"

