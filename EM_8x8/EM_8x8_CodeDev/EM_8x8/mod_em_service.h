/*
 * mod_em_service.h
 *
 * Created: 1/13/2016 1:03:51 PM
 *  Author: Chip
 */ 


#ifndef MOD_EM_SERVICE_H_
#define MOD_EM_SERVICE_H_

#define MOD_EM_SERVICE_ID	0x20

#define MES_SET_ICON		0x01
#define MES_LOAD_ICON		0x02

#define MES_STORE_ICON		0x20

void mod_em_service_init();
void mod_em_service_service();

void mes_setIcon();
void mes_loadIcon();
void mes_storeIcon();

void mes_setAnimDelay();
void mes_setAnimSequence();

#endif /* MOD_EM_SERVICE_H_ */