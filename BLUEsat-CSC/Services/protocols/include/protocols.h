#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_
#include "UniversalReturnCode.h"

enum PROTOCOL_TO_USE_SENDING{
	AX25_UI=0,//AX 25 with unconnected mode
	nonsence //nothing else so far -.-
};

#define CALLSIGN_SIZE 6
#define BLANK_SPACE 0x40

typedef struct //rawPacket
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
typedef struct //buffer
 {
    char * buff;
    unsigned int index;
    unsigned int byte_pos;
    unsigned int buff_size;
    unsigned int connectedOnes; // Counter to track the number of consecutive 1s when this hits the threshold a 0 is inserted
 }buffer;

typedef enum //Bool
 {
    false,
    true,
 }Bool;

typedef enum //MessageType
 {
    Command,
    Response,
 }MessageType;

typedef enum //LocationType
 {
    Source,
    Destination,
    Repeater,
 }LocationType;

typedef struct //Location
{
   char callSign[CALLSIGN_SIZE];
   unsigned int callSignSize;
   unsigned int ssid;
}Location;

typedef struct //ReptLoc
{
   Location loc;
   Bool visited;
}ReptLoc;

typedef struct //DeliveryInfo
{
   Location src;
   Location dest;
   MessageType type;
   ReptLoc * repeats;
   unsigned int totalRepeats;
}DeliveryInfo;

typedef struct //LocSubField
{
   char callSign[CALLSIGN_SIZE];
   char cORh :1; //Command / Response / Has been Bit
   char res_1:1; // Reserved bit default 1 - network may use
   char res_2:1;
   char ssid :4;
   char rept :1;
}LocSubField;


/*
 * Control Field Structures
 * */
typedef enum //SFrameSendSeqNumOpts
{
   RecReady        = 0,
   RecNotReady     = 2,
   Reject          = 4,
   SelectiveReject = 6,
   NoSFrameOpts,
}SFrameSendSeqNumOpts;

typedef enum //UFrameCtlOpts
{
   SetAsyncBalModeReq,
   SetAsyncBalModeExtendedReq, //Modulo 128
   DiscReq,
   FrameReject,
   UnnumInfoFrame,
   DiscModeSysBusyDisconnected,
   ExchangeID,
   UnnumAck,
   Test,
   NoUFrameOpts,
}UFrameCtlOpts;

typedef enum //CtrlFieldTypes
{
   IFrame,
   SFrame,
   UFrame,
}CtrlFieldTypes;

typedef struct //ControlInfo
{
   CtrlFieldTypes type;
   char recSeqNum;
   char sendSeqNum;
   char poll;
   SFrameSendSeqNumOpts sFrOpt;
   UFrameCtlOpts uFrOpt;
}ControlInfo;
/*
Type    |765 | 4 |321 |0|
-------------------------
I Frame |N(R)| P |N(S)|0|
S Frame |N(R)|P/F|SS0 |1|
U Frame |MMM |P/F|MM1 |1|
*/
typedef struct {//ControlFrame
   char recSeqNum:3;
   char poll:1;
   char sendSeqNum:3;
   char sFrame:1;
}ControlFrame;



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
UnivRetCode test_ctrlBuilder (char * output, unsigned int remaining, ControlInfo* input);

#endif

#endif
