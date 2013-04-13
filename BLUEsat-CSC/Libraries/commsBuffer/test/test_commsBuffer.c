#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"
#include "commsBuffer.h"

void TestInitBuffer(CuTest* tc)
{
   char test[] = "MoooooCow";
   unsigned int length = strlen (test);
   buffer output;
   CuAssertTrue(tc, initBuffer(&output,test,length) == URC_SUCCESS);
   buffer expected;
   expected.buff = test;
   expected.buff_size = length;
   expected.index = 0;
   expected.byte_pos=0;
   expected.connectedOnes=0;
   CuAssertTrue(tc, memcmp(&expected, &output, sizeof (buffer)) == 0);
}

void TestBitPopLSBtoMSB(CuTest* tc)
{
   char test [] = {0x87,0x78};
   char exp [] = {1,1,1,0,0,0,0,1,0,0,0,1,1,1,1,0};
   unsigned int length = 2;
   buffer in;
   CuAssertTrue(tc, initBuffer(&in,test,length) == URC_SUCCESS);
   char out;
   unsigned int index = 0;
   for (index = 0; index< 20; ++index)
      {
         if (bitPopLSBtoMSB (&in, &out, sizeof (char))==URC_SUCCESS)
            {
             CuAssertTrue(tc, out == exp[index]);
            }
         else break;
      }
   CuAssertTrue(tc, bitPopLSBtoMSB (&in, &out, sizeof (char))==URC_FAIL);
}

void TestBitPopMSBtoLSB(CuTest* tc)
{
   char test [] = {0x87,0x78};
   char exp [] = {1,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0};
   unsigned int length = 2;
   buffer in;
   CuAssertTrue(tc, initBuffer(&in,test,length) == URC_SUCCESS);
   char out;
   unsigned int index = 0;
   for (index = 0; index< 20; ++index)
      {
         if (bitPopMSBtoLSB (&in, &out, sizeof (char))==URC_SUCCESS)
            {
             CuAssertTrue(tc, out == exp[index]);
            }
         else break;
      }
   CuAssertTrue(tc, bitPopMSBtoLSB (&in, &out, sizeof (char))==URC_FAIL);
}

void TestBitPushLSBtoMSB(CuTest* tc)
{
   char exp [] = {0xe1,0x1e};
   char input [] = {1,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0};
   char test [2];
   unsigned int length = 2;
   memset (test, 0, 2);
   buffer in;
   CuAssertTrue(tc, initBuffer(&in, test,length)==URC_SUCCESS);
   unsigned int index = 0;
   for (index = 0; index< 16; ++index)
      {
         if (bitPushLSBtoMSB (&in,input[index])==URC_FAIL) break;
      }
   CuAssertTrue(tc, memcmp(exp,test,2)==0);
   //Test pushing in more than the buffer can hold
   CuAssertTrue(tc, bitPushLSBtoMSB (&in,1)==URC_FAIL);
}

void TestBitPushMSBtoLSB(CuTest* tc)
{
   char exp [] = {0x71,0x17};
   char input [] = {0,1,1,1,0,0,0,1,0,0,0,1,0,1,1,1};
   char test [2];
   unsigned int length = 2;
   memset (test, 0, 2);
   buffer in;
   CuAssertTrue(tc, initBuffer(&in, test,length)==URC_SUCCESS);
   unsigned int index = 0;
   for (index = 0; index< 16; ++index)
      {
         if (bitPushMSBtoLSB (&in,input[index])==URC_FAIL) break;
      }
   CuAssertTrue(tc, memcmp(exp,test,2)==0);
   //Test pushing in more than the buffer can hold
   CuAssertTrue(tc, bitPushMSBtoLSB (&in,1)==URC_FAIL);
}


void TestStuffBufLSBtoMSB(CuTest* tc)
{
   char input []= {0xff,0xff};
   char expected [] ={0xDF,0xF7,0x05,0,0};
   char output [5];
   buffer testBuff;
   CuAssertTrue(tc, initBuffer(&testBuff, output, 5)==URC_SUCCESS);
   memset (output, 0, 5);

   CuAssertTrue(tc, stuffBufLSBtoMSB(input, 2, &testBuff)==URC_SUCCESS);
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);

   //Test multiple appends
   CuAssertTrue(tc, stuffBufLSBtoMSB(input, 1 , &testBuff)==URC_SUCCESS);
   expected[2] = 0x7D;
   expected[3] = 0x0F;
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);
}

void TestStuffBufMSBtoLSB(CuTest* tc)
{
   char input []= {0xff,0xff};
   char expected [] ={0xFB,0xEF,0xA0,0,0};
   char output [5];
   buffer testBuff;
   CuAssertTrue(tc, initBuffer(&testBuff, output, 5)==URC_SUCCESS);
   memset (output, 0, 5);

   CuAssertTrue(tc, stuffBufMSBtoLSB(input, 2, &testBuff)==URC_SUCCESS);
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);

   //Test multiple appends
   CuAssertTrue(tc, stuffBufMSBtoLSB(input, 1 , &testBuff)==URC_SUCCESS);
   expected[2] = 0xBE;
   expected[3] = 0xF0;
   CuAssertTrue(tc, memcmp(expected,output, 5)==0);
}

/*-------------------------------------------------------------------------*
 * main
 *-------------------------------------------------------------------------*/

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, TestInitBuffer);
   SUITE_ADD_TEST(suite, TestBitPopLSBtoMSB);
   SUITE_ADD_TEST(suite, TestBitPopMSBtoLSB);
   SUITE_ADD_TEST(suite, TestBitPushLSBtoMSB);
   SUITE_ADD_TEST(suite, TestBitPushMSBtoLSB);
   SUITE_ADD_TEST(suite, TestStuffBufLSBtoMSB);
   SUITE_ADD_TEST(suite, TestStuffBufMSBtoLSB);
	return suite;
}
