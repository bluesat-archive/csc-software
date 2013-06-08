/*
 * power_monitor.c
 *
 *  Created on: 08/06/2013
 *      Author: andyc
 */

#include "FreeRTOS.h"
#include "service.h"
#include "i2c.h"
#include "assert.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"
#include "debug.h"
#include "power_monitor.h"
#include "telemetry.h"

#define SLAVE_ADDRESS_PREFIX        24 /* 11000 << 1 = 110000 */
#define POWER_MON_BLOCK_TIME       (portTICK_RATE_MS * 5)

static xSemaphoreHandle powerMonMutex;
static int vRangesList[POWER_MON_COUNT] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int shuntOhmList[POWER_MON_COUNT] = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000,
        1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000};

static unsigned int
power_monitor_write(unsigned int bus, unsigned int addr, char *cmd, unsigned int writeByte)
{
    int isValid = 1;
    unsigned int length;
    portBASE_TYPE returnVal;

    /* Reset the state machine (I2C to 1 wire). */
    length = writeByte;
    vDebugPrint(telemTaskToken, "POWER | write addr is %d\n\r", addr, NO_INSERT, NO_INSERT);
    returnVal = Comms_I2C_Master(addr, I2C_WRITE, (char*)&isValid, (char*)cmd, (short*)&length,
            powerMonMutex, bus);
    xSemaphoreTake(powerMonMutex, POWER_MON_BLOCK_TIME);
    vSleep(1);

    return length;
}

static unsigned int
power_monitor_read(unsigned int bus, unsigned int addr, char *buf, unsigned int readByte)
{
    int isValid = 1;
    unsigned int length;
    portBASE_TYPE returnVal;
    /* Perform a read. */
    length = readByte;
    vDebugPrint(telemTaskToken, "POWER | read addr is %d\n\r", addr, NO_INSERT, NO_INSERT);
    returnVal = Comms_I2C_Master(addr, I2C_READ, (char*)&isValid, (char*)&buf,
            (short*)&length, powerMonMutex, bus);
    xSemaphoreTake(powerMonMutex, POWER_MON_BLOCK_TIME);
    vSleep(1);

    return length;
}

void
power_mon_core_semph_create(void)
{
    vSemaphoreCreateBinary(powerMonMutex);
}

/* Note shuntOhm is in Ohm. Original C# code is in MOhm. */
static void
power_monitor_read_voltage_current(unsigned int bus, unsigned int addr, unsigned int vRange,
        unsigned int shuntOhm, unsigned short *voltage, unsigned short *current)
{
    char command[10];
    char result[10];
    unsigned int nbytes;
    unsigned int iCode;
    unsigned int vCode;
    unsigned int vInt;
    unsigned int iInt;

    /* Write the 1 byte command. */
    memset(command, 0, 10);
    command[0] = (1 << 1 | 1 << 3); /* V_ONCE, I_ONCE */
    if (!vRange) command[0] |= (1 << 4); /* VRANGE */
    nbytes = power_monitor_write(bus, addr, command, 1);

    vSleep(10);
    /* Read 3 bytes. */
    memset(result, 0, 10);
    nbytes = power_monitor_read(bus, addr, result, 3);

    vCode = (result[0] << 4) | (result[2] >> 4);
    iCode = (result[1] << 4) | (result[2] & 0xF);
    vDebugPrint(telemTaskToken, "POWER | vCode is %d\n\r", vCode, NO_INSERT, NO_INSERT);
    vDebugPrint(telemTaskToken, "POWER | iCode is %d\n\r", iCode, NO_INSERT, NO_INSERT);

    vInt = vCode * (vRange ? 26520 : 6650) / 4096; /* mV */
    iInt = (iCode * 105840 / 4096) * 1000 / shuntOhm; /* uA */

    /* Cast to short. */
    *voltage = (unsigned short)vInt;
    *current = (unsigned short)iInt;
}

void
power_monitor_sweep(unsigned int bus, unsigned short *voltages, unsigned short *currents)
{
    /* Address in the xlsx sheet is already left shifted. We shift it back. */
    unsigned int startAddr;
    int i;

    startAddr = 0x60;
    for (i = 0; i < POWER_MON_COUNT; i++) {
        power_monitor_read_voltage_current(bus, (startAddr >> 1), vRangesList[i],
                shuntOhmList[i], &voltages[i], &currents[i]);
        startAddr += 2;
    }
}





