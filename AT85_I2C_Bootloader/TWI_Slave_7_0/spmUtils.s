
/*
 * spmUtils.s
 *
 * Created: 1/24/2017		0.01	ndp
 *  Author: Chip
 */ 

 #include <avr/io.h>

.global spm_erase
/*
* 19.1 Performing Page Erase by SPM
* To execute Page Erase, set up the address in the Z-pointer, write “00000011” to SPMCSR and execute SPM
* within four clock cycles after writing SPMCSR. The data in R1 and R0 is ignored. The page address must be written
* to PCPAGE in the Z-register. Other bits in the Z-pointer will be ignored during this operation
*/
spm_erase:
	MOV		r31, r25		; ZH
	MOV		r30, r24		; ZL
	LDI		R24, 0x03		; (0b00000011)
	RCALL	do_spm
	ret

.global spm_filltemp
/*
* 19.2 Filling the Temporary Buffer (Page Loading)
* To write an instruction word, set up the address in the Z-pointer and data in R1:R0, write “00000001” to SPMCSR
* and execute SPM within four clock cycles after writing SPMCSR. The content of PCWORD in the Z-register is used
* to address the data in the temporary buffer. The temporary buffer will auto-erase after a Page Write operation or by
* writing the CTPB bit in SPMCSR. It is also erased after a system reset. Note that it is not possible to write more
* than one time to each address without erasing the temporary buffer.
*/
spm_filltemp:
	push	r0					; save R0. R1 always sets to 0.

	MOV		r31, r25			; addrH ZH
	MOV		r30, r24			; addrL ZL
	MOV		r1, r23				; dataH R1
	MOV		r0, r22				; dataL R0
	LDI		R24, 0x01			; (0b00000001)
	RCALL	do_spm

	CLR r1
	pop r0
	ret

.global spm_pagewrite
/*
 * 19.3 Performing a Page Write
 * To execute Page Write, set up the address in the Z-pointer, write “00000101” to SPMCSR and execute SPM within
 * four clock cycles after writing SPMCSR. The data in R1 and R0 is ignored. The page address must be written to
 * PCPAGE. Other bits in the Z-pointer must be written to zero during this operation
 * void spm_pagewrite(uint16_t pageAdrs)
 */
spm_pagewrite:
	MOV		r31, r25			; pageAdrsH ZH
	MOV		r30, r24			; pageArdsL	ZL
	LDI		R24, 0x05			; (0b00000101)
	RCALL	do_spm
	ret

.global do_spm
/* ref: AVR_8bit_Instruction_Set pg 141
 * void do_spm(uint8_t data)
 */
do_spm:
	; Save SREG
	lds		r18, SREG
	CLI

;	while(SPMCSR & (1<<SPMEN));		// wait
;check for previous SPM complete
wait:
	LDS		r19, SPMCSR
	sbrc	r19, SPMEN
	rjmp	wait

	STS		SPMCSR, r24			; data
	SPM

	STS		SREG, r18
	ret
