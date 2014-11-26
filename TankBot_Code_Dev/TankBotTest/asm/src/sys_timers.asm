/*
 * System Timer Utility
 *
 * org: 6/28/2014
 * rev: 11/19/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include sys_timers.asm
 *
 * Dependentcies:
 *  pwm_dc_motor_lib.asm
 *
 * Timer0
 * 	General purpose system timer. Provides 10ms and 1ms TIC flags for services.
 * 	Embedded a call to start PWM pulses every 10ms.
 * 
 * Timer1
 * 	Can be configured for Servo Timing (8us res) or DC Motor Timing (32us res).
 *  Just adjust clock divider.
 * 	Supports Two Servo Channels or Two DC Motors.
 * 
 * Timer2
 * 	Used to time Sonar. Set for 56.4us to read out in 1 cm per count.
 * 
 */ 

; 1ms tic flags
;.equ					= GPIOR00		;
;.equ					= GPIOR01		; 
;.equ					= GPIOR02		; 
;.equ					= GPIOR03		; 
; 10ms tic flags
.equ	RNG_10MS_TIC	= GPIOR04		; Optical Range Sensor
.equ	RNG_S_10MS_TIC	= GPIOR05		; Sonar Range Sensor
;.equ					= GPIOR06		;
.equ	DEMO_10MS_TIC	= GPIOR07		; Demo service

.equ	SLOW_TIC		= 10			; 1ms * N for the slow tic

.DSEG
st_cnt_10ms:		.BYTE	1		; secondary timer counter.
st_tmr2_count:		.BYTE	1

.CSEG
/*
 * Set up Timer0 to generate 1ms and 10ms System Time Tics using 20MHz CPU clock.
 * GPIOR0 is used to provide TIC flags.
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: 10ms and 1ms flags generated.
 *
 */
st_init_tmr0:
;;	ldi		R16, 31				; ( 8MHz) 2 * 256 * (1 + OCR0A) : 512 * (16*2) : 1024 * 8 * 2
	ldi		R16, 77				; (20MHz) 2 * 256 * (1 + OCR0A) : 512 * (40*2) : 1024 * 40 * 2
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
 * Set up Timer1 for PWM at 8us (Servo) or 32us (DC Motor) rate using 20MHz CPU clock
 * Call this once after RESET.
 *
 * PWM use OCR1A and OCR1B interrupt services.
 * Timer1 counts up normally and is stopped and cleared once both PWM channels are triggered.
 *
 * Rate = 20MHz / 1024 = 51.2us rate
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
	clr		r16
	sts		OCR1AH, r16
	sts		OCR1BH, r16
	ldi		r16, 1				; set to STOP
	sts		OCR1AL, r16
	sts		OCR1BL, r16
; WGM11:0 = 00					; WGM13:0 = 0000 for Normal count up.
	ldi		r16, (0<<WGM11)|(0<<WGM10)
	sts		TCCR1A, r16
; WGM13:2 = 00, CS12:0 = 101, CPU div 1024 = 51.2 us
	ldi		r16, (0<<WGM13)|(0<<WGM12)|(1<<CS12)|(0<<CS11)|(1<<CS10)
	sts		TCCR1B, r16
;
	ret

/*
 * Set up Timer2 to generate 56.4us interrupt for Sonar Time (1cm) using 20MHz CPU clock
 * Call this once after RESET.
 *
 * Modifies: OCR0A, TCCR0A, TIMSK0, TCCR0B, and GPIOR0
 *
 * input reg:	none
 * output reg:	none
 * resources:	R16
 *
 * NOTE: Rate set for 56.4us
 */
st_init_tmr2:
	ldi		R16, 140
	sts		OCR2A, R16
;
	ldi		R16, (1<<WGM01)		; reset on CTC match
	sts		TCCR2A, R16
;
	ldi		R16, 0b010			; CPU div 8
	sts		TCCR2B, R16
;
	ldi		R16, (1<<OCIE2A)	; enable timer2 intr on A match.
	sts		TIMSK2, R16
;
	ret

/*
 * Clear Timer2
 * Use for more accurate distance. Optional.
 */
st_clr_tmr2:
	push	r16
	clr		r16
	sts		TCNT2, r16
	pop		r16
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
	call	pwm_start_pulse		; add PWM support for 10ms period
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

/*
 * Timer1 interrupt service.
 *
 * see pwm_tmr1A_intr and pwm_tmr1B_intr
 */


/*
 * Timer2 CTC (compare) interrupt service.
 * Called each 56.4us to allow direct readout in cm for sonar.
 *
 * input reg:	none
 * output reg:	none
 * resources:	st_tmr2_count	SRAM	1 byte
 *
 */
st_tmr2A_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	r16
;
	lds		r16, st_tmr2_count
	inc		r16
	sts		st_tmr2_count, r16
;
	pop		R16
;
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
