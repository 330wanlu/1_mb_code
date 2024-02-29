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

//ģ����������
#define DF_PC_USB_ORDER_CONNECT			0X01				//��������
#define DF_PC_USB_ORDER_GET_CARD_IFNFO	0X11				//��ȡ����Ϣ


enum enMemoryFormat_local {
	MF_REMOVE_HEADZERO_LOCAL,         // �Ƴ��ַ���ǰ��0; 00000123->123,trimstart
	MF_TRANSLATE_SPACE2ZERO_LOCAL,    // array:0x20 -> 0x00
	MF_TRANSLATE_ZERO2SPACE_LOCAL,    // array:0x00 -> 0x20
	MF_TRANSLATE_ABSOLUTE_LOCAL,      //�ַ�����ʽ���ȡ����ֵ
	MF_TRANSLATE_RED_LOCAL,           //�ַ�����ʽ���ȡ��Ʊȡ��ֵ
	MF_TRANSLATE_HEADTAILZERO_LOCAL,  //�Ƴ��ַ�����β0��00123000->123,trim
	MF_TRANSLATE_TAILZERO_LOCAL,		//�Ƴ��ַ���β0��0.123000->0.123,trim
	MF_TRANSLATE_SPACE2STRZERO_LOCAL  // array:0x20 -> '0'(0x30)
};

enum enumNisecOPTypeLocal { NOP_CLOSE_LOCAL, NOP_OPEN_LOCAL, NOP_OPEN_FULL_LOCAL };
struct USBSession_Local {
	libusb_context *context;
	libusb_device_handle *handle;
	uint8 endpoint_d2h;
	uint8 endpoint_h2d;
	char szLastDevError[256];       //�ϴδ�����룬���̰߳�ȫ�����ܱ����; aisino��nisec��cntax��TA˰������IO��֧��,ֻ�з���ֵ����ʱ��ȡ�Ĵ��������ſ�����ȷ�������п�������һ��δ�����������,6�ֽڴ����־+���������������־[NET],[DEV],[USB]
	void *pTestData;                //�� _NODEVICE_TEST ������ʱ����ֵ
};
typedef struct USBSession_Local *HUSB_LOCAL;

// AΪaisino,NΪnisec,CΪchinatax-ueky;������GBK������ռ2���ֽڣ���UTF8������ռ3���ֽ�
struct StaticDeviceInfoLocal {
	//====��̬����,������ʹ��ʱ���µ�API��������
	char szDeviceTime[24];  // ANC �豸ʱ�� yyyymmddhhmmss 20191019084029 MBģ����yyyy-mm-dd hh��mm:ss
	uint8 abInvTemplate[64];          // N   ��ǰ��Ʊģ��,���ݷ�Ʊ��������ͬ,��4�ֽ�Ϊ��Ʊ����,7/15λ��Ʊǿ��12
	//====��̬����
	char szNicMAC[24];                // A   ��Ʊͨ������MAC��ַ
	char szDeviceID[24];              // ANC �豸���SN
	char szDriverVersion[32];         // ANC �ײ�Ӳ�������汾��
	char szJMBBH[4];                  // A   ���ܰ汾��
	char szCommonTaxID[24];           // ANC ͨ����ʾ˰��,˰��������˾˰��=����˰�� ex:92320105MA1WMUAP8T
	char szCompressTaxID[24];         // A   ѹ��˰��/����˰��? ex:320100344089314
	char sz9ByteHashTaxID[24];        // A   9λhash˰��/8byte+1 ex:32010034408931401
	char szRegCode[8];                // AN  �������6�ֽ�,aisino��Ʊʹ�ã���Ʊ�ϴ��͹�������ƽ̨ʱ���ݸ�ֵѡ���Ӧ������
	char szCompanyName[256];          // ANC ˰��������˾����,���50������==��������    
	char szTaxAuthorityCode[16];      // NC  ˰����ش���(�ٷ���������)�淶��ʽ��11λ������˰����9λ,��ֵ�������û����� www.chinatax.gov.cn/n810341/n810755/c3665232/content.html    
	char szTaxAuthorityCodeEx[16];    // ANC ˰����ش�����չ(�ڲ�ʹ��)����׼12λ(Nisec:11λʱ��0��9λʱ��200,Cntax:����12λʱ�Ҳⲹ�ո�,aisino:����12λʱ�Ҳⲹ�ո�)
	char szTaxAuthorityName[128];     // ANC ˰��������� swjgmc
	uint8 abSupportInvtype[16];       // ANC ֧�ֵķ�Ʊ����,��һ���ֽ�Ϊ�������������ֽ�����֧������
	uint16 uICCardNo;                 // ANC ˰�̷ֻ���
	uint8 bTrainFPDiskType;           // AN  0��ʽ�̣�1=A���ղ�����/N��������̣�2=ɽ��������
	uint8 bDeviceType;                // ANC aisino/nisec/cntax
	uint8 bOperateInvType;            // N   ��ǰIO�������ڣ��򿪽��в����ķ�Ʊͨ������
	uint8 bCryptBigIO;                // A   ���������ȡ����2k��С,��������ȡ(��Ҫ��֤�����),СIO�����
	uint32 nDriverVersionNum;         // A   �ײ�Ӳ���������ְ汾������ֱ�ӱȶ�
	//====���ӵĶ����豸��Ϣ�����п���
	char szDeviceEffectDate[24];      // ANC �豸��������(�Ǽ�ʱ��) yyyymmdd
	char szPubServer[64];             // AN  ��������ƽ̨����, "" or ƽ̨����(2)+ƽ̨����...
	uint32 nMaxOfflineHour;           // A   �豸�������Сʱ��
	uint8 bNatureOfTaxpayer;          // ANC ��˰������ 0=����˰����� 1=С��ģ��˰�� 2=һ����˰�� 3=ת�Ǽ���˰��
	//====�ڲ�ָ��
	void *pX509Cert;                  // AN  ���ڿ�Ʊǩ����֤��ָ��
	HUSB_LOCAL hUSBDevice;                  // ANC �豸��libusb���
	char szProxyStr[48];              // ANC �豸�������Ӵ����ַ������������ﲻ��ÿ��������������һ�����
	uint8 bBreakAllIO;                // ANC �ж��豸����IO��ѭ������������ָ��豸�ȴ�״̬����1���辡��ָ���ʼ
};
typedef struct StaticDeviceInfoLocal *HDEV_LOCAL;

#define UR_CertComNameMAXLEN 1000
#define UR_TaxNumberMAXLEN  50
struct stUSBReaderRet
{
	int nRet;          // nRet�Ƿ���״̬�룬0 ��˰�����ƺ�˰�Ŷ�û�� 1 �ܶ�����˰������ 2 �ܶ�����˰��˰�� 3 ���ܶ��� ������<-10Ϊ����������<-100��ģ�麯������,=-999�ǲ�֧�ֵ��ͺ�
	int nStrEncording; // 0 unicode; 1 utf8
	unsigned char bufCertComName[UR_CertComNameMAXLEN];
	char bufTaxNumber[UR_TaxNumberMAXLEN];
	unsigned char key[8];
};


#define DF_TAX_ERR_CODE_TER_SYSTEM_ERROR			-10301//�ն�ϵͳ����
#define DF_TAX_ERR_CODE_COMMAND_UNSUPPORTED			-10302//�����ֲ�֧��
#define DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT		-10303//ϵͳ�ڴ治��
#define DF_TAX_ERR_CODE_DATA_PARSING_FAILED			-10304//json����ʧ��
#define DF_TAX_ERR_CODE_EXCEED_KP_END_DATE			-10305//������Ʊ��ֹ����
#define DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT			-10306//�������ŷ�Ʊ��Ʊ�޶�
#define DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT		-10307//���������޶�
#define DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE			-10308//��֧�ָ÷�Ʊ����
#define DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR		-10309//��Ʊ���ݽ���ʧ��
#define DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED		-10310//��Ȩ˰�ʲ�֧��
#define DF_TAX_ERR_CODE_HAVE_NO_INVOICE				-10311//û�п��÷�Ʊ
#define DF_TAX_ERR_CODE_SYSTEM_TIME_ERROR			-10312//��ǰ�ն�ϵͳʱ�����
#define DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE			-10313//��Ʊ��������
#define DF_TAX_ERR_CODE_PLATE_IN_USED				-10314//˰�����ڱ�ʹ��
#define DF_TAX_ERR_CODE_INV_DATA_DISACCORD			-10315//��Ʊ������˰����Ϣ��һ��
#define DF_TAX_ERR_CODE_PLATE_OPER_FAILED			-10316//˰�̲���ʧ��
#define DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR		-10317//֤��������
#define DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME	-10318//��������ʱ��
#define DF_TAX_ERR_CODE_PLATE_NON_EXISTENT			-10319//˰�̲��ڸ��ն���
#define DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE		-10320//��ǰҪ��ѯ�޷�Ʊ
#define DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8		-10321//ʹ���˷Ƿ��ַ���utf8����
#define DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV			-10322//˰���в���ʹ��IOZSV�Ƿ��ַ�
#define DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST	-10323//��Ʊ��ˮ�Ų�����
#define DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED	-10324//�÷�Ʊ��ˮ���Ѿ����߹���
#define DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR			-10325//�������ݿ�ʧ��
#define DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING			-10326//��ǰ��ˮ�����ڿ�Ʊ��
#define DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH			-10327//USB�˿ں�δ��Ȩ
#define DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR			-10328//USB���ʧЧ
#define DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_UPDATING			-10329//USB�˿ںŶ�Ӧ�Ļ�е����������
#define DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_NO_POWERE	-10330//�˶˿��ǻ�е��
#define ERR_DEV_PASSWD_ERR	-108		//�豸�������

#define ROBOT_ARM_HUB_PID_OLD  		0x8091  //��е��hub��pid
#define ROBOT_ARM_HUB_VID_OLD		0x1a86  //��е��hub��vid

#define ROBOT_ARM_HUB_PID_NEW  		0x0201  //��е��hub��pid
#define ROBOT_ARM_HUB_VID_NEW		0x1a40  //��е��hub��vid

#define ROBOT_ARM_PID  		0x2107  //��е��pid
#define ROBOT_ARM_VID		0x413d  //��е��vid

enum enDeviceType_local {

	DEVICE_UNKNOW_LOCAL = 0,
	DEVICE_AISINO_LOCAL = 1,    //����˰��
	DEVICE_NISEC_LOCAL = 2,    //����˰��
	DEVICE_AISINO_CNTAX_UNKNOW_LOCAL = -10,  //˰������ChinaTax UKey AISINO �汾
	DEVICE_MENGBAI_LOCAL = 3,   //�ɰ�ģ����

	DEVICE_CNTAX_LOCAL = 101,   //˰��ukey
	DEVICE_SKP_KEY_LOCAL = 201,	//������ѡkey
	DEVICE_JSP_KEY_LOCAL = 202,	//���Ź�ѡkey
	DEVICE_MENGBAI2_LOCAL = 301,   //�ɰط�����key
	DEVICE_NONGYE_LOCAL = 401,   //ũҵ����key wang
	DEVICE_ROBOT_ARM_LOCAL = 501 //��е��
};

int NisecLogicIORawLocal(HUSB_LOCAL device, uint8 *pInOutCmd, int nInCmdLen, int nInCmdBufLen, uint8 *pTransferData, int nTransferDataLen);
int GetNisecDeviceLastErrorCode(HUSB_LOCAL hUSB, char *szOutErrorCode);
//++++++++++++++++++++++++++Asino++++++++++++++++++++++++++++//
int function_common_get_basic_tax_info_local(char *busid, int usb_type, char *ca_name, char *ca_serial, char *sn, int *passwd_right, int *left_num);

#endif