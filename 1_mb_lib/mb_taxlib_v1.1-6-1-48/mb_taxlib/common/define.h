/*****************************************************************************
File name:   define.h
Description: 用于存放全局定义和宏定义，任何一个参数都会对程序全局产生影响
Author:      Zako
Version:     1.0
Date:        2020.08
History:
20200813     初始化格式文档
****************************************************************************
源码几大重要注释方法说明
//++debug     调试临时代码，一般发布时需要注释
//++todo      短期急待解决的问题
//++version   已知版本差异问题，短期内难以改善，待问题暴露时整体修改
//++issue     潜在可能存在问题，未经测试改动容易造成未知问题，待问题暴露时修改
//++conflict  和蒙柏方分歧后，已确定标准
*****************************************************************************/
#ifndef COMMON_DEFINE_H
#define COMMON_DEFINE_H
#include "common.h"
#include "error.h"

//##全局控制宏
#define TAX_SPECIAL  //部分代码支持
#define PROTOCOL_GBK

// 税局最新规定商品条目最大2000行，2000行商品*sizeof(STSPXXv3)=700k左右，aisino最大发票长度64K/400K
#define DEF_MAX_FPBUF_LEN 0x100000  //十进制1024*1024=1048576=1MBytes
#define DEF_BMBBBH_DEFAULT "50.0"  //在线实时更新, 税收分类编码表版本号 39.0-20210825
#define DEF_CERT_PASSWORD_DEFAULT "12345678"    //所有设备证书口令，cntax设备口令
#define DEF_DEVICE_PASSWORD_DEFAULT "88888888"  // nisec设备口令 aisino默认设备密码23456789
#define DEF_SWZJ_INVOICE_GATEWAY "/acceptFramework/UniAcceptService"

enum enDeviceType {  //数值对蒙柏方重要，请勿重定义
    DEVICE_UNKNOW = 0,
    DEVICE_AISINO = 1,     //爱信诺金税盘
    DEVICE_NISEC = 2,      //百旺税控盘
    DEVICE_MENGBAI = 3,    //蒙柏模拟盘
    DEVICE_CNTAX = 101,    //税务ukey
    DEVICE_SKP_KEY = 201,  //百旺勾选key
    DEVICE_JSP_KEY = 202,  //航信勾选key
    DEVICE_MENGBAI2 = 301  //蒙柏服务商key
};

enum enFPLX {
    //发票类型最大数目
    FPLX_MAX_NUM = 8,
    //通用发票类型/百旺发票类型
    FPLX_COMMON_ZYFP = 4,   //专用发票
    FPLX_COMMON_JDCFP = 5,  //机动车发票
    FPLX_COMMON_ESC = 6,    //二手车发票
    FPLX_COMMON_PTFP = 7,   //普通发票
    FPLX_COMMON_HYFP = 9,   //货运发票
    FPLX_COMMON_JSFP = 25,  //卷式发票
    FPLX_COMMON_DZFP = 26,  //电子普票
    FPLX_COMMON_DZZP = 28,  //电子专票
    //爱信诺发票类型
    FPLX_AISINO_ZYFP = 0,    //专用发票
    FPLX_AISINO_PTFP = 2,    //普通发票
    FPLX_AISINO_HYFP = 11,   //货运发票
    FPLX_AISINO_JDCFP = 12,  //机动车发票
    FPLX_AISINO_JSFP = 41,   //卷式发票
    FPLX_AISINO_ESC = 42,    //二手车发票
    FPLX_AISINO_DZFP = 51,   //电子发票
    FPLX_AISINO_DZZP = 52,    //电子专票
    //电子税务局(全电发票)类型
    FPLX_ETAX_ZYFP = 1,  //增值税专用发票
    FPLX_ETAX_PTFP,      //普通发票
    FPLX_ETAX_JDC,       //机动车统一销售发票
    FPLX_ETAX_ESC,       //二手车统一销售发票
    FPLX_ETAX_TLDZKP,    //铁路电子客票
    FPLX_ETAX_HKDZKP,    //航空运输客票电子行程单
};

//专用发票类型(蒙柏已写入文档，顺序不可调整)
enum enZYFPLX {
    ZYFP_NULL,  //非专用发票=一般专普票
    //油类
    ZYFP_HYSY,      //海洋石油
    ZYFP_SNY,       //石脑油
    ZYFP_SNY_DDZG,  //石脑油_定点直供
    ZYFP_RLY,       //燃料油
    ZYFP_RLY_DDZG,  //燃料油_定点直供
    //稀土类
    ZYFP_XT_YCL,  //稀土_原材料
    ZYFP_XT_CCP,  //稀土_产成品
    //农产品类
    ZYFP_NCP_XS,  //农产品_销售
    ZYFP_NCP_SG,  //农产品_收购
    //征收减免
    ZYFP_JYZS,      //简易征税(5%减按1.5%)
    ZYFP_CEZS,      //差额征收
    ZYFP_TXF_KDK,   //通行费_可抵扣
    ZYFP_TXF_BKDK,  //通行费_不可抵扣
    ZYFP_CPY,       //成品油
    ZYFP_XT_JSJHJ,  //稀土_金属及合金
    ZYFP_XT_CPJGF,  //稀土_产品加工费
};

//发票特殊处理，目前预留32位，即32种状态；如果不够最多扩展至64位
enum enInvoiceSpecial {
    TSFP_NULL = 0,
    // 0:无;1: 年月日前发生纳税义务;2:前期已开具X%征收率发票,发生销售折让、中止或者退回等情形
    //需要开具红字发票,或者开票有误需要重新开具;3:因为实际经营业务需要,放弃享受减按X%征收率征收增值税政策;
    // x%税率;小规模x%税率原因,目前包含1%和3%税率
    TSFP_XGM_SPECIAL_TAX = 0b1,
    TSFP_RESERVE2 = 0b10,
    TSFP_RESERVE3 = 0b100,
    TSFP_RESERVE4 = 0b1000,
};

//发票行性质
enum enFPHXZ {
    //发票行性质aisino
    FPHXZ_AISINO_SPXX = 0,     //商品信息
    FPHXZ_AISINO_XJXHQD,       //详见销货清单
    FPHXZ_AISINO_SPXX_ZK = 3,  //商品信息折扣(被折扣)
    FPHXZ_AISINO_ZKXX,         //折扣信息(折扣)
    FPHXZ_AISINO_XHQDZK,       //销货清单折扣
    FPHXZ_AISINO_XJDYZSFPQD,   //详见对应正数发票清单
    //发票行性质nisec/common
    FPHXZ_COMMON_SPXX = 0,  //商品信息
    FPHXZ_COMMON_ZKXX,      //折扣信息(折扣)
    FPHXZ_COMMON_SPXX_ZK,   //商品信息折扣(被折扣)
    FPHXZ_COMMON_XJXHQD,    //详见销货清单
};

// Nisec/Common 发票状态
enum enCOMMON_FP_STATUS {
    FPSTATUS_BLUE = 0,     // 0：已开具的正数发票
    FPSTATUS_RED,          // 1：已开具的负数发票
    FPSTATUS_BLANK_WASTE,  // 2：未开具发票的作废发票=空白作废
    FPSTATUS_BLUE_WASTE,   // 3：已开正数票的作废发票
    FPSTATUS_RED_WASTE,    // 4：已开负数票的作废发票
};

//开卡发票类型
enum SELECT_INVOICE_OPTION {
    SELECT_OPTION_NULL = 0,        //什么都不获取
    SELECT_OPTION_ALL = -1,        //全部获取
    SELECT_OPTION_TEMPLATE = 0b1,  //发票模板信息
    SELECT_OPTION_MONITOR = 0b10,  //设备对应发票类型监控信息
    SELECT_OPTION_TIME = 0b100,    //获取设备最新标准时间
};

// clang-format off
//税局端API操作类型,税局端网络操作命令
enum TA_OP_TYPE {
    // aisino 固定操作代码
    TACMD_AISINO_CHECK_SERVERUP = 0,                // 0 判断服务器存活
    TACMD_AISINO_SYNC_COMPANYINFO,                  // 1 企业信息同步
    TACMD_AISINO_UNKNOW2,                           // 2 unknow
    TACMD_AISINO_UPFP_ZZS,                          // 3 专普票 上传
    TACMD_AISINO_QUERY_FPUP_ZZS,                    // 4 根据受理序列号查询发现处理状态
    TACMD_AISINO_UPFP_HYJDC,                        // 5 货运发票 机动车发票 上传 --(本代码未实现)
    TACMD_AISINO_UNKNOW6,                           // 6 unknow
    TACMD_AISINO_REDFORM_PTCHECK,                   // 7 PT红字发票在线校验
    TACMD_AISINO_REDFORM_UPLOAD_ZZS,                // 8 (专票)红字发票信息表上传(申请)
    TACMD_AISINO_UNKNOW9,                           // 9 unknow
    TACMD_AISINO_REDFORM_DOWNLOAD_ZZS,              // 10 (专票)红字发票信息表下载(查询)
    TACMD_AISINO_UNKNOW11,                          // 11 unknow
    TACMD_AISINO_UNKNOW12,                          // 12 unknow
    TACMD_AISINO_REPORT_ZZS,                        // 13 专普票 抄报上报汇总
    TACMD_AISINO_CLEAR_ZZS,                         // 14 专普票 清卡
    TACMD_AISINO_REPORT_HY,                         // 15 货运发票 抄报上报汇总  --(本代码未实现)
    TACMD_AISINO_REPORT_DZ,                         // 16 电子发票/机动车 抄报上报汇总
    TACMD_AISINO_CLEAR_HY,                          // 17 货运发票 清卡   --(本代码未实现)
    TACMD_AISINO_CLEAR_DZ,                          // 18 电子发票/机动车 清卡
    TACMD_AISINO_QUERY_NETINV_READY,                // 19 在线发票领取,获取已经确认后可领取发票卷列表
    TACMD_AISINO_DOWNLOAD_NETINV_READY,             // 20 在线发票领取,下载确认的发票卷
    TACMD_AISINO_UNKNOW21,                          // 21 unknow
    TACMD_AISINO_UNKNOW22,                          // 22 unknow
    TACMD_AISINO_QUERY_INVTYPELIST,                 // 23 获取可申领发票种类列表

    TACMD_AISINO_UPFP_JS = 29,                      // 29 卷式发票 上传  --(本代码未实现)
    TACMD_AISINO_UPFP_DZ = 31,                      // 31 电子发票 上传
    TACMD_AISINO_UNKNOW32,                          // 32 unknow
    TACMD_AISINO_REDFORM_DZCHECK,                   // 33 DZ红字发票在线校验

    TACMD_AISINO_QUERY_APPLICANT = 44,              // 44 获取登记的默认联系人证件信息
    TACMD_AISINO_UPFP_ESC = 47,                     // 47 二手车发票 上传  --(本代码未实现)
    TACMD_AISINO_PUBSERV_JOINCANCEL = 52,           // 52 公共服务平台申请/撤销
    TACMD_AISINO_TZDBH_CANCEL = 54,                 // 54 红字通知单撤销,笔误old:TACMD_AISINO_DZDBH_CANCEL

    //##nisec 固定操作代码
    TACMD_NISEC_UPFP_ZZS = 9020,                    // 9020 专普票 上传
    TACMD_NISEC_REPORT_ZZS = 9042,                  // 9042 专普票网上抄报
    TACMD_NISEC_CLEAR_ZZS = 9043,                   // 9043 专普票网上清卡
    TACMD_NISEC_REDFORM_UPLOAD_ZZS = 9031,          // 9031 (专票)红字发票信息表上传(申请)
    TACMD_NISEC_REDFORM_DOWNLOAD_ZZS = 9032,        // 9032 (专票)红字发票信息表下载

    //###自定义，非官方固定代码
    TACMD_AISINO_QUERY_FPUP_DZ = 50000,             // 查询电票上报状态
    TACMD_NISEC_QUERY_FPUP_ZZS,                     // 根据受理序列号查询发现处理状态
    TACMD_NISEC_UPFP_DZ,                            // DZFP 上传
    TACMD_NISEC_QUERY_FPUP_DZ,                      // DZ查询发现处理状态
    TACMD_NISEC_REPORT_DZ,                          // DZFP抄报
    TACMD_NISEC_CLEAR_DZ,                           // DZFP清卡
    TACMD_NISEC_TZDBH_CANCEL,                       // 红字通知单撤销
    TACMD_NISEC_PUBSERV_JOINCANCEL,                 // 公共服务平台申请/撤销
    TACMD_NISEC_NETINV_DO,                          // 在线发票领取,申请/撤销/确认
    TACMD_NISEC_SYNC_COMPANYINFO,                   // 同步企业信息

    // Cntax，基本都无固定操作代码，由XML内部字母来进行辨别，且所有的操作基本都不区分票种
    TACMD_CNTAX_UPFP,                               // 发票上传
    TACMD_CNTAX_QUERY_UPFP,                         // 发票上传结果查询
    TACMD_CNTAX_REPORT,                             // 抄报
    TACMD_CNTAX_CLEAR,                              // 清卡
    TACMD_CNTAX_NETINV_QUERY,                       // 购票查询
    TACMD_CNTAX_NETINV_WRITE,                       // 购票写盘
    TACMD_CNTAX_NETINV_CONFIRM,                     // 购票确认
    TACMD_CNTAX_REDFORM_UPLOAD,                     // 红字信息表上传
    TACMD_CNTAX_REDFORM_DOWNLOAD,                   // 红字信息表下载
    TACMD_CNTAX_REDFORM_CANCEL,                     // 红字信息表撤销
    TACMD_CNTAX_RAWXML,                             // 原始XML
    TACMD_CNTAX_SYNC_COMPANYINFO,                   // 同步企业信息
    //##尚未实现
};

//发票查询接口枚举类型
enum enFPCXType {
    FPCX_COMMON_MONTH = 0,    // NC 月度查询
    FPCX_COMMON_NUMBER,       // NC 指定发票代码号码 old:FPCX_COMMON_FPDMHM
    FPCX_COMMON_OFFLINE,      // NC 未上传发票 old:FPCX_COMMON_UPLOAD
    FPCX_COMMON_MODIFY,       // C 1)已上传发票验签失败、2)解析有误、3)开具后作废
    FPCX_COMMON_OFFLINE_NUM,  // C 未上传发票张数
    FPCX_COMMON_LASTINV,      // C 获取已开具的最后一张发票
};

uint8 g_bLogLevel;
enum enumLoglevel {
    LL_OFF,
    LL_FATAL,
    LL_INFO,  // default
    LL_WARN,
    LL_DEBUG,
};

struct USBSession {
    libusb_context *context;
    libusb_device_handle *handle;
    uint8 endpoint_d2h;
    uint8 endpoint_h2d;
    //错误信息
    int nLastErrorCode;
    char szLastErrorDescription[512];//上次错误编码，非线程安全，可能被冲掉; aisino、nisec、cntax、TA税局网络IO都支持,只有返回值错误时读取的错误描述才可能正确，否则有可能是上一次未清理错误描述,6字节错误标志+错误描述，错误标志[NET],[DEV],[USB]
    //蒙柏自用,自维护
	int errcode;
	char errinfo[2048];
    //调试用指针
    void *pTestData;                //当 _NODEVICE_TEST 宏启用时被赋值
};
typedef struct USBSession *HUSB;

struct StMonitorInfo {
    char kpjzsj[32];                // NC 开票截止时间
    char bsqsrq[32];                // NC 数据报送起始日期
    char bszzrq[32];                // NC 数据报送终止日期
    char dzkpxe[32];                // NC 单张发票开票金额限额
    char zsljxe[32];                // NC 正数累计限额
    char fsljxe[32];                // NC 负数累计限额
    char zxbsrq[32];                // NC 最新报税日期
    char syrl[32];                  // N 剩余容量
    char scjzrq[32];                // NC 上传截止日期
    char xdgnbs[32];                // N 限定功能标识
    char lxkpsc[32];                // NC 离线开票时长
    char lxkpzs[32];                // NC 离线开票张数
    char lxzsljje[32];              // NC 离线正数累计金额
    char lxfsljje[32];              // NC 离线负数累计金额
    char lxkzxx[162];               // N 离线扩展信息
    char ggfwpt[32];                // N 公共服务平台 001100000000000 or “”
};

struct StStockInfoGroup {
    char fpdm[32];                  // N
    char qshm[32];                  // N
    char zzhm[32];                  // N
    char dqhm[32];                  // N
    int fpfs;                       // N
    int syfs;                       // N
    char lgrq[32];                  // N
    char lgry[64];                  // N
};

struct StStockInfoHead {
    char dqfpdm[16];                // NC 当前代码
    char dqfphm[16];                // NC 当前号码
    char dqfpzzhm[16];              // N 当前终止号码
    int zsyfs;                      // NC
    int nGroupCount;                // NC
    void *pArray_StStockInfoGroup;  //此处存放上述结构体的结构体数组指针
};

enum enumTaAddrType {
    TAADDR_NOT_SET,
    TAADDR_UPLOAD_RSA,              // RSA上报\抄报服务器 nisec/aisino
    TAADDR_PLATFORM,                // 公共平台服务器 nisec/aisino
    TAADDR_UPLOAD_GM,               // 国密GM上报\抄报服务器 cntax
    TAADDR_CONFIRM,                 // 增值税发票选择确认平台(增值税发票综合服务平台)
    TAADDR_ETAX,                    // 电子税务局服务器
    TAADDR_ETAX_TPASS,              // [动态获取]电子税务局服务器-令牌服务器
    TAADDR_ETAX_DPPT,               // [动态获取]电子税务局服务器-数字账户服务器
};
struct UploadAddrModel {
    enum enumTaAddrType bServType;
    char szRegName[16];
    char szRegCode[16];
    char szTaxAuthorityURL[128];
};

struct EtaxUploadAddrModel {
    enum enumTaAddrType bServType;
    char szRegName[16];
    char szRegCode[16];
    char szTaxAuthorityURL[128];
    char szInitRequestPage[64];
};

struct ContextConfirmRequest {
    bool bLoginOK;
    char szVersion[12];
    char szToken[128];
};

//注意：此处各种设备的使用方式略有不同
//aisino 指定周期内统计，多发票类型，有链表，每个节点代表一个发票类型
//nisec  指定周期内统计，单发票类型，无链表，只有一个节点一个票种
//ukey   单月或者前3个月，单发票类型，链表可有可无，根据next链表指针判断
struct StMonthStatistics {
    uint8 fplx;         //A    发票类型 Aisino包含多个票种，其他则为一个票种
    char qssj[24];      //ANC  起始时间 yyyymmdd, Aisino yymm00
    char jzsj[24];      //ANC  截止时间 yyyymmdd, Aisino yymm00
    char qckcfs[24];    //ANC  期初库存
    char lgfpfs[24];    //ANC  领购发票份数
    char thfpfs[24];    //ANC  退回发票份数
    char zsfpfs[24];    //ANC  正数发票份数
    char zffpfs[24];    //ANC  正废发票份数
    char fsfpfs[24];    //ANC  负数发票份数
    char fffpfs[24];    //ANC  负废发票份数
    char kffpfs[24];    //NC   空废发票份数
    char qmkcfs[24];    //ANC  期末库存份数
    char zsfpljje[64];  //ANC  正数发票累计金额
    char zsfpljse[64];  //ANC  正数发票累计税额
    char zffpljje[64];  //NC   正废发票累计金额
    char zffpljse[64];  //NC   正废发票累计税额
    char fsfpljje[64];  //ANC  负数发票累计金额
    char fsfpljse[64];  //ANC  负数发票累计税额
    char fffpljje[64];  //NC   负废发票累计金额
    char fffpljse[64];  //NC   负废发票累计税额
    struct StMonthStatistics* next; //AC
};

//每个设备中包含的各个票种对应的信息，包括监控信息等
struct DeviceInvoiceTypeInfo{
    uint8 bInvType;                    // 发票类型(通用类型,不可为0)
    char szTaxrate[192];              //  当前发票类型下，从设备中动态获取的税率信息，包含含税不含税两部分，使用|分割，128+64=192
    struct StMonitorInfo monitor;
};
typedef struct DeviceInvoiceTypeInfo *HDITI;

struct DeviceExtendInfo {
    bool bmtsqybs;                    //  N  unknow
    bool tdqy;                        //  ANC 特定企业    
    bool dxqy;                        //  AC 电信企业
    bool xgmkjzpbs;                   // ANC 小规模开具专票标识
    bool cpybmdbs;                    // ANC 成品油白名单标识
    uint8 ncpqy;                      //  AC 农产品企业 1收购 2销售 3既收购又销售
    uint8 xtqy;                       // AC 00非稀土企业 01稀土企业-矿产品 02稀土企业-冶炼分离 03稀土企业-其它
    uint8 esjdcbs;                    // ANC 00非二手机动车纳税人 01经营单位 02拍卖单位 03二手车市场
    uint8 cpybs;                      // ANC 00非成品油企业 01生产企业 02经销企业
    uint8 jyqy;                       // ANC  卷烟企业
    uint8 jdcqy;                      // ANC  机动车企业
    uint8 jdcbmdbs;                   // NC  机动车白名单标识
    uint8 wtdkqy;                     // NC  委托代开企业
    uint8 xgmjdczyms;                 // C  小规模机动车专用ms
    uint8 tlqy;                       // A  铁路运输企业标识
    uint8 kpbz;                       // C  开票标志,01禁止开票
    char cpybsyxq[18];                // ANC 成品油有效期
    char cpybmdbsyxq[18];             // ANC 成品油白名单标识有效期
    char jyqyyxq[18];                 // NC  卷烟企业有效期
    char jdcqyyxq[18];                // NC  机动车企业有效期
    char jdcbmdyxq[18];               // NC  机动车白名单有效期
    char wtdkqyyxq[18];               // NC  委托代开企业有效期
    char djzclx[4];                   // NC  登记注册类型3字节
    char xgmjdczymsyxqzz[8];          // C   小规模机动车专用ms有效期zz
};
struct DevHttpConnectInfo{
	long long start_time;
	long long end_time;
	char start_time_s[30];
	char end_time_s[30];
	int	time_consuming;
	//char connect_errinfo[1024];
};

// A为aisino,N为nisec,C为chinatax-ueky;汉字在GBK编码中占2个字节，在UTF8编码中占3个字节
struct StaticDeviceInfo {
    //====动态数据,尽量在使用时重新调API更新数据
    char szFsyz[64];
    char szDeviceTime[24];            // ANC 设备时间 yyyymmddhhmmss 20191019084029 MB模拟盘yyyy-mm-dd hh：mm:ss
    uint8 abInvTemplate[128];          // NC  当前发票类型下模板,根据发票种类结果不同,第4字节为发票类型,7/15位普票强制12
    char szClientWanIP[24];           // ANC 特殊字段，税局返回给客户端的WanIP，与税局通信握手时才能获取到
    char szClientWanPort[8];          // ANC 特殊字段，税局返回给客户端的WanPort，与税局通信握手时才能获取到
    //====静态数据
    char szDeviceID[24];              // ANC 设备编号SN
    char szDriverVersion[32];         // ANC 底层硬件驱动版本号
    char szJMBBH[4];                  // A   加密版本号  
    char szCommonTaxID[24];           // ANC 通用显示税号,税盘所属公司税号=销方税号 ex:92320105MA1WMUAP8T
    char szCompressTaxID[24];         // A   压缩税号/发行税号? ex:320100344089314
    char sz9ByteHashTaxID[24];        // A   9位hash税号/8byte+1 ex:32010034408931401
    char szRegCode[8];                // AN  区域代码6字节,aisino开票使用，发票上传和公共服务平台时根据该值选择对应服务器
    char szCompanyName[256];          // ANC 税盘所属公司名称,最大50个汉字==销方名称    
    char szTaxAuthorityCode[16];      // NC  税务机关代码(官方公布代码)规范格式是11位，部分税局是9位,该值仅用于用户可视 www.chinatax.gov.cn/n810341/n810755/c3665232/content.html    
    char szTaxAuthorityCodeEx[16];    // ANC 税务机关代码扩展(内部使用)，标准12位(Nisec:11位时左补0，9位时左补200,Cntax:不足12位时右测补空格,aisino:不足12位时右测补空格)
    char szTaxAuthorityCodeBelong[64];//E   (暂未使用)税务机关代码4个层级，'|'分割, "Ssjswjgdm": "13100000000"|"Dsjswjgdm": "13102000000"|"Qxjswjgdm": "13102300000"|"Swjgdm": "13102300000",
    char szTaxAuthorityName[128];     // ANC 税务机关名称 swjgmc
    char szDeviceEffectDate[16];      // ANC 设备启用日期(登记时间) 14位yyyymmddhhmmss，aisino最后6位为0,nisec/cntax有的取出来后6位也是0
    char szPubServer[24];             // ANC 公共服务平台配置, "" or 平台类型(2)+平台号码...
    char szSignParameter[18];         // N   签名参数,16字节固定长度,签名参数配置应该是：01a8表示是测试环境，01b4表示是正式环境(个别测试盘也是01b4)。20210909测试为小写,01b4000000000000
    uint8 abSupportInvtype[16];       // ANC 支持的发票类型,第一个字节为个数，后面逐字节排列支持类型
    uint16 uICCardNo;                 // ANC 税盘分机号
    uint8 bTrainFPDiskType;           // ANC 0正式盘，1=A江苏测试盘/N重庆测试盘，2=山西测试盘
    uint8 bDeviceType;                // ANC aisino/nisec/cntax
    uint8 bOperateInvType;            // NC  当前IO生命周期，打开进行操作的发票通用类型
    uint8 bCryptBigIO;                // A   加密区域读取采用2k大小,大容量读取(主要是证书操作),小IO会加密
    uint8 bNatureOfTaxpayer;          // ANC 纳税人性质 0=代开税务机关 1=小规模纳税人 2=一般纳税人 3=转登记纳税人 9=辅导期一般纳税人
    uint32 nDriverVersionNum;         // A   底层硬件驱动数字版本，用于直接比对
    uint32 nMaxOfflineHour;           // A   设备最大离线小时数
    //====扩展信息，附加的额外设备信息
    struct DeviceExtendInfo stDevExtend;//NC 扩展信息，主要包括成品油、二手车之类的
    HDITI lstHeadDiti;                // NC  存放设备各发票类型信息链表
    //====内部指针及变量
    void *pX509Cert;                  // ANC 用于开票签名的证书指针
    HUSB hUSB;                        // ANC 设备的libusb句柄
    char szProxyStr[48];              // ANC 设备网络连接代理字符串，放在这里不用每次起连接再设置一遍代理
    uint8 bBreakAllIO;                 // ANC 默认为0，中断设备所有IO、循环操作，尽快恢复设备等待状态，置1后需尽快恢复初始值0

	int postMessage_flag;       //电子税务局标志
	int connect_success_count;
	int connect_failed_count;
	int connect_success_time;
	int connect_failed_time;
	struct DevHttpConnectInfo DevHttp;
};
typedef struct StaticDeviceInfo *HDEV;

// aisino/nisec叫法不一致
struct Spxx {
    //====程序内部指针
    struct Spxx *stp_next;
    //====标准字段
    char xh[32];                        //序号
    char spsmmc[128];                   //商品税目名称
    char spmc[128];                     //商品名称
    char yhsm[128];                     //优惠说明=增值税特殊管理ZZSTSGL,减按计增时使用
    char spbh[48];                      //商品编号(spbh)=分类编码(flbm)=商品编码(spbm)
    char zxbh[48];     					//自行编码,目前我方尚未使用
    char spsm[48];                      //商品税目编码,目前我方尚未使用
    char ggxh[128];                     //规格型号,打印36个字符（18个汉字）,从22（含22）个字符开始换行
    char jldw[128];                     //计量单位
    char dj[64];                        //单价,nisec存在小数点后10位转换
    char sl[64];                        //数量
    char je[64];                        //金额
    char se[64];                        //税额
    char slv[16];                       //税率
    char fphxz[16];                     //发票行性质,0 正常行;1 折扣行;2 被折扣行
    char hsjbz[16];                     //含税价标志
    char xsyh[16];                      //销售优惠=优惠政策标识YHZCBS 0未使用，1使用
    char lslvbs[16];                    //零税率标示=零税率标识LSLBS 空代表正常税率;1 出口免税和其他免税优惠政策（免税）;2 不征增值税（不征税）;3 普通零税率（0%）
    //====我方扩展字段
    char kce[48];                       //差额征税扣除额
    char hsje[64];                      //含税金额，用于校验
    //====新增字段
};

//这里仅仅是单个信息，如果导入时有多个发票，还需要做出链表形式
struct Fpxx {   
    //==设备相关，由hDev刷新，发票恢复时需要遵循该发票开票时的信息，不要使用hDev刷新
    char jqbh[32];                      // AN 机器编号近似等于设备编号，aisino部分发票类型该值需要为空，nisec全填充
    int kpjh;                           // ANC 开票机号
    char kprjbbh[32];                   // A 开票软件版本号,nisec亦有尚未使用到
    char driverVersion[32];             // A 设备底层版本号,nisec亦有尚未使用到
    char jmbbh[8];                      // A 加密版本号,nisec亦有尚未使用到 aisino_jmbbh
    char taVersion[32];                 // A 总局版本号,nisec亦有尚未使用到 aisino_xfver
    int nVerInvoice;                    // NC 发票版式版本
    //==发票相关
    char blueFpdm[16];                  //蓝票(原)发票代码-红票专用
    char blueFphm[16];                  //蓝票(原)发票号码-红票专用
    char blueKpsj[20];                  //蓝票(原)开票时间-红票专用 yyyymmddhhmmss
    char fpdm[16];                      //发票代码
    char fphm[16];                      //发票号码
    char fpendhm[16];                   //发票卷结束号码
    char pubcode[64];                   //公共服务平台配置 类型;代码;提取码随机数；01公共服务平台02企业自建平台03第三方平台
    uint32 specialFlag;                 //特殊发票处理标志（非税局，我方自定义）
    uint8 fpzt;                         // ANC 发票状态,参考enCOMMON_FP_STATUS,NC支持全状态标记。注意！Aisino目前读取全部支持，开具仅支持FPSTATUS_BLANK_WASTE空白作废标记
    uint8 fplx;                         //税局、百旺发票类型,与fplx_aisino相关联,两个发票类型同时存在消亡
    uint8 fplx_aisino;                  //航信发票类型,在航信中使用，否则使用通用发票类型
    uint8 zyfpLx;                       //专用发票类型
    uint8 sslkjly;                      // A SSLKJLY ?税率开具理由
    uint8 hcyy;                         // AN 红冲原因
    uint8 blueFplx;                     // AN 蓝票(原)发票类型-红票专用(通用发票类型);007-纸普 025-卷式 026-电普 999-其他
    bool zfbz;                          //作废标志，开票时带上作废标志即为空白作废
    bool isRed;                         //是否红票/蓝票（正常发票）
    bool isMultiTax;                    //是否多税率
    bool utf8Invoice;                   //是否UTF8发票（默认为否）
    //====发票抬头相关
    char gfdzdh[256];                   //购方地址电话
    char gfmc[256];                     //购方名称
    //若客户是企业，客户识别号是客户企业的税号。 若客户属机关、事业等单位性质的，可填写组织机构代码，并在前面补足6位“0”。若客户无税务登记证或组织机构代码证的，参照增值税普通发票开具规范
    //规定如下：前四位录入“0”，第五、六位录入特殊行业的两位特定编码：军警类：11（包括部队、武警、边防等）；运输类：21（包括船舶、航空公司等）；行政类：31（包括街道居委、村委、社会团体等）；外事类：41（包括外事机构、外籍居民等）；无法归属的类别列其他：91。剩余位数补足“0”。
    //例如开具给某部队的，纳税人识别号栏填写：000011000000000。若客户是个人，开具发票单笔销售额不超过万元，税号可以空着。开具发票单笔销售额超过万元，填写个人的真实姓名和身份证号。
    char gfsh[32];                      //购方税号
    char gfyhzh[256];                   //购方银行账号
    char xfdzdh[256];                   //销方地址电话
    char xfmc[256];                     //销方名称
    char xfsh[32];                      //销方税号
    char xfyhzh[256];                   //销方银行账号
    //====时间相关
    char ssyf[12];                      //所属月份
    char zfsj[24];                      //作废时间,标准时间格式yyyymmddhhmmss
    char zfsj_F2[24];                   //作废时间格式2 2019-07-19 14:51:00
    char kpsj_standard[24];             //开票时间统一标准格式 20190719145100,后续格式可以通过TranslateStandTime函数转换后获得
    char kpsj_F1[24];                   //开票时间格式1 20190719 14:51:00
    char kpsj_F2[24];                   //开票时间格式2 2019-07-19 14:51:00
    char kpsj_F3[24];                   //开票时间格式3 20190719145100
    //====价格相关
    char jshj[64];                      //价税合计
    char se[64];                        //合计税额
    char je[64];                        //合计金额
    char slv[16];                       //综合税率
    //====商品相关
    char zyspmc[128];                   //主要商品名称
    char qdbj[4];                       //清单标记, 只有"Y"/"N"两个值，考虑到aisino代码，不要修改为bool类型
    char dkbdbs[128];                   //抵扣标示?,废弃
    char yddk[8];                       //远端抵扣?,废弃
    char redNum[24];                    //专票红票-红票通知单编号
	bool isHzxxb;						//是否为红字信息表申请  0开票  1红字信息表申请
	uint8 hzxxbsqsm;					//红字信息表申请说明  0销售方发起（0000000100）  1购买方发起--已抵扣（1100000000）   2购买方发起--未抵扣（1010000000）
    char hsjbz[8];                      //SPXX含税价标志,官方只在商品信息中定义是否含税,0不含税;1含税;2混合(cntax暂无混合含税价标志)
    char zyspsmmc[128];                 //主要商品税目名称   
    char yysbz[50];                     //营业税标志 0000000010:减按 1.5%税率(J);0000000020:差额税;0000000060:农产品收购(JU);0000000090:成品油(J);0000000012:机动车专票(J);0000000000:其他;默认为0000000000
    char bmbbbh[8];                     //编码表版本号,研究后确认aisino是从数据库中查询已恢复发票最大值进行选择和通过在线更新方式进行更新，都不符合我方使用要求，因此采用固定值
    int spsl;                           //商品数量
    //====发票底部底层相关
    char fhr[128];                      //复核人
    char kpr[128];                      //开票人
    char skr[128];                      //收款人
    char bz[512];                       //备注
    char jym[64];                       //校验码（专票为空）
    char mw[128];                       //密文
    char sign[512];                     //签名 Base64
    char qmcs[18];                      // NC 签名参数,nisec16字节,cntax4字节，实际只有前4个字节为签名长度有效
    char kjbbh[256];                    // A 开具版本号Base64(RsaPubKeyEncrypt(fpdm@fphm@zSwzjVersion)))
    //====上报查询相关
    char slxlh[64];                     //上报获取受理序列号，查询服务器发票上报后的处理状态
    char szPeriodCount[16];             //税期（该值错误将会导致月底抄报清卡错误）,报税期数 (增普增专)",
    uint32 dzsyh;                 		//电子受验号，该号码将用于后期税盘对于刚开发票的查找，很重要
    bool bIsUpload;                     //==1 已经上报，==0 尚未上报

    //====程序内部使用指针
    struct Spxx *stp_MxxxHead;          //商品明细信息
    HDEV hDev;                          //税盘环境变量和静态参数信息
    uint8 *pRawFPBin;                   // N 原始发票解密后数据 4byte长度+data,用于百旺发票上报；aisino暂且为空
    //====蒙柏兼容自维护，仅提供结构体支持
    char kplx[4];                       // 开票类型,0/1, blue or red，已被isRed替代
    char fpqqlsh[128];                  //发票请求流水号
    char redNum_serial[50];             //专票红票-红票申请流水号
    char redNum_ztmc[64];               //专票红票-红票申请状态名称
    char redNum_ztdm[64];               //专票红票-红票申请状态代码
	char redNum_sqsm[10];               //专票红票-红票申请说明
    //====Deprecated，主干代码不再支持,多个版本后将考虑删除,请逐渐淘汰该变量
    char tax_nature[20];				//纳税人性质
	int need_restart;
	int need_ofdurl;       
	int allow_error;	                //允许计算误差
	long double d_zsewc;
    bool isRedDetailList;              //是否为红字清单发票	
};
typedef struct Fpxx *HFPXX;
// clang-format on

//################################各个模块所需结构############################
//-----httpsclt begin
struct HTTP {
    HDEV hDev;                               //经典设备操作句柄
    struct UploadAddrModel modelTaAddr;      //税局地址描述信息
    struct ContextConfirmRequest ctxCfmReq;  //确认平台上下文
    // ssl
    SSL_CTX *ctx;
    SSL *ssl;
    // libevent
    struct evhttp_uri *uri;            // evhttp_uri_free
    struct event_base *evBase;         // 主loop循环
    struct evhttp_connection *evConn;  // 连接句柄
    struct bufferevent *evEvent;       // free auto
    struct evdns_base *evDns;          // dns异步查询
    struct event *evBreak;             // IO中断检测
    struct event *evIoTimeout;         // IO总超时检测
    // dynamic data
    int nHttpRepCode;
    struct evbuffer *bufHttpRep;
    struct evbuffer *bufCookies;
#ifdef _CONSOLE
    BIO *bioKeylog;
#endif
};
typedef struct HTTP *HHTTP;
//-----httpsclt finish




#define DF_MAX_INV_TYPE		8
#define DF_MAX_INV_COILS	20
#define DF_MAX_SUPPORT_SLV	6
#define DF_MAX_INV_LEN		3*1024*1024

#define DF_UPLOAD_INV 0
#define DF_UPLOAD_SUM 1

struct HandlePlate
{
	int nBusID;
	int nDevID; 
	int usb_type;
	int need_check_passwd;
	char busid[32];
	int usb_port;
	int need_free;
	HUSB hUSB;
	HDEV hDev;
};
//税盘基本信息
struct _plate_basic_info
{
	int result;          // result是返回状态码，0 纳税人名称和税号都没有 1 能读出纳税人名称 2 能读出纳税人税号 3 都能读出 
	unsigned char ca_name[200];
	unsigned char ca_number[50];
	unsigned char plate_num[20];
	unsigned short plate_type;
	unsigned short plate_test;
	unsigned short extension;
	unsigned short cert_passwd_right;//默认口令是否正确,0不正确 1正确		金税盘：12345678		模拟盘：11111111		税控盘：12345678
	unsigned short dev_passwd_right;//默认设备密码是否正确,0不正确 1正确    金税盘：				模拟盘：				税控盘：88888888		税务ukey：88888888
	//unsigned short auth;	//此税盘所在端口是否授权，0未授权，1已授权
	//unsigned short aisino_sup;
	//unsigned short nisec_sup;
	//unsigned short cntax_sup;
	//unsigned short mengbai_sup;
	//unsigned char start_time[20];
	//unsigned char end_time[20];
	//unsigned char frozen_ago_time[20];
	unsigned char used_ofd[3];	//是否接入公共服务平台
	unsigned char server_type[20];//公共服务平台类型
	unsigned char server_number[50];//公共服务平台号码
	int cert_left_num;//证书口令验证剩余次数  cert_passwd_right等于0时有效  0-10为剩余次数  小于0则代表没有成功验证口令
	int dev_left_num;//设备密码验证剩余次数  dev_passwd_right等于0时有效  0-10为剩余次数  小于0则代表没有成功验证口令
	int cert_err_reason; //
	int dev_err_reason;//
	int use_downgrade_version;//是否为降版本使用，0否 1是
	char minimum_version[50];
};

//税盘税务相关信息
struct _plate_tax_info
{
	int inv_type_num;
	
	unsigned char area_code[20];		//区域代码
	unsigned char inv_type[10];			//支持的发票种类 4 5 6 7 25 26
	unsigned char tax_office_code[20];	//所属税务机关代码
	unsigned char tax_office_name[100];	//所属税务机关名称
	unsigned char tax_server_url[256];	//所属税局服务器地址
	unsigned char integrated_server_url[256];	//增值税综合服务平台地址
	unsigned char report_tax_status[200];//报税状态
	unsigned char offline_inv_status[200];//离线发票上传状态
	unsigned char current_report_time[20]; //当前清卡时间
	unsigned char driver_ver[200];			//驱动版本号
	unsigned char driver_ver_err[200];		//驱动版本过低错误状态
	unsigned char next_report_time[20];		//下次清卡时间
	unsigned char startup_date[20];		//启用日期
	unsigned char tax_nature[20];		//纳税人性质
	unsigned char plate_time[20];		//税盘当前时间
	//unsigned int lxsc;			//离线时长

	int off_inv_num;					//是否存在离线票
	int off_inv_dzsyh;					//离线票地址索引号
	unsigned char off_inv_fpdm[20];		//离线票发票代码
	unsigned char off_inv_fphm[20];		//离线票发票号码
	unsigned char off_inv_kpsj[20];		//离线票开票时间
	
};

struct _inv_type_sum_data
{
	//char qssj[24];      //ANC  起始时间 yyyymmdd, Aisino yymm00
	//char jzsj[24];      //ANC  截止时间 yyyymmdd, Aisino yymm00
	//char qckcfs[24];    //ANC  期初库存
	//char lgfpfs[24];    //ANC  领购发票份数
	//char thfpfs[24];    //ANC  退回发票份数
	//char qmkcfs[24];    //ANC  期末库存份数

	int qckcfs;				//期初库存
	int lgfpfs;				//领购发票份数
	int thfpfs;				//退回发票份数
	int qmkcfs;				//期末库存份数
	int zsfpfs;				//正数发票份数
	int zffpfs;				//正废发票份数
	int fsfpfs;				//负数发票份数
	int fffpfs;				//负废发票份数
	int kffpfs;				//空废发票份数
	long double zsfpljje;	//正数发票累计金额
	long double zsfpljse;	//正数发票累计税额
	long double zffpljje;	//正废发票累计金额
	long double zffpljse;	//正废发票累计税额
	long double fsfpljje;	//负数发票累计金额
	long double fsfpljse;	//负数发票累计税额
	long double fffpljje;	//负废发票累计金额
	long double fffpljse;	//负废发票累计税额
	long double sjxsje;		//实际销售金额
	long double sjxsse;		//实际销售税额
};

//税盘某票种监控信息
struct _monitor_info
{
	unsigned short state;		//结构体状态
	unsigned char cbqkzt[3];		//0待抄报  1待清卡  2已完成
	unsigned char kpjzsj[19];	//开票截止日期
	unsigned char bsqsrq[19];	//报送起始日期
	unsigned char bszzrq[19];	//报送终止日期
	unsigned char zxbsrq[19];	//最新报税日期
	unsigned char dzkpxe[64];	//单张开票限额
	unsigned char lxzsljje[64];	//离线正数累积金额
	unsigned int lxkpsc;			//离线时长
	unsigned char lxzssyje[64];//离线正数剩余金额

	struct _inv_type_sum_data  type_sum;

	long double sjxsje;			//当月实际开票金额
	long double sjxsse;			//当月实际开票税额
};

//税盘未使用发票卷信息
struct _invoice_coil
{
	unsigned short state;		//结构体状态
	unsigned char fplbdm[20];	//发票类别代码
	unsigned int fpqshm;		//发票起始号码
	unsigned int fpzzhm;		//发票终止号码
	unsigned int fpsyfs;		//发票剩余份数
	unsigned char fpgmsj[19];	//购买时间
	unsigned int fpgmsl;		//购买数量
};

//税盘某票种未使用发票卷信息
struct _invoice_coils_info
{
	unsigned short state;		//结构体状态
	unsigned int zsyfpfs;		//总剩余发票份数
	unsigned short wsyfpjsl;	//未使用发票卷数量
	unsigned char dqfpdm[20];   //当前发票代码
	unsigned int  dqfphm;		//当前发票号码
	struct _invoice_coil invoice_coil[DF_MAX_INV_COILS];
};



//税盘全部授权税率信息
struct _plate_support_slv
{
	unsigned short state;						//结构体状态
	unsigned short slv_sl;
	unsigned int slv[DF_MAX_SUPPORT_SLV];		//授权税率支持数组
};

//离线票信息
struct _invoice_offline_info
{
	unsigned short state;						//结构体状态
	int offline_num;
};

//税盘类型中信息结构体
struct _invoice_type_infos
{
	unsigned short state;						//结构体状态
	unsigned char fplxdm;						//发票类型代码内存序号固定 内存0 4专票  内存1=5机动车  内存2=6二手车   内存3=7普票  内存4=9货运   内存5=25卷票  内存6=26电票 内存7=28电专
	struct _monitor_info			monitor_info;			//税盘某票种监控信息
	struct _invoice_coils_info		invoice_coils_info;		//税盘某票种未使用发票卷信息
	struct _plate_support_slv		plate_support_slv;		//税盘全部授权税率信息
	//struct _plate_support_slv		plate_support_noslv;	//税盘不含税税率信息
	struct _invoice_offline_info	invoice_offline_info;	//离线票信息
};

//税盘全部信息结构体
struct _plate_infos
{
	struct _plate_basic_info		plate_basic_info;		//税盘基本信息
	struct _plate_tax_info			plate_tax_info;			//税盘税务相关信息
	struct _invoice_type_infos		invoice_type_infos[DF_MAX_INV_TYPE + 1];//最多支持7个票种 004专票 005机动车 006二手车 007普票 009货运 025卷票 026电普 028电专
};

//发票开具结果
struct _make_invoice_result
{
	unsigned char fplxdm;	//发票类型
	unsigned char fpdm[20];	//发票代码
	unsigned char fphm[20];	//发票号码
	unsigned char kpsj[20];	//开票时间
	unsigned char hjje[20];	//合计金额
	unsigned char hjse[20]; //合计税额
	unsigned char jshj[20]; //价税合计
	unsigned char jym[22];	//校验码
	unsigned char mwq[200];	//密文区
	unsigned char fpqqlsh[55];//发票请求流水号
	unsigned char bz[500];
	int sfsylsh;//是否使用流水号
	int fpdzsyh;//发票地址索引号
	int need_restart;//需要重启税盘电源
	//int need_retryup;//需要重试上传
	int test_only;
	char *scfpsj;//要上传的发票数据,数据为zlib压缩数据
	unsigned char errinfo[1024];
	unsigned char ofdurl[256];
};

//离线发票上传结构体
struct _offline_upload_result
{
	unsigned char fplxdm;	//发票类型
	unsigned char fpdm[20];	//发票代码
	unsigned char fphm[20];	//发票号码
	unsigned char kpsj[20];	//开票时间
	unsigned char hjje[20];	//合计金额
	unsigned char hjse[20]; //合计税额
	unsigned char jshj[20]; //价税合计
	unsigned char jym[22];	//校验码
	unsigned char mwq[200];	//密文区
	//unsigned char fpqqlsh[55];//发票请求流水号
	unsigned char bz[500];
	//int sfsylsh;//是否使用流水号
	int fpdzsyh;//发票地址索引号
	int zfbz;//0未作废  1已作废
	//int need_restart;//需要重启税盘电源
	int need_retryup;//需要重试上传
	//int test_only;
	char *scfpsj;//要上传的发票数据,数据为zlib压缩数据
	unsigned char errinfo[1024];
	//unsigned char ofdurl[256];
};

//红字信息表申请结果
struct _askfor_tzdbh_result
{
	unsigned char lzfpdm[20];	//发票代码
	unsigned char lzfphm[20];	//发票号码
	unsigned char tzdbh[50];
	unsigned char sqlsh[100];	//申请流水号
	unsigned char errinfo[1024];
	unsigned char *red_inv;
	int dzsyh;//发票地址索引号
};


//发票作废结果
struct _cancel_invoice_result
{
	unsigned char fplxdm;	//发票类型
	unsigned char fpdm[20];	//发票代码
	unsigned char fphm[20];	//发票号码
	unsigned char kpsj[20]; //开票时间
	unsigned char hjje[20];	//合计金额
	unsigned char hjse[20]; //合计税额
	unsigned char jshj[20]; //价税合计
	unsigned char zfsj[20]; //作废时间
	unsigned char jym[22];	//校验码
	unsigned char mwq[110];	//密文区
	unsigned char zfr[20];	//作废人
	unsigned char errinfo[1024];
	char *scfpsj;//要上传的发票数据,数据为base数据
	int need_restart;//需要重启税盘电源
	int zfzs;	//作废总数
};

//抄报数据
struct _copy_report
{
	unsigned char fplxdm;
	unsigned char cbsj[20];
	unsigned char hzksrq[20];	//汇总开始日期
	unsigned char hzjzrq[20];	//汇总截止日期
	unsigned int zcfpkjfs;		//正常发票开具份数
	unsigned int hpkjfs;		//红票开具份数
	unsigned int zcfpzffs;		//正常发票作废份数
	unsigned int hpzffs;		//红票作废份数
	unsigned int kbfs;			//空白份数
	unsigned char zcfpkjje[20]; //正常发票开具金额
	unsigned char zcfpse[20];   //正常发票税额
	unsigned char zcfpjshj[20];	//正常发票价税合计
	unsigned char hpkjje[20];	//红票开具金额
	unsigned char hpse[20];		//红票税额
	unsigned char hpjshj[20];	//红票价税合计
};

#define DF_INV_ACTION_KP 100
#define DF_INV_ACTION_ZF 101
#define DF_INV_ACTION_SC 102

struct _upload_inv_type
{
	int state;
	int fplx;
	char zhkpsj[20];
	char zhzfsj[20];
	char zhkpdm[14];
	char zhkphm[10];
	char zhzfdm[14];
	char zhzfhm[10];
};

struct _need_upload_dmhm
{
	int state;
	char fpdm[14];
	char fphm[10];
	int up_flag;
};



struct _inv_sum_data
{
	struct _inv_type_sum_data  type_sum[4]; //专、普、电、电专
};

struct _upload_inv_num
{
	int state;
	int in_used;			//0未使用  1在使用
	char summary_month[10];	//汇总数据上传月份
	int just_single_month;		//仅同步一个月发票
	char invoice_month[10];	//月份
	char *dmhmzfsbstr;//当月已有发票数据
	char plate_num[13];		//盘号
	char year_month[10];	//月份
	char year_month_data[7500];//当月已有发票数据
	int now_num;			//当前发票总数
	int auto_up_end;		//自动上传结束标志 0未结束  1结束
	int need_fpdmhm_flag;		//有单张发票需要上传 0无需要上传的单张发票  1有需要上传的单张发票
	struct _upload_inv_type upload_inv_type[4];//支持4、7、26、28四个票种
	struct _need_upload_dmhm need_upload_dmhm[10];//支持10张发票缓存，超过10张发票则整月查询上传
};

//固定了，不能再改动
enum enBufSize {
    BS_BLOCK = 1024,  //字符串类(块))通用缓冲区大小
    BS_HUGE = 512,    //字符串类(巨大)通用缓冲区大小
    BS_BIG = 256,     //字符串类(大)通用缓冲区大小
    BS_NORMAL = 128,  //字符串类通用缓冲区大小
    BS_SMALL = 64,    //字符串类(小)通用缓冲区大小
    BS_TINY = 24,     //字符串类(很小)通用缓冲区大小，一般用作时间
    BS_LITTLE = 48,   //字符串类(比较小)通用缓冲区大小
    // int和long 32位范围为2147483647（2^31-1 ~ 10^9）-2147483648(-2^31)
    // long long 64位范围为9223372036854775807（10^18），-9223372036854775808
    // float 范围为3.40282e+038（10^38），1.17549e-038（10^-38）
    // double 范围为1.79769e+308（10^308），2.22507e-308（10^-308）
    // float范围为3.40282e+038（10^38），1.17549e-038（10^-38）
    //
    // 1111111111111111111111111111111111111111111111
    // 0.11111111111111111111111111111111111111111111
    // 1111111111111111.11111111111111111111111111111
    BS_JE = 48,  //金额类通用缓冲区大小(内部计算支持，但是对外不支持双精度浮点,只支持到单精度浮点)
    BS_COUNT = 16,   //计数类通用缓冲区大小
    BS_NAME = 12,    //人员姓名
    BS_SERIAL = 32,  //编号类通用缓冲区大小
    BS_TAXID = 24,   //税号缓冲区大小，一般最长20位,+\0,21位，多留几位
    BS_DEVID = 14,   //税号缓冲区大小，一般最长12位,+\0,13位
    BS_FPDMHM = 14,  //税号缓冲区大小，一般最长12位,+\0,13位
    BS_FLAG = 2,     //标识位，只有0和1 Y或N
    BS_16 = 16,      //固定16字节
    BS_8 = 8,        //固定8字节
};



struct ContextHttpConn {
     // ssl
    SSL_CTX *ctxSsl;
    SSL *ssl;
    // libevent
    struct evhttp_uri *uri;            // 请求目的URL
    struct event_base *evBase;         // 主loop循环
    struct evhttp_connection *evConn;  // 连接句柄
    struct bufferevent *evEvent;       // free auto
    struct evdns_base *evDns;          // dns异步查询
    struct event *evBreak;             // IO中断检测
    struct event *evIoTimeout;         // IO总超时检测
    //HTTP部分靠连接层的变量
    struct evbuffer *bufCookies;//连接过程中使用的cookies
    struct evhttp_uri *uriLocation;//如果nHttpRepCode是302，该值才有效
    struct evbuffer *bufLocation;      //evhttp_uri兼容性不好(#后字符串不保留)，且为单向转换，因此需要保留原始字符串
    int nHttpRepCode;
    bool bReponseCallbackCheckFail;//默认为false，如果为true则错
};


struct EHTTP {
    struct EtaxUploadAddrModel modelTaAddr;      //税局地址描述信息
    struct ContextConfirmRequest ctxCfmReq;  //确认平台上下文
    // ssl
    SSL_CTX *ctx;
    SSL *ssl;
    // libevent
    struct evhttp_uri *uri;            // evhttp_uri_free
    struct event_base *evBase;         // 主loop循环
    struct evhttp_connection *evConn;  // 连接句柄
    struct bufferevent *evEvent;       // free auto
    struct evdns_base *evDns;          // dns异步查询
    struct event *evBreak;             // IO中断检测
    struct event *evIoTimeout;         // IO总超时检测
    // dynamic data
    struct ContextHttpConn ctxCon;       // HTTP连接层上下文
    //====回调，分态处理，调用者细节处理
    // 新的HTTP请求创建时执行的函数，用于设置新的HTTP请求头信息等等
    int (*cbHttpNewReq)(struct EHTTP *hi, struct evhttp_request *req);
    //服务器产生响应后的回调
    int (*cbHttpReponse)(struct EHTTP *hi, struct evhttp_request *req, struct evkeyvalq *headers);
    // HTTP关闭回调函数，用于HTTP清理和关闭，子模块上下文的清理
    int (*cbHttpClose)(struct EHTTP *hi);
    //====动态数据
    struct evbuffer *bufHttpRep;  // HTTP响应和返回数据保存，因使用较多，暂不存放ContextHttpConn
    //====内部指针
    HDEV hDev;  //经典设备操作句柄
    //子模块自定义应用层上下文(子模块需求太复杂，且申请内存较多，尽量不在主模块占用内存)
    void *ctxApp;
    int nHttpRepCode;
    struct evbuffer *bufCookies;
    
    //用户参数指针
    char *user_args;

#ifdef _CONSOLE
    BIO *bioKeylog;
#endif
};
typedef struct EHTTP *EHHTTP;


// aisino/nisec叫法不一致
struct ESpxx {
    //====程序内部指针
    struct ESpxx *stp_next;
    //====标准字段
    char xh[BS_8];                          //序号
    char spsmmc[BS_SMALL];                  //商品税目名称
    char spmc[BS_NORMAL];                   //商品名称
    char yhsm[BS_SMALL];                    //优惠说明=增值税特殊管理ZZSTSGL,减按计增时使用
    char spbh[BS_SERIAL];                   //商品编号(spbh)=分类编码(flbm)=商品编码(spbm)
    char zxbh[BS_SERIAL];                   //自行编码,目前我方尚未使用
    char spsm[BS_SERIAL];                   //商品税目编码,目前我方尚未使用
    char ggxh[BS_SMALL];                    //规格型号,打印36个字符（18个汉字）,从22（含22）个字符开始换行
    char jldw[BS_TINY];                     //计量单位
    char dj[BS_JE];                         //单价,nisec存在小数点后10位转换
    char sl[BS_TINY];                       //数量
    char je[BS_JE];                         //金额
    char se[BS_JE];                         //税额
    char slv[BS_8];                         //税率
    char fphxz[BS_FLAG];                    //发票行性质,0 正常行;1 折扣行;2 被折扣行
    char hsjbz[BS_FLAG];                    //含税价标志
    char xsyh[BS_FLAG];                     //销售优惠=优惠政策标识YHZCBS 0未使用，1使用
    char lslvbs[BS_FLAG];                   //零税率标示=零税率标识LSLBS 空代表正常税率;1 出口免税和其他免税优惠政策（免税）;2 不征增值税（不征税）;3 普通零税率（0%）
    //====全电字段 全新设计，避免经典开具导入时金额和单价计算混乱问题
    char hsje[BS_JE];                       //含税金额
    char bhsje[BS_JE];                      //不含税金额
    char hsdj[BS_JE];                       //含税单价
    char bhsdj[BS_JE];                      //不含税单价
    //====我方扩展字段
    char kce[BS_JE];                        //差额征税扣除额
    //====新增字段
};

//这里仅仅是单个信息，如果导入时有多个发票，还需要做出链表形式
struct EFpxx {   
    //==设备相关，由hDev刷新，发票恢复时需要遵循该发票开票时的信息，不要使用hDev刷新
    char jqbh[BS_DEVID];                       // AN 机器编号近似等于设备编号，aisino部分发票类型该值需要为空，nisec全填充   
    char kprjbbh[BS_SERIAL];                   // A 开票软件版本号,nisec亦有尚未使用到
    char driverVersion[BS_SERIAL];             // A 设备底层版本号,nisec亦有尚未使用到
    char jmbbh[BS_8];                          // A 加密版本号,nisec亦有尚未使用到 aisino_jmbbh
    char taVersion[BS_SERIAL];                 // A 总局版本号,nisec亦有尚未使用到 aisino_xfver
    int nVerInvoice;                           // NC 发票版式版本
    int kpjh;                                  // ANC 开票机号
    //==发票相关
    char blueFpdm[BS_FPDMHM];                  //蓝票(原)发票代码-红票专用
    char blueFphm[BS_FPDMHM];                  //蓝票(原)发票号码-红票专用
    char blueKpsj[BS_TINY];                    //蓝票(原)开票时间-红票专用 yyyymmddhhmmss
    char fpdm[BS_FPDMHM];                      //发票代码
    char fphm[BS_FPDMHM];                      //发票号码
    char fpendhm[BS_FPDMHM];                   //发票卷结束号码
    char pubcode[BS_SMALL];                    //公共服务平台配置 类型;代码;提取码随机数；01公共服务平台02企业自建平台03第三方平台
    uint32 specialFlag;                        //特殊发票处理标志（非税局，我方自定义）
    uint8 fpzt;                                // ANC 发票状态,参考enCOMMON_FP_STATUS,NC支持全状态标记。注意！Aisino目前读取全部支持，开具仅支持FPSTATUS_BLANK_WASTE空白作废标记
    uint8 fplx;                                //ANC 税局、百旺发票类型,与fplx_aisino相关联,两个发票类型同时存在消亡
    uint8 fplx_aisino;                         //ANC 航信发票类型,在航信中使用，否则使用通用发票类型
    uint8 fplx_etax;                           //E 电子税务局全电发票类型
    uint8 zyfpLx;                              //专用发票类型
    uint8 sslkjly;                             // A SSLKJLY ?税率开具理由
    uint8 hcyy;                                // AN 红冲原因
    uint8 blueFplx;                            // AN 蓝票(原)发票类型-红票专用(通用发票类型);007-纸普 025-卷式 026-电普 999-其他
    bool gfzrr;                                // E  购方自然人标识
    bool zfbz;                                 //作废标志，开票时带上作废标志即为空白作废
    bool isRed;                                //是否红票/蓝票（正常发票）
    bool isMultiTax;                           //是否多税率
    bool utf8Invoice;                          //是否UTF8发票（默认为否）
    bool isRedDetailList;                      //是否为红字清单发票
    //====发票抬头相关
    char gfdzdh[BS_NORMAL];                    //购方地址电话
    char gfmc[BS_NORMAL];                      //购方名称
    //若客户是企业，客户识别号是客户企业的税号。 若客户属机关、事业等单位性质的，可填写组织机构代码，并在前面补足6位“0”。若客户无税务登记证或组织机构代码证的，参照增值税普通发票开具规范
    //规定如下：前四位录入“0”，第五、六位录入特殊行业的两位特定编码：军警类：11（包括部队、武警、边防等）；运输类：21（包括船舶、航空公司等）；行政类：31（包括街道居委、村委、社会团体等）；外事类：41（包括外事机构、外籍居民等）；无法归属的类别列其他：91。剩余位数补足“0”。
    //例如开具给某部队的，纳税人识别号栏填写：000011000000000。若客户是个人，开具发票单笔销售额不超过万元，税号可以空着。开具发票单笔销售额超过万元，填写个人的真实姓名和身份证号。
    char gfsh[BS_TAXID];                      //购方税号
    char gfyhzh[BS_NORMAL];                   //购方银行账号
    char gfkhh[BS_NORMAL];                    //E 购方开户行
    char xfdzdh[BS_NORMAL];                   //销方地址电话
    char xfmc[BS_NORMAL];                     //销方名称
    char xfsh[BS_TAXID];                      //销方税号
    char xfyhzh[BS_NORMAL];                   //销方银行账号
    char xfkhh[BS_NORMAL];                    //E 销方开户行
    //====时间相关
    char ssyf[BS_TINY];                       //所属月份
    char zfsj[BS_TINY];                       //作废时间,标准时间格式yyyymmddhhmmss
    char zfsj_F2[BS_TINY];                    //作废时间格式2 2019-07-19 14:51:00
    char kpsj_standard[BS_TINY];              //开票时间统一标准格式 20190719145100,后续格式可以通过TranslateStandTime函数转换后获得
    char kpsj_F1[BS_TINY];                    //开票时间格式1 20190719 14:51:00
    char kpsj_F2[BS_TINY];                    //开票时间格式2 2019-07-19 14:51:00
    char kpsj_F3[BS_TINY];                    //开票时间格式3 20190719145100
    //====价格相关
    char jshj[BS_JE];                         //价税合计
    char se[BS_JE];                           //合计税额
    char je[BS_JE];                           //合计金额
    char slv[BS_16];                          //综合税率
    //====商品相关
    char zyspmc[BS_NORMAL];                   //主要商品名称
    char dkbdbs[BS_FLAG];                     //抵扣标示?,废弃
    char yddk[BS_FLAG];                       //远端抵扣?,废弃
    char redNum[BS_TINY];                     //专票红票-红票通知单编号
    char hsjbz[BS_FLAG];                      //SPXX含税价标志,官方只在商品信息中定义是否含税,0不含税;1含税;2混合(cntax暂无混合含税价标志)
    char zyspsmmc[BS_NORMAL];                 //主要商品税目名称   
    char yysbz[BS_SMALL];                     //营业税标志 0000000010:减按 1.5%税率(J);0000000020:差额税;0000000060:农产品收购(JU);0000000090:成品油(J);0000000012:机动车专票(J);0000000000:其他;默认为0000000000
    char bmbbbh[BS_8];                        //编码表版本号,研究后确认aisino是从数据库中查询已恢复发票最大值进行选择和通过在线更新方式进行更新，都不符合我方使用要求，因此采用固定值
    int spsl;                                 //商品数量
    bool isHzxxb;	   	                      //是否为红字信息表申请  0开票  1红字信息表申请
    bool qdbz;                                //清单发票标志
	uint8 hzxxbsqsm;			      	      //红字信息表申请说明  0销售方发起（0000000100）  1购买方发起--已抵扣（1100000000）   2购买方发起--未抵扣（1010000000）
    //====二手车相关
    char gfdh[BS_TINY];                       //CE 购方电话-二手车、全电发票
    char gfdz[BS_NORMAL];                     //CE 购方地址-二手车、全电发票
    char xfdh[BS_TINY];                       //CE 销方电话-二手车、全电发票
    char xfdz[BS_NORMAL];                     //CE 销方地址-二手车、全电发票
    char cpzh[BS_TINY];                       //C 车牌照号
    char djzh[BS_TINY];                       //C 登记证号
    char cllx[BS_SMALL];                      //C 车辆类型
    char clsbh[BS_TINY];                      //C 车辆识别号
    char cpxh[BS_SMALL];                      //C 厂牌型号
    char zrdclglsmc[BS_SMALL];                //C 转入地车辆管理所名称
    char zjsmc[BS_NORMAL];                    //C 中间商名称(经营拍卖|二手车市场)
    char zjsdz[BS_NORMAL];                    //C 中间商地址(经营拍卖|二手车市场)
    char zjssh[BS_TAXID];                     //C 中间商税号(经营拍卖|二手车市场)
    char zjsyhzh[BS_NORMAL];                  //C 中间商银行账号(经营拍卖|二手车市场)
    char zjsdh[BS_TINY];                      //C 中间商电话(经营拍卖|二手车市场)
    bool fxkjbz;                              //C 反向开具标志 0为正向开具发票 1为反向开具发票
    uint8 kpflx;                              //C 开票方类型，等同于hDev->stDevExtend.esjdcbs
    //====发票底部底层相关
    char gfjbr[BS_NAME];                     //E 购方经办人
    char jbrsfzjhm[BS_TINY];                 //E 经办人身份证件号码
    char jbrzrrnsrsbh[BS_TINY];              //E 经办人自然人纳税人识别号
    uint8 jbrsfzjzldm;                       //E 经办人身份证件号码代码
    uint8 jbrgjdm;                           //E 经办人国家代码
    char fhr[BS_NAME];                       //复核人
    char kpr[BS_NAME];                       //开票人
    char skr[BS_NAME];                       //收款人
    char bz[BS_HUGE];                        //备注
    char jym[BS_SMALL];                      //校验码（专票为空）
    char mw[BS_NORMAL];                      //密文(二手车为空)
    char sign[BS_HUGE];                      //签名 Base64
    char qmcs[BS_TINY];                      // NC 签名参数,nisec16字节,cntax4字节，实际只有前4个字节为签名长度有效
    char kjbbh[BS_BIG];                      // A 开具版本号Base64(RsaPubKeyEncrypt(fpdm@fphm@zSwzjVersion)))
    //====上报查询相关
    char slxlh[BS_SMALL];                    //上报获取受理序列号，查询服务器发票上报后的处理状态
    char szPeriodCount[BS_TINY];             //税期（该值错误将会导致月底抄报清卡错误）,报税期数 (增普增专)",
    uint32 dzsyh;                 		     //电子受验号，该号码将用于后期税盘对于刚开发票的查找，很重要
    char cezslxDm[BS_TINY];                  //差额征收代码
    bool bIsUpload;                          //==1 已经上报，==0 尚未上报

    //====程序内部使用指针
    struct ESpxx *stp_MxxxHead;               //商品明细信息
    HDEV hDev;                               //税盘环境变量和静态参数信息
    uint8 *pRawFPBin;                        // N 原始发票解密后数据 4byte长度+data,用于百旺发票上报；aisino暂且为空
    //====蒙柏兼容自维护，仅提供结构体支持  
    char fpqqlsh[BS_SMALL];                  //发票请求流水号蒙柏方为60位
    //====Deprecated，主干代码不再支持,多个版本后将考虑删除,请逐渐淘汰该变量
};
typedef struct EFpxx *EHFPXX;

#endif