 /**
 *  \file application.h
 *
 *  \brief Header file contain vital #defines for application
 * 			ALL APPLICATIONS should include this file
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifdef SERVICE_H_
	#error "Task can only be application or service"
#endif

#ifndef APPLICATION_H_
#define APPLICATION_H_

	#include "FreeRTOS.h"
	#include "command.h"

	/* Application unique defines */
	#define APP_STACK_SIZE		configMINIMAL_STACK_SIZE
	#define APP_TASK_TYPE		TYPE_APPLICATION

#endif /* APPLICATION_H_ */
