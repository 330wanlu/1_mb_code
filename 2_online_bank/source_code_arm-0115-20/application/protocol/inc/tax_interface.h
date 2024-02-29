#ifndef _tax_interface_main_h
#define _tax_interface_main_h

#include "opt.h"

#include "ty_usb.h"

#define tax_interface_out(s,arg...)  log_out("all.c",s,##arg) 

#ifdef _tax_interface_c
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg)
#endif	
#endif
#define MSC_DIR_D2H (0x80)
#define MSC_DIR_H2D (0x00)

#if defined(__GNUC__)
typedef long long ll;
typedef unsigned long long ull;
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8 char
#define MAKELL(num) num##LL
#define FMT_64 "ll"
#elif defined(_MSC_VER)
typedef __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num##i64
#define FMT_64 "I64"
#elif defined(__BORLANDC__)
typedef __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num##i64
#define FMT_64 "L"
#else
#error "unknown compiler"
#endif
#define PROTOCOL_GBK
typedef void *LPVOID;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

//typedef char int8;
typedef signed char sint8;
typedef unsigned char uint8;
typedef short int16;
typedef signed short sint16;
typedef unsigned short uint16;
//typedef int int32;
typedef signed int sint32;
//typedef unsigned int uint32;
typedef ll int64;
typedef ll sint64;
typedef ull uint64;

#define _BYTE uint8
#define _WORD uint16
#define _DWORD uint32
#define _QWORD uint64
#if !defined(_MSC_VER)
#define _LONGLONG __int128
#endif


#ifndef _WINDOWS_
typedef int8 BYTE;
typedef int16 WORD;
typedef int32 DWORD;
typedef int32 LONG;
// typedef int BOOL;  // uppercase BOOL is usually 4 bytes
#endif
typedef int64 QWORD;
//#ifndef __cplusplus
//typedef int bool; // we want to use bool in our C programs
//#endif

// Some convenience macros to make partial accesses nicer
// first unsigned macros:
#define LOBYTE(x) (*((_BYTE *)&(x)))    // low byte
#define LOWORD(x) (*((_WORD *)&(x)))    // low word
#define LODWORD(x) (*((_DWORD *)&(x)))  // low dword
#define HIBYTE(x) (*((_BYTE *)&(x) + 1))
#define HIWORD(x) (*((_WORD *)&(x) + 1))
#define HIDWORD(x) (*((_DWORD *)&(x) + 1))
#define BYTEn(x, n) (*((_BYTE *)&(x) + n))
#define WORDn(x, n) (*((_WORD *)&(x) + n))
#define BYTE1(x) BYTEn(x, 1)  // byte 1 (counting from 0)
#define BYTE2(x) BYTEn(x, 2)
#define BYTE3(x) BYTEn(x, 3)
#define BYTE4(x) BYTEn(x, 4)
#define BYTE5(x) BYTEn(x, 5)
#define BYTE6(x) BYTEn(x, 6)
#define BYTE7(x) BYTEn(x, 7)
#define BYTE8(x) BYTEn(x, 8)
#define BYTE9(x) BYTEn(x, 9)
#define BYTE10(x) BYTEn(x, 10)
#define BYTE11(x) BYTEn(x, 11)
#define BYTE12(x) BYTEn(x, 12)
#define BYTE13(x) BYTEn(x, 13)
#define BYTE14(x) BYTEn(x, 14)
#define BYTE15(x) BYTEn(x, 15)
#define WORD1(x) WORDn(x, 1)
#define WORD2(x) WORDn(x, 2)  // third word of the object, unsigned
#define WORD3(x) WORDn(x, 3)
#define WORD4(x) WORDn(x, 4)
#define WORD5(x) WORDn(x, 5)
#define WORD6(x) WORDn(x, 6)
#define WORD7(x) WORDn(x, 7)

// now signed macros (the same but with sign extension)
#define SLOBYTE(x) (*((int8 *)&(x)))
#define SLOWORD(x) (*((int16 *)&(x)))
#define SLODWORD(x) (*((int32 *)&(x)))
#define SHIBYTE(x) (*((int8 *)&(x) + 1))
#define SHIWORD(x) (*((int16 *)&(x) + 1))
#define SHIDWORD(x) (*((int32 *)&(x) + 1))
#define SBYTEn(x, n) (*((int8 *)&(x) + n))
#define SWORDn(x, n) (*((int16 *)&(x) + n))
#define SBYTE1(x) SBYTEn(x, 1)
#define SBYTE2(x) SBYTEn(x, 2)
#define SBYTE3(x) SBYTEn(x, 3)
#define SBYTE4(x) SBYTEn(x, 4)
#define SBYTE5(x) SBYTEn(x, 5)
#define SBYTE6(x) SBYTEn(x, 6)
#define SBYTE7(x) SBYTEn(x, 7)
#define SBYTE8(x) SBYTEn(x, 8)
#define SBYTE9(x) SBYTEn(x, 9)
#define SBYTE10(x) SBYTEn(x, 10)
#define SBYTE11(x) SBYTEn(x, 11)
#define SBYTE12(x) SBYTEn(x, 12)
#define SBYTE13(x) SBYTEn(x, 13)
#define SBYTE14(x) SBYTEn(x, 14)
#define SBYTE15(x) SBYTEn(x, 15)
#define SWORD1(x) SWORDn(x, 1)
#define SWORD2(x) SWORDn(x, 2)
#define SWORD3(x) SWORDn(x, 3)
#define SWORD4(x) SWORDn(x, 4)
#define SWORD5(x) SWORDn(x, 5)
#define SWORD6(x) SWORDn(x, 6)
#define SWORD7(x) SWORDn(x, 7)

//模拟盘命令字
#define DF_PC_USB_ORDER_CONNECT			0X01				//联机命令
#define DF_PC_USB_ORDER_GET_CARD_IFNFO	0X11				//读取卡信息


enum enMemoryFormat_local {
	MF_REMOVE_HEADZERO_LOCAL,         // 移除字符串前置0; 00000123->123,trimstart
	MF_TRANSLATE_SPACE2ZERO_LOCAL,    // array:0x20 -> 0x00
	MF_TRANSLATE_ZERO2SPACE_LOCAL,    // array:0x00 -> 0x20
	MF_TRANSLATE_ABSOLUTE_LOCAL,      //字符串格式金额取绝对值
	MF_TRANSLATE_RED_LOCAL,           //字符串格式金额取红票取负值
	MF_TRANSLATE_HEADTAILZERO_LOCAL,  //移除字符串首尾0，00123000->123,trim
	MF_TRANSLATE_TAILZERO_LOCAL,		//移除字符串尾0，0.123000->0.123,trim
	MF_TRANSLATE_SPACE2STRZERO_LOCAL  // array:0x20 -> '0'(0x30)
};

enum enumNisecOPTypeLocal { NOP_CLOSE_LOCAL, NOP_OPEN_LOCAL, NOP_OPEN_FULL_LOCAL };
struct USBSession_Local {
	libusb_context *context;
	libusb_device_handle *handle;
	uint8 endpoint_d2h;
	uint8 endpoint_h2d;
	char szLastDevError[256];       //上次错误编码，非线程安全，可能被冲掉; aisino、nisec、cntax、TA税局网络IO都支持,只有返回值错误时读取的错误描述才可能正确，否则有可能是上一次未清理错误描述,6字节错误标志+错误描述，错误标志[NET],[DEV],[USB]
	void *pTestData;                //当 _NODEVICE_TEST 宏启用时被赋值
};
typedef struct USBSession_Local *HUSB_LOCAL;

// A为aisino,N为nisec,C为chinatax-ueky;汉字在GBK编码中占2个字节，在UTF8编码中占3个字节
struct StaticDeviceInfoLocal {
	//====动态数据,尽量在使用时重新调API更新数据
	char szDeviceTime[24];  // ANC 设备时间 yyyymmddhhmmss 20191019084029 MB模拟盘yyyy-mm-dd hh：mm:ss
	uint8 abInvTemplate[64];          // N   当前发票模板,根据发票种类结果不同,第4字节为发票类型,7/15位普票强制12
	//====静态数据
	char szNicMAC[24];                // A   开票通信网卡MAC地址
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
	char szTaxAuthorityName[128];     // ANC 税务机关名称 swjgmc
	uint8 abSupportInvtype[16];       // ANC 支持的发票类型,第一个字节为个数，后面逐字节排列支持类型
	uint16 uICCardNo;                 // ANC 税盘分机号
	uint8 bTrainFPDiskType;           // AN  0正式盘，1=A江苏测试盘/N重庆测试盘，2=山西测试盘
	uint8 bDeviceType;                // ANC aisino/nisec/cntax
	uint8 bOperateInvType;            // N   当前IO生命周期，打开进行操作的发票通用类型
	uint8 bCryptBigIO;                // A   加密区域读取采用2k大小,大容量读取(主要是证书操作),小IO会加密
	uint32 nDriverVersionNum;         // A   底层硬件驱动数字版本，用于直接比对
	//====附加的额外设备信息，可有可无
	char szDeviceEffectDate[24];      // ANC 设备启用日期(登记时间) yyyymmdd
	char szPubServer[64];             // AN  公共服务平台配置, "" or 平台类型(2)+平台号码...
	uint32 nMaxOfflineHour;           // A   设备最大离线小时数
	uint8 bNatureOfTaxpayer;          // ANC 纳税人性质 0=代开税务机关 1=小规模纳税人 2=一般纳税人 3=转登记纳税人
	//====内部指针
	void *pX509Cert;                  // AN  用于开票签名的证书指针
	HUSB_LOCAL hUSBDevice;                  // ANC 设备的libusb句柄
	char szProxyStr[48];              // ANC 设备网络连接代理字符串，放在这里不用每次起连接再设置一遍代理
	uint8 bBreakAllIO;                // ANC 中断设备所有IO、循环操作，尽快恢复设备等待状态，置1后需尽快恢复初始
};
typedef struct StaticDeviceInfoLocal *HDEV_LOCAL;

#define UR_CertComNameMAXLEN 1000
#define UR_TaxNumberMAXLEN  50
struct stUSBReaderRet
{
	int nRet;          // nRet是返回状态码，0 纳税人名称和税号都没有 1 能读出纳税人名称 2 能读出纳税人税号 3 都能读出 ，负数<-10为主函数出错，<-100是模块函数出错,=-999是不支持的型号
	int nStrEncording; // 0 unicode; 1 utf8
	unsigned char bufCertComName[UR_CertComNameMAXLEN];
	char bufTaxNumber[UR_TaxNumberMAXLEN];
	unsigned char key[8];
};


#define DF_TAX_ERR_CODE_TER_SYSTEM_ERROR			-10301//终端系统错误
#define DF_TAX_ERR_CODE_COMMAND_UNSUPPORTED			-10302//命令字不支持
#define DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT		-10303//系统内存不足
#define DF_TAX_ERR_CODE_DATA_PARSING_FAILED			-10304//json解析失败
#define DF_TAX_ERR_CODE_EXCEED_KP_END_DATE			-10305//超过开票截止日期
#define DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT			-10306//超过单张发票开票限额
#define DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT		-10307//超过离线限额
#define DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE			-10308//不支持该发票类型
#define DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR		-10309//发票数据解析失败
#define DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED		-10310//授权税率不支持
#define DF_TAX_ERR_CODE_HAVE_NO_INVOICE				-10311//没有可用发票
#define DF_TAX_ERR_CODE_SYSTEM_TIME_ERROR			-10312//当前终端系统时间错误
#define DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE			-10313//发票数量过大
#define DF_TAX_ERR_CODE_PLATE_IN_USED				-10314//税盘正在被使用
#define DF_TAX_ERR_CODE_INV_DATA_DISACCORD			-10315//开票数据与税盘信息不一致
#define DF_TAX_ERR_CODE_PLATE_OPER_FAILED			-10316//税盘操作失败
#define DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR		-10317//证书口令错误
#define DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME	-10318//超过离线时长
#define DF_TAX_ERR_CODE_PLATE_NON_EXISTENT			-10319//税盘不在该终端上
#define DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE		-10320//当前要查询无发票
#define DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8		-10321//使用了非法字符或utf8编码
#define DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV			-10322//税号中不能使用IOZSV非法字符
#define DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST	-10323//发票流水号不存在
#define DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED	-10324//该发票流水号已经开具过了
#define DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR			-10325//连接数据库失败
#define DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING			-10326//当前流水号正在开票中
#define DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH			-10327//USB端口号未授权
#define DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR			-10328//USB句柄失效
#define DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_UPDATING			-10329//USB端口号对应的机械臂正在升级
#define DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_NO_POWERE	-10330//此端口是机械臂
#define ERR_DEV_PASSWD_ERR	-108		//设备密码错误

#define ROBOT_ARM_HUB_PID_OLD  		0x8091  //机械臂hub的pid
#define ROBOT_ARM_HUB_VID_OLD		0x1a86  //机械臂hub的vid

#define ROBOT_ARM_HUB_PID_NEW  		0x0201  //机械臂hub的pid
#define ROBOT_ARM_HUB_VID_NEW		0x1a40  //机械臂hub的vid

#define ROBOT_ARM_PID  		0x2107  //机械臂pid
#define ROBOT_ARM_VID		0x413d  //机械臂vid

enum enDeviceType_local {

	DEVICE_UNKNOW_LOCAL = 0,
	DEVICE_AISINO_LOCAL = 1,    //航信税盘
	DEVICE_NISEC_LOCAL = 2,    //百旺税盘
	DEVICE_AISINO_CNTAX_UNKNOW_LOCAL = -10,  //税务中心ChinaTax UKey AISINO 版本
	DEVICE_MENGBAI_LOCAL = 3,   //蒙柏模拟盘

	DEVICE_CNTAX_LOCAL = 101,   //税务ukey
	DEVICE_SKP_KEY_LOCAL = 201,	//百旺勾选key
	DEVICE_JSP_KEY_LOCAL = 202,	//航信勾选key
	DEVICE_MENGBAI2_LOCAL = 301,   //蒙柏服务商key
	DEVICE_NONGYE_LOCAL = 401,   //农业银行key wang
	DEVICE_ROBOT_ARM_LOCAL = 501 //机械臂
};

int NisecLogicIORawLocal(HUSB_LOCAL device, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData, int nTransferDataLen);
int GetNisecDeviceLastErrorCode(HUSB_LOCAL hUSB, char *szOutErrorCode);
//++++++++++++++++++++++++++Asino++++++++++++++++++++++++++++//
int function_common_get_basic_tax_info_local(char *busid, int usb_type, char *ca_name, char *ca_serial, char *sn, int *passwd_right, int *left_num);

#endif