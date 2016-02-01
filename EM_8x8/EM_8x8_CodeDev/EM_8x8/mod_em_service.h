/*
 * mod_em_service.h
 *
 * Created: 1/13/2016	0.01	ndp
 *  Author: Chip
 * revision: 2/01/2016	0.02	ndp	Added marquee support
 */ 


#ifndef MOD_EM_SERVICE_H_
#define MOD_EM_SERVICE_H_

#define MOD_EM_SERVICE_ID	0x20

#define MES_SET_ICON		0x01
#define MES_LOAD_ICON		0x02
#define MES_SEL_MARQUEE		0x40
#define MES_SET_KERNING		0x41
#define MES_SET_CONT_FLAG	0x42
#define MES_SET_RATE		0x43
#define MES_LOAD_MSERIES	0x44

#define MES_STORE_ICON		0x20

void mod_em_service_init();
void mod_em_service_service();

void mes_setIcon();
void mes_loadIcon();
void mes_storeIcon();

void mes_setAnimDelay();
void mes_setAnimSequence();

void mes_selectMarquee();
void mes_setKerningFlag();
void mes_setContinuousFlag();
void mes_setRate();
void mes_setIconSeries();


#endif /* MOD_EM_SERVICE_H_ */