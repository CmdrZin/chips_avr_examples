/*
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