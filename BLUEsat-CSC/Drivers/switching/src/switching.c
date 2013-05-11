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

void Switching_Init(void){
	//set S0 to S7 to be GPIO
	set_Gpio_func(2, 11, 0);
	set_Gpio_func(2, 12, 0);
	set_Gpio_func(2, 13, 0);
	set_Gpio_func(2, 15, 0);
	set_Gpio_func(2, 17, 0);
	set_Gpio_func(1, 26, 0);
	set_Gpio_func(2, 21, 0);
	set_Gpio_func(1, 25, 0);

	//set S0 to S7 to be output
	setGPIOdir(2, 11, OUTPUT);
	setGPIOdir(2, 12, OUTPUT);
	setGPIOdir(2, 13, OUTPUT);
	setGPIOdir(2, 15, OUTPUT);
	setGPIOdir(2, 17, OUTPUT);
	setGPIOdir(1, 26, OUTPUT);
	setGPIOdir(2, 21, OUTPUT);
	setGPIOdir(1, 25, OUTPUT);

}

void switching_TX(unsigned char TX){
	setGPIO(2, 17, TX);
}

void switching_RX(unsigned char RX){
	setGPIO(2, 13, RX);
}

void switching_OPMODE(unsigned char mode){
	setGPIO(2, 11, mode);
}

void switching_TX_Device(unsigned char device){
	switch (device){
		case BEACON:
			setGPIO(1, 25, 0);
			setGPIO(2, 21, 0);
			setGPIO(1, 26, 0);
		break;
		case AFSK_1:
			setGPIO(1, 25, 0);
			setGPIO(2, 21, 0);
			setGPIO(1, 26, 1);
		break;
		case AFSK_2:
			setGPIO(1, 25, 0);
			setGPIO(2, 21, 1);
			setGPIO(1, 26, 0);
		break;
		case GMSK_1:
			setGPIO(1, 25, 1);
			setGPIO(2, 21, 1);
			setGPIO(1, 26, 0);
		break;
		case GMSK_2:
			setGPIO(1, 25, 1);
			setGPIO(2, 21, 0);
			setGPIO(1, 26, 0);
		break;
	}
}

void switching_RX_Device(unsigned char device){
	switch (device){
		case AFSK_1:
			setGPIO(2, 15, 0);
		break;
		case AFSK_2:
			setGPIO(2, 15, 1);
		break;
		case GMSK_1:
			setGPIO(2, 12, 0);
		break;
		case GMSK_2:
			setGPIO(2, 12, 1);
		break;
	}
}
