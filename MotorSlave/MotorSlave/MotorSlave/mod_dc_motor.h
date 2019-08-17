/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Nels D. "Chip" Pearson (aka CmdrZin)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
 * mod_dc_motor.h
 *
 * Created: 11/6/2015 11:18:21 AM
 *  Author: Chip
 * revised: 04/16/2019		0.10	ndp	for use by Motor Slave.
 */ 


#ifndef MOD_DC_MOTOR_H_
#define MOD_DC_MOTOR_H_

void mdm_init();
void mdm_service();
void mdm_SetSpeed(uint8_t motor, uint8_t speed);
void mdm_SetDirection(uint8_t motor, uint8_t direction);
void mdm_SetPositionHB(uint8_t motor, uint8_t data);
void mdm_SetPositionLB(uint8_t motor, uint8_t data);
float mdm_move(uint8_t motorIndex, float endPosition, float position);
void mdm_motorHome(uint8_t motor);

#endif /* MOD_DC_MOTOR_H_ */