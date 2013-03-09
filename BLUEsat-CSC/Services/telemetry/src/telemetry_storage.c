/*
 * telemetry_storage.c
 *
 *  Created on: Feb 2, 2013
 *      Author: andyc
 *
 *  Simple circular buffer implementation for telemetry storage using FRAM
 */

#include "FreeRTOS.h"
#include "service.h"
#include "telemetry_storage.h"
#include "storage.h"
#include "memory.h"
#include "assert.h"
#include "debug.h"

unsigned portCHAR *idList;

void
telemetry_storage_init(TaskToken telemTaskToken)
{
    /* Initialise the DID pool using malloc. */
    idList = (unsigned portCHAR *)pvJMalloc(TELEM_STORAGE_IDLIST_SIZE);
    if (idList == NULL) {
        vDebugPrint(telemTaskToken, "Storage init failed!\r\n",
                    NO_INSERT, NO_INSERT, NO_INSERT);
    }
}

size_t
telemetry_storage_write(TaskToken telemTaskToken, char *buf, size_t nbytes)
{
    (void)telemTaskToken;
    (void)buf;
    (void)nbytes;
    return 0;
}

size_t
telemetry_storage_read(TaskToken telemTaskToken, char *buf, size_t nbytes)
{
    (void)telemTaskToken;
    (void)buf;
    (void)nbytes;
    return 0;
}
