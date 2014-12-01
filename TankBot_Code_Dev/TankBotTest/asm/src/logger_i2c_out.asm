/*
 * Logger I2C Output Service
 *
 * org: 11/17/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P
 *
 * Logger: LCD_CDM-16100 Demo board with a five line 1x16 character display.
 *
 * This service is used to output debugging text to an I2C Logger device.
 *
 * A table of predefined labels are provided.
 *
 * Dependentcies
 *   i2c_master.asm
 *
 */

// Avaliable Loggers
.equ	LCD_CDM_16100_DEMO	= 0x25
// Logger Slave Address
.equ	SLAVE_ADDRESS		= LCD_CDM_16100_DEMO

// Define a data buffer.
.equ	I2C_BUFF_OUT_SIZE	= 16		; write Slave data from here.


.DSEG
log_buffer:		.BYTE	I2C_BUFF_OUT_SIZE		; Text output buffer.
log_out_count:	.BYTE	1						; number of bytes in buffer.
log_buffer_XH:	.BYTE	1						; running pointer into buffer.
log_buffer_XL:	.BYTE	1

.CSEG
/*
 * Clear the output buffer and prepare for next message.
 * 
 */
logger_clear_buffer:
	ldi		r16, LOW(log_buffer)
	sts		log_buffer_XL, r16
	ldi		r16, HIGH(log_buffer)
	sts		log_buffer_XH, r16
	clr		r16
	sts		log_out_count, r16
	ret


/*
 * Send Output Buffer to Slave
 *
 */
 logger_send:
 	ldi		r17, SLAVE_ADDRESS
 	lds		r18, log_out_count				; get number of bytes to send.
	ldi		XL, LOW(log_buffer)
	ldi		XH, HIGH(log_buffer)
	call	i2c_write
	ret


/*
 * Append Byte as 2 Char HEX ASCII bytes to Output Buffer
 *
 * input:	R17 - 8bit Data
 *
 * This function will convert the data into two ASCII HEX character and
 * try to append them to the log_buffer and adjust the log_out_count value.
 * A running limit check is made to prevent overflowing the buffer.
 *
 */
logger_append_byte_text:
	lds		r18, log_out_count
	lds		XL, log_buffer_XL
	lds		XH, log_buffer_XH
// Check for room in the buffer
	cpi		r18, I2C_BUFF_OUT_SIZE-1
	brge	labt_exit					; No room
; add MSB character
	mov		r16, r17
	andi	r16, 0xF0
	swap	r16
	call	con_4bit2hex				; returns r16=HEX ASCII
; append to the buffer
	st		X+, r16
	inc		r18
// Check for room in the buffer
	cpi		r18, I2C_BUFF_OUT_SIZE-1
	brge	labt_exit					; No room
; add LSB character
	mov		r16, r17
	andi	r16, 0x0F
	call	con_4bit2hex				; returns r16=HEX ASCII
; append to the buffer
	st		X+, r16
	inc		r18
;
labt_exit:
	sts		log_out_count, r18			; update
	sts		log_buffer_XL, XL
	sts		log_buffer_XH, XH
	ret


/*
 * Append SRAM Text to Output Buffer
 *
 * input:	Z - pointer to text in SRAM
 *			R17 - Number of bytes to copy
 *
 * This function will append text to the log_buffer and adjust the
 * log_out_count value.
 * A running limit check is made to prevent overflowing the buffer.
 *
 * This function can be used to send non-ASCII values to the logger for 
 * control functions if they are supported.
 *
 */
logger_append_sram_text:
	lds		r16, log_out_count
	lds		XL, log_buffer_XL
	lds		XH, log_buffer_XH
last_loop00:
// Check for room in the buffer
	cpi		r16, I2C_BUFF_OUT_SIZE-1
	brge	last_exit					; No room
; next character
	ld		r18, Z+						; get char
	tst		r18
	breq	last_exit					; NULL..Done
; append to the buffer
	st		X+, r18
	inc		r16
	dec		r17
	brne	last_loop00
;
last_exit:
	sts		log_out_count, r16			; update
	sts		log_buffer_XL, XL
	sts		log_buffer_XH, XH
	ret


/*
 * Append Flash Text to Output Buffer
 *
 * input:	Z - pointer to text in Flash. Text string is NULL terminated.
 *
 * This function will append text to the log_buffer and adjust the
 * log_out_count value.
 * A running limit check is made to prevent overflowing the buffer.
 *
 */
logger_append_flash_text:
// Adjust address to 16bit (FlashAdrs<<1)
	lsl		ZL
	rol		ZH
;
	lds		r16, log_out_count
	lds		XL, log_buffer_XL
	lds		XH, log_buffer_XH
laft_loop00:
// Check for room in the buffer
	cpi		r16, I2C_BUFF_OUT_SIZE-1
	brge	laft_exit					; No room
; next character
	lpm		r17, Z+						; get char
	tst		r17
	breq	laft_exit					; NULL..Done
; append to the buffer
	st		X+, r17
	inc		r16
	rjmp	laft_loop00
;
laft_exit:
	sts		log_out_count, r16			; update
	sts		log_buffer_XL, XL
	sts		log_buffer_XH, XH
	ret


// Preformatted Text .. NULL terminated. Must be MOD 2 in length.
LOG_TEXT_BREAK_POINT:
.db		'B','P',':',0
LOG_TEXT_SPACE:
.db		' ',0
TANK_BOT_ALIVE:
.db		'T','A','N','K',' ','B','O','T',0,0

log_text04:
.db		'L','i','n','e',':','4',' ',' ',0,0
log_text05:
.db		'L','i','n','e',':','0','0','5',0,0

