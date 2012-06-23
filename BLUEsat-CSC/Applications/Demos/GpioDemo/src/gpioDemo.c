 /**
 *  \file gpioDemo.c
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-06-02 16:35:54 +1100 (Sun, 02 June 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "gpioDemo.h"
#include "debug.h"

#define INPUT 0
#define OUTPUT 1
#define MESSAGE_WAIT_TIME 500

static TaskToken DEMO_TaskToken;

void setGPIO(unsigned char portNo, unsigned char pinNo, unsigned char newValue);
void setGPIOdir(unsigned char portNo, unsigned char pinNo, unsigned char direction);
void set_Gpio_func(unsigned char portNo, unsigned char pinNo, unsigned char func);

static portTASK_FUNCTION(vDemoTask, pvParameters);
static void setGPIOOutput(void);
static void setGPIOallLow(void);
static void setGPIOallHigh(void);



void vGpioDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_GPIO_DEMO,
								"gpio_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);

	// initialise the port direction
	setGPIOOutput();
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	int i;
	int j = 0;

	(void) pvParameters;

	static unsigned int dummy = 0xffff;

	for ( ; ; )
	{
		for (i = 0; i < 0xfffff; ++i)
		{
			dummy =~ dummy;
		}

		if (j % 2 == 0)
		{
			setGPIOallHigh();
		} else
		{
			setGPIOallLow();
		}
		++j;

	}
}

static void setGPIOallHigh(void)
{
	int i, j;

	for (j = 1; j < 4; ++j)
	{
		for (i = 0; i < 32; ++i)
		{
			setGPIO(j, i, 1);
		}
	}
}

static void setGPIOallLow(void)
{
	int i, j;

	for (j = 1; j < 4; ++j)
	{
		for (i = 0; i < 32; ++i)
		{
			setGPIO(j, i, 0);
		}
	}
}

static void setGPIOOutput(void)
{
	int i, j;

	for (j = 0; j < 4; ++j)
	{
		for (i = 0; i < 32; ++i)
		{
			set_Gpio_func(j, i, 0);
			setGPIOdir(j, i, OUTPUT);
		}
	}
}

