#ifndef AISINO_UPLOADINVOICE_H
#define AISINO_UPLOADINVOICE_H
#include "aisino_common.h"

//###########发票上传接口-开始
//包含上传、查询、反写,该方法是如下两个方法的合并
int InvUp_UploadAndUpdate(HDEV hDev, void *pInBuff, bool bInbufFpxxOrJson, bool bUpdateFlag);
//上传全过程之一:上传发票
int InvUp_Upload(HFPXX fpxx);
//上传全过程之二:查询、反写
int InvUp_Update(HFPXX fpxx, bool bUpdateFlag);
//###########发票上传接口-结束

int AisinoPreCompanyXml(HDEV hDev, char *cmp_xml);

#endif
