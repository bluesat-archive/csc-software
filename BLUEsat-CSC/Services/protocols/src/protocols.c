#include "service.h"
#include "debug.h"
#include "Protocols.h"

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


void AX25fcsCalc( char input[], unsigned int len,unsigned char *fcsByte0, unsigned char * fcsByte1);
UnivRetCode buildPacket (rawPacket * inputDetails );

#ifdef UNIT_TEST

UnivRetCode test_buildPacket (rawPacket * inputDetails)
{
   return buildPacket ( inputDetails );
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
   if (inputDetails==NULL) {return result;}
   if (inputDetails->addr == NULL ||
       inputDetails->ctrl == NULL ||
       inputDetails->fcs  == NULL ||
       inputDetails->info == NULL){return result;}
   if (inputDetails->addr_size == 0 ||
       inputDetails->ctrl_size == 0 ||
       inputDetails->fcs_size  == 0 ||
       inputDetails->info_size == 0){return result;}

   result = URC_SUCCESS;
   return result;
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
