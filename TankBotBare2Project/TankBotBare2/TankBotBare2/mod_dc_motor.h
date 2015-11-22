/*
 * mod_dc_motor.h
 *
 * Created: 11/6/2015 11:18:21 AM
 *  Author: Chip
 */ 


#ifndef MOD_DC_MOTOR_H_
#define MOD_DC_MOTOR_H_

#define MOD_DC_MOTOR_ID	0x30

typedef enum { MDM_NONE, MDM_MOTOR_LEFT, MDM_MOTOR_RIGHT, MDM_MOTOR_BOTH } MDM_MOTOR_SELECT;

void mdm_init();
void mdm_service();
void mdm_SetSpeed();
void mdm_SetDirection();
void mdm_ControlUpdate();

#endif /* MOD_DC_MOTOR_H_ */