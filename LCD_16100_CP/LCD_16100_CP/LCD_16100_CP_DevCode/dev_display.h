/*
 * dev_display.h
 *
 * Created: 6/3/2015 11:16:18 PM
 *  Author: Chip
 */ 


#ifndef DEV_DISPLAY_H_
#define DEV_DISPLAY_H_


// Display 4
#define DEV_DISPLAY_ID 4

void dev_display_init(uint8_t adrs);
void dev_display_service(void);

void setText(char *text, uint8_t len);
void setTextOff(char *text, uint8_t len, uint8_t offset);

void dev_display_set_text(void);

void dev_display_line_text(uint8_t line, char buff[]);
void dev_display_insert4_text(void);
void dev_display_insert8_text(void);


#endif /* DEV_DISPLAY_H_ */