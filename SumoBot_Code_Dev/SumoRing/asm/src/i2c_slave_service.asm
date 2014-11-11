/*
 * I2C Slave Service
 *
 * org: 10/27/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Sumo Bot Demo Board, 8MHz w/65ms reset delay,
 *
 *
 */



/*
 * i2c_slave_service()
 *
 * Service I2C input messages.
 *
 * input reg:	none
 *
 * output reg:	none
 *
 * resources:	i2c_status
 *				i2c_buffer_in
 *				i2c_buffer_in_cnt
 *
 * Copy any input to the LCD display.
 *
 */
i2c_slave_service:
	lds		R16, i2c_status
	cpi		R16, I2C_STATUS_NEW
	breq	iss_skip00
	ret								; EXIT
;
iss_skip00:
	ldi		XL, LOW(i2c_buffer_in)
	ldi		XH, HIGH(i2c_buffer_in)
	ldi		R17, 6						; Line 2 Value
	lds		R18, i2c_buffer_in_cnt
	call	out_ram_text_n
;
	ret
