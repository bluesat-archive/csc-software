/*
 * telemetry.h
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_


#include "UniversalReturnCode.h"
#include "sensor_sweep.h"

typedef short sensor_result;

typedef struct {
	unsigned short address;
	unsigned char channel_mask;
	unsigned char bus;
} sensor_lc;

typedef enum
{
	SETSWEEP,
	READSWEEP
} Telem_Ops;

typedef struct
{
    /* Telem server operation. */
	Telem_Ops operation;
	/* Declare the size of the input buffer or the sweep buffer. */
	unsigned int size;
	union
	{
	    /* For storing the sweep settings. */
		Entity_sweep_params *paramBuffer;
		/* Pointer to the buffer where the results are. */
		sensor_result *buffer;
	};
} Telem_Cmd;

extern TaskToken telemTask_token;

void telem_debug_print(void);

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRY_H_ */
