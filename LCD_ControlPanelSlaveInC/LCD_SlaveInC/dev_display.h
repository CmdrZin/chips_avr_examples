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

void dev_display_init(void);
void dev_display_service(void);
void dev_display_set_text(void);

void dev_display_line_text(uint8_t line, char buff[]);
void dev_display_insert4_text(void);
void dev_display_insert8_text(void);


#endif /* DEV_DISPLAY_H_ */