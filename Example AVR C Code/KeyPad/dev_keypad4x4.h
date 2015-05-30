/*
 * dev_keypad4x4.h
 *
 * Created: 5/22/2015 3:45:04 PM
 *  Author: Chip
 */ 


#ifndef DEV_KEYPAD4X4_H_
#define DEV_KEYPAD4X4_H_


// Key Pad 4x4
#define DEV_KEYPAD_ID 2

void dev_keypad4x4_init(void);
void dev_keypad4x4_service(void);
void dev_keypad4x4_get_data(void);


void dev_keypad4x4_set_scan_1(void);
void dev_keypad4x4_set_scan_2(void);
void dev_keypad4x4_set_scan_3(void);
void dev_keypad4x4_set_scan_4(void);
void dev_keypad4x4_set_scan_5(void);
void dev_keypad4x4_set_scan_6(void);
void dev_keypad4x4_set_scan_7(void);
void dev_keypad4x4_set_scan_8(void);
void dev_keypad4x4_set_scan_9(void);
void dev_keypad4x4_set_scan_10(void);
void dev_keypad4x4_set_scan_11(void);
void dev_keypad4x4_set_scan_12(void);
void dev_keypad4x4_set_scan_13(void);
void dev_keypad4x4_set_scan_14(void);
void dev_keypad4x4_set_scan_15(void);
void dev_keypad4x4_set_scan_num( uint8_t scans );

void dev_keypad4x4_set_delay_1(void);
void dev_keypad4x4_set_delay_2(void);
void dev_keypad4x4_set_delay_3(void);
void dev_keypad4x4_set_delay_4(void);
void dev_keypad4x4_set_delay_5(void);
void dev_keypad4x4_set_delay_6(void);
void dev_keypad4x4_set_delay_7(void);
void dev_keypad4x4_set_delay_8(void);
void dev_keypad4x4_set_delay_9(void);
void dev_keypad4x4_set_delay_10(void);
void dev_keypad4x4_set_delay_11(void);
void dev_keypad4x4_set_delay_12(void);
void dev_keypad4x4_set_delay_13(void);
void dev_keypad4x4_set_delay_14(void);
void dev_keypad4x4_set_delay_15(void);
void dev_keypad4x4_set_scan_delay(uint8_t delay );


#endif /* DEV_KEYPAD4X4_H_ */