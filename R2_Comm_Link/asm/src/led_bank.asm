/*
 * LED Bank Service
 *
 * org: 8/27/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include led_bank.asm
 *
 * Resources;
 *		led_bank	1 byte
 *		led_cntl	1 byte
 * 		led_ZH		1 byte
 *		led_ZL		1 byte
 *		led_cnt		1 byte
 *		led_bar0	1 byte
 *		led_bar1	1 byte
 */ 

.DSEG

led_bank:		.BYTE	1
led_cntl:		.BYTE	1
led_ZH:			.BYTE	1
led_ZL:			.BYTE	1
led_cnt:		.BYTE	1
led_bar0:		.BYTE	1
led_bar1:		.BYTE	1

.CSEG
/*
 * Initialize LED Bank Service
 * input reg:	none
 * output reg:	none
 *
 * The LED bank service runs by being called continuously from main().
 * It is time sync'd to the 1 ms tics of Timer0.
 *
 */
led_bank_init:
	call	led_bank_init_io
;
	ldi		R16, 12					; LED count
	ldi		ZH, high(led_pat_table<<1)	; Reset Z pointer
	ldi		ZL, low(led_pat_table<<1)
;
	sts		led_cnt, R16
	sts		led_ZH, ZH
	sts		led_ZL, ZL
;
	ret

/*
 * Initialize IO for LED
 * input reg:	none
 * output reg:	none
 *
 * NOTE: PORTC shared with I2C.
 */
led_bank_init_io:
	// Configure PORTC.2:5 as output
	sbi		DDRC, PORTC2
	sbi		DDRC, PORTC3
	sbi		DDRC, PORTC4
	sbi		DDRC, PORTC5
;
	ret

/*
 * get LED bank pattern v2
 * input reg:	CY bit = 0:ignore..1:get
 * output reg:	R17 = I/O pattern
 *				R18 = Enable pattern
 * resources:	ZH, ZL
 *
 * Load ZH:ZL
 * If CY
 *   Get I/O and Enable patterns into R17,R18 and inc Z twice.
 * else
 *   Set R17=R18=0
 * Update ZH:ZL .. They are reset in led_bank_service() when led_cnt = 0.
 *
 * bit_cnt		Pattern		Enable
 *				  76543210	  76543210
 *   0			0b00001000	0bxx0011xx	Tone 1
 *   1			0b00000100	0bxx0011xx	Tone 2
 *   2			0b00010000	0bxx1100xx	Tone 3
 *   3			0b00001000	0bxx1100xx	Tone 4
 *   4			0b00100000	0bxx1010xx	Tone 5
 *   5			0b00001000	0bxx1010xx	Tone 6
 *   6			0b00100000	0bxx1100xx	Tone 7
 *   7			0b00010000	0bxx1100xx	Tone 8
 *   8			0b00010000	0bxx0101xx	Ov +
 *   9			0b00000100	0bxx0101xx	0
 *   10			0b00100000	0bxx1001xx	Ov -
 *   11			0b00000100	0bxx1001xx	CAL
 *
 */
get_led_bank:
; restore Z
	lds		ZH, led_ZH
	lds		ZL, led_ZL
; always get to inc Z
	lpm		R17, Z+
	lpm		R18, Z+
;test
	brcs	glb_skip20		; get? .. CY = 1?
	clr		R17				; no
	clr		R18
;
glb_skip20:
; update
	sts		led_ZH, ZH
	sts		led_ZL, ZL
;
	ret

/* LED pattern table */
led_pat_table:
//		I/O pattern	Enable pattern
.db		0b00001000, 0b00001100	; Tone 1
.db		0b00000100, 0b00001100	; Tone 2
.db		0b00010000, 0b00011000	; Tone 3
.db		0b00001000, 0b00011000	; Tone 4
.db		0b00100000, 0b00101000	; Tone 5
.db		0b00001000, 0b00101000	; Tone 6
.db		0b00100000, 0b00110000	; Tone 7
.db		0b00010000, 0b00110000	; Tone 8
.db		0b00010000, 0b00010100	; Ov +
.db		0b00000100, 0b00010100	; 0
.db		0b00100000, 0b00100100	; Ov -
.db		0b00000100, 0b00100100	; CAL

/*
 * LED Bank Service v2
 * input reg:	none
 * output reg:	none
 * resources:
 *   SRAM		5 bytes
 *    led_bank
 *    led_cntl
 *    led_bar0, led_bar1
 *    led_cnt
 *
 * Continous scan of LED bank and light those with bit set.
 *
 * Process:
 *    ROR led_bar1 -> led_bar0 -> CY bit.
 *    call get_led_bank(CY)
 *  else
 *    --bit_cnt
 *  if led_cnt = 0, copy led_cntl -> led_bar1, led_bank -> ledbar0, set led_cnt=12
 *
 * NOTE: led_bar0 and led_bar1 are updated after the first run through.
 *
 */
led_bank_service:
	sbis	GPIOR0, LED_BANK_TIC	; test 10ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, LED_BANK_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		R16, led_cnt
	dec		R16
	brne	lbs_skip20				; reset?
; yes
; reload shift bar
	lds		R16, led_cntl
	sts		led_bar1, R16
	lds		R16, led_bank
	sts		led_bar0, R16
; reset Z
	ldi		ZH, high(led_pat_table<<1)	; Reset Z pointer
	ldi		ZL, low(led_pat_table<<1)
	sts		led_ZH, ZH
	sts		led_ZL, ZL
;
	ldi		R16, 12				; NUM_BANK_LEDS
;
lbs_skip20:	
	sts		led_cnt, R16			; update
;
	lds		R16, led_bar1			; get pattern and rotate right
	lsr		R16
	sts		led_bar1, R16
;
	lds		R16, led_bar0
	ror		R16
	sts		led_bar0, R16
;
	call	get_led_bank			; CY=0 (F) or CY=1 (T)
;
; output
	out		DDRC, R18				; enables
	out		PORTC, R17				; pattern
;
	ret
