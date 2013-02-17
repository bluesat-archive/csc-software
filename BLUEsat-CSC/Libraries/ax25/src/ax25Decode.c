#include "ax25.h"
#include "lib_string.h"

#define CALLSIGNSIZE 6
#define UCTOLCVALUE 32


typedef struct // ax25PacketEnd
   {
      unsigned char FCS0;
      unsigned char FCS1;
   }ax25FCS;






#define FCS_OFFSET_FROM_END = 3;



static Bool verifyFCS(char * packet, unsigned int size, ax25FCS * origFCS );
static decodeOptions determineDest ( rxPktStubs* input, const Location * self);
static UnivRetCode getAddressInfo (char* input, rxPktStubs * output, unsigned int inputSize);
static UnivRetCode ctrlDecode (ControlInfo* output, rxPktStubs * input);
static UnivRetCode populateRxPacketDetails (receivedPacket* out,rxPktStubs *packetStubs);
protoReturn ax25Receive (receivedPacket* out, char* input, unsigned int inputSize, const Location * self );

#ifdef UNIT_TEST

decodeOptions test_determineDest ( rxPktStubs* input, const Location * self)
{
   return determineDest (input, self);
}
protoReturn test_ax25Receive (receivedPacket* out, char* input, unsigned int inputSize, const Location * self )
{
   return ax25Receive (out,  input, inputSize,self );
}

#endif


/*
   Assumes that the packet has had its bit stuffing removed prior to decoding
*/
protoReturn ax25Receive (receivedPacket* out, char* input, unsigned int inputSize, const Location * self )
{
   rxPktStubs packetStubs;
   memset ((char*)&packetStubs,0,sizeof(packetStubs));
   if (out==NULL || input ==NULL || self==NULL || inputSize>SIZE_PACK ) return stateError;
   if (input[0]!=FLAG || input[inputSize-1]!=FLAG) return packetError; // Check that the packet has both start and end flags
   if (out->info==NULL || out->infoSize<MAX_PAYLOAD) return destBuffError;
   if (verifyFCS(&input[1], inputSize-4, (ax25FCS *) &input[inputSize-3] )==false) return FCSError; // inputSize-4 is because of the flags and the FCS
   if ( getAddressInfo (input, &packetStubs, inputSize)==URC_FAIL)return packetError;

   switch (determineDest ( &packetStubs, self))
   {
      case forUs:    if (ctrlDecode (&out->control, &packetStubs)!=URC_SUCCESS)return packetError; // If w are using repeaters we need the ctrl field to be at a dynamic offset
                     // At this point the stubs have been identified and we can pull out the info
                     break;//
      case weRepeat: break;// TODO
      case notUs:    return notUsError;
      default:       break;
   }
   if (populateRxPacketDetails (out,&packetStubs)==URC_FAIL) return packetError;

   // Handle cases

   return decodeSuccess;
}

// WARNING: Buffer overflow possibility
static UnivRetCode getAddressInfo (char* input, rxPktStubs * output, unsigned int inputSize)
{
   char * temp = input;
   if (input == NULL || output ==NULL) return URC_FAIL;

   temp = temp + 1; // Move past the flag
   output->dest = (LocSubField *) temp;
   temp = temp + sizeof(LocSubField);
   output->src = (LocSubField *)temp;
   temp = temp + sizeof(LocSubField);
  /* if (output->src->rept == 0)// Ignore repeaters for now
   {
      output->rep1 = (LocSubField *)temp;
      temp = temp + sizeof(LocSubField);
   }
   if (output->rep1 != NULL && output->rep1->rept == 0)
   {
      output->rep2 = (LocSubField *)temp;
      temp = temp + sizeof(LocSubField);
   }*/
   output->ctrl = *(ControlFrame*)temp;
   temp = temp+1;
   output->rest = temp;
   output->left = inputSize - (input - temp);
   return URC_SUCCESS;
}

static UnivRetCode getLocation (Location*out, LocSubField * in)
{
   unsigned int index;
   out->ssid = in->ssid;
   for (index = 0; index< CALLSIGNSIZE; ++index)
   {
         if (in->callSign[index]==0)break;
         out->callSign[index] = in->callSign[index];
   }
   out->callSignSize = index;
   return URC_SUCCESS;
}

static UnivRetCode populateRxPacketDetails (receivedPacket* out,rxPktStubs *packetStubs)
{
   if (out == NULL || packetStubs == NULL) return URC_FAIL;
   if (out->infoSize< packetStubs->left) return URC_FAIL;
   out->pid = packetStubs->pid;
   memcpy (out->info, packetStubs->info, packetStubs->infoSize);
   out->infoSize = packetStubs->infoSize;
   getLocation (&out->dest, packetStubs->dest);
   getLocation (&out->source, packetStubs->src);
   return URC_SUCCESS;
}


typedef struct  //decodeCtrlType
   {
      char info:1;
      char SupUnnum:1;
      char ignore:2;
      char pollFinal:1;
      char rest:3;
   }decodeCtrlType;


static CtrlFieldTypes packetType (char * input)
{
   decodeCtrlType * inputParts = (decodeCtrlType*) input;
   if (inputParts->info==0) return IFrame;
   if (inputParts->SupUnnum==0) return SFrame;
   return UFrame;
}

static UFrameCtlOpts uFrameCtlDecode (char * input)
{
   switch (input && 0xEC)
   {
      case 0x6C: return SetAsyncBalModeExtendedReq;
      case 0x1C: return SetAsyncBalModeReq;
      case 0x40: return DiscReq;
      case 0x0C: return DiscModeSysBusyDisconnected;
      case 0x60: return UnnumAck;
      case 0x84: return FrameReject;
      case 0x00: return UnnumInfoFrame;
      case 0xAC: return ExchangeID;
      case 0xE0: return Test;
   }
   return FrameReject; // Default
}

static UnivRetCode ctrlDecode (ControlInfo* output, rxPktStubs * input)
{
   UnivRetCode  result = URC_FAIL;
   if (output==NULL) return result;
   switch (packetType ((char*)&input->ctrl))
   {
      case IFrame:
                     output->type = IFrame;
                     output->sendSeqNum = input->ctrl.sendSeqNum;
                     output->poll       = input->ctrl.poll;
                     output->recSeqNum  = input->ctrl.recSeqNum;
                     input->pid         = * input->rest;
                     input->rest        = input->rest + 1;
                     input->left        = input->left - 1;
                     break;
      case SFrame:
                     output->type = SFrame;
                     output->sFrOpt     = input->ctrl.sendSeqNum>>1;
                     output->poll       = input->ctrl.poll;
                     output->recSeqNum  = input->ctrl.recSeqNum;

                     break;
      case UFrame:
                     output->type = SFrame;
                     output->poll       = input->ctrl.poll;
                     output->uFrOpt     = uFrameCtlDecode((char*)& input->ctrl);
                     break;
      default:
                     return result;
   }

   input->info        = input->rest;
   input->infoSize    = input->left-3;

   return URC_SUCCESS;
}


static inline unsigned char chartolower(unsigned char input)
{
   if (input > 0x40 && input < 0x91) return input + UCTOLCVALUE;
   return input;
}


static Bool fieldcmp (void * loc1, void* loc2, unsigned int size)
{
   unsigned char * temp_loc1 = (unsigned char *)loc1;
   unsigned char * temp_loc2 = (unsigned char *)loc2;
   unsigned char u1, u2;

   for ( ; size-- ; temp_loc1++, temp_loc2++) {
      u1 = chartolower(* (unsigned char *) temp_loc1);
      u2 = chartolower(* (unsigned char *) temp_loc2);
      if ( u1 != u2) {
          return (u1-u2);
      }
   }
   return 0;
}

static decodeOptions determineDest ( rxPktStubs* input, const Location * self)
{
   if (input == NULL) return notUs;
   if (fieldcmp ((void*) input->dest->callSign, (void*) &self->callSign, CALLSIGNSIZE)==0) return forUs;
   return notUs;
}

static unsigned short fcsEngine(unsigned short shiftReg, char * buff, unsigned int length)
{
   unsigned int inputbit;
   unsigned int inputbyte;
   unsigned short shiftedOutBit,xorMask;
   for(inputbyte=0,inputbit=0; inputbyte < length;)
   {
      shiftedOutBit = shiftReg & 0x0001;//shift the rightmost bit out

      shiftReg = shiftReg>>1;//shift one bit to the right

      //translate SR=xor(SR, XORMask) and XORMask = ...
      xorMask=( (((buff[inputbyte] & (0x1<<inputbit))>>inputbit) ^ shiftedOutBit))?AX25_CRC_POLYNOMIAL_FLIPED:0;
      shiftReg = shiftReg ^ xorMask;

      inputbit++;
      if(inputbit >= 8){
         inputbit=0;
         inputbyte++;
      }
   }
   return shiftReg;
}

static Bool verifyFCS(char * packet, unsigned int size, ax25FCS * origFCS )
{
   unsigned short shiftRegister = 0xFFFF; // Initial value for shift register
   unsigned char fcsByte0,fcsByte1;
   Bool result =false;
   if (packet == NULL ) return result;
   shiftRegister = fcsEngine(shiftRegister, packet,size);
   //flip and reverse the shift register to get the result
   shiftRegister =~shiftRegister;
   /*
    * The FCS are transmitted bit 15(leftmost) first
    *
    * This ought to be send from left to right(for the whole 16 bits!)
    * Also note that the modem sends bytes in Reverse
    * Also note that the ShiftRegister's MSB is the rightmost bit
    * i.e. no reverse inside bytes
    */
   fcsByte0 = shiftRegister&0x00FF;
   fcsByte1 = (shiftRegister&0xFF00)>>8;
   if (fcsByte0==origFCS->FCS0 && fcsByte1==origFCS->FCS1) result = true;
   return result;
}





























