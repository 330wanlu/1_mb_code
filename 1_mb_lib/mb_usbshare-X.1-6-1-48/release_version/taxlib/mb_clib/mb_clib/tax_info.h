#ifndef _TAX_INFO_H
#define _TAX_INFO_H
//#include "common.h"
#include "mb_typedef.h"
#include <string.h>

enum enDeviceType {
    DEVICE_UNKNOW = 0,
    DEVICE_AISINO,  //爱信诺金税盘
    DEVICE_NISEC,   //百旺税控盘
    DEVICE_UKEY,    //税务中心UKey

};

enum enCommonFPLX {
    COMMOM_FPLX_ZYFP = 4,   //专用发票
    COMMOM_FPLX_PTFP = 7,   //普通发票
    COMMOM_FPLX_HYFP = 9,   //货运发票
    COMMOM_FPLX_JDCFP = 5,  //机动车发票
    COMMOM_FPLX_JSFP = 25,  //卷式发票
    COMMOM_FPLX_DZFP = 26,  //电子发票
    COMMOM_FPLX_ESC = 6     //二手车发票
};

enum enAnsinoFPLX {
    AISINO_FPLX_ZYFP = 0,    //专用发票
    AISINO_FPLX_PTFP = 2,    //普通发票
    AISINO_FPLX_HYFP = 11,   //货运发票
    AISINO_FPLX_JDCFP = 12,  //机动车发票
    AISINO_FPLX_JSFP = 41,   //卷式发票
    AISINO_FPLX_DZFP = 51,   //电子发票
    AISINO_FPLX_ESC = 42     //二手车发票
};


// 0一般专票 2 石脑油、燃料油增值税专用发票  普票时 0 一般普票 8农产品销售 9农产品收购
/*专用发票类型*/
enum enZYFP_LX {
    ZYFP_ZYFP,      //一般专普票
    ZYFP_HYSY,      //海洋石油
    ZYFP_SNY,       //石脑油
    ZYFP_SNY_DDZG,  //石脑油_DDZG
    ZYFP_RLY,       //燃料油
    ZYFP_RLY_DDZG,  //燃料油_DDZG
    ZYFP_XT_YCL,    //稀土_原材料
    ZYFP_XT_CCP,    //稀土_产成品
    ZYFP_NCP_XS,    //农产品_销售
    ZYFP_NCP_SG,    //农产品_收购
    ZYFP_JZ_50_15,  //建筑_50_15
    ZYFP_CEZS,      //差额征收
    ZYFP_TXF_KDK,   //通行费_可抵扣
    ZYFP_TXF_BKDK,  //通行费_不可抵扣
    ZYFP_CPY,       //成品油
    ZYFP_XT_JSJHJ,  //稀土_金属及合金
    ZYFP_XT_CPJGF   //稀土_产品加工费
};

//发票行性质
enum FPHXZ {
    FPHXZ_SPXX,         //商品信息
    FPHXZ_XJXHQD,       //详见销货清单
    FPHXZ_SPXX_ZK = 3,  //商品信息折扣(被折扣)
    FPHXZ_ZKXX,         //折扣信息(折扣)
    FPHXZ_XHQDZK,       //销货清单折扣
    FPHXZ_XJDYZSFPQD    //详见对应正数发票清单
};


struct StaticDeviceInfo {
    unsigned short usICCardNo;  //税盘分机号
    char szDeviceTime[24];      //设备时间 yyyymmddhhmmss 20191019084029
    char szNicMAC[24];          //开票通信网卡MAC地址
    char szDeviceID[24];        //设备编号SN
    char szDriverVersion[32];   //底层硬件驱动版本号
    char szJMBBH[4];            //加密版本号
    char szCommonTaxID[24];     //通用显示税号
    char szCompressTaxID[24];   //压缩税号
    char sz9ByteHashTaxID[24];  // 9位hash税号
    char szRegCode[12];         //区域代码
    //税盘公司名称,最大50个汉字 汉字在GBK编码中占2个字节，在UTF8编码中占3个字节
    char szComName[256];
    void *pX509Cert;         //用于开票签名的证书指针
    unsigned char bTrainFP;  //是否江苏测试盘(航信培训企业)
	unsigned char bTrainFP2;  //是否山西测试盘(名称等于税号)
    HUSB hUSBDevice;         //设备的libusb句柄
};
typedef struct StaticDeviceInfo *HDEV;  // replace 'struct StaticDeviceInfo *' to 'HDEV '

struct Spxx {
    char spmc[128];  //商品名称
    char spbh[128];  //商品编号(spbh)=分类编码(flbm)=商品编码(spbm);统一为商品编号,其他说法废除
    char spsm[128];   //商品税目
    char ggxh[128];   //规格型号
    char jldw[128];   //计量单位
    char dj[128];     //单价
    char sl[128];     //数量
    char je[128];     //金额（固定不含税金额）
    char slv[128];    //税率
    char se[128];     //税额
    char fphxz[128];  //发票行性质
    char hsjbz[128];  //含税价标志
    char bz[128];     //备注,请注意，这里不是UTF8明文，是GBK后的Base64
    char xh[128];     //序号
    char xthash[128];
    char bhsdj[128];
    char xsyh[128];  //销售优惠 YHZCBS
    char yhsm[128];
    char lslvbs[128];  //零税率标示
    char kce[128];     //差额征税扣除额
    char flag[128];
    char spsmmc[128];  //商品税目名称
    char zzstsgl[10];  //增值税特殊管理
    char zsfs[10];     //征收方式
    struct Spxx *stp_next;

    //新增字段
};

//这里仅仅是单个信息，如果导入时有多个发票，还需要做出链表形式
struct Fpxx {
    //---暂时不实现
    // int bool_0;
    // int bsbz;  //报税标志
    // int bsq;
    // int bszt;
    // char ccdw[128];
    // char cd[128];
    // char cllx[128];
    // char clsbdh[128];
    // char cpxh[128];
    // char cyrmc[128];
    // char cyrnsrsbh[128];
    // char czch[128];
    // char dkqymc[128];
    // char dkqysh[128];
    // int dybz;
    // char dy_mb[128];  //打印模板
    // char fdjhm[128];
    // char fhrmc[128];
    // char fhrnsrsbh[128];
    // int flag;
    // char fpqqlsh[128];
    // char hgzh[128];
    // char hxm[128];
    // int hzfw;
    // long invQryNo;
    // int isBlankWaste;
    // int isNewJdcfp;
    // char jkzmsh[128];
    // char jmbbh[64];
    // unsigned long keyFlagNo;
    // char kpsxh[128];
    // char qyd[128];
    // char rdmByte[128];
    // char sbbz[128];
    // char sccjmc[128];
    // char sfzhm[128];
    // char shrmc[128];
    // char shrnsrsbh[128];
    // char sjdh[128];
    // char wspzhm[128];
    // char xcrs[128];
    // int xfbz[128];
    // char xfyh[128];
    // char xsdjbh[128];
    // char yshwxx[128];
    // char zfsj[128];
    // char zgswjgdm[128];
    // char zgswjgmc[128];

    //-------实现
    HDEV pDevInfo;              //税盘环境变量和静态参数信息
    struct Spxx *stp_MxxxHead;  //商品明细信息
    char bmbbbh[128];           //编码表版本号
    char bz[512];               //备注;UTF8明文
    char fhr[128];              //复核人
    char fpdm[128];             //发票代码  szInvTypecode
    char fphm[128];             //发票号码  szCurrentInvNum
    char fpendhm[128];          //发票卷结束号码  szInvEndNum
    char fplx[12];              //发票类型（004=专票 005 006 007=普票 009 025 026=电票）
    char gfdzdh[256];           //购方地址电话
    char gfmc[256];             //购方名称
    char gfsh[128];             //购方税号
    char gfyhzh[256];           //购方银行账号
    char je[128];               //金额
    char jqbh[32];              //机器编号
    char jym[256];              //校验码
    char kpr[128];              //开票人
    char kpsj_standard[24];     //开票时间统一标准格式
                             // 20190719145100,后续格式可以通过TranslateStandTime函数转换后获得
    char kpsj_F1[24];           //开票时间格式1 20190719 14:51:00
    char kpsj_F2[24];           //开票时间格式2 2019-07-19 14:51:00
    char kpsj_F3[24];           //开票时间格式3 20190719145100
    char mw[128];               //密文
    char se[128];               //税额
    char sign[512];             //签名 Base64
    char skr[128];              //收款人
    char slv[128];              //税率
    char ssyf[12];              //所属月份
    char xfdzdh[256];           //销方地址电话
    char xfmc[256];             //销方名称
    char xfsh[128];             //销方税号
    char xfyhzh[256];           //销方银行账号
    uint8 zyfpLx;       //专用发票类型
    char zyspmc[128];           //专用商品名称
    char kprjbbh[128];          //开票软件版本号
    char qdbj[128];             //清单标记
    char spfmc[128];            //
    char spfnsrsbh[128];        //
    int isRed;                  //是否红票
    char dkbdbs[128];           //
    char yddk[128];             //远端端口？
    char redNum[32];            //专用发票红票通知单编号
    int zfbz;                   //作废标志
    char blueFpdm[128];         //蓝票发票代码
    char blueFphm[128];         //蓝票发票号码
    char jshj[20];              //价税合计
    char hsbz[12];              //含税标志
    char hzfw[12];              //汉字防伪
    char dslp[12];              //多税率票
    char zhsl[12];              //综合税率
    char hsjbz[128];            //含税价标志
    char zyspsmmc[128];         //专用商品税目名称
    int spsl;                   //商品数量
    char fpqqlsh[128];          //发票请求流水号
    char kplx[12];              //开票类型,kplx 0/1, blue or red
    uint8 fplx_aisino;  //航信自定义发票类型
    char tspz[12];  //特殊票种专票时（与zyfpLx重复，目前仅做解析用）   0一般专票 2
                    //石脑油、燃料油增值税专用发票  普票时 0 一般普票 8农产品销售 9农产品收购
    char yysbz[50];  //营业税标志
    char zfsj[24];   //作废时间,标准时间格式yyyymmddhhmmss
    int kpjh;        //开票机号
    //发票上报成功后，获取的受理序列号，可以通过该序列号查询服务器发票上报后的处理状态
    char slxlh[64];
    //地址索引号，发票上报成功后税盘返回的号码，该号码将用于后期税盘对于刚开发票的查找，很重要
    unsigned int dzsyh;
    char aisino_jmbbh[12];   //加密版本号
    char aisino_driver[32];  //金税设备底层版本号
    char aisino_inner[32];
    char aisino_xfver[32];
    char aisino_bmbbh[12];
    uint8 bIsUpload;  //==1 已经上报，==0 尚未上报

    //新增字段
};
typedef struct Fpxx *HFPXX;

#define DF_MAX_INV_TYPE		7
#define DF_MAX_INV_COILS	10
#define DF_MAX_SUPPORT_SLV	6
#define DF_MAX_INV_LEN		3*1024*1024



//税盘基本信息
struct _plate_basic_info
{
	int result;          // result是返回状态码，0 纳税人名称和税号都没有 1 能读出纳税人名称 2 能读出纳税人税号 3 都能读出 
	unsigned char ca_name[200];
	unsigned char ca_number[50];
	unsigned char plate_num[20];
	unsigned short plate_type;
	unsigned short extension;
};

//税盘税务相关信息
struct _plate_tax_info
{
	int inv_type_num;
	unsigned char area_code[20];		//区域代码
	unsigned char inv_type[10];			//支持的发票种类 4 5 6 7 25 26
	unsigned char tax_office_code[20];	//所属税务机关代码
	unsigned char tax_office_name[100];	//所属税务机关名称
	unsigned char startup_date[20];		//启用日期
};

//税盘某票种监控信息
struct _monitor_info
{
	unsigned short state;		//结构体状态
	unsigned char kpjzsj[19];	//开票截止日期
	unsigned char bsqsrq[19];	//报送起始日期
	unsigned char bszzrq[19];	//报送终止日期
	unsigned char zxbsrq[19];	//最新报税日期
	unsigned char dzkpxe[20];	//单张开票限额
	unsigned char lxzsljje[20];	//离线正数累积金额
	unsigned int lxsc;			//离线时长
	unsigned char lxzssyje[20];//离线正数剩余金额
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
	struct _invoice_coil invoice_coil[10];
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
	unsigned char fplxdm;						//发票类型代码内存序号固定 内存0 4专票  内存1=5机动车  内存2=6二手车   内存3=7普票  内存4=9货运   内存5=25卷票  内存6=26电票
	struct _monitor_info			monitor_info;			//税盘某票种监控信息
	struct _invoice_coils_info		invoice_coils_info;		//税盘某票种未使用发票卷信息
	struct _plate_support_slv		plate_support_slv;		//税盘全部授权税率信息
	struct _plate_support_slv		plate_support_noslv;	//税盘不含税税率信息
	struct _invoice_offline_info	invoice_offline_info;	//离线票信息
};

//税盘全部信息结构体
struct _plate_infos
{
	struct _plate_basic_info		plate_basic_info;		//税盘基本信息
	struct _plate_tax_info			plate_tax_info;			//税盘税务相关信息
	struct _invoice_type_infos		invoice_type_infos[DF_MAX_INV_TYPE+1];//最多支持7个票种 004专票 005机动车 006二手车 007普票 009货运 025卷票 026电票
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
	unsigned char mwq[110];	//密文区
	unsigned char fpqqlsh[55];//发票请求流水号
	unsigned char bz[500];
	int sfsylsh;//是否使用流水号
	int fpdzsyh;
	char *scfpsj;//要上传的发票数据,数据为zlib压缩数据
	unsigned char errinfo[500];
};


//发票作废结果
struct _cancel_invoice_result
{
	unsigned char fplxdm;	//发票类型
	unsigned char fpdm[20];	//发票代码
	unsigned char fphm[20];	//发票号码
	unsigned char zfsj[20]; //作废时间
	unsigned char jym[22];	//校验码
	unsigned char mwq[110];	//密文区
	unsigned char errinfo[500];
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


#endif