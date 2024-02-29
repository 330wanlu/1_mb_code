/*****************************************************************************
File name:   aisino_uploadinvoice.c
Description: 用于爱信诺盘的发票上传业务
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     最初代码实现
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
    uint8 cb[2048] = {0};  // 768太小，部分税盘有问题20220510改
    int ncbLen = sizeof cb;
    memcpy(cb, "\xfe\xff\xcc\xbc\x53\xe1\x54\x00\x00\x00", 10);
    int nRet = AisinoLogicIO(hUSB, cb, 10, ncbLen, NULL, 0);
    if (nRet < 128)
        return -1;
    return Base64_Encode((const char *)cb, nRet - 1, szOutInfo);
}

//上传xml第三层, pc_xml > 768
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

//上传xml第三层, pc_xml > 1024
int PreDownloadDZFPXML(HFPXX fpxx, char *szOut)
{
    // <?xml version="1.0" encoding="gbk"?>
    // <business id="HX_FPMXJG" comment="发票明细结果">
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
    mxmlElementSetAttr(xml_Node, "comment", "发票明细结果");
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
    //格式整理，确保和官方一个格式
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
    //明细序号
    mxml_node_t *xml_Element = mxmlNewElement(xml_Mxxx_Node, "MXXH");
    if (!xml_Element)
        return -1;
    char xh_tmp[10] = {0};
    sprintf(xh_tmp, "%d", nNo);
    mxmlNewText(xml_Element, 0, xh_tmp);

    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //商品编码
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SPBM");
        if (!xml_Element)
            return -2;
        mxmlNewText(xml_Element, 0, spxx->spbh);
        //自行编码
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "ZXBM");
        if (!xml_Element)
            return -3;
        mxmlNewText(xml_Element, 0, "");
        //优惠政策标识
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "YHZCBS");
        if (!xml_Element)
            return -4;
        char tmp[256] = {0};
        if (memcmp(spxx->xsyh, tmp, sizeof(spxx->xsyh)) == 0) {
            mxmlNewText(xml_Element, 0, "");
        } else {
            mxmlNewText(xml_Element, 0, spxx->xsyh);
        }
        //零税率标识
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "LSLBS");
        if (!xml_Element)
            return -5;
        if (atoi(spxx->lslvbs) == 0)
            mxmlNewText(xml_Element, 0, "");
        else
            mxmlNewText(xml_Element, 0, spxx->lslvbs);
        //增值税特殊管理
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "ZZSTSGL");
        if (!xml_Element)
            return -6;
        mxmlNewText(xml_Element, 0, spxx->yhsm);  //优惠说明
    }
    //名称
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "MC");
    if (!xml_Element)
        return -7;
    char spmc_tmp[300] = {0};
    sprintf(spmc_tmp, "%s%s", spxx->spsmmc, spxx->spmc);
    mxmlNewText(xml_Element, 0, spmc_tmp);
    //金额
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
    //税率
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "SL");
    if (!xml_Element)
        return -10;
    mxmlNewText(xml_Element, 0, spxx->slv);
    //税额
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
    //数量
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "SHUL");
    if (strlen(spxx->sl) && xml_Element) {
        if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
            PriceRound(spxx->sl, 8, szBuf);
            mxmlNewText(xml_Element, 0, szBuf);
        } else
            mxmlNewText(xml_Element, 0, spxx->sl);
    }
    //单价
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
    //规格型号
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "GGXH");
    if (!xml_Element)
        return -15;
    mxmlNewText(xml_Element, 0, spxx->ggxh);
    //计量单位
    xml_Element = mxmlNewElement(xml_Mxxx_Node, "JLDW");
    if (!xml_Element)
        return -16;
    mxmlNewText(xml_Element, 0, spxx->jldw);

    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //商品编码
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SPBM");
        if (!xml_Element)
            return -17;
        mxmlNewText(xml_Element, 0, spxx->spbh);
        //企业商品编码
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "QYSPBM");
        if (!xml_Element)
            return -18;
        mxmlNewText(xml_Element, 0, "");
        //使用优惠政策标识
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "SYYHZCBZ");
        if (!xml_Element)
            return -19;
        char tmp[256] = {0};
        if (memcmp(spxx->xsyh, tmp, sizeof(spxx->xsyh)) == 0) {
            mxmlNewText(xml_Element, 0, "");
        } else {
            mxmlNewText(xml_Element, 0, spxx->xsyh);
        }
        //优惠政策
        xml_Element = mxmlNewElement(xml_Mxxx_Node, "YHZC");
        if (!xml_Element)
            return -20;
        mxmlNewText(xml_Element, 0, spxx->yhsm);

        //零税率标志
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

//第三层接口 构建发票信息xml
//参考 aisino.framework.mainform.dll!ns11.class20.method_7(Fpxx, ref XmlDocument)
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
    //纳税人识别号
    xml_Node2 = mxmlNewElement(xml_Node, "NSRSBH");
    if (!xml_Node2) {
        nRet = -4;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Node2, 0, fpxx->xfsh);
    //开票机号
    xml_Node2 = mxmlNewElement(xml_Node, "KPJH");
    if (!xml_Node2) {
        nRet = -5;
        goto PreBuildInvXML_Finish;
    }
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(xml_Node2, 0, szBuf);
    //机器编号
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
        //编码表版本号
        xml_Element = mxmlNewElement(xml_Node2, "BMB_BBH");
        if (!xml_Element) {
            nRet = -8;
            goto PreBuildInvXML_Finish;
        }
        mxmlNewText(xml_Element, 0, fpxx->bmbbbh);
    }
    //发票种类
    xml_Element = mxmlNewElement(xml_Node2, "FPZL");
    if (!xml_Element) {
        nRet = -9;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        mxmlNewText(xml_Element, 0, "026");
    } else {
        char tmp[200] = {0};  //非电子发票时，填入的发票类型为航信专用发票类型即 0 2 。。。
        sprintf(tmp, "%d", fpxx->fplx_aisino);
        mxmlNewText(xml_Element, 0, tmp);
    }
    //开票机号
    xml_Element = mxmlNewElement(xml_Node2, "KPJH");
    if (!xml_Element) {
        nRet = -10;
        goto PreBuildInvXML_Finish;
    }
    sprintf(szBuf, "%d", fpxx->kpjh);
    mxmlNewText(xml_Element, 0, szBuf);
    //税率
    xml_Element = mxmlNewElement(xml_Node2, "SL");
    if (!xml_Element) {
        nRet = -11;
        goto PreBuildInvXML_Finish;
    }
    if (fpxx->isMultiTax == 1) {
        _WriteLog(LL_DEBUG, "多税率情况");
        mxmlNewText(xml_Element, 0, "");  //此处税率可能为多税率
    } else
        mxmlNewText(xml_Element, 0, fpxx->slv);  //此处税率可能为综合税率，多税率情况未知
    //发票代码
    xml_Element = mxmlNewElement(xml_Node2, "FPDM");
    if (!xml_Element) {
        nRet = -12;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fpdm);
    //发票号码
    xml_Element = mxmlNewElement(xml_Node2, "FPHM");
    if (!xml_Element) {
        nRet = -13;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fphm);
    //特殊票种
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
    // W票列表
    xml_Element = mxmlNewElement(xml_Node2, "WPLB");
    if (!xml_Element) {
        nRet = -15;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, innerText);  //(貌似与专用发票类型有关V2 V3 V5 V6 VA ...)
    //作废标志
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
    //密文
    xml_Element = mxmlNewElement(xml_Node2, "MW");
    if (!xml_Element) {
        nRet = -17;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->mw);
    //销方税号
    xml_Element = mxmlNewElement(xml_Node2, "XFSH");
    if (!xml_Element) {
        nRet = -18;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfsh);
    //销方名称
    xml_Element = mxmlNewElement(xml_Node2, "XFMC");
    if (!xml_Element) {
        nRet = -19;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfmc);
    //销方地址电话
    xml_Element = mxmlNewElement(xml_Node2, "XFDZDH");
    if (!xml_Element) {
        nRet = -20;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfdzdh);
    //销方银行账号
    xml_Element = mxmlNewElement(xml_Node2, "XFYHZH");
    if (!xml_Element) {
        nRet = -21;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->xfyhzh);
    //购方税号
    xml_Element = mxmlNewElement(xml_Node2, "GFSH");
    if (!xml_Element) {
        nRet = -22;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->gfsh);
    //购方名称
    xml_Element = mxmlNewElement(xml_Node2, "GFMC");
    if (!xml_Element) {
        nRet = -23;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->gfmc);
    //购方地址电话
    xml_Element = mxmlNewElement(xml_Node2, "GFDZDH");
    if (!xml_Element) {
        nRet = -24;
        goto PreBuildInvXML_Finish;
    }
    if (strlen(fpxx->gfdzdh) > 0)
        mxmlNewText(xml_Element, 0, fpxx->gfdzdh);
    //购方银行账号
    xml_Element = mxmlNewElement(xml_Node2, "GFYHZH");
    if (!xml_Element) {
        nRet = -25;
        goto PreBuildInvXML_Finish;
    }
    if (strlen(fpxx->gfyhzh) > 0)
        mxmlNewText(xml_Element, 0, fpxx->gfyhzh);
    //开票日期格式 kprq	"2019-09-02 14:44:27";这里的标准时间格式对不上 需要转换下
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
        //时间格式 yyyymmddhhmmss
        mxmlNewText(xml_Element, 0, fpxx->kpsj_standard);
    }
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //开票时间HHmmss
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
        //发票状态标志
        xml_Element = mxmlNewElement(xml_Node2, "FPZTBZ");
        if (!xml_Element) {
            nRet = -28;
            goto PreBuildInvXML_Finish;
        }
        sprintf(szBuf, "%d", fpxx->fpzt);
        mxmlNewText(xml_Element, 0, szBuf);
    }
    //作废日期
    xml_Element = mxmlNewElement(xml_Node2, "ZFRQ");
    if (!xml_Element) {
        nRet = -29;
        goto PreBuildInvXML_Finish;
    }
    memset(szTimeBuf, 0, sizeof(szTimeBuf));
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {  //时间格式 yyyymmdd
        memcpy(szTimeBuf, fpxx->zfsj, 8);
    } else {  //时间格式 yyyymmddhhmmss
        memcpy(szTimeBuf, fpxx->zfsj, 14);
    }
    if (strlen(szTimeBuf) > 0)
        mxmlNewText(xml_Element, 0, szTimeBuf);
    memset(szTimeBuf, 0, sizeof(szTimeBuf));
    if (fpxx->fplx_aisino == FPLX_AISINO_DZFP) {
        //开票时间HHmmss
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
    //金额
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
    //税额
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
    //价税合计
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
    //备注
    xml_Element = mxmlNewElement(xml_Node2, "BZ");
    if (!xml_Element) {
        nRet = -36;
        goto PreBuildInvXML_Finish;
    }
    memset(szBuf, 0, sizeof(szBuf));
    UTF8ToGBKBase64(fpxx->bz, strlen(fpxx->bz), szBuf);
    mxmlNewText(xml_Element, 0, szBuf);
    //开票人
    xml_Element = mxmlNewElement(xml_Node2, "KPR");
    if (!xml_Element) {
        nRet = -37;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->kpr);
    //收款人
    xml_Element = mxmlNewElement(xml_Node2, "SKR");
    if (!xml_Element) {
        nRet = -38;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->skr);
    //复核人
    xml_Element = mxmlNewElement(xml_Node2, "FHR");
    if (!xml_Element) {
        nRet = -39;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->fhr);
    //红字发票信息版本号
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
    //校验码
    xml_Element = mxmlNewElement(xml_Node2, "JYM");
    if (!xml_Element) {
        nRet = -41;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, fpxx->jym);
    //蓝字发票代码
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
    //蓝字发票号码
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
            //红字票才填红字信息
            // 红冲原因
            sprintf(szBuf, "%d", fpxx->hcyy);
            mxmlNewText(mxmlNewElement(xml_Node2, "HCYY"), 0, szBuf);
            // 蓝票类型,这里是通用类型，交给税局的
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
    //报税期,找不到其他值，填固定值1，尚未找到和发票信息中的报税期对应证据
    xml_Element = mxmlNewElement(xml_Node2, "BSQ");
    if (!xml_Element) {
        nRet = -44;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "1");
    //抵扣纳税人识别号
    xml_Element = mxmlNewElement(xml_Node2, "DKNSRSBH");
    if (!xml_Element) {
        nRet = -45;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "");  //暂填固定值空
    //抵扣企业名称
    xml_Element = mxmlNewElement(xml_Node2, "DKQYMC");
    if (!xml_Element) {
        nRet = -46;
        goto PreBuildInvXML_Finish;
    }
    mxmlNewText(xml_Element, 0, "");  //暂填固定值空
    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //商品编码版本号
        xml_Element = mxmlNewElement(xml_Node2, "SPBMBBH");
        if (!xml_Element) {
            nRet = -47;
            goto PreBuildInvXML_Finish;
        }
        mxmlNewText(xml_Element, 0, fpxx->bmbbbh);
    }
    if (fpxx->fplx_aisino != FPLX_AISINO_DZFP) {
        //税率标志
        xml_Element = mxmlNewElement(xml_Node2, "SLBZ");
        if (!xml_Element) {
            nRet = -48;
            goto PreBuildInvXML_Finish;
        }
        double num4 = 0.0;
        num4 = atof(fpxx->slv);
        //通过税率进行一些计算
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
        //税率标志
        xml_Element = mxmlNewElement(xml_Node2, "SLBZ");
        if (!xml_Element) {
            nRet = -49;
            goto PreBuildInvXML_Finish;
        }
        double num5 = 0.0;
        num5 = atof(fpxx->slv);
        //通过税率进行一些计算
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
        // KJLX,官方尚未测试出来先不填（202204结论）。官方已取消（202208测试结论），部分发票存在部分不存在，不统一
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

    //明细xml,这里不用管FPXX里什么发票清单格式,只根据qdbj来判定
    if (!strcmp(fpxx->qdbj, "N")) {
        //无清单
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
    } else {  //有清单
        //<root>
        //   <MX>
        //     <MXXX>
        //       <MXXH>1</MXXH>
        //       <MC>(详见销货清单)</MC>
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
        for (i = 0; i < fpxx->spsl + 1; i++) {  // spsl + 1 包含个首节点
            if (stp_spxx_tmp == NULL) {
                nRet = -56;
                goto PreBuildInvXML_Finish;
            }
            if (0 == i) {  //首节点
                xml_Mx_Node = mxmlNewElement(xml_Node2, "MX");
                xml_Mxxx_Node = mxmlNewElement(xml_Mx_Node, "MXXX");
                nNo = 1;
            } else if (1 <= i) {  //第二节点正常
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
    //段尾调整
    if (pOutXMLBuf[nStrLen - 1] == '\n')
        pOutXMLBuf[nStrLen - 1] = '\0';

PreBuildInvXML_Finish:
    mxmlDelete(root);
    return nRet;
}

//返回值将在fpxx->szSLXLH中写入32位受理序列号，后期可以直接通过该序列号查询接口
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
            SetLastError(hi->hDev->hUSB, ERR_LOGIC_UPLOADINV_BUILD_PREXML, "上传数据组包失败");
            nRet = ERR_LOGIC_UPLOADINV_BUILD_PREXML;
            break;
        }
        //电子接口需要额外套一层ZZS
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


//拆分成两步，上传和查询，将耗时较为长的分离出去
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
                         "发票上传过程中被其他接口中断[Place 1]");  //此错误值上层判断不可任意修改
            logout(INFO, "TAXLIB", "发票上传", "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n",
                   fpxx->hDev->szDeviceID, fpxx->hDev->hUSB->errinfo);
            fpxx->hDev->bBreakAllIO = 0;  //清除缓存
            nRet = -2;
            free(response);
            return nRet;
        }
        if ((nRet = CreateServerTalk(&hi, fpxx->hDev)) < 0) {
            if (fpxx->hDev->bBreakAllIO) {
                SetLastError(
                    fpxx->hDev->hUSB, ERR_IO_BREAK,
                    "发票上传过程中被其他接口中断[Place 2]");  //此错误值上层判断不可任意修改
                logout(INFO, "TAXLIB", "发票上传",
                       "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n", fpxx->hDev->szDeviceID,
                       fpxx->hDev->hUSB->errinfo);
                fpxx->hDev->bBreakAllIO = 0;  //清除缓存
                nRet = -3;
                break;
            }
            if ((strstr(fpxx->hDev->hUSB->errinfo, "ERROR@98@证书已挂失") != NULL) ||
                (strstr(fpxx->hDev->hUSB->errinfo, "ERROR@103@证书状态未知") != NULL) ||
                (strstr(fpxx->hDev->hUSB->errinfo, "ERROR@97@证书已作废") != NULL)) {
                SetLastError(fpxx->hDev->hUSB, ERR_TA_CERT_ERROR, "证书已挂失或已作废或状态未知");
                logout(INFO, "TAXLIB", "发票上传", "盘号：%s,发票上传与局端连接异常,提示：%s\r\n",
                       fpxx->hDev->szDeviceID, fpxx->hDev->hUSB->errinfo);
                nRet = -4;
                break;
            } else {
                nRet = -5;
                break;
            }
        }
        _WriteLog(LL_INFO, "盘号：%s,发票上传连接服务器完成,即将上传发票数据",
                  fpxx->hDev->szDeviceID);
        if ((nRet = UploadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "UploadInvoice failed, nRet:%d", nRet);
            _WriteLog(LL_WARN, "盘号：%s,发票数据上传失败", fpxx->hDev->szDeviceID);
            break;
        }
        for (count = 0; count < 8; count++)  // 5次足够?,查询超时大约为18秒
        {
            if (fpxx->hDev->bBreakAllIO) {
                SetLastError(fpxx->hDev->hUSB, ERR_IO_BREAK,
                             "发票上传过程中被其他接口中断[Place 3]");
                logout(INFO, "TAXLIB", "发票上传",
                       "盘号：%s,正在上传发票,检测到外部要求中断,%s\r\n", fpxx->hDev->szDeviceID,
                       fpxx->hDev->hUSB->errinfo);
                fpxx->hDev->bBreakAllIO = 0;  //清除缓存
                nRet = -2;
                break;
            }
            sleep(count + 1);  //这里要等下服务器处理，如果是刚报送上传完就查询，肯定返回1=处理中
            // sleep(count * 3);  //这里要等下服务器处理，如果是刚报送上传完就查询，肯定返回1=处理中
            //_WriteLog(LL_WARN, "\n\n发票上传成功，即将下载查询该张发票");
            if ((nRet = DownloadFPBySingle(&hi, fpxx, response, nRepBufSize)) < 0) {
                _WriteLog(LL_WARN, "QueryResult download failed, nRet:%d,errinfo = %s", nRet,
                          fpxx->hDev->hUSB->errinfo);

                if (strstr(fpxx->hDev->hUSB->errinfo, "签名值错误") ||
                    strstr(fpxx->hDev->hUSB->errinfo, "签名验证错误")) {
                    char report_data[1024] = {0};
                    sprintf(report_data,
                            "签名验证错误,发票代码：%s,发票号码：%s,开票时间：%s,特殊票种：%d",
                            fpxx->fpdm, fpxx->fphm, fpxx->kpsj_F2, fpxx->zyfpLx);
                    logout(INFO, "TAXLIB", "发票上传", "盘号：%s,错误信息：%s\r\n",
                           fpxx->hDev->szDeviceID, report_data);
                    report_event(fpxx->hDev->szDeviceID, "验签失败", report_data, -1);
                    nRet = -3;
                    break;
                }
                logout(INFO, "TAXLIB", "发票上传", "盘号：%s,发票数据上传完成,查询受理序列失败\r\n",
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
                _WriteLog(LL_INFO, "上传完全成功,禁用最终写入");
            }

            // finish
            logout(INFO, "TAXLIB", "发票上传",
                   "盘号：%s,上传发票并回写至设备均成功! FPDM:%s FPHM:%s 名称:%s \r\n",
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
    _WriteLog(LL_FATAL, "_NODEVICE_TEST Mode，Jump really upload!");
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
            goto upload_invoice_Finish;  //非json也非fpxx结构体
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
    bool bool_5 = true;  //赋初始化为true,与是不是服务器上传方式有关（有待确认）
    mxml_node_t *xml_root = NULL, *xml_Node = NULL, *xml_Node2 = NULL;
    mxml_node_t *root = mxmlNewXML("1.0");
    int nRet = -2;
    xml_root = mxmlNewElement(root, "FPXT");
    if (!xml_root)
        goto Finish;
    xml_Node = mxmlNewElement(xml_root, "INPUT");
    if (!xml_Node)
        goto Finish;
    //纳税人识别号
    xml_Node2 = mxmlNewElement(xml_Node, "NSRSBH");
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szCommonTaxID);
    //开票机号
    xml_Node2 = mxmlNewElement(xml_Node, "KPJH");
    if (!xml_Node2)
        goto Finish;
    sprintf(szBuf, "%d", hDev->uICCardNo);
    mxmlNewText(xml_Node2, 0, szBuf);
    //税盘编号
    xml_Node2 = mxmlNewElement(xml_Node, "SBBH");
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szDeviceID);
    // FX税号,数据匹配已核实
    xml_Node2 = mxmlNewElement(xml_Node, "FXSH");  //未知
    if (!xml_Node2)
        goto Finish;
    mxmlNewText(xml_Node2, 0, hDev->szCompressTaxID);
    // HS税号,数据匹配已核实
    xml_Node2 = mxmlNewElement(xml_Node, "HSSH");  //未知
    if (!xml_Node2)
        goto Finish;
    GetHashTaxCode(szBuf, hDev->sz9ByteHashTaxID);
    mxmlNewText(xml_Node2, 0, szBuf);
    //授权信息
    xml_Node2 = mxmlNewElement(xml_Node, "SQXX");
    if (!xml_Node2)
        goto Finish;
    char szSQXXB64[2550] = "";  //原内存太小会导致溢出
    if (ReadCompanyInfo(hDev->hUSB, szSQXXB64) < 0)
        goto Finish;
    mxmlNewText(xml_Node2, 0, szSQXXB64);
    if (bool_5) {
        //商品编码表版本号
        xml_Node2 = mxmlNewElement(xml_Node, "SPBMBBH");
        if (!xml_Node2)
            goto Finish;
        mxmlNewText(xml_Node2, 0, "");
        //商品编码
        xml_Node2 = mxmlNewElement(xml_Node, "SPBM");
        if (!xml_Node2)
            goto Finish;
    }
    char *ptr = mxmlSaveAllocString(root, MXML_NO_CALLBACK);
    int nStrLen = strlen(ptr);
    char tmp[102400] = {0};
    memcpy(tmp, ptr, nStrLen);  //此段代码处理方式主要为兼容4口mtk
    memcpy(cmp_xml, tmp, nStrLen);
    //这里移除末尾\n,其实xml没必要这段代码都可以解析，但是这里的内容经常容易出错，为保持和开票软件绝对格式和数据完全一致，为了方便比对hash值和快速定位错误，特地进行段尾调整
    if (cmp_xml[nStrLen - 1] == '\n')
        cmp_xml[nStrLen - 1] = '\0';
    free(ptr);
    nRet = 0;
Finish:
    mxmlDelete(root);
    return nRet;
}
