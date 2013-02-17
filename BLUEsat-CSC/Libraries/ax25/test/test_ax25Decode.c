#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "lib_string.h"
#include "ax25.h"
#include "CuTest.h"


void TestDetermineDest(CuTest* tc)
{
   Location satellite;
   LocSubField input[2];
   rxPktStubs stubs;
   //Setup the destination information
   memcpy (satellite.callSign,"VK2BS",5);
   satellite.callSignSize = 5;
   //Build the location header for the ax25 packet
   memset ((void*)&input[0],0,sizeof(LocSubField));   // Zero memory slot
   memcpy (input[0].callSign,"VK2BS",5);     // Fill in the destination
   memset ((void*)&input[1],0,sizeof(LocSubField));   // Zero memory slot
   memcpy (input[1].callSign,"VK2UNS",6);     // Fill in the Source

   stubs.dest = &input[0];
   stubs.src  = &input[1];
   CuAssertTrue(tc, test_determineDest ( &stubs,  &satellite) == forUs); // Check the for us match
   memcpy (input[0].callSign,"VK2BS1",6);     // Fill in the destination
   CuAssertTrue(tc, test_determineDest ( &stubs,  &satellite) == notUs); // Check the for us match
}

/*
 * FCS Support Functions
 * */
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

static void calculateFCS (char * input, unsigned int size, unsigned char * fcs1, unsigned char * fcs2)
{
  unsigned short shiftRegister = 0xFFFF; // Initial value for shift register
  shiftRegister = fcsEngine(shiftRegister, input,size);
  shiftRegister = ~shiftRegister;
  *fcs1 = shiftRegister&0x00FF;
  *fcs2 = (shiftRegister&0xFF00)>>8;
}

typedef struct
{
   char sFlag;
   char dest[7];
   char src[7];
   char control;
   char info[21];
   char FCS1;
   char FCS2;
   char eFlag;
}testReceivedPacket;

static unsigned int buildUnnumberedPacket (char * buffer, unsigned int size)
{
   if (size< sizeof(testReceivedPacket)) return URC_FAIL;
   testReceivedPacket * out = (testReceivedPacket*) buffer;
   memset (buffer,0,size);
   out->sFlag = FLAG;
   out->eFlag = FLAG;
   memcpy (out->dest,"VK2BS",5);     // Fill in the destination
   out->dest[6] = 0x80;
   memcpy (out->src,"VK2UNS",5);     // Fill in the Source
   out->src[6] = 0x00;                // C bit alternating 0 and 1 for ax25 v2.x versions
   out->control = 0x03;
   memcpy (out->info,"Hello This is Bluesat",21);
   calculateFCS(out->dest, 36, (unsigned char *)&out->FCS1, (unsigned char *)&out->FCS2);
   return sizeof(testReceivedPacket);
}


//Test Receiving a broad cast packet
void TestAx25Receive(CuTest* tc)
{
   char buffer[100];
   char outBuffer[300];
   Location satellite;
   unsigned int packetSize = buildUnnumberedPacket(buffer, 100);
   receivedPacket out;
   memset ((char*)outBuffer, 0, 300);
   memset ((char*)&out,0,sizeof(receivedPacket));
   out.info = outBuffer;
   out.infoSize = 300;
   memcpy (satellite.callSign,"VK2BS",5);
   satellite.callSignSize = 5;




   test_ax25Receive (&out, buffer, packetSize, (const Location *) &satellite );
   CuAssertTrue(tc, 1==1); // Check the for us match

}

/*-------------------------------------------------------------------------*
 * main
 *-------------------------------------------------------------------------*/

CuSuite* CuGetSuite(void)
{
   CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, TestDetermineDest);
   SUITE_ADD_TEST(suite, TestAx25Receive);
   return suite;
}
