#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_

enum PROTOCOL_TO_USE_SENDING{
	AX25_UI=0,//AX 25 with unconnected mode
	nonsence //nothing else so far -.-
};

void vProtocols_Service_Init(unsigned portBASE_TYPE uxPriority);

#endif
