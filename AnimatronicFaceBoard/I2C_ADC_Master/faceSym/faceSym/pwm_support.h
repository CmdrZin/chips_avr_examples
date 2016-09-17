/*
 * pwm_support.h
 *
 * Created: 7/26/2016 2:27:33 PM
 *  Author: Chip
 */ 


#ifndef PWM_SUPPORT_H_
#define PWM_SUPPORT_H_

#include <avr/io.h>

#define PWM_L	0
#define PWM_R	1

#define PWM_L_STOP		471
#define PWM_L_FAST_REV	PWM_L_STOP-51
#define PWM_L_MED_REV	PWM_L_STOP-25
#define PWM_L_SLOW_REV	PWM_L_STOP-10
#define PWM_L_SLOW_FWD	PWM_L_STOP+14
#define PWM_L_MED_FWD	PWM_L_STOP+25
#define PWM_L_FAST_FWD	PWM_L_STOP+51

#define PWM_R_STOP		505
#define PWM_R_FAST_REV	PWM_R_STOP+44
#define PWM_R_MED_REV	PWM_R_STOP+32
#define PWM_R_SLOW_REV	PWM_R_STOP+20
#define PWM_R_SLOW_FWD	PWM_R_STOP-42
#define PWM_R_MED_FWD	PWM_R_STOP-45
#define PWM_R_FAST_FWD	PWM_R_STOP-65


void pwm_support_init();
void pwm_setMotor( uint8_t motor, uint16_t speed );
uint16_t pwm_getMotor( uint8_t motor );

#endif /* PWM_SUPPORT_H_ */