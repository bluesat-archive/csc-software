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

#ifndef SYSBOOTAGENT_H_
	#ifdef SERVICE_H_
		#error "Task can only be application or service"
	#endif
#endif

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "FreeRTOS.h"
#include "command.h"

/* Application unique defines */
#define NO_BLOCK			0
#define APP_STACK_SIZE		configMINIMAL_STACK_SIZE

#endif /* APPLICATION_H_ */
