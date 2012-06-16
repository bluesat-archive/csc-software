/*
 * telemetry.h
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

typedef struct {
	unsigned char address;
	unsigned char bus;
	unsigned short channel_mask;
} sensor_lc;

void vTelem_Init(unsigned portBASE_TYPE uxPriority);

void retrieve_data(char* output);

void setSweep(sensor_lc* config, unsigned int time_interval);



#endif /* TELEMETRY_H_ */
