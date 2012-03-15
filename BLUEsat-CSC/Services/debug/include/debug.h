/**
 * debug.h - Using UART to output debug message to terminal
 *
 * Create by: James Qin
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "command.h"

void vDebug_Init(unsigned portBASE_TYPE uxPriority);

#define DEBUG_PRINT_DEFERRED	0
#define DEBUG_PRINT_COMPLETE	1
signed portBASE_TYPE vDebug_Print(TaskToken taskToken,
								signed portCHAR *pcDebugString,
								unsigned portSHORT usLength,
								CALLBACK CallBackFunc);

#endif /* DEBUG_H_ */
