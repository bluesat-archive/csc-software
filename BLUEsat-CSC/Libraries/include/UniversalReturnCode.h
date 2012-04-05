 /**
 *  \file UniversalReturnCode.h
 *
 *  \brief Define return code used in Applcations and Services
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */
 
 #ifndef UNIVERSALRETURNCODE_H_
 #define UNIVERSALRETURNCODE_H_
 
#include "FreeRTOS.h"

typedef enum
{
	URC_SUCCESS = pdPASS,
	URC_FAIL	= pdFAIL,
	URC_BUSY,
	URC_MEM_INVALID_DID,
	URC_CMD_NO_QUEUE,
	URC_CMD_INVALID_TASK,
	URC_CMD_NO_TASK
} UnivRetCode;

#endif /* UNIVERSALRETURNCODE_H_ */
