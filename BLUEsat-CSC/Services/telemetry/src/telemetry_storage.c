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

/*
 *  data structure
 *
 *  bit
 *  0       1       2       3  ...  255
 *
 * info  entry 1  entry 2
 *
 */

struct telem_storage_info_t
{
    unsigned char idFreeList[TELEM_STORAGE_IDLIST_SIZE];
    unsigned int idFreeListCount;
    unsigned int current;
    unsigned int latest;
};

static struct telem_storage_info_t info;

static unsigned int
telemetry_get_next_free_id(void)
{
    unsigned int returnVal;

    if (info.idFreeListCount == 0) {
        if (info.current == (TELEM_STORAGE_IDLIST_SIZE - 1)) {
            info.current = 1;
        }
        returnVal = info.current;
        info.current++;
    } else {
        returnVal = info.idFreeList[info.idFreeListCount - 1];
        info.idFreeList[info.idFreeListCount - 1] = 0;
        info.idFreeListCount--;
    }

    /* Update the latest entry. */
    info.latest = returnVal;

    return returnVal;
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
    unsigned int i;

    enResult = enDataSize(telemTaskToken, 0, &size);
    /* Check if the storage DID exists. */
    if (size != sizeof(struct telem_storage_info_t)) {
        /* Initialise the structure. */
        info.idFreeListCount = 0;
        info.current = 1;

        for (i = 0; i < TELEM_STORAGE_IDLIST_SIZE; i++) {
            info.idFreeList[i] = 0;
        }
        telemetry_storage_update_info(telemTaskToken);
    }
}

int
telemetry_storage_write(TaskToken telemTaskToken, char *buf, size_t nbytes)
{
    UnivRetCode enResult;
    int returnDID;

    returnDID = telemetry_get_next_free_id();
    /* Delete the entry before right every time. Allow overwrite to happen. */
    enResult = enDataDelete(telemTaskToken, returnDID);

    enResult = enDataStore(telemTaskToken, returnDID, nbytes, buf);

    /* Perform memory access for a single entry. */
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
