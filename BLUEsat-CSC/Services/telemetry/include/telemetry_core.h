/*
 * telemetry_core.h
 *
 *  Created on: Feb 16, 2013
 *      Author: andyc
 */

#ifndef TELEMETRY_CORE_H_
#define TELEMETRY_CORE_H_

void telem_core_semph_create(void);

unsigned int telemetry_core_read(unsigned int bus, unsigned int interface, char *sensorID);

void telemetry_core_print_temperature(unsigned int result, TaskToken telemTaskToken);

void telemetry_core_conversion(unsigned int bus, unsigned int interface);


#endif /* TELEMETRY_CORE_H_ */
