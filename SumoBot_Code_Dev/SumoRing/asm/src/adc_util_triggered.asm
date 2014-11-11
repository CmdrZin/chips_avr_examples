/*
 * ADC Utilities
 *
 * org: 10/21/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: LCD_CDM-116100 Demo Board, 8MHz, ATmega328P
 *
 * Usage:
 * 	.include adc_util_triggered.asm
 *
 * These functions are for triggered ADC sampling.
 *
 * Resources;
 * 		adc_buff	4 bytes
 *
 *	NOTE: PORTC shared supports I2C.
 */ 

; Move to ADC module
.equ	ADC_LEFT_CHAN	= 0
.equ	ADC_RIGHT_CHAN	= 1


.DSEG
adc_leftH:	.BYTE	1			; store 10 bits as 7:0..unsigned 10 bits
adc_leftL:	.BYTE	1			; store 10 bits as 7:6
adc_rightH:	.BYTE	1			; store 10 bits as 7:0..unsigned 10 bits
adc_rightL:	.BYTE	1			; store 10 bits as 7:6

.CSEG
/*
 * Initialize Hardware for ADC use
 * input reg:	none
 * output reg:	none
 *
 * 8MHz CPU clock.
 * Single Ended
 * Available channels 0,1,2,3
 * ATmega328P has Internal 1.1v reference.
 *
 * resources:
 */
adc_init_hdwr:
	ldi		R16, (1<<REFS1)|(1<<REFS0)|(1<<ADLAR)	; Internal Vref=1.1v, Left Adj
	sts		ADMUX, R16
;
	ldi		R16, (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)	; ADC Enable, CPU/64 clock (8MHz)
	sts		ADCSRA, R16
;
	ret

/*
 * Get ADC Data
 *
 * input:	R17 - ADC channel. ADC_LEFT_CHAN, ADC_RIGHT_CHAN
 *
 * output reg:	R17 - b7:0 MSBs
 *				R18 - b7:6 LSBs
 *
 */
adc_get_data:
	cpi		R17, ADC_LEFT_CHAN
	brne	agd_skip10
; Get Left
	lds		R18, adc_leftL
	lds		R17, adc_leftH
	rjmp	agd_exit
;
agd_skip10:
; Get Right
	lds		R18, adc_rightL
	lds		R17, adc_rightH
;
agd_exit:
	ret


/* *** ADC Support *** */

/*
 * Trigger ADC channel
 *
 * input:	R17 - ADC channel. ADC_LEFT_CHAN, ADC_RIGHT_CHAN
 *
 * output:	adc_left		Range data
 *			adc_right		Range data
 *
 * NOTE: MUST read ADCL first then ADCH.
 */
adc_trigger:
	cpi		R17, ADC_LEFT_CHAN
	brne	at_skip10
; Do Left side
	lds		R16, ADMUX
	andi	R16, 0xF0
	ori		R16, ADC_LEFT_CHAN
	sts		ADMUX, R16
; Trigger
	lds		R16, ADCSRA
	ori		R16, (1<<ADSC)
	sts		ADCSRA, R16
; Wait ~125us
at_loop00:
	lds		R16, ADCSRA
	andi	R16, (1<<ADSC)
	brne	at_loop00
; Get Data
	lds		R18, ADCL			; get 2 LSBs .. d7:6
	lds		R17, ADCH			; get 8 MSBs .. d7:0..releases ADC data reg.
	sts		adc_leftL, R18
	sts		adc_leftH, R17
	rjmp	at_exit				; EXIT
;
at_skip10:
; Do Right side
	lds		R16, ADMUX
	andi	R16, 0xF0
	ori		R16, ADC_RIGHT_CHAN
	sts		ADMUX, R16
; Trigger
	lds		R16, ADCSRA
	ori		R16, (1<<ADSC)
	sts		ADCSRA, R16
; Wait ~125us
at_loop10:
	lds		R16, ADCSRA
	andi	R16, (1<<ADSC)
	brne	at_loop10
; Get Data
	lds		R18, ADCL			; get 2 LSBs .. d7:6
	lds		R17, ADCH			; get 8 MSBs .. d7:0..releases ADC data reg.
	sts		adc_rightL, R18
	sts		adc_rightH, R17
;
at_exit:
	ret
