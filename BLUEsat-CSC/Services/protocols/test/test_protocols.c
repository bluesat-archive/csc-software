#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "protocols.h"
#include "CuTest.h"

#define MAX_PACKETS_SIZE_IN_BYTES 10
#define MAX_INFO_FIELD_BYTES 10
#define AX25_BUFF_ELEMENTS 5//how many elements can the AX25 queue hold
#define AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE 0xF0

#define N_FLAGS_BETWEEN_PACKETS 10
#define BLUESAT_SAT_SSID 0x2
#define BLUESAT_GS_SSID 0x7
#define AX25_NOT_LAST_CALLSIGN 0x0
#define AX25_IS_LAST_CALLSIGN 0x1


typedef struct _AX25BufferItem{
   char array[MAX_PACKETS_SIZE_IN_BYTES];
   int arraylength;
}AX25BufferItem;

void AX25fcsCalc( char input[], int len,unsigned char *fcsByte0, unsigned char * fcsByte1);
unsigned int sendArray(char *array,int len, char * output, unsigned int output_size);
unsigned int AX25Old(AX25BufferItem buffer, char * output, unsigned int output_size);

void TestInitBuffer(CuTest* tc)
{
   char test[] = "MoooooCow";
   unsigned int length = strlen (test);
   buffer output;
   CuAssertTrue(tc, test_initBuffer(&output,test,length) == URC_SUCCESS);
   buffer expected;
   expected.buff = test;
   expected.buff_size = length;
   expected.index = 0;
   expected.byte_pos=0;
   expected.connectedOnes=0;
   CuAssertTrue(tc, memcmp(&expected, &output, sizeof (buffer)) == 0);
}

void TestBitPop(CuTest* tc)
{
   char test [] = {0x87,0x78};
   char exp [] = {1,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0};
   unsigned int length = 2;
   buffer in;
   CuAssertTrue(tc, test_initBuffer(&in,test,length) == URC_SUCCESS);
   char out;
   unsigned int index = 0;
   for (index = 0; index< 20; ++index)
      {
         if (test_bitPop (&in, &out, sizeof (char))==URC_SUCCESS)
            {
               CuAssertTrue(tc, out == exp[index]);
            }
         else break;
      }
   CuAssertTrue(tc, test_bitPop (&in, &out, sizeof (char))==URC_FAIL);
}

void TestBitPush(CuTest* tc)
{
   char exp [] = {0x87,0x78};
   char input [] = {1,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0};
   char test [2];
   unsigned int length = 2;
   memset (test, 0, 2);
   buffer in;
   CuAssertTrue(tc, test_initBuffer(&in, test,length)==URC_SUCCESS);
   unsigned int index = 0;
   for (index = 0; index< 16; ++index)
      {
         if (test_bitPush (&in,input[index])==URC_FAIL) break;
      }
   CuAssertTrue(tc, memcmp(exp,test,2)==0);
   //Test pushing in more than the buffer can hold
   CuAssertTrue(tc, test_bitPush (&in,1)==URC_FAIL);
}

void TestBitStuffer(CuTest* tc)
{
   char input []= {0xff,0xff};
   char expected [] ={0xFB,0xEF,0xA0,0,0};
   char output [5];
   buffer testBuff;
   CuAssertTrue(tc, test_initBuffer(&testBuff, output, 5)==URC_SUCCESS);
   memset (output, 0, 5);
   CuAssertTrue(tc, test_stuffBuf(input, 2, &testBuff)==URC_SUCCESS);
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);

   //Test multiple appends
   CuAssertTrue(tc, test_stuffBuf(input, 1 , &testBuff)==URC_SUCCESS);
   expected[2] = 0xBE;
   expected[3] = 0xF0;
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);
}

void TestBuildLocation (CuTest* tc)
{
   char buffer[20];
   char * loc = buffer;
   unsigned int remaining = 20;
   Location inputLoc;
   MessageType inputMsgType;
   LocationType inputLocType;
   Bool inputVisited;
   Bool inputLast;
   LocSubField expected;
   LocSubField * output;

   memset (buffer, 'a', 20);
   buffer[20]='\0';
   memcpy (&inputLoc,"abcd",4);
   inputLoc.callSignSize = 4;
   inputLoc.ssid = 2;
   inputMsgType = Command;
   inputLocType = Source;
   inputVisited = false;
   inputLast = false;
   test_buildLocation ((LocSubField **)&loc, &remaining, &inputLoc,inputMsgType, inputLocType, inputVisited, inputLast);
   // Call sing letters are all shifted over by 1 bit
   memset (&expected.callSign,BLANK_SPACE<<1,6);
   expected.callSign[0] = 'a'<<1;
   expected.callSign[1] = 'b'<<1;
   expected.callSign[2] = 'c'<<1;
   expected.callSign[3] = 'd'<<1;
   expected.res_1 = 1;
   expected.res_2 = 1;
   expected.rept = 0;
   expected.ssid = 2;
   expected.cORh = 0;
   CuAssertTrue(tc, 0== memcmp ((void*)buffer, (void*)&expected, CALLSIGN_SIZE));
   output =(LocSubField *) buffer;
   CuAssertTrue(tc,  expected.res_1==output->res_1);
   CuAssertTrue(tc,  expected.res_2==output->res_2);
   CuAssertTrue(tc,  expected.rept== output->rept);
   CuAssertTrue(tc,  expected.ssid==output->ssid);
   CuAssertTrue(tc,  expected.cORh== output->cORh);

   //Test inserting multiple fields into the buffer
   inputMsgType = Response;
   test_buildLocation ((LocSubField **)&loc, &remaining, &inputLoc,inputMsgType, inputLocType, inputVisited, inputLast);
   expected.cORh = 1;
   CuAssertTrue(tc,  expected.res_1==output[1].res_1);
   CuAssertTrue(tc,  expected.res_2==output[1].res_2);
   CuAssertTrue(tc,  expected.rept== output[1].rept);
   CuAssertTrue(tc,  expected.ssid==output[1].ssid);
   CuAssertTrue(tc,  expected.cORh== output[1].cORh);

   // Test fail due to insufficient destination buffer space
   CuAssertTrue(tc,  URC_FAIL==test_buildLocation ((LocSubField **)&loc, &remaining, &inputLoc,inputMsgType, inputLocType, inputVisited, inputLast));
}

void TestAddrBuilder (CuTest* tc)
{
   char buffer[100];
   unsigned int left = 100;
   DeliveryInfo addrs;
   LocSubField expected;
   LocSubField * output = (LocSubField *)buffer;
   memcpy (&addrs.dest.callSign,"a12b",4);
   addrs.dest.callSignSize = 4;
   addrs.dest.ssid = 0;
   memcpy (&addrs.src.callSign,"1ab2",4);
   addrs.src.callSignSize = 4;
   addrs.src.ssid = 1;
   addrs.totalRepeats = 0;
   addrs.type = Response;
   CuAssertTrue(tc,  URC_SUCCESS==test_addrBuilder (buffer, &left, &addrs));
   memset (expected.callSign,BLANK_SPACE<<1,6);
   expected.callSign[0] = 'a'<<1;
   expected.callSign[1] = '1'<<1;
   expected.callSign[2] = '2'<<1;
   expected.callSign[3] = 'b'<<1;
   expected.res_1 = 1;
   expected.res_2 = 1;
   expected.rept = 0;
   expected.ssid = 0;
   expected.cORh = 0;
   CuAssertTrue(tc,  memcmp((void *)&output[0], (void *)&expected, sizeof (LocSubField))==0);
   memset (expected.callSign,BLANK_SPACE<<1,6);
   expected.callSign[0] = '1'<<1;
   expected.callSign[1] = 'a'<<1;
   expected.callSign[2] = 'b'<<1;
   expected.callSign[3] = '2'<<1;
   expected.res_1 = 1;
   expected.res_2 = 1;
   expected.rept = 0;
   expected.ssid = 1;
   expected.cORh = 1;
   CuAssertTrue(tc,  output[1].cORh == expected.cORh);
   CuAssertTrue(tc,  output[1].rept == expected.rept);
   CuAssertTrue(tc,  output[1].res_1 == expected.res_1);
   CuAssertTrue(tc, output[1].res_2 == expected.res_2);
   CuAssertTrue(tc,  output[1].ssid == expected.ssid);
   CuAssertTrue(tc,  memcmp((void *)output[1].callSign, (void *)expected.callSign, 6)==0);
   CuAssertTrue(tc,  memcmp((void *)&output[1], (void *)&expected, sizeof (LocSubField))==0);

}

void TestCtrlBuilder (CuTest* tc)
{
   char buffer[10];
   ControlInfo input;
   ControlFrame expected;
   ControlFrame * actual = (ControlFrame *) buffer;
   // Test I Frame
   input.sendSeqNum  = 3;
   input.recSeqNum   = 2;
   input.poll        = 1;
   input.type        = IFrame;
   CuAssertTrue(tc,  URC_SUCCESS == test_ctrlBuilder (buffer, &input));
   expected.poll        = 1;
   expected.recSeqNum   = 2;
   expected.sendSeqNum  = 3;
   expected.sFrame      = 0;
   CuAssertTrue(tc, 0 == memcmp((void *)buffer, (void*)&expected,1));

   // Test S Frame
   input.type           = SFrame;
   input.sFrOpt         = RecNotReady;
   expected.sendSeqNum  = RecNotReady;
   expected.sFrame      = 1;
   CuAssertTrue(tc, URC_SUCCESS == test_ctrlBuilder (buffer, &input));
   CuAssertTrue(tc, expected.poll       == actual->poll);
   CuAssertTrue(tc, expected.recSeqNum  == actual->recSeqNum);
   CuAssertTrue(tc, expected.sFrame     == actual->sFrame);
   CuAssertTrue(tc, expected.sendSeqNum == actual->sendSeqNum);
   CuAssertTrue(tc, 0 == memcmp((void *)buffer, (void*)&expected,1));

   // Test U Frame
   input.type           = UFrame;
   input.sFrOpt         = NoSFrameOpts;
   input.uFrOpt         = FrameReject;
   expected.recSeqNum   = 0x04;
   expected.sendSeqNum  = 0x03;
   CuAssertTrue(tc, URC_SUCCESS == test_ctrlBuilder (buffer, &input));
   CuAssertTrue(tc, expected.poll       == actual->poll);
   CuAssertTrue(tc, expected.recSeqNum  == actual->recSeqNum);
   CuAssertTrue(tc, expected.sendSeqNum == actual->sendSeqNum);
   CuAssertTrue(tc, expected.sFrame     == actual->sFrame);
   CuAssertTrue(tc, 0 == memcmp((void *)buffer, (void*)&expected,1));
}

void TestInfoBuilder(CuTest* tc)
{
   stateBlock presentState;
   rawPacket outPacket;
   char largeBuff[300];
   char smallBuff[100];
   unsigned int maxInfo = SIZE_ACT_INFO;
   CuAssertTrue(tc, test_InfoBuilder(NULL, NULL)               ==URC_FAIL);
   CuAssertTrue(tc, test_InfoBuilder(&presentState, NULL)      ==URC_FAIL);
   CuAssertTrue(tc, test_InfoBuilder(NULL, &outPacket)         ==URC_FAIL);
   CuAssertTrue(tc, test_InfoBuilder(&presentState, &outPacket)==URC_FAIL);

   presentState.src = largeBuff;
   presentState.srcSize = 300;
   presentState.nxtIndex = 0;
   presentState.packetCnt = 0;

   // Test partial data fitting in packet
   CuAssertTrue(tc, test_InfoBuilder(&presentState, &outPacket)==URC_SUCCESS);
   CuAssertTrue(tc, outPacket.info == largeBuff);
   CuAssertTrue(tc, outPacket.info_size == SIZE_ACT_INFO);
   CuAssertTrue(tc, presentState.completed == false);

   // Test final data fitting in packet
   CuAssertTrue(tc, test_InfoBuilder(&presentState, &outPacket)==URC_SUCCESS);
   CuAssertTrue(tc, outPacket.info == largeBuff+SIZE_ACT_INFO);
   CuAssertTrue(tc, outPacket.info_size == presentState.srcSize-maxInfo);
   CuAssertTrue(tc, presentState.completed == true);

   presentState.src = smallBuff;
   presentState.srcSize = 100;
   presentState.nxtIndex = 0;
   presentState.packetCnt = 0;

   // Test all data fits in packet
   CuAssertTrue(tc, test_InfoBuilder(&presentState, &outPacket)==URC_SUCCESS);
   CuAssertTrue(tc, outPacket.info == smallBuff);
   CuAssertTrue(tc, outPacket.info_size == 100);
   CuAssertTrue(tc, presentState.completed == true);
}

void TestAX25Entry (CuTest* tc)
{
   stateBlock present;
   AX25BufferItem input;
   char actual [300];
   char expected [300];
   unsigned int actual_size = 300;
   unsigned int expected_size = 300;
   unsigned int index = 0;
   protoReturn result;
   memset (actual, 0, 300);
   memcpy (input.array,"abcedfghij",10);
   input.arraylength = 10;

   //Build state block
   present.src = input.array;
   present.srcSize = 10;
   memcpy (present.route.dest.callSign,"BLUEGS",6);
   memcpy (present.route.src.callSign, "BLUSAT",6);
   present.route.dest.callSignSize = 6;
   present.route.src.callSignSize = 6;
   present.route.dest.ssid = BLUESAT_GS_SSID;
   present.route.src.ssid = BLUESAT_SAT_SSID;
   present.route.repeats  = NULL;
   present.route.totalRepeats = 0;
   present.route.type = Response;
   present.presState = stateless;
   present.pid = AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE;
   present.packetCnt = 0;
   present.nxtIndex = 0;
   present.mode = unconnected;
   present.completed = false;

   expected_size = AX25Old(input, expected, 300);
   result = ax25Entry (&present, actual, &actual_size );
   CuAssertTrue(tc, result == generationSuccess);
   printf ("\n-%d-\n",result);
   for (index = 0 ; index< 25; ++index)
      {
         printf ("0x%2x - 0x%2x\n",expected[index],actual[index]);
      }

   CuAssertTrue(tc, true == true);
}



/*
 * create a buffer with data.
 * from the buffer create the raw packet
 * after the packet has been created fun cthe fcs on both and see what the results are. by right they should match.
 *
 * */

void TestAX25FcsCalc(CuTest* tc)
{
 char buff [200];
 rawPacket packet;
 unsigned int index;
 unsigned char actchar0, actchar1, expchar0, expchar1;
 for (index = 0; index< 200;++index)
 {
       buff[index]=(index%2)?0xffff:0;
 }
 packet.addr = buff;
 packet.addr_size = 50;
 packet.ctrl = *(ControlInfo*)&buff[50];
 packet.pid = &buff[51];
 packet.pid_size = 1;
 packet.info = &buff[52];
 packet.info_size = 200-52;
 AX25fcsCalc( buff, 200, &expchar0, &expchar1);
 test_AX25fcsCalc( &packet, &actchar0, &actchar1);
 CuAssertTrue(tc, actchar0==expchar0);
 CuAssertTrue(tc, actchar1==expchar1);
}

unsigned int AX25Old(AX25BufferItem buffer, char * output, unsigned int output_size){
   int i=0, j;
   unsigned char fcsByte0,fcsByte1;
   char BufferArray[40+MAX_INFO_FIELD_BYTES];

   i=0;
   BufferArray[i++]=('B'<<1);
   BufferArray[i++]=('L'<<1);
   BufferArray[i++]=('U'<<1);
   BufferArray[i++]=('E'<<1);
   BufferArray[i++]=('G'<<1);
   BufferArray[i++]=('S'<<1);

   //SSID byte, the seventh byte
   BufferArray[i++]=(0x60+ (BLUESAT_GS_SSID<<1 )+ AX25_NOT_LAST_CALLSIGN);//0x60= 01100000 is specified by AX25

   //Callsign of the Source
   BufferArray[i++]=('B'<<1);
   BufferArray[i++]=('L'<<1);
   BufferArray[i++]=('U'<<1);
   BufferArray[i++]=('S'<<1);
   BufferArray[i++]=('A'<<1);
   BufferArray[i++]=('T'<<1);

   //SSID byte, the seventh byte
   // Its format is 0x11XXXX0 for anything but the last callsign
   // and          0x11XXXX1 for the last callsign
   //where XXXX is the SSID
   BufferArray[i++]=(0x60 + (BLUESAT_SAT_SSID << 1) +AX25_IS_LAST_CALLSIGN);//0x60= 01100000 is specified by AX25

   //There is no digipeter
   //Control Field
   BufferArray[i++]=(AX25_CONTROL_UI_INFORMATION);

   //PID Field
   BufferArray[i++]=(AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE);

   //Info Field

   // copy the array message into buffer array
   //we don't have packet splitting yet

   for(j=0;j< buffer.arraylength;j++){
      BufferArray[i++] = buffer.array[j];
   }

   //FCS Field
   //note that bit stuffing happens after FCS is inserted
   AX25fcsCalc(BufferArray,(i),&fcsByte0, &fcsByte1);
   BufferArray[i++]=(fcsByte0);
   BufferArray[i++]=(fcsByte1);


   //Flag: included in sendArray
   return sendArray(BufferArray,(i), output,output_size);//note that i is indeed the length, not i+1, since the last i++ increased it by one
}

unsigned int sendArray(char *array,int len, char * output, unsigned int output_size)
 {
   int stuff=0;
   int ibyte=0,jbyte=0,ibit=0,jbit=0;
   int flagbit=0;
   unsigned int index;
   //ibyte keey track of which byte of the input we are at
   //jbyte for which byte of the output
   //same for the bits

  // int j;

   //notes that we don't actually reverse the bytes, but just do it when figuring out when is bit stuffing needed
   //worst case scenario: it's all ones, so we get an extra bit every 5 bits, one to cover rounding, four for extra flags due to the last incomplete byte
   //char bufferArray[len+len/5+2+N_FLAGS_BETWEEN_PACKETS+N_FLAGS_BETWEEN_PACKETS];
   char bufferArray[MAX_INFO_FIELD_BYTES+MAX_INFO_FIELD_BYTES/5+2+N_FLAGS_BETWEEN_PACKETS];

   bufferArray[0]=0x0;//initialised the first byte to zero

   //all counters are initialised to 0

   /*
    * Construct the frame, except the flags at the begining and the end
    */

   while(ibyte < (len)){
      if (array[ibyte] & (0x1<<ibit) ) stuff++;
      else stuff=0;
      //write the current bit to the target array
      bufferArray[jbyte] |= (((array[ibyte]& (0x1<<ibit)) >>ibit) <<jbit);
      jbit++;
      if (stuff==5){
         //stuff a zero
         if(jbit==8){
            jbit = 1;  //deliberately skipping a bit to stuff a zero
            jbyte++;
            bufferArray[jbyte]=0x0;//initialise bytes to zero
         }
         else jbit++;
         // we don't have to actually write a zero bit since bytes are initialised to zero
         // just have to skip a bit
         stuff=0;
      }
      ibit++;
      if(ibit==8){
         ibit=0;
         ibyte++;
      }
      if(jbit==8){
         jbit=0;
         jbyte++;
         bufferArray[jbyte]=0x0;//initialise bytes to zero
      }
   }
   /*
    *
    * Put some flags after the FCS Field, ending the frame
    *
    */
   while (ibyte<(len+N_FLAGS_BETWEEN_PACKETS)){
      bufferArray[jbyte] |= (((FLAG & (0x1<< flagbit))>>flagbit )<<jbit);
      jbit++;
      flagbit++;
      if(flagbit ==8) flagbit = 0;

      if(jbit==8){

         jbit=0;
         jbyte++;
         ibyte++;
         bufferArray[jbyte]=0x0;//initialise bytes to zero
      }
   }


   /*
    *NRZI is done by the modem at the moment, we should probably move it over when we have time
    */
   index = 0;
   output[index++] = FLAG;

   //send the buffer array bytes by bytes

   for(ibyte=0;(ibyte<=jbyte);ibyte++){
      //ignore the last byte if it's empty
      if (ibyte==jbyte && bufferArray[ibyte]==0) break;
      output[index++] = bufferArray[ibyte];
   }
   return index;
}

//Original AX25 FCS Calculation function
void AX25fcsCalc( char input[], int len,unsigned char *fcsByte0, unsigned char * fcsByte1){
   //short should be 16bits, change data type if it isn't
   unsigned int inputbit;
   unsigned int inputbyte;
   char ch1,ch2,ch3;
   unsigned short shiftRegister,shiftedOutBit,xorMask;

   ch1 = len/100+'0';
   ch2 = (len%100)/10+'0';
   ch3 = len%10+'0';

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
/*-------------------------------------------------------------------------*
 * main
 *-------------------------------------------------------------------------*/

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, TestInitBuffer);
   SUITE_ADD_TEST(suite, TestBitPop);
   SUITE_ADD_TEST(suite, TestBitPush);
   SUITE_ADD_TEST(suite, TestBitStuffer);
   SUITE_ADD_TEST(suite, TestBuildLocation);
   SUITE_ADD_TEST(suite, TestAddrBuilder);
   SUITE_ADD_TEST(suite, TestCtrlBuilder);
   SUITE_ADD_TEST(suite, TestInfoBuilder);
   SUITE_ADD_TEST(suite, TestAX25FcsCalc);
  // SUITE_ADD_TEST(suite, TestAX25Entry);


	return suite;
}
