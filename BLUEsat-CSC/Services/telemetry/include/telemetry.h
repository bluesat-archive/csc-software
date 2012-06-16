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
#include "i2c.h"
typedef struct {
	unsigned char address;
	I2C_BUS_CHOICE bus;
	unsigned char channel_mask;
} sensor_lc;

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);

int retrieve_data(unsigned char* output, unsigned int size);

UnivRetCode setSweep(sensor_lc* config, unsigned int time_interval);



#endif /* TELEMETRY_H_ */
