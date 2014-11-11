/*
 * LED and Switch
 *
 * org: 7/14/2014
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

.DSEG
led_buff:			.BYTE	1
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
	sbi		PORTC, PORTC4			; turn OFF LED (could also call function)
	call	led_sw_init_io
	clr		R16
	sts		led_buff, R16
;
; Set up delay counters
	ldi		R16, 100
	sts		count_1000ms, R16
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
 * output reg:	R17 - Data	b1:data, b0:state
 * resources:	R15 - switch_buff
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
	lds		R17, switch_buff
	andi	R17, 0x02
ss_new_data:
	ret

/*
 * LED service
 * input reg:	none
 * output reg:	none
 * resources:
 *	R0		led_buff
 *	R1		flash_cnt
 *	R2		count_10ms
 *	R3		count_1000ms, count_100ms, count_20ms .. 100,10,2
 *	R4		led_service_state	0:idle, 1:flash on, 2:flash off
 *
 * Each second,pulse LED 100ms/20ms x led_buff value
 *
 * Process:
 *	if tic1ms=1
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
	sbis	GPIOR0, LED_10MS_TIC	; test 10ms tic
	rjmp	ls_exit					; EXIT..not set
;
	cbi		GPIOR0, LED_10MS_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		R16, led_service_state
; switch(state)
	tst		R16
	breq	ls_skip00			; case 0?
	dec		R16
	breq	ls_skip10			; case 1?
	dec		R16
	breq	ls_skip20			; case 2?
; default
	clr		R16
	sts		led_service_state, R16	; reset..was invalid
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
	brne	ls_exit
;
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
	cbi		PORTC, PORTC4			; turn ON LED (could also call function)
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
	sbi		PORTC, PORTC4			; turn OFF LED (could also call function)
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
	cbi		PORTC, PORTC4			; turn ON LED (could also call function)
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
