/*
 * PWM Servo Lib
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board, 8MHz
 *
 * Resources
 * SRAM
 *
 * IO
 *
 * PWM servo driver. 1.0ms(Fwd) - 1.5ms (Stop) - 2.0ms (Rev) pulse at 10-20ms period.
 *		PB1 and PB2 link to PWM counters.
 */

.equ	PWM_RIGHT	=	1
.equ	PWM_LEFT	=	2

.equ	PWM_FLAG_RIGHT	=	0x01
.equ	PWM_FLAG_LEFT	=	0x02

.equ	PWM_R_STOP		=	125+78
.equ	PWM_R_SLOW_F	=	PWM_R_STOP
.equ	PWM_R_SLOW_R	=	PWM_R_STOP
.equ	PWM_R_MED_F		=	PWM_R_STOP-15
.equ	PWM_R_MED_R		=	PWM_R_STOP+15
.equ	PWM_R_FAST_F	=	PWM_R_STOP
.equ	PWM_R_FAST_R	=	PWM_R_STOP

.equ	PWM_L_STOP		=	125+66
.equ	PWM_L_SLOW_F	=	PWM_L_STOP
.equ	PWM_L_SLOW_R	=	PWM_L_STOP
.equ	PWM_L_MED_F		=	PWM_L_STOP+15
.equ	PWM_L_MED_R		=	PWM_L_STOP-15
.equ	PWM_L_FAST_F	=	PWM_L_STOP
.equ	PWM_L_FAST_R	=	PWM_L_STOP


.DSEG
pwm_flags:		.BYTE	1
pwm_val_right:	.BYTE	1		; 8us res..125 = 1ms, 250 = 2ms..High = 0
pwm_val_left:	.BYTE	1		; 8us res..125 = 1ms, 250 = 2ms..High = 0
pwm_20ms_cnt:	.BYTE	1

.CSEG
/*
 * pwm_init()
 * Set up I/O and PWM counters
 *
 * PortB.2		- Ch 2 servo
 * PortB.1		- Ch 1 servo
 * 
 */
pwm_init:
	sbi		DDRB, PWM_RIGHT
	sbi		DDRB, PWM_LEFT
; Set up Timer 1
	clr		R16
	sts		OCR1AH, R16
	sts		OCR1BH, R16
	call	pwm_stop_right
	call	pwm_stop_left
;
	ldi		R16, (OCIE1A<<1)|(OCIE1B<<1)
	sts		TIMSK1, R16
;
	ldi		R16, (0<<WGM11)|(0<<WGM10)
	sts		TCCR1A, R16
;
	ret

/*
 * Set PWM RIGHT to STOP
 * Rate is N * 8us
 *
 */
pwm_stop_right:
	ldi		R17, PWM_R_STOP
	rjmp	pwm_set_right
/*
 * Adjust PWM rate RIGHT
 * Rate is N * 8us
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
 * Rate is N * 8us
 *
 * input:	R17	- Data
 */
pwm_set_right:
	sts		pwm_val_right, R17
	sts		OCR1AL, R17
	ret

/*
 * Set PWM LEFT to STOP
 * Rate is N * 8us
 *
 */
pwm_stop_left:
	ldi		R17, PWM_L_STOP
	rjmp	pwm_set_left
/*
 * Adjust PWM rate LEFT
 * Rate is N * 8us
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
 * Rate is N * 8us
 *
 * input:	R17	- Data
 */
pwm_set_left:
	sts		pwm_val_left, R17
	sts		OCR1BL, R17
	ret

/*
 * Start PWM Servo Pulses
 * Uses 8 MHz clock
 *
 * Call this at a 10ms to 20ms rate.
 * Can call from interrupt service.
 *
 */
pwm_start_pulse:
	push	R16
;
	lds		R16, pwm_20ms_cnt
	tst		R16
	brne	psp_skip00
	inc		R16
	sts		pwm_20ms_cnt, R16					; toggle
	rjmp	pwp_exit
;
psp_skip00:
	clr		R16
	sts		pwm_20ms_cnt, R16					; toggle
;
	sbi		PORTB, PWM_RIGHT
	sbi		PORTB, PWM_LEFT
;
	ldi		R16, (0<<WGM13)|(0<<WGM12)|0x03		; div 64 for 8us resolution
	sts		TCCR1B, R16							; START Timer 1
;
pwp_exit:
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
	cbi		PORTB, PWM_RIGHT
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
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	pop	R16
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
	cbi		PORTB, PWM_LEFT
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
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	pop	R16
	reti
