/*
 * telemetry.h
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#define TELEM_QUEUE_SIZE 16


#include "UniversalReturnCode.h"

typedef struct {
	unsigned char address;
	unsigned char bus;
	unsigned short channel_mask;
} sensor_lc;

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);

UnivRetCode retrieve_data(char* output);

UnivRetCode setSweep(sensor_lc* config, unsigned int time_interval);



#endif /* TELEMETRY_H_ */
