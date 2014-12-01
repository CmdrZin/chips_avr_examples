/*
 * LCD_CDM-16100 Display Text Service
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board w/ LCD display, 8MHz w/65ms reset delay,
 *
 * CDM-16100 is organized as a 2x40 char display with one display line split into two sections.
 * Line 1 used the first 8 char of display. Using DDRAM 0x00:0x27
 * Line 2 used the last 8 char of display. Using DDRAM 0x40:0x67
 * Shifing the display shifts BOTH lines.
 *
 * Example:					Display
 *	Line 1: 0123456789A		01234567abcdefgh
 *	Line 2:	abcdefghijk
 *
 *	Shift Left once:		12345678bcdefghi
 */

.equ	LCD_DELAY_COUNT	=	200		; 2 sec

.equ	LCD_MODE_STATIC	=	1
.equ	LCD_MODE_CYCLE	=	2
.equ	LCD_DISPLAY_MODE	= LCD_MODE_CYCLE


.DSEG
lcd_text_line:			.BYTE	1
lcd_text_delay_count:	.BYTE	1
lcd_text_mode:			.BYTE	1

.CSEG



/*
 * Initialize LCD Text Parameters
 *
 * 
 */
lcd_text_init:
	clr		R16
	sts		lcd_text_line, R16
;
	ldi		R16, LCD_DELAY_COUNT
	sts		lcd_text_delay_count, R16
;
	ldi		R16, LCD_DISPLAY_MODE
	sts		lcd_text_mode, R16
;
	ret

/*
 * lcd_text_service()
 *
 * Display text on display.
 *
 * input reg:	R17	- Line number 0-9
 *				Z	- Pointer to text
 * output reg:	none
 * resources:	R16
 *
 * Test 10ms flag..consume
 * if delay == 0
 * 	delay = 250
 * else
 * 	--delay
 * 	Exit
 * if MODE == CYCLE
 * 	if Line == 5
 * 		Line = 0
 * 	set_dgram( Line<<3 )		; changes both lines
 * 	++Line
 * 	Exit
 * else
 * 	Exit
 */
lcd_text_service:
	sbis	GPIOR0, LCD_TIC			; test 10ms tic
	ret								; EXIT..not set
;
	cbi		GPIOR0, LCD_TIC			; clear tic10ms flag set by interrup
// Run service
	lds		R16, lcd_text_delay_count
	dec		R16
	sts		lcd_text_delay_count, R16
	breq	lts_skip00
	ret								; EXIT..not done
;
lts_skip00:
; Run Service
	ldi		R16, LCD_DELAY_COUNT
	sts		lcd_text_delay_count, R16
;
	lds		R16, lcd_text_mode		; get mode
; switch(mode)
	cpi		R16, LCD_MODE_STATIC
	brne	lts_skip10
; Keep on first line.
	call	lcd_return_home
	ret								; EXIT..Reset to HOME
;
lts_skip10:
	cpi		R16, LCD_MODE_CYCLE
	brne	lts_skip20
; Cycle lines.
	lds		R17, lcd_text_line		; get line
	inc		R17
	cpi		R17, 5					; max line
	brne	lts_skip01
	clr		R17						; reset
	sts		lcd_text_line, R17		; update
;
	call	lcd_return_home
	ret								; EXIT
;
lts_skip01:
	sts		lcd_text_line, R17		; update
	ldi		R18, 8
lts_loop00:
	ldi		R17, LCD_SHIFT_DISPLAY_LEFT
	push	R18
	call	lcd_cursor_display_shift
	pop		R18
	dec		R18
	brne	lts_loop00
;
	ret								; EXIT

; default
lts_skip20:
	ldi		R16, LCD_MODE_STATIC
	sts		lcd_text_mode, R16
	ret								; EXIT


/*
 * lcd_set_text_right(line, &buffer, n)
 * Starts with position 8.
 * Pad right side unused char with spaces.
 *
 * input:	r17		Line index (1-5)
 *			Y		Ref to SRAM buffer
 *			r18		Number of bytes
 *
 * 	(R17-1)<<3]+0x40	(x8)+0x40
 * 	call set_dgram_adrs( R17 ) (40us)
 * 	do()
 * 		call send_data( *Y+ )
 *		while(--n)
 */
 lcd_set_text_right:
 	push	r18			; save n
	dec		r17			; adjust index to zero base
	lsl		r17			; x8
	lsl		r17
	lsl		r17
	ldi		r16, 0x40	; add address offset
	add		r17, r16
;
	push	YL
	push	YH
	push	r18
	call	lcd_set_ddram_adrs
	pop		r18
	pop		YH
	pop		YL
;
lstr_loop0:
	ld		r17, Y+
	call	lcd_write_data
	dec		r18
	brne	lstr_loop0
; Zero rest of charaters
	pop		r17			; get n
	ldi		r18, 8
	sub		r18, r17	; 8 - n = blanks
	breq	lstr_exit
lstr_loop1:
	ldi		r17, ' '
	call	lcd_write_data
	dec		r18
	brne	lstr_loop1
;
lstr_exit:
 	ret
 
/*
 * lcd_set_text_left(line, &buffer, n)
 * Starts with position 0.
 * Pad right side unused char with spaces.
 *
 * input:	r17		Line index (1-5)
 *			Y		Ref to SRAM buffer
 *			n		Number of bytes
 *
 * 	(R17-1)<<3	(x8)
 * 	call set_dgram_adrs( R17 ) (40us)
 * 	do()
 * 		call send_data( *Y+ )
 *		while(--n)
 */
lcd_set_text_left:
 	push	r18			; save n
	dec		r17			; adjust index to zero base
	lsl		r17			; x8
	lsl		r17
	lsl		r17
;
	push	YL
	push	YH
	push	r18
	call	lcd_set_ddram_adrs
	pop		r18
	pop		YH
	pop		YL
;
lstl_loop:
	ld		r17, Y+
	call	lcd_write_data
	dec		r18
	brne	lstl_loop
; Zero rest of charaters
	pop		r17			; get n
	ldi		r18, 8
	sub		r18, r17	; 8 - n = blanks
	breq	lstl_exit
lstl_loop1:
	ldi		r17, ' '
	call	lcd_write_data
	dec		r18
	brne	lstl_loop1
;
lstl_exit:
	ret

/*
 * lcd_set_sram_text_line_n(R17, Y, R18)
 *
 * Set 16 char text line from SRAM
 * Send first 8 char to Line and next 8 char
 * to Line+5.
 * 
 * input reg:	R17	- Line number 1-5
 *				Y	- Pointer to text
 *				R18 - byte count
 *
 * output reg:	none
 *
 * resources:	R16
 *
 */
lcd_set_sram_text_line_n:
	cpi		r18, 9
	brge	lsstln_skip00
// 8 or less characters.
	call	lcd_set_text_left
	ret
lsstln_skip00:
// More than 8 characters.
	push	r18						; save byte count
	push	r17						; save line number
	ldi		r18, 8
	call	lcd_set_text_left
	pop		r17
	pop		r18
	subi	r18, 8
	call	lcd_set_text_right
	ret


/*
 * lcd_set_sram_text(R17, X)
 *
 * Set text from SRAM data.
 * 
 * input reg:	R17	- Line number 0-9
 *				X	- Pointer to text. NULL treminated.
 * output reg:	none
 * resources:	R16
 *
 */
lcd_set_sram_text:
	cpi		R17, 5
	brge	lsst_skip00
; Line 0-4..0x00-0x27
	ldi		R16, 0
	rjmp	lsst_skip10
;
lsst_skip00:
; Line 5-8..0x40-0x67
	ldi		R16, 0x40
lsst_skip10:
	subi	R17, 5				; normalize
	lsl		R17					; adj for 8 char
	lsl		R17
	lsl		R17
	add		R17, R16
	call	lcd_set_ddram_adrs
	call	out_ram_text
;
	ret

/*
 * lcd_set_sram_text_n(R17, X, R18)
 *
 * Set text from SRAM data.
 * 
 * input reg:	R17	- Line number 0-9
 *				X	- Pointer to text
 *				R18 - max byte count
 * output reg:	none
 * resources:	R16
 *
 */
lcd_set_sram_text_n:
	cpi		R17, 5
	brge	lsstn_skip00
; Line 0-4..0x00-0x27
	ldi		R16, 0
	rjmp	lsstn_skip10
;
lsstn_skip00:
; Line 5-8..0x40-0x67
	ldi		R16, 0x40
lsstn_skip10:
	subi	R17, 5				; normalize
	lsl		R17					; adj for 8 char
	lsl		R17
	lsl		R17
	add		R17, R16
	call	lcd_set_ddram_adrs
	mov		R17, R18
	call	out_ram_text_n
;
	ret


/*
 * lcd_set_flash_text(R17, Z)
 *
 * Set text from Flash data.
 * 
 * input reg:	R17	- Line number 0-9
 *				Z	- Pointer to text. NULL terminated.
 * output reg:	none
 * resources:	R16
 *
 */
lcd_set_flash_text:
	lsl		R17					; adj for 8 char
	lsl		R17
	lsl		R17
	call	lcd_set_ddram_adrs
;
	call	out_flash_banner
;
	ret


/*
 * Get the display pattern
 * input reg:	X-> - data
 *
 * NOTE: Uses NULL terminated string
 */
out_ram_text:
;
ort_loop00:
	ld		R16, X+				; get char
	tst		R16
	breq	orb_exit			; Done
;
	mov		R17, R16
	call	lcd_write_data
	rjmp	ort_loop00
;
orb_exit:
	ret

/*
 * Get the display pattern
 * input reg:	X-> - data
 *				R17 - max bytes
 *
 * NOTE: Uses NULL terminated string OR max bytes.
 */
out_ram_text_n:
;
ortn_loop00:
	tst		R17
	breq	ortn_exit
	dec		R17
;
	ld		R16, X+				; get char
	tst		R16
	breq	ortn_exit			; Done
;
	push	r17
	mov		R17, R16
	call	lcd_write_data
	pop		r17
	rjmp	ortn_loop00
;
ortn_exit:
	ret

/*
 * Copy FLASH text into LCD DDRAM
 *
 * input reg:	Z	Pointer to start of text in FLASH.
 *
 * NOTE: Uses NULL terminated string.
 */
out_flash_banner:
;
ofb_loop00:
	lpm		R17, Z+				; get char
	tst		R17
	breq	ofb_exit			; Done
	call	lcd_write_data
	rjmp	ofb_loop00
;
ofb_exit:
	ret
;
/* *** Preset labels in FLASH *** */
lcd_bt_text00:
.db		'L','i','n','e',':','0',' ',' '
lcd_bt_text01:
.db		'L','i','n','e',':','1',' ',' '
lcd_bt_text02:
.db		'L','i','n','e',':','2',' ',' '
lcd_bt_text03:
.db		'L','i','n','e',':','3',' ',' '
lcd_bt_text04:
.db		'L','i','n','e',':','4',' ',' ',0,0

lcd_clr_text:
.db		' ',' ',' ',' ',' ',' ',' ',' ',0,0

lcd_bt_text05:
.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'
.db		'q','r','s','t','u','v','w','x','y','z','.',',','<','>','?','/'
.db		'~','!','@','#','$','%','^','&',0,0

lcd_bt_text06:
.db		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P'
.db		'Q','R','S','T','U','V','W','X','Y','Z','*','(',')','_','+','{'
.db		'}','[',']',':',';','"',''','|',0,0

lcd_bt_hellow:
.db		'H','e','l','l','o',' ',' ',' '
.db		'W','o','r','l','d',' ',' ',' '
.db		'H','a','v','e',' ','a',' ',' '
.db		'N','i','c','e',' ','D','a','y'
.db		' ','o','r',' ','N','O','T','!',0,0

lcd_bt_text07:
.db		'M','O','D','E',':',' ',' ',' ',0,0

lcd_bt_text08:
.db		'S','Q','U','A','R','E',' ',' ',0,0

lcd_bt_text09:
.db		'A','V','O','I','D',' ',' ',' ',0,0
