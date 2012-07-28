#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "Protocols.h"
#include "CuTest.h"

void TestBuildPacket(CuTest* tc)
{
   rawPacket input;
   CuAssertTrue(tc, test_buildPacket(&input)==URC_SUCCESS);
}

void TestPasses(CuTest* tc)
{
   CuAssert(tc, "test should pass", 1 == 0 + 1);
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
