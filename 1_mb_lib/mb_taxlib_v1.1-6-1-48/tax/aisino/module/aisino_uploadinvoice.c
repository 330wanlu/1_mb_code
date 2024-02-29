/*****************************************************************************
File name:   aisino_uploadinvoice.c
Description: ���ڰ���ŵ�̵ķ�Ʊ�ϴ�ҵ��
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     �������ʵ��
*****************************************************************************/
#include "aisino_uploadinvoice.h"


int ReadCompanyInfo(HUSB hUSB, char *szOutInfo)
{
#ifdef _NODEVICE_TEST
    strcpy(szOutInfo,
           "ifw885PpNY2noMQ6IkyE1YBYyzVKNvzaFpY8NILQ15Uf1tD5ugn3UiCLznnOI+"
           "xcKV9Pc0l2o5k4nDD6PekoCucXkFvvrXB8K2DaxDIbQNL/"
           "jVjpO34MwpJqDRLa0lVT0Kb1gRoVD057hhuNXm7hZ5qBTcLjbqZrw2UV6b9bvXBwwOEFtpuq8NuwyU"
           "RnOSWa1ox+CWCxv4UDw6RddaSQuQ==");
    return 0;
#endif
    uint8 cb[2048] = {0};  // 768̫С������˰��������20220510��
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x54\x00\x00\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 128)
        return -1;
    return Base64_Encode((const char *)cb, nRet - 1, szOutInfo);
}

//�ϴ�xml������, pc_xml > 768
int PreDownloadFPXML(HFPXX fpxx, char *pc_xml)
{
    HDEV hDev = (HDEV)fpxx->hDev;
    char szBuf[256] = "";
    int nRet = -1;
    mxml_node_t *root = mxmlNewXML("1.0");
    mxml_node_t *xml_root = mxmlNewElement(root, "FPXT");
    if (!xml_root)
        goto PreDownloadFPXML_Finish;
    xml_root = mxmlNewElement(xml_root, "INPUT");
    mxmlNewText(mxmlNewElement(xml_root, "NSRSBH"), 0, hDev->szCommonTaxID);
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(mxmlNewElement(xml_root, "KPJH"), 0, szBuf);
    mxmlNewText(mxmlNewElement(xml_root, "SBBH"), 0, hDev->szDeviceID);
    mxmlNewText(mxmlNewElement(xml_root, "FXSH"), 0, hDev->szCompressTaxID);
    mxmlNewText(mxmlNewElement(xml_root, "SLXLH"), 0, fpxx->slxlh);
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    strcpy(pc_xml, ptr);
    free(ptr);
    nRet = 0;
PreDownloadFPXML_Finish:
    mxmlDelete(root);
    return nRet;
}

//�ϴ�xml������, pc_xml > 1024
int PreDownloadDZFPXML(HFPXX fpxx, char *szOut)
{
    // <?xml version="1.0" encoding="gbk"?>
    // <business id="HX_FPMXJG" comment="��Ʊ��ϸ���">
    //     <body count="1" skph="661814461322" nsrsbh="92320105MA1WMUAP8T">
    //         <group xh="1">
    //             <data name="fplx_dm" value="026" />
    //             <data name="slxlh" value="01d525ea-9ab9-49b7-af73-d5ed7a81971820191009043849" />
    //             <data name="fxsh" value="320100344089314" />
    //         </group>
    //     </body>
    // </business>
    int nRet = -1;
    HDEV hDev = (HDEV)fpxx->hDev;
    mxml_node_t *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    if (!root)
        goto PreDownloadDZFPXML_Finish;

    xml_Node = mxmlNewElement(root, "business");
    if (!xml_Node)
        goto PreDownloadDZFPXML_Finish;
    mxmlElementSetAttr(xml_Node, "id", "HX_FPMXJG");
    mxmlElementSetAttr(xml_Node, "comment", "��Ʊ��ϸ���");
    xml_Node = mxmlNewElement(xml_Node, "body");
    if (!xml_Node)
        goto PreDownloadDZFPXML_Finish;
    mxmlElementSetAttr(xml_Node, "count", "1");
    mxmlElementSetAttr(xml_Node, "skph", hDev->szDeviceID);
    mxmlElementSetAttr(xml_Node, "nsrsbh", hDev->szCommonTaxID);
    xml_Node = mxmlNewElement(xml_Node, "group");
    if (!xml_Node)
        goto PreDownloadDZFPXML_Finish;
    mxmlElementSetAttr(xml_Node, "xh", "1");
    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fplx_dm");
    mxmlElementSetAttr(xml_Node2, "value", "026");
    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "slxlh");
    mxmlElementSetAttr(xml_Node2, "value", fpxx->slxlh);
    xml_Node2 = mxmlNewElement(xml_Node, "data");
    mxmlElementSetAttr(xml_Node2, "name", "fxsh");
    mxmlElementSetAttr(xml_Node2, "value", hDev->szCompressTaxID);
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    //��ʽ����ȷ���͹ٷ�һ����ʽ
    char *pNewFormat = StrReplace(ptr, "\n", " ");
    strcpy(szOut, pNewFormat);
    free(ptr);
    free(pNewFormat);
    nRet = 0;
    int nStrLen = strlen(szOut);
    if (szOut[nStrLen - 1] == ' ') {
        szOut[nStrLen - 1] = '\0';
    }
PreDownloadDZFPXML_Finish:
    mxmlDelete(root);
    return nRet;
}

int InsertSpxxNode(int nNo, HFPXX fpxx, struct Spxx *spxx, mxml_node_t *xml_Mxxx_Node)
{
    char szBuf[512] = "";
    //��ϸ���
    mxml_node_t *xml_Element = mxmlNewElement(xml_Mxxx_Node, "MXXH");
    if (!xml_Element)
        return -1;
    char xh_tmp[10] = {0};
    sprintf(xh_tmp, "%d", nNo);
    mxmlNewText(xml_Element, 0, xh_tmp);

    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //��Ʒ����
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SPBM");
        if (!xml_Element)
            return -2;
        mxmlNewText(xml_Element, 0, spxx->spbh);
        //���б���
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "ZXBM");
        if (!xml_Element)
            return -3;
        mxmlNewText(xml_Element, 0, "");
        //�Ż����߱�ʶ
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "YHZCBS");
        if (!xml_Element)
            return -4;
        char tmp[256] = {0};
        if (memcmp(spxx->xsyh, tmp, sizeof(spxx->xsyh)) == 0) {
            mxmlNewText(xml_Element, 0, "");
        } else {
            mxmlNewText(xml_Element, 0, spxx->xsyh);
        }
        //��˰�ʱ�ʶ
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "LSLBS");
        if (!xml_Element)
            return -5;
        if (atoi(spxx->lslvbs) == 0)
            mxmlNewText(xml_Element, 0, "");
        else
            mxmlNewText(xml_Element, 0, spxx->lslvbs);
        //��ֵ˰�������
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "ZZSTSGL");
        if (!xml_Element)
            return -6;
        mxmlNewText(xml_Element, 0, spxx->yhsm);  //�Ż�˵��
    }
    //����
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "MC");
    if (!xml_Element)
        return -7;
    char spmc_tmp[300] = {0};
    sprintf(spmc_tmp, "%s%s", spxx->spsmmc, spxx->spmc);
    mxmlNewText(xml_Element, 0, spmc_tmp);
    //���
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "JE");
    if (!xml_Element)
        return -8;
    if (JudgeDecimalPoint(spxx->je, 2, 2) != 0) {
        char szJE[20] = {0};
        PriceRound(spxx->je, 2, szJE);
        _WriteLog(LL_WARN, "[+] InsertSpxxNode JudgeDecimalPoint failed, spxx->je:%s", spxx->je);
        mxmlNewText(xml_Element, 0, szJE);
    } else
        mxmlNewText(xml_Element, 0, spxx->je);
    //˰��
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "SL");
    if (!xml_Element)
        return -10;
    mxmlNewText(xml_Element, 0, spxx->slv);
    //˰��
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "SE");
    if (!xml_Element)
        return -11;
    if (JudgeDecimalPoint(spxx->se, 2, 2) != 0) {
        char szSE[20] = {0};
        PriceRound(spxx->se, 2, szSE);
        _WriteLog(LL_WARN, "[+] InsertSpxxNode JudgeDecimalPoint failed, spxx->se:%s", spxx->se);
        mxmlNewText(xml_Element, 0, szSE);
    } else
        mxmlNewText(xml_Element, 0, spxx->se);
    //����
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "SHUL");
    if (strlen(spxx->sl) && xml_Element) {
        if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            PriceRound(spxx->sl, 8, szBuf);
            mxmlNewText(xml_Element, 0, szBuf);
        } else
            mxmlNewText(xml_Element, 0, spxx->sl);
    }
    //����
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "DJ");
    if (strlen(spxx->dj) && xml_Element) {
        //_WriteLog(LL_DEBUG, "Raw DJ:%s", spxx->dj);
        char szDJ[64] = "";
        int nRoundSize = 15;
        if (FPLX_AISINO_DZFP == fpxx->fplx_aisino || FPLX_AISINO_DZZP == fpxx->fplx_aisino)
            nRoundSize = 8;
        if (atoi(spxx->hsjbz))
            PriceRemoveTax(spxx->dj, spxx->slv, nRoundSize, szDJ);
        else
            PriceRound(spxx->dj, nRoundSize, szDJ);
        mxmlNewText(xml_Element, 0, szDJ);
    }
    //����ͺ�
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "GGXH");
    if (!xml_Element)
        return -15;
    mxmlNewText(xml_Element, 0, spxx->ggxh);
    //������λ
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "JLDW");
    if (!xml_Element)
        return -16;
    mxmlNewText(xml_Element, 0, spxx->jldw);

    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //��Ʒ����
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SPBM");
        if (!xml_Element)
            return -17;
        mxmlNewText(xml_Element, 0, spxx->spbh);
        //��ҵ��Ʒ����
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "QYSPBM");
        if (!xml_Element)
            return -18;
        mxmlNewText(xml_Element, 0, "");
        //ʹ���Ż����߱�ʶ
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SYYHZCBZ");
        if (!xml_Element)
            return -19;
        char tmp[256] = {0};
        if (memcmp(spxx->xsyh, tmp, sizeof(spxx->xsyh)) == 0) {
            mxmlNewText(xml_Element, 0, "");
        } else {
            mxmlNewText(xml_Element, 0, spxx->xsyh);
        }
        //�Ż�����
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "YHZC");
        if (!xml_Element)
            return -20;
        mxmlNewText(xml_Element, 0, spxx->yhsm);

        //��˰�ʱ�־
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "LSLBZ");
        if (!xml_Element)
            return -21;
        if (atoi(spxx->lslvbs) == 0)
            mxmlNewText(xml_Element, 0, "");
        else
            mxmlNewText(xml_Element, 0, spxx->lslvbs);
    }
    return 0;
}

//������ӿ� ������Ʊ��Ϣxml
//�ο� aisino.framework.mainform.dll!ns11.class20.method_7(Fpxx, ref XmlDocument)
int PreInvoiceXML(HFPXX fpxx, char *pOutXMLBuf)
{
    HDEV hDev = (HDEV)fpxx->hDev;
    int nRet = -1;
    char szBuf[768] = "";
    if (!fpxx || !pOutXMLBuf) {
        goto PreBuildInvXML_Finish;
    }
    mxml_node_t *xml_root = NULL, *xml_Element = NULL, *xml_Node = NULL, *xml_Node2 = NULL,
                *xml_Mx_Node = NULL, *xml_Mxxx_Node = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    xml_root = mxmlNewElement(root, "FPXT");
    if (!xml_root) {
        nRet = -2;
        goto PreBuildInvXML_Finish;
    }
    xml_Node = mxmlNewElement(xml_root, "INPUT");
    if (!xml_Node) {
        nRet = -3;
        goto PreBuildInvXML_Finish;
    }
    //��˰��ʶ���
    xml_Node2 = mxmlNewElement(xml_Node, "NSRSBH");
    if (!xml_Node2) {
        nRet = -4;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Node2, 0, fpxx->xfsh);
    //��Ʊ����
    xml_Node2 = mxmlNewElement(xml_Node, "KPJH");
    if (!xml_Node2) {
        nRet = -5;
        goto PreBuildInvXML_Finish;
    }
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(xml_Node2, 0, szBuf);
    //�������
    xml_Node2 = mxmlNewElement(xml_Node, "SBBH");
    if (!xml_Node2) {
        nRet = -6;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Node2, 0, hDev->szDeviceID);
    xml_Node2 = mxmlNewElement(xml_Node, "FP");
    if (!xml_Node2) {
        nRet = -7;
        goto PreBuildInvXML_Finish;
    }

    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //�����汾��
        xml_Element = mxmlNewElement(xml_Node2, "BMB_BBH");
        if (!xml_Element) {
            nRet = -8;
            goto PreBuildInvXML_Finish;
        }
        mxmlNewText(xml_Element, 0, fpxx->bmbbbh);
    }
    //��Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "FPZL");
    if (!xml_Element) {
        nRet = -9;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        mxmlNewText(xml_Element, 0, "026");
    } else {
        char tmp[200] = {0};  //�ǵ��ӷ�Ʊʱ������ķ�Ʊ����Ϊ����ר�÷�Ʊ���ͼ� 0 2 ������
        sprintf(tmp, "%d", fpxx->fplx_aisino);
        mxmlNewText(xml_Element, 0, tmp);
    }
    //��Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "KPJH");
    if (!xml_Element) {
        nRet = -10;
        goto PreBuildInvXML_Finish;
    }
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(xml_Element, 0, szBuf);
    //˰��
    xml_Element = mxmlNewElement(xml_Node2, "SL");
    if (!xml_Element) {
        nRet = -11;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->isMultiTax == 1) {
        _WriteLog(LL_DEBUG, "��˰�����");
        mxmlNewText(xml_Element, 0, "");  //�˴�˰�ʿ���Ϊ��˰��
    } else
        mxmlNewText(xml_Element, 0, fpxx->slv);  //�˴�˰�ʿ���Ϊ�ۺ�˰�ʣ���˰�����δ֪
    //��Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "FPDM");
    if (!xml_Element) {
        nRet = -12;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fpdm);
    //��Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "FPHM");
    if (!xml_Element) {
        nRet = -13;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fphm);
    //����Ʊ��
    xml_Element = mxmlNewElement(xml_Node2, "TSPZ");
    if (!xml_Element) {
        nRet = -14;
        goto PreBuildInvXML_Finish;
    }
    char tspz_s[10] = {0};
    if (fpxx->zyfpLx == ZYFP_NCP_SG) {
        sprintf(tspz_s, "02");
    } else if (fpxx->zyfpLx == ZYFP_NCP_XS) {
        sprintf(tspz_s, "01");
    } else {
        if (fpxx->zyfpLx != ZYFP_XT_CCP && fpxx->zyfpLx != ZYFP_XT_YCL &&
            fpxx->zyfpLx != ZYFP_XT_CPJGF) {
            if (fpxx->zyfpLx != ZYFP_XT_JSJHJ) {
                if (fpxx->zyfpLx != ZYFP_CPY) {
                    goto IL_289;
                }
                if (fpxx->yysbz[10] == '2') {
                    sprintf(tspz_s, "18");
                    goto IL_289;
                }
                sprintf(tspz_s, "08");
                goto IL_289;
            }
        }
        sprintf(tspz_s, "03");
    }
IL_289:
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        if (fpxx->zyfpLx == ZYFP_NCP_SG) {
            sprintf(tspz_s, "02");
        } else if (fpxx->zyfpLx == ZYFP_NCP_XS) {
            sprintf(tspz_s, "01");
        } else if (fpxx->zyfpLx == ZYFP_XT_YCL) {
            sprintf(tspz_s, "03");
        } else if (fpxx->zyfpLx == ZYFP_XT_CCP) {
            sprintf(tspz_s, "04");
        } else if (fpxx->zyfpLx == ZYFP_SNY) {
            sprintf(tspz_s, "05");
        } else if (fpxx->zyfpLx == ZYFP_CPY) {
            sprintf(tspz_s, "08");
        } else {
            ;
        }
    }
    mxmlNewText(xml_Element, 0, tspz_s);

    char innerText[10] = {0};
    switch (fpxx->zyfpLx) {
        case ZYFP_SNY:
            sprintf(innerText, "V4");
            break;
        case ZYFP_XT_YCL:
            sprintf(innerText, "V2");
            break;
        case ZYFP_XT_CCP:
            sprintf(innerText, "V3");
            break;
        case ZYFP_NCP_XS:
            sprintf(innerText, "V5");
            break;
        case ZYFP_NCP_SG:
            sprintf(innerText, "V6");
            break;
        case ZYFP_CPY:
            if (fpxx->yysbz[10] == '2') {
                sprintf(innerText, "VA");
            } else {
                sprintf(innerText, "V9");
            }
            break;
        case ZYFP_XT_JSJHJ:
            sprintf(innerText, "VE");
            break;
        case ZYFP_XT_CPJGF:
            sprintf(innerText, "VF");
            break;
        default:
            break;
    }
    // WƱ�б�
    xml_Element = mxmlNewElement(xml_Node2, "WPLB");
    if (!xml_Element) {
        nRet = -15;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, innerText);  //(ò����ר�÷�Ʊ�����й�V2 V3 V5 V6 VA ...)
    //���ϱ�־
    xml_Element = mxmlNewElement(xml_Node2, "ZFBZ");
    if (!xml_Element) {
        nRet = -16;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->zfbz != 0) {
        mxmlNewText(xml_Element, 0, "Y");
    } else {
        mxmlNewText(xml_Element, 0, "N");
    }
    //����
    xml_Element = mxmlNewElement(xml_Node2, "MW");
    if (!xml_Element) {
        nRet = -17;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->mw);
    //����˰��
    xml_Element = mxmlNewElement(xml_Node2, "XFSH");
    if (!xml_Element) {
        nRet = -18;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfsh);
    //��������
    xml_Element = mxmlNewElement(xml_Node2, "XFMC");
    if (!xml_Element) {
        nRet = -19;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfmc);
    //������ַ�绰
    xml_Element = mxmlNewElement(xml_Node2, "XFDZDH");
    if (!xml_Element) {
        nRet = -20;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfdzdh);
    //���������˺�
    xml_Element = mxmlNewElement(xml_Node2, "XFYHZH");
    if (!xml_Element) {
        nRet = -21;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfyhzh);
    //����˰��
    xml_Element = mxmlNewElement(xml_Node2, "GFSH");
    if (!xml_Element) {
        nRet = -22;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->gfsh);
    //��������
    xml_Element = mxmlNewElement(xml_Node2, "GFMC");
    if (!xml_Element) {
        nRet = -23;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->gfmc);
    //������ַ�绰
    xml_Element = mxmlNewElement(xml_Node2, "GFDZDH");
    if (!xml_Element) {
        nRet = -24;
        goto PreBuildInvXML_Finish;
    }
    if (strlen(fpxx->gfdzdh) > 0)
        mxmlNewText(xml_Element, 0, fpxx->gfdzdh);
    //���������˺�
    xml_Element = mxmlNewElement(xml_Node2, "GFYHZH");
    if (!xml_Element) {
        nRet = -25;
        goto PreBuildInvXML_Finish;
    }
    if (strlen(fpxx->gfyhzh) > 0)
        mxmlNewText(xml_Element, 0, fpxx->gfyhzh);
    //��Ʊ���ڸ�ʽ kprq	"2019-09-02 14:44:27";����ı�׼ʱ���ʽ�Բ��� ��Ҫת����
    xml_Element = mxmlNewElement(xml_Node2, "KPRQ");
    if (!xml_Element) {
        nRet = -26;
        goto PreBuildInvXML_Finish;
    }
    char szTimeBuf[24] = "";
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        memset(szTimeBuf, 0, sizeof(szTimeBuf));
        strncpy(szTimeBuf, fpxx->kpsj_standard, 8);
        mxmlNewText(xml_Element, 0, szTimeBuf);
    } else {
        //ʱ���ʽ yyyymmddhhmmss
        mxmlNewText(xml_Element, 0, fpxx->kpsj_standard);
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //��Ʊʱ��HHmmss
        xml_Element = mxmlNewElement(xml_Node2, "KPSJ");
        if (!xml_Element) {
            nRet = -27;
            goto PreBuildInvXML_Finish;
        }
        memset(szTimeBuf, 0, sizeof(szTimeBuf));
        memcpy(szTimeBuf, fpxx->kpsj_standard + 8, 6);
        mxmlNewText(xml_Element, 0, szTimeBuf);
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //��Ʊ״̬��־
        xml_Element = mxmlNewElement(xml_Node2, "FPZTBZ");
        if (!xml_Element) {
            nRet = -28;
            goto PreBuildInvXML_Finish;
        }
        sprintf(szBuf, "%d", fpxx->fpzt);
        mxmlNewText(xml_Element, 0, szBuf);
    }
    //��������
    xml_Element = mxmlNewElement(xml_Node2, "ZFRQ");
    if (!xml_Element) {
        nRet = -29;
        goto PreBuildInvXML_Finish;
    }
    memset(szTimeBuf, 0, sizeof(szTimeBuf));
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {  //ʱ���ʽ yyyymmdd
        memcpy(szTimeBuf, fpxx->zfsj, 8);
    } else {  //ʱ���ʽ yyyymmddhhmmss
        memcpy(szTimeBuf, fpxx->zfsj, 14);
    }
    if (strlen(szTimeBuf) > 0)
        mxmlNewText(xml_Element, 0, szTimeBuf);
    memset(szTimeBuf, 0, sizeof(szTimeBuf));
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //��Ʊʱ��HHmmss
        xml_Element = mxmlNewElement(xml_Node2, "ZFSJ");
        if (!xml_Element) {
            nRet = -30;
            goto PreBuildInvXML_Finish;
        }
        if (!strlen(fpxx->zfsj))
            mxmlNewText(xml_Element, 0, "000000");
        else {
            memcpy(szTimeBuf, fpxx->zfsj + 8, 6);
            mxmlNewText(xml_Element, 0, szTimeBuf);
        }
    }
    //���
    xml_Element = mxmlNewElement(xml_Node2, "JE");
    if (!xml_Element) {
        nRet = -32;
        goto PreBuildInvXML_Finish;
    }
    if (JudgeDecimalPoint(fpxx->je, 2, 2) != 0) {
        nRet = -31;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->je);
    //˰��
    xml_Element = mxmlNewElement(xml_Node2, "SE");
    if (!xml_Element) {
        nRet = -32;
        goto PreBuildInvXML_Finish;
    }
    if (JudgeDecimalPoint(fpxx->se, 2, 2) != 0) {
        nRet = -33;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->se);
    //��˰�ϼ�
    xml_Element = mxmlNewElement(xml_Node2, "JSHJ");
    if (!xml_Element) {
        nRet = -34;
        goto PreBuildInvXML_Finish;
    }
    if (JudgeDecimalPoint(fpxx->jshj, 2, 2) != 0) {
        nRet = -35;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->jshj);
    //��ע
    xml_Element = mxmlNewElement(xml_Node2, "BZ");
    if (!xml_Element) {
        nRet = -36;
        goto PreBuildInvXML_Finish;
    }
    memset(szBuf, 0, sizeof(szBuf));
    UTF8ToGBKBase64(fpxx->bz, strlen(fpxx->bz), szBuf);
    mxmlNewText(xml_Element, 0, szBuf);
    //��Ʊ��
    xml_Element = mxmlNewElement(xml_Node2, "KPR");
    if (!xml_Element) {
        nRet = -37;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->kpr);
    //�տ���
    xml_Element = mxmlNewElement(xml_Node2, "SKR");
    if (!xml_Element) {
        nRet = -38;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->skr);
    //������
    xml_Element = mxmlNewElement(xml_Node2, "FHR");
    if (!xml_Element) {
        nRet = -39;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fhr);
    //���ַ�Ʊ��Ϣ�汾��
    xml_Element = mxmlNewElement(xml_Node2, "HZFPXXBBH");
    if (!xml_Element) {
        nRet = -40;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
        mxmlNewText(xml_Element, 0, "");
    } else {
        mxmlNewText(xml_Element, 0, fpxx->redNum);
    }
    //У����
    xml_Element = mxmlNewElement(xml_Node2, "JYM");
    if (!xml_Element) {
        nRet = -41;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->jym);
    //���ַ�Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "LZFPDM");
    if (!xml_Element) {
        nRet = -42;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
        mxmlNewText(xml_Element, 0, "");
    } else {
        mxmlNewText(xml_Element, 0, fpxx->blueFpdm);
    }
    //���ַ�Ʊ����
    xml_Element = mxmlNewElement(xml_Node2, "LZFPHM");
    if (!xml_Element) {
        nRet = -43;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP) {
        mxmlNewText(xml_Element, 0, "");
    } else {
        mxmlNewText(xml_Element, 0, fpxx->blueFphm);
    }
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        if (fpxx->fpzt == FPSTATUS_RED || fpxx->fpzt == FPSTATUS_RED_WASTE) {
            //����Ʊ���������Ϣ
            // ���ԭ��
            sprintf(szBuf, "%d", fpxx->hcyy);
            mxmlNewText(mxmlNewElement(xml_Node2, "HCYY"), 0, szBuf);
            // ��Ʊ����,������ͨ�����ͣ�����˰�ֵ�
            sprintf(szBuf, "%03d", fpxx->blueFplx);
            mxmlNewText(mxmlNewElement(xml_Node2, "LPLX"), 0, szBuf);
            // LPKPRQ
            strcpy(szBuf, fpxx->blueKpsj);
            szBuf[8] = '\0';
            mxmlNewText(mxmlNewElement(xml_Node2, "LPKPRQ"), 0, szBuf);
        } else {
            mxmlNewElement(xml_Node2, "HCYY");
            mxmlNewElement(xml_Node2, "LPLX");
            mxmlNewElement(xml_Node2, "LPKPRQ");
        }
    }
    //��˰��,�Ҳ�������ֵ����̶�ֵ1����δ�ҵ��ͷ�Ʊ��Ϣ�еı�˰�ڶ�Ӧ֤��
    xml_Element = mxmlNewElement(xml_Node2, "BSQ");
    if (!xml_Element) {
        nRet = -44;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "1");
    //�ֿ���˰��ʶ���
    xml_Element = mxmlNewElement(xml_Node2, "DKNSRSBH");
    if (!xml_Element) {
        nRet = -45;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "");  //����̶�ֵ��
    //�ֿ���ҵ����
    xml_Element = mxmlNewElement(xml_Node2, "DKQYMC");
    if (!xml_Element) {
        nRet = -46;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "");  //����̶�ֵ��
    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //��Ʒ����汾��
        xml_Element = mxmlNewElement(xml_Node2, "SPBMBBH");
        if (!xml_Element) {
            nRet = -47;
            goto PreBuildInvXML_Finish;
        }
        mxmlNewText(xml_Element, 0, fpxx->bmbbbh);
    }
    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //˰�ʱ�־
        xml_Element = mxmlNewElement(xml_Node2, "SLBZ");
        if (!xml_Element) {
            nRet = -48;
            goto PreBuildInvXML_Finish;
        }
        double num4 = 0.0;
        num4 = atof(fpxx->slv);
        //ͨ��˰�ʽ���һЩ����
        if ((fpxx->fplx_aisino == FPLX_AISINO_ZYFP && num4 == 0.05 && fpxx->yysbz[8] == '0') ||
            num4 == 0.015) {
            mxmlNewText(xml_Element, 0, "1");
        } else if (fpxx->yysbz[8] == '2') {
            mxmlNewText(xml_Element, 0, "2");
        } else {
            mxmlNewText(xml_Element, 0, "0");
        }
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //˰�ʱ�־
        xml_Element = mxmlNewElement(xml_Node2, "SLBZ");
        if (!xml_Element) {
            nRet = -49;
            goto PreBuildInvXML_Finish;
        }
        double num5 = 0.0;
        num5 = atof(fpxx->slv);
        //ͨ��˰�ʽ���һЩ����
        if (fpxx->yysbz[8] == '2') {
            mxmlNewText(xml_Element, 0, "2");
        } else if (num5 == 0.015) {
            mxmlNewText(xml_Element, 0, "1");
        } else {
            mxmlNewText(xml_Element, 0, "0");
        }
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
        xml_Element = mxmlNewElement(xml_Node2, "YDDK");
        if (!xml_Element) {
            nRet = -50;
            goto PreBuildInvXML_Finish;
        }
        if (atoi(fpxx->yddk) == 1) {
            mxmlNewText(xml_Element, 0, "1");
        } else {
            mxmlNewText(xml_Element, 0, "0");
        }

        xml_Element = mxmlNewElement(xml_Node2, "DKBDBS");
        if (!xml_Element) {
            nRet = -51;
            goto PreBuildInvXML_Finish;
        }
        if (atoi(fpxx->dkbdbs) == 1) {
            mxmlNewText(xml_Element, 0, "1");
        } else {
            mxmlNewText(xml_Element, 0, "0");
        }
    }
    if ((FPLX_AISINO_DZFP == fpxx->fplx_aisino || FPLX_AISINO_DZZP == fpxx->fplx_aisino) &&
        strchr(fpxx->pubcode, ';')) {
        strcpy(szBuf, fpxx->pubcode);
        char *p2 = strchr(szBuf, ';');
        *p2 = '\0';
        p2 += 1;
        char *p3 = strchr(p2, ';');
        if (!p3)
            goto PreBuildInvXML_Finish;
        *p3 = '\0';
        p3 += 1;

        mxmlNewText(mxmlNewElement(xml_Node2, "PTLX"), 0, szBuf);
        mxmlNewText(mxmlNewElement(xml_Node2, "PTXH"), 0, p2);
        mxmlNewText(mxmlNewElement(xml_Node2, "TQM"), 0, p3);
    }
    if (IsAisino3Device(fpxx->hDev->nDriverVersionNum)) {
        // SSLKJLY
        sprintf(szBuf, "%d", fpxx->sslkjly);
        mxmlNewText(mxmlNewElement(xml_Node2, "SSLKJLY"), 0, szBuf);
        // KJLX,�ٷ���δ���Գ����Ȳ��202204���ۣ����ٷ���ȡ����202208���Խ��ۣ������ַ�Ʊ���ڲ��ֲ����ڣ���ͳһ
        // mxmlNewElement(xml_Node2, "KJLX");
        // KJBBH
        mxmlNewText(mxmlNewElement(xml_Node2, "KJBBH"), 0, fpxx->kjbbh);
    }
    if (0 == fpxx->spsl)  // blankwaste
        goto NoSPXX;
    if ((fpxx->spsl < 0) || (fpxx->spsl > 2000)) {
        nRet = -52;
        goto PreBuildInvXML_Finish;
    }

    //��ϸxml,���ﲻ�ù�FPXX��ʲô��Ʊ�嵥��ʽ,ֻ����qdbj���ж�
    if (!strcmp(fpxx->qdbj, "N")) {
        //���嵥
        struct Spxx *stp_spxx_tmp = fpxx->stp_MxxxHead->stp_next;
        xml_Mx_Node = mxmlNewElement(xml_Node2, "MX");
        if (!xml_Mx_Node)
            goto PreBuildInvXML_Finish;
        int i, nNo;
        for (i = 0; i < fpxx->spsl; i++) {
            if (stp_spxx_tmp == NULL) {
                nRet = -53;
                goto PreBuildInvXML_Finish;
            }
            xml_Mxxx_Node = mxmlNewElement(xml_Mx_Node, "MXXX");
            if (!xml_Mxxx_Node) {
                nRet = -54;
                goto PreBuildInvXML_Finish;
            }
            nNo = i + 1;
            if ((nRet = InsertSpxxNode(nNo, fpxx, stp_spxx_tmp, xml_Mxxx_Node)) < 0) {
                _WriteLog(LL_WARN, "InsertSpxxNode failed nRet %d", nRet);
                nRet = -55;
                goto PreBuildInvXML_Finish;
            }
            stp_spxx_tmp = stp_spxx_tmp->stp_next;
        }
    } else {  //���嵥
        //<root>
        //   <MX>
        //     <MXXX>
        //       <MXXH>1</MXXH>
        //       <MC>(��������嵥)</MC>
        //       <JE>37202.96</JE>
        //     ...
        //     </MXXX>
        //   </MX>
        //   <QD>
        //     <QDXX>
        //       <MXXH>1</MXXH>
        //       ...
        //     </QDXX>
        //   </QD>
        //</root>
        struct Spxx *stp_spxx_tmp = fpxx->stp_MxxxHead;
        if (!xml_Node2)
            goto PreBuildInvXML_Finish;

        int i, nNo;
        for (i = 0; i < fpxx->spsl + 1; i++) {  // spsl + 1 �������׽ڵ�
            if (stp_spxx_tmp == NULL) {
                nRet = -56;
                goto PreBuildInvXML_Finish;
            }
            if (0 == i) {  //�׽ڵ�
                xml_Mx_Node = mxmlNewElement(xml_Node2, "MX");
                xml_Mxxx_Node = mxmlNewElement(xml_Mx_Node, "MXXX");
                nNo = 1;
            } else if (1 <= i) {  //�ڶ��ڵ�����
                if (1 == i)
                    xml_Mx_Node = mxmlNewElement(xml_Node2, "QD");
                xml_Mxxx_Node = mxmlNewElement(xml_Mx_Node, "QDXX");
                nNo = i;
            }
            if (!xml_Mxxx_Node) {
                nRet = -57;
                goto PreBuildInvXML_Finish;
            }

            if (InsertSpxxNode(nNo, fpxx, stp_spxx_tmp, xml_Mxxx_Node) < 0) {
                nRet = -58;
                goto PreBuildInvXML_Finish;
            }
            stp_spxx_tmp = stp_spxx_tmp->stp_next;
        }
    }

NoSPXX:
    xml_Element = mxmlNewElement(xml_Node2, "SIGN");
    if (!xml_Element) {
        nRet = -59;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->sign);
    char *ptr;
    ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    //_WriteLog(LL_WARN, "%s", ptr);
    int nStrLen = strlen(ptr);
    memcpy(pOutXMLBuf, ptr, nStrLen);
    free(ptr);
    nRet = 0;
    //��β����
    if (pOutXMLBuf[nStrLen - 1] == '\n')
        pOutXMLBuf[nStrLen - 1] = '\0';

PreBuildInvXML_Finish:
    mxmlDelete(root);
    return nRet;
}

//����ֵ����fpxx->szSLXLH��д��32λ�������кţ����ڿ���ֱ��ͨ�������кŲ�ѯ�ӿ�
int UploadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    int nRet = ERR_LOGIC_UPLOADINV_FAIL;
    int nOpType = 0;
    do {
        if (!strlen(fpxx->xfsh))
            break;
        if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_PTFP)
            nOpType = TACMD_AISINO_UPFP_ZZS;
        else if (fpxx->fplx_aisino == FPLX_AISINO_DZFP)
            nOpType = TACMD_AISINO_UPFP_DZ;
        else if (fpxx->fplx_aisino == FPLX_AISINO_HYFP || fpxx->fplx_aisino == FPLX_AISINO_JDCFP)
            nOpType = TACMD_AISINO_UPFP_HYJDC;
        else if (fpxx->fplx_aisino == FPLX_AISINO_ESC)
            nOpType = TACMD_AISINO_UPFP_ESC;
        else if (fpxx->fplx_aisino == FPLX_AISINO_JSFP)
            nOpType = TACMD_AISINO_UPFP_JS;
        else
            break;
        if (PreInvoiceXML(fpxx, pRepBuf) < 0) {
            _WriteLog(LL_WARN, "[+] UploadFPBySingle PreInvoiceXML failed");
            SetLastError(hi->hDev->hUSB, ERR_LOGIC_UPLOADINV_BUILD_PREXML, "�ϴ��������ʧ��");
            nRet = ERR_LOGIC_UPLOADINV_BUILD_PREXML;
            break;
        }
        //���ӽӿ���Ҫ������һ��ZZS
        if (FPLX_AISINO_DZFP == fpxx->fplx_aisino &&
            AddDZFPXMLPackage(pRepBuf, true, fpxx->hDev, pRepBuf) < 0) {
            nRet = ERR_TA_DZFPXML_PACKAGE;
            break;
        }
        if (TaNetIo(hi, nOpType, pRepBuf, nRepBufLen) < 0) {
            nRet = ERR_IO_FAIL;
            break;
        }
        // ZZS ==32 2471a544404e4e1fb70cd3d490a184a3
        // DZ ==52 2f99a6d244bc47678122bd6a86c8c95911dd3307aa6545a5af
        if (strlen(pRepBuf) > 54 || strlen(pRepBuf) < 28) {
            nRet = ERR_LOGIC_UPLOADINV_RESULTCHECK;
            break;
        }
        strcpy(fpxx->slxlh, pRepBuf);
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int DownloadFPBySingle(HHTTP hi, HFPXX fpxx, char *pRepBuf, int nRepBufLen)
{
    int nRet = ERR_LOGIC_UPLOADINV_FAIL, nOpType = 0;
    do {
        if (fpxx->fplx_aisino == FPLX_AISINO_ZYFP || fpxx->fplx_aisino == FPLX_AISINO_PTFP) {
            nOpType = TACMD_AISINO_QUERY_FPUP_ZZS;
            if (PreDownloadFPXML(fpxx, pRepBuf) < 0)
                break;
        } else if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            nOpType = TACMD_AISINO_QUERY_FPUP_DZ;
            if (PreDownloadDZFPXML(fpxx, pRepBuf) < 0)
                break;
        } else
            break;
        if (TaNetIo(hi, nOpType, pRepBuf, nRepBufLen) < 0) {
            nRet = ERR_IO_FAIL;
            break;
        }
        nRet = RET_SUCCESS;
    } while (false);
    return nRet;
}

int UpdateInvUploadFlag(char *pServB64Rep, HFPXX fpxx)
{
    HDEV hDevInfo = fpxx->hDev;
    int nDZSYH = fpxx->dzsyh;
    int nServB64Len = strlen(pServB64Rep);
    if (nDZSYH < 0 || nServB64Len > 48 || nServB64Len < 8)
        return -1;
    uint8 abServB64Rep[64] = "";
    int nServRepBinLen = Base64_Decode(pServB64Rep, nServB64Len, (char *)abServB64Rep);
    if (nServRepBinLen < 0)
        return -2;
    uint8 abDZSYHRaw[64] = "";
    int nDZSYHRawLen =
        CalcDZSYH_method13(abServB64Rep, nServRepBinLen, nDZSYH, (uint8 *)abDZSYHRaw);
    if (nDZSYHRawLen < 0)
        return -3;
    abDZSYHRaw[nDZSYHRawLen] = MakePackVerityFlag(abDZSYHRaw, nDZSYHRawLen);
    uint8 cb[256];
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\xde\x00\x00\x00", 10);
    AisinoLogicIO(hDevInfo->hUSB, cb, 10, ncbLen, abDZSYHRaw, nDZSYHRawLen + 1);
    int nDevRetCode = GetDeviceRepCode(cb);
    if (nDevRetCode)
        return -4;
    return 0;
}


//��ֳ��������ϴ��Ͳ�ѯ������ʱ��Ϊ���ķ����ȥ
int InvUp_Upload_Update_IO(HFPXX fpxx, bool bUpdateFlag)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
    int nRet = -1;
    int count = 0;
    int nRepBufSize = CalcAllocBuffLen(fpxx);
    char *response = calloc(1, nRepBufSize);
    if (!response)
        return -1;
    while (1) {
        if (fpxx->hDev->bBreakAllIO) {
            SetLastError(fpxx->hDev->hUSB, ERR_IO_BREAK,
                         "��Ʊ�ϴ������б������ӿ��ж�[Place 1]");  //�˴���ֵ�ϲ��жϲ��������޸�
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n",
                   fpxx->hDev->szDeviceID, fpxx->hDev->hUSB->errinfo);
            fpxx->hDev->bBreakAllIO = 0;  //�������
            nRet = -2;
            free(response);
            return nRet;
        }
        if ((nRet = CreateServerTalk(&hi, fpxx->hDev)) < 0) {
            if (fpxx->hDev->bBreakAllIO) {
                SetLastError(
                    fpxx->hDev->hUSB, ERR_IO_BREAK,
                    "��Ʊ�ϴ������б������ӿ��ж�[Place 2]");  //�˴���ֵ�ϲ��жϲ��������޸�
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                       "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n", fpxx->hDev->szDeviceID,
                       fpxx->hDev->hUSB->errinfo);
                fpxx->hDev->bBreakAllIO = 0;  //�������
                nRet = -3;
                break;
            }
            if ((strstr(fpxx->hDev->hUSB->errinfo, "ERROR@98@֤���ѹ�ʧ") != NULL) ||
                (strstr(fpxx->hDev->hUSB->errinfo, "ERROR@103@֤��״̬δ֪") != NULL) ||
                (strstr(fpxx->hDev->hUSB->errinfo, "ERROR@97@֤��������") != NULL)) {
                SetLastError(fpxx->hDev->hUSB, ERR_TA_CERT_ERROR, "֤���ѹ�ʧ�������ϻ�״̬δ֪");
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,��Ʊ�ϴ���ֶ������쳣,��ʾ��%s\r\n",
                       fpxx->hDev->szDeviceID, fpxx->hDev->hUSB->errinfo);
                nRet = -4;
                break;
            } else {
                nRet = -5;
                break;
            }
        }
        _WriteLog(LL_INFO, "�̺ţ�%s,��Ʊ�ϴ����ӷ��������,�����ϴ���Ʊ����",
                  fpxx->hDev->szDeviceID);
        if ((nRet = UploadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "UploadInvoice failed, nRet:%d", nRet);
            _WriteLog(LL_WARN, "�̺ţ�%s,��Ʊ�����ϴ�ʧ��", fpxx->hDev->szDeviceID);
            break;
        }
        for (count = 0; count < 8; count++)  // 5���㹻?,��ѯ��ʱ��ԼΪ18��
        {
            if (fpxx->hDev->bBreakAllIO) {
                SetLastError(fpxx->hDev->hUSB, ERR_IO_BREAK,
                             "��Ʊ�ϴ������б������ӿ��ж�[Place 3]");
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                       "�̺ţ�%s,�����ϴ���Ʊ,��⵽�ⲿҪ���ж�,%s\r\n", fpxx->hDev->szDeviceID,
                       fpxx->hDev->hUSB->errinfo);
                fpxx->hDev->bBreakAllIO = 0;  //�������
                nRet = -2;
                break;
            }
            sleep(count + 1);  //����Ҫ���·�������������Ǹձ����ϴ���Ͳ�ѯ���϶�����1=������
            // sleep(count * 3);  //����Ҫ���·�������������Ǹձ����ϴ���Ͳ�ѯ���϶�����1=������
            //_WriteLog(LL_WARN, "\n\n��Ʊ�ϴ��ɹ����������ز�ѯ���ŷ�Ʊ");
            if ((nRet = DownloadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
                _WriteLog(LL_WARN, "QueryResult download failed, nRet:%d,errinfo = %s", nRet,
                          fpxx->hDev->hUSB->errinfo);

                if (strstr(fpxx->hDev->hUSB->errinfo, "ǩ��ֵ����") ||
                    strstr(fpxx->hDev->hUSB->errinfo, "ǩ����֤����")) {
                    char report_data[1024] = {0};
                    sprintf(report_data,
                            "ǩ����֤����,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����Ʊ�֣�%d",
                            fpxx->fpdm, fpxx->fphm, fpxx->kpsj_F2, fpxx->zyfpLx);
                    logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,������Ϣ��%s\r\n",
                           fpxx->hDev->szDeviceID, report_data);
                    report_event(fpxx->hDev->szDeviceID, "��ǩʧ��", report_data, -1);
                    nRet = -3;
                    break;
                }
                logout(INFO, "TAXLIB", "��Ʊ�ϴ�", "�̺ţ�%s,��Ʊ�����ϴ����,��ѯ��������ʧ��\r\n",
                       fpxx->hDev->szDeviceID);
                continue;
            }
            _WriteLog(LL_INFO, "DownloadFPBySingle OK, get PrefixDZSYH:%s\n", response);
            if (bUpdateFlag) {
                if ((nRet = UpdateInvUploadFlag(response, fpxx)) < 0) {
                    _WriteLog(LL_WARN, "QueryResult write device failed, nRet:%d", nRet);
                    nRet = -4;
                    break;
                }
            } else {
                _WriteLog(LL_INFO, "�ϴ���ȫ�ɹ�,��������д��");
            }

            // finish
            logout(INFO, "TAXLIB", "��Ʊ�ϴ�",
                   "�̺ţ�%s,�ϴ���Ʊ����д���豸���ɹ�! FPDM:%s FPHM:%s ����:%s \r\n",
                   fpxx->hDev->szDeviceID, fpxx->fpdm, fpxx->fphm, fpxx->hDev->szCompanyName);
            nRet = 0;
            break;
        }

        break;
    }
    HTTPClose(&hi);
    free(response);
    return nRet;
}

// fpxx = true, json = false
int InvUp_UploadAndUpdate(HDEV hDev, void *pInBuff, bool bInbufFpxxOrJson, bool bUpdateFlag)
{
#ifdef _NODEVICE_TEST
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode��Jump really upload!");
    return 0;
#endif
    int nRet = -1;
    if (hDev->bTrainFPDiskType) {
        nRet = 0;
        _WriteLog(LL_INFO, "Train device, jum upload!");
        goto upload_invoice_Finish;
    }
    HFPXX fpxx = NULL;
    if (!bInbufFpxxOrJson) {
        HFPXX fpxxNew = MallocFpxxLinkDev(NULL);
        int nChildRet = Json2FPXX(pInBuff, fpxxNew);
        if (nChildRet < 0) {
            FreeFpxx(fpxxNew);
            _WriteLog(LL_WARN, "InvUp_UploadAndUpdate Json2FPXX failed, nRet:%d", nChildRet);
            goto upload_invoice_Finish;  //��jsonҲ��fpxx�ṹ��
        }
        fpxx = fpxxNew;
    } else {
        fpxx = pInBuff;
    }
    fpxx->hDev = hDev;
    if (InvUp_Upload_Update_IO(fpxx, bUpdateFlag) < 0) {
        goto upload_invoice_Finish;
    }

    nRet = 0;
upload_invoice_Finish:
    return nRet;
}

int AisinoPreCompanyXml(HDEV hDev, char *cmp_xml)
{
    char szBuf[512] = "";
    if (!hDev || !cmp_xml)
        return -1;
    bool bool_5 = true;  //����ʼ��Ϊtrue,���ǲ��Ƿ������ϴ���ʽ�йأ��д�ȷ�ϣ�
    mxml_node_t *xml_root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    int nRet = -2;
    xml_root = mxmlNewElement(root, "FPXT");
    if (!xml_root)
        goto Finish;
    xml_Node = mxmlNewElement(xml_root, "INPUT");
    if (!xml_Node)
        goto Finish;
    //��˰��ʶ���
    xml_Node2 = mxmlNewElement(xml_Node, "NSRSBH");
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szCommonTaxID);
    //��Ʊ����
    xml_Node2 = mxmlNewElement(xml_Node, "KPJH");
    if (!xml_Node2)
        goto Finish;
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(xml_Node2, 0, szBuf);
    //˰�̱��
    xml_Node2 = mxmlNewElement(xml_Node, "SBBH");
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szDeviceID);
    // FX˰��,����ƥ���Ѻ�ʵ
    xml_Node2 = mxmlNewElement(xml_Node, "FXSH");  //δ֪
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szCompressTaxID);
    // HS˰��,����ƥ���Ѻ�ʵ
    xml_Node2 = mxmlNewElement(xml_Node, "HSSH");  //δ֪
    if (!xml_Node2)
        goto Finish;
    GetHashTaxCode(szBuf, hDev->sz9ByteHashTaxID);
    mxmlNewText(xml_Node2, 0, szBuf);
    //��Ȩ��Ϣ
    xml_Node2 = mxmlNewElement(xml_Node, "SQXX");
    if (!xml_Node2)
        goto Finish;
    char szSQXXB64[2550] = "";  //ԭ�ڴ�̫С�ᵼ�����
    if (ReadCompanyInfo(hDev->hUSB, szSQXXB64) < 0)
        goto Finish;
    mxmlNewText(xml_Node2, 0, szSQXXB64);
    if (bool_5) {
        //��Ʒ�����汾��
        xml_Node2 = mxmlNewElement(xml_Node, "SPBMBBH");
        if (!xml_Node2)
            goto Finish;
        mxmlNewText(xml_Node2, 0, "");
        //��Ʒ����
        xml_Node2 = mxmlNewElement(xml_Node, "SPBM");
        if (!xml_Node2)
            goto Finish;
    }
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    int nStrLen = strlen(ptr);
    char tmp[102400] = {0};
    memcpy(tmp, ptr, nStrLen);  //�˶δ��봦��ʽ��ҪΪ����4��mtk
    memcpy(cmp_xml, tmp, nStrLen);
    //�����Ƴ�ĩβ\n,��ʵxmlû��Ҫ��δ��붼���Խ�����������������ݾ������׳���Ϊ���ֺͿ�Ʊ������Ը�ʽ��������ȫһ�£�Ϊ�˷���ȶ�hashֵ�Ϳ��ٶ�λ�����صؽ��ж�β����
    if (cmp_xml[nStrLen - 1] == '\n')
        cmp_xml[nStrLen - 1] = '\0';
    free(ptr);
    nRet = 0;
Finish:
    mxmlDelete(root);
    return nRet;
}
