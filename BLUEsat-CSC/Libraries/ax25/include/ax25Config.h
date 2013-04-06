/*
 * ax25Config.h
 *
 *  Created on: Mar 29, 2013
 *      Author: colin
 */

#ifndef AX25CONFIG_H_
#define AX25CONFIG_H_

#define CALLSIGN_SIZE 6
#define BLANK_SPACE 0x20
#define PROCTOCOLS_Q_SIZE  5
#define SIZE_FLAG          1   //Byte
#define SIZE_ADDR          14
#define SIZE_CTRL          1
#define SIZE_FCS           2
#define SIZE_PID           1
#define SIZE_MAX_INFO      256
#define SIZE_PACK          SIZE_FLAG+SIZE_ADDR+SIZE_CTRL+SIZE_PID+SIZE_MAX_INFO+SIZE_FCS+SIZE_FLAG
#define SIZE_STUFF         (SIZE_PACK*2)/8

/*Actual max amount of data that can be sent in a packet taking into account stuffing and a fixed maximum info field size*/
#define SIZE_ACT_INFO      SIZE_MAX_INFO - SIZE_STUFF
#define MAX_PAYLOAD        SIZE_ADDR+SIZE_CTRL+SIZE_PID+SIZE_ACT_INFO+SIZE_FCS
#define MAX_FIELDS         5
#define FLAG               0x7E
#define AX25_CONTROL_UI_INFORMATION 0x3
#define AX25_CRC_POLYNOMIAL_FLIPED 0x8408 // AX25 crc polynomial reversed bits by bits

#define NO_L3_PROTO        0xF0

#define MAX_ADDR_FIELD     7*4

#endif /* AX25CONFIG_H_ */
