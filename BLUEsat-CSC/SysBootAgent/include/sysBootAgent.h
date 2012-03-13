/*
 *
 *  sysBootAgent.h - Header file for system boot agent
 *
 *  Created by: James Qin
 */

#ifndef SYSBOOTAGENT_H_
#define SYSBOOTAGENT_H_

unsigned int initDrivers(void);

unsigned int initServices(void);

unsigned int initApplications(void);

#endif /* SYSBOOTAGENT_H_ */
