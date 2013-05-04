/*
 * commsBuffer.h
 *
 *  Created on: Mar 29, 2013
 *      Author: colin
 */

#ifndef COMMSBUFFER_H_
#define COMMSBUFFER_H_
#include "UniversalReturnCode.h"

//NOTE: At present we can only push a buffer or pop but not both
typedef struct //buffer
 {
    char * buff;
    unsigned int index;
    unsigned int byte_pos;
    unsigned int buff_size;
    unsigned int connectedOnes; // Counter to track the number of consecutive 1s when this hits the threshold a 0 is inserted
 }buffer;

#define PatternLimit       4
#define MSB_bit_mask       0x80
#define LSB_bit_mask       0x01

UnivRetCode stuffBufMSBtoLSB (char * inputBuff, unsigned int input_size, buffer * outputBuff);
UnivRetCode stuffBufLSBtoMSB (char * inputBuff, unsigned int input_size, buffer * outputBuff);
UnivRetCode pushBuf    (char * inputBuff, unsigned int input_size, buffer * outputBuff);
UnivRetCode initBuffer (buffer * input, char * buff, unsigned int size);
UnivRetCode bitPopMSBtoLSB (buffer* buff, char * out, unsigned int size);
UnivRetCode bitPopLSBtoMSB (buffer* buff, char * out, unsigned int size);
UnivRetCode bitPushLSBtoMSB(buffer* buff, char in);
UnivRetCode bitPushMSBtoLSB(buffer* buff, char in);

#endif /* COMMSBUFFER_H_ */
