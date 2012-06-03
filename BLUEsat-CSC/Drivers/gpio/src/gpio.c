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
			FIO0MASK &= ~(0x1<<pinNo);
			FIO0SET = (0x1<<pinNo);
		} else if (portNo == 1){
			FIO1MASK &= ~(0x1<<pinNo);
			FIO1SET = (0x1<<pinNo);
		} else if (portNo == 2){
			FIO2MASK &= ~(0x1<<pinNo);
			FIO2SET = (0x1<<pinNo);
		} else if (portNo == 3){
			FIO3MASK &= ~(0x1<<pinNo);
			FIO3SET = (0x1<<pinNo);
		} else {
			FIO4MASK &= ~(0x1<<pinNo);
			FIO4SET = (0x1<<pinNo);
		}
	} else {
		if (portNo == 0){
			FIO0MASK &= ~(0x1<<pinNo);
			FIO0CLR = (0x1<<pinNo);
		} else if (portNo == 1){
			FIO1MASK &= ~(0x1<<pinNo);
			FIO1CLR = (0x1<<pinNo);
		} else if (portNo == 2){
			FIO2MASK &= ~(0x1<<pinNo);
			FIO2CLR = (0x1<<pinNo);
		} else if (portNo == 3){
			FIO3MASK &= ~(0x1<<pinNo);
			FIO3CLR = (0x1<<pinNo);
		} else {
			FIO4MASK &= ~(0x1<<pinNo);
			FIO4CLR = (0x1<<pinNo);
		}
	}
}
//get GPIO input value
int getGPIO(unsigned char portNo, unsigned char pinNo){
	if (portNo == 0){
		FIO0MASK &= ~(0x1<<pinNo);
		return FIO0PIN&(0x1<<pinNo);
	} else if (portNo == 1){
		FIO1MASK &= ~(0x1<<pinNo);
		return FIO1PIN&(0x1<<pinNo);
	} else if (portNo == 2){
		FIO2MASK &= ~(0x1<<pinNo);
		return FIO2PIN&(0x1<<pinNo);
	} else if (portNo == 3){
		FIO3MASK &= ~(0x1<<pinNo);
		return FIO3PIN&(0x1<<pinNo);
	} else {
		FIO4MASK &= ~(0x1<<pinNo);
		return FIO4PIN&(0x1<<pinNo);
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
				PINSEL0|=(func<<shiftposition);
			} else {
				PINSEL1|=(func<<shiftposition);
			}
		break;
		case 1:
			if (pinNo<16){
				PINSEL2|=(func<<shiftposition);
			} else {
				PINSEL3|=(func<<shiftposition);
			}
		break;
		case 2:
			if (pinNo<16){
				PINSEL4|=(func<<shiftposition);
			} else {
				PINSEL5|=(func<<shiftposition);
			}
		break;
		case 3:
			if (pinNo<16){
				PINSEL6|=(func<<shiftposition);
			} else {
				PINSEL7|=(func<<shiftposition);
			}
		break;
		case 4:
			if (pinNo<16){
				PINSEL8|=(func<<shiftposition);
			} else {
				PINSEL9|=(func<<shiftposition);
			}
		break;
	}
}
void Gpio_Init(void){
	SCS|= 1; //make port 0 and port 1 non legacy
}
