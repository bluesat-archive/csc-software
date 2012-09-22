#include "service.h"
#include "debug.h"
#include "protocols.h"
#include "lib_string.h"



/*
 * AX. 25
 */


static UnivRetCode AX25fcsCalc( rawPacket* input, unsigned char *fcsByte0, unsigned char * fcsByte1);
static UnivRetCode buildPacket (rawPacket * inputDetails, char * outFinal, unsigned int * outFinalSize );
static UnivRetCode stuffBuf   (char * inputBuff, unsigned int input_size, buffer * outputBuff);
static UnivRetCode initBuffer (buffer * input, char * buff, unsigned int size);
static UnivRetCode bitPop     (buffer* buff, char * out, unsigned int size);
static UnivRetCode bitPush    (buffer* buff, char in);
static UnivRetCode buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater);
static UnivRetCode addrBuilder (char * output, unsigned int * outputSize, DeliveryInfo * addrInfo);
static UnivRetCode ctrlBuilder (char * output, ControlInfo* input);
static UnivRetCode unconnectedEngine (stateBlock* presentState,  rawPacket* output);
static UnivRetCode InfoBuilder (stateBlock * presentState, rawPacket* output);
static UnivRetCode pushBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff);

#ifdef UNIT_TEST

UnivRetCode test_stuffBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   return stuffBuf (inputBuff, input_size, outputBuff);
}

UnivRetCode test_initBuffer(buffer * input, char * buff, unsigned int size)
{
   return initBuffer(input, buff, size);
}

UnivRetCode test_bitPop (buffer* buff, char * out, unsigned int size)
{
   return bitPop (buff, out, size);
}

UnivRetCode test_bitPush (buffer* buff, char in)
{
   return bitPush (buff, in);
}

UnivRetCode test_buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater)
{
   return buildLocation (destBuffer, sizeLeft, loc, msgType, locType, visitedRepeater, isLastRepeater);
}

UnivRetCode test_addrBuilder (char * output, unsigned int * outputSize, DeliveryInfo * addrInfo)
{
   return addrBuilder (output, outputSize, addrInfo);
}

UnivRetCode test_ctrlBuilder (char * output,  ControlInfo* input)
{
   return ctrlBuilder (output, input);
}

UnivRetCode test_buildPacket (rawPacket * inputDetails, char * outFinal, unsigned int * outFinalSize )
{
   return buildPacket (inputDetails, outFinal, outFinalSize );
}

UnivRetCode test_InfoBuilder (stateBlock * presentState, rawPacket* output)
{
   return InfoBuilder (presentState, output);
}

UnivRetCode test_unconnectedEngine (stateBlock* presentState,  rawPacket* output)
{
   return unconnectedEngine (presentState,  output);
}

UnivRetCode test_AX25fcsCalc( rawPacket* input, unsigned char *fcsByte0, unsigned char * fcsByte1)
{
   return AX25fcsCalc( input, fcsByte0, fcsByte1);
}

#endif


//State block built by comms task
//Return multiple negative numbers to indicate the issue
/*
 * input validation issues
 *
 * */
protoReturn ax25Entry (stateBlock* presentState, char* output, unsigned int * outputSize )
{
   rawPacket packet;
   stateBlock tempState;
   char addrBuff [MAX_ADDR_FIELD];
   unsigned int addrBuffSize = MAX_ADDR_FIELD;
   if ( output == NULL || outputSize == NULL) return destBuffError;
   if ( presentState == NULL) return stateError;


   tempState = *presentState;

   // Process State
   switch (tempState.mode)
   {
      case unconnected:   if (unconnectedEngine (&tempState,  &packet) == URC_FAIL) return stateError;
                     break;
      case connected:                             //TODO: Create connected state
      default:
                     return stateError;      //TODO: Possible unknown mode error
   }

   // Build Address
   if (addrBuilder (addrBuff, &addrBuffSize, &tempState.route) == URC_FAIL) return addrGenError;
   packet.addr       = addrBuff;
   packet.addr_size  = addrBuffSize;

   // BUild Info
   if (InfoBuilder (&tempState,  &packet) == URC_FAIL) return infoGenError;

   //build packet in output buffer
   if ( buildPacket (&packet, output, outputSize ) == URC_FAIL) return packError;

   *presentState = tempState;

   return generationSuccess;
}

/*
 * State Engines
 *
 * */


static UnivRetCode unconnectedEngine (stateBlock* presentState,  rawPacket* output)
{
   UnivRetCode result = URC_FAIL;
   ControlInfo ctrlIn;
   if (presentState == NULL || output == NULL) return result;
   output->pid = &presentState->pid;
   output->addr_size = 1; // PID is 1 byte.
   ctrlIn.type = UFrame;
   ctrlIn.poll = 0;
   ctrlIn.uFrOpt = UnnumInfoFrame;
   return ctrlBuilder ((char *)&output->ctrl, &ctrlIn);
}

// Updates the state block with the next position to read the src from and the output block with the position to start reading from the the size
// The info should always have stuff in it.
static UnivRetCode InfoBuilder (stateBlock * presentState, rawPacket* output)
{
   UnivRetCode result = URC_FAIL;
   unsigned int remaining;
   if (presentState == NULL || output == NULL) return result;
   if (presentState->src == NULL || presentState->nxtIndex >= presentState->srcSize ) return result;
   output->info = &presentState->src [presentState->nxtIndex];
   remaining  = presentState->srcSize - presentState->nxtIndex;
   if (remaining > SIZE_ACT_INFO) //There is a need to split the data over multiple packets
   {
      output->info_size       = SIZE_ACT_INFO;
      presentState->nxtIndex += SIZE_ACT_INFO;
      presentState->completed = false;
   }
   else
   {
      output->info_size       = remaining;
      presentState->nxtIndex  = presentState->srcSize;
      presentState->completed = true;
   }
   return URC_SUCCESS;
}



static UnivRetCode buildPacket (rawPacket * inputDetails, char * outFinal, unsigned int * outFinalSize )
{
   UnivRetCode result = URC_FAIL;
   char flag = FLAG;
   buffer outBuff;
   unsigned char fcs0, fcs1;
   if (inputDetails==NULL) {return result;}
   if (inputDetails->addr == NULL ||
       inputDetails->info == NULL){return result;}
   if (inputDetails->addr_size == 0 ||
       inputDetails->info_size == 0){return result;}
   // Init Output buffer
   if (initBuffer(&outBuff, outFinal,*outFinalSize) == URC_FAIL)return result;

   // Calculate FCS
   if (AX25fcsCalc( inputDetails, &fcs0, &fcs1) == URC_FAIL ) return result;

   // Stuff data into output packet
   if (pushBuf  (&flag, 1, &outBuff) == URC_FAIL ) return result;
   if (stuffBuf (inputDetails->addr, inputDetails->addr_size, &outBuff) == URC_FAIL ) return result;
   if (stuffBuf ((char *) &inputDetails->ctrl, 1, &outBuff) == URC_FAIL ) return result;
   if (inputDetails->pid!=NULL)
   {
      if (stuffBuf ((char *) inputDetails->pid,  1, &outBuff) == URC_FAIL ) return result; // Assume PID is of size 1 byte
   }
   if (stuffBuf (inputDetails->info, inputDetails->info_size, &outBuff) == URC_FAIL ) return result;
   if (stuffBuf (&fcs0, 1 , &outBuff) == URC_FAIL ) return result;
   if (stuffBuf (&fcs1, 1 , &outBuff) == URC_FAIL ) return result;
   if (pushBuf  (&flag, 1,  &outBuff) == URC_FAIL ) return result;

   // The index the the present location in the buffer not the total size.
   // The result returns the total size in the buffer
   *outFinalSize = outBuff.index + 1;
   return URC_SUCCESS;
}




/*
 * Address Field Functions
 * -----------------------
 * NOTE:AX25 v2.2 in use
 * */

static UnivRetCode buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater)
{
   LocSubField * dest;
   UnivRetCode result = URC_FAIL;
   unsigned int index;
   if (destBuffer==NULL || sizeLeft == NULL || loc == NULL) return result;
   if (*sizeLeft< sizeof (LocSubField))return result;
   dest =  * destBuffer;

   // Populate Callsign
   memset (&dest->callSign,BLANK_SPACE<<1,CALLSIGN_SIZE);
   for (index=0;index<loc->callSignSize; ++index)
      {
         dest->callSign[index] = loc->callSign[index]<<1;
      }

   // Populate SSID Field
   dest->cORh =((msgType == Command      && locType == Destination) ||
                (msgType == Response     && locType == Source)      ||
                (visitedRepeater == true && locType == Repeater))?1: 0;
   dest->rept = (isLastRepeater == true)?1:0;
   dest->ssid = loc->ssid;
   dest->res_1 = 1;
   dest->res_2 = 1;
   // Reposition buffer pointers
   *destBuffer += 1;
   *sizeLeft   -= sizeof (LocSubField);

   result = URC_SUCCESS;
   return result;
}


// this code modifies the buffer directly and in the event of a code failure the buffer should be discarded
static UnivRetCode addrBuilder (char * output, unsigned int * outputSize, DeliveryInfo * addrInfo)
{
   UnivRetCode result = URC_FAIL;
   ReptLoc *temp;
   char * temp_output;
   Bool last;
   unsigned int index;
   unsigned int tempSize;
   if (output == NULL || outputSize == NULL || addrInfo == NULL) return result;

   tempSize = *outputSize;
   temp_output = output;
   if (buildLocation ((LocSubField **)&temp_output, &tempSize, &addrInfo->dest, addrInfo->type, Destination, false, false) == URC_FAIL) return result;
   if (buildLocation ((LocSubField **)&temp_output, &tempSize, &addrInfo->src,  addrInfo->type, Source,      false, (addrInfo->repeats==NULL)) == URC_FAIL) return result;

   // Populate Repeater Fields
   if (addrInfo->repeats!=NULL)
   {
      temp = addrInfo->repeats;
      for (index = 0; index< addrInfo->totalRepeats;++index)
      {
         last = (index==( addrInfo->totalRepeats-1))?true:false;
         if (buildLocation ((LocSubField **)&temp_output, &tempSize, &temp[index].loc , addrInfo->type, Repeater, temp[index].visited, last) == URC_FAIL) return result;
      }
   }
   *outputSize = *outputSize - tempSize;
   return URC_SUCCESS;
}

/*
 * Control Field Functions
 *
 * */

static char UFrF1Decode (UFrameCtlOpts input)
{
   char output;
   switch (input)
   {
      case NoUFrameOpts:
      case UnnumInfoFrame:
      case DiscModeSysBusyDisconnected:
                                          output = 0x00;
                                          break;
      case SetAsyncBalModeReq:            output = 0x01;
                                          break;
      case DiscReq:                       output = 0x02;
                                          break;
      case UnnumAck:
      case SetAsyncBalModeExtendedReq:    output = 0x03;
                                          break;
      case FrameReject:                   output = 0x04;
                                          break;
      case ExchangeID:                    output = 0x05;
                                          break;
      case Test:                          output = 0x07;
                                          break;
   }
   return output;
}

static char UFrF2Decode (UFrameCtlOpts input)
{
   char output;
      // Field 2 are for bits 1-3 where bit 1 is always 1
      switch (input)
      {
         case NoUFrameOpts:
         case UnnumAck:
         case DiscReq:
         case UnnumInfoFrame:
         case Test:                          output = 0x01;
                                             break;
         case FrameReject:                   output = 0x03;
                                             break;
         case DiscModeSysBusyDisconnected:
         case SetAsyncBalModeReq:
         case SetAsyncBalModeExtendedReq:
         case ExchangeID:                    output = 0x07;
                                             break;
      }
      return output;
}

// Does not update the output pointer and count
//assume modulo 8 operation mode
static UnivRetCode ctrlBuilder (char * output, ControlInfo* input)
{
   UnivRetCode result = URC_FAIL;
   ControlFrame * tempOut = (ControlFrame *)output;
   if (output == NULL|| input == NULL) return result;

   switch (input->type)
   {
      case IFrame:   tempOut->recSeqNum  = input->recSeqNum;
                     tempOut->sendSeqNum = input->sendSeqNum;
                     tempOut->poll       = input->poll;
                     tempOut->sFrame     = 0;
                     break;
      case SFrame:   if (input->sFrOpt == NoSFrameOpts) return result;
                     tempOut->recSeqNum  = input->recSeqNum;
                     tempOut->sendSeqNum = input->sFrOpt;
                     tempOut->poll       = input->poll;
                     tempOut->sFrame     = 1;
                     break;
      case UFrame:   if (input->uFrOpt == NoUFrameOpts) return result;
                     tempOut->recSeqNum  = UFrF1Decode(input->uFrOpt);
                     tempOut->sendSeqNum = UFrF2Decode(input->uFrOpt);
                     tempOut->poll       = input->poll;
                     tempOut->sFrame     = 1;
                     break;
      default:
                     return result;
   }
   return URC_SUCCESS;
}


/*
 * FCS Field Function
 * ------------------
 * */

/*
 * This function calculates the FCS checksum based on a MATLAB implementation
 * Which is obtained from :
 *		The Cyclic Redundancy Check (CRC) for AX.25
 *		Bill Newhall, KB2BRD
 *		billnewhall@yahoo.com
 *		Boulder, Colorado
 *
 */
static unsigned short fcsEngine(unsigned short shiftReg, char * buffer, unsigned int length)
{
   unsigned int inputbit;
   unsigned int inputbyte;
   unsigned short shiftedOutBit,xorMask;
   for(inputbyte=0,inputbit=0; inputbyte < length;)
   {
      shiftedOutBit = shiftReg & 0x0001;//shift the rightmost bit out

      shiftReg = shiftReg>>1;//shift one bit to the right

      //translate SR=xor(SR, XORMask) and XORMask = ...
      xorMask=( (((buffer[inputbyte] & (0x1<<inputbit))>>inputbit) ^ shiftedOutBit))?AX25_CRC_POLYNOMIAL_FLIPED:0;
      shiftReg = shiftReg ^ xorMask;

      inputbit++;
      if(inputbit >= 8){
         inputbit=0;
         inputbyte++;
      }
   }
   return shiftReg;
}

static UnivRetCode AX25fcsCalc( rawPacket* input,unsigned char *fcsByte0, unsigned char * fcsByte1){
   //short should be 16bits, change data type if it isn't
   unsigned short shiftRegister = 0xFFFF; // Initial value for shift register

   if (fcsByte0==NULL||fcsByte1==NULL||input==NULL) return URC_FAIL;
   if (input->addr==NULL) return URC_FAIL;
   if (input->info==NULL) return URC_FAIL;

   shiftRegister = fcsEngine(shiftRegister, input->addr,input->addr_size);
   shiftRegister = fcsEngine(shiftRegister, (char*) &input->ctrl,1);
   if (input->pid!=NULL)
   {
      shiftRegister = fcsEngine(shiftRegister, input->pid,input->pid_size);
   }
   shiftRegister = fcsEngine(shiftRegister, input->info,input->info_size);

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
   (*fcsByte0) = shiftRegister&0x00FF;
   (*fcsByte1) = (shiftRegister&0xFF00)>>8;
   return URC_SUCCESS;
}


/*
 * Bit Stuffing Functions
 * ---------------------
 * */
static UnivRetCode stuffBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   UnivRetCode result = URC_FAIL;
   char temp;
   buffer input;
   if (inputBuff==NULL || outputBuff == NULL ||input_size==0)
   {
         return result;
   }
   if (initBuffer(&input, inputBuff, input_size) == URC_FAIL)return result;
   while (bitPop (&input, &temp, sizeof (char))==URC_SUCCESS)
      {
         outputBuff->connectedOnes = (temp==0)?0: outputBuff->connectedOnes+1;
         if ( outputBuff->connectedOnes > PatternLimit)
            {
               outputBuff->connectedOnes = 1; // Take into account the 1 to be added after this if block
               if (bitPush (outputBuff, 0)== URC_FAIL)return result;
            }
         if (bitPush (outputBuff, temp)== URC_FAIL)return result;
      }
   return URC_SUCCESS;
}

static UnivRetCode pushBuf (char * inputBuff, unsigned int input_size, buffer * outputBuff)
{
   UnivRetCode result = URC_FAIL;
   char temp;
   buffer input;
   if (inputBuff==NULL || outputBuff == NULL ||input_size==0)
   {
         return result;
   }
   if (initBuffer(&input, inputBuff, input_size) == URC_FAIL)return result;
   while (bitPop (&input, &temp, sizeof (char))==URC_SUCCESS)
      {
         outputBuff->connectedOnes = (temp==0)?0: outputBuff->connectedOnes+1;
         if (bitPush (outputBuff, temp)== URC_FAIL)return result;
      }
   return URC_SUCCESS;
}

static UnivRetCode initBuffer(buffer * input, char * buff, unsigned int size)
{
   if (input == NULL || buff == NULL ||size == 0) return URC_FAIL;
   input->buff = buff;
   input->buff_size = size;
   input->byte_pos = 0;
   input->index = 0;
   input->connectedOnes=0;
   return URC_SUCCESS;
}

static UnivRetCode bitPop (buffer* buff, char * out, unsigned int size)
{
   UnivRetCode result = URC_FAIL;
   char temp  = 0;
   if (size == 0||out ==NULL ||buff==NULL)
      {
         return result;
      }
   if (buff->index>=buff->buff_size)
      {
         return result;
      }
   temp = buff->buff[buff->index];
   *out = (temp& (MSB_bit_mask>>buff->byte_pos++))?1:0;

   if (buff->byte_pos%8==0)
      {
         ++buff->index;
         buff->byte_pos = 0;
      }
   return URC_SUCCESS;
}

static UnivRetCode bitPush (buffer* buff, char in)
{
   UnivRetCode result = URC_FAIL;
   unsigned int  temp; // Use unsigned int due to left shifting later on
   if (buff==NULL)
      {
         return result;
      }
   if (buff->index>=buff->buff_size)
      {
         return result;
      }
   temp = (in == 0)?0:MSB_bit_mask;
   buff->buff[buff->index] = (buff->buff[buff->index]| (temp>>buff->byte_pos++));
   if (buff->byte_pos%8==0)
      {
         ++buff->index;
         buff->byte_pos = 0;
      }
   return URC_SUCCESS;
}
