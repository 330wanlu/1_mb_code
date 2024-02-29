/*****************************************************************************
File name:   aisino_usbio.c
Description: 用于爱信诺盘的设备USB协议操作
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     最初代码实现
20220630     格式化协议错误
*****************************************************************************/
#include "aisino_usbio.h"

uint8 g_CmdE1AA[7] = "\xfe\xff\xcc\xbc\x53\xe1\xaa";
uint8 g_CmdE1AB[7] = "\xfe\xff\xcc\xbc\x53\xe1\xab";

uint16 GetDeviceRepCode(uint8 *pAARep)
{
    uint8 *v13 = pAARep;
    int v17;
    int v18;
    LOWORD(v17) = *(_BYTE *)v13;
    LOWORD(v18) = *(_BYTE *)(v13 + 1);
    int v19 = v18 + (v17 << 8);
    return (uint16)v19;
}

int AisinoVendorErrorCodeToDescription(int wRepCode, char *describe)
{
    int nFormatErrorCode = ERR_AISINO_COMMON;
    switch (wRepCode) {
        case 83:
            nFormatErrorCode = ERR_AISINO_QUERYTIME_LATER;
            strcpy(describe, "查询时间大于当前时间！");
            break;
        case 84:
            nFormatErrorCode = ERR_AISINO_QUERYTIME_EARLY;
            strcpy(describe, "查询时间过早");
            break;
        case 87:
            nFormatErrorCode = ERR_AISINO_INVOICEVOLUME_TIME;
            strcpy(describe, "金税设备中发票卷时间错！");
            break;
        case 99:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_DATA;
            strcpy(describe, "开票数据有问题！");
        case 161:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_MAX;
            strcpy(describe, "可开发票最大数量已满！");
            break;
        case 220:
            nFormatErrorCode = ERR_AISINO_NO_DATA;
            strcpy(describe, "检索函数出错或者检索不到数据！");
            break;
        case 223:
            nFormatErrorCode = ERR_AISINO_NO_STOCK;
            strcpy(describe, "检索不到指定月份领用存信息");
            break;
        case 246:
            nFormatErrorCode = ERR_AISINO_MAKEINVOICE_DEADLINE;
            strcpy(describe, "已到上传截止日期，上月有已开未上传发票，不能继续开票！");
            break;
        case 248:
            nFormatErrorCode = ERR_AISINO_NEED_REPORT;
            strcpy(describe, "有票种没有完成征期抄报税，不能继续开票");
            break;
        case 1521:
            nFormatErrorCode = ERR_AISINO_UNKNOW;
            strcpy(describe, "异常不允许开票");
            break;
        default:
            sprintf(describe, "设备底层错误，号码:%d", wRepCode);
            break;
    }
    //_WriteLog(LL_WARN, "AisinoLogicIO error, code:%d desc:%s", wRepCode, describe);
    return nFormatErrorCode;
}

//输入包括pInOutCmd和nInCmdLen、nInCmdBufLen负责第一个子IO的输入,pTransferData和nTransferDataLen负责第二个子IO输入
//整个函数的输出包括pInOutCmd和nRet，一个为缓冲区，一个为返回数值长度
int AisinoLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                  uint8 *pTransferData, int nTransferDataLen)
{
    int nRetChild = 0;
    //--send command
    if ((nRetChild =
             mass_storage_io(hUSB, pInOutCmd, nInCmdLen, 0, MSC_DIR_H2D, (uint8 *)pTransferData,
                             nTransferDataLen, pInOutCmd, nInCmdBufLen)) < 0) {
        _WriteLog(LL_DEBUG, "[AisinoLogicIO] Command send failed\r\n");
        return -1;
    }
    //--recv header
    if ((nRetChild = mass_storage_io(hUSB, g_CmdE1AA, sizeof g_CmdE1AA, 11, MSC_DIR_D2H, 0, 0,
                                     pInOutCmd, nInCmdBufLen)) < 0) {
        _WriteLog(LL_FATAL, "[AisinoLogicIO] Logic header recv failed\r\n");
        return -2;
    }
    int wRepCode = GetDeviceRepCode(pInOutCmd);  //返回执行成功是否
    if (wRepCode != 0) {
        char describe[512] = {0};
        int nFormatErrorCode = AisinoVendorErrorCodeToDescription(wRepCode, describe);
        SetLastError(hUSB, nFormatErrorCode, "%d,%s", wRepCode, describe);
        return -3;
    }
    wRepCode = GetDeviceRepCode(pInOutCmd + 4);  //返回content长度,实际返回长度为len+1位包校验码
    if (!wRepCode)
        return 0;
    //返回的bufflen大于缓冲区,预留一定长度字节给我们自己返回额外参数
    uint8 abExtraMyBuf[12] = {0};
    //额外参数内存排列 00[1] signed myret flag[1] CmdE1AARet[10]
    int nMyExtraLen = sizeof(abExtraMyBuf);
    if (wRepCode + nMyExtraLen > nInCmdBufLen) {
        _WriteLog(LL_FATAL, "[AisinoLogicIO] Out buf's size not enough, need:%d\r\n",
                  wRepCode + nMyExtraLen);
        return -4;
    }
    //拷贝CmdE1AARet用于后期备用
    memcpy(abExtraMyBuf + 2, pInOutCmd, 10);  //设备返回校验位1字节，自己空1字节;
    if (pInOutCmd[3] != 0xe3)                 //==\xe3 终止读取，否则继续
        *(sint8 *)(abExtraMyBuf + 1) = 1;     //继续读取
                                              //--recv data
    nRetChild = mass_storage_io(hUSB, g_CmdE1AB, sizeof g_CmdE1AB, nInCmdBufLen, MSC_DIR_D2H, 0, 0,
                                pInOutCmd, nInCmdBufLen);
    if (nRetChild < 0 || nRetChild != wRepCode + 1) {
        _WriteLog(LL_FATAL,
                  "[AisinoLogicIO] Output buff's size not match header's num,nRetChild = "
                  "%d,wRepCode = %d\r\n",
                  nRetChild, wRepCode);
        return -5;
    }
    //保存的自定义数据贴在数据后面备用
    uint8 *pExtraMyRetBuf = pInOutCmd + nRetChild;
    memcpy(pExtraMyRetBuf, abExtraMyBuf, nMyExtraLen);
    //只返回实际长度，自定义长度不包含在返回长度中
    return nRetChild;
}

int AisinoCryptIO(HUSB hUSB, bool bBigCryptIO, uint8 *TransferData, int nTransferLen,
                  uint8 *pRecvDataBuff, int nRecvDataBuffLen)
{
    int nRetChild = 0;
    uint8 cb[64], abTransferData[512], abRep[512];
    memset(cb, 0, sizeof(cb));
    cb[0] = 0xf1;
    // TransferData 一般比较小,最大的长度应该是签名模板
    //先用固定大小数据,后期如果有大缓冲再改为堆分配
    memcpy(abTransferData, TransferData, nTransferLen);
    if (!bBigCryptIO) {
        cb[1] = 0xf1;
        //只加密前8字节
        AisinoDesEcbEncrypt(abTransferData);
    }
    cb[6] = 0x2;  // writeflag
    memcpy(cb + 7, &nTransferLen, 2);
    cb[9] = 0xff;
    nRetChild = mass_storage_io(hUSB, cb, 16, 0, MSC_DIR_H2D, (uint8 *)abTransferData, nTransferLen,
                                abRep, sizeof(abRep));
    if (nRetChild < 0)
        return -1;
    memset(pRecvDataBuff, 0, nRecvDataBuffLen);
    cb[6] = 0x1;  // readflag
    memcpy(cb + 7, &nRecvDataBuffLen, 2);
    nRetChild =
        mass_storage_io(hUSB, cb, 16, 260, MSC_DIR_D2H, NULL, 0, pRecvDataBuff, nRecvDataBuffLen);
    if (nRetChild < 0)
        return -2;
    return nRetChild;
}