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
 *	NOTE: PORTA also supports IR power control.
 */ 


.DSEG
adc_H:		.BYTE	1			; store 10 bits as 7:0..unsigned 10 bits
adc_L:		.BYTE	1			; store 10 bits as 7:6

.CSEG
/*
 * Initialize Hardware for ADC use
 * input reg:	none
 * output reg:	none
 *
 * 20MHz CPU clock.
 * Single Ended
 * Available channels 0,1,2,3
 * ATmega164P has Internal 2.56v reference.
 *
 * resources:
 */
adc_init_hdwr:
	ldi		R16, (1<<REFS1)|(1<<REFS0)|(1<<ADLAR)	; Internal Vref=2.56v, Left Adj
	sts		ADMUX, R16								; lower 3 bits select channel.
;
	ldi		R16, (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)	; ADC Enable, CPU/128 clock (20MHz)
	sts		ADCSRA, R16
;
	ret

/*
 * Get ADC Data
 *
 * input:	none
 *
 * output:	R17 - b7:0 MSBs
 *			R18 - b7:6 LSBs
 *
 */
adc_get_data:
	lds		R17, adc_H
	lds		R18, adc_L
;
agd_exit:
	ret


/* *** ADC Support *** */

/*
 * Trigger ADC channel
 *
 * input:	R17 - ADC channel 0-3
 *
 * output:	adc_HL		Range data
 *
 * NOTE: MUST read ADCL first then ADCH.
 */
adc_trigger:
	lds		r16, ADMUX					; get flags
	andi	r16, 0xF8					; zero channel select
	or		r16, r17					; set channel
	sts		ADMUX, R16					; lower 3 bits select channel.
; Trigger
	lds		r16, ADCSRA
	ori		r16, (1<<ADSC)
	sts		ADCSRA, r16
; Wait ~125us
at_loop00:
	lds		r16, ADCSRA
	andi	r16, (1<<ADSC)
	brne	at_loop00
; Get Data
	lds		r18, ADCL			; get 2 LSBs .. d7:6
	lds		r17, ADCH			; get 8 MSBs .. d7:0..releases ADC data reg.
	sts		adc_L, r18
	sts		adc_H, r17
;
	ret
