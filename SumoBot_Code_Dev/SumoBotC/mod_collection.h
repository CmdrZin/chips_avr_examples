/*
 * mod_collection.h
 *
 * Created: 10/19/2015		0.01	ndp
 *  Author: Chip
 */ 


#ifndef MOD_COLLECTION_H_
#define MOD_COLLECTION_H_

#include <stdbool.h>

void mco_init();
void mco_service();

int mco_GetLastVal();
int mco_GetPeakVal();

#endif /* MOD_COLLECTION_H_ */