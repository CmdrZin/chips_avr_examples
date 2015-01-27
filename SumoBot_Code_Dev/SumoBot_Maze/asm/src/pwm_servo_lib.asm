/*
 * PWM Servo Lib
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Maze Bot Board, 20MHz, ATmega164P, Sumo Bot base
 *
 * Dependentcies
 *	sys_timers.asm
 *
 * Resources
 * SRAM
 *
 * IO
 *
 * PWM servo driver. 1.0ms(Fwd) - 1.5ms (Stop) - 2.0ms (Rev) pulse at 40ms period.
 * Uses PWM Counters and direct output I/O pins OC1A & OC1B.
 *
 */


; Uses 16bit values
.equ	PWM_R_STOP		=	468
.equ	PWM_R_SLOW_F	=	PWM_R_STOP-39
.equ	PWM_R_SLOW_R	=	PWM_R_STOP+39
.equ	PWM_R_MED_F		=	PWM_R_STOP-78
.equ	PWM_R_MED_R		=	PWM_R_STOP+78
.equ	PWM_R_FAST_F	=	PWM_R_STOP-156
.equ	PWM_R_FAST_R	=	PWM_R_STOP+156

.equ	PWM_L_STOP		=	468
.equ	PWM_L_SLOW_F	=	PWM_L_STOP+39
.equ	PWM_L_SLOW_R	=	PWM_L_STOP-39
.equ	PWM_L_MED_F		=	PWM_L_STOP+78
.equ	PWM_L_MED_R		=	PWM_L_STOP-78
.equ	PWM_L_FAST_F	=	PWM_L_STOP+156
.equ	PWM_L_FAST_R	=	PWM_L_STOP-156


.DSEG
pwm_flags:		.BYTE	1
pwm_val_right:	.BYTE	2		; L:H 3.2us res..312 = 1ms, 625 = 2ms.
pwm_val_left:	.BYTE	2		; L:H 3.2us res..312 = 1ms, 625 = 2ms.


.CSEG
/*
 * pwm_init()
 * Set up I/O and PWM counters
 *
 * OC1B		- Ch 2 servo
 * OC1A		- Ch 1 servo
 *
 * Timer1 is set for 20 - 40ms duration in sys_timers.asm
 *
 * Set up Timer1 for Two Channel Servo 40ms PWM using 20MHz CPU clock
 * Call this once after RESET.
 *
 * Set for ~20ms with 1/64 and 20MHz clock..4.77us resolution
 *
 * input reg:	none
 * output reg:	none
 * resources:	r16, r17
 *
 * NOTE: Write H L, Read L H.
 * 
 */
pwm_init:
; Set up Timer 1
	ldi		r16, (1<<COM1A1)|(0<<COM1A0)|(1<<COM1B1)|(0<<COM1B0)|(1<<WGM11)|(0<<WGM10)	; Use OC1A & OC1B pins for PWM
	sts		TCCR1A, r16
	ldi		r16, (1<<WGM13)|(1<<WGM12)|(0<<CS12)|(1<<CS11)|(1<<CS10)	; WGM = 1110 for ICR1 as TOP..CPU div 8 = 0.4us
	sts		TCCR1B, r16
; Interrupts not used. TIMSK1 = 00 (default)
; DB++ Turn ON Timer1 OV interrupt
	ldi		r16, (1<<TOIE1)
	sts		TIMSK1, r16
; DB--
	ldi		r16, 0x30
	sts		ICR1H, r16
	ldi		r16, 0xD4
	sts		ICR1L, r16
;  Set IO pins for output.
	sbi		DDRD, PORTD4
	sbi		DDRD, PORTD5
	call	pwm_stop_right
	call	pwm_stop_left
;
	ret

/*
 * Set PWM RIGHT to STOP
 * Rate is N * 4.77us
 *
 */
pwm_stop_right:
	ldi		r17, low(PWM_R_STOP)
	ldi		r18, high(PWM_R_STOP)
	rjmp	pwm_set_right

/*
 * Add PWM rate RIGHT
 * Rate is N * 4.77us
 *
 * input:	r17:18	Data16 L:H
 *
 * NOTE: No range check
 */
pwm_add16_right:
	lds		r16, pwm_val_right
	add		r17, r16				; Low byte
	lds		r16, pwm_val_right+1
	adc		r18, r16				; High byte
	rjmp	pwm_set_right

/*
 * Subtract PWM rate RIGHT
 * Rate is N * 4.77us
 *
 * input:	r17:18	Data16 L:H
 *
 * NOTE: No range check
 */
pwm_sub16_right:
	lds		r16, pwm_val_right
	sub		r16, r17				; Low byte
	mov		r17, r16
	lds		r16, pwm_val_right+1
	sbc		r16, r18				; High byte
	mov		r18, r16
	rjmp	pwm_set_right

/*
 * Set PWM rate RIGHT
 * Rate is N * 4.77us
 *
 * input:	r17:18	- Data16 L:H
 */
pwm_set_right:
	sts		pwm_val_right, r17
	sts		pwm_val_right+1, r18
; NOTE: Write H L, Read L H to registers.
	sts		OCR1AH, r18
	sts		OCR1AL, r17
	ret

/*
 * Set PWM LEFT to STOP
 * Rate is N * 4.77us
 *
 */
pwm_stop_left:
	ldi		r17, low(PWM_L_STOP)
	ldi		r18, high(PWM_L_STOP)
	rjmp	pwm_set_left

/*
 * Add PWM rate LEFT
 * Rate is N * 4.77us
 *
 * input:	r17:18	Data16 L:H
 *
 * NOTE: No range check
 */
pwm_add16_left:
	lds		r16, pwm_val_left
	add		r17, r16				; Low byte
	lds		r16, pwm_val_left+1
	adc		r18, r16				; High byte
	rjmp	pwm_set_left

/*
 * Subtract PWM rate LEFT
 * Rate is N * 4.77us
 *
 * input:	r17:18	Data16 L:H
 *
 * NOTE: No range check
 */
pwm_sub16_left:
	lds		r16, pwm_val_left
	sub		r16, r17				; Low byte
	mov		r17, r16
	lds		r16, pwm_val_left+1
	sbc		r16, r18				; High byte
	mov		r18, r16
	rjmp	pwm_set_left

/*
 * Set PWM rate LEFT
 * Rate is N * 4.77us
 *
 * input:	R17	- Data
 */
pwm_set_left:
	sts		pwm_val_left, r17
	sts		pwm_val_left+1, r18
; NOTE: Write H L, Read L H to registers.
	sts		OCR1BH, r18
	sts		OCR1BL, r17
	ret

