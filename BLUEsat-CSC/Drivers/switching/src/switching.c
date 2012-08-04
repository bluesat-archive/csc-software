/**
 *  \file switching.c
 *
 *  \brief THE switching circuit driver
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-06-02 16:38:58 +1100 (Sat, 02 June 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note
 */

#include "lpc24xx.h"
#include "switching.h"
#include "gpio.h"
#include "task.h"
#include "semphr.h"

static xSemaphoreHandle switching_MUTEX;

static int createSwitchingSemaphore(void)
{
	vSemaphoreCreateBinary( switching_MUTEX );
	if(!switching_MUTEX) return pdFAIL;

	return pdTRUE;
}

void switching_takeSemaphore(void)
{
	xSemaphoreTake( switching_MUTEX, SWITCHING_NO_BLOCK );
}

void switching_giveSemaphore(void)
{
	xSemaphoreGive( switching_MUTEX );
}

void Switching_Init(void){
	int pinNo;
	//set S0 to S7 to be GPIO
	for (pinNo = 8; pinNo < 16; pinNo++){
		set_Gpio_func(1, pinNo, 0);
	}
	//set S0 to S7 to be output
	for (pinNo = 8; pinNo < 16; pinNo++){
		setGPIOdir(1, pinNo, OUTPUT);
	}
	//Initialise the semaphore
	createSwitchingSemaphore();

}

void switching_TX(unsigned char TX){
	setGPIO(1, 15, TX);
}

void switching_RX(unsigned char RX){
	setGPIO(1, 14, RX);
}

void switching_OPMODE(unsigned char mode){
	setGPIO(1, 13, mode);
}

void switching_TX_Device(unsigned char device){
	switch (device){
		case BEACON:
			setGPIO(1, 12, 0);
			setGPIO(1, 11, 0);
			setGPIO(1, 10, 0);
		break;
		case AFSK_1:
			setGPIO(1, 12, 1);
			setGPIO(1, 11, 0);
			setGPIO(1, 10, 0);
		break;
		case AFSK_2:
			setGPIO(1, 12, 0);
			setGPIO(1, 11, 1);
			setGPIO(1, 10, 0);
		break;
		case GMSK_1:
			setGPIO(1, 12, 1);
			setGPIO(1, 11, 1);
			setGPIO(1, 10, 0);
		break;
		case GMSK_2:
			setGPIO(1, 12, 0);
			setGPIO(1, 11, 0);
			setGPIO(1, 10, 1);
		break;
	}
}

void switching_RX_Device(unsigned char device){
	switch (device){
		case AFSK_1:
			setGPIO(1, 9, 0);
			setGPIO(1, 8, 0);
		break;
		case AFSK_2:
			setGPIO(1, 9, 0);
			setGPIO(1, 8, 1);
		break;
		case GMSK_1:
			setGPIO(1, 9, 1);
			setGPIO(1, 8, 0);
		break;
		case GMSK_2:
			setGPIO(1, 9, 1);
			setGPIO(1, 8, 1);
		break;
	}
}
