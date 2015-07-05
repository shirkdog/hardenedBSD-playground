/* AUTOGENERATED FILE. DO NOT EDIT. */

//=======Test Runner Used To Run Each Test Below=====
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Unity.CurrentTestName = #TestFunc; \
  Unity.CurrentTestLineNumber = TestLineNum; \
  Unity.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
      setUp(); \
      TestFunc(); \
  } \
  if (TEST_PROTECT() && !TEST_IS_IGNORED) \
  { \
    tearDown(); \
  } \
  UnityConcludeTest(); \
}

//=======Automagically Detected Files To Include=====
#include "unity.h"
#include <setjmp.h>
#include <stdio.h>

//=======External Functions This Runner Calls=====
extern void setUp(void);
extern void tearDown(void);
void resetTest(void);
extern void test_GenerateUnauthenticatedPacket(void);
extern void test_GenerateAuthenticatedPacket(void);
extern void test_OffsetCalculationPositiveOffset(void);
extern void test_OffsetCalculationNegativeOffset(void);
extern void test_HandleUnusableServer(void);
extern void test_HandleUnusablePacket(void);
extern void test_HandleServerAuthenticationFailure(void);
extern void test_HandleKodDemobilize(void);
extern void test_HandleKodRate(void);
extern void test_HandleCorrectPacket(void);


//=======Test Reset Option=====
void resetTest()
{
  tearDown();
  setUp();
}

char *progname;


//=======MAIN=====
int main(int argc, char *argv[])
{
  progname = argv[0];
  Unity.TestFile = "packetHandling.c";
  UnityBegin("packetHandling.c");
  RUN_TEST(test_GenerateUnauthenticatedPacket, 35);
  RUN_TEST(test_GenerateAuthenticatedPacket, 58);
  RUN_TEST(test_OffsetCalculationPositiveOffset, 97);
  RUN_TEST(test_OffsetCalculationNegativeOffset, 141);
  RUN_TEST(test_HandleUnusableServer, 184);
  RUN_TEST(test_HandleUnusablePacket, 195);
  RUN_TEST(test_HandleServerAuthenticationFailure, 206);
  RUN_TEST(test_HandleKodDemobilize, 217);
  RUN_TEST(test_HandleKodRate, 241);
  RUN_TEST(test_HandleCorrectPacket, 252);

  return (UnityEnd());
}
