/*
 * System Timer Utility
 *
 * org: 6/28/2014
 * rev: 10/20/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include sys_timers.asm
 *
 */ 

; 1ms tic flags
.equ	FLOOR_1MS_TIC	= GPIOR00		; Floor/Line Detectors
;.equ					= GPIOR01		; 
;.equ					= GPIOR02		; 
.equ	STEPPER_1MS_TIC	= GPIOR03		; 
; 10ms tic flags
.equ	RNG_10MS_TIC	= GPIOR04		; Optical Range Sensor
;.equ					= GPIOR05		;
.equ	LCD_TIC			= GPIOR06		; Tone Comm service
.equ	DEMO_10MS_TIC	= GPIOR07		; Demo service

.equ	SLOW_TIC		= 10			; 1ms * N for the slow tic

.DSEG
st_cnt_10ms:		.BYTE	1		; secondary timer counter.

.CSEG
/*
 * Set up Timer0 to generate System Time Tic for 1 ms using 8MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: 1ms
 */
st_init_tmr0:
	ldi		R16, 31				; 2 * 256 * (1 + OCR0A) : 512 * (16*2) : 1024 * 8 * 2
	out		OCR0A, R16
	ldi		R16, (1<<WGM01)
	out		TCCR0A, R16
	ldi		R16, (1<<OCIE0A)
	sts		TIMSK0, R16			; enable counter 0 OCO intr
;
	ldi		R16, 0b100			; CPU div 256
	out		TCCR0B, R16
;
	ldi		R16, SLOW_TIC		; set for 10ms count.
	sts		st_cnt_10ms, R16
;
	clr		R16
	sts		GPIOR0, R16			; clear all tic flags
;
	ret

/*
 * Set up Timer1 for ADC Trigger and Tone Gen at 125us using 20MHz CPU clock
 * Call this once after RESET.
 * ADC used CTC-B
 * Tone Gen used CTC-A interrupt service
 *
 * Modifies: OCR1A, OCR1B, TCCR1A, TCCR1B, and TIMSK1
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: Other values are provided for different clock rates.
 */
st_init_tmr1:
;	clr		R16					; 0    .. 1 MHz
	ldi		R16, 3				; 768  .. 8 MHz CPU
;	ldi		R16, 9				; 2304 .. 20 MHz CPU
	sts		OCR1AH, R16
	sts		OCR1BH, R16
;	ldi		R16, 62				; 2 * n * (1+62) = 126 us..1 MHz CPU
	ldi		R16, 231			; 2 * n * (1+768+231) = 125 us..8 MHz CPU
;	ldi		R16, 155			; 2 * n * (1+2304+155) = 125 us..20 MHz CPU
	sts		OCR1AL, R16
	sts		OCR1BL, R16
;
	clr		R16
	sts		TCCR1A, R16		; WGM11:0 = 00 for CTC
;
	ldi		R16, 0b00001001	; WGM13:2 = 01, CPU div 1 = 1 us
	sts		TCCR1B, R16
;
	ldi		R16, (1<<OCIE1B)
	sts		TIMSK1, R16
;
	ret

/*
 * Timer0 CTC (compare) interrupt service.
 * Called each 1ms
 *
 * input reg:	none
 * output reg:	none
 * resources:	GPIOR0.GPIR00:7
 * 				SRAM	1 byte
 *				Stack:3
 *
 */
st_tmr0_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R16
; tic1ms flags
	sbi		GPIOR0, GPIOR00		;
	sbi		GPIOR0, GPIOR01		; 
	sbi		GPIOR0, GPIOR02		; 
	sbi		GPIOR0, GPIOR03		;
;
	lds		R16, st_cnt_10ms	; get counter
	dec		R16
	brne	st_skip00
	ldi		R16, SLOW_TIC		; reload 10ms count down.
; tic10ms flags
	sbi		GPIOR0, GPIOR04		; 
	sbi		GPIOR0, GPIOR05		;
	sbi		GPIOR0, GPIOR06		; 
	sbi		GPIOR0, GPIOR07		;
;
st_skip00:
	sts		st_cnt_10ms, R16	; update
	pop		R16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
