/*
 * Keypad Matrix 4x4
 *
 * org: 6/24/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include key_matrix_4x4.asm
 *
 * Resources
 *	kms_buff:			; Key Matrix Scan last valid data buffer
 *						; d3:0 = data..d7: 0=old..1=new
 *	kms_state:			; scan state. 0:idle 1:debounce
 *	kms_db_cnt:			; debounce wait counter
 * IO
 *	GPIOR0.GPIOR01		key scan tic1ms flag
 * 	
 */ 

.equ	COL0 = PORTD0
.equ	COL1 = PORTD1
.equ	COL2 = PORTD2
.equ	COL3 = PORTD3
.equ	ROW0 = PORTD4
.equ	ROW1 = PORTD5
.equ	ROW2 = PORTD6
.equ	ROW3 = PORTD7

.equ	COL_MASK = (1<<COL3 | 1<<COL2 | 1<<COL1 | 1<<COL0)	; 00001111

.equ	KMS_DB_WAIT	= 0x0A	; scan wait 10 ms

.DSEG
kms_buff:	.BYTE	1		; Key Matrix Scan last valid data buffer
							; d3:0 = data..d7: 0=old..1=new
kms_state:	.BYTE	1		; scan state. 0:idle 1:debounce
kms_db_cnt:	.BYTE	1	; debounce wait counter

.CSEG
/* Initialize Key Pad Service
 * input reg:	none
 * output reg:	none
 */
 key_pad_init:
	call	key_pad_init_io
;
	clr		R16
	sts		kms_buff, R16
	sts		kms_state, R16
	sts		kms_db_cnt, R16
;
	ret

/*
 * Initialize IO for Key Pad
 * input reg:	none
 * output reg:	none
 *
 * Set Port D control reg for PD3-PD0 as inputs
 *
 * NOTE: PORTC shared with key pad scan. ALWAYS call this for each scan.
 */
key_pad_init_io:
	// Configure 4 inputs and 4 outputs
	ldi		R16, (1<<ROW3 | 1<<ROW2 | 1<<ROW1 | 1<<ROW0)	; out:1, in:0
	out		DDRD, R16				; set 7-4 ouput (1), 3-0 input (0) w/pullup
	// Set 0-3 as input w/pullup...4-7 as HIGH
	ser		R16
	out		PORTD, R16
;
	ret

/*
 * Service Key Pad
 * input reg:	none
 * output reg:	R17 - Data
 *				R18 - Data valid. 0:new..1:old data
 * Process
 * 	This code is called in a fast loop (<1ms) and is self timed.
 * State
 *	0: Scan for any key press -> Update data, New=1, state=1
 *	1: Key down. Wait for ALL Key up -> Set db_count, state=2
 *	2: Key up. Wait for debounce delay -> state=0. IF any key down -> state=1
 */
 key_pad_service:
 // Only service every 10 ms
	sbis	GPIOR0, KPAD_1MS_TIC
	rjmp	kps_skip01				; return OLD data
; service time delay. dec count
	cbi		GPIOR0, KPAD_1MS_TIC	; clear tic1ms flag. interrupt sets it.
;
	lds		R16, kms_db_cnt
	dec		R16
	sts		kms_db_cnt, R16
	breq	kps_skip02				; run service
	rjmp	kps_skip01				; return OLD data
;	
kps_skip02:
	ldi		R17, KMS_DB_WAIT		; reset delay count
	sts		kms_db_cnt, R17
// Run servcice
; switch(state)
	lds		R16, kms_state
	tst		R16
	breq	kps_skip00			; case 0?
	dec		R16
	breq	kps_skip10			; case 1?
	dec		R16
	breq	kps_skip20			; case 2?
; default
	clr		R16
	sts		kms_state, R16		; reset..was invalid
	rjmp	kps_skip01			; return OLD data
; case: 0
kps_skip00:
	call	key_pad_scan
	tst		R18
	brne	kps_skip01
	// store the valid data
	sts		kms_buff, R17
	ldi		R16, $1				; set state=1
	sts		kms_state, R16
	ret							; EXIT..R18 already = 0
; case: 1
kps_skip10:
	call	key_pad_scan
	tst		R18
	breq	kps_skip01			; key still pressed. Return OLD data.
; key Up
	ldi		R16, $2			; set state=2
	sts		kms_state, R16
	rjmp	kps_skip01			; return OLD data
; case: 2
// key up for entire delay time or go back to state=1
kps_skip20:
	call	key_pad_scan
	tst		R18
	breq	kps_skip01			; still down. return OLD data
; debounce completed. Return to state=0
	clr		R16
	sts		kms_state, R16
;								; return OLD data
kps_skip01:
	// return old data
	lds		R17, kms_buff
	ldi		R18, 1				; OLD data
	ret							; EXIT

/*
 * Scan Key Pad
 * input reg:	none
 * output reg:	R17 - Data
 *				R18 - Data valid. 0:valid..1:no data
 *
 * subcnt = 0
 * step through rows
 * if !(COL & COL_MASK) != 0 then
 * val_cnt = R18 << 2			base from row
 * COL>>1  						test LSB in Carry Bit
 * if COL != 0
 *    ++sub_cnt
 *    loop back to shift
 * else							found bit
 *	  row<<2 + sub_cnt = key number.
 * NOTE: Could also decrement to speed up end test.
 */
key_pad_scan:
	call	key_pad_init_io
	ldi		R17, (1<<ROW0)			; select row
	clr		R18						; row counter = 0
	clr		R19						; sub_cnt = 0

kps000:
	// scan row
	com		R17						; generate scan pattern. 0 on row to scan.
	out		PORTD, R17				; set ROWn low
	com		R17						; restore
	in		R16, PIND				; read columns
	com		R16					; complement..only 1 bit shoud be set if only 1 key pressed
	andi	R16, COL_MASK			; mask out col bits
	// test for key press
	tst		R16
	brne	kps001					; found a key..skip to process it
	// try next row
	lsl		R17
	inc		R18						; ++row_cnt
	sbrs	R18, $2					; at 4?
	rjmp	kps000					; loop back
	ldi		R18, $1					; no data
	ret								; EXIT
;
kps001:
	// key pressed
	lsr		R16						; COL>>1
	tst		R16
	breq	kps002					; skip if 0..create key value
	inc		R19						; ++sub_cnt
	rjmp	kps001					; try next bit
;
	// key decode
kps002:
	lsl		R18						; x2
	lsl		R18						; x4
	add		R18, R19				; row<<2 + sub_cnt
	mov		R17, R18				; save data
	clr		R18						; data valid
	ret

/*
 * Translate Key Code to Key Cap
 * input reg:	R17 - Key Code
 * output reg:	R17 - Key Cap Value
 * resources: 	TEMP, R21
 *
 * Use offset read lookup table element
 */
key_pad_trans:
	andi	R17, $0F					; mask off valid data..could test
	ldi		ZH, high(key_pad_table<<1)	; Initialize Z pointer
	ldi		ZL, low(key_pad_table<<1)
	add		ZL, R17						; add offset
	clr		R16
	adc		ZH, R16						; propigate carry bit
	lpm		R17, Z						; Load pattern
										; memory pointed to by Z (r31:r30)
	ret

key_pad_table:
.db			0x0D, 0x0E	; 0, 1	->	D, #(E)
.db			0x00, 0x0F	; 2, 3	->	0, *(F)
.db			0x0C, 0x09	; 4, 5	->	C, 9
.db			0x08, 0x07	; 6, 7	->	8, 7
.db			0x0B, 0x06	; 8, 9	->	B, 6
.db			0x05, 0x04	; A, B	->	5, 4
.db			0x0A, 0x03	; C, D	->	A, 3
.db			0x02, 0x01	; E, F	->	2, 1
