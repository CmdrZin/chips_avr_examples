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
 * MotorSlave.c
 *
 * Created: 4/15/2019 12:57:57 PM
 * Author : Chip
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "sysTimer.h"
#include "mod_led.h"
#include "twiSlave.h"
#include "mod_dc_motor.h"
#include "pwm_dc_motor.h"
#include "mod_quad_decode.h"
#include "serial.h"							// for development only

#define I2C_ADRS	0x50

#define MAX_MESSAGE_WAIT	10				// N * ms wait time.

#define REG_STATUS		0x00
#define REG_LED			0xF0
#define REG_MOTOR1		0x10
#define REG_MOTOR2		0x20
#define REG_MOTOR3		0x30
#define REG_MOTOR4		0x40

#define MOT_CMD_POS		0x00
#define MOT_CMD_SPD		0x01
#define MOT_CMD_DIR		0x02
#define MOT_CMD_HOME	0x03

#define MASK_M_CMD	0x0F					// Apply to Motor Reg to extract command.

typedef enum {MC_IDLE, MC_LED, MC_POS_HB, MC_POS_LB, MC_DIR, MC_SPEED, MC_HOME} MC_STATE;

void service_serial();
void service_twi();
void statusCheck();
void serialLog();
char bin2hex(char val);

MC_STATE	twiState;						// TWI command Control state
MC_STATE	serState;						// Serial command Control state

uint8_t	dCount;
uint8_t thisMotor = 0;						// Used by BOTH serService() AND twiService()

int main(void)
{
	twiState = MC_IDLE;						// Initialize command state machines
	serState = MC_IDLE;

	st_init_tmr0();
	mod_led_init();							// LED IO
	twiSlaveInit(I2C_ADRS);					// I2C bus
	mdm_init();								// Motor control IO
	mqd_init();
	initUsart();							// Development support. 9600 8N1.
	
	sei();

	statusCheck();
	twiSlaveEnable();
	
    while (1) 
    {
		// Development debug
		serialLog();
		
		mdm_service();						// Update motors at a self-timed fixed rate.

		service_twi();
		service_serial();
    }
}

/*
 * Service incoming SERIAL PORT messages
 * Protocol: MOT:CMD DATA DATA
 * 11 90	- set Motor 1 speed to 0x90
 */

void service_serial()
{
	uint8_t	data;
	
	if( isChar() )
	{
		data = recvChar();					// blocking call, so check for char first.

		switch (serState)
		{
			case MC_IDLE:
				// Process new message. Process REG byte of message.
				switch(data & 0xF0)
				{
					case REG_LED:					// 0xF0
						serState = MC_LED;			// LED control..next byte is state OFF(0):ON(1)
						break;

					case REG_MOTOR1:				// 0x10
					case REG_MOTOR2:				// 0x20
					case REG_MOTOR3:				// 0x30
					case REG_MOTOR4:				// 0x40
						// Motor command. Determine which motor.
						thisMotor = (data & 0xF0) >> 4;
						// Determine motor command.
						switch(data & MASK_M_CMD)
						{
							case MOT_CMD_POS:
								serState = MC_POS_HB;				// expect HB of 16bit position next
								break;

							case MOT_CMD_SPD:
								serState = MC_SPEED;
								break;

							case MOT_CMD_DIR:
								serState = MC_DIR;
								break;

							case MOT_CMD_HOME:
								mdm_motorHome(thisMotor);
								serState = MC_IDLE;
								break;

							default:
								serState = MC_IDLE;					// recover from error
								break;
						}		// 0x0F to get command section.
						break;

					default:
						serState = MC_IDLE;					// recover from error
						break;
				}
				break;

			case MC_LED:
				// Control LED with next byte.
				if(data == 2) {
					mod_led_off();
				} else {
					mod_led_on();
				}
				serState = MC_IDLE;	// reset for next message
				break;

			case MC_POS_HB:
				// Get Position High byte
				mdm_SetPositionHB(thisMotor, data);
				serState = MC_POS_LB;
				break;

			case MC_POS_LB:
				// Get Position Low byte
				mdm_SetPositionLB(thisMotor, data);		// this will update the next Motor Position value.
				serState = MC_IDLE;
				break;

			case MC_DIR:
				// Get Direction byte 0:CW, !0:CCW
				mdm_SetDirection(thisMotor, data);		// does not update until speed command received.
				serState = MC_IDLE;
				break;

			case MC_SPEED:
				// Get Speed byte
				mdm_SetSpeed(thisMotor, data);			// this will update the next Motor Direction and Speed values.
				serState = MC_IDLE;
				break;

			default:
				serState = MC_IDLE;					// recover from error
				break;
		}
	}
}

// Service incoming TWI messages
// Protocol: REG CMD D0 D1 ...
void service_twi()
{
	uint8_t	data;

	if( twiDataInReceiveBuffer() )
	{
		data = twiReceiveByte();
		switch (twiState)
		{
			case MC_IDLE:
				// Process new message. Process REG byte of message.
				switch(data & 0xF0)
				{
					case REG_LED:					// 0x01
						twiState = MC_LED;			// LED control..next byte is state OFF(0):ON(1)
						break;

					case REG_MOTOR1:				// 0x10
					case REG_MOTOR2:				// 0x20
					case REG_MOTOR3:				// 0x30
					case REG_MOTOR4:				// 0x40
						// Motor command. Determine which motor.
						thisMotor = data & 0xF0;
						// Determine motor command. 0x0F to get command section.
						switch(data & MASK_M_CMD)
						{
							case MOT_CMD_POS:
								twiState = MC_POS_HB;				// expect HB of 16bit position next
								break;

							case MOT_CMD_SPD:
								twiState = MC_SPEED;
								break;

							case MOT_CMD_DIR:
								twiState = MC_DIR;
								break;

							case MOT_CMD_HOME:
								mdm_motorHome(thisMotor);
								twiState = MC_IDLE;
								break;

							default:
								twiState = MC_IDLE;					// recover from error
								break;
						}

					default:
						twiState = MC_IDLE;					// recover from error
						break;
				}
				break;

			case MC_LED:
				// Control LED with next byte.
				if(data == 0) {
					mod_led_off();
				} else {
					mod_led_on();
				}
				twiState = MC_IDLE;	// reset for next message
				break;

			case MC_POS_HB:
				// Get Position High byte
				mdm_SetPositionHB(thisMotor, data);
				twiState = MC_POS_LB;
				break;

			case MC_POS_LB:
				// Get Position Low byte
				mdm_SetPositionLB(thisMotor, data);		// this will update the next Motor Position value.
				twiState = MC_IDLE;
				break;

			case MC_DIR:
				// Get Direction byte
				mdm_SetDirection(thisMotor, data);		// does not update until speed command received.
				twiState = MC_IDLE;
				break;

			case MC_SPEED:
				// Get Speed byte
				mdm_SetSpeed(thisMotor, data);			// this will update the next Motor Direction and Speed values.
				twiState = MC_IDLE;
				break;

			default:
				twiState = MC_IDLE;					// recover from error
				break;
		}
	}
}

// Blink status LED three times.
void statusCheck()
{
	long ledTime = st_millis();
	
	for(int i=0; i<3; i++)
	{
		while( ledTime > st_millis());			// Wait here
		ledTime += 500L;
		mod_led_on();
		while( ledTime > st_millis());			// Wait here
		ledTime += 500L;
		mod_led_off();
	}
}

// Development support. Output to serial port.
void serialLog()
{
	int data;
	char buf[16];
	static long serialTime = 0L;

	// Output current position data periodically.
	if( serialTime < st_millis() ) {
		serialTime += 500L;
		
		for(int i=0; i<4; i++) {
			data = mqd_getPosition(i);
			buf[0] = i + '1';
			buf[1] = ':';
			buf[2] = bin2hex((data>>12) & 0x000F);
			buf[3] = bin2hex((data>>8) & 0x000F);
			buf[4] = bin2hex((data>>4) & 0x000F);
			buf[5] = bin2hex(data & 0x000F);
			buf[6] = ' ';
			buf[7] = ' ';
			
			for( int x=0; x<8; x++ ) {
				sendChar(buf[x]);				// Blocking call
			}
		}
		// Send line termination
		sendChar('\n');
	}
}

char bin2hex(char val)
{
	char result;

	if(val > 9)
	{
		result = (val - 10) + 'A';
	}
	else
	{
		result = val + '0';
	}

	return result;
}
