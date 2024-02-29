/*****************************************************************************
File name:   nisec_usbio.c
Description: 用于百旺盘的设备USB协议操作
Author:      Zako
Version:     1.0
Date:        2019.09
History:
20190902     最初代码实现
20220630     格式化协议错误
*****************************************************************************/
#include "nisec_usbio.h"

int NisecVendorErrorCodeToDescription(int wRepCode, char *describe)
{
    int nFormatErrorCode = ERR_NISEC_COMMON;
    switch (wRepCode) {
        case 0x4:
            nFormatErrorCode = ERR_NISEC_DEVICE;
            strcpy(describe, "硬件或设备错误");
            break;
        case 0x5:
        case 0x092400:
            nFormatErrorCode = ERR_NISEC_PARAMENTER;
            strcpy(describe, "参数或命令错误");
            break;
        case 0x092403:
            nFormatErrorCode = ERR_NISEC_INCORRECT_INPUT_DATE;
            strcpy(describe, "输入日期不合法");
            break;
        case 0x09240d:
            // 百旺盘20221128实际测试得到结论(底层驱动版本<1106xxxxxx，TC_GetAuthKeyVersion返回0，且尚未TC_UpdateAuthKey，开票会报此错误)
            nFormatErrorCode = ERR_DEVICE_NEEDUPDATE_CONFIG;
            strcpy(describe, "需要更新安全认证密钥");
            break;
        case 0x092000:
            nFormatErrorCode = ERR_NISEC_ILLEGAL_COMMAND;
            strcpy(describe, "非法命令");
            break;
        case 0x092100:
            nFormatErrorCode = ERR_NISEC_OVERFLOW;
            strcpy(describe, "数据读写溢出");
            break;
        case 0x092700:
            nFormatErrorCode = ERR_NISEC_WRITE_PROTECT;
            strcpy(describe, "设备写保护");
            break;
        case 0x09d110:
            nFormatErrorCode = ERR_NISEC_UNSUPPORT_INVOICETYPE;
            strcpy(describe, "发票类型不支持");
            break;
        case 0x09d122:
            nFormatErrorCode = ERR_NISEC_DEADLINE_DATE;
            strcpy(describe, "超过离线开票上传限定截止日期");
            break;
        case 0x09d121:
            nFormatErrorCode = ERR_NISEC_DEADLINE_PRICE;
            strcpy(describe, "超过离线开票限定金额");
            break;
        case 0x09d120:
            nFormatErrorCode = ERR_NISEC_DEADLINE_COUNT;
            strcpy(describe, "超过离线开票限定张数");
            break;
        case 0x09d11f:
            nFormatErrorCode = ERR_NISEC_DEADLINE_TIME;
            strcpy(describe, "超过离线开票限定时长");
            break;
        case 0x09c000:
            nFormatErrorCode = ERR_NISEC_DEVICE_NOTOPEN;
            strcpy(describe, "设备未开启");
            break;
        case 0x0963c7:
        case 0x0963c6:
        case 0x0963c5:
        case 0x0963c4:
        case 0x0963c3:
        case 0x0963c2:
        case 0x0963c1:
        case 0x0963c0:
            //最后一个字节低8位是剩余尝试次数，需要传出去 0x0963cX
            //'ECode:0x'为关键字,获取密码剩余次数时会作出判断,不能随意修改
            nFormatErrorCode = ERR_NISEC_CERTPASSWORD_INCORRECT;
            sprintf(describe, "证书口令错误，ECode:0x%x", wRepCode);
            break;
        case 0x09d106:
        case 0x09d200:  //发票恢复测试后，得到该错误值未无更多数据
            nFormatErrorCode = ERR_NISEC_NO_DATA;
            strcpy(describe, "没有找到指定的数据");
            break;
        case 0x096a80:
            nFormatErrorCode = ERR_NISEC_REPORT;
            strcpy(describe, "税控盘读时钟汇总数据失败：时钟芯片操作错误");
            break;
        case 0x09d109:
            nFormatErrorCode = ERR_NISEC_DEADLINE;
            strcpy(describe, "已到开票截止日期，禁止开票");
            break;
        case 0x09d11c:
            nFormatErrorCode = ERR_NISEC_CANNOT_WASTE;
            strcpy(describe, "此发票已报税不能作废");
            break;
        case 0x09d108:
            nFormatErrorCode = ERR_NISEC_INCORRECT_FPDMHM;
            strcpy(describe, "开票信息中的发票号码、代码不合法");
            break;
        case 0x09e105:
            nFormatErrorCode = ERR_NISEC_CLOCK_DAMAGE;
            strcpy(describe, "时钟错误");
            break;
        default:  //有个密码错误的动态错误号码在其中,'ECode:0x'为关键字，不能随意修改
            sprintf(describe, "设备底层错误，ECode:0x%x", wRepCode);
            break;
    }
    //  _WriteLog(LL_WARN, "Nisec/Cntax LogicIO error, code:0x%x desc:%s", wRepCode, describe);
    return nFormatErrorCode;
}

int GetNisecDeviceLastErrorCode(HUSB hUSB, char *szOutErrorCode)
{
    int nRet = 0;
    char szErrorCode[512] = {0};
    char describe[512] = {0};
    int nErrorCode = 0;
    uint v2 = 0, dwDeviceLastErrorCode = 0;  // dword_3BAF30
    uint8 cb[256] = {0};
    memcpy(cb, "\x03\x00\x00\x00\x60", 5);
    if ((nRet = NisecLogicIORaw(hUSB, cb, 5, sizeof(cb), NULL, 0)) < 16)
        return -1;
    if (0xa != cb[7] && 0xc != cb[7])  // nisec after len == 10, cntax == 12
        return -2;
    //官方的错误代码格式化，不能动
    dwDeviceLastErrorCode = bswap_32(*(uint32 *)(cb + 10));
    if (dwDeviceLastErrorCode) {
        v2 = (unsigned int)dwDeviceLastErrorCode >> 16;
        if (BYTE1(v2))
            sprintf(szErrorCode, "%.8x", dwDeviceLastErrorCode);
        else
            sprintf(szErrorCode, "09%.4x", dwDeviceLastErrorCode % 0xFFFFu);
    } else
        sprintf(szErrorCode, "%.2x", dwDeviceLastErrorCode);
    sscanf(szErrorCode, "%x", &nErrorCode);
    int nFormatErrorCode = NisecVendorErrorCodeToDescription(nErrorCode, describe);
    SetLastError(hUSB, nFormatErrorCode, "%s,%s", szErrorCode, describe);
    return 0;
}

//原生发送，不改动USBCmd
int NisecLogicIORaw(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen,
                    uint8 *pTransferData, int nTransferDataLen)
{
    // pInOutCmd 0xfe0x6f 固定
    int nRet = 0;
    uint8 cmd[24] = {0};
    if (nInCmdLen > 16)
        return -1;
    memcpy(cmd, pInOutCmd, nInCmdLen);
    uint8 bTransferArrow = MSC_DIR_D2H;
    if (pTransferData)
        bTransferArrow = MSC_DIR_H2D;
    nRet = mass_storage_io(hUSB, cmd, nInCmdLen, nInCmdBufLen, bTransferArrow, pTransferData,
                           nTransferDataLen, (uint8 *)pInOutCmd, nInCmdBufLen);
    if (-100 == nRet)  // SCSCI 错误
        GetNisecDeviceLastErrorCode(hUSB, hUSB->szLastErrorDescription);
    return nRet;
}

// pInOutCmd缓冲区部分数据会被修改，但是不会越界修改
int NisecLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen)
{
    memset(pInOutCmd + nInCmdLen, 0, 16 - nInCmdLen);
    if (pTransferData)
        pInOutCmd[0xe] = BuildTransferVCode(pTransferData, nTransferDataLen);
    return NisecLogicIORaw(hUSB, pInOutCmd, 16, nInCmdBufLen, pTransferData, nTransferDataLen);
}


int CntaxLogicIO(HUSB hUSB, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData,
                 int nTransferDataLen)
{
    memset(pInOutCmd + nInCmdLen, 0, 16 - nInCmdLen);
    if (pTransferData)
        pInOutCmd[0xe] = BuildTransferVCode(pTransferData, nTransferDataLen);
    return NisecLogicIORaw(hUSB, pInOutCmd, 16, nInCmdBufLen, pTransferData, nTransferDataLen);
}