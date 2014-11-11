/*
 * LCD_CDM-16100 Display Driver - 4 bit interface
 *
 * org: 9/29/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board w/ LCD display and keypad I/O, 8MHz
 * LCD control: ST7066U
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */


.equ	LC_CLEAR_DISPLAY		= 0x01	; Write 0x20 to DDRAM and reset AC to 00. [1.52ms]
.equ	LC_RETURN_HOME			= 0x02	; Reset AC to 00 and return cursor to home. [1.52ms]
.equ	LC_SET_ENTRY_MODE		= 0x04	; b1: I/D, b0: S Set cursor move and display shift. [37us]
.equ	LC_DISPLAY_ONOFF		= 0x08	; b2: Display, b1: Cursor, b0: Cursor position.  [37us]
.equ	LC_CURSOR_DISPLAY_SHIFT	= 0x10	; b3: Select, b2: R/L. [37us]
.equ	LC_SET_FUNCTION			= 0x20	; b4: Interface, b3: Lines, b2: Font. [37us]
.equ	LC_SET_CGRAM_ADRS		= 0x40	; b5:0 CGRAM Address. [37us]
.equ	LC_SET_DDRAM_ADRS		= 0x80	; b6:0 DDRAM Address. [37us]


.equ	LCD_SHIFT_CURSOR_LEFT	= 0b00000000
.equ	LCD_SHIFT_CURSOR_RIGHT	= 0b00000100
.equ	LCD_SHIFT_DISPLAY_LEFT	= 0b00001000
.equ	LCD_SHIFT_DISPLAY_RIGHT	= 0b00001100


; PORTC bits
.equ	LCD_RS = 3
.equ	LCD_RW = 2
.equ	LCD_E  = 1

/*
 * lcd_delay_40us()
 * Tweak delay loop based on CPU clock
 * Call twice for 80us and 39 times for 1.52ms
 *
 * For 8MHz
 *
 */
lcd_delay_40us:
	push	R16
;
	ldi		R16, 40	; 8 * 40 cycles
; 8 cycle loop = 1us
ld1_loop1:
	nop
	nop
	nop
	nop
	nop
	dec		R16
	brne	ld1_loop1
;
	pop		R16
	ret

/*
 * lcd_delay_1ms(n)
 * Tweak delay loop based on CPU clock
 *
 * input: 	R17 - n ms
 * For 8MHz
 *
 */
lcd_delay_1ms:
	push	R16
;
ld1m_loop2:
	ldi		R16, 25
ld1m_loop1:
	call	lcd_delay_40us
	dec		R16
	brne	ld1m_loop1
	dec		R17
	brne	ld1m_loop2
;
	pop		R16
	ret

/*
 * lcd_init()
 * Set up I/O and LDC module for 8 bit access.
 *
 * PortD.7:0	- Data 7:0	in/out
 * PortC.3		- RS		out
 * PortC.2		- R/W		out
 * PortC.1		- E			out
 * 
 */
lcd_init:
; Set up I/O
	sbi		DDRC, LCD_RS
	sbi		DDRC, LCD_RW
	sbi		DDRC, LCD_E
	cbi		PORTC, LCD_RS
	cbi		PORTC, LCD_RW
	cbi		PORTC, LCD_E
; Wait 40ms after power up.
	ldi		R17, 40
	call	lcd_delay_1ms
; Interface starts as 8 bit access
	ldi		R17, 0x20				; Function set..chenage from 8bit to 4bit
	call	lcd_write_ins_once
	ldi		R17, 40
	call	lcd_delay_1ms
; Set interface for 4 bit access
	ldi		R17, 0x2C				; Function set..MUST write TWICE
	call	lcd_write_ins_nochk
	call	lcd_delay_40us
	ldi		R17, 40
	call	lcd_delay_1ms
;	
; Set interface for 4 bit access
	ldi		R17, 0x2C				; Function set..MUST write TWICE
	call	lcd_write_ins_nochk
	call	lcd_delay_40us
	ldi		R17, 40
	call	lcd_delay_1ms
;	
	call	lcd_clear_display
;
	ldi		R17, 0b00001110			; Display ON, Cursor ON, Blink OFF
	call	lcd_write_ins_nochk
	ldi		R17, 1
	call	lcd_delay_1ms
;	
	ldi		R18, 1
	ldi		R19, 1
	call	lcd_set_entry_mode
;
	ret

/*
 * Instruction Access
 * input: 	R17 - Data
 */
lcd_write_ins_chk:
	call	lcd_ckeck_busy			; Blocking call while Busy
lcd_write_ins_nochk:
	cbi		PORTC, LCD_RS
	cbi		PORTC, LCD_RW
	call	lcd_raw_write
	swap	R17
lcd_write_ins_once:
	call	lcd_raw_write
	ret

/* Blocking call while Busy */
lcd_ckeck_busy:
	push	R17
lcb_loop00:
	call	lcd_delay_40us		; wait 80us
	call	lcd_delay_40us
;
	cbi		PORTC, LCD_RS
	sbi		PORTC, LCD_RW
	call	lcd_raw_read		; read upper 4 bits
	push	R17
	call	lcd_raw_read		; read upper 4 bits
	pop		R17
	lsl		R17					; test Busy bit..D7
	brcs	lcb_loop00			; BS=1..Busy
;
	cbi		PORTC, LCD_RS
	cbi		PORTC, LCD_RW
;
	pop		R17
	ret

/*
 * output:	R17 - Data in
 */
lcd_raw_read:
	push	R16
	clr		R16
	out		DDRD, R16			; set for input
	ser		R16
	out		PORTD, R16			; turn on pull-ups
;
	sbi		PORTC, LCD_E		; min PW = 500ns..4 cycles @ 8MHz
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	in		R17, PORTD
	nop
	cbi		PORTC, LCD_E
;
	pop		R16
	ret

/*
 * output:	R17 - Data out
 */
lcd_raw_write:
	push	R16
	ldi		R16, 0xF0
	out		DDRD, R16			; set for output
	nop
	nop
;
	out		PORTD, R17
	nop
	nop
;
	sbi		PORTC, LCD_E		; min PW = 500ns..4 cycles @ 8MHz
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	cbi		PORTC, LCD_E
;
	pop		R16
	ret


/*
 * lcd_clear_display()
 */
lcd_clear_display:
	ldi		R17, LC_CLEAR_DISPLAY
	call	lcd_write_ins_nochk
	ldi		R17, 10					; 1.52ms delay
	call	lcd_delay_1ms
	ret

/*
 * lcd_return_home()
 */
lcd_return_home:
	ldi		R17, LC_RETURN_HOME
	call	lcd_write_ins_nochk
	ldi		R17, 10					; 1.52ms delay
	call	lcd_delay_1ms
	ret

/*
 * lcd_set_entry_mode( direction, shift_enable )
 *
 * input	R18		Adrs Direction. 0: Left (Dec), 1:Right (Inc)
 *			R19		Enable display shift. 0:False, 1:True
 */
lcd_set_entry_mode:
	ldi		R17, LC_SET_ENTRY_MODE
	tst		R18
	breq	lse_skip00
	ori		R17, 0x02				; Inc and move cursor right
;
lse_skip00:
	tst		R19
	breq	lse_skip01
	ori		R17, 0x01				; Display shift with cursor
;
lse_skip01:
	call	lcd_write_ins_nochk
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
;
	ret

/*
 * lcd_display_on_off( display, cursor, blink )
 */
lcd_display_on_off:

	ret

/*
 * lcd_cursor_display_shift( LCD_SHIFT_XXXX )
 * Shifts the cursor position one step.
 *
 * input:	R17 - Control
 *
 *	LCD_SHIFT_CURSOR_LEFT
 *	LCD_SHIFT_CURSOR_RIGHT
 *	LCD_SHIFT_DISPLAY_LEFT
 *	LCD_SHIFT_DISPLAY_RIGHT
 */
lcd_cursor_display_shift:
	ori		R17, LC_CURSOR_DISPLAY_SHIFT
	call	lcd_write_ins_nochk
	call	lcd_delay_40us
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
	ret

/*
 * lcd_set_cgram_adrs( address )
 * input:	R17 - CGRAM addres
 */
lcd_set_cgram_adrs:
	ori		R17, 0x40
	call	lcd_write_ins_nochk
	call	lcd_delay_40us
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
	ret

/*
 * lcd_set_ddram_adrs( address )
 */
lcd_set_ddram_adrs:
	ori		R17, 0x80
	call	lcd_write_ins_nochk
	call	lcd_delay_40us
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
	ret

/*
 * char lcd_get_busy_flag()
 * Non-blocking Busy Flag read.
 * output:	R17	- b7: Busy Flag, b6:0 DDRAM/CGRAM address
 */
lcd_get_busy_flag:
	cbi		PORTC, LCD_RS
	sbi		PORTC, LCD_RW
	call	lcd_raw_read
	push	R17
	call	lcd_raw_read
;
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
;
	cbi		PORTC, LCD_RW
	pop		R17
	ret

/*
 * lcd_write_data( data )
 * input: 	R17 - Data
 *
 */
lcd_write_data:
	sbi		PORTC, LCD_RS
	cbi		PORTC, LCD_RW
	call	lcd_raw_write
	swap	R17
	call	lcd_raw_write
;
	call	lcd_delay_40us
	call	lcd_delay_40us
;
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
;
	call	lcd_get_busy_flag
;
	cbi		PORTC, LCD_RS
	ret

/*
 * char lcd_read_data()
 * output: 	R17 - Data
 *
 */
lcd_read_data:
	sbi		PORTC, LCD_RS
	sbi		PORTC, LCD_RW
	call	lcd_raw_read
	push	R17
	call	lcd_raw_read
	swap	R17
	andi	R17, 0x0F
	pop		R16
	andi	R16, 0xF0
	or		R17, R16			; combine
	push	R17
;
	call	lcd_delay_40us
	call	lcd_delay_40us
;
	ldi		R17, 1
	call	lcd_delay_1ms			; HACK until BF check resolved.
;
	call	lcd_get_busy_flag
;
	pop		R17
	ret

