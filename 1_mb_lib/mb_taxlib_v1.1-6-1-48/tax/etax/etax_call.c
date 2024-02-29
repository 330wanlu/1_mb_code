/*****************************************************************************
File name:   etax_call.c
Description: 用于etax模块中接口调用示例
Author:      Zako
Version:     1.0
Date:        2023.02
History:
*****************************************************************************/
#include "etax_call.h"
#include "../common/console.h"

#include "jiangsu_call.h"
#include "shanghai_call.h"
#include "shanxi_call.h"
#include "henan_call.h"
#include "hubei_call.h"
#include "sichuan_call.h"
#include "qingdao_call.h"
#include "chongqing_call.h"
#include "beijing_call.h"
#include "anhui_call.h"
#include "hunan_call.h"
#include "guangxi_call.h"
#include "xinjiang_call.h"
#include "guangdong_call.h"
#include "jiangxi_call.h"
#include "ningbo_call.h"
#include "hainan_call.h"
#include "fujian_call.h"
#include "guizhou_call.h"
#include "gansu_call.h"
#include "shandong_call.h"
#include "hebei_call.h"
#include "zhejiang_call.h"
#include "shaanxi_call.h"

ETAX_STRUCT etax_array[] = {
    {"31",&etax_shanghai_fuction},//8
    {"32",&etax_jiangsu_fuction},//7
    {"14",&etax_shanxi_fuction},//6
    {"41",&etax_henan_fuction},
    {"42",&etax_hubei_fuction},
    {"51",&etax_sichuan_fuction},
    {"3702",&etax_qingdao_fuction},
    {"50",&etax_chongqing_fuction},//5
    {"11",&etax_beijing_fuction},
    {"34",&etax_anhui_fuction},
    {"45",&etax_guangxi_fuction},
    {"43",&etax_hunan_fuction},
    {"65",&etax_xinjiang_fuction},//4
    {"44",&etax_guangdong_fuction},//3
    {"36",&etax_jiangxi_fuction},   //2
    {"3302",&etax_ningbo_fuction},    //1
    {"46",&etax_hainan_fuction},//9
    {"35",&etax_fujian_fuction},//10
    {"52",&etax_guizhou_fuction},//11
    {"62",&etax_gansu_fuction},//12  盘和软证是暂时都不行
    {"37",&etax_shandong_fuction},//12
    {"13",&etax_hebei_fuction},//13
    {"33",&etax_zhejiang_fuction},//14
    {"61",&etax_shaanxi_fuction},//15
};

ETAX_FUCTION *get_etax_function(char *szRegCode)
{
    int i = 0;
    for(i = 0;i<sizeof(etax_array)/sizeof(ETAX_STRUCT);++i)
    {
        if(0 == strcmp(etax_array[i].szRegCode,szRegCode))
        {
            return (etax_array[i].etax_fuction);
        }
    }
    return NULL;
}

int EtaxGetBusDevId(int *nBusID, int *nDevID,char *busid)
{
    bool ret = false;
    do {
        char szStr[4096] = "";  // ReadBinFile 读取大小4k
        char szMountPath[256] = "";
        strcpy(szStr, busid);
        char *p = NULL;
        if ((p = strchr(szStr, ':'))) {
            // busid:devid
            *p = '\0';
            *nBusID = atoi(szStr);
            *nDevID = atoi(p + 1);
            ret = true;
            break;
        }
        if (!(p = strchr(szStr, '.'))) {
            _WriteLog(LL_FATAL, "Error mount path1");
            break;
        }
        // mount path
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/busnum", busid);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFile(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            _WriteLog(LL_FATAL, "Read mount path failed :%s", szMountPath);
            break;
        }
        *nBusID = atoi(szStr);  //末尾有个\n不影响
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/devnum", busid);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFile(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            _WriteLog(LL_FATAL, "Read mount path failed :%s", szMountPath);
            break;
        }
        *nDevID = atoi(szStr);  //末尾有个\n不影响
        if (!*nBusID || !*nDevID) {
            _WriteLog(LL_FATAL, "Error mount path2");
            break;
        }
        ret = true;
    } while (false);
    #ifdef MB2201
    ret = true;
    #endif
    return ret;
}

/********************************全电接口***********************************/
//一键登录
int etax_login_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    ETAX_LOGIN_PARAMETER parameter = {
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .lpgoto = lpgoto,
        .lpcode = lpcode,
        .lptoken = lptoken,
        .sys_type = sys_type,
        .svr_container = svr_container,
        .soft_cert_ip = soft_cert_ip,
        .soft_cert_port = soft_cert_port,
        .login_method = login_method,
        .area_code = area_code
    };


    if(0 == memcmp(parameter.login_method,"02",2))
    {
        if(!EtaxGetBusDevId(&nBusID,&nDevID,busid))
        {
            sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
        }
    }
   
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();

        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        if(0 == memcmp(parameter.login_method,"02",2))   //数字证书
        {
            nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
            if (nDeviceType < 0) {
                sprintf((char *)errinfo_uft8, "306设备初始化失败");
                return nDeviceType;
            }
        }
        else if(0 == memcmp(parameter.login_method,"01",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }
        else if(0 == memcmp(parameter.login_method,"21",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }

        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                    sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                }
                strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                printf("szRegCode = %s\r\n",etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                    break;
                }
                else
                {
                    nChildRet = etax_function->etax_login(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                    if(nChildRet >= 0){
                        CtxBridge ctx = (CtxBridge)etaxConn.tpass.ctxApp;

                        strcat(lptoken,";new_key16:");
                        strcat(lptoken,ctx->szRandom16BytesKey);
                        //printf("szRandom16BytesKey=%s\r\n%d\r\n",ctx->szRandom16BytesKey,strlen(ctx->szRandom16BytesKey));

                        strcat(lptoken,";naturepublicKey:");
                        strcat(lptoken,ctx->sPublickKey);
                        //printf("sPublickKey=%s\r\n%d\r\n",ctx->sPublickKey,strlen(ctx->sPublickKey));

                        strcat(lptoken,";natureuuid:");
                        strcat(lptoken,ctx->szUuid);
                        //printf("szUuid=%s\r\n%d\r\n",ctx->szUuid,strlen(ctx->szUuid));

                        nRet = RET_SUCCESS;
                    }
                }
                //释放内存关闭设备
                if(0 == memcmp(parameter.login_method,"01",2) || 0 == memcmp(parameter.login_method,"21",2))
                {
                    etaxConn.tpass.hDev=NULL;
                    etaxConn.etax.hDev=NULL;
                    etaxConn.dppt.hDev=NULL;
                }
                CloseEtaxConnections(&etaxConn);  
                ConsoleClose(nDeviceType, hUSB);
                FreeUSBSession(hUSB);
                FreeDevInfo(hDev);
                ProgInitDestory(0);   
            } while (false);          
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "login:error:", "%s\r\n",errinfo);
    }
    return nRet;
}

//用户信息获取
int etax_user_query_Interface(char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    ETAX_USER_QUERY_PARAMETER parameter = {
        .outinfo = outinfo,
        
        .svr_container = svr_container,
        .soft_cert_ip = soft_cert_ip,
        .soft_cert_port = soft_cert_port,
        .login_method = login_method,
        .area_code = area_code
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(0 == memcmp(parameter.login_method,"02",2))
    {
        if(!EtaxGetBusDevId(&nBusID,&nDevID,busid))
        {
            sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
        }
    }

    HDEV hDev = MallocDevInfo();
    HUSB hUSB = MallocUSBSession();
    
    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    //步骤2 打开USB设备
    if(0 == memcmp(parameter.login_method,"02",2))   //数字证书
    {
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
    }
    else if(0 == memcmp(parameter.login_method,"01",2))  //软证书
    {
        strcpy(hDev->szRegCode,parameter.area_code);
    }

    else if(0 == memcmp(parameter.login_method,"21",2))  //软证书
    {
        strcpy(hDev->szRegCode,parameter.area_code);
    }



    do{
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        printf("szRegCode = %s\r\n",etaxConn.etax.modelTaAddr.szRegCode);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);

        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
            break;
        }
        else
        {
            nChildRet = etax_function->etax_user_query(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
            if(nChildRet >= 0)
                nRet = RET_SUCCESS;
        }
    }while (false);
    //释放内存关闭设备
    if(0 == memcmp(parameter.login_method,"01",2) || 0 == memcmp(parameter.login_method,"21",2))
    {
        etaxConn.tpass.hDev=NULL;
        etaxConn.etax.hDev=NULL;
        etaxConn.dppt.hDev=NULL;
    }
    CloseEtaxConnections(&etaxConn);  
    ConsoleClose(nDeviceType, hUSB);
    FreeUSBSession(hUSB);
    FreeDevInfo(hDev);
    ProgInitDestory(0);             
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "user_query:error:", "%s\r\n",errinfo);
    }
    return nRet;
}

//企业查询
int etax_relationlist_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    ETAX_ENTERPRISELIST_PARAMETER parameter = {
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .outenterpriselist = outenterpriselist,

        .svr_container = svr_container,
        .soft_cert_ip = soft_cert_ip,
        .soft_cert_port = soft_cert_port,
        .login_method = login_method,
        .area_code = area_code
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(0 == memcmp(parameter.login_method,"02",2))
    {
        if(!EtaxGetBusDevId(&nBusID,&nDevID,busid))
        {
            sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
        }
    }

    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();

        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        if(0 == memcmp(parameter.login_method,"02",2))   //数字证书
        {
            nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
            if (nDeviceType < 0) {
                sprintf((char *)errinfo_uft8, "306设备初始化失败");
                return nDeviceType;
            }
        }
        else if(0 == memcmp(parameter.login_method,"01",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }
        else if(0 == memcmp(parameter.login_method,"21",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }

        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                    //break;
                }
                strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                printf("szRegCode = %s\r\n",etaxConn.etax.modelTaAddr.szRegCode);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                    break;
                }
                else
                {
                    nChildRet = etax_function->etax_enterpriselist(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                    if(nChildRet >= 0)
                        nRet = RET_SUCCESS;
                }


            } while (false);
            //释放内存关闭设备
            if(0 == memcmp(parameter.login_method,"01",2) || 0 == memcmp(parameter.login_method,"21",2))
            {
                etaxConn.tpass.hDev=NULL;
                etaxConn.etax.hDev=NULL;
                etaxConn.dppt.hDev=NULL;
            }
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);             
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "relationlist:error:", "%s\r\n",errinfo);
    }
    return nRet;
}

//企业切换
int etax_relationchange_Interface(char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    ETAX_RELATIONCHANGE_PARAMETER parameter = {
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .lpgoto = lpgoto,
        .lpcode = lpcode,
        .lptoken = lptoken,
        .sys_type = sys_type,
        
        .svr_container = svr_container,
        .soft_cert_ip = soft_cert_ip,
        .soft_cert_port = soft_cert_port,
        .login_method = login_method,
        .area_code = area_code
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(0 == memcmp(parameter.login_method,"02",2))
    {
        if(!EtaxGetBusDevId(&nBusID,&nDevID,busid))
        {
            sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
        }
    }

    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();

        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        if(0 == memcmp(parameter.login_method,"02",2))   //数字证书
        {
            nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
            if (nDeviceType < 0) {
                sprintf((char *)errinfo_uft8, "306设备初始化失败");
                return nDeviceType;
            }
        }
        else if(0 == memcmp(parameter.login_method,"01",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }
        else if(0 == memcmp(parameter.login_method,"21",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }


        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                    sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                    //break;
                }
                strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                printf("szRegCode = %s\r\n",etaxConn.etax.modelTaAddr.szRegCode);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306获取税局地址失败,该省份暂未兼容,目前兼容的城市:上海市、江苏省、山西省、河南省、四川省、青岛市、北京市、安徽省、广西省、湖南省、湖北省、重庆市");
                    break;
                }
                else
                {
                    nChildRet = etax_function->etax_relationchange(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                    if(nChildRet >= 0){
                        CtxBridge ctx = (CtxBridge)etaxConn.tpass.ctxApp;

                        strcat(lptoken,";new_key16:");
                        strcat(lptoken,ctx->szRandom16BytesKey);
                        //printf("szRandom16BytesKey=%s\r\n%d\r\n",ctx->szRandom16BytesKey,strlen(ctx->szRandom16BytesKey));

                        strcat(lptoken,";naturepublicKey:");
                        strcat(lptoken,ctx->sPublickKey);
                        //printf("sPublickKey=%s\r\n%d\r\n",ctx->sPublickKey,strlen(ctx->sPublickKey));

                        strcat(lptoken,";natureuuid:");
                        strcat(lptoken,ctx->szUuid);
                        //printf("szUuid=%s\r\n%d\r\n",ctx->szUuid,strlen(ctx->szUuid));

                        nRet = RET_SUCCESS;
                    }
                }

            } while (false);
            //释放内存关闭设备
            if(0 == memcmp(parameter.login_method,"01",2) || 0 == memcmp(parameter.login_method,"21",2))
            {
                etaxConn.tpass.hDev=NULL;
                etaxConn.etax.hDev=NULL;
                etaxConn.dppt.hDev=NULL;
            }
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);             
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "relationchange:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票查询
int etax_invoice_query_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_invoice_query == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_invoice_query(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票查询详情
int etax_invoice_query_detail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_DETAIL parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_invoice_query_detail == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_invoice_query_detail(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_detail:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票查询下载
int etax_invoice_query_down_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_DOWN parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata,
        .fpcxname = fpcxname
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_invoice_query_down == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_invoice_query_down(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_down:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票开具RPA扫码接口
int etax_rpa_invoice_issuance_Interface(char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    RPA_INVOICE_ISSUANCE parameter = {
        .fpkjurl = fpkjurl,
        .nsrsbh = nsrsbh,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpkjdata = fpkjdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_rpa_invoice_issuance == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_rpa_invoice_issuance(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "rpa_invoice:error:", "%s\r\n",errinfo);
    }
    return nRet;        
}

//rpa by_cookies
int etax_invoice_rpa_invoice_issuance_status_by_cookie_Interface(char *area_code,char *rzid,char *app_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjurl,char *nsrsbh,char **fpkjdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    RPA_INVOICE_ISSUANCE_STATUS_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .nsrsbh = nsrsbh,
        .fpkjurl = fpkjurl,
        .fpkjdata = fpkjdata,
        .rzid = rzid,
        .app_code = app_code
    };

    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }
        
    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            //break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_rpa_invoice_issuance_status_by_cookies == NULL){
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else 
            {
                nChildRet = etax_function->etax_rpa_invoice_issuance_status_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_down by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票开具RPA扫码接口【状态查询】接口
int etax_rpa_invoice_issuance_status_Interface(char *busid,char *rzid,char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    RPA_INVOICE_STATUS_ISSUANCE parameter = {
        .fpkjurl = fpkjurl,
        .nsrsbh = nsrsbh,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpkjdata = fpkjdata,
        .rzid = rzid,
        .app_code = app_code
    };
    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_rpa_invoice_issuance_status == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_rpa_invoice_issuance_status(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "rpa_invoice:error:", "%s\r\n",errinfo);
    }
    return nRet;        
}

int etax_credit_limit_Interface(char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    CREDIT_LIMIT parameter = {
        .nsrsbh = nsrsbh,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .sxeddata = sxeddata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_credit_limit == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_credit_limit(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "credit_limit:error:", "%s\r\n",errinfo);
    }
    return nRet;        
}

//蓝字发票开具
int etax_blue_ink_invoice_issuance_Interface(char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    BLUE_INK_INVOICE_ISSUANCE parameter = {
        .fpkjjson = fpkjjson,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpkjout = fpkjout,
        .hqnsrjcxx = hqnsrjcxx
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    nChildRet = etax_function->etax_blue_ink_invoice_issuance(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                    if(nChildRet >= 0)
                        nRet = RET_SUCCESS;
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "blue_ink_invoice:error:", "%s\r\n",errinfo);
    }
    return nRet;        
}

//可开红票的蓝票查询
int etax_hzqrxxSzzhQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_hzqrxxSzzhQuery == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_hzqrxxSzzhQuery(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxSzzhQuery:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单开具
int etax_hzqrxxSave_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_hzqrxxSave == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_hzqrxxSave(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxSave:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单查询
int etax_hzqrxxQuery_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_hzqrxxQuery == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_hzqrxxQuery(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxQuery:error:", "读取失败:%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单详情查询
int etax_hzqrxxQueryDetail_Interface(char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .full_name = full_name,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_hzqrxxQueryDetail == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_hzqrxxQueryDetail(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxQueryDetail:error:", "失败:%s\r\n",errinfo);
    }
    return nRet;    
}

int etax_hcstaquery_invoice_issuance_Interface(char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_HCSTAQUERY parameter = {
        .fpcxsj = fpcxsj,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_hcstaquery_invoice_issuance == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_hcstaquery_invoice_issuance(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hcstaquery:error:", "读取失败:%s\r\n",errinfo);
    }
    return nRet;    
}

int etax_fjxxpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    FJXXCJMB_MODE parameter = {
        .fpcxsj = fpcxsj,
        .mode = mode,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_fjxxpeizhi_query == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_fjxxpeizhi_query(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "fjxxpeizhi:error:", ":%s\r\n",errinfo);
    }
    return nRet;    
}

int etax_cjmbpeizhi_Interface(char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    FJXXCJMB_MODE parameter = {
        .fpcxsj = fpcxsj,
        .mode = mode,
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .fpcxdata = fpcxdata
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(!EtaxGetBusDevId(&nBusID,&nDevID,busid)){
        sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
    }
    else 
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
        if (nDeviceType < 0) {
            sprintf((char *)errinfo_uft8, "306设备初始化失败");
            return nDeviceType;
        }
        else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_cjmbpeizhi_query == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_cjmbpeizhi_query(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                            nRet = RET_SUCCESS;
                    }
                }
            } while (false);
            //释放内存关闭设备
            CloseEtaxConnections(&etaxConn);  
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "cjmbpeizhi:error:", ":%s\r\n",errinfo);
    }
    return nRet;    
}

//剥离后的接口
//获取cookie
int etax_get_cookie_Interface(char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    GET_COOKIE parameter = {
        .related_type = related_type,
        .mobile = mobile,
        .idcard = idcard,
        .sz_password = sz_password,
        .realationtype = realationtype,
        .uniqueIdentity = uniqueIdentity,
        .realationStatus = realationStatus,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .svr_container = svr_container,
        .soft_cert_ip = soft_cert_ip,
        .soft_cert_port = soft_cert_port,
        .area_code = area_code,
        .login_method = login_method,
        .temp_data = temp_data,
    };

    //步骤1 使用busid取出设备的nBusID和nDevID
    if(0 == memcmp(parameter.login_method,"02",2))
    {
        if(!EtaxGetBusDevId(&nBusID,&nDevID,busid))
        {
            sprintf((char *)errinfo_uft8, "306接口设备未找到busid");
        }
    }
    
    {
        HDEV hDev = MallocDevInfo();
        HUSB hUSB = MallocUSBSession();
        
        if (ProgInitDestory(1) < 0) {
            sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
            return nRet;
        }

        //步骤2 打开USB设备
        if(0 == memcmp(parameter.login_method,"02",2))
        {
            nDeviceType = DetectDeviceInit(hDev, hUSB, true, nBusID, nDevID);
            if (nDeviceType < 0) {
                sprintf((char *)errinfo_uft8, "306设备初始化失败");
                return nDeviceType;
            }
        }
        else if(0 == memcmp(parameter.login_method,"21",2))  //软证书
        {
            strcpy(hDev->szRegCode,parameter.area_code);
        }
       // else 
        {
            do {
                //步骤3 判断当前盘的省份
                if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
                   sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    //break;
                }
				strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
                strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
                etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
                if(NULL == etax_function)
                {
                    sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                    break;
                }
                else
                {
                    if(etax_function->etax_get_cookie == NULL){
                        sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                        break;                        
                    }
                    else 
                    {
                        nChildRet = etax_function->etax_get_cookie(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                        if(nChildRet >= 0)
                        {
                            CtxBridge ctx = (CtxBridge)etaxConn.tpass.ctxApp;

                            strcat(temp_data,";new_key16:");
                            strcat(temp_data,ctx->szRandom16BytesKey);
                            printf("szRandom16BytesKey=%s\r\n%d\r\n",ctx->szRandom16BytesKey,strlen(ctx->szRandom16BytesKey));

                            strcat(temp_data,";naturepublicKey:");
                            strcat(temp_data,ctx->sPublickKey);
                            printf("sPublickKey=%s\r\n%d\r\n",ctx->sPublickKey,strlen(ctx->sPublickKey));

                            strcat(temp_data,";natureuuid:");
                            strcat(temp_data,ctx->szUuid);
                            printf("szUuid=%s\r\n%d\r\n",ctx->szUuid,strlen(ctx->szUuid));

                            nRet = RET_SUCCESS;
                        }
                    }
                }
            } while (false);
            //释放内存关闭设备
            if(0 == memcmp(parameter.login_method,"21",2))
            {
                etaxConn.tpass.hDev=NULL;
                etaxConn.etax.hDev=NULL;
                etaxConn.dppt.hDev=NULL;
            }
            CloseEtaxConnections(&etaxConn);
            ConsoleClose(nDeviceType, hUSB);
            FreeUSBSession(hUSB);
            FreeDevInfo(hDev);
            ProgInitDestory(0);
        }
    }
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "get cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

int etax_get_nsrjcxx_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    NRSXXCX_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_get_nsrjcxx_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_get_nsrjcxx_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "etax_get_nsrjcxx_by_cookies:error", "%s\r\n",errinfo);
    }
    return nRet;     
}

int etax_get_nsrfxxx_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *errinfo,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    NRSXXCX_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_get_nsrfxxx_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_get_nsrfxxx_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "etax_get_nsrfxxx_by_cookies:error", "%s\r\n",errinfo);
    }
    return nRet;     
}

//发票列表查询by_cookies
int etax_invoice_query_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_invoice_query_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_invoice_query_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票详情查询by_cookies
int etax_invoice_query_detail_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_invoice_query_detail_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_invoice_query_detail_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_detail by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//发票下载by_cookies
int etax_invoice_query_down_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char *errinfo,char **fpcxdata,char **fpcxname)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_DOWN_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata,
        .fpcxname = fpcxname
    };

    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }
        
    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            //break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_invoice_query_down_by_cookies == NULL){
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else 
            {
                nChildRet = etax_function->etax_invoice_query_down_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_down by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//rpa by_cookies
int etax_invoice_rpa_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjurl,char *nsrsbh,char **fpkjdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    RPA_INVOICE_ISSUANCE_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .nsrsbh = nsrsbh,
        .fpkjurl = fpkjurl,
        .fpkjdata = fpkjdata
    };

    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }
        
    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            //break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_rpa_invoice_issuance_by_cookies == NULL){
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else 
            {
                nChildRet = etax_function->etax_rpa_invoice_issuance_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "invoice_query_down by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//授信额度查询by_cookies
int etax_credit_limit_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *nsrsbh,char **sxeddata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    CREDIT_LIMITB_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .nsrsbh = nsrsbh,
        .sxeddata = sxeddata
    };

    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }
        
    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            //break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_credit_limit_by_cookies == NULL){
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else 
            {
                nChildRet = etax_function->etax_credit_limit_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "credit_limit by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红冲类查询by_cookies
int etax_hcstaquery_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char **fpcxdata)
{
    struct EtaxConnections etaxConn = {};

    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;

    char errinfo_uft8[1024]={0};
    strcpy(errinfo_uft8,"306接口请求失败,请检查请求参数后再次请求");

    ETAX_FUCTION *etax_function = NULL;
    INVOICE_HCSTAQUERY_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };

    HDEV hDev = MallocDevInfo();
    printf("etax_hcstaquery_invoice_issuance_by_cookie_Interface\n");
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }
        
    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0) {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            //break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_hcstaquery_invoice_issuance_by_cookies == NULL){
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else 
            {
                nChildRet = etax_function->etax_hcstaquery_invoice_issuance_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    ProgInitDestory(0);
        
    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hcstaquery_invoice_issuance by cookie", "读取失败:%s\r\n",errinfo);
    }
    return nRet;    
}

//蓝票开具 by cookie
int etax_blue_ink_invoice_issuance_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpkjjson,char **fpkjout,char **hqnsrjcxx)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    BLUE_INK_INVOICE_ISSUANCE_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpkjjson = fpkjjson,
        .fpkjout = fpkjout,
        .hqnsrjcxx = hqnsrjcxx
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        strncpy(etaxConn.dppt.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_blue_ink_invoice_issuance_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_blue_ink_invoice_issuance_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
    CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "blue_ink_invoice_issuance by cookie:error:", "%s\r\n",errinfo);
    }
    return nRet;    
}

//可开红票的蓝票查询 by cookie
int etax_hzqrxxSzzhQuery_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_hzqrxxSzzhQuery_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_hzqrxxSzzhQuery_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxSzzhQuery by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单开具 by cookie
int etax_hzqrxxSave_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_hzqrxxSave_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_hzqrxxSave_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxSave by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单列表查询 by cookie
int etax_hzqrxxQuery_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_hzqrxxQuery_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_hzqrxxQuery_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxQuery by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}

//红字确认单详情查询 by cookie
int etax_hzqrxxQueryDetail_by_cookies_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *fpcxsj,char **fpcxdata,char *errinfo)
{
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    INVOICE_QUERY_BY_COOKIES parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_hzqrxxQueryDetail_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_hzqrxxQueryDetail_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "hzqrxxQueryDetail by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}

int etax_fjxxpeizhi_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char *mode,char **fpcxdata)
{  
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    FJXXCJMB_MODE_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .mode = mode,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_fjxxpeizhi_query_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_fjxxpeizhi_query_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "fjxxpeizhi by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}   

int etax_cjmbpeizhi_by_cookie_Interface(char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,char *errinfo,char *fpcxsj,char *mode,char **fpcxdata)
{  
    struct EtaxConnections etaxConn = {};
    int nRet = ERR_GENERIC, nChildRet = 0, nDeviceType = 0;
    int nBusID = 0 ,nDevID = 0;
    char errinfo_uft8[1024] = {0x00};
    ETAX_FUCTION *etax_function = NULL;
    FJXXCJMB_MODE_BY_COOKIE parameter = {
        .area_code = area_code,
        .etax_cookie = etax_cookie,
        .tpass_cookie = tpass_cookie,
        .dppt_cookie = dppt_cookie,
        .new_etax_cookie = new_etax_cookie,
        .new_tpass_cookie = new_tpass_cookie,
        .new_dppt_cookie = new_dppt_cookie,
        .mode = mode,
        .fpcxsj = fpcxsj,
        .fpcxdata = fpcxdata
    };
    HDEV hDev = MallocDevInfo();
    //为了保持原有接口(EtaxGetTaServerURL)不变
    strncpy(hDev->szRegCode,area_code,(sizeof(hDev->szRegCode)>=strlen(area_code)?strlen(area_code):sizeof(hDev->szRegCode)));

    if (ProgInitDestory(1) < 0) {
        sprintf((char *)errinfo_uft8, "306初始化SSL库或者校验算法失败");
        return nRet;
    }

    do {
        //步骤3 判断当前盘的省份
        if ((nChildRet = EtaxGetEtaxServerURL(&etaxConn.etax, hDev)) < 0)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        strncpy(etaxConn.etax.modelTaAddr.szRegCode,hDev->szRegCode,2);
        etax_function = get_etax_function(etaxConn.etax.modelTaAddr.szRegCode);
        if(NULL == etax_function)
        {
            sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
            break;
        }
        else
        {
            if(etax_function->etax_cjmbpeizhi_query_by_cookies == NULL)
            {
                sprintf((char *)errinfo_uft8, "306该省份暂未兼容,目前兼容的城市:上海市、江苏省、河南省");
                break;                        
            }
            else
            {
                nChildRet = etax_function->etax_cjmbpeizhi_query_by_cookies(&etaxConn,hDev,(void *)(&parameter),errinfo_uft8);
                if(nChildRet >= 0)
                    nRet = RET_SUCCESS;
            }
        }
    } while (false);
    //释放内存关闭设备
    FreeDevInfo(hDev);
    etaxConn.etax.hDev = NULL;
    etaxConn.dppt.hDev = NULL;
    etaxConn.tpass.hDev = NULL;
CloseEtaxConnections(&etaxConn);
    //FreeDevInfo(hDev);
    ProgInitDestory(0);

    if(nRet != RET_SUCCESS){
        EncordingConvert( "utf-8","gbk",errinfo_uft8, strlen(errinfo_uft8), errinfo, strlen(errinfo_uft8));
        logout(INFO, "TAXLIB", "fjxxpeizhi by cookie:error", "%s\r\n",errinfo);
    }
    return nRet;    
}   