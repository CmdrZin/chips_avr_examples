/*
 * lcd_hd44780.h
 *
 * Created: 3/27/2025
 * Author: Chip
 */ 

#ifndef LCD_HD44780_H_
#define LCD_HD44780_H_


void lcd_init(void);

void lcd_print(uint8_t line, uint8_t pos, char* str);
void lcd_clearScreen();

void lcd_setDDadrs( uint8_t line, uint8_t pos );
void lcd_setCGadrs( uint8_t adrs );
void toggleE();
void lcd_delay_40us(void);
void lcd_delay_1ms(uint8_t msDelay);
void lcd_busy_check(void);
void lcd_writeCommand(uint8_t cmd);
void lcd_writeCommand8NB(uint8_t cmd);
uint8_t lcd_readRam();
void lcd_writeRam(uint8_t data);



#endif /* LCD_HD44780_H_ */