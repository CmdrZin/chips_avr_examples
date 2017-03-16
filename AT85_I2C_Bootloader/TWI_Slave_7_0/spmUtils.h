/*
 * spmUtils.h
 *
 * Created: 1/24/2017 1:07:13 PM
 *  Author: Chip
 */ 


#ifndef SPMUTILS_H_
#define SPMUTILS_H_

void spm_erase( uint16_t adrs );
void spm_filltemp(uint16_t adrs, uint16_t data);
void spm_pagewrite(uint16_t pageAdrs);
void do_spm(uint8_t data);


#endif /* SPMUTILS_H_ */