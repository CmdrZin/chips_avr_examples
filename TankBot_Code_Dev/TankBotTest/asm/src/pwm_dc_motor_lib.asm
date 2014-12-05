/*
 * PWM DC Motor Control Lib
 *
 * org: 11/18/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: TankBot Board, 20MHz, ATmega164P
 *
 *
 * Dual DC Motor PWM driver.
 *    STBY: 0-Motor OFF  1-Motor ON
 *
 *    Control
 *      0 0		STOP
 *      0 1		FWD
 *      1 0		REV
 *      1 1		BREAK
 *
 *    PWM: 10ms period. 8us resolution. 00:0% (OFF)..C3:100% (MaxON)
 *
 * PORT D used.
 *
 * Process
 *    MOT1 & MOT2 outputs set HIGH on 10ms TIC and Counter2 is started.
 *    MOT1 output is cleared on CT1A match. Counter1 is reset and stopped if MOT2 also is cleared.
 *    MOT2 output is cleared on CT1B match. Counter1 is reset and stopped if MOT1 also is cleared.
 *
 */

.equ	PWM_RIGHT		=	PORTD7			; Assign I/O pins
.equ	PWM_A2_RIGHT	=	PORTD6
.equ	PWM_A1_RIGHT	=	PORTD5

.equ	PWM_B1_LEFT		=	PORTD4			; Assign I/O pins
.equ	PWM_B2_LEFT		=	PORTD3
.equ	PWM_LEFT		=	PORTD2

.equ	PWM_FLAG_RIGHT	=	0x01
.equ	PWM_FLAG_LEFT	=	0x02

.equ	PWM_STBY		=	PORTB1

; Timer1 = 51.2us
; 128 = 6.55ms..66%
; 195 = 10.0ms..100%
.equ	PWM_R_STOP		=	1				; Speed constants
.equ	PWM_R_SLOW		=	PWM_R_STOP+49
.equ	PWM_R_MED		=	PWM_R_STOP+98
.equ	PWM_R_FAST		=	PWM_R_STOP+195

.equ	PWM_L_STOP		=	1
.equ	PWM_L_SLOW		=	PWM_L_STOP+49
.equ	PWM_L_MED		=	PWM_L_STOP+85
.equ	PWM_L_FAST		=	PWM_L_STOP+195

.equ	DIR_FWD			=	0				; Direction constants
.equ	DIR_REV			=	1


.DSEG
pwm_flags:			.BYTE	1
pwm_val_right:		.BYTE	1		; 51.2us res
pwm_val_left:		.BYTE	1		; 51.2us res


.CSEG
/*
 * pwm_init()
 * Set up I/O and PWM counters
 *
 * PortD.2		- Left Motor
 * PortD.7		- Right Motor
 * 
 */
pwm_dc_init:
; set up I/O
	call	pwm_dc_init_io
; Set up Timer 1
	clr		R16
	sts		OCR1AH, R16
	sts		OCR1BH, R16
	call	pwm_stop_right
	call	pwm_stop_left
; Enable A & B interrupts
	ldi		R16, (1<<OCIE1A)|(1<<OCIE1B)
	sts		TIMSK1, R16
;
	ret

/*
 * pwm_init_io()
 * Set up I/O hardware for PWM
 *
 * PWM_RIGHT	=	PORTD7
 * PWM_A2_RIGHT	=	PORTD6
 * PWM_A1_RIGHT	=	PORTD5
 *
 * PWM_B1_LEFT	=	PORTD4
 * PWM_B2_LEFT	=	PORTD3
 * PWM_LEFT		=	PORTD2
 * 
 */
pwm_dc_init_io:
; set i/o for output
	sbi		DDRD, PWM_RIGHT
	sbi		DDRD, PWM_A1_RIGHT
	sbi		DDRD, PWM_A2_RIGHT
	sbi		DDRD, PWM_LEFT
	sbi		DDRD, PWM_B1_LEFT
	sbi		DDRD, PWM_B2_LEFT
;
	sbi		DDRB, PWM_STBY
;
	ret

/*
 * Set STBY On
 */
 pwm_stby_on:
	sbi		PORTB, PWM_STBY
 	ret

/*
 * Set STBY Off
 */
 pwm_stby_off:
	cbi		PORTB, PWM_STBY
 	ret

/*
 * Set PWM RIGHT to STOP
 * Rate is N * 32us for DC Motor control
 *
 * input:	none
 */
pwm_stop_right:
	ldi		R17, PWM_R_STOP
	cbi		PORTD, PWM_A1_RIGHT
	cbi		PORTD, PWM_A2_RIGHT
	rjmp	pwm_set_right
/*
 * Adjust PWM rate RIGHT
 * Rate is N * 32us for DC Motor control
 *
 * input:	R17 - signed value
 * NOTE: No range check
 */
pwm_adj_right:
	lds		R16, pwm_val_right
	add		R17, R16
	rjmp	pwm_set_right
/*
 * Set PWM rate RIGHT
 * Rate is N * 32us for DC Motor control
 *
 * input:	R17	- Data
 */
pwm_set_right:
	sts		pwm_val_right, R17
	sts		OCR1AL, R17
	ret

/*
 * Set PWM direction RIGHT
 *
 * input:	R17	- DIR: 0-FWD..1-REV
 */
pwm_set_right_dir:
	tst		r17
	breq	psrd_skip00
; FWD
	sbi		PORTD, PWM_A2_RIGHT
	cbi		PORTD, PWM_A1_RIGHT
	ret
;
psrd_skip00:
; REV
	cbi		PORTD, PWM_A2_RIGHT
	sbi		PORTD, PWM_A1_RIGHT
	ret

/*
 * Set PWM LEFT to STOP
 * Rate is N * 32us for DC Motor control
 *
 * input:	none
 */
pwm_stop_left:
	ldi		R17, PWM_L_STOP
	cbi		PORTD, PWM_B1_LEFT
	cbi		PORTD, PWM_B2_LEFT
	rjmp	pwm_set_left
/*
 * Adjust PWM rate LEFT
 * Rate is N * 32us for DC Motor control
 *
 * input:	R17 - signed value
 * NOTE: No range check
 */
pwm_adj_left:
	lds		R16, pwm_val_left
	add		R17, R16
	rjmp	pwm_set_left
/*
 * Set PWM rate LEFT
 * Rate is N * 32us for DC Motor control
 *
 * input:	R17	- Data
 */
pwm_set_left:
	sts		pwm_val_left, R17
	sts		OCR1BL, R17
	ret

/*
 * Set PWM direction LEFT
 *
 * input:	R17	- DIR: 0-FWD..1-REV
 */
pwm_set_left_dir:
	tst		r17
	breq	psld_skip00
; FWD
	sbi		PORTD, PWM_B2_LEFT
	cbi		PORTD, PWM_B1_LEFT
	ret
;
psld_skip00:
; REV
	cbi		PORTD, PWM_B2_LEFT
	sbi		PORTD, PWM_B1_LEFT
	ret

/*
 * Start PWM DC Motor Pulses
 *
 * Call this at a 10ms rate.
 *
 * SAFE: Can call from interrupt service.
 *
 */
pwm_start_pulse:
	push	R16
;
; Enable Clock
; WGM13:2 = 00, CS12:0 = 101, CPU div 1024 = 51.2 us
	ldi		r16, (0<<WGM13)|(0<<WGM12)|(1<<CS12)|(0<<CS11)|(1<<CS10)
	sts		TCCR1B, r16
; Set PWM
	sbi		PORTD, PWM_RIGHT
	sbi		PORTD, PWM_LEFT
;
	pop		R16
	ret

/*
 * Timer/Counter1 Compare Match A
 * If tmrB has tripped,
 *   stop Timer 1
 *   clear ALL pwm flags
 * else
 *   set flagA
 */
pwm_tmr1A_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R16
;
	cbi		PORTD, PWM_RIGHT
; check flag
	lds		R16, pwm_flags
	andi	R16, PWM_FLAG_LEFT
	breq	pr1A_skip00
; B set..stop and clear
	clr		R16
	sts		TCCR1B, R16			; STOP Timer 1
	sts		TCNT1H, R16			; RESET Timer 1
	sts		TCNT1L, R16
	sts		pwm_flags, R16		; clear flags
	rjmp	pr1A_exit
;
pr1A_skip00:
	ldi		R16, PWM_FLAG_RIGHT
	sts		pwm_flags, R16
;
pr1A_exit:
	pop	R16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti

/*
 * Timer/Counter1 Compare Match B
 * If tmrA has tripped,
 *   stop Timer 1
 *   clear ALL pwm flags
 * else
 *   set flagB
 */
pwm_tmr1B_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R16
;
	cbi		PORTD, PWM_LEFT
; check flag
	lds		R16, pwm_flags
	andi	R16, PWM_FLAG_RIGHT
	breq	pr1B_skip00
; A set..stop and clear
	clr		R16
	sts		TCCR1B, R16			; STOP Timer 1
	sts		TCNT1H, R16			; RESET Timer 1
	sts		TCNT1L, R16
	sts		pwm_flags, R16		; clear flags
	rjmp	pr1B_exit
;
pr1B_skip00:
	ldi		R16, PWM_FLAG_LEFT
	sts		pwm_flags, R16
;
pr1B_exit:
	pop	R16
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
