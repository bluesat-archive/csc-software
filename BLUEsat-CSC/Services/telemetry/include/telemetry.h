/*
 * telemetry.h
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_


#include "UniversalReturnCode.h"
#include "i2c.h"

typedef short sensor_result;

typedef struct {
	unsigned short address;
	unsigned char channel_mask;
	I2C_BUS_CHOICE bus : 8;
} sensor_lc;



UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority);



#endif /* TELEMETRY_H_ */
