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
#include "string.h"

struct telem_storage_info_t
{
    unsigned char idList[TELEM_STORAGE_IDLIST_SIZE];
    unsigned int current;
};

static struct telem_storage_info_t info;

static void
telemetry_storage_update_current(void)
{
    while (info.idList[info.current] != 0) {
        info.current++;
    }
}

static void
telemetry_storage_update_info(TaskToken telemTaskToken)
{
    UnivRetCode enResult;
    /* DID 0 is reserved for telemtry data info. */
    enResult = enDataDelete(telemTaskToken, 0);

    enResult = enDataStore(telemTaskToken, 0, sizeof(struct telem_storage_info_t), (char *)&info);

    /* FIXME: error handling. */
}

void
telemetry_storage_init(TaskToken telemTaskToken)
{
    unsigned long size;
    UnivRetCode enResult;

    enResult = enDataSize(telemTaskToken, 0, &size);
    /* Check if the storage DID exists. */
    if (size != sizeof(struct telem_storage_info_t)) {
        /* Initialise the structure. */
        info.current = 0;
        telemetry_storage_update_info(telemTaskToken);
    }
}

int
telemetry_storage_write(TaskToken telemTaskToken, char *buf, size_t nbytes)
{
    UnivRetCode enResult;
    int returnDID;

    enResult = enDataStore(telemTaskToken, (info.current + 1), nbytes, buf);
    info.idList[info.current] = info.current + 1;
    returnDID = info.current;

    telemetry_storage_update_current();
    telemetry_storage_update_info(telemTaskToken);
    return returnDID;
}

int
telemetry_storage_read(TaskToken telemTaskToken, unsigned char DID, char *buf, size_t nbytes,
        size_t offset)
{
    UnivRetCode enResult;
    unsigned long size;
    enResult = enDataRead(telemTaskToken, DID, nbytes, offset, buf, &size);

    return size;
}
