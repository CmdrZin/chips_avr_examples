/*
 * Display with Keypad
 *   Dual 7 Segment display
 *   4x4 keypad
 *
 * org: 9/08/2014
 * rev: 3/18/2015
 * auth: Nels "Chip" Pearson
 *
 * Dependencies
 *		sys_timers.asm
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include disp_w_keypad.asm
 *
 * Resources;
 * SRAM
 *   dsp_buff	1 byte for 2 HEX number display
 *
 * NOTE: PortC.2:3 are used to control the display digits.
 *
 * NOTE: Change display to use one byte per digit to allow more than 16 charaters.
 * Add -, blank, dp, etc.
 *
 * This module supports a Dual 7-segment display and 4x4 keypad sharing PortD.
 * Both are multiplexed and share a 10ms tic for timing.
 *
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

.equ	DS1 = PORTC2
.equ	DS2 = PORTC3

.equ	DS_BLANK	= 0x10	; blank all segs
.equ	DS_DASH		= 0x11	; -
.equ	DS_S		= 0x12	; S
.equ	DS_L		= 0x12	; L
.equ	DS_J		= 0x12	; J
.equ	DS_P		= 0x12	; P
.equ	DS_U		= 0x12	; U

// dsp_state values
.equ	DSP_DS1		= 0x00
.equ	DSP_DS2		= 0x01
.equ	SCAN_KP		= 0x02


.DSEG
dsp_buff:	.BYTE	2		; Reserve 2 bytes in SRAM..one for each digit.
dsp_state:	.BYTE	1		; Reserve 1 byte in SRAM
							; 0: Display DS1
							; 1: Display DS2
							; 2: Scan keypad
dsp_dead:	.BYTE	2		; dead space

.CSEG
/*
 * Initialize Dual Display and Keypad
 *
 * input reg:	none
 * output reg:	none
 *
 * NOTE: Only need to call once.
 *
 */
dwk_init:
	call	dsp_dual_init_io
;
	clr		R16
	sts		dsp_state, R16
;
	ldi		R16, 0x08
	sts		dsp_buff, R16
	inc		R16
	sts		dsp_buff+1, R16
;
	call	key_pad_init
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
	cpi		R16, DSP_DS1
	breq	dks_skip000				; DS1
	cpi		R16, DSP_DS2
	breq	dks_skip010				; DS2
;
	rjmp	dks_skip020				; KeyPad
;
dks_skip000:
	ldi		R18, DSP_DS1			; select DS1
	call	dsp_dual_display		; run display service
;
	ldi		R16, DSP_DS2
	sts		dsp_state, R16			; update to do DS2 next time.
	ret
;
dks_skip010:
	ldi		R18, DSP_DS2			; select DS2
	call	dsp_dual_display		; run display service
;
	ldi		R16, SCAN_KP
	sts		dsp_state, R16			; update to do KeyPad next time.
	ret
;
dks_skip020:
	call	key_pad_service
;
	ldi		R16, DSP_DS1
	sts		dsp_state, R16			; update to do DS1 next time.
	ret

/*
 * Display 2 HEX digits
 *
 * input reg:	R18 - Select digit .. 0:DS1, 1:DS2
 * output reg:	none
 * resource:
 *
 * Called every 30ms.
 * Display each digit for 10ms.
 * Calling keyscan will disable display.
 *
 */
dsp_dual_display:
	cpi		R18, DSP_DS1
	brne	ddd_skip00
; DS1
	sbi		PORTC, DS2			; turn off DS2
	lds		R17, dsp_buff
	call	dsp_dual_7_get		; R17 has value. Get segment pattern.
	cbi		PORTC, DS1			; Display 1 .. PNP drive
	rjmp	ddd_exit			; EXIT
;
ddd_skip00:
; DS2
	sbi		PORTC, DS1			; turn off DS1
	lds		R17, dsp_buff+1
	call	dsp_dual_7_get		; R17 has value. Get segment pattern.
	cbi		PORTC, DS2			; Display 2 .. PNP drive
;
ddd_exit:
	ser		R16					; set all bits as outputs
	out		DDRD, R16
	out		PORTD, R17
;
	ret							; EXIT

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
;
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
;
dd7g001:
	pop		R21
;
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
;;.db			0b11001001,	0b10001001	; #(E), *(F)  KeyPad char
.db			0b10000110,	0b10001110	; E, F
.db			0b11111111,	0b10111111	; blank, dash


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

.equ	KMS_KP_SCAN = 0		; Scan for any key press
							; When key is detected as presssed.
							; First Key seen is saved into kms_buff with MSBit set=1.
.equ	KMS_KP_KEY_DOWN = 1	; Check that same key is down.
.equ	KMS_KP_KEY_UP = 2	; Check if ALL released.
.equ	KMS_KP_KEY_WAIT = 3	; Check if ALL still released.

.DSEG
dead_buff:	.BYTE	2		; dead space
kms_scan:	.BYTE	1		; Raw Scan value.
kms_buff:	.BYTE	1		; Key Matrix Scan last valid data buffer
							; d3:0 = data..d7: 0=old..1=new
kms_state:	.BYTE	1		; scan state. 0:idle 1:debounce


.CSEG
/* Initialize Key Pad Service
 * input reg:	none
 * output reg:	none
 */
 key_pad_init:
	clr		R16
	sts		kms_buff, R16
	sts		kms_state, R16
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
 * output reg:	none
 * resource:	key_buff..b7 = 1:new, 0:old		b3:0 = data
 *
 * Process
 * 	This code is called from the disp_kp service avery 30ms.
 * State
 *	0: Scan for any key press -> Update data, New=1, state=1
 *	1: Key down. check if same. Wait for ALL Key up -> Set db_count, state=2
 *	2: Key up. Wait for debounce delay -> state=0. IF any key down -> state=1
 */
 key_pad_service:
; switch(state)
	lds		R16, kms_state
	cpi		R16, KMS_KP_SCAN
	breq	kps_skip00			; case 0
	cpi		R16, KMS_KP_KEY_DOWN
	breq	kps_skip10			; case 1
	cpi		R16, KMS_KP_KEY_UP
	breq	kps_skip20			; case 2
	cpi		R16, KMS_KP_KEY_WAIT
	breq	kps_skip30			; case 3
; default
	ldi		R16, KMS_KP_SCAN
	sts		kms_state, R16		; reset..was invalid
	rjmp	kps_skip01			; return OLD data
; case: 0
kps_skip00:
	call	key_pad_scan		; R18=0 if valid. R17=data
	tst		R18
	brne	kps_skip01			; no keys
;
	sts		kms_scan, R17		; save Raw Scan value.
	ldi		R16, KMS_KP_KEY_DOWN
	sts		kms_state, R16
;
	ret							; EXIT
; case: 1
kps_skip10:
; Check that scan is the same. Same key pressed.
	call	key_pad_scan		; R18=0 if valid. R17=data
	tst		R18
	brne	kps_skip11
// store the valid data
; Compare
	lds		R16, kms_scan		; get last scan


	call	key_pad_trans		; translate Key code to Key cap
	ori		R17, 0x80			; set valid bit
	sts		kms_buff, R17
;
	ldi		R16, KMS_KP_KEY_UP
	sts		kms_state, R16
	ret
;
kps_skip11:
; key up..not held long enough.
	ldi		R16, KMS_KP_SCAN
	sts		kms_state, R16
	ret							; EXIT
;
; case: 2
kps_skip20:
	call	key_pad_scan
	tst		R18
	breq	kps_skip01			; still down. return OLD data
; Keys up.
	ldi		R16, KMS_KP_KEY_WAIT
	sts		kms_state, R16
	ret
;								; return OLD data
; case: 3
kps_skip30:
	call	key_pad_scan
	tst		R18
	breq	kps_skip01			; still down. return OLD data
; debounce completed. Return to state=0
	ldi		R16, KMS_KP_SCAN
	sts		kms_state, R16
;								; return OLD data
kps_skip01:
	ret							; EXIT

/*
 * Scan Key Pad
 * input reg:	none
 * output reg:	R17 - Data
 *				R18 - Data valid. 0:valid..1:no data
 *
 * Delay for 256 with 50 low
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
; delay to settle IO line
	clr		R16
kps_loop0:
	dec		R16
	brne	kps_loop0
;
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
; Arduino Flat KeyPad
.db			0x0D, 0x0E	; 0, 1	->	D, #(E)
.db			0x00, 0x0F	; 2, 3	->	0, *(F)
.db			0x0C, 0x09	; 4, 5	->	C, 9
.db			0x08, 0x07	; 6, 7	->	8, 7
.db			0x0B, 0x06	; 8, 9	->	B, 6
.db			0x05, 0x04	; A, B	->	5, 4
.db			0x0A, 0x03	; C, D	->	A, 3
.db			0x02, 0x01	; E, F	->	2, 1

