/*
 * Tone Generation Utilities
 *
 * org: 8/04/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include sig_generate.asm
 *
 * General:
 * 	R00 - R15 are protected registers. Restore if used.
 * 	R16 - R16
 *  R17 - R31 are transfer and local registers. NOT preserved.
 *
 * Resources;
 *
 * Process:
 *   Call any of the buffer load routines after getting a Z pointer
 *   for the desired tone.
 *   Call tgu_init to prep the buffer data for output.
 *   Set T bit = 1 to enable output code.
 *   Set T bit = 0 to disable output code.
 *
 *
 *	SYMTDLY(2):	Timeout Delay. N=Symbol delays to wait for next Symbol. Default = 3.
 *
 *	SYMRATE(3):	Symbol Rate. Rate to send Symbols. N=n * 10ms. Default = 10.
 *
 *	SYMDUR(4): 	Symbol Duration. Time to hold Tone output. N=n * 10 ms. Default = 5.
 */ 

.DSEG
// Send Parameters
tgu_sample_cnt:		.BYTE		1		; index into tone buffer.
tgu_buffer:			.BYTE		33		; tone output buffer,
;
tgu_msg_buff:		.BYTE		8		; Outgoing message buffer
tgu_msg_status:		.BYTE		1		; Message buffer status
										; b3:0..byte count..cnt down to 0.
tgu_delay_cnt:		.BYTE		1		; SYMDUR: time on delay for each tone..N*10ms
										; Set MSB if dead time.
tgu_sym_rate:		.BYTE		1		; SYMRATE: New Symbol sent.

tgu_dead_dly:		.BYTE		1		; Dead time = Sym rate - Sym duration.

tgu_saveHL:			.BYTE		2		; save X

// Receive Parameters
tgu_sym_dly:		.BYTE		1		; SYMDLY: Max wait for next symbol.


;
.CSEG

.equ	TONE_TIME	= 5			; N * 10ms


/*
 * Translate bit pattern to dual tones
 * input reg:	R17 = Value .. 0x00-0x0F
 * output reg:	none
 *
 * Translation Table
 *	Value	Tones	Indexes
 *	  0x00	1+5		0,4
 *	  0x01	1+6		0,5
 *	  0x02	1+7		0,6
 *	  0x03	1+8		0,7
 *	  0x04	2+5		1,4
 *	  0x05	2+6		1,5
 *	  0x06	2+7		1,6
 *	  0x07	2+8		1,7
 *	  0x08	3+5		2,4
 *	  0x09	3+6		2,5
 *	  0x0A	3+7		2,6
 *	  0x0B	3+8		2,7
 *	  0x0C	4+5		3,4
 *	  0x0D	4+6		3,5
 *	  0x0E	4+7		3,6
 *	  0x0F	4+8		3,7
 *
 * Tone A = (val>>2) & 0x03		.. 0-3
 * Tone B = (val & 03) + 0x04	.. 4-7
 *
 * Call this to load a tone pair into the buffer.
 */
tgu_bits2tones:
	push	R17		; save input
; get A Tone.
	lsr		R17
	lsr		R17
	andi	R17, 0x03
	call	tgu_get_table		; Z = table
	call	tgu_copy_tone		; table -> buffer
; add B Tone
	pop		R17
	andi	R17, 0x03
	ldi		R16, 0x04
	add		R17, R16
	call	tgu_get_table		; Z = table
	call	tgu_sum_tone
;
	ret

/*
 * Initialize output IO and counter for tone generation.
 * Also shift bits up to align with b7:b1
 *
 * Set X = tgu_buffer
 * Set tgu_sample_cnt = 32 (counts down.)
 *
 */
tgu_init:
	ldi		XH, high(tgu_buffer)		; set up X (R27:R26) = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R18, 32					; table size
;
tgui_loop00:
	ld		R17, X
; Convert and set DC offset to 0x40..7bit value.
	subi	R17, 0xC0					; sub (-0x40)
;
	lsl		R17							; Adjust up 1 bit to match hardware.
	st		X+, R17
	dec		R18
	brne	tgui_loop00
;
	ldi		XH, high(tgu_buffer)		; set up X = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R18, 32					; table size
	sts		tgu_saveHL, XH
	sts		tgu_saveHL+1, XL
	sts		tgu_sample_cnt, R18
	ldi		R18, 0xFE					; b7:1 as outputs..b0 is ADC0 input
	out		DDRA, R18
	ret

/*
 * Zero tone buffer.
 * input reg:	none
 * output reg:	none
 * resources:	R16, R17
 *				X (r27:r26)
 */
tgu_zero_tone:
	ldi		XH, high(tgu_buffer)		; set up X = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R17, 32						; table size
	clr		R16
;
tguzt_loop00:
	st		X+, R16
	dec		R17
	brne	tguzt_loop00
;
	ret

/*
 * Use Tone Index to generate sin table address.
 * input reg:	R17 = index
 * output reg:	Z (r31:r30) = sin table address
 * resources:	R16
 */
tgu_get_table:
	ldi		ZH, high(sin_tables<<1)	; Initialize Z pointer
	ldi		ZL, low(sin_tables<<1)
	lsl		R17					; double for 16 bit address entries
;
	clr		R16
	add		ZL, R17
	adc		ZH, R16
;
	lpm		R16, Z+
	lpm		R17, Z
	mov		ZL, R16			; LSB, MSB
	mov		ZH, R17
;
	ret

/*
 * Copy tone table to output buffer.
 * input reg:	Z (r31:r30) - sin table address in FLASH
 * output reg:	none
 * resources:	R16, R17, X (r27:r26)
 */
tgu_copy_tone:
	ldi		XH, high(tgu_buffer)		; set up X = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R17, 32						; table size
;
tguct_loop00:
	lpm		R16, Z+
	st		X+, R16
	dec		R17
	brne	tguct_loop00
;
	ret

/*
 * Sum this tone with tone buffer.
 * Add sample point and divide by 2. (Signed values.)
 * input reg:	Z (r31:r30) - sin table address in FLASH
 * output reg:	none
 * resources:	R16, R17, R18, X (r27:r26)
 *
 * NOTE: Also convert to UNSIGNED centered about 0x40 as 0.
 */
tgu_sum_tone:
	ldi		XH, high(tgu_buffer)		; set up X = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R17, 32						; table size
;
tgust_loop00:
	lpm		R16, Z+
	ld		R18, X						; get current sample
	add		R16, R18
	brmi	tgust_skip00				; Sign bit set?
	clc									; no
	rjmp	tgust_skip01
;
tgust_skip00:
	sec									; yes
tgust_skip01:
	ror		R16						; signed div by 2 w/ sign extend
	st		X+, R16
	dec		R17
	brne	tgust_loop00
;
	ret

/*
 * Output next sample point from buffer.
 * Called from/as 125us Timer B interrupt.
 * This code will cycle through the buffer until the T bit is cleared to 0.
 *
 * NOTE: X has to be saved!!!!
 */
tgu_gen_tone:
	brts	tgut_skip00				; T=1 to enable.
	reti
;
tgut_skip00:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R17
	push	XH
	push	XL
;
	lds		XH, tgu_saveHL
	lds		XL, tgu_saveHL+1
;
	ld		R17, X+				; get next sample
	andi	R17, 0xFE			; mask?
	out		PORTA, R17
;
	lds		R17, tgu_sample_cnt
	dec		R17
	brne	tgut_skip01
; reset tone output system
;
	ldi		XH, high(tgu_buffer)	; set up X (r27:r26) = tgu_buffer
	ldi		XL, low(tgu_buffer)
	ldi		R17, 32				; table size
;
tgut_skip01:
	sts		tgu_saveHL, XH
	sts		tgu_saveHL+1, XL
	sts		tgu_sample_cnt, R17	; update
;
	pop		XL
	pop		XH
	pop		R17
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti

/*
 * Flash Storage of Sin Tables for 7 bit DAC
 * Table is 32 points @ a fixed interval (0.000125us or 8kps nominal) so
 * that all tables generate N complete sin waves. All waves are from 0x00
 * to 0x7F with center is at 0x3F.
 * NOTE: Should be 0x01<->0x40<->0x7F...could use 0x00 as a flag or trigger.
 * Tone table is at the end tables.
 * NOTE: These HAVE to be SIGNED values to SUM them later.. 0x80 is now unused.
 *
 */

; F = 750 Hz @ 8kps
sin_table_3:		; DC offset = 0x3F 
.db		0x00, 0x23
.db		0x3A, 0x3E
.db		0x2D, 0x0C
.db		0xE8, 0xCC
.db		0xC1, 0xCC
.db		0xE8, 0x0C
.db		0x2D, 0x3E
.db		0x3A, 0x23
.db		0x00, 0xDD
.db		0xC6, 0xC2
.db		0xD3, 0xF4
.db		0x18, 0x34
.db		0x3F, 0x34
.db		0x18, 0xF4
.db		0xD3, 0xC2
.db		0xC6, 0xDD

; F = 1000 Hz @ 8kps
sin_table_4:
.db		0x00, 0x2D
.db		0x3F, 0x2D
.db		0x00, 0xD3
.db		0xC1, 0xD3
.db		0x00, 0x2D
.db		0x3F, 0x2D
.db		0x00, 0xD3
.db		0xC1, 0xD3
.db		0x00, 0x2D
.db		0x3F, 0x2D
.db		0x00, 0xD3
.db		0xC1, 0xD3
.db		0x00, 0x2D
.db		0x3F, 0x2D
.db		0x00, 0xD3
.db		0xC1, 0xD3

; F = 1250 Hz @ 8kps
sin_table_5:
.db		0x00, 0x34
.db		0x3A, 0x0C
.db		0xD3, 0xC2
.db		0xE8, 0x23
.db		0x3F, 0x23
.db		0xE8, 0xC2
.db		0xD3, 0x0C
.db		0x3A, 0x34
.db		0x00, 0xCC
.db		0xC6, 0xF4
.db		0x2D, 0x3E
.db		0x18, 0xDD
.db		0xC1, 0xDD
.db		0x18, 0x3E
.db		0x2D, 0xF4
.db		0xC6, 0xCC

; F = 1500 Hz @ 8kps
sin_table_6:
.db		0x00, 0x3A
.db		0x2D, 0xE8
.db		0xC1, 0xE8
.db		0x2D, 0x3A
.db		0x00, 0xC6
.db		0xD3, 0x18
.db		0x3F, 0x18
.db		0xD3, 0xC6
.db		0x00, 0x3A
.db		0x2D, 0xE8
.db		0xC1, 0xE8
.db		0x2D, 0x3A
.db		0x00, 0xC6
.db		0xD3, 0x18
.db		0x3F, 0x18
.db		0xD3, 0xC6

; F = 1750 Hz @ 8kps
sin_table_7:
.db		0x00, 0x3E
.db		0x18, 0xCC
.db		0xD3, 0x23
.db		0x3A, 0xF4
.db		0xC1, 0xF4
.db		0x3A, 0x23
.db		0xD3, 0xCC
.db		0x18, 0x3E
.db		0x00, 0xC2
.db		0xE8, 0x34
.db		0x2D, 0xDD
.db		0xC6, 0x0C
.db		0x3F, 0x0C
.db		0xC6, 0xDD
.db		0x2D, 0x34
.db		0xE8, 0xC2

; F = 2000 Hz @ 8kps
sin_table_8:
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1
.db		0x00, 0x3F
.db		0x00, 0xC1

; F = 2250 Hz @ 8kps
sin_table_9:
.db		0x00, 0x3E
.db		0xE8, 0xCC
.db		0x2D, 0x23
.db		0xC6, 0xF4
.db		0x3F, 0xF4
.db		0xC6, 0x23
.db		0x2D, 0xCC
.db		0xE8, 0x3E
.db		0x00, 0xC2
.db		0x18, 0x34
.db		0xD3, 0xDD
.db		0x3A, 0x0C
.db		0xC1, 0x0C
.db		0x3A, 0xDD
.db		0xD3, 0x34
.db		0x18, 0xC2

; F = 2500 Hz @ 8kps
sin_table_10:
.db		0x00, 0x3A
.db		0xD3, 0xE8
.db		0x3F, 0xE8
.db		0xD3, 0x3A
.db		0x00, 0xC6
.db		0x2D, 0x18
.db		0xC1, 0x18
.db		0x2D, 0xC6
.db		0x00, 0x3A
.db		0xD3, 0xE8
.db		0x3F, 0xE8
.db		0xD3, 0x3A
.db		0x00, 0xC6
.db		0x2D, 0x18
.db		0xC1, 0x18
.db		0x2D, 0xC6

; F = 2750 Hz @ 8kps
sin_table_11:
.db		0x00, 0x34
.db		0xC6, 0x0C
.db		0x2D, 0xC2
.db		0x18, 0x23
.db		0xC1, 0x23
.db		0x18, 0xC2
.db		0x2D, 0x0C
.db		0xC6, 0x34
.db		0x00, 0xCC
.db		0x3A, 0xF4
.db		0xD3, 0x3E
.db		0xE8, 0xDD
.db		0x3F, 0xDD
.db		0xE8, 0x3E
.db		0xD3, 0xF4
.db		0x3A, 0xCC

; F = 3000 Hz @ 8kps
sin_table_12:
.db		0x00, 0x2D
.db		0xC1, 0x2D
.db		0x00, 0xD3
.db		0x3F, 0xD3
.db		0x00, 0x2D
.db		0xC1, 0x2D
.db		0x00, 0xD3
.db		0x3F, 0xD3
.db		0x00, 0x2D
.db		0xC1, 0x2D
.db		0x00, 0xD3
.db		0x3F, 0xD3
.db		0x00, 0x2D
.db		0xC1, 0x2D
.db		0x00, 0xD3
.db		0x3F, 0xD3

; F = 3250 Hz @ 8kps
sin_table_13:
.db		0x00, 0x23
.db		0xC6, 0x3E
.db		0xD3, 0x0C
.db		0x18, 0xCC
.db		0x3F, 0xCC
.db		0x18, 0x0C
.db		0xD3, 0x3E
.db		0xC6, 0x23
.db		0x00, 0xDD
.db		0x3A, 0xC2
.db		0x2D, 0xF4
.db		0xE8, 0x34
.db		0xC1, 0x34
.db		0xE8, 0xF4
.db		0x2D, 0xC2
.db		0x3A, 0xDD

; List of sin tables index 0-7 by tone.
sin_tables:
;.dw		sin_table_3<<1		; Tone x
.dw		sin_table_4<<1		; Tone 1
.dw		sin_table_5<<1		; Tone 2
.dw		sin_table_6<<1		; Tone 3
.dw		sin_table_7<<1		; Tone 4
.dw		sin_table_8<<1		; Tone 5
.dw		sin_table_9<<1		; Tone 6
.dw		sin_table_10<<1		; Tone 7
.dw		sin_table_11<<1		; Tone 8
;.dw		sin_table_12<<1		; Tone x
;.dw		sin_table_13<<1		; Tone x
