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

typedef struct {
	unsigned short address;
	unsigned char channelMask;
	unsigned char bus;
} SensorLocation;

typedef enum
{
	SETSWEEP,
	READSWEEP
} TelemOperation;

typedef struct
{
    /* Telem server operation. */
	TelemOperation operation;
	/* Declare the size of the input buffer or the sweep buffer. */
	unsigned int size;
	union
	{
	    /* For storing the sweep settings. */
		TelemEntityConfig *paramBuffer;
		/* Pointer to the buffer where the results are. */
		unsigned short *buffer;
	};
} TelemCommand;

UnivRetCode enTelemServiceMessageSend(TaskToken taskToken, unsigned portLONG data);

void vTelemDebugPrint(void);

UnivRetCode vTelemInit(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRY_H_ */
