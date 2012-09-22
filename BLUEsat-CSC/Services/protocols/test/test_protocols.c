#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "protocols.h"
#include "CuTest.h"

void AX25fcsCalc( char input[], int len,unsigned char *fcsByte0, unsigned char * fcsByte1);

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
   memset (&expected.callSign,0x40,6);
   memcpy (&expected.callSign,"abcd",4);
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
   memset (&expected.callSign,0x40,6);
   memcpy (&expected.callSign,"a12b",4);
   expected.res_1 = 1;
   expected.res_2 = 1;
   expected.rept = 0;
   expected.ssid = 0;
   expected.cORh = 0;
   CuAssertTrue(tc,  memcmp((void *)&output[0], (void *)&expected, sizeof (LocSubField))==0);
   memset (&expected.callSign,0x40,6);
   memcpy (&expected.callSign,"1ab2",4);
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

void TestBuildPacket(CuTest* tc)
{
 /*  rawPacket input;
   char info [10];
   char fcs [2];
   char addr [28];
   char ctrl [2];
   char pid;
   CuAssertTrue(tc, test_buildPacket(NULL)==URC_FAIL);
   CuAssertTrue(tc, test_buildPacket(&input)==URC_FAIL);
   input.addr=addr;
   input.ctrl=ctrl;
   input.pid=&pid;
   input.info=info;
   CuAssertTrue(tc, test_buildPacket(&input)==URC_FAIL);
   input.addr_size=28;
   input.info_size=10;
   CuAssertTrue(tc, test_buildPacket(&input)==URC_SUCCESS);
*/
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
   SUITE_ADD_TEST(suite, TestBuildPacket);
   SUITE_ADD_TEST(suite, TestInitBuffer);
   SUITE_ADD_TEST(suite, TestBitPop);
   SUITE_ADD_TEST(suite, TestBitPush);
   SUITE_ADD_TEST(suite, TestBitStuffer);
   SUITE_ADD_TEST(suite, TestBuildLocation);
   SUITE_ADD_TEST(suite, TestAddrBuilder);
   SUITE_ADD_TEST(suite, TestCtrlBuilder);
   SUITE_ADD_TEST(suite, TestInfoBuilder);
   SUITE_ADD_TEST(suite, TestAX25FcsCalc);
	return suite;
}
