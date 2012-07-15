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

void telem_debug_print(void);

void vTelemSampleSetSweep(int resolution, int rate, TaskToken token);

void vTelemReadSweep(void *buffer, TaskToken token);

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRY_H_ */
