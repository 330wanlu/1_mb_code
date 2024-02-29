#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H
#include "common.h"

#ifdef _CONSOLE
int DetectDeviceInit(HDEV hDev, HUSB hUSB, uint8 bAuthDev, int nBusID, int nDevID);
int ParamentInit(int argc, const char *argv[], uint8 *bDebugMode, uint8 *bFuncNum, int *nBusID,
                 int *nDevID, char *szParament1, char *szParament2, char *szParament3);
int FunctionCall(HDEV hDev, uint8 bFuncNum, char *szParament1, char *szParament2,
                 char *szParament3);
int ConsoleClose(int nDeviceType, HUSB hUsb);
#endif

#endif