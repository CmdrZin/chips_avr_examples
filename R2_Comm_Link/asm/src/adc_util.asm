/*
 * ADC Utilities
 *
 * org: 7/22/2014
 * auth: Nels "Chip" Pearson
 *
 * Usage:
 * 	.include adc_util.asm
 *
 * General:
 * 	R00 - R15 are protected registers. Restore if used.
 * 	R16 - R16
 *  R17 - R31 are transfer and local registers. NOT preserved.
 *
 * Resources;
 * 		adc_buff	2 bytes
 *
 *	NOTE: PORTC shared supports I2C.
 */ 

.DSEG
adc_buffH:	.BYTE	1			; store 10 bits as ->7 & 3<- bits..Flip b9 for SIGN
adc_buffL:	.BYTE	1			; store 10 bits as ->7 & 3<- bits..Flip b9 for SIGN
adc_status:	.BYTE	1			; flags for ADC data state
								; 0:new data
								; (TODO) non-0:error or old data

.CSEG
/*
 * Initialize Hardware for ADC use
 * input reg:	none
 * output reg:	none
 *
 * 8MHz CPU clock.
 * ADC0, Single Ended, Internal 2.56V
 *
 * resources:
 */
adc_init_hdwr:
;	ldi		R16, 0b11100000		; ADC0, Left Adj, Single Ended ADC0, Internal 2.56V
	ldi		R16, 0b11000000		; ADC0, Right Adj, Single Ended ADC0, Internal 2.56V
	sts		ADMUX, R16
;
;	ldi		R16, 0b11101011		; ADC Enable, ADC Start Conv, AUTO trigger, ADC Intr enable, CPU/8 clock (1MHz)
;	ldi		R16, 0b11101111		; ADC Enable, ADC Start Conv, AUTO trigger, ADC Intr enable, CPU/128 clock (20MHz)
	ldi		R16, 0b10101111		; ADC Enable, AUTO trigger, ADC Intr enable, CPU/128 clock (20MHz)
	sts		ADCSRA, R16
;
;	ldi		R16, 0b00000011		; Auto trigger on TIM0 Compare match (used for 1ms tic)
	ldi		R16, 0b00000101		; Auto trigger on TIM1B Compare match
	sts		ADCSRB, R16
;
	ser		R16					; disable digital input buffers.
	sts		DIDR0, R16
	sts		adc_status, R16		; set state as bad data.
;
	ret

/*
 * Get ADC Data
 * input reg:	none (Could be channel though.)
 * output reg:	R17 (Could also return all 10bits or more if oversampled.)
 *				R18 - 0:new data..!0:old or bad data.
 */
adc_get_data:
	lds		R18, adc_status
	lds		R17, adc_buffH
; set to old data if was good new data.
	tst		R18
	brne	agd_skip00
	ori		R18, 0x01
	sts		adc_status, R18		; set as old data.
	clr		R18					; return status as NEW data.
;
agd_skip00:
	ret

/*
 * ADC Interrupt Service
 * input reg:	none (Could be channel though.)
 * output reg:	R17 (Could also return all 10bits or more if oversampled.)
 *
 * Read data and reset intr flag.
 * NOTE: Data is Right Adjusted, so ADCH.1:0 have the 2 MSbs and ADCL has the LSB.
 * NOTE: MUST read ADCL first then ADCH.
 */
adc_intr:
; Save SREG
	push	R0
	in		R0, SREG
	push	R0
;
	push	R17
	push	R18
;
	lds		R18, ADCL			; get 8 LSBs .. d7:0
	lds		R17, ADCH			; get 2 MSBs .. d1:0..releases ADC data reg.
; just subtract 0x02
	subi	R17, 0x02
;
ai_skip00:
	sts		adc_buffL, R18
	sts		adc_buffH, R17
;
	clr		R17
	sts		adc_status, R17		; set state as new data.
;
; ADIF bit cleared by hardware.
	pop		R18
	pop		R17
; Restore SREG
	pop		R0
	out		SREG, R0
	pop		R0
;
	reti
