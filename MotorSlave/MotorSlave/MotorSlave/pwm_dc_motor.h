/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
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

typedef enum { PWM_CW, PWM_CCW } PWM_DIR_CONTROL;

void pwm_dc_motor_init();
void pwm_set_dir(uint8_t motor, PWM_DIR_CONTROL dir );
void pwm_stop(uint8_t motor);
void pwm_set_speed(uint8_t motor, uint8_t val );


#endif /* PWM_DC_MOTOR_H_ */