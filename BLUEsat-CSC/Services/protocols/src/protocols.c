#include "service.h"
#include "debug.h"
#include "protocols.h"
#include "lib_string.h"

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
 #define AX25_CRC_POLYNOMIAL_FLIPED 0x8408 // AX25 crc polynomial reversed bits by bits

 #define NO_L3_PROTO        0xF0

#define MSB_bit_mask 0x80
#define PatternLimit 5

typedef enum
 {
   address = 0,
   control,
   pid,
   info,
   fcs
 }fields;

typedef enum
{
   poll = 0,
   final
}pfBit;

 typedef struct
 {
    char rec_seq:7;    //receive sequence number [bit 5 (or bit 9 for modulo 128) is the LSB]
    char pf_bit:1;     //Poll/Final bit
    char send_seq:7;   //send sequence number (bit 1 is the LSB)
    char pad:1;
 } ctrlField;


typedef struct {
    enum PROTOCOL_TO_USE_SENDING protocol;
    char * data;
    //size must be in bytes
    unsigned int size;
 } protMessage;


/*
 * AX. 25
 */


void AX25fcsCalc              ( char input[], unsigned int len,unsigned char *fcsByte0, unsigned char * fcsByte1);
UnivRetCode buildPacket       (rawPacket * inputDetails );
static UnivRetCode stuffBuf   (char * inputBuff, unsigned int input_size, buffer * outputBuff);
static UnivRetCode initBuffer (buffer * input, char * buff, unsigned int size);
static UnivRetCode bitPop     (buffer* buff, char * out, unsigned int size);
static UnivRetCode bitPush    (buffer* buff, char in);
static UnivRetCode buildLocation (LocSubField ** destBuffer, unsigned int * sizeLeft, Location * loc,
                                  MessageType msgType, LocationType locType,
                                  Bool visitedRepeater, Bool isLastRepeater);
static UnivRetCode addrBuilder (char * output, unsigned int * sizeLeft, DeliveryInfo * addrInfo);

#ifdef UNIT_TEST

UnivRetCode test_buildPacket (rawPacket * inputDetails)
{
   return buildPacket ( inputDetails );
}

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

UnivRetCode test_addrBuilder (char * output, unsigned int * sizeLeft, DeliveryInfo * addrInfo)
{
   return addrBuilder (output, sizeLeft, addrInfo);
}

#endif


#ifndef UNIT_TEST
static portTASK_FUNCTION(vProtocolsTask, pvParameters);
static TaskToken Proctocols_TaskToken;
void vProtocols_Service_Init(unsigned portBASE_TYPE uxPriority){
	vDebugPrint(Proctocols_TaskToken, "Protocols Task running \r\n", NO_INSERT, NO_INSERT, NO_INSERT);

	Proctocols_TaskToken = ActivateTask(TASK_PROTOCOLS,
			"Protcolos Task",
			SEV_TASK_TYPE,
			uxPriority,
			SERV_STACK_SIZE,
			vProtocolsTask);
	vActivateQueue(Proctocols_TaskToken, PROCTOCOLS_Q_SIZE);

}

static portTASK_FUNCTION(vProtocolsTask, pvParameters){
	(void) pvParameters;
	//UnivRetCode enResult;
	//MessagePacket incoming_packet;
	//struct protBufferItem * protMessage;
	while(1){

			//enResult = enGetRequest(Proctocols_TaskToken, &incoming_packet, portMAX_DELAY);
			//if (enResult != URC_SUCCESS) continue;

			//extract the pointer from the IPC message
			//protMessage = (struct protBufferItem *) incoming_packet.Data;

			//do the actual stuff here

			//print message
			//vDebugPrint(Proctocols_TaskToken, "Protocols Task running \r\n", NO_INSERT, NO_INSERT, NO_INSERT);

			//complete request by passing the status to the sender
			//vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
		}
}

#endif

UnivRetCode buildPacket (rawPacket * inputDetails )
{
   UnivRetCode result = URC_FAIL;
   char flag = FLAG;
   char output[MAX_PAYLOAD];
   buffer outBuff;
   if (initBuffer(&outBuff, output,MAX_PAYLOAD) == URC_FAIL)return result;
   if (inputDetails==NULL) {return result;}
   if (inputDetails->addr == NULL ||
       inputDetails->ctrl == NULL ||
       inputDetails->fcs  == NULL ||
       inputDetails->info == NULL){return result;}
   if (inputDetails->addr_size == 0 ||
       inputDetails->ctrl_size == 0 ||
       inputDetails->fcs_size  == 0 ||
       inputDetails->info_size == 0){return result;}
   stuffBuf (&flag, 1, &outBuff);
   stuffBuf (inputDetails->addr, inputDetails->addr_size, &outBuff);
   stuffBuf (inputDetails->ctrl, inputDetails->ctrl_size, &outBuff);
   stuffBuf (inputDetails->pid, inputDetails->pid_size, &outBuff);
   stuffBuf (inputDetails->info, inputDetails->info_size, &outBuff);
   stuffBuf (inputDetails->fcs, inputDetails->fcs_size, &outBuff);
   stuffBuf (&flag, 1, &outBuff);
   result = URC_SUCCESS;
   return result;
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
   if (destBuffer==NULL || sizeLeft == NULL || loc == NULL) return result;
   if (*sizeLeft< sizeof (LocSubField))return result;
   dest =  * destBuffer;

   // Populate Callsign
   memset (&dest->callSign,BLANK_SPACE,CALLSIGN_SIZE);
   memcpy (dest->callSign,loc->callSign, loc->callSignSize);

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
static UnivRetCode addrBuilder (char * output, unsigned int * sizeLeft, DeliveryInfo * addrInfo)
{
   UnivRetCode result = URC_FAIL;
   ReptLoc *temp;
   char * temp_output;
   Bool last;
   unsigned int index;
   if (output == NULL || sizeLeft == NULL || addrInfo == NULL) return result;

   temp_output = output;
   buildLocation ((LocSubField **)&temp_output, sizeLeft, &addrInfo->dest, addrInfo->type, Destination, false, false);
   buildLocation ((LocSubField **)&temp_output, sizeLeft, &addrInfo->src,  addrInfo->type, Source,      false, false);

   // Populate Repeater Fields
   if (addrInfo->repeats!=NULL)
   {
      temp = addrInfo->repeats;
      for (index = 0; index< addrInfo->totalRepeats;++index)
      {
         last = (index==( addrInfo->totalRepeats-1))?true:false;
         buildLocation ((LocSubField **)&temp_output, sizeLeft, &temp[index].loc , addrInfo->type, Repeater, temp[index].visited, last);
      }
   }
   return URC_SUCCESS;
}


/*
 * This function calculates the FCS checksum based on a MATLAB implementation
 * Which is obtained from :
 *		The Cyclic Redundancy Check (CRC) for AX.25
 *		Bill Newhall, KB2BRD
 *		billnewhall@yahoo.com
 *		Boulder, Colorado
 *
 */
void AX25fcsCalc( char input[], unsigned int len,unsigned char *fcsByte0, unsigned char * fcsByte1){
	//short should be 16bits, change data type if it isn't
	unsigned int inputbit;
	unsigned int inputbyte;
	unsigned short shiftRegister,shiftedOutBit,xorMask;

	for(inputbyte=0,inputbit=0,shiftRegister=0xFFFF; inputbyte < len;){
		shiftedOutBit = shiftRegister & 0x0001;//shift the rightmost bit out

		shiftRegister = shiftRegister>>1;//shift one bit to the right

		//translate SR=xor(SR, XORMask) and XORMask = ...
		if( (((input[inputbyte] & (0x1<<inputbit))>>inputbit) ^ shiftedOutBit)){
			xorMask = AX25_CRC_POLYNOMIAL_FLIPED;
		}
		else xorMask = 0;

		shiftRegister = shiftRegister ^ xorMask;

		inputbit++;
		if(inputbit == 8){
			inputbit=0;
			inputbyte++;
		}
	}

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
	return;
}
