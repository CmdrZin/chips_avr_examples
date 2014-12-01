/*
 * LCD_CDM-16100 Display Demo Board
 *
 * org: 10/07/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board w/ LCD display and keypad I/O, 8MHz w/65ms reset delay, RSTDISBL
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
lcd_board_test:
	call	turn_on_green
	call	turn_on_red
	call	turn_on_yellow
;
lbt_loop00:
;
;	call	turn_on_green
;	call	turn_off_red
;	call	lcd_delay_40us
;	call	turn_on_red
;	call	turn_off_green
;
	clr		R16
	sts		led_count, R16
	call	led_cycle
;

	ldi		R17, 0x00
	call	lcd_set_ddram_adrs
	call	out_banner
;
	ldi		R17, 0x40
	call	lcd_set_ddram_adrs
	call	out_banner2
;
	call	lcd_return_home
;
	ldi		R20, 40
ibt_loop01:
	ldi		R17, 250
	call	lcd_delay_1ms
	ldi		R17, 250
	call	lcd_delay_1ms

	ldi		R17, LCD_SHIFT_DISPLAY_LEFT
	call	lcd_cursor_display_shift
;
	dec		R20
	brne	ibt_loop01
;
	ldi		R20, 40
ibt_loop02:
	ldi		R17, 250
	call	lcd_delay_1ms
	ldi		R17, 250
	call	lcd_delay_1ms

	ldi		R17, LCD_SHIFT_DISPLAY_RIGHT
	call	lcd_cursor_display_shift
;
	dec		R20
	brne	ibt_loop02
;


;	rjmp	lbt_loop00
	ret


/*
 * Get the display pattern
 * input reg:	R17 - data
 *
 * Use offset read lookup table element
 */
out_banner2:
	ldi		ZH, high(lcd_bt_text2<<1)	; Initialize Z pointer
	ldi		ZL, low(lcd_bt_text2<<1)
	rjmp	ob_loop00
;
out_banner:
	ldi		ZH, high(lcd_bt_text<<1)	; Initialize Z pointer
	ldi		ZL, low(lcd_bt_text<<1)
;
ob_loop00:
	lpm		R17, Z+					; get char
	tst		R17
	breq	ob_exit					; Done
	call	lcd_write_data
	rjmp	ob_loop00
;
ob_exit:
	ret
;
lcd_bt_text:
; 1st line uses 0x00-0x27
.db		'a','-','b','-','c','-','d','-','e','-','f','-','g','-','h','-'
.db		'i','-','j','-','k','-','l','-','m','-','n','-','o','-','p','-'
.db		'q','-','r','-','s','-','t','-',0,0


.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'
.db		'q','r','s','t','u','v','w','x',0,0
.db		'0','1','2','3','4','5','6','7','8','9','!','@','#','$','%','^'
.db		'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V'
.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','-','*','#',0
.db		'H','e','l','l','o','w',' ','W','o','r','l','d','&','*','(',')'
.db		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P'
.db		'Q','R','S','T','U','V','W','X',0,0


lcd_bt_text2:
; 2nd line uses 0x40-0x67
.db		'A','.','B','.','C','.','D','.','E','.','F','.','G','.','H','.'
.db		'I','.','J','.','K','.','L','.','M','.','N','.','O','.','P','.'
.db		'Q','.','R','.','S','.','T','.',0,0


.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'


init_led_io:
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
