#ifndef AISINO_UPLOADINVOICE_H
#define AISINO_UPLOADINVOICE_H
#include "aisino_common.h"

//###########��Ʊ�ϴ��ӿ�-��ʼ
//�����ϴ�����ѯ����д,�÷������������������ĺϲ�
int InvUp_UploadAndUpdate(HDEV hDev, void *pInBuff, bool bInbufFpxxOrJson, bool bUpdateFlag);
//�ϴ�ȫ����֮һ:�ϴ���Ʊ
int InvUp_Upload(HFPXX fpxx);
//�ϴ�ȫ����֮��:��ѯ����д
int InvUp_Update(HFPXX fpxx, bool bUpdateFlag);
//###########��Ʊ�ϴ��ӿ�-����

int AisinoPreCompanyXml(HDEV hDev, char *cmp_xml);

#endif
