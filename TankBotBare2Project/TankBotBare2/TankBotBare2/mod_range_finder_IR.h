/*
 * mod_range_finder_IR.h
 *
 * Created: 10/20/2015 11:19:22 AM
 *  Author: Chip
 */ 


#ifndef MOD_RANGE_FINDER_IR_H_
#define MOD_RANGE_FINDER_IR_H_

#define MOD_RANGE_FINDER_IR_ID 0x24

// Set to match ADC channel assignments
#define MRFIR_LEFT_F	0
#define MRFIR_LEFT_R	1
#define MRFIR_RIGHT_R	2
#define MRFIR_RIGHT_F	3

void mrfir_init();
void mrfir_service();

void mrfir_SendIR();
void mrfir_SendAllIR();
void mrfir_SetLimit();

#endif /* MOD_RANGE_FINDER_IR_H_ */