/**
 * application.h - Header file contain vital #defines for application
 * ALL APPLICATIONS should include this file
 *
 * Created by: James Qin
 */

#ifdef SERVICE_H_
	error "Task can only be application or service"
#endif

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FreeRTOS.h"
#include "semphr.h"

/* Application unique defines */

#define APP_STACK_SIZE		configMINIMAL_STACK_SIZE

#endif /* APPLICATION_H_ */
