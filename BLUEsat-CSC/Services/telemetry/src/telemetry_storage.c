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

struct telem_storage_entry_t *cur = NULL;

void
telemetry_storage_init(void)
{
    cur = (struct telem_storage_entry_t *)TELEM_STORAGE_BASE_ADDR;
}

void
telemetry_storage_reset(void)
{
    unsigned int *ptr = (unsigned int *)TELEM_STORAGE_BASE_INDEX;
    *ptr = 0;
}

void
telemetry_storage_save_cur(void)
{
    unsigned int *ptr = (unsigned int *)TELEM_STORAGE_BASE_INDEX;
    *ptr = (unsigned int)cur;
}

void
telemetry_storage_load_prev_cur(void)
{
    cur = (struct telem_storage_entry_t *)*((volatile unsigned int *)TELEM_STORAGE_BASE_INDEX);
}

int
telemetry_storage_write(struct telem_storage_entry_t *buf)
{
    if (!buf) {
        return -1;
    }
    memcpy((char *)cur, (char *)buf, sizeof(struct telem_storage_entry_t));
    cur++;

    if ((unsigned int)cur == (TELEM_STORAGE_BASE_ADDR + (TELEM_MAX_ENTRIES *
            sizeof(struct telem_storage_entry_t)))) {
        cur = (struct telem_storage_entry_t *)TELEM_STORAGE_BASE_ADDR;
    }

    return 0;
}

int
telemetry_storage_read_cur(struct telem_storage_entry_t *buf)
{
	if (cur == NULL){
		return -1;
	}
    struct telem_storage_entry_t *entry = cur;
    entry--;
    if (!buf) {
        return -1;
    }


    memcpy((char *)buf, (char *)entry, sizeof(struct telem_storage_entry_t));
    return 0;
}

int
telemetry_storage_read_index(unsigned int i, struct telem_storage_entry_t *buf)
{
    struct telem_storage_entry_t *entry;

    if (i > TELEM_MAX_ENTRIES) return -1;

    entry = (struct telem_storage_entry_t *)(TELEM_STORAGE_BASE_ADDR +
            (i * sizeof(struct telem_storage_entry_t)));
    memcpy((char *)buf, (char *)entry, sizeof(struct telem_storage_entry_t));

    return 0;
}


