 /**
 *  \file Demo_Application_1.h
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef DEMO_APPLICATION_1_H_
#define DEMO_APPLICATION_1_H_

#include "UniversalReturnCode.h"

/**
 * \brief Initialise Demo application
 *
 * \param[in] uxPriority Priority for debug service.
 */
void vDemoApp1_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Put message to queue
 *
 * \param[in] taskToken Task token from sender task
 *
 * \param[in] pcDebugString Pointer to message.
 *
 * \param[in] usLength Length of message.
 */
UnivRetCode enMessage_To_Q(TaskToken taskToken,
							signed portCHAR *pcDebugString,
							unsigned portSHORT usLength);

#endif /* DEMO_APPLICATION_1_H_ */
