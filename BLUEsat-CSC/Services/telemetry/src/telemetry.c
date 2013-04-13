/*
 * telemetry.c
 * Telemetry service
 * Created on: 16/06/2012
 * Author: andyc, colint
 */

#include "service.h"
#include "telemetry.h"
#include "i2c.h"
#include "semphr.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"
#include "debug.h"
#include "telemetry_core.h"
#include "telemetry_storage.h"
#include "telemetry_sensor_map.h"

/* Telemetry hardware information definition. */
#define TRANSLATOR_COUNT                4
#define MAX_TRANSLATOR_SENSOR_COUNT     10
#define TELEM_QUEUE_SIZE                16

/* Telemetry I2C control definition. */
#define TELEM_SEMAPHORE_BLOCK_TIME      (portTICK_RATE_MS * 5)

/* Telemetry sweep control definition. */
#define DEF_SWEEP_TIME                  2000 / portTICK_RATE_MS /* 20 seconds. */

/* Telemetry interface sensor count definition. */
static unsigned int telemInterfaceSensorCount[] = {10, 10, 10, 10};

TaskToken telemTaskToken;
static xSemaphoreHandle telemMutex;

static void
telemetry_read_single(unsigned int index, char *buffer, unsigned int size)
{
    /* Perform memory access for a single entry. */
    unsigned int nbytes = ((size < sizeof(struct telem_storage_entry_t)) ?
            size : sizeof(struct telem_storage_entry_t));
    struct telem_storage_entry_t entry;

    telemetry_storage_read_index(index, &entry);
    memcpy(buffer, &entry, nbytes);
}

static void
telemetry_read_latest(char *buffer, unsigned int size)
{
    /* Perform memory access for the latest entry. */
    unsigned int nbytes = ((size < sizeof(struct telem_storage_entry_t)) ?
            size : sizeof(struct telem_storage_entry_t));
    struct telem_storage_entry_t entry;

    telemetry_storage_read_cur(&entry);
    memcpy(buffer, &entry, nbytes);
}

static void
telemetry_sensor_store(int interface, struct telem_storage_entry_t *entry)
{
    unsigned int i;
    char *curSensor;
    unsigned short curResult;
    unsigned int baseIndex = 0;

    /* Calculate base index. */
    for (i = 0; i < (unsigned int)interface; i++) {
        baseIndex += telemInterfaceSensorCount[interface];
    }

    for (i = 0; i < telemInterfaceSensorCount[interface]; i++) {
        curSensor = (char *)telemetry_sensor_map[interface][i];
        curResult = (unsigned short)telemetry_core_read(BUS1, interface, curSensor);

        /* Store current result. */
        entry->values[baseIndex + i] = curResult;
    }

    /* Calculate timestamp. */
    entry->timestamp = 1337; /* TODO: */
}

static void
telemetry_sensor_poll(void)
{
    int i;
    struct telem_storage_entry_t entry;

    for (i = 0; i < TRANSLATOR_COUNT; i++) {
        telemetry_core_conversion(BUS1, i);
    }

    /* Read all data and store into the storage. */
    telemetry_sensor_store(0, &entry);
    //telemetry_sensor_store(1, &entry);
    //telemetry_sensor_store(2, &entry);
    //telemetry_sensor_store(3, &entry);

    /* Store the data in memory. */
    telemetry_storage_write(&entry);
}


/* Telemetry service main function. */
static portTASK_FUNCTION(vTelemTask, pvParameters)
{
    (void) pvParameters;
    UnivRetCode enResult;
    MessagePacket incomingPacket;
    telem_command_t *pComamndHandle;

    /* Initialise telemetry semaphore. */
    vSemaphoreCreateBinary(telemMutex);

    telem_core_semph_create();

    for (;;)
    {
        enResult = enGetRequest(telemTaskToken, &incomingPacket, DEF_SWEEP_TIME);

        /* Poll once when it gets unblocked regardless. */
        telemetry_sensor_poll();

        if (enResult != URC_SUCCESS) continue;

        vDebugPrint(telemTaskToken, "Message | process message...\n\r", 0,
                    NO_INSERT, NO_INSERT);
        /* Process command. */
        pComamndHandle = (telem_command_t *)incomingPacket.Data;
        switch (pComamndHandle->operation)
        {
            case TELEM_READ_SINGLE:
                vDebugPrint(telemTaskToken, "Message | read single sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_single(pComamndHandle->index, pComamndHandle->buffer,
                        pComamndHandle->size);
                break;
            case TELEM_READ_LATEST:
                vDebugPrint(telemTaskToken, "Message | read latest sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_latest(pComamndHandle->buffer, pComamndHandle->size);
                break;
            default:
                vCompleteRequest(incomingPacket.Token, URC_FAIL);
                break;
        }

        /* Complete request by passing the status to the sender. */
        vCompleteRequest(incomingPacket.Token, URC_SUCCESS);
    }
}

/*-----------------------------Telemetry public interfaces-------------------------------*/
UnivRetCode enTelemServiceMessageSend(TaskToken taskToken, unsigned portLONG data)
{
    MessagePacket outgoingPacket;

    /* Save information into the outgoing packet. */
    outgoingPacket.Src         = enGetTaskID(taskToken);
    outgoingPacket.Dest        = TASK_TELEM;
    outgoingPacket.Token       = taskToken;
    outgoingPacket.Data        = data;

    return enProcessRequest(&outgoingPacket, portMAX_DELAY);
}


UnivRetCode
vTelemInit(unsigned portBASE_TYPE uxPriority)
{
    telemTaskToken = ActivateTask(TASK_TELEM,
                                "Telem",
                                SEV_TASK_TYPE,
                                uxPriority,
                                SERV_STACK_SIZE,
                                vTelemTask);

    vActivateQueue(telemTaskToken, TELEM_QUEUE_SIZE);

    return URC_SUCCESS;
}

