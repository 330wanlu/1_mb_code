/*****************************************************************************
File name:   cntax_makeinvoice.c
Description: cntax�ķ�Ʊ������ش���
Author:      Zako
Version:     1.0
Date:        2021.04
History:
20210405     �������ʵ��
*****************************************************************************/
#include "cntax_makeinvoice.h"

//Ҫ��֤Dstbuff�㹻��
// nisec����%0*x��������%0*X���ַ���ǰ����ַ�����λΪ��д������������Ʊ�ַ�������Сд
int StrToCntaxTemplate(char *pDstBuf, char *abInput, int nInputLen)
{
    return StrToNisecTemplate(true, pDstBuf, abInput, nInputLen);
}

int CntaxUpdateNodeInfo(HFPXX fpxx)
{
    //����Ʊ�ֵĴ���
    if (fpxx->isMultiTax) {
        strcpy(fpxx->slv, "99.01");
    }
    if (ZYFP_CEZS == fpxx->zyfpLx) {
        strcpy(fpxx->hsjbz, "2");
    }
    return 0;
}

int MakeInvFormatStr(char *szInput, uint16 minSize, uint8 *pDstBuf)
{
    int nRetLen = 0;
    char *szNewbuf = (char *)calloc(1, minSize * 2 + 256);
    if (!szNewbuf)
        return 0;
    sprintf(szNewbuf, "%-*s", minSize, szInput);
    nRetLen = Byte2StrRaw((uint8 *)szNewbuf, strlen(szNewbuf), (char *)pDstBuf);
    free(szNewbuf);
    return nRetLen;
}

// ���ɷ�����Ŀ���嵥��Ŀ���ݣ�0�����嵥, 1���嵥
int CntaxBuildFYXMAndQDXM(uint8 bPrimaryQDorFY, int nSPXXCount, struct Spxx *headSpxx,
                          char *pDetailBuff)
{
    char szBuf[512] = "";
    int nIndex = 0;
    struct Spxx *pObjSpxx = NULL;
    if (0 == bPrimaryQDorFY) {
        pObjSpxx = headSpxx->stp_next;  //��ϸ��Ŀ
    } else if (1 == bPrimaryQDorFY)
        pObjSpxx = headSpxx;  //�嵥��Ŀ
    else
        return -2;
    uint8 bDoFYXM = 1;
    int i = 0;
    while (pObjSpxx) {
        if (0 == i) {
            int nToWriteHead = nSPXXCount;
            if (bPrimaryQDorFY && bDoFYXM)
                nToWriteHead = 1;  //�嵥��Ŀfyxmֻ��һ��'����������嵥��'
            // FYXM QDXM --head
            sprintf(pDetailBuff + nIndex, "%04x", nToWriteHead);
            nIndex += 4;
            i++;
            continue;
        }
        // No.
        sprintf(szBuf, "%d", i);
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, szBuf, strlen(szBuf));
        // fphxz
        nIndex +=
            StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->fphxz, strlen(pObjSpxx->fphxz));
        // je(no tax) nisec��100.00ת��Ϊ100,cntax��ת��
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->je, strlen(pObjSpxx->je));
        // slv
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->slv, strlen(pObjSpxx->slv));
        // se
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->se, strlen(pObjSpxx->se));
        // SPSMMC+SPMC
        sprintf(szBuf, "%s%s", pObjSpxx->spsmmc, pObjSpxx->spmc);
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, szBuf, strlen(szBuf));
        // spsm;�ò��������ҷ��������ݲ�֧��,�ٷ���Ʊһֱ����(��Ʊδ����)
        //ʵ�ʲ��Ժ�Ϊ��Ʊ��������б༭�ı�����ţ�˳��Ϊ000001\000002....063030303030=000005���ɰع�ͨ������뿪Ʊ�������
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->spsm, strlen(pObjSpxx->spsm));
        // ggxh
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->ggxh, strlen(pObjSpxx->ggxh));
        // dw
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->jldw, strlen(pObjSpxx->jldw));
        // spsl
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->sl, strlen(pObjSpxx->sl));
        // dj nisec��100.00ת��Ϊ100,cntax��ת��
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->dj, strlen(pObjSpxx->dj));
        if (!strcmp(pObjSpxx->hsjbz, "1")) {
            _WriteLog(LL_FATAL, "Cannot make invoice with hsjbz(spxx)");
            return -3;
        }
        // hsjbz
        nIndex +=
            StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->hsjbz, strlen(pObjSpxx->hsjbz));
        // spbh
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->spbh, strlen(pObjSpxx->spbh));
        // zxbm;�ò��������ҷ��������ݲ�֧�֣��ٷ�һֱ����(��ƱҲ����)��Ĭ�Ϻ���Ʒ����һ�£����ҷ�û�У��ɰع�ͨ������뿪Ʊ�������
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->zxbh, strlen(pObjSpxx->zxbh));
        // yhzcbs
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->xsyh, strlen(pObjSpxx->xsyh));
        // lslvbs
        nIndex +=
            StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->lslvbs, strlen(pObjSpxx->lslvbs));
        // ZZSTSGL
        nIndex += StrToCntaxTemplate(pDetailBuff + nIndex, pObjSpxx->yhsm, strlen(pObjSpxx->yhsm));
        // Next
        i++;
        pObjSpxx = pObjSpxx->stp_next;
        // QDXM,fyxm finish
        if (bPrimaryQDorFY && bDoFYXM) {
            i = 0;  //�嵥����FYXMheadֻ��һ�еģ�д����ɣ���ʼд��QDXM����
            bDoFYXM = 0;
        }
    }
    if (!bPrimaryQDorFY) {  //��ϸ д��FYXM��д��QDXM����
        sprintf(pDetailBuff + nIndex, "%04x", 0);
        nIndex += 4;
    }
    return nIndex;
}

//�ֶ��չ�8��16�����������������������8λ
//֮ǰ���ȷ������Ҳ��䣬����������⣬20220618��Ʊ��������²��Բ��ֽ����ͳ�Ʒ��0002�豸���������?
int PaddingOrigString(char *szInput, int nSize, int *nPaddingSize)
{
    int nNewSize = nSize;
    *nPaddingSize = 0;
    int v100 = ((nSize & 1) + (uint8)(nSize >> 1)) & 0xF;
    *nPaddingSize = 16 - v100;
    int j = 0;
    for (j = 0; j < *nPaddingSize; ++j) {
        if (j)
            strcat(szInput, "00");
        else
            strcat(szInput, "80");
        nNewSize += 2;
    }
    *nPaddingSize *= 2;
    return nNewSize - nSize;
}

int V2EncryptInvString(char *pBuf, int nTailIndex, const int nV2EncryptIndex)
{
    if ((nTailIndex - nV2EncryptIndex) % 2 == 1)
        return -1;  //������������
    int nChildRet = 0, nChildRet2 = 0, nRet = -3, nPaddingSize = 0, nTmpBufSize = 0;
    nTailIndex += PaddingOrigString(pBuf + nTailIndex, nTailIndex - nV2EncryptIndex, &nPaddingSize);
    nTmpBufSize = (nTailIndex - nV2EncryptIndex) * 3 + 1024;
    uint8 *pOrigBuf = (uint8 *)pBuf + nV2EncryptIndex;
    uint8 *pTmpBuf = (uint8 *)calloc(1, nTmpBufSize);
    if (!pTmpBuf)
        return -2;
    while (true) {
        //  _WriteHexToDebugFile("plaintextv2_cntax_my.txt", pOrigBuf, nTailIndex -
        //  nV2EncryptIndex);
        if ((nChildRet = Str2Byte(pTmpBuf, (char *)pOrigBuf, nTailIndex - nV2EncryptIndex)) <= 0)
            break;
        _WriteHexToDebugFile("plaintextv2_cntax_my.bin", pTmpBuf, nChildRet);
        if ((nChildRet2 = EncryptDecryptV2String(1, pTmpBuf, nChildRet, pOrigBuf)) <= 0)
            break;
        memset(pTmpBuf, 0, nTmpBufSize);
        if ((nChildRet = Byte2Str((char *)pTmpBuf, pOrigBuf, nChildRet2)) <= 0)
            break;
        //���ü������ܳ��Ⱥ�padding�ĳ���
        snprintf(pBuf + nV2EncryptIndex, 9, "%08x", nChildRet);         // 9-1=8
        snprintf(pBuf + nV2EncryptIndex + 8, 3, "%02x", nPaddingSize);  // 3-1=2
        strncpy(pBuf + nV2EncryptIndex + 10, (char *)pTmpBuf, nChildRet);
        // new tail
        nRet = nV2EncryptIndex + 10 + nChildRet;
        pBuf[nRet] = '\0';
        // up to upr
        mystrupr(pBuf + nV2EncryptIndex + 10);
        break;
    }
    free(pTmpBuf);
    return nRet;
}

int CntaxMakeInvHexStr(HFPXX hFpxx, char *pBuff)
{
    int nRet = -1, nIndex = 0, nChildRet = 0, nV2EncryptIndex = 0;
    char szBuf[512] = "", szFPDMHMFormater[24] = "";
    uint8 bFPDMDefaultLen = 0, bFPHMDefaultLen = 0;
    bool bUtf8Buf = true;  // 20220429֮��Ĭ������utf8��Ʊ,��������ɾ��GBK��Ʊ
    HDEV hDev = hFpxx->hDev;
    while (1) {
        // init
        if (hDev->abInvTemplate[3] != hDev->bOperateInvType ||
            hDev->abInvTemplate[3] != hFpxx->fplx)
            break;  //����Ʊ����һ��
        if ((strlen(hFpxx->fpdm) < 8) || strlen(hFpxx->fphm) < 6)
            break;
        bFPDMDefaultLen = CntaxGetInvTemplateValue(hDev->abInvTemplate, 0, 1) * 2;  // FPDM'len
        bFPHMDefaultLen = CntaxGetInvTemplateValue(hDev->abInvTemplate, 1, 1) * 2;  // FPHM'len
        hFpxx->utf8Invoice = true;  //Ĭ�ϲ����°�UTF8����
        //�ȿ�8���ֽڣ�������䷢Ʊbuf����
        sprintf(pBuff, "%08x", 0);
        nIndex += 8;
        // device time-yymmdd
        strncpy(pBuff + nIndex, hFpxx->kpsj_standard, 8);  //ĩβ����0
        nIndex += 8;
        // je
        GetFotmatJEStr(hFpxx->je, szBuf);
        if (12 != strlen(szBuf))
            break;
        sprintf(pBuff + nIndex, "%s", szBuf);
        nIndex += 12;
        // se
        GetFotmatJEStr(hFpxx->se, szBuf);
        if (12 != strlen(szBuf))
            break;
        sprintf(pBuff + nIndex, "%s", szBuf);
        nIndex += 12;
        //˰����ش���-��չ����
        nIndex += MakeInvFormatStr(hFpxx->hDev->szTaxAuthorityCodeEx, 12, (uint8 *)pBuff + nIndex);
        // xfsh
        nIndex += MakeInvFormatStr(hFpxx->xfsh, 20, (uint8 *)pBuff + nIndex);
        // gfsh
        nIndex += MakeInvFormatStr(hFpxx->gfsh, 20, (uint8 *)pBuff + nIndex);
        // xfsh2
        nIndex += MakeInvFormatStr(hFpxx->xfsh, 20, (uint8 *)pBuff + nIndex);
        //���ַ�Ʊ? ʵ�ʶ����Ʋ��Ժ���ƱΪ1����ƱΪ0
        nIndex += FillTwocharRepeat(pBuff + nIndex, hFpxx->isRed ? "01" : "00", 1);
        // device time-hhmmss
        strncpy(pBuff + nIndex, hFpxx->kpsj_standard + 8, 6);  //ĩβ����0
        nIndex += 6;
        // zfrq
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 7);
        // zfr
        nIndex += FillTwocharRepeat(pBuff + nIndex, "20", 20);
        // unknow
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 1);
        // version? ver=1 2021�����·�֮��ǿ��Ҫ��v2�汾��Ʊ
        strcpy(pBuff + nIndex, "0002");
        nIndex += 4;
        // devid
        if (12 != strlen(hFpxx->hDev->szDeviceID))
            break;
        strcpy(pBuff + nIndex, hFpxx->hDev->szDeviceID);
        nIndex += 12;
        // ykfsje �ѿ��������
        nIndex += FillTwocharRepeat(pBuff + nIndex, "00", 6);
        // qmcs static,USB IO�㱻��̬��ֵ���ǣ�Ŀǰ�����Ϊ׼0768
        //д����ֵ����δǩ��ʱ��ȡ������0000��д��ǩ�����ȡ������ֵ��ǩ�����ȣ�һ��Ϊ0164
        strcpy(pBuff + nIndex, "0768");
        nIndex += 4;
        // nExtraBufSize if not qd?
        strcpy(pBuff + nIndex, "0000");
        nIndex += 4;
        //--fpdm cntaxר�÷�Ʊ��ʽ�������ľ���6*2=12λ����nisec��һ��
        // 20221010ȷ�ϣ�cntax������ķ�Ʊ������벢����nisecһ������ת������ֱ��ƴ�ӿ�Ʊ�ַ���
        sprintf(szFPDMHMFormater, "%0*llu", bFPDMDefaultLen, atoll(hFpxx->fpdm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--fphm
        sprintf(szFPDMHMFormater, "%0*llu", bFPHMDefaultLen, atoll(hFpxx->fphm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--yfpdm
        sprintf(szFPDMHMFormater, "%0*llu", bFPDMDefaultLen, atoll(hFpxx->blueFpdm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        //--yfphm
        sprintf(szFPDMHMFormater, "%0*llu", bFPHMDefaultLen, atoll(hFpxx->blueFphm));
        strcpy(pBuff + nIndex, szFPDMHMFormater);
        nIndex += strlen(szFPDMHMFormater);
        // v2��Ʊ�Ӵ˼���
        nV2EncryptIndex = nIndex;
        // utf8flag
        if (bUtf8Buf) {
            strcat(pBuff + nIndex, "efbbbf");
            nIndex += 6;
        }
        // rednum
        memset(szBuf, 0, sizeof szBuf);
        if ((FPLX_COMMON_ZYFP == hFpxx->fplx || FPLX_COMMON_DZZP == hFpxx->fplx) && hFpxx->isRed) {
            if (strlen(hFpxx->redNum) < 15)
                break;
            strcpy(szBuf, hFpxx->redNum);
        }
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // kpjh
        sprintf(szBuf, "%d", hFpxx->kpjh);
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // tspz?
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // gfmc
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->gfmc, strlen(hFpxx->gfmc));
        // gfdzdh
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->gfdzdh, strlen(hFpxx->gfdzdh));
        // gfyhzh
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->gfyhzh, strlen(hFpxx->gfyhzh));
        // xfmc
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->xfmc, strlen(hFpxx->xfmc));
        // xfdzdh
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->xfdzdh, strlen(hFpxx->xfdzdh));
        // xfyhzh
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->xfyhzh, strlen(hFpxx->xfyhzh));
        // unknow
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // slv  ��˰��ʱǿ��Ϊ99.01
        if (!strcmp(hFpxx->slv, "0.00"))
            nIndex += StrToCntaxTemplate(pBuff + nIndex, "0", 1);  // 0˰�ʣ�ֻ��'0'
        else
            nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->slv, strlen(hFpxx->slv));
        // jshj
        memset(szBuf, 0, sizeof(szBuf));
        strcpy(szBuf, hFpxx->jshj);
        MemoryFormart(MF_TRANSLATE_ABSOLUTE, szBuf, strlen(szBuf));
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // ZYSPSMMC+ZYSPMC
        sprintf(szBuf, "%s%s", hFpxx->zyspsmmc, hFpxx->zyspmc);
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // spsm?
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // xfmc2, 20210908����0630�Ķ�ʱ��������Ʊʱ�˴�Ϊ��
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // swjgxx ˰���������
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->hDev->szTaxAuthorityName,
                                     strlen(hFpxx->hDev->szTaxAuthorityName));
        // bz
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->bz, strlen(hFpxx->bz));
        // kpr
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->kpr, strlen(hFpxx->kpr));
        // fhr
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->fhr, strlen(hFpxx->fhr));
        // skr
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->skr, strlen(hFpxx->skr));
        // qdbz
        if (!strcasecmp(hFpxx->qdbj, "N"))
            strcpy(szBuf, "0");  //���嵥
        else if (!strcasecmp(hFpxx->qdbj, "Y"))
            strcpy(szBuf, "1");  //�嵥
        else {
            _WriteLog(LL_FATAL, "Unknow input data...");
            break;
        }
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // ssyf
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->ssyf, strlen(hFpxx->ssyf));
        // bmbbbh
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->bmbbbh, strlen(hFpxx->bmbbbh));
        // hsslbs
        nIndex += StrToCntaxTemplate(pBuff + nIndex, hFpxx->hsjbz, strlen(hFpxx->hsjbz));
        // pubserver
        memset(szBuf, 0, sizeof(szBuf));
        if ((strlen(hFpxx->hDev->szPubServer)) &&
            (hFpxx->fplx == FPLX_COMMON_DZFP || hFpxx->fplx == FPLX_COMMON_DZZP))
            strcpy(szBuf, hFpxx->pubcode);
        char szDst[3][768];  //��ʹ��memset���������澯
        memset(szDst, 0, sizeof(szDst));
        GetSplitStringSimple(szBuf, ";", szDst, 3);
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szDst[0], strlen(szDst[0]));
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szDst[1], strlen(szDst[1]));
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szDst[2], strlen(szDst[2]));
        // unknow
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // unknow
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // 3% �Ż�˰�ʿ���ԭ��cntax��nisecһ������ѡ2��ֵ0x32 33;
        if (hFpxx->specialFlag == TSFP_XGM_SPECIAL_TAX)  // cntax��nisec��һ����0429רƱҲ����
        {
            if ((hFpxx->fplx == 7) || (hFpxx->fplx == 26))
            {
                strcpy(szBuf, "2");
            }
			else 
			{
	            if(hFpxx->sslkjly != 2 && hFpxx->sslkjly != 3)
	                strcpy(szBuf,"2");
	            else
	                sprintf(szBuf, "%d", hFpxx->sslkjly);			
			} 
        }
        else
            strcpy(szBuf, "0");
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // unknow
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // �����Ϣ����nisecһ��
        if (hFpxx->fplx != FPLX_COMMON_DZZP && hFpxx->fplx != FPLX_COMMON_ZYFP) {
            if (hFpxx->isRed) {  //���ԭ��
                sprintf(szBuf, "%d,%03d,%.8s,%s,%s", hFpxx->hcyy, hFpxx->blueFplx, hFpxx->blueKpsj,
                        hFpxx->blueFpdm, hFpxx->blueFphm);
            } else
                strcpy(szBuf, ",,,,");
        } else  //רƱ����
            strcpy(szBuf, "");
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // unknow
        nIndex += StrToCntaxTemplate(pBuff + nIndex, "", 0);
        // ���߰汾�� 20220401��������nisecһ��
        memset(szBuf, 0, sizeof(szBuf));
        strncpy(szBuf, hFpxx->kpsj_standard, 8);
        NisecKjbbhEncrypt((char *)defTaxAuthorityVersionCntax, hFpxx->fpdm, hFpxx->fphm,
                          hDev->szDeviceID, szBuf, (uint8 *)szBuf);
        mystrupr(szBuf);
        nIndex += StrToCntaxTemplate(pBuff + nIndex, szBuf, strlen(szBuf));
        // fyxm
        uint8 bQDorFY = 0;
        if (!strcasecmp(hFpxx->qdbj, "N"))
            bQDorFY = 0;  //���嵥
        else if (!strcasecmp(hFpxx->qdbj, "Y"))
            bQDorFY = 1;  //�嵥
        nChildRet =
            CntaxBuildFYXMAndQDXM(bQDorFY, hFpxx->spsl, hFpxx->stp_MxxxHead, pBuff + nIndex);
        if (nChildRet < 0)
            break;
        nIndex += nChildRet;
        // qtxm
        nIndex += BuildQTXM(bUtf8Buf, pBuff + nIndex, hFpxx);
		//_WriteLog(LL_FATAL, "����V2����ǰ������");
        //printf_array(pBuff, nIndex);
        // V2���ܲ���
        if ((nIndex = V2EncryptInvString(pBuff, nIndex, nV2EncryptIndex)) < 0)
            break;
        // calc bufsize,���´�С
        nChildRet = (nIndex - 8) / 2;
        myxtoa(nChildRet, szBuf, 16, 0);
        if (strlen(szBuf) > 8)
            break;
        strncpy(pBuff + (8 - strlen(szBuf)), szBuf, strlen(szBuf));
        //���usbIO 0��ȫ,ע����8���ֽڲ�����д�뷢Ʊ�ĳ������棬���㳤��ʱ��Ҫ+8
        //��ȷ�ϣ�8�ֽ�ȷʵ���ڿ�Ʊ�ַ��������ǵײ������׷��
        strcat(pBuff + nIndex, "00000000");
        nIndex += 8;
        nRet = nIndex;
        break;
    }
    return nRet;
}

//����ΪKP��Ϣ���ɵ�hexstr�����Ϊbin
int CntaxHexStr2USBRaw(char *szInOutBuf)
{
    int nChildRet = -1;
    int nInputOrigLen = strlen(szInOutBuf);
    if (nInputOrigLen < 0x68)
        return -1;
    char *szDst = (char *)calloc(1, nInputOrigLen + 1024);  // hexstr -> bin, len/2
    if (!szDst)
        return -2;
    while (1) {
        int nIndex = Str2Byte((uint8 *)szDst, szInOutBuf, nInputOrigLen);
        // qmcs����Ϊ�̶�ֵ0x200,������þ�ֵ̬Ϊ0768���豸Ring3API�ڷ�������֮ǰ��ǿ�Ƹ���Ϊ0x200;20220619ȷ����Ȼ�Ǿ�ֵ̬
        *(uint16_t *)(szDst + 0x8a) = 0x02;
        // copy to orig
        memset(szInOutBuf, 0, nInputOrigLen);  // set zero
        memcpy(szInOutBuf, szDst, nIndex);
        nChildRet = nIndex;
        break;
    }
    free(szDst);
    return nChildRet;
}

//�������������ǲµġ�����
int CntaxDecKPRep(HFPXX hFpxx, uint8 *pUSBRaw, int nUSBRawLen)
{
    uint16 usMWBytesSize = 0x30;
    char usbRawStr[256] = "";
    uint8 abBufTmp[64] = "";
    char szOutFPDM[24] = "";
    char szOutFPHM[24] = "";
    char szOutKPSJ[24] = "";
    uint8 bFPDMLen = 0, bFPHMLen = 0;
    // ZY��PT��DZʵ�ʳ��ȷ��س���64�ֽ�
    // 48�ֽ�Ϊ���ģ�5Byte FPDM, 4Byte FPHM, 7Byte KPSJ
    if (nUSBRawLen > 75 || nUSBRawLen < 55) {
        logout(INFO, "TAXLIB", "��Ʊ����", "˰�̿��߽����ѯ���س������� nRet = %d\r\n",
               nUSBRawLen);
        return -1;
    }
    Byte2Str((char *)usbRawStr, pUSBRaw, nUSBRawLen);
    usbRawStr[usMWBytesSize * 2] = '\0';
    if (CntaxMWDecAPI(hFpxx->fplx, hFpxx->isRed, hFpxx->hDev->szDeviceID,
                      hFpxx->hDev->szTaxAuthorityCodeEx, hFpxx->kpsj_standard, hFpxx->xfsh,
                      (uint8 *)usbRawStr, hFpxx->mw) < 0)
        return -2;
    Byte2Str((char *)abBufTmp, pUSBRaw + usMWBytesSize, nUSBRawLen - usMWBytesSize);
    //������ukey 032001900104 78170102 JYM:11213886965837863183
    // 00<25-52>913<8/5629/832/>146660<8/3/493416>5154296-<>9+29660069374+3/+-*744>//106-961*9+348064216-01575+061/*950
    bFPDMLen = CntaxGetInvTemplateValue(hFpxx->hDev->abInvTemplate, 0, 1) * 2;
    strncpy(szOutFPDM, (char *)abBufTmp, bFPDMLen);
    RemovePrefixTwoZero(szOutFPDM);  //������8λ����
    bFPHMLen = CntaxGetInvTemplateValue(hFpxx->hDev->abInvTemplate, 1, 1) * 2;
    strncpy(szOutFPHM, (char *)abBufTmp + bFPDMLen, bFPHMLen);
    strcpy(szOutKPSJ, (char *)abBufTmp + bFPDMLen + bFPHMLen);
    if (strcasecmp(szOutFPDM, hFpxx->fpdm) || strcasecmp(szOutFPHM, hFpxx->fphm)) {
        _WriteLog(LL_WARN,
                  "Make invoice maybe some question, orig: FPDM:%s FPHM:%s KPSJ:%s, "
                  "out:FPDM:%s FPHM:%s KPSJ:%s",
                  hFpxx->fpdm, hFpxx->fphm, hFpxx->kpsj_standard, szOutFPDM, szOutFPHM, szOutKPSJ);
    }
    //�Կ��������صĿ�Ʊʱ��Ϊ׼��������д��Ŀ�Ʊʱ��
    UpdateFpxxAllTime(szOutKPSJ, hFpxx);
    if (NisecJYMDecAPI(pUSBRaw, usMWBytesSize, hFpxx->jym) < 0)  // ok
        return -3;
    _WriteLog(LL_DEBUG, "MakeInvRet: MW:%s JYM:%s", hFpxx->mw, hFpxx->jym);
    return 0;
}

int CntaxMakeInvIO(HUSB hUSB, HFPXX hFpxx, uint8 *pBuff, int nBuffLen)
{
    int nRet = -3;
    int nChildRet = -1;
    uint8 cb[512] = {0};
#ifdef _NODEVICE_TEST
    nBuffLen = 0;
    pBuff = NULL;
    hFpxx = NULL;
    hUSB = NULL;
    //�˴���cbӦΪ���Ϳ�Ʊ������ɺ��USB��ӦIO
    //������ukey 032001900104 78170102
    unsigned char data[65] = {0xC2, 0x5D, 0x52, 0xE9, 0x13, 0xC8, 0xF5, 0x62, 0x9F, 0x83, 0x2F,
                              0xE1, 0x46, 0x66, 0x0C, 0x8F, 0x3F, 0x49, 0x34, 0x16, 0xE5, 0x15,
                              0x42, 0x96, 0xDC, 0xE9, 0xB2, 0x96, 0x60, 0x06, 0x93, 0x74, 0xB3,
                              0xFB, 0xDA, 0x74, 0x4E, 0xFF, 0x10, 0x6D, 0x96, 0x1A, 0x9B, 0x34,
                              0x80, 0x64, 0x21, 0x6D, 0x03, 0x20, 0x01, 0x90, 0x01, 0x04, 0x78,
                              0x17, 0x01, 0x02, 0x20, 0x21, 0x03, 0x10, 0x11, 0x12, 0x09};
    memcpy(cb, data, 65);
    nChildRet = 65;
    if (nChildRet) {
    }
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode��Jump really make!");
    nRet = 0;
#else
    //++debug �������Ʊ���Ƚ���
    ////_WriteLog(LL_FATAL, "��Ʊ���̻�������Ϊ��ֹʧ��Ʊ�����ѽ������տ�Ʊ�����ͣ�"
    ////                    "���ֶ��ڴ�����ȡ��ע�Ͳ����Ʊģ��");
    ////SetLastError(hFpxx->hDev->hUSB, ERR_PROGRAME_NEED_UNLOCK,
    ////             "�ѽ��õײ����տ�Ʊ������,����ϵ������Ա");
    ////logout(INFO, "TAXLIB", "��Ʊ����", "�̺�%s,�ײ㿪Ʊʧ�ܣ�%s\r\n", hFpxx->hDev->szDeviceID,
    ////       hFpxx->hDev->hUSB->errinfo);
    ////return -100;

    while (1) {
        int nCmdLen = 2;
        memcpy(cb, "\xfe\x15", nCmdLen);
        if (hFpxx->isRed) {
            nCmdLen = 3;
            memcpy(cb, "\xfe\x15\x01", nCmdLen);
        }
        if ((nChildRet = CntaxLogicIO(hUSB, cb, nCmdLen, sizeof cb, pBuff, nBuffLen)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "���һ����Ʊ�ײ����ʧ��,NisecMakeInvIO failed 1,nChildRet = %d\r\n",
                   nChildRet);
            break;
        }
        memcpy(cb, "\xfe\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 16, sizeof cb, NULL, 0)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "���һ����Ʊ�ײ����ʧ��,NisecMakeInvIO failed 2,nChildRet = %d\r\n",
                   nChildRet);
            break;
        }
        // get make result
        if ((nChildRet = CntaxDecKPRep(hFpxx, cb, nChildRet)) < 0)
            break;
        // fe0e...01,Ϊ��һ��IO������������һ��,��2��IO  fe1f.....02;
        //һ���һ��IO�϶���ȡ��ϣ���˺����ڶ�IO�����ȡ������
        nRet = 0;
        break;
    }
#endif
    return nRet;
}

int GetCryptHashSendIOBuf(const char *sSignStrGBKBuff, int nGBKBuffLen, uint8 *dataUsbIOKey,
                          uint8 *outDigest)
{
    unsigned char abStaticPrekey2[128] = {
        0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFC, 0x28, 0xE9, 0xFA, 0x9E, 0x9D, 0x9F, 0x5E, 0x34, 0x4D, 0x5A, 0x9E, 0x4B, 0xCF,
        0x65, 0x09, 0xA7, 0xF3, 0x97, 0x89, 0xF5, 0x15, 0xAB, 0x8F, 0x92, 0xDD, 0xBC, 0xBD, 0x41,
        0x4D, 0x94, 0x0E, 0x93, 0x32, 0xC4, 0xAE, 0x2C, 0x1F, 0x19, 0x81, 0x19, 0x5F, 0x99, 0x04,
        0x46, 0x6A, 0x39, 0xC9, 0x94, 0x8F, 0xE3, 0x0B, 0xBF, 0xF2, 0x66, 0x0B, 0xE1, 0x71, 0x5A,
        0x45, 0x89, 0x33, 0x4C, 0x74, 0xC7, 0xBC, 0x37, 0x36, 0xA2, 0xF4, 0xF6, 0x77, 0x9C, 0x59,
        0xBD, 0xCE, 0xE3, 0x6B, 0x69, 0x21, 0x53, 0xD0, 0xA9, 0x87, 0x7C, 0xC6, 0x2A, 0x47, 0x40,
        0x02, 0xDF, 0x32, 0xE5, 0x21, 0x39, 0xF0, 0xA0};

    int index = 0;
    uint8 abBuf[0xd2] = {0};
    uint8 digestTmp[32] = {0};  // SM3_DIGEST_LENGTH 32
    char *pString = (char *)abBuf + 2;
    char szConst[] = "1234567812345678";

    abBuf[0] = 0x00;
    abBuf[1] = 0x80;
    strncpy(pString, szConst, 16);
    memcpy(abBuf + 18, abStaticPrekey2, sizeof(abStaticPrekey2));
    // index = 2+16+128=146
    // sub dataUsbIOKey,[2-0x22, 0x24-0x44];
    index = 146;
    memcpy(abBuf + index, dataUsbIOKey + 2, 0x20);
    index += 0x20;
    memcpy(abBuf + index, dataUsbIOKey + 2 + 0x20 + 2, 0x20);
    index += 0x20;
    // index:146=0x92, +=0x40 ==d2(0xd2)
    CalcSM3(abBuf, sizeof(abBuf), digestTmp);
    //��������hash������sm3 update���Ǳ�׼sm3�㷨
    int nDigestSize = 32;
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit(md_ctx, EVP_sm3());
    EVP_DigestUpdate(md_ctx, digestTmp, sizeof(digestTmp));  //==32
    EVP_DigestUpdate(md_ctx, (uint8 *)sSignStrGBKBuff, nGBKBuffLen);
    EVP_DigestFinal(md_ctx, outDigest, (unsigned int *)&nDigestSize);
    EVP_MD_CTX_free(md_ctx);
    return 0;
}

// bHeadWith0x3044 = falseʱ��Ʊǩ��ʹ�ã�=trueʱ˰����֤ʱǩ������ʹ��
int CntaxGetDevice64ByteSignRepAsn1(HDEV hDev, bool bHeadWith0x3044, const char *sSignStrGBKBuff,
                                    int nGBKBuffLen, char *pRecvBuff, int nRecvBuffLen)
{
    uint8 abTmp[256] = "";
    int nIndex = 0;
    if (CntaxGetDevice64ByteSignRep(hDev, sSignStrGBKBuff, nGBKBuffLen, (char *)abTmp,
                                    sizeof(abTmp)) < 0)
        return -1;
    //������Ҫfix�� ���ص�64�ֽڣ�������Ҫ�ʵ��任�£�64�ֽڷֳ�����
    //�ٷ���ʵ��ʱ�о������⣿ż����0���������ε����ֽڣ�����Ϊ64-66�ֽڲ��ȣ�����ȡ���׼����64
    if (bHeadWith0x3044) {
        memcpy(pRecvBuff, "\x30\x44", 0x2);
        nIndex += 2;
    }
    //++todo �˴�������Ҫ���� BuildCntaxPkcs11CallbackBuf
    //�������ã���̬asn���ȣ����ǹ̶�����64���øĶ��ϴ���δ�����������⣬��ʱ����
    memcpy(pRecvBuff + nIndex, "\x02\x20", 0x2);
    nIndex += 2;
    memcpy(pRecvBuff + nIndex, abTmp + 3, 0x20);
    nIndex += 0x20;
    memcpy(pRecvBuff + nIndex, "\x02\x20", 0x2);
    nIndex += 2;
    memcpy(pRecvBuff + nIndex, abTmp + 3 + 0x20, 0x20);
    nIndex += 0x20;
    return nIndex;
}
//����Ʊǩ���ַ������͵�˰�̲���ȡ��ӦֵHash0x40=64�ֽ�,һ�㲻������ӿڣ��������Ǹ���asn1�Ľӿ�
int CntaxGetDevice64ByteSignRep(HDEV hDev, const char *sSignStrGBKBuff, int nGBKBuffLen,
                                char *pRecvBuff, int nRecvBuffLen)
{
    int nChildRet = -1;
    uint8 cb[512] = {0};
    uint8 abSM3Hash[32 + 1] = {0x81};  //���ֽ�Ϊ0x81,����20�ֽ�Ϊsha1Hash SM3_DIGEST_LENGTH 32
    if (Cntax_Crypt_Password(hDev->hUSB, hDev->szDeviceID, DEF_CERT_PASSWORD_DEFAULT, NULL) <= 0)
        return -1;
    //ˢ��IOָ�������һ����������
    memcpy(cb, "\xfe\x6f\x0d\x01", 4);
    if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 4, sizeof(cb), NULL, 0)) < 0)
        return -2;
    memcpy(cb, "\xfe\x6f\x1d\x00", 4);
    if ((nChildRet = CntaxLogicIO(hDev->hUSB, cb, 4, sizeof(cb), NULL, 0)) != 68) {
        _WriteLog(LL_FATAL, "Get device sign prekey failed");
        return -3;
    }
    //���� 0xfe6f1d...���Ի�ȡ������
    // example
    // unsigned char dataUsbIOKey[68] = {
    //     0x58, 0x20, 0x8E, 0x4F, 0xC8, 0xD4, 0x7C, 0x54, 0x62, 0xB1, 0xE3, 0xC1, 0xA2, 0xF2,
    //     0x2A, 0xB8, 0x3B, 0xB2, 0xD0, 0x1A, 0xD6, 0xF8, 0x09, 0x94, 0xD5, 0x24, 0xC7, 0x64,
    //     0x7B, 0x33, 0xD9, 0xDD, 0xB9, 0x6C, 0x59, 0x20, 0x82, 0xE9, 0x3F, 0x81, 0x38, 0x0F,
    //     0x1B, 0xB0, 0xA7, 0x61, 0x8A, 0x16, 0x9F, 0xE0, 0x46, 0x9C, 0x05, 0x34, 0xF3, 0xCB,
    //     0x84, 0x0E, 0xA9, 0x1E, 0x13, 0x69, 0x34, 0x2B, 0x29, 0x0D, 0xDD, 0xF0};
    GetCryptHashSendIOBuf(sSignStrGBKBuff, nGBKBuffLen, cb, abSM3Hash + 1);
    // buf �ճ�3���ֽ���Ϊ�˺�֮ǰ��aisino�������
    //�ڶ�������  bHashType==3 cntaxǩ��hash(SM3),==4��Ŀǰ��δʵ�ֹ���4
    // SM3_DIGEST_LENGTH 32
    if (NisecCryptHash(hDev->hUSB, 3, abSM3Hash, 32 + 1, pRecvBuff + 3, nRecvBuffLen - 3))
        return -4;
    return 0;
}

//����δ�ϴ�ͬһ�ŷ�Ʊ���Զ���ظ�ǩ���������һ��ǩ��IOΪ׼ˢ��ǩ��ֵ
//���Ǻ�nisec��һ�����ǣ�ͬһ�ŷ�Ʊ����ǩ���ǲ�һ�µ�
//��bEraseSign=trueʱ,��Ĩ��ǩ��,�����ɿ�Ʊ��������Զ�ǩ�����ڷ�Ʊ�ָ�ʱ����ǩ��
int CntaxFpxxAppendSignIO(HFPXX fpxx, bool bEraseSign)
{
    HUSB hUSB = fpxx->hDev->hUSB;
    int nChildRet = -1;
    uint8 cb[128] = {0};
    char *pSignBase64 = NULL;
    char *pIOBuf = NULL;
    char szBuf[256] = "", szBuf2[128] = "";
    if (NULL == hUSB)
        return -1;
    if (strlen(fpxx->fpdm) < 10 || strlen(fpxx->fphm) < 8 || strlen(fpxx->hDev->szDeviceTime) < 8)
        return -2;
    while (1) {
        if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0) {
            nChildRet = -3;
            break;
        }
        if (CntaxSelectInvoiceType(fpxx->hDev, fpxx->fplx, SELECT_OPTION_TEMPLATE) < 0) {
            nChildRet = -4;
            break;
        }
        if (!(pSignBase64 = GetSignBase64(fpxx))) {
            nChildRet = -5;
            break;
        }
        //�ٷ�����480==0x1e0��������ٿհף�ʵ�����Ŀհ�IO����Ҫ�෢
        int nIOBufLen = 16 + strlen(pSignBase64);
        if (!(pIOBuf = calloc(1, nIOBufLen))) {
            nChildRet = -6;
            break;
        }
        //����ȷ�Ϲ��ǵ�ǰʱ�䣬���ǿ�Ʊʱ��
        strcpy(szBuf2, fpxx->kpsj_standard);
        szBuf2[8] = '\0';

        nChildRet = strlen(pSignBase64);
        if (nChildRet > 65535) {
            nChildRet = -7;
            break;
        }
        //�ڴ�����fpdm|fphm|currentTime|sign��sSize|sign'sBase64(asn1)
        snprintf(szBuf, sizeof(szBuf), "%012llu%08llu%s%04x", atoll(fpxx->fpdm), atoll(fpxx->fphm),
                 szBuf2, nChildRet);
        if (strlen(szBuf) != 32) {
            nChildRet = -7;
            break;
        }
        Str2Byte((uint8 *)pIOBuf, szBuf, 32);

        if (!bEraseSign)
            memcpy(pIOBuf + 16, pSignBase64, strlen(pSignBase64));
        memcpy(cb, "\xfe\x21\x01", 3);
        _WriteHexToDebugFile("cntax-sign_io_my.txt", (uint8 *)pIOBuf, nIOBufLen);
        _WriteLog(LL_DEBUG, "SignedBase64:%s", pSignBase64);
        // _WriteHexToDebugFile("sign-my.txt", pSignBase64, strlen(pSignBase64));
        nChildRet = CntaxLogicIO(hUSB, cb, 3, sizeof cb, (uint8 *)pIOBuf, nIOBufLen);
        if (nChildRet < 0) {
            nChildRet = -7;
            break;
        }
        if (bEraseSign) {
            strcpy(fpxx->sign, "");
            _WriteLog(LL_DEBUG, "Invoice's sign has benn erased");
        } else {
            strcpy(fpxx->sign, pSignBase64);
            _WriteLog(LL_DEBUG, "Update invoice's sign, successful");
        }

        nChildRet = 0;
        break;
    }
    NisecEntry(hUSB, NOP_CLOSE);
    if (pSignBase64)
        free(pSignBase64);
    if (pIOBuf)
        free(pIOBuf);
    return nChildRet;
}

int CntaxCheckToReSign(HDEV hDev, uint8 bFplx, char *szFpdm, char *szFphm)
{
    HFPXX fpxx = MallocFpxxLinkDev(hDev);
    int nRet = -1;
    while (1) {
        if (CntaxQueryInvInfo(hDev, bFplx, szFpdm, szFphm, fpxx))
            break;
        if (fpxx->bIsUpload) {
            _WriteLog(LL_FATAL, "Invoice has been set upload flag, cannot resign");
            break;
        }
        //----check exist fp's sign
        if (!strlen(fpxx->sign)) {
            _WriteLog(LL_INFO, "fp's sign is empty");
            // debug-sign again
            CntaxFpxxAppendSignIO(fpxx, false);
        } else {
            // cntax�ķ�Ʊǩ����������ӣ����ÿ��ǩ������仯�����Ƕ�����ȷ��
            //�����Ҫ��֤��Ʊǩ���Ƿ���ȷ����Ҫ������ʵ��һ��API����signverity,�ýӿڶ��ҷ���˵�������岻����˲���ʵ�ָýӿ�
            //�����Ʊ��δ�ϱ���д�����Բ���ǩ���ÿ�Ʊ����������������ǩ��
            _WriteLog(LL_INFO, "fp has been signed");
            // debug-sign again
            // CntaxFpxxAppendSignIO(fpxx, true);//������ǩ
            CntaxFpxxAppendSignIO(fpxx, false);  //��������������������������ǵ����ֶ��޸�
        }
        nRet = 0;
        break;
    }
    if (!nRet)
        _WriteLog(LL_DEBUG, "Sign check,dm:%s hm:%s successful.", fpxx->fpdm, fpxx->fphm);
    else
        _WriteLog(LL_FATAL, "Sign check,dm:%s hm:%s failed.", fpxx->fpdm, fpxx->fphm);

    FreeFpxx(fpxx);
    return nRet;
}

// UK_OperKeyVersion,pBuf����ֵ32�ֽڣ�pBuf����Ϊ���ֵ������Ȼ32�ֽ�
int CntaxOperKeyVersion(HUSB hUSB, uint8 *pBuf, int nBufSize)
{
    int nRet = 0;
#ifndef _NODEVICE_TEST
    uint8 cb[256];
    int nChildRet = 0;
    do {
        memcpy(cb, "\xfe\x28\x01\x01", 4);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 4, sizeof cb, pBuf, nBufSize)) < 0)
            break;
        memcpy(cb, "\xfe\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16);
        if ((nChildRet = CntaxLogicIO(hUSB, cb, 16, sizeof cb, NULL, 0)) < 0)
            break;
        //���滹��һ��IO fe1f.....02��һ���һ��IO�Ͷ�ȡ��ȫ�ˣ��ڶ������ᱨ��
        memcpy(pBuf, cb, 0x20);
        nRet = RET_SUCCESS;
    } while (false);
#else
    unsigned char data[32] = {0x44, 0xFC, 0x61, 0x3B, 0x74, 0xE4, 0x2F, 0x83, 0xA2, 0x4D, 0x33,
                              0x2B, 0x14, 0xBC, 0x6D, 0xAB, 0xEE, 0xDB, 0xD6, 0x86, 0xBC, 0x9A,
                              0x52, 0xEE, 0x10, 0x64, 0x37, 0xFB, 0xA5, 0x75, 0x36, 0x2D};
    memcpy(pBuf, data, 0x20);
    nRet = RET_SUCCESS;
#endif
    return nRet;
}

//�߰汾�����豸�ż���
int EncryptMakeInvoiceHeadBuffer0x60Bytes(HDEV hDev, char *pBuf, int nBufSize)
{
    uint8 szBuf[256] = {0};
    int nRet = ERR_GENERIC, nChilRet = 0;
    int nTableIndex = 0;  //��ֵ��IO�����ȡ��Ŀǰ�ֵ̬
    do {
        strncpy((char *)szBuf, hDev->szDriverVersion, 4);
        int nDriverVersionNum = atoi((char *)szBuf);
        //��ֹ20220610���ٷ�Ŀǰ����0001��0002�汾����˹ٷ���==0002ʱ����
        if (nDriverVersionNum == 1) {
            //Ŀǰֻ��0001��0002�汾������δ֪�汾nDriverVersionNum=[num]����0
            nRet = RET_SUCCESS;
            break;  //�������
        } else if (nDriverVersionNum != 2) {
            nRet = ERR_PROGRAME_UNSUPPORT;
            break;  //��֧���°汾���������������
        }
        logout(INFO, "TAXLIB", "��Ʊ����", "�̺ţ�%s,˰��Ϊ�±��̼�,�����°��߼�\r\n",
               hDev->szDeviceID);
        if (nBufSize < (8 + 0xc0))
            break;
        Get32bytesOperKeyVersionBuf(hDev->szDeviceID, szBuf);
        if (CntaxOperKeyVersion(hDev->hUSB, szBuf, 0x20))
            break;
        if ((nTableIndex = GetAlgIndexFromUsbReponse(hDev->szDeviceID, szBuf)) < 0)
            break;
        //����pBufǰ8�ֽ�buf��С
        GetHeadBuffer0x60BytesFinalBuf(hDev->szDeviceID, nTableIndex, (uint8 *)pBuf + 8, szBuf);
        nChilRet = strlen((char *)szBuf);
        if (nChilRet != 0xc0)
            break;
        //����ԭ�ڴ�,��󲻼�\0
        strncpy(pBuf + 8, (char *)szBuf, nChilRet);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

//��ͨ��Ʊ�͵��ӷ�Ʊ�İ�ʽһ�£����Ե���һ�ַ�Ʊ����
int CntaxMakeInvoiceMain(HFPXX fpxx)
{
    char szBuf[512] = "";
    HDEV hDev = fpxx->hDev;
    HUSB hUSB = fpxx->hDev->hUSB;
    int nRet = -1, nBufLen = 0, nChildRet = -1;
    char *pBuff = NULL;
    // ==������鼰׼��
    if (fpxx->fplx != FPLX_COMMON_DZFP && fpxx->fplx != FPLX_COMMON_PTFP &&
        fpxx->fplx != FPLX_COMMON_ZYFP && fpxx->fplx != FPLX_COMMON_DZZP) {
        return SetLastError(hUSB, ERR_PROGRAME_UNSUPPORT, "Not support invoice type:%d",
                            fpxx->fplx);
    }
    if (!strcmp(fpxx->hsjbz, "1"))
        return SetLastError(hUSB, ERR_LOGIC_UNSUPPORT_METHOD,
                            "Rawbin cannot make invoice with hsjbz(main)");
    if ((nBufLen = CalcAllocBuffLen(fpxx)) < 0)
        return SetLastError(hUSB, ERR_BUF_SIZE, "Alloc buf failed");
    // ���㹫������ƽ̨��ȡ����
    if (strlen(hDev->szPubServer)) {
        char szPubPtlx[16];
        //�������
        Build8BytesRandomTQM(szBuf);
#ifdef _NODEVICE_TEST
        strcpy(szBuf, "A7C1G2E0");
#endif
        memset(szPubPtlx, 0, sizeof(szPubPtlx));
        strncpy(szPubPtlx, hDev->szPubServer, 2);  // copy type 2bytes
        sprintf(fpxx->pubcode, "%s;%s;%s", szPubPtlx, hDev->szPubServer + 2, szBuf);
    }
    //==��ʽ��Ʊ
    if (CntaxEntry(hUSB, NOP_OPEN_FULL) < 0)
        return SetLastError(hUSB, ERR_DEVICE_OPEN, "USB����ʧ��,������");
    while (1) {
        if (!CheckDeviceSupportInvType(hDev->abSupportInvtype, fpxx->fplx)) {
            nRet = SetLastError(hDev->hUSB, ERR_LOGIC_NOTFOUND_INVOICETYPE,
                                "Device not has this invoice-type, type:%d", fpxx->fplx);
            break;
        }
        if (!(pBuff = (char *)calloc(1, nBufLen))) {
            nRet = SetLastError(hUSB, ERR_BUF_ALLOC, "Buf alloc");
            break;
        }
        // 1-��ȡ����ģ��,kp��������Ҫʹ��
        if (CntaxSelectInvoiceType(hDev, fpxx->fplx, SELECT_OPTION_TEMPLATE) < 0) {
            nRet = SetLastError(hUSB, ERR_DEVICE_SELECTINV, "ERR_DEVICE_SELECTINV");
            break;
        }
        // 2-��ȡ��ǰ��Ʊ������룬FPDM FPHM,12λ��8λ������ǰ�油0;
        if (CommonGetCurrentInvCode(hDev, fpxx->fpdm, fpxx->fphm, fpxx->fpendhm) < 0) {
            if ((strlen(fpxx->fphm) > 0) || (strlen(fpxx->fpdm) > 0))
                nRet = SetLastError(
                    hUSB, ERR_DEVICE_CURRENT_INVOICE,
                    "��ǰ��Ʊ��������봫��ķ�Ʊ������벻һ��,���ٴ�ȷ�ϱ����ظ�����");
            else
                nRet = SetLastError(hUSB, ERR_DEVICE_CURRENT_INVOICE,
                                    "��ȡ��ǰ��Ʊ�������ʧ��,���ٴ�ȷ�ϱ����ظ�����");
            break;
        }
        // 3-��Ʊ16�����ַ�������
        if ((nChildRet = CntaxMakeInvHexStr(fpxx, pBuff)) < 0) {
            nRet =
                SetLastError(hUSB, ERR_LOGIC_BUILD_FORMATBUF, "MakeInvHexStr failed:%d", nChildRet);
            break;
        }
        //_WriteLog(LL_FATAL, "����V2���ܺ������");
        // printf_array(pBuff, nChildRet);

        _WriteLog(LL_DEBUG, "MakeInvHexStr successful.len:%d", nChildRet);
        _WriteHexToDebugFile("kpstr_cntax_my.txt", (uint8 *)pBuff, nChildRet);
        // 4--0002�汾ǰ192(0xc0)�ֽڼ���
        //++todo ��nisecһ�������ú������뵽CntaxMakeInvHexStr������ȥ
        if (EncryptMakeInvoiceHeadBuffer0x60Bytes(hDev, pBuff, nChildRet)) {
            nRet = SetLastError(hUSB, ERR_LOGIC_BUILD_FORMATBUF,
                                "EncryptMakeInvoiceHeadBuffer0x60Bytes failed:%d", nChildRet);
            break;
        }
        // 5--�ַ���ת������
        if ((nChildRet = CntaxHexStr2USBRaw(pBuff)) < 0) {
            nRet = SetLastError(hUSB, ERR_BUF_CONVERT, "HexStr2USBRaw failed:%d", nChildRet);
            break;
        }
        // 6--��ƱIO
        _WriteHexToDebugFile("usbio_cntax_my.bin", (uint8 *)pBuff, nChildRet);
        if ((nChildRet = CntaxMakeInvIO(hUSB, fpxx, (uint8 *)pBuff, nChildRet)) < 0) {
            logout(INFO, "TAXLIB", "��Ʊ����",
                   "�̺ţ�%s,�ײ㿪Ʊʧ��,result = %d,LastError��%s��\r\n", hDev->szDeviceID,
                   nChildRet, hDev->hUSB->errinfo);
            break;
        }
        nRet = RET_SUCCESS;
        break;
    }
    _WriteLog(!nRet ? LL_INFO : LL_FATAL, "Makeinvoice %s, Type:%d(%d) FPDM:%s FPHM:%s",
              !nRet ? "successful" : "failed", fpxx->fplx, fpxx->fplx_aisino, fpxx->fpdm,
              fpxx->fphm);
    if (strcmp(fpxx->fphm, fpxx->fpendhm) == 0) {
        _WriteLog(LL_WARN, "��ʹ�õ���Ʊ�����һ�ŷ�Ʊ,������ɺ���Ҫ���жϵ����");
        fpxx->need_restart = 1;
    }
    if (pBuff)
        free(pBuff);
    if (CntaxEntry(hDev->hUSB, NOP_CLOSE) < 0)
        return ERR_DEVICE_CLOSE;
    return nRet;
}
