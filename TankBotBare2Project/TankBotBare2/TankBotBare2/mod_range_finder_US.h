/*
 * mod_range_finder_US.h
 *
 * Created: 8/16/2015		0.01	ndp
 *  Author: Chip
 */ 


#ifndef MOD_RANGE_FINDER_US_H_
#define MOD_RANGE_FINDER_US_H_

#define MOD_RANGE_FINDER_US_ID	0x20

#define MRFUS_GET_RANGE			0x01
#define MRFUS_GET_ALL_RANGE		0x02
#define MRFUS_SET_MINIMUM_RANGE	0x03


void mod_range_finder_us_init();
void mod_range_finder_us_service();

void mrfus_trigger_wait();

void mod_range_finder_us_getRange();
void mod_range_finder_us_getAllRange();
void mod_range_finder_us_setMinimumRange();



#endif /* MOD_RANGE_FINDER_US_H_ */