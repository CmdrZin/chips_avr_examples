/*
 * LCD_CDM-16100 Display Text Service
 *
 * org: 10/13/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board w/ LCD display, 8MHz w/65ms reset delay,
 *
 * Resources
 * SRAM
 *
 * IO
 *
 */

.equ	LCD_DELAY_COUNT	=	200		; 2 sec

.equ	LCD_MODE_STATIC	=	1
.equ	LCD_MODE_CYCLE	=	2


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
;;;	ldi		R16, LCD_MODE_STATIC
	ldi		R16, LCD_MODE_CYCLE
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
	ret								; EXIT..No adjust
;
lts_skip10:
	cpi		R16, LCD_MODE_CYCLE
	brne	lts_skip20
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
 * lcd_set_flash_text(R17, Z)
 *
 * Set text from Flash data.
 * 
 * input reg:	R17	- Line number 0-9
 *				X	- Pointer to text
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
 * lcd_set_flash_text(R17, Z)
 *
 * Set text from Flash data.
 * 
 * input reg:	R17	- Line number 0-9
 *				X	- Pointer to text
 *				R18 - byte count
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
 *				Z	- Pointer to text
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
 *				R17 - bytes
 *
 * NOTE: Uses NULL terminated string
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
	mov		R17, R16
	call	lcd_write_data
	rjmp	ortn_loop00
;
ortn_exit:
	ret

/*
 * Get the display pattern
 * input reg:	R17 - data
 *
 * Use offset read lookup table element
 */
out_flash_banner:
;
ofb_loop00:
	lpm		R17, Z+					; get char
	tst		R17
	breq	ofb_exit					; Done
	call	lcd_write_data
	rjmp	ofb_loop00
;
ofb_exit:
	ret
;
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
lcd_bt_text05:
.db		'L','i','n','e',':','0','0','5',0,0
lcd_bt_text06:
.db		'L','i','n','e',':','0','0','6',0,0
lcd_bt_text07:
.db		'L','i','n','e',':','0','0','7',0,0
lcd_bt_text08:
.db		'L','i','n','e',':','0','0','8',0,0
lcd_bt_text09:
.db		'L','i','n','e',':','0','0','9',0,0
lcd_clr_text:
.db		' ',' ',' ',' ',' ',' ',' ',' ',0,0

.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'
.db		'q','r','s','t','u','v','x','y','z','.',',','<','>','?','!','@'
.db		'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V'
.db		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','-','*','#',0

lcd_bt_hellow:
.db		'H','e','l','l','o',' ',' ',' '
.db		'W','o','r','l','d',' ',' ',' '
.db		'H','a','v','e',' ','a',' ',' '
.db		'N','i','c','e',' ','D','a','y',0,0

