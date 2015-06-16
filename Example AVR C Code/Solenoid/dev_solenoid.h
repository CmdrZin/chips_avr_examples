/*
 * dev_solenoid.h
 *
 * Created: 6/12/2015 10:30:19 PM
 *  Author: Chip
 */ 


#ifndef DEV_SOLENOID_H_
#define DEV_SOLENOID_H_

#define DEV_SOLENOID_1_ID	0x11
#define DEV_SOLENOID_2_ID	0x12


void dev_solenoid_init();
void dev_solenoid_service();

void dev_solenoid_1_pulse();
void dev_solenoid_1_toggle_swap();
void dev_solenoid_1_set_pulse();

void dev_solenoid_2_pulse();
void dev_solenoid_2_toggle_swap();
void dev_solenoid_2_set_pulse();


#endif /* DEV_SOLENOID_H_ */