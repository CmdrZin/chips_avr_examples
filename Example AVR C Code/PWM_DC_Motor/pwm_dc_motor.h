/*
 * pwm_dc_motor.h
 *
 * Created: 10/29/2015 1:51:47 PM
 *  Author: Chip
 */ 


#ifndef PWM_DC_MOTOR_H_
#define PWM_DC_MOTOR_H_

// Uses 16bit values
#define	PWM_R_STOP		0
#define	PWM_R_SLOW		PWM_R_STOP+3000
#define	PWM_R_MED		PWM_R_STOP+10000
#define	PWM_R_FAST		PWM_R_STOP+19000

#define	PWM_L_STOP		0
#define	PWM_L_SLOW		PWM_L_STOP+3000
#define	PWM_L_MED		PWM_L_STOP+10000
#define	PWM_L_FAST		PWM_L_STOP+19000

typedef enum { PWM_STOP, PWM_FWD, PWM_REV } PWM_DIR_CONTROL;

void pwm_dc_motor_init();

void pwm_set_right_dir( PWM_DIR_CONTROL dir );
void pwm_stop_right();
void pwm_add16_right( uint16_t val );
void pwm_sub16_right( uint16_t val );
void pwm_set_right( uint16_t val );

void pwm_set_left_dir( PWM_DIR_CONTROL dir );
void pwm_stop_left();
void pwm_add16_left(uint16_t val );
void pwm_sub16_left( uint16_t val );
void pwm_set_left( uint16_t val );


#endif /* PWM_DC_MOTOR_H_ */