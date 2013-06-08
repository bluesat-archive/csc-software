/*
 * power_monitor.h
 *
 *  Created on: 08/06/2013
 *      Author: andyc
 */

#ifndef POWER_MONITOR_H_
#define POWER_MONITOR_H_

#define POWER_MON_COUNT 16

void power_mon_core_semph_create(void);

void power_monitor_sweep(unsigned int bus, unsigned short *voltages, unsigned short *currents);

#endif /* POWER_MONITOR_H_ */
