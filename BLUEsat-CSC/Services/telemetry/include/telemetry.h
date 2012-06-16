/*
 * telemetry.h
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#define TELEM_QUEUE_SIZE 16
#define TELEM_SEMAPHORE_BLOCK_TIME 10
#define TELEM_I2C_CONFIG_BITS 0x88
#define TELEM_BYTE_INVALID 0xFF

#include "UniversalReturnCode.h"
#include "i2c.h"
typedef struct {
	unsigned short address;
	unsigned char channel_mask;
	I2C_BUS_CHOICE bus : 8;
} sensor_lc;

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);

int retrieve_data(unsigned char* output, unsigned int size);

UnivRetCode setSweep(sensor_lc* config, unsigned int time_interval);



#endif /* TELEMETRY_H_ */
