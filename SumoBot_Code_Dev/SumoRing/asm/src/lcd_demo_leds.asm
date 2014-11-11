/*
 * LCD_CDM-16100 Demo Board LEDs
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board w/ 3 LEDs
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	LED_GREEN	= PORTB6
.equ	LED_RED		= PORTB7
.equ	LED_YELLOW	= PORTB0


.DSEG
led_count:		.BYTE	1


.CSEG
init_led_io:
; Disable /RST with fuse bit RSTDISBL
; Set as outputs
	sbi		DDRB, LED_GREEN
	sbi		DDRB, LED_RED
	sbi		DDRB, LED_YELLOW
;
	ret

turn_on_green:
	cbi		PORTB, 	LED_GREEN
	ret

turn_off_green:
	sbi		PORTB, 	LED_GREEN
	ret

turn_on_red:
	cbi		PORTB, 	LED_RED
	ret

turn_off_red:
	sbi		PORTB, 	LED_RED
	ret

turn_on_yellow:
	cbi		PORTB, 	LED_YELLOW
	ret

turn_off_yellow:
	sbi		PORTB, 	LED_YELLOW
	ret


/* count through 3 LEDs */
led_cycle:
	lds		R16, led_count
; switch( count )
	cpi		R16, 0
	brne	lc_skip01
	call	turn_off_green
	call	turn_off_red
	call	turn_off_yellow
	rjmp	lc_exit
;
lc_skip01:
	cpi		R16, 1
	brne	lc_skip02
	call	turn_off_green
	call	turn_off_red
	call	turn_on_yellow
	rjmp	lc_exit
;
lc_skip02:
	cpi		R16, 2
	brne	lc_skip03
	call	turn_off_green
	call	turn_on_red
	call	turn_off_yellow
	rjmp	lc_exit
;
lc_skip03:
	cpi		R16, 3
	brne	lc_skip04
	call	turn_off_green
	call	turn_on_red
	call	turn_on_yellow
	rjmp	lc_exit
;
lc_skip04:
	cpi		R16, 4
	brne	lc_skip05
	call	turn_on_green
	call	turn_off_red
	call	turn_off_yellow
	rjmp	lc_exit
;
lc_skip05:
	cpi		R16, 5
	brne	lc_skip06
	call	turn_on_green
	call	turn_off_red
	call	turn_on_yellow
	rjmp	lc_exit
;
lc_skip06:
	cpi		R16, 6
	brne	lc_skip07
	call	turn_on_green
	call	turn_on_red
	call	turn_off_yellow
	rjmp	lc_exit
;
lc_skip07:
	call	turn_on_green
	call	turn_on_red
	call	turn_on_yellow
	clr		R16
	rjmp	lc_reset			; reset
;
lc_exit:
	inc		R16
lc_reset:
	sts		led_count, R16
	ret
