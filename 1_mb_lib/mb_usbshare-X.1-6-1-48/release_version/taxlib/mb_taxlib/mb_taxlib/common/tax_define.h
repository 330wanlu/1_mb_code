/*****************************************************************************
File name:   define.h
Description: 用于存放全局定义和宏定义，任何一个参数都会对程序全局产生影响
Author:      Zako
Version:     1.0
Date:        2020.08
History:
20200813     初始化格式文档
*****************************************************************************/
#ifndef TAX_DEFINE_H
#define TAX_DEFINE_H

// 税局最新规定商品条目最大2000行，2000行商品*sizeof(STSPXXv3)=700k左右，aisino最大发票长度64K/400K
#define DEF_MAX_FPBUF_LEN 0x100000  //十进制1024*1024=1048576=1MBytes
#define DEF_BMBBBH_DEFAULT "40.0"  //在线实时更新, 税收分类编码表版本号 39.0-20210825
#define DEF_CERT_PASSWORD_DEFAULT "12345678"    //所有设备证书口令，cntax设备口令
#define DEF_DEVICE_PASSWORD_DEFAULT "88888888"  // aisino、nisec设备口令
#define DEF_SWZJ_INVOICE_GATEWAY "/acceptFramework/UniAcceptService"

enum enFPLX {
    //发票类型最大数目
    FPLX_MAX_NUM = 8,
    //通用发票类型/百旺发票类型
    FPLX_COMMON_ZYFP = 4,   //专用发票
    FPLX_COMMON_JDCFP = 5,  //机动车发票
    FPLX_COMMON_ESCFP = 6,  //二手车发票
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
    FPLX_AISINO_ESCFP = 42,  //二手车发票
    FPLX_AISINO_DZFP = 51,   //电子发票
    FPLX_AISINO_DZZP = 52    //电子专票
};

//商品类型，主要根据商品编码来判别；商品编码经常更新改动，我们通过自己的函数查询商品编码规则转换成固定的数值方便判断
//不需要一次性加全，太多了，酌情更新即可
enum enSPLX {
    SPLX_UNCLASSIFIED,  //未定义
    SPLX_CPY,           //成品油
    SPLX_JDC,           //机动车
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
    TACMD_AISINO_TZDBH_CANCEL = 54,                 // 54 红字通知单撤销

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
    TACMD_CNTAX_JYESCFPKPXX,                        // 校验二手车发票开票信息
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

struct StStockInfoGroup {
    char fpdm[BS_FPDMHM];                  // N
    char qshm[BS_FPDMHM];                  // N
    char zzhm[BS_FPDMHM];                  // N
    char dqhm[BS_FPDMHM];                  // N
    int fpfs;                              // N
    int syfs;                              // N
    char lgrq[BS_SERIAL];                  // N
    char lgry[BS_SMALL];                   // N
};

struct StStockInfoHead {
    char dqfpdm[BS_FPDMHM];                // NC 当前代码
    char dqfphm[BS_FPDMHM];                // NC 当前号码
    char dqfpzzhm[BS_FPDMHM];              // N 当前终止号码
    int zsyfs;                             // NC
    int nGroupCount;                       // NC
    void *pArray_StStockInfoGroup;         //此处存放上述结构体的结构体数组指针
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
//上位服务器地址结构
struct UploadAddrModel {
    enum enumTaAddrType bServType;
    char szRegName[BS_16];
    char szRegCode[BS_16];
    char szTaxAuthorityURL[BS_NORMAL];
    char szInitRequestPage[BS_SMALL];
};
//HTTP连接层上下文结构
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
    int nHttpRepCode;
    bool bReponseCallbackCheckFail;//默认为false，如果为true则错
};

//注意：此处各种设备的使用方式略有不同
//aisino 指定周期内统计，多发票类型，有链表，每个节点代表一个发票类型
//nisec  指定周期内统计，单发票类型，无链表，只有一个节点一个票种
//ukey   单月或者前3个月，单发票类型，链表可有可无，根据next链表指针判断
struct StMonthStatistics {
    uint8 fplx;               //A    发票类型 Aisino包含多个票种，其他则为一个票种
    char qssj[BS_TINY];       //ANC  起始时间 yyyymmdd, Aisino yymm00
    char jzsj[BS_TINY];       //ANC  截止时间 yyyymmdd, Aisino yymm00
    char qckcfs[BS_COUNT];    //ANC  期初库存
    char lgfpfs[BS_COUNT];    //ANC  领购发票份数
    char thfpfs[BS_COUNT];    //ANC  退回发票份数
    char zsfpfs[BS_COUNT];    //ANC  正数发票份数
    char zffpfs[BS_COUNT];    //ANC  正废发票份数
    char fsfpfs[BS_COUNT];    //ANC  负数发票份数
    char fffpfs[BS_COUNT];    //ANC  负废发票份数
    char kffpfs[BS_COUNT];    //NC   空废发票份数
    char qmkcfs[BS_COUNT];    //ANC  期末库存份数
    char zsfpljje[BS_JE];     //ANC  正数发票累计金额
    char zsfpljse[BS_JE];     //ANC  正数发票累计税额
    char zffpljje[BS_JE];     //NC   正废发票累计金额
    char zffpljse[BS_JE];     //NC   正废发票累计税额
    char fsfpljje[BS_JE];     //ANC  负数发票累计金额
    char fsfpljse[BS_JE];     //ANC  负数发票累计税额
    char fffpljje[BS_JE];     //NC   负废发票累计金额
    char fffpljse[BS_JE];     //NC   负废发票累计税额
    struct StMonthStatistics* next; //AC
};

// aisino/nisec叫法不一致
struct Spxx {
    //====程序内部指针
    struct Spxx *stp_next;
    //====标准字段
    char xh[BS_8];                          //序号
    char spsmmc[BS_SMALL];                  //商品税目名称
    char spmc[BS_NORMAL];                   //商品名称
    char yhsm[BS_SMALL];                    //优惠说明=增值税特殊管理ZZSTSGL,减按计增时使用
    char spbh[BS_SERIAL];                   //商品编号(spbh)=分类编码(flbm)=商品编码(spbm)
    char zxbh[BS_SERIAL];     				//自行编码,目前我方尚未使用
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
    //====我方扩展字段
    char kce[BS_JE];                        //差额征税扣除额
    char hsje[BS_JE];                       //含税金额，用于校验
    //====新增字段
};

//这里仅仅是单个信息，如果导入时有多个发票，还需要做出链表形式
struct Fpxx {   
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
    uint8 fplx;                                //税局、百旺发票类型,与fplx_aisino相关联,两个发票类型同时存在消亡
    uint8 fplx_aisino;                         //航信发票类型,在航信中使用，否则使用通用发票类型
    uint8 zyfpLx;                              //专用发票类型
    uint8 sslkjly;                             // A SSLKJLY ?税率开具理由
    uint8 hcyy;                                // AN 红冲原因
    uint8 blueFplx;                            // AN 蓝票(原)发票类型-红票专用(通用发票类型);007-纸普 025-卷式 026-电普 999-其他
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
    char xfdzdh[BS_NORMAL];                   //销方地址电话
    char xfmc[BS_NORMAL];                     //销方名称
    char xfsh[BS_TAXID];                      //销方税号
    char xfyhzh[BS_NORMAL];                   //销方银行账号
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
    char gfdh[BS_TINY];                       //购方电话-仅二手车使用
    char gfdz[BS_NORMAL];                     //购方地址-仅二手车使用
    char xfdh[BS_TINY];                       //销方电话-仅二手车使用
    char xfdz[BS_NORMAL];                     //销方地址-仅二手车使用
    char cpzh[BS_TINY];                       //车牌照号
    char djzh[BS_TINY];                       //登记证号
    char cllx[BS_SMALL];                      //车辆类型
    char clsbh[BS_TINY];                      //车辆识别号
    char cpxh[BS_SMALL];                      //厂牌型号
    char zrdclglsmc[BS_SMALL];                //转入地车辆管理所名称
    char zjsmc[BS_NORMAL];                    //中间商名称(经营拍卖|二手车市场)
    char zjsdz[BS_NORMAL];                    //中间商地址(经营拍卖|二手车市场)
    char zjssh[BS_TAXID];                     //中间商税号(经营拍卖|二手车市场)
    char zjsyhzh[BS_NORMAL];                  //中间商银行账号(经营拍卖|二手车市场)
    char zjsdh[BS_TINY];                      //中间商电话(经营拍卖|二手车市场)
    bool fxkjbz;                              //反向开具标志 0为正向开具发票 1为反向开具发票
    uint8 kpflx;                              //开票方类型，等同于hDev->stDevExtend.esjdcbs
    //====发票底部底层相关
    char fhr[BS_TINY];                       //复核人
    char kpr[BS_TINY];                       //开票人
    char skr[BS_TINY];                       //收款人
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
    bool bIsUpload;                          //==1 已经上报，==0 尚未上报

    //====程序内部使用指针
    struct Spxx *stp_MxxxHead;               //商品明细信息
    HDEV hDev;                               //税盘环境变量和静态参数信息
    uint8 *pRawFPBin;                        // N 原始发票解密后数据 4byte长度+data,用于百旺发票上报；aisino暂且为空
    //====蒙柏兼容自维护，仅提供结构体支持  
    char fpqqlsh[BS_SMALL];                  //发票请求流水号蒙柏方为60位
    char tax_nature[20];				     //纳税人性质
    int need_restart;
	int need_ofdurl;       
	int allow_error;	                //允许计算误差
	long double d_zsewc;

    char qdbj[4];                       //清单标记, 只有"Y"/"N"两个值，考虑到aisino代码，不要修改为bool类型
    char redNum_serial[50];             //专票红票-红票申请流水号
    char redNum_ztmc[64];               //专票红票-红票申请状态名称
    char redNum_ztdm[64];               //专票红票-红票申请状态代码
	char redNum_sqsm[10];               //专票红票-红票申请说明
    //====Deprecated，主干代码不再支持,多个版本后将考虑删除,请逐渐淘汰该变量
};
typedef struct Fpxx *HFPXX;
// clang-format on

//################################各个模块所需结构############################
//-----httpsclt begin
struct HTTP {
    //====分类子结构
    struct UploadAddrModel modelTaAddr;  //税局地址描述信息
    struct ContextHttpConn ctxCon;       // HTTP连接层上下文
    //====回调，分态处理，调用者细节处理
    // 新的HTTP请求创建时执行的函数，用于设置新的HTTP请求头信息等等
    int (*cbHttpNewReq)(struct HTTP *hi, struct evhttp_request *req);
    //服务器产生响应后的回调
    int (*cbHttpReponse)(struct HTTP *hi, struct evhttp_request *req, struct evkeyvalq *headers);
    // HTTP关闭回调函数，用于HTTP清理和关闭，子模块上下文的清理
    int (*cbHttpClose)(struct HTTP *hi);
    //====动态数据
    struct evbuffer *bufHttpRep;  // HTTP响应和返回数据保存，因使用较多，暂不存放ContextHttpConn
    //====内部指针
    HDEV hDev;  //经典设备操作句柄
    //子模块自定义应用层上下文(子模块需求太复杂，且申请内存较多，尽量不在主模块占用内存)

    // libevent wang 0308
    struct evhttp_uri *uri;            // evhttp_uri_free
    struct event_base *evBase;         // 主loop循环
    struct evhttp_connection *evConn;  // 连接句柄
    struct bufferevent *evEvent;       // free auto
    struct evdns_base *evDns;          // dns异步查询
    struct event *evBreak;             // IO中断检测
    struct event *evIoTimeout;         // IO总超时检测

    void *ctxApp;
#ifdef _CONSOLE
    BIO *bioKeylog;
#endif
};
typedef struct HTTP *HHTTP;
//-----httpsclt finish

#endif