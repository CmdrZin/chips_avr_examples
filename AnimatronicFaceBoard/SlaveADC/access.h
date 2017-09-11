/*
 * access.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 *
 * revision: 8/13/2015	0.02	ndp		make getMsgData() global.
 *
 */ 


#ifndef ACCESS_H_
#define ACCESS_H_

uint8_t getMsgData( uint8_t index );

void access_init(void);
void access_all(void);


#endif /* ACCESS_H_ */