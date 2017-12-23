/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nels D. "Chip" Pearson (aka CmdrZin)
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
 * LCD_16100_CP_DevCode.c
 *
 * Created: 7/18/2017 7:16:42 PM
 * Author : Chip
 *
 * Target: ATmega328P, 8MHz
 *
 * This is a main.c file for the LCD 1x16 LCD_16100 module I2C Slave
 * It provides a 1x16 LCD display, three LEDs (RGY), and three buttons.
 * There are also general digital I/O pins, an analog input, two PWM outputs,
 * a FTDI/USB/Bluetooth port, and a 256k bit EEPROM available.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>

#include "sysTimer.h"
#include "twiSlave.h"
#include "core_functions.h"
#include "io_led_button.h"
#include "dev_display.h"
#include "adc_support.h"

#define SLAVE_ADRS		0x5E

typedef enum {M_CMD, M_CTRL, M_BUTTONS, M_BUTTONS_RAW, M_ADC, M_LCD_TEXT, M_LCD_PTEXT, 
	M_LCD_PTEXT_O, M_LCD_TEXT_G, M_G_LED, M_R_LED, M_Y_LED} M_SERVICE_TYPE;

/*
 * main()
 *
 * Each service manages their own time slice.
 *
 */
int main(void)
{
	uint8_t	data;
	M_SERVICE_TYPE	service = M_CMD;
	uint8_t msgLength;
	uint8_t msgOffset;
	uint8_t msgCharCnt;
	char msgBuff[16];
	
	uint8_t cmdRevCnt = 0;
	uint8_t dataRevCnt = 0;
	char tText[18];
	int tCount;
	
	st_init_tmr0();
	mod_io_init();
	adc_support_init();
	twiSlaveInit(SLAVE_ADRS);
	dev_display_init(SLAVE_ADRS);
	
	sei();							// Enable global interrupts.

	twiSlaveEnable();
		
	while(1)
	{
		adc_support_service();		// sample channel each 10 ms.
		mod_io_service();
		dev_display_service();		// Update if text changed.
		
		// Test for any incoming I2C data.
		if( twiDataInReceiveBuffer() )
		{
			// NOTE: Master should wait at least 1ms after requesting data before reading it. No clock stretching.
			data = twiReceiveByte();		// get the new data.
			
			switch(service)
			{
				// Waiting for a Command byte.
				case M_CMD:
					++cmdRevCnt;
					
					switch(data)
					{
						case 0x00:
							service = M_CTRL;
							break;
				
						// Read Buttons		
						case 0x01:
//							twiClearOutput();
							twiTransmitByte(mod_io_getButtons());
							break;
							
						// Read Buttons Raw
						case 0x02:
							service = M_BUTTONS_RAW;
							break;
							
						case 0x03:
							service = M_ADC;
							break;
							
						case 0x10:
							service = M_LCD_TEXT;
							break;
							
						case 0x11:
							service = M_LCD_PTEXT;
							break;
								
						case 0x20:
							service = M_G_LED;
							break;

						case 0x21:
							service = M_R_LED;
							break;

						case 0x22:
							service = M_Y_LED;
							break;

						case 0xF8:
							core_get_build_date();
							break;
				
						case 0xF9:
							core_get_build_time();
							break;
				
						case 0xFA:
							core_get_version();
							break;
				
						default:
							break;
					}
					break;

				case M_CTRL:
					++dataRevCnt;
					
					twiClearOutput();
					twiTransmitByte(0);			// Return Control register on next read.
					service = M_CMD;
					break;
					
				// SDA_W 0x10 LEN D[0] ... D[N]
				case M_LCD_TEXT:
					++dataRevCnt;

					msgLength = data;
					msgOffset = 0;
					msgCharCnt = 0;
					service = M_LCD_TEXT_G;
					break;
					
				// SDA_W 0x11 LEN POS D[0] ... D[N]
				case M_LCD_PTEXT:
					++dataRevCnt;

					msgLength = data;
					service = M_LCD_PTEXT_O;
					break;
					
				case M_LCD_PTEXT_O:
					++dataRevCnt;

					msgOffset = data;
					msgCharCnt = 0;
					service = M_LCD_TEXT_G;
					break;

				case M_LCD_TEXT_G:
					++dataRevCnt;

					msgBuff[msgCharCnt] = data;
					if(++msgCharCnt >= msgLength) {
						setTextOff(msgBuff, msgLength, msgOffset);
						service = M_CMD;
					}
					break;

				case M_G_LED:
					++dataRevCnt;

					mod_io_setGreenLed(data);
					service = M_CMD;
					break;

				case M_R_LED:
					++dataRevCnt;

					mod_io_setRedLed(data);
					service = M_CMD;
					break;

				case M_Y_LED:
					++dataRevCnt;

					mod_io_setYellowLed(data);
					service = M_CMD;
					break;

				default:
					service = M_CMD;
					break;
			}

			if(cmdRevCnt > 99) cmdRevCnt = 0;
			if(dataRevCnt > 99) dataRevCnt = 0;
			// Be careful of overflowing tText buffer. Limit 16+1 char.
			tCount = snprintf(tText,17,"CMD:%02d  DATA:%02d ", cmdRevCnt, dataRevCnt);
//			tCount = snprintf(tText,17,"CMD:%02d  DATA:%02X ", cmdRevCnt, data);
//			setText(tText, tCount);
		}
	}
}
