/*
 * ax25.h
 *
 *  Created on: Mar 29, 2013
 *      Author: colin
 */

#ifndef AX25_H_
#define AX25_H_
#include "UniversalReturnCode.h"
#include "command.h"
#include "ax25Config.h"

/*Delivery Location Structures*/

      typedef enum //Bool
      {
         false = pdFALSE,
         true = pdTRUE,
      }Bool;

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
#define sizeOfLocSubField CALLSIGN_SIZE+1

/**************************************************************************/

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

#define sizeOfControlFrame 1

/**************************************************************************/

      /*
       * State Block Structures
       * */

      typedef enum //protoReturn
      {
         stateError,
         destBuffError,
         addrGenError,
         infoGenError,
         packError,
         generationSuccess,
         decodeSuccess,
         FCSError,
         notUsError,
         packetError,

      }protoReturn;

      typedef enum //ax25States
      {
         stateless,
      }ax25States;

      typedef enum //ax25OpMode
      {
         connected,
         unconnected,
      }ax25OpMode;

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

/**************************************************************************/

void vSetToken(TaskToken         taskToken);

protoReturn ax25Entry (stateBlock* presentState, char* output, unsigned int * outputSize );

#ifdef UNIT_TEST
//Encode
UnivRetCode test_buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater);
UnivRetCode test_addrBuilder (char * output, unsigned int * outputSize, DeliveryInfo * addrInfo);
UnivRetCode test_ctrlBuilder (ControlFrame * output,  ControlInfo* input);
UnivRetCode test_buildPacket (rawPacket * inputDetails, char * outFinal, unsigned int * outFinalSize );
UnivRetCode test_InfoBuilder (stateBlock * presentState, rawPacket* output);
UnivRetCode test_unconnectedEngine (stateBlock* presentState,  rawPacket* output);
UnivRetCode test_AX25fcsCalc( rawPacket* input, unsigned char *fcsByte0, unsigned char * fcsByte1);

//Decode
decodeOptions test_determineDest ( rxPktStubs* input, const Location * self);
protoReturn test_ax25Receive (receivedPacket* out, char* input, unsigned int inputSize, const Location * self );
#endif
#endif /* AX25_H_ */
