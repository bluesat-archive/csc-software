/*
 * telemetry_core.c
 *
 *  Created on: Feb 16, 2013
 *      Author: andyc
 */

#include "FreeRTOS.h"
#include "service.h"
#include "i2c.h"
#include "telemetry_core.h"
#include "assert.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"
#include "debug.h"

#define SLAVE_ADDRESS_PREFIX        24
#define TELEM_CORE_BLOCK_TIME       (portTICK_RATE_MS * 5)

static xSemaphoreHandle telemMutex;

void
telem_core_semph_create(void)
{
    vSemaphoreCreateBinary(telemMutex);
}

#if 0
void
telem_init_hack(TaskToken telemTaskToken)
{
    int isValid = 1;
    char command[10];
    char readBuf[10];
    unsigned int length;
    portBASE_TYPE returnVal;

    /* Reset the state machine (I2C to 1 wire). */
    command[0] = 0xF0;
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Send passive config register (I2C to 1 wire). */
    command[0] = 0xD2;
    command[1] = 0xF0;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Generate 1 wire reset ((I2C to 1 wire). */
    command[0] = 0xB4;
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(2);

    /* Write 0xCC to 1-wire sensor (match ROM command). */
    command[0] = 0xA5;
    command[1] = 0xCC;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Strong pull-up configuration. */
    command[0] = 0xD2;
    command[1] = 0xB4;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Start conversion for sensor. */
    command[0] = 0xA5;
    command[1] = 0x44;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Sleep for 1 second. */
    vSleep(750);

    /* Passive config. */
    command[0] = 0xD2;
    command[1] = 0xF0;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Generate 1 wire reset ((I2C to 1 wire). */
    command[0] = 0xB4;
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(2);

    /* Write 0x55. (per chip) */
    command[0] = 0xA5;
    command[1] = 0x55;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Beginning of specifying unique ID. */
    command[0] = 0xA5; /* Write. */
    command[1] = 0x10;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0xF7;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x5E;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x83;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x02;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x08;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x00;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    command[0] = 0xA5;
    command[1] = 0x53;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);
    /* End of unique ID. */

    /* Write 0xBE to 1-wire bus. (request sratchpad) */
    command[0] = 0xA5;
    command[1] = 0xBE;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Request 1-wire read slots. Read first byte. */
    command[0] = 0x96;
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    //vSleep(1);

    /* Set data pointer to read data register. */
    command[0] = 0xE1;
    command[1] = 0xE1;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Perform a read. */
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_READ,
            (char*)&isValid, (char*)&readBuf[0], (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Request 1-wire read slots. Read second byte. */
    command[0] = 0x96;
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    //vSleep(1);

    /* Set data pointer to read data register. */
    command[0] = 0xE1;
    command[1] = 0xE1;
    length = 2;
    returnVal = Comms_I2C_Master(24, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Perform a read. */
    length = 1;
    returnVal = Comms_I2C_Master(24, I2C_READ,
            (char*)&isValid, (char*)&readBuf[1], (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    if (readBuf[1] == 0) {
        if ((readBuf[0] & 0x1) == 0) {
            vDebugPrint(telemTaskToken, "Current temperature is %d.0 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        } else {
            vDebugPrint(telemTaskToken, "Current temperature is %d.5 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        }
    } else {
        if ((readBuf[0] & 0x1) == 0) {
            vDebugPrint(telemTaskToken, "Current temperature is -%d.0 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        } else {
            vDebugPrint(telemTaskToken, "Current temperature is -%d.5 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        }
    }
}
#endif

unsigned int
telemetry_core_read(unsigned int bus, unsigned int interface, char *sensorID)
{
    int isValid = 1;
    char command[10];
    char readBuf[10];
    unsigned int length;
    portBASE_TYPE returnVal;
    int i;

    /* Passive config. */
    command[0] = 0xD2;
    command[1] = 0xF0;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Generate 1 wire reset ((I2C to 1 wire). */
    command[0] = 0xB4;
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(2);

    /* Write 0x55. (per chip) */
    command[0] = 0xA5;
    command[1] = 0x55;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Sending the unique ID. */
    for (i = 0; i < 8; ++i) {
        command[0] = 0xA5; /* Write. */
        command[1] = sensorID[7 - i];
        length = 2;
        returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
                (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
        xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
        vSleep(1);
    }

    /* Write 0xBE to 1-wire bus. (request sratchpad) */
    command[0] = 0xA5;
    command[1] = 0xBE;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Request 1-wire read slots. Read first byte. */
    command[0] = 0x96;
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Set data pointer to read data register. */
    command[0] = 0xE1;
    command[1] = 0xE1;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Perform a read. */
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_READ,
            (char*)&isValid, (char*)&readBuf[0], (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Request 1-wire read slots. Read second byte. */
    command[0] = 0x96;
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Set data pointer to read data register. */
    command[0] = 0xE1;
    command[1] = 0xE1;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Perform a read. */
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_READ,
            (char*)&isValid, (char*)&readBuf[1], (short*)&length, telemMutex, BUS1);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    return ((readBuf[1] << 8) | readBuf[0]);
}

void
telemetry_core_print_temperature(unsigned int result, TaskToken telemTaskToken)
{
    char readBuf[2];

    readBuf[1] = (result >> 8) & 0xFF;
    readBuf[0] = result & 0xFF;
    if (readBuf[1] == 0) {
        if ((readBuf[0] & 0x1) == 0) {
            vDebugPrint(telemTaskToken, "Current temperature is %d.0 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        } else {
            vDebugPrint(telemTaskToken, "Current temperature is %d.5 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        }
    } else {
        if ((readBuf[0] & 0x1) == 0) {
            vDebugPrint(telemTaskToken, "Current temperature is -%d.0 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        } else {
            vDebugPrint(telemTaskToken, "Current temperature is -%d.5 C\n\r",
                            readBuf[0]/2, NO_INSERT, NO_INSERT);
        }
    }
}

void
telemetry_core_conversion(unsigned int bus, unsigned int interface)
{
    int isValid = 1;
    char command[10];
    unsigned int length;
    portBASE_TYPE returnVal;

    /* Reset the state machine (I2C to 1 wire). */
    command[0] = 0xF0;
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Send passive config register (I2C to 1 wire). */
    command[0] = 0xD2;
    command[1] = 0xF0;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Generate 1 wire reset ((I2C to 1 wire). */
    command[0] = 0xB4;
    length = 1;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(2);

    /* Write 0xCC to 1-wire sensor (match ROM command). */
    command[0] = 0xA5;
    command[1] = 0xCC;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);
    vSleep(1);

    /* Strong pull-up configuration. */
    command[0] = 0xD2;
    command[1] = 0xB4;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Start conversion for sensor. */
    command[0] = 0xA5;
    command[1] = 0x44;
    length = 2;
    returnVal = Comms_I2C_Master(SLAVE_ADDRESS_PREFIX + interface, I2C_WRITE,
            (char*)&isValid, (char*)command, (short*)&length, telemMutex, bus);
    xSemaphoreTake(telemMutex, TELEM_CORE_BLOCK_TIME);

    /* Sleep for 1 second. */
    vSleep(750);
}


