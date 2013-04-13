/*
 * telemetry.h
 *
 *  Created on: Mar 23, 2013
 *      Author: andyc
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include "UniversalReturnCode.h"

typedef enum
{
    TRANSLATOR_A = 0,
    TRANSLATOR_B,
    TRANSLATOR_C,
    TRANSLATOR_D,
    TRANSLATOR_COUNT
} telem_interface;

typedef enum
{
    TELEM_READ_SINGLE = 0,
    TELEM_READ_LATEST
} telem_operation;

typedef struct
{
    /* Telem access operation. */
    telem_operation operation;

    /* Telem index for single read. */
    unsigned int index;

    char *buffer;
    char size;
} telem_command_t;

UnivRetCode enTelemServiceMessageSend(TaskToken taskToken, unsigned portLONG data);

UnivRetCode vTelemInit(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRY_H_ */
