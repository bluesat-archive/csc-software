 /**
 *  \file i2cTest.c
 *
 *  \brief A service to test the I2C Communications
 *
 *  \author $Author: Colin Tan $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "service.h"
#include "i2cTest.h"
#include "debug.h"
#include "lib_string.h"
#include "i2c.h"


static portTASK_FUNCTION(vI2CTestTask, pvParameters);

static TaskToken I2CTest_TaskToken;
void vI2CTest_Init(unsigned portBASE_TYPE uxPriority)
{

	I2CTest_TaskToken = ActivateTask(TASK_I2C_TEST,
									"I2CTest",
									TYPE_SERVICE,
									uxPriority,
									SERV_STACK_SIZE,
									vI2CTestTask);

}

static void i2cprint (char * message)
{
	vDebugPrint(I2CTest_TaskToken,
						message,
						NO_INSERT,
						NO_INSERT,
						NO_INSERT);
}

static portTASK_FUNCTION(vI2CTestTask, pvParameters)
{
	(void) pvParameters;
	unsigned int range = 0;
	unsigned int base = 0x50;
	unsigned int temp;
	Comms_I2C_Init();
	i2cprint("\n\rMOOOOOOOOOOOOOOOOO\n\r");

	for ( ; ; )
	{
		for (range = 0; range<8;range++)
		{
			temp = base&& range;


			/* process pollboards */
						while (FOREVER)
						{
							result = iTelemGetmVoltage(0, 0x50, &isValid);
							vIntToStr(result, 1, returnStr);
							Comms_UART_Write_Str(returnStr, 40);
							Comms_UART_Write_Str(" mV \n\r", 40);

						}

		}
	}
}




static unsigned int iTelemGetmVoltage(unsigned int channel, char addressOfADC, char *isValid)
{
	unsigned int result;
	/*
	 * Send control if fail return failure
	 * else
	 *
	 *
	 * */

	// request the voltage
	{
		portCHAR data;
		portSHORT length=1;
		data = (char)(0x88 + (channel << 4));
		Comms_I2C_Master(addressOfADC, I2C_WRITE, isValid, &data, &length, NULL, BUS0);
		xSemaphoreTake( sem, 5 );
	}

	// read the voltage
	{
		portSHORT length = 2;
		portCHAR data[2];
		data[0] = 0xff;
		data[1] = 0xff;
		Comms_I2C_Master(addressOfADC, I2C_READ, isValid, data, &length, NULL, BUS0);
		xSemaphoreTake( sem, 5 );
	}
    result = ((data[0]<<4 & data[1]>>4) * 2.4414); // 7F 10 -> 7F 1 -> 2033 (decimal)
    return result;
}

