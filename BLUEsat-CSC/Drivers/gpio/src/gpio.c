/**
 *  \file gpio.c
 *
 *  \brief THE GPIO driver, used to read and write to gpio pins
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-05-12 16:38:58 +1100 (Sat, 12 May 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note
 */

#include "lpc24xx.h"
#include "gpio.h"
//set GPIO output to be high/low
void setGPIO(unsigned char portNo, unsigned char pinNo, unsigned char newValue){
	if(newValue){
		if (portNo == 0){
			FIO0SET = (0x1<<pinNo);
		} else if (portNo == 1){
			FIO1SET = (0x1<<pinNo);
		} else if (portNo == 2){
			FIO2SET = (0x1<<pinNo);
		} else if (portNo == 3){
			FIO3SET = (0x1<<pinNo);
		} else {
			FIO4SET = (0x1<<pinNo);
		}
	} else {
		if (portNo == 0){
			FIO0CLR = (0x1<<pinNo);
		} else if (portNo == 1){
			FIO1CLR = (0x1<<pinNo);
		} else if (portNo == 2){
			FIO2CLR = (0x1<<pinNo);
		} else if (portNo == 3){
			FIO3CLR = (0x1<<pinNo);
		} else {
			FIO4CLR = (0x1<<pinNo);
		}
	}
}
//get GPIO input value
int getGPIO(unsigned char portNo, unsigned char pinNo){
	if (portNo == 0){
		return (FIO0PIN&(0x1<<pinNo))>>pinNo;
	} else if (portNo == 1){
		return (FIO1PIN&(0x1<<pinNo))>>pinNo;
	} else if (portNo == 2){
		return (FIO2PIN&(0x1<<pinNo))>>pinNo;
	} else if (portNo == 3){
		return (FIO3PIN&(0x1<<pinNo))>>pinNo;
	} else {
		return (FIO4PIN&(0x1<<pinNo))>>pinNo;
	}
}
//set GPIO direction to be input/output
void setGPIOdir(unsigned char portNo, unsigned char pinNo, unsigned char direction){
	if (direction == INPUT){//input
		if (portNo == 0){
			FIO0DIR &= ~(1<<pinNo);
		} else if (portNo == 1){
			FIO1DIR &= ~(1<<pinNo);
		} else if (portNo == 2){
			FIO2DIR &= ~(1<<pinNo);
		} else if (portNo == 3){
			FIO3DIR &= ~(1<<pinNo);
		} else {
			FIO4DIR &= ~(1<<pinNo);
		}
	} else {//output
		if (portNo == 0){
			FIO0DIR |= (1<<pinNo);
		} else if (portNo == 1){
			FIO1DIR |= (1<<pinNo);
		} else if (portNo == 2){
			FIO2DIR |= (1<<pinNo);
		} else if (portNo == 3){
			FIO3DIR |= (1<<pinNo);
		} else {
			FIO4DIR |= (1<<pinNo);
		}
	}
}
void set_Gpio_func(unsigned char portNo, unsigned char pinNo, unsigned char func){
	int shiftposition = (pinNo%16)*2;
	func = func&3; //mask the bottom 2 bits
	switch (portNo) {
		case 0:
			if (pinNo<16){
				PINSEL0&=~(3<<shiftposition);
				PINSEL0|=(func<<shiftposition);
			} else {
				PINSEL1&=~(3<<shiftposition);
				PINSEL1|=(func<<shiftposition);
			}
		break;
		case 1:
			if (pinNo<16){
				PINSEL2&=~(3<<shiftposition);
				PINSEL2|=(func<<shiftposition);
			} else {
				PINSEL3&=~(3<<shiftposition);
				PINSEL3|=(func<<shiftposition);
			}
		break;
		case 2:
			if (pinNo<16){
				PINSEL4&=~(3<<shiftposition);
				PINSEL4|=(func<<shiftposition);
			} else {
				PINSEL5&=~(3<<shiftposition);
				PINSEL5|=(func<<shiftposition);
			}
		break;
		case 3:
			if (pinNo<16){
				PINSEL6&=~(3<<shiftposition);
				PINSEL6|=(func<<shiftposition);
			} else {
				PINSEL7&=~(3<<shiftposition);
				PINSEL7|=(func<<shiftposition);
			}
		break;
		case 4:
			if (pinNo<16){
				PINSEL8&=~(3<<shiftposition);
				PINSEL8|=(func<<shiftposition);
			} else {
				PINSEL9&=~(3<<shiftposition);
				PINSEL9|=(func<<shiftposition);
			}
		break;
	}
}
void Gpio_Init(void){
	SCS|= 1; //make port 0 and port 1 non legacy
}
void led_init(void){
	int i;
	for (i = 23; i < 27; ++i)
	{
		set_Gpio_func(3, i, 0);
		setGPIOdir(3, i, OUTPUT);
	}
	set_Gpio_func(2, 18, 0);
	setGPIOdir(2, 18, OUTPUT);
	set_Gpio_func(2, 23, 0);
	setGPIOdir(2, 23, OUTPUT);
	set_Gpio_func(2, 25, 0);
	setGPIOdir(2, 25, OUTPUT);
	set_Gpio_func(2, 26, 0);
	setGPIOdir(2, 26, OUTPUT);
}
void led(char output){
	setGPIO(3, 23, output&1);
	setGPIO(2, 23, (output&2)>>1);
	setGPIO(2, 18, (output&4)>>2);
	setGPIO(3, 24, (output&8)>>3);
	setGPIO(2, 26, (output&16)>>4);
	setGPIO(3, 25, (output&32)>>5);
	setGPIO(3, 26, (output&64)>>6);
	setGPIO(2, 25, (output&128)>>7);
}
