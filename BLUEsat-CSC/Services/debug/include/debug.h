/**
 * debug.h - Using UART to output debug message to terminal
 *
 * Create by: James Qin
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "command.h"

void vDebug_Init(unsigned portBASE_TYPE uxPriority);

void vDebug_Print(TaskToken taskToken,
				signed portCHAR *pDebugString,
				unsigned portSHORT usLength,
				CALLBACK CallBackFunc);

#endif /* DEBUG_H_ */
