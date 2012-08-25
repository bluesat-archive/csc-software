#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_
#include "UniversalReturnCode.h"

enum PROTOCOL_TO_USE_SENDING{
	AX25_UI=0,//AX 25 with unconnected mode
	nonsence //nothing else so far -.-
};

#define CALLSIGN_SIZE 6
#define BLANK_SPACE 0x40

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



 typedef enum
 {
    false,
    true,
 }Bool;

 typedef enum
 {
    Command,
    Response,
 }MessageType;

 typedef enum
 {
    Source,
    Destination,
    Repeater,
 }LocationType;


typedef struct
{
   char callSign[CALLSIGN_SIZE];
   unsigned int callSignSize;
   unsigned int ssid;
}Location;

typedef struct
{
   Location loc;
   Bool visited;
}ReptLoc;

typedef struct
{
   Location src;
   Location dest;
   MessageType type;
   ReptLoc * repeats;
   unsigned int totalRepeats;
}DeliveryInfo;

typedef struct
{
   char callSign[CALLSIGN_SIZE];
   char cORh :1; //Command / Response / Has been Bit
   char res_1:1; // Reserved bit default 1 - network may use
   char res_2:1;
   char ssid :4;
   char rept :1;
}LocSubField;

void vProtocols_Service_Init(unsigned portBASE_TYPE uxPriority);

#ifdef UNIT_TEST
UnivRetCode test_buildPacket (rawPacket * inputDetails);
UnivRetCode test_stuffBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff);
UnivRetCode test_initBuffer(buffer * input, char * buff, unsigned int size);
UnivRetCode test_bitPop (buffer* buff, char * out, unsigned int size);
UnivRetCode test_bitPush (buffer* buff, char in);
UnivRetCode test_buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater);
UnivRetCode test_addrBuilder (char * output, unsigned int * sizeLeft, DeliveryInfo * addrInfo);


#endif

#endif
