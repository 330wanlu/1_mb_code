/*************************************************
File name:   ta_iface.c
Description: 用于处理航信、百旺等税盘与税务局交互的底层接口和共用封装
Author:      Zako
Version:     1.0
Date:        2020.02
History:
20211020     税局地址由Base64转明文二维方式，方便调整
*************************************************/
#include "ta_iface.h"

bool GetURLValueByIndex(int nIndex, struct UploadAddrModel *modelUnitHead, int nAvailSize,
                        struct UploadAddrModel *pstOutModel)
{
    bool ret = false;
    int i = 0;
    for (i = 0; i < nAvailSize; i++) {
        struct UploadAddrModel *modelUnit = &modelUnitHead[i];
        int nModelNumber = atoi(modelUnit->szRegCode);
        if (nIndex != nModelNumber)
            continue;
        memcpy(pstOutModel, modelUnit, sizeof(struct UploadAddrModel));
        ret = true;
        break;
    }
    return ret;
}
bool EtaxGetURLValueByIndex(int nIndex, struct EtaxUploadAddrModel *modelUnitHead, int nAvailSize,
                        struct EtaxUploadAddrModel *pstOutModel)
{
    bool ret = false;
    int i = 0;
    for (i = 0; i < nAvailSize; i++) {
        struct EtaxUploadAddrModel *modelUnit = &modelUnitHead[i];
        int nModelNumber = atoi(modelUnit->szRegCode);
        if (nIndex != nModelNumber)
            continue;
        memcpy(pstOutModel, modelUnit, sizeof(struct EtaxUploadAddrModel));
        ret = true;
        break;
    }
    return ret;
}


//税务机构代码共分6层,用11位数字表示.第1层1位数字,表示税务机构类别(0-国家税务总局,1-省级及以下税务机构),其后每2位数字为1层,分别表示省、地市、区县、乡镇、股级.
//税收征管业务系统中的税务机构代码使用11位定长码,编码不足11位时,按标准编码规则在右侧加"0"补齐.特殊情况下使用变长码的,从左至右按层次选取编码.
int GetTaServerURL(uint8 bServType, char *szRegCode6, struct UploadAddrModel *pstOutModel)
{
    //----------------------------------税务局各种上报地址、确认平台地址、版式文件地址、电子税务局地址-----------------------------------------
    struct UploadAddrModel stAisinoNisecUploadAddr[] = {
        {0, "北京", "11", "https://tysl.beijing.chinatax.gov.cn"},
        {0, "上海", "31", "https://skfp.shanghai.chinatax.gov.cn:443"},
        {0, "湖北", "42", "https://fp.hubei.chinatax.gov.cn"},
        {0, "湖南", "43", "https://fpsc.hnhtxx.com.cn:7001"},
        {0, "河北", "13", "https://zzsfp.hebei.chinatax.gov.cn:443"},
        {0, "河南", "41", "https://tysl.henan.chinatax.gov.cn:443"},
        {0, "陕西", "61", "https://fpkj.shaanxi.chinatax.gov.cn:7002"},
        {0, "江苏", "32", "https://fpsc.jiangsu.chinatax.gov.cn:8001"},
        {0, "浙江", "33", "https://fpjr.zhejiang.chinatax.gov.cn:7777"},
        {0, "宁波", "3302", "https://zzsfp.ningbo.chinatax.gov.cn:9002"},
        {0, "甘肃", "62", "https://61.178.20.147:443"},
        {0, "云南", "53", "https://yunnan.chinatax.gov.cn:7001"},
        {0, "福建", "35", "https://skkp.fujian.chinatax.gov.cn"},
        {0, "厦门", "3502", "https://222.76.203.45:7001"},
        {0, "吉林", "22", "https://fpsl.jilin.chinatax.gov.cn:4430"},
        {0, "辽宁", "21", "https://fpsl.liaoning.chinatax.gov.cn:9002"},
        {0, "宁夏", "64", "https://61.133.192.42:6001"},
        //天津的服务器三通，但是AAA IPv6响应有点问题
        // 123.150.99.14 天津电信|60.29.109.57 天津联通|117.131.140.14 天津移动
        {0, "天津", "12", "https://zzssjb.tjsat.gov.cn:443"},
        {0, "海南", "46", "https://zzs.hainan.chinatax.gov.cn:4431"},
        {0, "大连", "2102", "https://sbf.dalian.chinatax.gov.cn:8001"},
        {0, "山东", "37", "https://218.57.142.46:7002"},
        {0, "青岛", "3702", "https://einv.qingdao.chinatax.gov.cn:8002"},
        {0, "广东", "44", "https://skfp.guangdong.chinatax.gov.cn:7001"},
        {0, "深圳", "4403", "https://fp.shenzhen.chinatax.gov.cn:8001"},
        {0, "贵州", "52", "https://skfp.guizhou.chinatax.gov.cn:5002"},
        {0, "青海", "63", "https://fpkj.qinghai.chinatax.gov.cn"},
        {0, "新疆", "65", "https://222.82.232.193:7001"},
        {0, "广西", "45", "https://bswj.fpggfwpt.guangxi.chinatax.gov.cn:9013"},
        {0, "四川", "51", "https://fpsjb.sichuan.chinatax.gov.cn:7001"},
        {0, "山西", "14", "https://202.99.194.28:443"},
        {0, "重庆", "50", "https://fpkj.chongqing.chinatax.gov.cn:444"},
        {0, "黑龙江", "23", "https://fpxxt.heilongjiang.chinatax.gov.cn:7005"},
        {0, "安徽", "34", "https://fpgl.anhui.chinatax.gov.cn:7001"},
        {0, "内蒙古", "15", "https://zzsfp.nm-n-tax.gov.cn:17001"},
        {0, "江西", "36", "https://fpkj.jiangxi.chinatax.gov.cn:7001"},
        {0, "西藏", "54", "https://218.206.186.23:1002"}};
    struct UploadAddrModel stAisinoNisecBswjAddr[] = {
        {0, "北京", "11", "https://tysl.beijing.chinatax.gov.cn:9443"},
        {0, "天津", "12", "https://zzssjb.tjsat.gov.cn:446"},
        {0, "河北", "13", "https://zzsfp.hebei.chinatax.gov.cn:9001"},
        {0, "山西", "14", "https://fpcy.shanxi.chinatax.gov.cn:9901"},
        {0, "内蒙古", "15", "https://zzsfp.nm-n-tax.gov.cn:17007"},
        {0, "辽宁", "21", "https://fpsl.liaoning.chinatax.gov.cn:8001"},
        {0, "大连", "2102", "https://sbf.dalian.chinatax.gov.cn:9001"},
        {0, "吉林", "22", "https://fpsl.jilin.chinatax.gov.cn:4434"},
        {0, "黑龙江", "23", "https://fpxxt.heilongjiang.chinatax.gov.cn:9015"},
        {0, "上海", "31", "https://skfp.shanghai.chinatax.gov.cn:9008"},
        {0, "江苏", "32", "https://bswj.jiangsu.chinatax.gov.cn:7001"},
        {0, "浙江", "33", "https://fpjr.zhejiang.chinatax.gov.cn:9999"},
        {0, "宁波", "3302", "https://zzsfp.ningbo.chinatax.gov.cn:9008"},
        {0, "安徽", "34", "https://fpgl.anhui.chinatax.gov.cn:8002"},
        {0, "福建", "35", "https://fpggfw.fujian.chinatax.gov.cn:8443"},
        {0, "厦门", "3502", "https://fpdk.xiamen.chinatax.gov.cn:9001"},
        {0, "江西", "36", "https://fpcy.jiangxi.chinatax.gov.cn:9001"},
        {0, "山东", "37", "https://fpcy.shandong.chinatax.gov.cn:8443"},
        {0, "青岛", "3702", "https://einv.qingdao.chinatax.gov.cn:8008"},
        {0, "河南", "41", "https://tysl.henan.chinatax.gov.cn:9443"},
        {0, "湖北", "42", "https://fp.hubei.chinatax.gov.cn:8088"},
        {0, "湖南", "43", "https://fpdk.hunan.chinatax.gov.cn:8068"},
        {0, "广东", "44", "https://skfp.guangdong.chinatax.gov.cn:9001"},
        {0, "深圳", "4403", "https://fp.shenzhen.chinatax.gov.cn:8389"},
        {0, "广西", "45", "https://bswj.fpggfwpt.guangxi.chinatax.gov.cn:19001"},
        {0, "海南", "46", "https://zzs.hainan.chinatax.gov.cn:9008"},
        {0, "重庆", "50", "https://fpkj.chongqing.chinatax.gov.cn:666"},
        {0, "四川", "51", "https://fpsjb.sichuan.chinatax.gov.cn:7012"},
        {0, "贵州", "52", "https://skfp.guizhou.chinatax.gov.cn:6060"},
        {0, "云南", "53", "https://yunnan.chinatax.gov.cn:9008"},
        {0, "西藏", "54", "https://218.206.186.23:7001"},
        {0, "陕西", "61", "https://fpcy.shaanxi.chinatax.gov.cn:9002"},
        {0, "甘肃", "62", "https://fpxz.gansu.chinatax.gov.cn:8888"},
        {0, "青海", "63", "https://fpkj.qinghai.chinatax.gov.cn:9001"},
        {0, "宁夏", "64", "https://zzsfp.ningxia.chinatax.gov.cn:9001"},
        {0, "新疆", "65", "https://zzsfp.xj-n-tax.gov.cn:9001"}};
    struct UploadAddrModel stCntaxUploadAddr[] = {
        {0, "北京", "11", "https://tysl.beijing.chinatax.gov.cn:8443"},
        {0, "天津", "12", "https://zzssjb.tjsat.gov.cn:4431"},
        {0, "河北", "13", "https://zzsfp.hebei.chinatax.gov.cn:8001"},
        {0, "山西", "14", "https://202.99.194.28:556"},
        {0, "内蒙古", "15", "https://zzsfp.nm-n-tax.gov.cn:17005"},
        {0, "辽宁", "21", "https://fpsl.liaoning.chinatax.gov.cn:9001"},
        {0, "吉林", "22", "https://fpsl.jilin.chinatax.gov.cn:4433"},
        {0, "黑龙江", "23", "https://fpxxt.heilongjiang.chinatax.gov.cn:9014"},
        {0, "上海", "31", "https://skfp.shanghai.chinatax.gov.cn:9006"},
        {0, "江苏", "32", "https://fpsc.jiangsu.chinatax.gov.cn:9001"},
        {0, "浙江", "33", "https://fpjr.zhejiang.chinatax.gov.cn:8888"},
        {0, "安徽", "34", "https://fpgl.anhui.chinatax.gov.cn:8001"},
        {0, "湖南", "43", "https://fpdk.hunan.chinatax.gov.cn:8069"},
        {0, "福建", "35", "https://fpggfw.fujian.chinatax.gov.cn:443"},
        {0, "江西", "36", "https://fpkj.jiangxi.chinatax.gov.cn:7002"},
        {0, "山东", "37", "https://218.57.142.46:9001"},
        {0, "河南", "41", "https://tysl.henan.chinatax.gov.cn:8443"},
        {0, "湖北", "42", "https://fp.hubei.chinatax.gov.cn:8899"},
        {0, "广东", "44", "https://ggfw.web.guangdong.chinatax.gov.cn:443"},
        {0, "广西", "45", "https://bswj.fpggfwpt.guangxi.chinatax.gov.cn:19104"},
        {0, "海南", "46", "https://zzs.hainan.chinatax.gov.cn:9006"},
        {0, "重庆", "50", "https://fpkj.chongqing.chinatax.gov.cn:555"},
        {0, "四川", "51", "https://fpsjb.sichuan.chinatax.gov.cn:7011"},
        {0, "贵州", "52", "https://skfp.guizhou.chinatax.gov.cn:6002"},
        {0, "云南", "53", "https://yunnan.chinatax.gov.cn:9006"},
        {0, "西藏", "54", "https://218.206.186.23:9001"},
        {0, "陕西", "61", "https://fpkj.shaanxi.chinatax.gov.cn:9002"},
        {0, "甘肃", "62", "https://fpkj.gansu.chinatax.gov.cn:443"},
        {0, "青海", "63", "https://fpkj.qinghai.chinatax.gov.cn:9002"},
        {0, "宁夏", "64", "https://zzsfp.ningxia.chinatax.gov.cn:9000"},
        {0, "新疆", "65", "https://zzsfp.xj-n-tax.gov.cn:9443"},
        {0, "大连", "2102", "https://sbf.dalian.chinatax.gov.cn:8001"},
        {0, "宁波", "3302", "https://zzsfp.ningbo.chinatax.gov.cn:9006"},
        {0, "厦门", "3502", "https://fpdk.xiamen.chinatax.gov.cn:8001"},
        {0, "青岛", "3702", "https://einv.qingdao.chinatax.gov.cn:8006"},
        {0, "深圳", "4403", "https://fp.shenzhen.chinatax.gov.cn:8387"}};
    struct UploadAddrModel stCommonConfirmAddr[] = {
        {0, "北京", "11", "https://fpdk.beijing.chinatax.gov.cn"},
        {0, "天津", "12", "https://fpdk.tianjin.chinatax.gov.cn"},
        {0, "河北", "13", "https://fpdk.hebei.chinatax.gov.cn"},
        {0, "山西", "14", "https://fpdk.shanxi.chinatax.gov.cn"},
        {0, "内蒙古", "15", "https://fpdk.neimenggu.chinatax.gov.cn"},
        {0, "辽宁", "21", "https://fpdk.liaoning.chinatax.gov.cn"},
        {0, "大连", "2102", "https://sbf.dalian.chinatax.gov.cn:8401"},
        {0, "吉林", "22", "https://fpdk.jilin.chinatax.gov.cn:4431"},
        {0, "黑龙江", "23", "https://fpdk.heilongjiang.chinatax.gov.cn"},
        {0, "上海", "31", "https://fpdk.shanghai.chinatax.gov.cn"},
        {0, "江苏", "32", "https://fpdk.jiangsu.chinatax.gov.cn:81"},
        {0, "浙江", "33", "https://fpdk.zhejiang.chinatax.gov.cn"},
        {0, "宁波", "3302", "https://fpdk.ningbo.chinatax.gov.cn"},
        {0, "安徽", "34", "https://fpdk.anhui.chinatax.gov.cn"},
        {0, "福建", "35", "https://fpdk.fujian.chinatax.gov.cn"},
        {0, "厦门", "3502", "https://fpdk.xiamen.chinatax.gov.cn"},
        {0, "江西", "36", "https://fpdk.jiangxi.chinatax.gov.cn"},
        {0, "山东", "37", "https://fpdk.shandong.chinatax.gov.cn"},
        {0, "青岛", "3702", "https://fpdk.qingdao.chinatax.gov.cn"},
        {0, "河南", "41", "https://fpdk.henan.chinatax.gov.cn"},
        {0, "湖北", "42", "https://fpdk.hubei.chinatax.gov.cn"},
        {0, "湖南", "43", "https://fpdk.hunan.chinatax.gov.cn"},
        {0, "广东", "44", "https://fpdk.guangdong.chinatax.gov.cn"},
        {0, "深圳", "4403", "https://fpdk.shenzhen.chinatax.gov.cn"},
        {0, "广西", "45", "https://fpdk.guangxi.chinatax.gov.cn"},
        {0, "海南", "46", "https://fpdk.hainan.chinatax.gov.cn"},
        {0, "重庆", "50", "https://fpdk.chongqing.chinatax.gov.cn"},
        {0, "四川", "51", "https://fpdk.sichuan.chinatax.gov.cn"},
        {0, "贵州", "52", "https://fpdk.guizhou.chinatax.gov.cn"},
        {0, "云南", "53", "http://fpdk.yunnan.chinatax.gov.cn"},
        {0, "西藏", "54", "https://fpdk.xizang.chinatax.gov.cn"},
        {0, "陕西", "61", "https://fpdk.shaanxi.chinatax.gov.cn"},
        {0, "甘肃", "62", "https://fpdk.gansu.chinatax.gov.cn"},
        {0, "青海", "63", "https://fpdk.qinghai.chinatax.gov.cn"},
        {0, "宁夏", "64", "https://fpdk.ningxia.chinatax.gov.cn"},
        {0, "新疆", "65", "https://fpdk.xj-n-tax.gov.cn"}};

    if (strlen(szRegCode6) != 6)
        return ERR_PARM_SIZE;
    struct UploadAddrModel *pstAddr = NULL;
    int nAvailSize = 0;
    char szText[16] = "";
    switch (bServType) {
        case TAADDR_UPLOAD_RSA:  //航信百旺 上报抄报地址
            pstAddr = (struct UploadAddrModel *)&stAisinoNisecUploadAddr;
            nAvailSize = sizeof(stAisinoNisecUploadAddr) / sizeof(struct UploadAddrModel);
            break;
        case TAADDR_PLATFORM:
            pstAddr =  //航信百旺 公共平台地址,版式文件
                (struct UploadAddrModel *)&stAisinoNisecBswjAddr;
            nAvailSize = sizeof(stAisinoNisecBswjAddr) / sizeof(struct UploadAddrModel);
            break;
        case TAADDR_UPLOAD_GM:  // Cntax上报抄报地址
            pstAddr = (struct UploadAddrModel *)&stCntaxUploadAddr;
            nAvailSize = sizeof(stCntaxUploadAddr) / sizeof(struct UploadAddrModel);
            break;
        case TAADDR_CONFIRM:  // 增值税发票综合服务平台
            pstAddr = (struct UploadAddrModel *)&stCommonConfirmAddr;
            nAvailSize = sizeof(stCommonConfirmAddr) / sizeof(struct UploadAddrModel);
            break;
        default:
            return ERR_LOGIC_NOTFOUND;
            break;
    }
    //寻找对应结构，并拷贝
    bool bFoundServerStruct = true;
    do {  //发现即break
        strcpy(szText, szRegCode6);
        szText[4] = '\0';
        if (!strncmp(szRegCode6, "91", 2) &&
            GetURLValueByIndex(50, pstAddr, nAvailSize, pstOutModel))
            break;
        int nCode4 = atoi(szText);
        if (GetURLValueByIndex(nCode4, pstAddr, nAvailSize, pstOutModel))  //能找到4位的
            break;
        szText[2] = '\0';
        int nCode2 = atoi(szText);
        if (GetURLValueByIndex(nCode2, pstAddr, nAvailSize, pstOutModel))  //能找到2位的
            break;
        bFoundServerStruct = false;
    } while (false);
    if (!bFoundServerStruct)
        return ERR_LOGIC_NOTFOUND;
    //初始化其他字段
    pstOutModel->bServType = bServType;
    return RET_SUCCESS;
}

int GetEtaxServerURL(uint8 bServType, char *szRegCode6, struct EtaxUploadAddrModel *pstOutModel)
{
    //----------------------------------税务局各种上报地址、确认平台地址、版式文件地址、电子税务局地址-----------------------------------------
   struct EtaxUploadAddrModel stCommonEtaxAddr[] = {
        {0, "上海", "31", "https://etax.shanghai.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "四川", "51", "https://etax.sichuan.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "江苏", "32", "https://etax.jiangsu.chinatax.gov.cn","/sso/login"},
        {0, "山西", "14", "https://etax.shanxi.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "河南", "41", "https://etax.henan.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "青岛", "3702", "https://etax.qingdao.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "重庆", "50", "https://etax.chongqing.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "北京", "11", "https://etax.beijing.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "安徽", "34", "https://etax.anhui.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "新疆", "65", "https://etax.xinjiang.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "广东", "44", "https://etax.guangdong.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "江西", "36", "https://etax.jiangxi.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "宁波", "3302", "https://etax.ningbo.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "海南", "46", "https://etax.hainan.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "福建", "35", "https://etax.fujian.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "贵州", "52", "https://etax.guizhou.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "甘肃", "62", "https://etax.gansu.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "山东", "37", "https://etax.shandong.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "河北", "13", "https://etax.hebei.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "浙江", "33", "https://etax.zhejiang.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "陕西", "61", "https://etax.shaanxi.chinatax.gov.cn","/login-web/captcha.jpg"},
        {0, "广西", "45", "https://etax.guangxi.chinatax.gov.cn","/login-web/captcha.jpg"},
        };
    if (strlen(szRegCode6) != 6 && strlen(szRegCode6) != 2 && strlen(szRegCode6) != 4)
        return ERR_PARM_SIZE;
    struct EtaxUploadAddrModel *pstAddr = NULL;
    int nAvailSize = 0;
    char szText[16] = "";
    switch (bServType) {
        case TAADDR_ETAX:  // 电子税务局
            pstAddr = (struct EtaxUploadAddrModel *)&stCommonEtaxAddr;
            nAvailSize = sizeof(stCommonEtaxAddr) / sizeof(struct EtaxUploadAddrModel);
            break;
        default:
            return ERR_LOGIC_NOTFOUND;
            break;
    }
    //寻找对应结构，并拷贝
    bool bFoundServerStruct = true;
    do {  //发现即break
        strcpy(szText, szRegCode6);
        szText[4] = '\0';
        if (!strncmp(szRegCode6, "91", 2) &&
            EtaxGetURLValueByIndex(50, pstAddr, nAvailSize, pstOutModel))
            break;
        int nCode4 = atoi(szText);
        if (EtaxGetURLValueByIndex(nCode4, pstAddr, nAvailSize, pstOutModel))  //能找到4位的
            break;
        szText[2] = '\0';
        int nCode2 = atoi(szText);
        printf("lbc-debug nCode2=%d\n",nCode2);
        if (EtaxGetURLValueByIndex(nCode2, pstAddr, nAvailSize, pstOutModel))  //能找到2位的
            break;
        bFoundServerStruct = false;
    } while (false);
    if (!bFoundServerStruct)
        return ERR_LOGIC_NOTFOUND;
    //初始化其他字段
    pstOutModel->bServType = bServType;
    _WriteLog(LL_INFO, "Get server URL OK.Type:%d Value:[%s %s]", bServType, pstOutModel->szRegName,pstOutModel->szTaxAuthorityURL);
    printf("Get server URL OK.Type:%d Value:[%s %s],szRegCode=%s,szInitRequestPage=%s\r\n", pstOutModel->bServType, pstOutModel->szRegName,pstOutModel->szTaxAuthorityURL,pstOutModel->szRegCode,pstOutModel->szInitRequestPage);
    return RET_SUCCESS;
}

//入口是明文technology和bussiness
int PreMessageDigest(uint8 bDeviceType, char *szTechnologyXML, char *szBusinessContentXML,
                     char *szOutB64Digest)
{
    if (strlen(szTechnologyXML) < 16 || strlen(szBusinessContentXML) < 16)
        return -1;
    int nB64TecBufLen = strlen(szTechnologyXML) * 2 + 512;
    char *pB64TecBuf = calloc(1, nB64TecBufLen);
    int nB64BusBufLen = strlen(szBusinessContentXML) * 2 + 512;
    char *pB64BusBuf = calloc(1, nB64BusBufLen);
    char *pAllBuff = NULL;
    char szSHA1Out[64] = "";
    int nRet = -2;
    while (1) {
        if (!pB64BusBuf || !pB64TecBuf) {
            nRet = -3;
            break;
        }
        //前期可以使用的原因是因为aisino和nisec被xml包裹，都是ascii格式，是否由utf8转成GBK都无所谓
        //实际上可能确实是GBK格式
        if (bDeviceType == DEVICE_AISINO || bDeviceType == DEVICE_NISEC) {
            Base64_Encode(szTechnologyXML, strlen(szTechnologyXML), pB64TecBuf);
            Base64_Encode(szBusinessContentXML, strlen(szBusinessContentXML), pB64BusBuf);
        } else {  // cntax
            UTF8ToGBKBase64(szTechnologyXML, strlen(szTechnologyXML), pB64TecBuf);
            UTF8ToGBKBase64(szBusinessContentXML, strlen(szBusinessContentXML), pB64BusBuf);
        }
        pAllBuff = calloc(1, strlen(pB64TecBuf) + strlen(pB64BusBuf) + 1024);
        sprintf(pAllBuff, "%s%s", pB64TecBuf, pB64BusBuf);
        CalcSHA1(pAllBuff, strlen(pAllBuff), (void *)szSHA1Out);
        Base64_Encode(szSHA1Out, 0x14, szOutB64Digest);
        nRet = 0;
        break;
    }
    if (pB64BusBuf)
        free(pB64BusBuf);
    if (pB64TecBuf)
        free(pB64TecBuf);
    if (pAllBuff)
        free(pAllBuff);
    return nRet;
}

char *BuildAuthCode(char *pRepBuff, HDEV hDev)
{
    char *pRepRadom1 = strchr(pRepBuff, '@');
    if (!pRepRadom1)
        return NULL;
    *pRepRadom1 = '\0';
    pRepRadom1 += 1;
    char *pRepRadom2 = strchr(pRepRadom1, '@');
    if (!pRepRadom2)
        return NULL;
    *pRepRadom2 = '\0';
    pRepRadom2 += 1;
    // alloc buff
    int nRetBufLen = 16 * 1024;  //实际4k左右
    char *pRetBuff = calloc(1, nRetBufLen);
    if (hDev->bDeviceType == DEVICE_CNTAX)
        strcat(pRetBuff, "certType=ecc&");
    strcat(pRetBuff, "type=client-authcode&client-authcode=");
    int nLen = nRetBufLen - strlen(pRepBuff);
    if (BuildClientAuthCode((uint8 *)pRepRadom1, strlen(pRepRadom1),
                            (uint8 *)pRetBuff + strlen(pRetBuff), &nLen, hDev) < 0) {
        free(pRetBuff);
        return NULL;
    }
    strcat(pRetBuff, "&server-random=");
    strcat(pRetBuff, pRepRadom2);
    return pRetBuff;
}

int ServerIO(HHTTP hi, char *szQueryPath, uint8 *pContent, char *pRepBuf, int nRepBufLen)
{
    int nRet = -1, nChildRet = 0;
    while (1) {

        if (((nChildRet = HTTPPost(hi, szQueryPath, (char *)pContent, strlen((char *)pContent),
                                   pRepBuf, nRepBufLen)) < 0)) {
            _WriteLog(LL_WARN, "HTTPSPost post errror ret = %d", nChildRet);
            nRet = nChildRet;
            break;
        }

        if (strstr(pRepBuf, "SUCCESS") != pRepBuf && strstr(pRepBuf, "<?xml version=") != pRepBuf) {
            char szMsg[512] = "";
            G2U_GBKIgnore(pRepBuf, strlen(pRepBuf), szMsg, 512);
            _WriteLog(LL_WARN, "ServerIO failed logic reponse【%s】", szMsg);
            nRet = ERR_TA_REPONSE_CHECK;
            break;
        }
        nRet = RET_SUCCESS;
        break;
    }
    if (ERR_BUF_OVERFLOW == nRet)
        _WriteLog(LL_WARN, "ServerIO return buffer is too small");
    return nRet;
}

int ClientHello(HHTTP hi, char *pRepBuf, int nRepBufLen)
{
    char szPostBody[1024] = "";
    char szHelloString[768] = "";
    HDEV hDev = hi->hDev;
    if (GetTaClientHello(0, hDev, szHelloString) < 0)
        return ERR_TA_CLIENTHELLO;
    snprintf(szPostBody, sizeof(szPostBody), "type=client-hello&client-hello=%s", szHelloString);
    if (hDev->bDeviceType == DEVICE_CNTAX)
        strcat(szPostBody, "&certType=ecc");
    //_WriteHexToDebugFile("Hello.txt", szPostBody, strlen(szPostBody));
    int i = 0;
    int nRet = -1;
    int nChildRet = 0;
    for (i = 0; i < 3; i++) {
        if ((nChildRet =
                 ServerIO(hi, "/AuthLogin.auth", (uint8 *)szPostBody, pRepBuf, nRepBufLen)) < 0)
            break;
        //如果返回如下信息，这里有可能因为网络繁忙，导致握手建立不了，多试几次就行
        // ERROR@95@打开加密设备--网络通信错误
        if (!strncmp(pRepBuf, "ERROR@95@", 9)) {
            sleep(1);
            _WriteLog(LL_WARN, "ClientHello handleshake1 retry:%d", i);
            continue;
        }
        if (strstr(pRepBuf, "SUCCESS@") != pRepBuf) {
            sleep(1);
            _WriteLog(LL_WARN, "ClientHello handleshake2 retry:%d", i);
            continue;
        }
        nRet = 0;
        break;
    }
    return nRet;
}

int ClientAuthentication(HHTTP hi, char *pRepBuf, int nRepBufLen)
{
    char *pAuthCode = BuildAuthCode(pRepBuf, hi->hDev);
    if (!pAuthCode)
        return -1;
    int nRet = -2;
    int i = 0;
    for (i = 0; i < 3; i++) {
        memset(pRepBuf, 0, nRepBufLen);
        int nChildRet = ServerIO(hi, "/AuthLogin.auth", (uint8 *)pAuthCode, pRepBuf, nRepBufLen);
        if (nChildRet < 0) {
            if (strcmp(pRepBuf, "ERROR@98@证书已挂失") == 0) {
                nRet = ERR_TA_CERT_LOSS_REPORT;
                break;
            } else if (strcmp(pRepBuf, "ERROR@103@证书状态未知ph=-14202") == 0) {
                nRet = ERR_TA_CERT_STATUS_UNKNOWN;
                break;
            } else if (strcmp(pRepBuf, "ERROR@97@证书已作废ph=-14204") == 0) {
                nRet = ERR_TA_CERT_BEEN_CANCELED;
                break;
            }
            //如果返回如下信息，这里有可能因为网络繁忙，导致握手建立不了，多试几次就行
            break;
        }

        nRet = 0;
        break;
    }
    free(pAuthCode);
    return nRet;
}

//该函数作用仅用于向税局提交当前请求的企业信息，不做同步用
int SyncCompanyInfo(HHTTP hi, bool bNeedUpdateIO)
{
    int nRet = RET_SUCCESS;
    uint8 bDeviceType = hi->hDev->bDeviceType;
    do {
        if (DEVICE_AISINO == bDeviceType) {
            // aisino必须先同步企业信息才能进行后续操作
            if ((nRet = AisinoSyncCompanyInfo(hi, bNeedUpdateIO)) < 0)
                break;
        } else if (DEVICE_NISEC == bDeviceType) {
            if ((nRet = NisecSyncCompanyInfo(hi, bNeedUpdateIO)) < 0)
                break;
        } else if (DEVICE_CNTAX == bDeviceType) {
            // cntax企业信息同步之前有个固件升级检测CheckFirewareUpdate，此处忽略
            if ((nRet = CntaxSyncCompanyInfo(hi, bNeedUpdateIO)) < 0)
                break;
        } else {
            nRet = ERR_PROGRAME_API;
        }
    } while (false);
    return nRet;
}

// old:TrySyncAisinoWithTA
int SyncWithTA(HDEV hDev)
{
    struct HTTP hi;
    memset(&hi, 0, sizeof(struct HTTP));
    int nRet = -1;
    while (1) {
        if ((nRet = CreateServerTalk(&hi, hDev)) < 0)
            break;
        nRet = SyncCompanyInfo(&hi, true);
        if (nRet < 0)
            break;
        nRet = 0;
        _WriteLog(LL_INFO, "SyncWithTA all finish and successful");
        break;
    }
    HTTPClose(&hi);
    return nRet;
}

void ForceOtherProvinces(uint8 bServType, char *szRegCode, struct UploadAddrModel *model)
{
    // int nRet = GetTaServerURL(bServType, "530000", model);  //云南
    // int nRet = GetTaServerURL(bServType, "370000", model);  //山东
    // int nRet = GetTaServerURL(bServType, "420000", model);  //湖北
    // int nRet = GetTaServerURL(bServType, "120000", model);  //天津
    int nRet = GetTaServerURL(bServType, "310000", model);  //上海
    _WriteLog(LL_WARN, "Force upserver redirect to 【%s %s】 %s", model->szRegName,
              model->szTaxAuthorityURL, nRet < 0 ? "Failed" : "Successful");
}

//连接发票上传和开票业务服务器
int CreateServerTalk(HHTTP hi, HDEV hDev)
{
	char tmp_s[100] = {0};
	ClearLastError(hDev->hUSB);
	memset(&hDev->DevHttp, 0, sizeof(struct DevHttpConnectInfo));
	hDev->DevHttp.start_time = get_time_msec();
	timer_read_asc_ms(tmp_s);
	memset(hDev->DevHttp.start_time_s, 0, sizeof(hDev->DevHttp.start_time_s));
	memcpy(hDev->DevHttp.start_time_s, tmp_s + 1, 23);
    int nRet = -4;
    int nRepBufSize = 512000;
    memset(hi, 0, sizeof(struct HTTP));
    if (GetTaServerURL(hDev->bDeviceType == DEVICE_CNTAX ? TAADDR_UPLOAD_GM : TAADDR_UPLOAD_RSA,
                       hDev->szRegCode, &hi->modelTaAddr) != RET_SUCCESS) {
        _WriteLog(LL_WARN, "GetUPServerURL failed");
        SetLastError(hDev->hUSB, ERR_TA_GET_SERVERADDR, "区域代码：%s,无法获取上传地址",
                     hDev->szRegCode);
        return -1;
    }
#ifdef FORCE_OTHER_PROVINCES  //外省连接检测
    ForceOtherProvinces(hDev->bDeviceType == DEVICE_CNTAX ? TAADDR_UPLOAD_GM : TAADDR_UPLOAD_RSA,
                        hDev->szRegCode, &hi->modelTaAddr);
#endif
    _WriteLog(LL_INFO, "Get taxauthority ServerURL OK, value:【%s %s】", hi->modelTaAddr.szRegName,
              hi->modelTaAddr.szTaxAuthorityURL);
    if (strncmp(hi->modelTaAddr.szTaxAuthorityURL, "https://", 8)) {
        _WriteLog(LL_WARN, "Upload by http don't support");
        return -2;
    }
    char *response = calloc(1, nRepBufSize);
    if (!response)
        return -3;
    int nChildRet = 0;
    while (1) {
        if ((nChildRet = HTTPInit(hi, hDev)) < 0) {
            _WriteLog(LL_WARN, "HTTPS Init failed, nRet:%d", nChildRet);
            SetLastError(hDev->hUSB, ERR_TA_HTTPS_INIT, "税盘与服务器交互失败,HTTPS初始化失败");
            nRet = -100;
            break;
        }
        _WriteLog(LL_DEBUG, "Device:%s,HTTPSInit successful", hDev->szDeviceID);
        if ((nChildRet = HTTPOpen(hi)) < 0) {
            _WriteLog(LL_WARN, "HTTPOpen failed, nRet:%d", nChildRet);
            SetLastError(hDev->hUSB, ERR_TA_HTTPS_OPEN, "税盘与服务器交互失败,HTTPS sock 连接失败");
            nRet = -200;
            break;
        }
        if ((nChildRet = ClientHello(hi, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "ClientHello failed, nRet:%d", nChildRet);
            SetLastError(hDev->hUSB, ERR_TA_HTTPS_CLIENT_HELLO,
                         "税盘与服务器交互失败,ClientHello失败");
            break;
        }
        if ((nChildRet = ClientAuthentication(hi, response, nRepBufSize)) < 0) {
            _WriteLog(LL_WARN, "ClientAuthentication failed, nRet:%d", nChildRet);

            if (nChildRet == ERR_TA_CERT_LOSS_REPORT) {
                SetLastError(hDev->hUSB, nChildRet, "税盘与服务器交互失败,ERROR@98@证书已挂失");
                nRet = nChildRet;
            } else if (nChildRet == ERR_TA_CERT_STATUS_UNKNOWN) {
                SetLastError(hDev->hUSB, nChildRet, "税盘与服务器交互失败,ERROR@103@证书状态未知");
                nRet = nChildRet;
            } else if (nChildRet == ERR_TA_CERT_BEEN_CANCELED) {
                SetLastError(hDev->hUSB, nChildRet, "税盘与服务器交互失败,ERROR@97@证书已作废");
                nRet = nChildRet;
            } else {
                SetLastError(hDev->hUSB, ERR_TA_HTTPS_CLIENT_AUTH,
                             "税盘与服务器交互失败,ClientAuthentication失败");
            }
            break;
        }
        //!!!企业信息同步包含大量IO，强烈建议不要在发票上传、税局查询等场合作出同步操作，闲时进行更新同步
        //企业信息同步过程必须包含在握手过程中且在所有税局操作之前，后续操作无效
        //此处的bNeedUpdateIO不在此处同步，可以在空闲时调用该接口进行同步监控信息并反写设备
        //第二个参数不要置true，否则SyncWithTA函数无意义，重复执行2次同步操作
        if ((nChildRet = SyncCompanyInfo(hi, false)) < 0) {
            _WriteLog(LL_WARN, "SyncCompanyInfo failed, nRet:%d", nChildRet);
            SetLastError(hDev->hUSB, ERR_TA_REPONSE_FAILE,
                         "税盘与服务器交互失败,SyncCompanyInfo失败");
            break;
        }
        _WriteLog(LL_INFO, "盘号：%s,HTTPOpen HTTPS连接及认证全部完成\n", hDev->szDeviceID);
        nRet = 0;
        break;
    }
    free(response);
    return nRet;
}
