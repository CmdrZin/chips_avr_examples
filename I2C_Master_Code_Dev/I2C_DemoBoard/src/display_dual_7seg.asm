/*
 * Display Dual 7 Segment
 *
 * org: 6/25/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include display_dual_7seg.asm
 *
 * Resources;
 * SRAM
 *   dsp_buff	1 byte for 2 HEX number display
 *
 * NOTE: Change display to use one byte per digit to allow more than 16 charaters.
 * Add -, blank, dp, etc.
 *
 *	NOTE: PORTC shared with key pad scan.
 */ 

/* Segment mapping
 *  SEGa  = PORTD0
 *  SEGb  = PORTD1
 *  SEGc  = PORTD2
 *  SEGd  = PORTD3
 *  SEGe  = PORTD4
 *  SEGf  = PORTD5
 *  SEGg  = PORTD6
 *  SEGdp = PORTD7
 */

.equ	DS1 = PORTC3
.equ	DS2 = PORTC2

.equ	DISPLAY_RATE = 1	; Overall display rate. N x 1ms delay
.equ	DIGIT_RATE = 7		; Single digit display time. N x 1ms

.equ	DS_BLANK	= 0x10	; blank all segs
.equ	DS_DASH		= 0x11	; -
.equ	DS_S		= 0x12	; S
.equ	DS_L		= 0x12	; L
.equ	DS_J		= 0x12	; J
.equ	DS_P		= 0x12	; P
.equ	DS_U		= 0x12	; U


.DSEG
dsp_buff:	.BYTE	2		; Reserve 2 bytes in SRAM
dsp_state:	.BYTE	1		; Reserve 1 byte in SRAM

.CSEG
/*
 * Initialize Dual Display
 *
 * input reg:	none
 * output reg:	none
 *
 * NOTE: Only need to call once.
 *
 */
dsp_dual_init:
	call	dsp_dual_init_io
;
	clr		R16
	sts		dsp_state, R16
;
	ldi		R16, 0x11			; dash
	sts		dsp_buff, R16
	sts		dsp_buff+1, R16
;
	ret

/*
 * Initialize Dual Display IO
 *
 * input reg:	none
 * output reg:	none
 *
 * Set PORTC.2 as out	DS1
 * Set PORTC.3 as out	DS2
 *
 */
dsp_dual_init_io:
	sbi		DDRC, DS1
	sbi		DDRC, DS2
	ret

/*
 * Update Display Buffer
 *
 * input reg:	R17 - data
 * output reg:	none
 *
 * Shift new data into buffers.
 */
dsp_dual_store:
	lds		R16, dsp_buff
	sts		dsp_buff+1, R16
;
	sts		dsp_buff, R17
;
	ret

/*
 * Display and KeyPad Service
 *
 * input reg:	none
 * output reg:	none
 * resource:
 *
 * Call from main in fast loop
 *
 */
display_kp_service:
	sbis	GPIOR0, KP_DS_SW_10MS
	ret
	cbi		GPIOR0, KP_DS_SW_10MS	; clear flag set by interrup
; Service which?
	lds		R16, dsp_state
	tst		R16
	breq	dks_skip000				; DS1
	dec		R16
	breq	dks_skip010				; DS2
	breq	dks_skip020				; KeyPad
;
dks_skip000:
	inc		R16
	sts		dsp_state, R16			; update to do DS2 next time.
	call	dsp_dual_display		; run display service
	ret
;
dks_skip010:
	inc		R16
	sts		dsp_state, R16			; update to do KeyPad next time.
	call	dsp_dual_display		; run display service
	ret
;
dks_skip020:
	clr		R16
	sts		dsp_state, R16			; update to do DS1 next time.
	call	key_pad_service
	ret

/*
 * Display 2 HEX digits
 *
 * input reg:	none
 * output reg:	none
 * resource: 	tic1ms flag
 *
 * Called every 7ms.
 * Display each digit for 2ms. Use 1ms timer tic.
 *
 */
dsp_dual_display:
	lds		R17, dsp_buff
	// Display DS1
	// check for dp. NOT implimented
;
	// output DS1 number
	ldi		R18, 0				; DS1 select
	call	dsp_dual_out
	// Wait N ms
	ldi		R17, DIGIT_RATE		; 2ms wait
	call	dsp_waitNms
	sbi		PORTC, DS1			; disable Display 1 .. PNP drive
	clr		R16					; set all bit as inputs
	out		DDRD, R16
;
	// Display DS2
	// check for dp
;
	// output DS2 number
	lds		R17, dsp_buff+1
	ldi		R18, 1				; DS2 select
	call	dsp_dual_out
	// Wait N ms
	ldi		R17, DIGIT_RATE		; 2ms wait
	call	dsp_waitNms
	sbi		PORTC, DS2			; disable Display 2 .. PNP drive
	clr		R16					; set all bit as inputs
	out		DDRD, R16
;
	ret					; EXIT

/*
 * Output dispay pattern
 * input reg:	R17 - data (0-F or 0x10 for blank)
 *				with R17 b7 = 0 for dp
 * 				R18 - display select. 0:DS1, 1:DS2
 * output reg:	none
 */
dsp_dual_out:
	call	dsp_dual_7_get		; R17 has value. Get segment pattern.
;
	ser		R16					; set all bits as outputs
	out		DDRD, R16
	out		PORTD, R17
	// display select
	tst		R18
	brne	ddo001
	cbi		PORTC, DS1			; 0 = Display 1 .. PNP drive
	rjmp	ddo002
;
ddo001:
	cbi		PORTC, DS2			; 1 = Display 2 .. PNP drive
ddo002:
	ret

/*
 * Wait a minimum of N ms
 * input reg:	R17 - delay in ms
 * output reg:	none
 * resources: 	tic1ms
 *
 */
dsp_waitNms:
	cbi		GPIOR0, DELAY_1MS_TIC	; clear tic1ms flag set..interrupt will set it
	// wait
dw001:
	sbis	GPIOR0, DELAY_1MS_TIC
	rjmp	dw001					; loop waiting for tim1ms tic
	cbi		GPIOR0, DELAY_1MS_TIC	; clear tic1ms flag set by interrup
 	// Done?
	dec		R17
	brne	dw001					; no ..loop
	ret								; yes..EXIT

/*
 * Get the display pattern
 * input reg:	R17 - data (0x00-0x11)
 *				set R17 b7 = 1 for dp
 * output reg:	R17 - pattern
 * resources: 	R16, R21
 *
 * Use offset read lookup table element
 */
dsp_dual_7_get:
	push	R21
	mov		R21, R17				; save dp state
	andi	R17, $7F				; mask off dp flag
;
	ldi		ZH, high(seg_table<<1)	; Initialize Z pointer
	ldi		ZL, low(seg_table<<1)
	add		ZL, R17					; add offset
	clr		R16
	adc		ZH, R16					; propigate carry bit
	lpm		R17, Z					; Load pattern
									; memory pointed to by Z (r31:r30)
	// set dp line per state
	andi	R21, $80				; mask out bit to test
	breq	dd7g001
	andi	R17, $7F				; set line low to turn on dp if set
dd7g001:
	pop		R21
	ret

/*
 * 7 Segment pattern
 *		   a
 *		 f   b
 *		   g
 *		 e   c
 *		   d
 *		      dp
 */
seg_table:  ; 1gfedcba    1gfedcba
.db			0b11000000, 0b11111001	; 0, 1
.db			0b10100100, 0b10110000	; 2, 3
.db			0b10011001,	0b10010010	; 4, 5
.db			0b10000010, 0b11111000	; 6, 7
.db			0b10000000,	0b10011000	; 8, 9
.db			0b10001000,	0b10000011	; A, B
.db			0b11000110,	0b10100001	; C, D
.db			0b11001001,	0b10001001	; #(E), *(F)
.db			0b11111111,	0b10111111	; blank, dash
