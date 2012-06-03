/**
 *  \file switching.h
 *
 *  \brief THE switching circuit driver
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-06-02 16:38:58 +1100 (Sat, 2 June 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note
 */

#ifdef APPLICATION_H_
	#error "Applications should access drivers via services!"
#endif

#ifndef SWITCHING_H_
#define SWITCHING_H_

#include "FreeRTOS.h"

#define SWITCHING_NO_BLOCK 0

#define REPEATER_MODE 1
#define DEVICE_MODE 0

#define TX_1 0
#define TX_2 1

#define RX_1 0
#define RX_2 1

#define BEACON 0
#define AFSK_1 1
#define AFSK_2 2
#define GMSK_1 3
#define GMSK_2 4

void switching_takeSemaphore(void);

void switching_giveSemaphore(void);

void Switching_Init(void);

void switching_TX(unsigned char TX);

void switching_RX(unsigned char RX);

void switching_OPMODE(unsigned char mode);

void switching_TX_Device(unsigned char device);

void switching_RX_Device(unsigned char device);
#endif /* SWITCHING_H_*/
