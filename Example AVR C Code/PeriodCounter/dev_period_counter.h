/*
 * dev_period_counter.h
 *
 * Created: 6/22/2015 4:40:27 PM
 *  Author: Chip
 */ 


#ifndef DEV_PERIOD_COUNTER_H_
#define DEV_PERIOD_COUNTER_H_


#define DEV_PERIOD_COUNTER_ID	0x10

void dev_period_counter_init();
void dev_period_counter_service();

void dev_period_counter_get();


#endif /* DEV_PERIOD_COUNTER_H_ */