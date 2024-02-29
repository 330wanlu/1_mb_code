/*****************************************************************************
File name:   console.h
Description: 终端模式命令解析
Author:      Zako
Version:     1.0
Date:        2022.12
History:
20221201     初始化格式文档
****************************************************************************/
#include "console.h"

#ifdef _CONSOLE
#if defined(PROJECT_TAX)
#include "../tax/aisino/aisino_call.h"
#include "../tax/cntax/cntax_call.h"
#include "../tax/etax/etax_call.h"
#include "../tax/nisec/nisec_call.h"
#elif defined(PROJECT_BANK)
#include "../bank/common/bank_function.h"
#else
#endif

int ConsoleClose(int nDeviceType, HUSB hUsb)
{
#ifdef PROJECT_TAX
    if (DEVICE_AISINO == nDeviceType)
        AisinoClose(hUsb);
#endif
    return RET_SUCCESS;
}

//参数hDev 内存分配置0；参数hUsb 内存分配置0；参数bAuthDev 是否认证设备；
//参数nBusID nDevID，指定具体usbid后打开指定设备，否则自动挑选设备打开
//
//返回参数: <0错误，==0未知设备, >0 对应宏定义设备类型
//返回成功效果:hDev hUsb被分别初始化，hUsb被绑定在hDev上
int DetectDeviceInit(HDEV hDev, HUSB hUSB, uint8 bAuthDev, int nBusID, int nDevID)
{
    if (!hDev || !hUSB)
        return -1;
    int nUsbType = usb_device_open(hUSB, nBusID, nDevID);
    if (nUsbType <= 0) {
        _WriteLog(LL_FATAL, "Usb device open failed");
        return ERR_DEVICE_OPEN;
    }
#if defined(PROJECT_TAX)
    _WriteLog(LL_INFO, "Open [Tax] usb device, ignore others");
    if (DEVICE_AISINO == nUsbType) {  // aisino init
        _WriteLog(LL_INFO, "Aisino device found");
        //这里其实不用关闭的，但是为了保证每次都是新的状态，还是关闭再重新打开比较好
        // AisinoClose(hUSB);
        //设备不从USB口拔出，都不用重新TaxcardOpen，如果不需要可以再第一次加载税盘的时候执行此过程
        if (AisinoOpen(hUSB) < 0) {
            _WriteLog(LL_WARN, "Init device to open failed");
            return ERR_DEVICE_OPEN;
        }
    } else if (DEVICE_NISEC == nUsbType) {  // nisec init
        _WriteLog(LL_INFO, "Nisec device found");
    } else if (DEVICE_CNTAX == nUsbType) {
        _WriteLog(LL_INFO, "Cntax device found");
    } else
        return ERR_DEVICE_UNSUPPORT;
    if (LoadTaxDevice(nUsbType, hDev, hUSB, bAuthDev) < 0)
        return ERR_DEVICE_LOAD_DEVICE;
#elif defined(PROJECT_BANK)
    _WriteLog(LL_INFO, "Open [Bank] usb device, ignore others");
    if (DEVICE_ICBC == nUsbType) {
        _WriteLog(LL_INFO, "Icbc device found");
    } else
        return ERR_DEVICE_UNSUPPORT;
    if (LoadBankDevice(nUsbType, hDev, hUSB, bAuthDev) < 0)
        return ERR_DEVICE_LOAD_DEVICE;
#else
#endif
    return nUsbType;
}

//打印该机柜设备上所有设备类型、设备号、税号、公司名称，用于找盘
int EnumAllUSBDevice_CallBack(int nDeviceType, HUSB hUSB)
{
    bool bRet = false;
    libusb_device *device = libusb_get_device(hUSB->handle);
    int nBusID = libusb_get_bus_number(device);
    int nDevID = libusb_get_device_address(device);
    char szDeviceType[16] = "", szCommonTaxID[24] = "", szCompanyName[256] = "",
         szDeviceID[24] = "";
    switch (nDeviceType) {
#if defined(PROJECT_TAX)
        case DEVICE_AISINO: {
            strcpy(szDeviceType, "A");
            if (AisinoOpen(hUSB) < 0)
                break;
            if (AisinoReadTaxIDandCustomName(hUSB, szCommonTaxID, szCompanyName,
                                             sizeof(szCompanyName)) < 0)
                break;
            ReadTaxDeviceID(hUSB, szDeviceID);
            AisinoClose(hUSB);
            bRet = true;
            break;
        }
        case DEVICE_NISEC: {
            strcpy(szDeviceType, "N");
            HDEV hDev = MallocDevInfo();
            if (!hDev)
                break;
            hDev->hUSB = hUSB;
            if (NisecEntry(hUSB, NOP_OPEN_FULL) < 0)
                break;
            GetDiskEnvFromManageData(hDev);
            strcpy(szCommonTaxID, hDev->szCommonTaxID);
            strcpy(szCompanyName, hDev->szCompanyName);
            strcpy(szDeviceID, hDev->szDeviceID);
            FreeDevInfo(hDev);
            bRet = true;
            break;
        }
        case DEVICE_CNTAX: {
            strcpy(szDeviceType, "C");
            HDEV hDev = MallocDevInfo();
            if (!hDev)
                break;
            hDev->hUSB = hUSB;

            uint8 abDeviceOpenInfo[64] = {0};
            if (CntaxEntryRetInfo(hUSB, NOP_OPEN_FULL, abDeviceOpenInfo) < 0)
                break;
            if (CnTaxGetDeviceEnv(hDev, hUSB, abDeviceOpenInfo) < 0) {
                CntaxEntry(hUSB, NOP_CLOSE);
                break;
            }
            CntaxEntry(hUSB, NOP_CLOSE);
            strcpy(szCommonTaxID, hDev->szCommonTaxID);
            strcpy(szCompanyName, hDev->szCompanyName);
            strcpy(szDeviceID, hDev->szDeviceID);
            FreeDevInfo(hDev);
            bRet = true;
            break;
        }
#else
#endif
        default:
            _WriteLog(LL_WARN, "Unsupport device type");
            break;
    }
    if (!bRet)
        _WriteLog(LL_WARN, "Enum device failed, busid:03%d devid:03%d", nBusID, nDevID);
    else {
        _WriteLog(LL_INFO, "bus:%03d dev:%03d %s %s %s %s", nBusID, nDevID, szDeviceType,
                  szDeviceID, szCommonTaxID, szCompanyName);
    }
    return 0;
}

int FunctionCall(HDEV hDev, uint8 bFuncNum, char *szParament1, char *szParament2, char *szParament3)
{
#ifdef PROJECT_TAX
    HUSB hUsb = hDev->hUSB;
    char szOutBuf[1024] = "";
    if (bFuncNum > 99) {
        // common call
        switch (bFuncNum) {
            case 100: {
                //枚举所有设备
                usb_device_open_raw(hUsb, OPEN_DEV_ENUM, 0, 0, EnumAllUSBDevice_CallBack);
                break;
            }
            case 102: {
                //解密cntax软件SSL日志
                if (!strlen(szParament1)) {
                    _WriteLog(LL_FATAL, "Want path");
                    break;
                }
                int n1 = atoi(szParament2);
                CallNisecCntaxDecryptLog(n1, szParament1);
                break;
            }
            //> 200 依赖DetectDeviceInit
            case 200: {
                //存设备配置json
                int nFPLX = atoi(szParament1);
                _WriteLog(LL_INFO, "Test SaveNoDeviceJson, fplx:%d", nFPLX);
                SaveNoDeviceJson(hDev, nFPLX);
                break;
            }
            case 201: {
                //浏览器登陆税务局确认平台透传服务器
                my_etax_auth_server(hDev);
                break;
            }
            case 202: {
                //月底设备抄报清卡
                switch (hDev->bDeviceType) {
                    case DEVICE_AISINO:
                        my_aisino_read_taxInfo(hUsb);
                        my_aisino_report_clear(hDev, true);
                        my_aisino_report_clear(hDev, false);
                        my_aisino_read_taxInfo(hUsb);
                        break;
                    case DEVICE_NISEC:
                        my_nisec_read_taxinfo_alldisplay(hUsb);
                        my_nisec_report_clear(hDev, true);
                        my_nisec_report_clear(hDev, false);
                        my_nisec_read_taxinfo_alldisplay(hUsb);
                        break;
                    case DEVICE_CNTAX:
                        my_cntax_read_taxinfo_alldisplay(hUsb);
                        my_cntax_report_clear(hDev, true);
                        my_cntax_report_clear(hDev, false);
                        my_cntax_read_taxinfo_alldisplay(hUsb);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                _WriteLog(LL_FATAL, "Unknow common function number");
                break;
        }
        return 0;
    }
    if (DEVICE_AISINO == hDev->bDeviceType) {  //######aisino
        _WriteLog(LL_INFO, "Select aisino device to test");
        switch (bFuncNum) {
            case 0:
                _WriteLog(LL_INFO, "Test aisino my_read_taxID");
                my_aisino_read_taxID(hUsb);
                my_aisino_read_ID(hUsb);
                break;
            case 1:
                _WriteLog(LL_INFO, "Test aisino my_read_InvNumber");
                my_aisino_read_InvNumber(hUsb);
                break;
            case 2:
                _WriteLog(LL_INFO, "Test aisino my_read_taxInfo");
                my_aisino_read_taxInfo(hUsb);
                break;
            case 3:
                _WriteLog(LL_INFO, "Test aisino my_read_invoice");
                if (!strlen(szParament1)) {
                    _WriteLog(LL_FATAL, "my_read_invoice need parament1=yearmonth, example:201912");
                    break;
                }
                my_aisino_read_invoice(hDev, szParament1);
                break;
            case 4:
                _WriteLog(LL_INFO, "Test aisino my_waste_inv");
                if (!strlen(szParament1) || !strlen(szParament2)) {
                    _WriteLog(LL_FATAL, "my_waste_inv need parament1 parament2,p1=fpdm p2=fphm");
                    break;
                }
                my_aisino_waste_inv(hDev, szParament1, szParament2);
                break;
            case 5:
                _WriteLog(LL_INFO, "Test aisino my_readmy_make_invoice_InvNumber");
                my_aisino_make_invoice(hDev);
                break;
            case 6:
                _WriteLog(LL_INFO, "Test aisino my_upload_OfflineInv");
                my_aisino_upload_OfflineInv(hDev, szParament1, szParament2, atoi(szParament3));
                break;
            case 7: {
                if (!strlen(szParament1) || !strlen(szParament2)) {
                    _WriteLog(LL_FATAL,
                              "QueryInvInfo need parament1 parament2,p1=fpdm p2=fphm [p3=dzsyh]");
                    break;
                }
                _WriteLog(LL_INFO, "Test aisino QueryInvInfo");
                int nDZSYH = atoi(szParament3);
                HFPXX fpxx = MallocFpxxLinkDev(hDev);
                int nRet = QueryInvInfo(hDev, szParament1, szParament2, nDZSYH, fpxx);
                FreeFpxx(fpxx);
                if (nRet < 0)
                    _WriteLog(LL_INFO, "Test aisino QueryInvInfo failed");
                else
                    _WriteLog(LL_INFO, "Test aisino QueryInvInfo OK");
                break;
            }
            case 8: {
                if (NetInvoice_QueryReady(hDev, szParament1, szOutBuf) < 0) {
                    _WriteLog(LL_INFO, "Test NetInvoice_QueryReady failed");
                    break;
                }
                _WriteLog(LL_INFO, "Query OK\r\n%s", szOutBuf);
                break;
            }
            case 9: {  //-l4 -f 9 -a 2\|032001900204\|51496536\|5
                if (NetInvoice_DownloadUnlock(hDev, szParament1) < 0) {
                    _WriteLog(LL_INFO, "Test NetInvoice_DownloadUnlock failed");
                    break;
                }
                _WriteLog(LL_INFO, "Test NetInvoice_DownloadUnlock successful");
                break;
            }
            case 10: {  // temp use
                _WriteLog(LL_INFO, "Debug--Test aisino my_read_invoice");
                if (!strlen(szParament1)) {
                    _WriteLog(LL_FATAL, "my_read_invoice need parament1=yearmonth, example:201912");
                    break;
                }
                my_aisino_read_invoice(hDev, szParament1);
                break;
            }
            case 11: {
                _WriteLog(LL_INFO, "Debug--Test aisino my_aisino_report_clear");
                uint8 bOPType = atoi(szParament1);
                my_aisino_report_clear(hDev, bOPType);
                break;
            }
            case 12: {
                _WriteLog(LL_INFO, "Debug--Test aisino my_get_pubserviceurl");
                int nDZSYH = atoi(szParament3);
                my_aisino_get_pubserviceurl(hDev, szParament1, szParament2, nDZSYH);
                break;
            }
            case 13: {
                _WriteLog(LL_INFO, "Debug--Test redform upload/cancel");
                char szOutMsg[256] = "";
                if (!strcmp(szParament1, "0"))
                    my_aisino_upload_check_redinvform(hDev);
                else
                    RedinvTZDCancel(hDev, szParament2, szOutMsg, 256);
                break;
            }
            default:
                _WriteLog(LL_FATAL, "Unknow aisino function number");
                break;
        }
    } else if (DEVICE_NISEC == hDev->bDeviceType) {  //######nisec
        _WriteLog(LL_INFO, "Select nisec device to test");
        switch (bFuncNum) {
            case 0:
                _WriteLog(LL_INFO, "Test nisec my_read_nisec_id");
                my_nisec_read_id(hDev);
                break;
            case 1:
                _WriteLog(LL_INFO, "Test nisec my_read_nisec_taxinfo_alldisplay");
                my_nisec_read_taxinfo_alldisplay(hUsb);
                break;
            case 2:
                _WriteLog(LL_INFO, "Test nisec my_read_nisec_buyinvinfo_alldisplay");
                my_nisec_read_buyinvinfo_alldisplay(hDev);
                break;
            case 3:
                _WriteLog(LL_INFO, "Test nisec my_read_nisec_invoice");
                if (!strlen(szParament1)) {
                    _WriteLog(LL_FATAL, "my_read_invoice need parament1=queryrange, "
                                        "example:20191201-20191220");
                    break;
                }
                my_nisec_read_invoice(hDev, szParament1);
                break;
            case 4: {
                _WriteLog(LL_INFO, "Test nisec my_nisec_upload_invoice_condition");
                uint8 bInvType = atoi(szParament1);
                my_nisec_upload_invoice_condition(hDev, bInvType, szParament2, szParament3);
                break;
            }
            case 5: {
                _WriteLog(LL_INFO, "Test nisec my_waste_nisec_inv");
                uint8 bInvType = atoi(szParament1);
                NisecWasteInvoice(hDev, false, bInvType, szParament2, szParament3, "管理员", 1);
                break;
            }
            case 6: {
                _WriteLog(LL_INFO, "Test nisec my_make_nisec_invoice");
                my_nisec_make_invoice(hDev);
                break;
            }
            case 7: {
                _WriteLog(LL_INFO, "Test nisec NisecQueryInvInfo");
                HFPXX fpxx = MallocFpxxLinkDev(hDev);
                if (!fpxx)
                    break;
                uint8 bInvType = atoi(szParament1);
                NisecQueryInvInfo(hDev, bInvType, szParament2, szParament3, fpxx);
                FreeFpxx(fpxx);
                break;
            }
            default:
                _WriteLog(LL_FATAL, "Unknow nisec function number");
                break;
        }
    } else if (DEVICE_CNTAX == hDev->bDeviceType) {  //######cntax
        _WriteLog(LL_INFO, "Select cntax device to test");
        switch (bFuncNum) {
            case 0:
                _WriteLog(LL_INFO, "Test cntax my_read_cntax_id");
                my_cntax_read_id(hDev);
                break;
            case 1:
                _WriteLog(LL_INFO, "Test cntax my_read_cntax_taxinfo_alldisplay");
                my_cntax_read_taxinfo_alldisplay(hUsb);
                break;
            case 2:
                _WriteLog(LL_INFO, "Test cntax my_read_cntax_buyinvinfo_alldisplay");
                my_cntax_read_buyinvinfo_alldisplay(hDev);
                break;
            case 3:
                _WriteLog(LL_INFO, "Test cntax my_read_cntax_invoice");
                if (!strlen(szParament1)) {
                    _WriteLog(LL_FATAL, "my_read_invoice need parament1=queryrange, "
                                        "example:20191201-20191220");
                    break;
                }
                my_cntax_read_invoice(hDev, szParament1);
                break;
            case 4: {
                _WriteLog(LL_INFO, "Test CntaxCheckToReSign,example:fplx fpdm fphm");
                uint8 bInvType = atoi(szParament1);
                CntaxCheckToReSign(hDev, bInvType, szParament2, szParament3);
                break;
            }
            case 5: {
                _WriteLog(LL_INFO, "Test Getoffline invoice upload");
                my_cntax_read_upload_invoice(hDev);
                break;
            }
            case 6: {
                _WriteLog(LL_INFO, "Test report invoice");
                my_cntax_report_clear(hDev, true);
                break;
            }
            case 7: {
                _WriteLog(LL_INFO, "Test clearcard invoice");
                my_cntax_report_clear(hDev, false);
                break;
            }
            default:
                _WriteLog(LL_FATAL, "Unknow cntax function number");
                break;
        }
    } else
        _WriteLog(LL_FATAL, "Unknow device type");
#endif
    return 0;
}

void Usage()
{
    // example:
    // ureader -l4 -f3 -i 1:35 -a 20191201-20191213
    printf("Auxiliary for usb disk read.\r\nThe program is for testing only, and you are "
           "responsible for any damage of devices or legal issues caused by using this "
           "program.\r\n\r\n");
    printf("-h [print help]\r\n");
    printf("-d [debug one function]\r\n");
    printf("-f funcNumber[0-100-200]\r\n");
    printf("-i busid:devid(1:45)|mountpath 1-x.x.x.x(1.2.2.1)\r\n");
    printf("-l loglevel[0-4]\r\n");
    printf("-a param1\r\n");
    printf("-b param2\r\n");
    printf("-c param3\r\n");
    printf("\r\n");
    printf("example:\r\n");
    printf("ureader -l3 -f3 -i 1:35 -a 20191201-20191213\r\n");
    printf("ureader -l1 -f3 -i 1.2.2.1 -a 20191201-20191213\r\n");
    printf("ureader -l2 -f100\r\n");
    printf("ureader -l4 -d\r\n");
}

int GetBusDevId(int *nBusID, int *nDevID)
{
    bool ret = false;
    do {
        char szStr[4096] = "";  // ReadBinFile 读取大小4k
        char szMountPath[256] = "";
        strcpy(szStr, optarg);
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
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/busnum", optarg);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFile(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            _WriteLog(LL_FATAL, "Read mount path failed :%s", szMountPath);
            break;
        }
        *nBusID = atoi(szStr);  //末尾有个\n不影响
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/devnum", optarg);
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
    return ret;
}


int ParamentInit(int argc, const char *argv[], uint8 *bDebugMode, uint8 *bFuncNum, int *nBusID,
                 int *nDevID, char *szParament1, char *szParament2, char *szParament3)
{
    int ch;
    g_bLogLevel = 4;  //默认是4
    char *szDate = __DATE__;
    char szMoon[16] = "";
    int nYear = 0, nDay = 0;
    sscanf(szDate, "%s %d %d", szMoon, &nDay, &nYear);
    _WriteLog(LL_INFO, "Build version:[%d %s %d]", nYear, szMoon, nDay);

    while ((ch = getopt(argc, (char *const *)argv, "hdf:i:l:a:b:c:")) != -1) {
        switch (ch) {
            case 'h':
                Usage();
                exit(0);
                break;
            case 'd':
                *bDebugMode = 1;
                break;
            case 'f':
                *bFuncNum = atoi(optarg);
                break;
            case 'i': {
                if (!GetBusDevId(nBusID, nDevID))
                    return -1;
                break;
            }
            case 'l': {
                int nLoglevel = atoi(optarg);
                if (nLoglevel < 0 || nLoglevel > 4) {
                    printf("Loglevel error!\r\n");
                    return -1;
                }
                g_bLogLevel = nLoglevel;
                break;
            }
            case 'a':
                strcpy(szParament1, optarg);
                break;
            case 'b':
                strcpy(szParament2, optarg);
                break;
            case 'c':
                strcpy(szParament3, optarg);
                break;
            default:
                printf("other option:%c\n", ch);
                exit(0);
                break;
        }
    }
    return RET_SUCCESS;
}
#endif  //_CONSOLE