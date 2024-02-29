#ifndef IMPORT_INV_H
#define IMPORT_INV_H
#include "common.h"

int ImportJsonFromFile(uint8 bDeviceType, uint8 *szJsonPath, HFPXX fpxx);
int FreeFpxx(HFPXX stp_Root);
HFPXX MallocFpxx();
HFPXX MallocFpxxLinkDev(HDEV hDev);
int AisinoFPLX2Str(enum enFPLX fplx, char *szRet);
int AnalyzeJsonBuff(uint8 bDeviceType, char *json_data, HFPXX stp_fpxx, char *errinfo);
int AisinoFPLXFlag2CommonFPLX(uint8 *fpzl, uint8 *fplxdm);
int JudgeDecimalPoint(char *pc_jg, int min_num, int max_num);
int JudgeTaxID(char *szTaxID);
int InsertMxxx(HFPXX stp_Root, struct Spxx *stp_InsertNode);
int CheckFpxxNode(HFPXX fpxx, HDEV hDev, char *errifno);
int CheckFpxx(struct EFpxx *fpxx);
bool CheckTzdh(char *szRedNum, char *szInvFlag);
int GetAvailImportData(cJSON *jsonInputParentNode, char *szGetNodeKey, char *szOut, int nOutMaxLen,
                       bool bCannotNull, char *json_err);
int ImportJSONToFpxx(HFPXX fpxx, HDEV hDev);
EHFPXX EMallocFpxxLinkDev(HDEV hDev);
int EInsertMxxx(EHFPXX stp_Root, struct ESpxx *stp_InsertNode);
#endif