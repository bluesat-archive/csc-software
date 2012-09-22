#ifndef PROTOCOLS_H_
#define PROTOCOLS_H_
#include "UniversalReturnCode.h"

enum PROTOCOL_TO_USE_SENDING{
	AX25_UI=0,//AX 25 with unconnected mode
	nonsence //nothing else so far -.-
};

#define CALLSIGN_SIZE 6
#define BLANK_SPACE 0x40
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

#define MSB_bit_mask       0x80
#define PatternLimit       5

#define MAX_ADDR_FIELD     7*4


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
   char rept :1;
   char ssid :4;
   char res_2:1;
   char res_1:1; // Reserved bit default 1 - network may use
   char cORh :1; //Command / Response / Has been Bit

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
   char sFrame:1;
   char sendSeqNum:3;
   char poll:1;
   char recSeqNum:3;
}ControlFrame;

typedef enum //ax25States
{
   stateless,
}ax25States;

typedef enum //ax25OpMode
{
   connected,
   unconnected,
}ax25OpMode;


typedef struct //rawPacket
{
   char * addr;
   unsigned int addr_size;
   ControlInfo ctrl;
   char *  pid;            //PID is a pointer as it may not be required to be part of the packet
   unsigned int pid_size;
   char * info;
   unsigned int info_size;
} rawPacket;


typedef struct //stateBlock
{
   ax25OpMode mode;
   ax25States presState;
   DeliveryInfo route;
   char pid;
   char * src;
   unsigned int srcSize;
   unsigned int nxtIndex;
   unsigned int packetCnt;
   Bool completed; // Flag for the comms task to tell if the stream is complete
}stateBlock;


typedef enum //protoReturn
{
   stateError,
   destBuffError,
   addrGenError,
   infoGenError,
   packError,
   generationSuccess,
}protoReturn;



protoReturn ax25Entry (stateBlock* presentState, char* output, unsigned int * outputSize );



#ifdef UNIT_TEST

UnivRetCode test_stuffBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff);
UnivRetCode test_initBuffer(buffer * input, char * buff, unsigned int size);
UnivRetCode test_bitPop (buffer* buff, char * out, unsigned int size);
UnivRetCode test_bitPush (buffer* buff, char in);
UnivRetCode test_buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater);
UnivRetCode test_addrBuilder (char * output, unsigned int * outputSize, DeliveryInfo * addrInfo);
UnivRetCode test_ctrlBuilder (char * output,  ControlInfo* input);
UnivRetCode test_buildPacket (rawPacket * inputDetails, char * outFinal, unsigned int * outFinalSize );
UnivRetCode test_InfoBuilder (stateBlock * presentState, rawPacket* output);
UnivRetCode test_unconnectedEngine (stateBlock* presentState,  rawPacket* output);
UnivRetCode test_AX25fcsCalc( rawPacket* input, unsigned char *fcsByte0, unsigned char * fcsByte1);

#endif

#endif
