/* 
 * File:   systime.h
 * Author: Cmdrzin
 *
 * Created on August 8, 2024, 12:54 PM
 */

#ifndef SYSTIME_H
#define	SYSTIME_H

#include <avr/interrupt.h>          // to support the use of interrupts

uint32_t millis(void);              // prototype for millis() function
void init_systime(void);

#endif	/* SYSTIME_H */

