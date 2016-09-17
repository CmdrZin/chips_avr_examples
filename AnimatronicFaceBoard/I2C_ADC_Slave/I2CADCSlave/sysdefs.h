/*
 * sysdefs.h
 *
 * Created: 8/08/2015	0.01	ndp
 *  Author: Chip
 */ 


#ifndef SYSDEFS_H_
#define SYSDEFS_H_

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif


/* General purpose struct for init() and service(), neither return values. */
/* Access only puts data into the output fifo to be read. Does not return data. */
typedef struct
{
	const uint16_t	id;
	void		(*function)();
} MOD_FUNCTION_ENTRY;

typedef struct
{
	const uint16_t	id;
	const MOD_FUNCTION_ENTRY*	cmd_table;		// address of the command table for the device ID.
} MOD_ACCESS_ENTRY;


#endif /* SYSDEFS_H_ */