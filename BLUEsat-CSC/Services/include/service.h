/**
 * service.h - Header file contain vital #defines for services
 * ALL SERVICES should include this file
 *
 * Created by: James Qin
 */

#ifdef APPLICATION_H_
	error "Task can only be application or service"
#endif

#ifndef SERVICE_H_
#define SERVICE_H_

#include "FreeRTOS.h"

/* Service unique defines */

#define SERV_STACK_SIZE		configMINIMAL_STACK_SIZE

#endif /* SERVICE_H_ */
