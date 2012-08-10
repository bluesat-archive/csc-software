#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "protocols.h"
#include "CuTest.h"

void TestBuildPacket(CuTest* tc)
{
   rawPacket input;
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
   input.fcs=fcs;
   input.info=info;
   CuAssertTrue(tc, test_buildPacket(&input)==URC_FAIL);
   input.addr_size=28;
   input.ctrl_size=2;
   input.pid_size=1;
   input.fcs_size=2;
   input.info_size=10;
   CuAssertTrue(tc, test_buildPacket(&input)==URC_SUCCESS);

}

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
	return suite;
}
