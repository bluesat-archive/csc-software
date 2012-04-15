/**
 *  \file service.h
 *
 *  \brief Header file contain vital #defines for services
 * 			ALL SERVICES should include this file
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifdef APPLICATION_H_
	#error "Task can only be application or service"
#endif

#ifndef SERVICE_H_
#define SERVICE_H_

	#include "FreeRTOS.h"
	#include "semphr.h"
	#include "command.h"

	/* Service unique defines */
	#define SERV_STACK_SIZE		configMINIMAL_STACK_SIZE
	#define SEV_TASK_TYPE		TYPE_SERVICE

#endif /* SERVICE_H_ */

