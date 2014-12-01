/*
 * I2C Slave Interface
 *
 * org: 6/22/2014
 * rev: 11/17/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include logger.asm
 *
 */ 

.DSEG
logger_line_num:		.BYTE	1	; line number 1-5

.CSEG
/*
 * logger_init
 *
 * Initialize logger parameters.
 *
 * input:	none
 * output:	none
 */
logger_init:
	ldi		r16, 1
	sts		logger_line_num, r16
;
	call	i2c_slave_init				; start Slave receive.
;
	ret

/*
 * logger_check_for_message
 *
 * Called from main() continually to check for in coming messages.
 *
 * input:	none
 * output:	none
 *
 * Process
 * If i2c_status = I2C_STATUS_MSG_RCVD
 *   If i2c_buffer_in_cnt != 0
 *     Set X = i2c_buffer_in
 *     Set r17 = line 1-5
 *     Set r18 = i2c_buffer_in_cnt
 *     Call lcd_set_text_sram()
 *   Set i2c_status = I2C_STATUS_IDLE
 *
 */
logger_check_for_message:
	lds		r16, i2c_status
	cpi		r16, I2C_STATUS_MSG_RCVD
	breq	lcfm_skip00
	rjmp	lcfm_exit					; EXIT
;
lcfm_skip00:
;
	call	turn_off_green
;
	lds		r18, i2c_buffer_in_cnt
	tst		r18
	breq	lcfm_skip01
; Set parameter to LCD function
	ldi		YL, LOW(i2c_buffer_in)
	ldi		YH, HIGH(i2c_buffer_in)
	lds		r17, logger_line_num
; r18 has byte count
	push	r17
	call	lcd_set_sram_text_line_n
	pop		r17
;
; update line number
	inc		r17
	cpi		r17, 6
	brlt	lcfm_skip02
	ldi		r17, 1
lcfm_skip02:
	sts		logger_line_num, r17
;
lcfm_skip01:
	call	i2c_slave_init				; restart Slave receive.
;
lcfm_exit:
	ret
