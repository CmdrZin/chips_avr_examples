/* 
 * File:   io_ctrl.h
 * Author: Cmdrzin
 *
 * Created on August 8, 2024, 12:55 PM
 */

#ifndef IO_CTRL_H
#define	IO_CTRL_H

#include <stdbool.h>

typedef enum { LED_RED, LED_YELLOW, LED_GREEN } LED;

void init_io(void);    
void toggle_LED();
void setLED(bool state);
void setAN(bool stateAN1, bool stateAN2);
void pulseLED(uint8_t val);

#endif	/* IO_CTRL_H */

