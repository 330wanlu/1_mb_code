#ifndef TAX_UNIT_TEST_H
#define TAX_UNIT_TEST_H
#include "tax_common.h"
#ifdef _CONSOLE

void Test_TAConnect(HDEV hDev);
void Test_DecodeAisinoRawInv();
int Test_DecodeNisecRawInv();
int Test_DecodeCntaxRawInv();
void Test_BenmarkAisinoMakeInv(HDEV hDev);
void Test_MpfrMultiThread();
void Test_DevicesMultiThread();
void Test_NisecBaseConv();
int Test_LibeventHttpsClient();
void Test_IPv4Private();
int Test_NisecCntaxLogEncrypt();
void Test_TAEtaxConnect(HDEV hDev);
void Test_ETaxHMacSha256();

#endif  //_DEBUG_H
#endif  // TAX_UNIT_TEST_H