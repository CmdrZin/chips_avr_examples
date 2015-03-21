/*
 * LED and Switch
 *
 * org: 7/14/2014
 * rev: 3/17/2015
 * auth: Nels "Chip" Pearson
 *
 * Target: I2C Demo Board w/ display and keypad I/O, 20MHz
 *
 * Usage:
 * 	.include led_switch.asm
 *
 * Resources;
 * 		led_buff	1 byte
 *
 *	NOTE: PORTC shared supports I2C.
 */ 

.equ	BD_LED	= PORTC4

.DSEG
led_buff:			.BYTE	1	; set to number of flashes
; LOCAL registers
flash_cnt:			.BYTE	1
count_1000ms:		.BYTE	1
led_service_state:	.BYTE	1	; 0:idle, 1:flash on, 2:flash off
;
switch_buff:		.BYTE	1

.CSEG

.equ	count_100ms		= count_1000ms
.equ	count_20ms		= count_1000ms

/* Initialize LED Service IO
 * input reg:	none
 * output reg:	none
 */
led_init:
	call	turn_off_led			; turn OFF LED
	call	led_sw_init_io
;
	clr		r16
	sts		led_buff, r16
	sts		led_service_state, r16
;
; Set up delay counters
	ldi		r16, 100
	sts		count_1000ms, r16
;
	ret

/*
 * Initialize IO for LED
 * input reg:	none
 * output reg:	none
 *
 * NOTE: PORTC shared with I2C.
 */
led_sw_init_io:
	// Configure PORTC.4 as output PORTC.5 as input.
	sbi		DDRC, PORTC4		; LED drive output.
;
	cbi		DDRC, PORTC5		; Key press input.
	sbi		PORTC, PORTC5		; enable pullup.
;
	ret

/*
 * Single switch service
 * input reg:	none
 * output reg:	none. Check switch_buff for data - Data	b1:data, b0:state
 * resources:	switch_buff
 *				PINC5
 *
 * Read switch every 10ms.
 * switch buff:
 *	b5:	state		0:wait for up..1:wait for down
 *	b4:	last check	0:down..1:up
 *	b1:	data		0:open..1:closed
 *	b0: data state	0:data old..1:data new
 *
 * NOTE: PORTC shared with I2C.
 */
switch_service:
	sbis	GPIOR0, SWITCH_10MS_TIC
	rjmp	ss_no_data
;
	cbi		GPIOR0, SWITCH_10MS_TIC		; clear tic10ms flag set by interrup
; Service LED first.
	call	led_service
; get state
	lds		R17, switch_buff
	andi	R17, 0x20
	breq	ss_skip00
; waiting for down
	sbic	PINC, PINC5
	rjmp	ss_no_data			; no..still up
; yes..down
	ldi		R17, 0x03
	sts		switch_buff, R17	; update
	rjmp	ss_new_data
;
ss_skip00:
; waiting for up
	sbis	PINC, PINC5
	rjmp	ss_no_data			; no..still down
; is up now.
	ldi		R17, 0x21
	sts		switch_buff, R17	; update
	ldi		R17, 0x01
	rjmp	ss_new_data
;
ss_no_data:
;
ss_new_data:
;
	ret

/*
 * LED service
 * input reg:	none
 * output reg:	none
 * resources:
 *	led_buff
 *	flash_cnt
 *	count_1000ms
 *	led_service_state	0:idle, 1:flash on, 2:flash off
 *
 * Each second, pulse LED 100ms/20ms x led_buff value
 *
 * Process:
 *	if tic10ms=1
 * 		Check R2 (10ms) count down. If 0, reload and continue. Get state (R4).
 * 0: Idle - Wait for 1000ms.
 *			Dec R3 to 0 then reload with 100ms and set R4=1. (FLASH ON)
 *			Read led_buff into R1 (flash_cnt)
 *			Turn ON LED.
 * 1: Flash ON - Wait for 100ms.
 *		 	Reload R3 with 20ms and set R4=2 (FLASH OFF)
 *			Turn OFF LED
 * 2: Flash OFF - Wait for 20ms
 *			Dec R1 (flash_cnt).
 *			If 0, set R4=0 (IDLE)
 *			Else
 *				Reload R3 with 100ms
 *			Turn ON LED. Set R4=1 (FLASH ON)
 *
 * NOTE: PORTC shared with I2C.
 */
led_service:
;;;	sbis	GPIOR0, LED_10MS_TIC	; test 10ms tic
;;;	rjmp	ls_exit					; EXIT..not set
;
;;;	cbi		GPIOR0, LED_10MS_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		r16, led_service_state
; switch(state)
	tst		r16
	breq	ls_skip00			; case 0?
	dec		r16
	breq	ls_skip10			; case 1?
	dec		r16
	breq	ls_skip20			; case 2?
; default
	clr		r16
	sts		led_service_state, r16	; reset..was invalid
	rjmp	ls_exit					; EXIT
;
/* 0: Idle - Wait for 1000ms.
 *			Dec R3 to 0 then reload with 100ms and set R4=1. (FLASH ON)
 *			Read led_buff into R1 (flash_cnt)
 *			Turn ON LED.
 */
; case: 0
ls_skip00:
	lds		R16, count_1000ms
	dec		R16
	sts		count_1000ms, R16
	breq	ls_skip001
	rjmp	ls_exit
;
ls_skip001:
	ldi		R16, 10				; set 100ms time count
	sts		count_100ms, R16
; Test for ZERO data
	lds		R16, led_buff
	tst		R16
	brne	ls_skip01
	rjmp	ls_exit					; EXIT..0 Data
;
ls_skip01:
	lds		R16, led_service_state
	inc		R16						; state=1 (FLASH_ON)
	sts		led_service_state, R16
;
	lds		R16, led_buff
	sts		flash_cnt, R16
	call	turn_on_led				; turn ON LED
	rjmp	ls_exit					; EXIT
/* 1: Flash ON - Wait for 100ms.
 *		 	Reload R3 with 20ms and set R4=2 (FLASH OFF)
 *			Turn OFF LED
 */
ls_skip10:
	lds		R16, count_100ms
	dec		R16
	sts		count_100ms, R16
	brne	ls_exit
;
	ldi		R16, 10					; set new time delay
	sts		count_20ms, R16
	call	turn_off_led			; turn OFF LED
;
	lds		R16, led_service_state
	inc		R16						; state=2 (FLASH_OFF)
	sts		led_service_state, R16
	rjmp	ls_exit					; EXIT
/* 2: Flash OFF - Wait for 20ms
 *			Dec R1 (flash_cnt).
 *			If 0, set R4=0 (IDLE)
 *			Else
 *				Reload R3 with 100ms
 *			Turn ON LED. Set R4=1 (FLASH ON)
 */
ls_skip20:
	lds		R16, count_20ms
	dec		R16
	sts		count_20ms, R16
	brne	ls_exit
;
	lds		R16, flash_cnt
	dec		R16
	sts		flash_cnt, R16
	breq	ls_skip21
; continue flashing
	call	turn_on_led				; turn ON LED
	ldi		R16, 10					; set 100ms time count
	sts		count_100ms, R16
;
	lds		R16, led_service_state
	dec		R16						; state=1 (FLASH_ON)
	sts		led_service_state, R16
	rjmp	ls_exit					; EXIT
;
ls_skip21:
	clr		R16
	sts		led_service_state, R16	; state=0 (IDLE)
;
	ldi		R16, 100				; set 1000ms time count
	sts		count_1000ms, R16
;
ls_exit:
	ret

/* Turn Single LED ON */
turn_on_led:
	cbi		PORTC, BD_LED
	ret

/* Turn Single LED OFF */
turn_off_led:
	sbi		PORTC, BD_LED
	ret
