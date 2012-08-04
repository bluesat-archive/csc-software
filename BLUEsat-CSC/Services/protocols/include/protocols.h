#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_
#include "UniversalReturnCode.h"

enum PROTOCOL_TO_USE_SENDING{
	AX25_UI=0,//AX 25 with unconnected mode
	nonsence //nothing else so far -.-
};

typedef struct
{
   char * addr;
   unsigned int addr_size;
   char * ctrl;
   unsigned int ctrl_size;
   char * pid;
   unsigned int pid_size;
   char * info;
   unsigned int info_size;
   char * fcs;
   unsigned int fcs_size;
} rawPacket;

//NOTE: At present we can only push a buffer or pop but not both
typedef struct
 {
    char * buff;
    unsigned int index;
    unsigned int byte_pos;
    unsigned int buff_size;
    unsigned int connectedOnes;
 }buffer;

void vProtocols_Service_Init(unsigned portBASE_TYPE uxPriority);

#ifdef UNIT_TEST
UnivRetCode test_buildPacket (rawPacket * inputDetails);
UnivRetCode test_stuffBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff);
buffer test_initBuffer(char * buff, unsigned int size);
UnivRetCode test_bitPop (buffer* buff, char * out, unsigned int size);
UnivRetCode test_bitPush (buffer* buff, char in);
#endif

#endif
