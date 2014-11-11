/*
 * Sumo Bot Floor Detector Code
 *
 * org: 10/17/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board, 8MHz, Sumo Bot, 3 IR Front Sensors
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	FLOOR_DELAY_COUNT	= 1

.equ	FLOOR_LED_IO_RIGHT	= PORTD1
.equ	FLOOR_LED_IO_CENTER	= PORTD2
.equ	FLOOR_LED_IO_LEFT	= PORTD3

.equ	FLOOR_DET_IO_RIGHT	= PORTD5
.equ	FLOOR_DET_IO_CENTER	= PORTD6
.equ	FLOOR_DET_IO_LEFT	= PORTD7

.equ	FLOOR_MASK_RIGHT	= 0x20
.equ	FLOOR_MASK_CENTER	= 0x40
.equ	FLOOR_MASK_LEFT		= 0x80

; floor_status
.equ	FLOOR_STATUS_RIGHT	= 0x01
.equ	FLOOR_STATUS_CENTER	= 0x02
.equ	FLOOR_STATUS_LEFT	= 0x04


.DSEG
floor_delay_cnt:	.BYTE	1			; service time delay * 10ms
floor_status:		.BYTE	1			; which LED/Detector to service
floor_det_status:	.BYTE	1			; Detector status


.CSEG
/*
 * Initialize Floor Service Parameters
 *
 * 
 */
floor_det_init:
	call	floor_det_init_io
;
	ldi		R16, FLOOR_STATUS_RIGHT
	sts		floor_status, R16
;
	ldi		R16, FLOOR_DELAY_COUNT
	sts		floor_delay_cnt, R16
;
	ret

/*
 * Initialize Floor Detector IO
 * 
 */
floor_det_init_io:
; init IO output
	sbi		DDRD, FLOOR_LED_IO_RIGHT		; out
	sbi		PORTD, FLOOR_LED_IO_RIGHT		; OFF
	sbi		DDRD, FLOOR_LED_IO_CENTER
	sbi		PORTD, FLOOR_LED_IO_CENTER
	sbi		DDRD, FLOOR_LED_IO_LEFT
	sbi		PORTD, FLOOR_LED_IO_LEFT
; init IO input
	cbi		DDRD, FLOOR_DET_IO_RIGHT		; in
	sbi		PORTD, FLOOR_DET_IO_RIGHT		; Pull-up ON
	cbi		DDRD, FLOOR_DET_IO_CENTER
	sbi		PORTD, FLOOR_DET_IO_CENTER
	cbi		DDRD, FLOOR_DET_IO_LEFT
	sbi		PORTD, FLOOR_DET_IO_LEFT
;
	ret

/*
 * floor_det_service()
 *
 * Support for three IR sensors at front of Sumo Bot.
 *
 * Test 1ms flag..consume
 * if delay == 0
 * 	delay = 10
 * else
 * 	--delay then Exit
 * Cycle through three detectors.
 *   Turn on LED, Wait 16us, Sample det.
 *
 */
fds_exite:
	rjmp	fds_exit

floor_det_service:
	sbis	GPIOR0, FLOOR_1MS_TIC	; test 10ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, FLOOR_1MS_TIC	; clear tic10ms flag set by interrup
// Run service
	lds		R16, floor_delay_cnt
	dec		R16
	sts		floor_delay_cnt, R16
	brne	fds_exite					; reset or exit
; yes
	ldi		R16, FLOOR_DELAY_COUNT
	sts		floor_delay_cnt, R16
;
	call	floor_det_init_io			; reset I/O
	lds		R16, floor_status			; get mode
	lds		R17, floor_det_status		; get current status
; switch(state)
	cpi		R16, FLOOR_STATUS_RIGHT
	brne	fds_skip10
; RIGHT
	ldi		R16, FLOOR_STATUS_CENTER	; next detector
; test
	cbi		PORTD, FLOOR_LED_IO_RIGHT	; LED on
	call	floor_delay
	in		R18, PIND					; read det
	sbi		PORTD, FLOOR_LED_IO_RIGHT	; LED off
	andi	R18, FLOOR_MASK_RIGHT		; test for high (BLACK)
	breq	fds_skip00
; sense black (1)
	call	turn_on_green
	ori		R17, FLOOR_STATUS_RIGHT		; set bit
	rjmp	fds_update
fds_skip00:
; sense white (0)
	call	turn_off_green
	andi	R17, ~FLOOR_STATUS_RIGHT	; clr bit
	rjmp	fds_update
;
fds_skip10:
	cpi		R16, FLOOR_STATUS_CENTER
	brne	fds_skip20
; CENTER
	ldi		R16, FLOOR_STATUS_LEFT		; next detector
; test
	cbi		PORTD, FLOOR_LED_IO_CENTER	; LED on
	call	floor_delay
	in		R18, PIND					; read det
	sbi		PORTD, FLOOR_LED_IO_CENTER	; LED off
	andi	R18, FLOOR_MASK_CENTER		; test for low (ON)
	breq	fds_skip11
; sense black (1)
	call	turn_on_red
	ori		R17, FLOOR_STATUS_CENTER	; set bit
	rjmp	fds_update
fds_skip11:
; sense white (0)
	call	turn_off_red
	andi	R17, ~FLOOR_STATUS_CENTER	; clr bit
	rjmp	fds_update
;
fds_skip20:
	cpi		R16, FLOOR_STATUS_LEFT
	brne	fds_skip30
; LEFT
	ldi		R16, FLOOR_STATUS_RIGHT		; next detector
; test
	cbi		PORTD, FLOOR_LED_IO_LEFT	; LED on
	call	floor_delay
	in		R18, PIND					; read det
	sbi		PORTD, FLOOR_LED_IO_LEFT	; LED off
	andi	R18, FLOOR_MASK_LEFT		; test for low (ON)
	breq	fds_skip21
; sense black (1)
	call	turn_on_yellow
	ori		R17, FLOOR_STATUS_LEFT		; set bit
	rjmp	fds_update
fds_skip21:
; sense white (0)
	call	turn_off_yellow
	andi	R17, ~FLOOR_STATUS_LEFT		; clr bit
	rjmp	fds_update
;
fds_skip30:
; default
	clr		R17
	ldi		R16, FLOOR_STATUS_RIGHT		; set to default
fds_update:
	sts		floor_det_status, R17
	sts		floor_status, R16
;
fds_exit:
	ret

/*
 * Delay 16us with 8MHz clock
 * 8 cycles = 1us
 * NOTE: Safe to call from any where.
 */
floor_delay:
	push	R16
;
	ldi		R16, 251			; 32 -1 for overhead
; 4 cycle loop = 0.5us
fd1_loop1:
	nop
	dec		R16
	brne	fd1_loop1
;
	pop		R16
	ret
