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

void vProtocols_Service_Init(unsigned portBASE_TYPE uxPriority);

#ifdef UNIT_TEST
UnivRetCode test_buildPacket (rawPacket * inputDetails);
#endif

#endif
