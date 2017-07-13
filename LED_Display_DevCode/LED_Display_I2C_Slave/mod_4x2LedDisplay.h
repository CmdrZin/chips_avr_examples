/*
 * mod_4x2LedDisplay.h
 *
 * Created: 6/29/2017 5:09:16 PM
 *  Author: Chip
 */ 


#ifndef MOD_4X2LEDDISPLAY_H_
#define MOD_4X2LEDDISPLAY_H_

typedef enum ledStates {MLD_TEST, MLD_NORMAL} MLD_STATE;

void mod_LD_init();
void mod_LD_service();
void mod_LD_setMode(MLD_STATE state);
void mod_LD_setDigit(uint8_t digit, uint8_t val);
void mod_LD_setDP(uint8_t digit, uint8_t val);

#endif /* MOD_4X2LEDDISPLAY_H_ */