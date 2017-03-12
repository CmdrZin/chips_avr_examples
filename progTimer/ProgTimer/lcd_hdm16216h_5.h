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
 * lcd_hdm16216h_5.h
 *
 * Created: 6/4/2015 2:40:12 PM
 *  Author: Chip
 */ 


#ifndef LCD_HDM16216H_5_H_
#define LCD_HDM16216H_5_H_

void lcd_init(void);
void lcd_set_ddram( uint8_t adrs );
void lcd_delay_40us(void);
void lcd_delay_1ms(uint8_t msDelay);
void lcd_write_ins_chk(uint8_t data);
void lcd_write_ins_nochk(uint8_t data);
void lcd_write_ins_once(uint8_t data);
void lcd_busy_check(void);
uint8_t lcd_cmd_read(void);
void lcd_cmd_write(uint8_t data);
uint8_t lcd_ram_read(void);
void lcd_ram_write(uint8_t data);


#endif /* LCD_HDM16216H_5_H_ */