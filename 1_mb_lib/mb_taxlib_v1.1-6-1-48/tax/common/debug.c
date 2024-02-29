/*****************************************************************************
File name: debug.c
Description: �������е��Ժ������
Author: Zako
Version: 1.0
Date: 2020.06
History:
*****************************************************************************/
#include "debug.h"
// clang-format off
/**************************��������� ����Ubuntu20.04 x64**********************************
1)libusb1.0����
====PC
apt-get install libusb-1.0-0-dev

2)�찲openssl1.11 ����debug
====PC
./config --prefix=/opt/mengbai_dev/tassl111bd --debug shared
====Device
#https://www.cxyzjd.com/article/qq_38399914/111823907
./config --prefix=/opt/tassl111bd-arm --debug shared
#config�ű�437�����ң����GUESSOS��read GUESSOS
# GUESSOS="armv7-genneric-linux2"
#echo Operating system: $GUESSOS
./config no-asm shared host=arm-linux CROSS_COMPILE=/opt/arm/bin/arm-linux-gnueabi-
--prefix=/opt/tassl111bd-arm make && make install

3)libiconv����
./configure CC=/opt/arm/bin/arm-linux-gnueabi-gcc --prefix=/opt/libiconv --host=arm-linux
--enable-shared
make && make install

4)mpfr���룬��Ҫ����4.0�汾��ע�����--enable-thread-safe������������̴߳���
====PC
apt-get install libmpfr-dev
====Device
./configure CC=/opt/arm/bin/arm-linux-gnueabi-gcc --host=arm-linux --enable-thread-safe
--prefix=/opt/mpfr --enable-shared
--with-gmp-include=/opt/arm/lib/gcc/arm-linux-gnueabi/4.6.3/include/ --with-gmp-lib=/opt/arm/lib

5)libevent���룬�����opensslģ������һ�����
====PC �򵥲��Կ���ֱ�Ӱ�װ
apt-get install libevent-dev
������Ե��Ա�����ͬTaOpensslģ��һͬ����
./configure --enable-debug-mode --disable-static --enable-thread-support CFLAGS=-ggdb3 CPPFLAGS=-ggdb3 --prefix=/opt/libevent/ CFLAGS="-I/opt/mengbai_dev/tassl111bd/include" LDFLAGS="-L/opt/mengbai_dev/tassl111bd/lib -lssl -lcrypto"
====arm������
./configure --disable-static --prefix=/opt/libevent/ --host=arm-none-linux-gnueabi
CC=/opt/arm/bin/arm-linux-gnueabi-gcc CXX=/opt/arm/bin/arm-linux-gnueabi-c++
CPPFLAGS="-I/opt/arm/arm-linux-gnueabi/libc/usr/include" LDFLAGS="-L/opt/arm/lib -lssl -lcrypto"

6)Զ����άTCL�ű��Զ�������
sudo apt-get install tcl tk expect

***************************************************************************/
// clang-format on

int _WriteHexToDebugFile(char *szName, uint8 *pBuff, int nBuffLen)
{
#ifdef NEED_FP_DATA
	char path_name[256] = {0};
	sprintf(path_name, "/mnt/masstore/%s", szName);
	write_file(path_name, (char *)pBuff, nBuffLen);
#endif
    return 0;
}

int _ReadHexFromDebugFile(char *szName, uint8 *pBuff, int nBuffLen)
{
#ifdef _CONSOLE
    char szPathName[256];
    sprintf(szPathName, "/mnt/hgfs/vtmp/%s", szName);
    FILE *fp = fopen(szPathName, "r");
    if (fp == NULL) {
        sprintf(szPathName, "/tmp/%s", szName);
        if ((fp = fopen(szPathName, "r")) == NULL) {
            _WriteLog(LL_DEBUG, "[Debug] Write hex to %s failed!", szPathName);
            return -1;
        }
    }
    struct stat statbuf;
    stat(szPathName, &statbuf);
    int size = statbuf.st_size;
    if (size > nBuffLen) {
        fclose(fp);
        return -2;
    }

    int nRet = fread(pBuff, 1, size, fp);
    fflush(fp);
    fclose(fp);
    if (nRet != size)
        return -3;
    return size;
#else
    return 0;
#endif
}

int _PrintfHexBuf(uint8 *pBuff, int nBuffLen)
{
    int i = 0;
    for (i = 0; i < nBuffLen; i++) {
        printf("0x%02x, ", pBuff[i]);
        fflush(stdout);
    }
    printf("\r\n");
    fflush(stdout);
    return 0;
}

//�������nBuffLen == 0�Զ�����malloc�ڴ棬��Ҫ�ֶ�free�ڴ�
//�����Ϊ��������ȡnBuffLen����*pBuff�ڴ�,ע������Ļ�������С���ֵΪnBuffLen
//
//����ֵ�������������ֽ���
int ReadStrFile(char *szPath, void **pBuff, int nBuffLen)
{
    int nRet = ReadBinFile(szPath, pBuff, nBuffLen);
    if (nRet < 0)
        return nRet;
    char *pStr = NULL;
    if (nBuffLen == 0) {  // maloc
        pStr = *pBuff;
    } else  // arraby
        pStr = (char *)pBuff;
    if (CheckStrEncoding(pStr)) {
        if (nBuffLen == 0) {  // maloc
            free(pStr);
        }
        return -10;
    }
    return nRet;
}

//�������nBuffLen == 0�Զ�����malloc�ڴ棬��Ҫ�ֶ�free�ڴ�
//�����Ϊ��������ȡnBuffLen����*pBuff�ڴ�,ע������Ļ�������С���ֵΪnBuffLen
//
//����ֵ�������������ֽ���
int ReadBinFile(char *szPath, void **pBuff, int nBuffLen)
{
    uint8 *pDst = NULL;
    struct stat statbuf;
    int nFileSize = 0;
    int nRet = stat(szPath, &statbuf);
    if (nRet < 0)
        return -1;
    nFileSize = statbuf.st_size;
    int nReadMax = 0;
    uint8 bDynamicbuff = 0;
    nRet = -3;
    if (nBuffLen == 0) {
        bDynamicbuff = 1;
        pDst = malloc(nFileSize);
        if (!pDst)
            return -1;
        nReadMax = nFileSize;
    } else {
        if (nFileSize > nBuffLen)
            return -2;

        nReadMax = nFileSize;
        pDst = (uint8 *)pBuff;  //��̬�����һ���׵�ַ���������ַ
    }
    FILE *fp = fopen(szPath, "r");
    if (fp == NULL)
        goto ReadFileFromPath_Finish;
    int nByteIO = fread(pDst, 1, nReadMax, fp);
    fclose(fp);
    // 4096ʱ�п���ʱ�����С��ʵ���ļ�����4096��С�����ж�
    if (nReadMax != 4096) {
        //������ļ����жϴ�С�������豸��Ķ���4096��С
        if (nByteIO != nReadMax)
            goto ReadFileFromPath_Finish;
    }
    nRet = 0;
ReadFileFromPath_Finish:
    if (nRet) {
        if (bDynamicbuff)
            free(pDst);
    } else {
        if (bDynamicbuff)
            *pBuff = pDst;
        nRet = nByteIO;
    }
    return nRet;
}

int WriteBinFile(char *szPathName, uint8 *pBuff, int nBuffLen)
{
    FILE *fp = fopen(szPathName, "wb+");
    if (fp == NULL) {
        _WriteLog(LL_DEBUG, "Write to %s failed!, errno:%d", szPathName, errno);
        return -1;
    }
    int nByteIO = fwrite(pBuff, 1, nBuffLen, fp);
    fflush(fp);
    fclose(fp);
    if (nByteIO != nBuffLen)
        return -2;
    return 0;
}

// LoadDevinfo�У����������̵���ʱ���Զ����ػ�ȡ���ߵ�����Ϣ���������̵���
int LoadNoDeviceJson(HDEV hDev)
{
    char pFileBuf[1024] = "";
    if (_ReadHexFromDebugFile("devcfg.json", (uint8 *)pFileBuf, 1024) < 0)
        return -1;
    cJSON *root = NULL, *jTmp = NULL;
    if ((root = cJSON_Parse((const char *)pFileBuf)) == NULL)
        return -2;
    //��һЩ��Ҫ���Բ���
    PTData pTD = (PTData)(hDev->hUSB->pTestData);
    pTD->hDev = hDev;
    while (1) {
        // load static
        if (!(jTmp = cJSON_GetObjectItem(root, "szDeviceTime")))
            break;
        strcpy(hDev->szDeviceTime, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szDeviceID")))
            break;
        strcpy(hDev->szDeviceID, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szCommonTaxID")))
            break;
        strcpy(hDev->szCommonTaxID, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szCompressTaxID")))
            break;
        strcpy(hDev->szCompressTaxID, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "sz9ByteHashTaxID")))
            break;
        strcpy(hDev->sz9ByteHashTaxID, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szCompanyName")))
            break;
        strcpy(hDev->szCompanyName, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szRegCode")))
            break;
        strcpy(hDev->szRegCode, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szDriverVersion")))
            break;
        strcpy(hDev->szDriverVersion, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szJMBBH")))
            break;
        strcpy(hDev->szJMBBH, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szTaxAuthorityCodeEx")))
            break;
        strcpy(hDev->szTaxAuthorityCodeEx, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szTaxAuthorityName")))
            break;
        strcpy(hDev->szTaxAuthorityName, jTmp->valuestring);

        if (!(jTmp = cJSON_GetObjectItem(root, "abSupportInvtype")))
            break;
        strcpy((char *)hDev->abSupportInvtype, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "uICCardNo")))
            break;
        hDev->uICCardNo = jTmp->valueint;
        if (!(jTmp = cJSON_GetObjectItem(root, "bTrainFPDiskType")))
            break;
        hDev->bTrainFPDiskType = jTmp->valueint;
        if (!(jTmp = cJSON_GetObjectItem(root, "bDeviceType")))
            break;
        hDev->bDeviceType = jTmp->valueint;
        if (!(jTmp = cJSON_GetObjectItem(root, "bOperateInvType")))
            break;
        hDev->bOperateInvType = jTmp->valueint;
        if (!(jTmp = cJSON_GetObjectItem(root, "nDriverVersionNum")))
            break;
        hDev->nDriverVersionNum = jTmp->valueint;
        if ((jTmp = cJSON_GetObjectItem(root, "B64CurInvoTempb")))
            Base64_Decode(jTmp->valuestring, strlen(jTmp->valuestring),
                          (char *)hDev->abInvTemplate);
        if (!(jTmp = cJSON_GetObjectItem(root, "szDeviceEffectDate")))
            break;
        strcpy(hDev->szDeviceEffectDate, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szQmcs")))
            break;
        strcpy(hDev->szSignParameter, jTmp->valuestring);

        // load dynamic
        if ((jTmp = cJSON_GetObjectItem(root, "szPubServer")))
            strcpy(hDev->szPubServer, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szFPDM")))
            break;
        strcpy(pTD->szFPDM, jTmp->valuestring);
        if (!(jTmp = cJSON_GetObjectItem(root, "szFPHM")))
            break;
        strcpy(pTD->szFPHM, jTmp->valuestring);

        break;
    }
    cJSON_Delete(root);
    return 0;
}

// LoadDevinfo�������ֶ����øú���,��ȡ���ߵ�����Ϣ�������ַ�����������ڣ��������̵���
int SaveNoDeviceJson(HDEV hDev, uint8 bCommonFplx)
{
    char szSavePath[] = "/tmp/devcfg.json";
    // static data
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "szDeviceTime", hDev->szDeviceTime);
    cJSON_AddStringToObject(json, "szDeviceID", hDev->szDeviceID);
    cJSON_AddStringToObject(json, "szCommonTaxID", hDev->szCommonTaxID);
    cJSON_AddStringToObject(json, "szCompressTaxID", hDev->szCompressTaxID);
    cJSON_AddStringToObject(json, "sz9ByteHashTaxID", hDev->sz9ByteHashTaxID);
    cJSON_AddStringToObject(json, "szCompanyName", hDev->szCompanyName);
    cJSON_AddStringToObject(json, "szRegCode", hDev->szRegCode);
    cJSON_AddStringToObject(json, "szDriverVersion", hDev->szDriverVersion);
    cJSON_AddStringToObject(json, "szJMBBH", hDev->szJMBBH);
    cJSON_AddStringToObject(json, "szTaxAuthorityCodeEx", hDev->szTaxAuthorityCodeEx);
    cJSON_AddStringToObject(json, "szTaxAuthorityName", hDev->szTaxAuthorityName);
    cJSON_AddStringToObject(json, "abSupportInvtype", (char *)(hDev->abSupportInvtype));
    cJSON_AddNumberToObject(json, "uICCardNo", hDev->uICCardNo);
    cJSON_AddNumberToObject(json, "bTrainFPDiskType", hDev->bTrainFPDiskType);
    cJSON_AddNumberToObject(json, "bDeviceType", hDev->bDeviceType);
    cJSON_AddNumberToObject(json, "bOperateInvType", bCommonFplx);
    cJSON_AddStringToObject(json, "szDeviceEffectDate", hDev->szDeviceEffectDate);
    cJSON_AddStringToObject(json, "szQmcs", hDev->szSignParameter);
    // dynamic data
    char szFPDM[24] = "", szFPHM[24] = "", szFPZZHM[24] = "";
    uint8 abBuf[256] = "";
    uint8 abBuf2[256] = "";
    int nChildRet = -1;
    if (hDev->bDeviceType == DEVICE_AISINO) {
        uint8 bFplxAisno = 0;
        CommonFPLX2AisinoFPLX(bCommonFplx, &bFplxAisno);
        nChildRet = AisinoGetCurrentInvCode(hDev->hUSB, bFplxAisno, szFPHM, (char *)abBuf, szFPDM);
    } else if (hDev->bDeviceType == DEVICE_NISEC) {
        while (1) {
            //��ȡ����ģ��,kp��������Ҫʹ��
            if (NisecEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
                break;
            NisecSelectInvoiceType(hDev, bCommonFplx, SELECT_OPTION_TEMPLATE);
            Base64_Encode((char *)hDev->abInvTemplate, sizeof(hDev->abInvTemplate), (char *)abBuf2);
            if ((nChildRet = CommonGetCurrentInvCode(hDev, szFPDM, szFPHM, szFPZZHM)) < 0)
                break;  //���ûƱ�ˣ�����ʧ��
            if (NisecEntry(hDev->hUSB, NOP_CLOSE) < 0)
                break;
            nChildRet = 0;
            break;
        }
    } else if (hDev->bDeviceType == DEVICE_CNTAX) {
        while (1) {
            //��ȡ����ģ��,kp��������Ҫʹ��
            if (CntaxEntry(hDev->hUSB, NOP_OPEN_FULL) < 0)
                break;
            CntaxSelectInvoiceType(hDev, bCommonFplx, SELECT_OPTION_TEMPLATE);
            Base64_Encode((char *)hDev->abInvTemplate, sizeof(hDev->abInvTemplate), (char *)abBuf2);
            if ((nChildRet = CommonGetCurrentInvCode(hDev, szFPDM, szFPHM, szFPZZHM)) < 0)
                break;  //���ûƱ�ˣ�����ʧ��
            if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
                break;
            nChildRet = 0;
            break;
        }
    }
    if (nChildRet < 0)
        _WriteLog(LL_DEBUG, "SaveNoDeviceJson get dynamic data failed");

    cJSON_AddStringToObject(json, "szFPDM", szFPDM);
    cJSON_AddStringToObject(json, "szFPHM", szFPHM);
    cJSON_AddStringToObject(json, "B64CurInvoTempb", (char *)abBuf2);
    cJSON_AddStringToObject(json, "szPubServer", hDev->szPubServer);
    cJSON_AddNumberToObject(json, "nDriverVersionNum", hDev->nDriverVersionNum);

    char *pJsonBuf = cJSON_Print(json);
    if (WriteBinFile(szSavePath, (uint8 *)pJsonBuf, strlen(pJsonBuf)) < 0)
        _WriteLog(LL_DEBUG, "SaveNoDeviceJson failed to %s", szSavePath);
    else
        _WriteLog(LL_DEBUG, "SaveNoDeviceJson successful to %s", szSavePath);
    free(pJsonBuf);
    cJSON_Delete(json);
    return 0;
}

void SaveFpxxToDisk(HFPXX fpxx)
{
    char *p = NULL;
    char szFile[128] = "";
    FPXX2Json(fpxx, &p);
    sprintf(szFile, "savefpxx/fp-%s-%s.json", fpxx->fpdm, fpxx->fphm);
    _WriteHexToDebugFile(szFile, (uint8 *)p, strlen(p));
    // WriteBinFile("/d/UReader/output/import.json", p, strlen(p));
    free(p);
}

void SaveFpxxByDMHM(HDEV hDev, char *szFPDM, char *szFPHM)
{
    int nRet = -1;
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    if (!fpxx)
        return;
    if (DEVICE_AISINO == hDev->bDeviceType)
        nRet = QueryInvInfo(hDev, szFPDM, szFPHM, 0, fpxx);
    else
        nRet = NisecQueryInvInfo(hDev, FPLX_COMMON_PTFP, szFPDM, szFPHM, fpxx);

    if (!nRet)  // query OK
        SaveFpxxToDisk(fpxx);
    FreeFpxx(fpxx);
}
