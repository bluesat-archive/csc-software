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



/*-------------------------------------------------------------------------*
 * main
 *-------------------------------------------------------------------------*/

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, TestBuildPacket);
	return suite;
}
