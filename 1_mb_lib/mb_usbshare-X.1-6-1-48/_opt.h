#ifndef _TYPE_DEFINE_DEFINE
	#define _TYPE_DEFINE_DEFINE
	/*==============================�����Ĺ����ļ�========================================*/
	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>	
	#include <unistd.h>
    #include <limits.h>

	#include <mb_clib/mb_typedef.h>
	#include <mb_clib/_algorithm.h>
	#include <mb_clib/_base64.h>
	#include <mb_clib/_code_s.h>
	#include <mb_clib/_confl.h>
	#include <mb_clib/_Date.h>
	#include <mb_clib/_des.h>
	#include <mb_clib/_utf82gbk.h>
	#include <mb_clib/_zlib.h>
	#include <mb_clib/cJSON.h>
	#include <mb_clib/test_malloc.h>
	#include <mb_clib/_folder.h>
	#include <mb_clib/_file_pro.h>
	#include <mb_clib/_t_file_c.h>
	#include <mb_clib/_timer.h>
	#include <mb_clib/_log.h>
	#include <mb_clib/_dec_file.h>
	#include <mb_taxlib/interface_main.h>
	#include <mb_taxlib/common/common.h>
	#include <mb_taxlib/common/define.h>
	#include <mb_taxlib/common/function.h>

	/*================================����������==========================================*/
    #define DF_TY_PATH_MAX                  256                //·�����ֵ
    #define DF_TY_USB_BUSID                 32                 //BUSID��ֵ
    
	#define DF_TRAN_OPEN

	#define DF_MQ_QOS0		0
	#define DF_MQ_QOS1		1
	#define DF_MQ_QOS2		2
	#define DF_TY_USBIP_PORT					3241
	#define DF_TY_TRAN_PORT						3240
	#define DF_TY_TRAN_OPEN						1

	#define DF_TY_SEV_ORDER_PORT				10001
	#define DF_TY_UDP_SER_PORT					10002	
	/*===============================����ʹ�û���========================================*/
	#define NEW_USB_SHARE
	#define TAX_SPECIAL				//��ֵ˰ר��
	//#define GENERAL_SERIES			//ͨ��ϵ��
	
	//#define TEST_KP_MEM				//���Կ�Ʊ�ڴ�

	//#define NEED_FP_DATA			//��Ʊ����д�ļ�

	/*===============================����ģʽ============================================*/
	//#define TRAN

	/*==========================�Ƿ���CPU�ڴ���=======================================*/
	//#define MONITOR_OEPN
	#define MONITOR_CLOSE
    
	/*===============================ϵͳ��Сʱ�䶨��=====================================*/
	#define SYSTEM_LEAST_TIME	"2021-05-25 00:00:00"
    
	/*===============================����ѡ��============================================*/
	//====DEBUG����====//
	//#define DEBUG					//alpha beta�汾���� DEBUG
	
	//====������������====//
	//#define RELEASE_TEST			//ʹ��������������������

	//====��ʽ�汾���������԰汾����====//
	#define RELEASE					//release�汾���� RELEASE
	
	//====�����汾����====//�����汾��VER_ALPHA ˫���汾��VER_BETA
	#define VER_ALPHA
	//#define VER_BETA
	//#define VER_GAMMA

	//===������Ͷ���===//

	//#define RELEASE_SO

	/*==============================���������Ͳ������===================================*/
	//#define CONTROL_OUT
	//#define INTERRUPT_OUT
	//#define BULK_OUT
	
    /*=================================���Ͷ���============================================*/	
#ifndef RELEASE_SO
	typedef unsigned char  										uint8;      
	typedef signed   char  										int8;        
	typedef unsigned short 										uint16;      
	typedef signed   short 										int16;        	
	typedef unsigned int   										uint32; 
	typedef unsigned long long   								uint64;
	typedef signed   int   										int32;       	
	typedef float          										fp32;        
	typedef double         										fp64; 
	typedef unsigned int 										uint;	
#endif	
	typedef struct
	{	char *name;						//ģ������
		char *r_date;					//��������
		char *ver;						//�����汾
		char *v_num;					//˳������汾��
		char *abdr;						//��ǰ�汾���ͣ�alpha beta debug release
		char *cpy;						//��˾
		char *writer;					//��д��
		char *note;						//��ע
		char *type;						//�ͺ�
		char *code;						//��������
	}_so_note;
	



#define CA_READ_ERR_NOT_FOUND_SO		-200



#ifdef MTK_OPENWRT
#define DF_MQTT_ERR_RED_OFF_GREEN_TWI		999
#define DF_MQTT_ERR_RED_OFF_GREEN_ON		888
#endif

#ifdef  NEW_USB_SHARE
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED		801
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND		802
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL		803
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START				804
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE				805
#define MQTT_GW_USBSHARE_OPERA_ERR			806
#define MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR			807
#endif	

	//����ͷ
#define DF_MQTT_SYNC_PROTOCOL_VER				"mb_fpkjxt_20190520"
#define DF_MQTT_SYNC_CODE_REPORT				"report"
#define DF_MQTT_SYNC_CODE_REQUEST				"request"
#define DF_MQTT_SYNC_CODE_RESPONSE				"response"

	//�����ϱ�������
#define DF_MQTT_SYNC_REPORT_TER_STATUS			"sbzdztxx"			//�ϱ��ն�״̬��Ϣ

	//������ѯ������
#define DF_MQTT_SYNC_REQUEST_KPRJ_ADDR			"cxkprjfwdz"		//��ѯ�ڿ�Ʊ�����������ַ

	//������ѯ������
	/*ͨ�ýӿ�*/
#define DF_MQTT_SYNC_REQUEST_CXZDQBXX			"cxzdqbxx"			//��ѯ�ն�ȫ����Ϣ
//#define DF_MQTT_SYNC_REQUEST_QQKQZFLL			"qqkqzfll"			//������ת����·
//#define DF_MQTT_SYNC_REQUEST_QQQXZFLL			"qqqxzfll"			//����ȡ��ת����·
#define DF_MQTT_SYNC_REQUEST_AYCXYKFPZS			"aycxykfpzs"		//���²�ѯ�ѿ���Ʊ����
#define DF_MQTT_SYNC_REQUEST_AYCXYKFPXX			"aycxykfpxx"		//���²�ѯ�ѿ���Ʊ��Ϣ
#define DF_MQTT_SYNC_REQUEST_AFPDMHMCXFP		"afpdmhmcxfp"		//����Ʊ��������ѯ��Ʊ
#define DF_MQTT_SYNC_REQUEST_AFPDMHMCXFPBS		"afpdmhmcxfpbs"		//����Ʊ��������ѯ��Ʊ�����Ƽ���Ʊ�ַ���
#define DF_MQTT_SYNC_REQUEST_TZYKFPCX			"tzykfpcx"			//ֹͣ�ѿ���Ʊ��ѯ
#define DF_MQTT_SYNC_REQUEST_GET_DQFPDMHM		"hqdqfpdmhm"		//ͨ����Ʊ�����ȡ��ǰ��Ʊ�������	
#define DF_MQTT_SYNC_REQUEST_CXZDJCXX			"cxzdjcxx"			//��ѯ�ն˻�����Ϣ
#define DF_MQTT_SYNC_REQUEST_OPEN_PORT			"openport"			//
#define DF_MQTT_SYNC_REQUEST_CLOSE_PORT			"closeport"			//

#define DF_MQTT_SYNC_REQUEST_TZZDSJSQ			"tzhqbbfbsqxx"		//֪ͨ�ն˻�ȡ��������Ȩ�ļ�
#define	DF_MQTT_SYNC_REQUEST_SQZDDK				"sqzddk"			//��Ȩ�ն˶˿�
#define DF_MQTT_SYNC_REQUEST_CXDLXX				"cxdlxx"			//��ѯ��������Ϣ
#define DF_MQTT_SYNC_REQUEST_CXLSCLJG			"cxlscljg"			//��ѯ��ʷ������

	/*��Ʊר�ýӿ�*/
#define DF_MQTT_SYNC_REQUEST_PLATE_TIME			"cxspdqsj"			//��ѯ˰�̵�ǰʱ��
#define DF_MQTT_SYNC_REQUEST_PLATE_TAX_INFO		"swxxcx"			//˰��˰����Ϣ��ѯ
#define DF_MQTT_SYNC_REQUEST_PLATE_INVOICE		"ykfpcx"			//�ѿ���Ʊ��ѯ
#define DF_MQTT_SYNC_REQUEST_PLATE_FPKJ			"fpkj"				//��Ʊ����
#define DF_MQTT_SYNC_REQUEST_PLATE_FPZF			"fpzf"				//��Ʊ����
#define DF_MQTT_SYNC_REQUEST_PLATE_CBKQ			"cbjkxxhc"			//���������Ϣ�ش�
#define DF_MQTT_SYNC_REQUEST_SET_REPORT_TIME	"szzdsbpl"			//�����ն��ϱ�Ƶ��
#define DF_MQTT_SYNC_REQUEST_CHECK_CERT_PASSWD	"mrzsmmyz"			//˰��Ĭ��֤��������֤
#define DF_MQTT_SYNC_REQUEST_GET_LSH_INV_DATA	"cxfpqqlsh"			//ͨ����Ʊ��ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ
#define DF_MQTT_SYNC_REQUEST_AFPDMHMSCFP		"afpdmhmscfp"		//����Ʊ��������ϴ���Ʊ
#define DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH			"hqhzxxbbh"			//��ȡרƱ���ַ�Ʊ֪ͨ�����
#define DF_MQTT_SYNC_REQUEST_CHECK_LZFPDMHM		"jylzfpdmhm"		//У�����ַ�Ʊ�������
#define DF_MQTT_SYNC_REQUEST_GET_INV_REDNUM		"hzxxbhqfp"			//ͨ��������Ϣ���ȡ��Ʊ
//#define DF_MQTT_SYNC_REQUEST_GET_NET_INV_COIL	"asjdlyfp"			//��ʱ������÷�Ʊ//ͣ��
#define DF_MQTT_SYNC_REQUEST_UPDATE_INV_MSERVER	"tzzdscmyfp"		//֪ͨ�ն˲�ѯ���ϴ�ĳ�·�Ʊ��M����
#define DF_MQTT_SYNC_REQUEST_CHANGE_CERT_PASSWD	"xgmrzsmmyz"		//�޸�Ĭ��֤�����
#define DF_MQTT_SYNC_REQUEST_CHANCEL_TZDBH		"hzxxbhcx"			//������Ϣ����
#define DF_MQTT_SYNC_REQUEST_QUERY_TZDBH_DATE	"asjdcxhzxxb"		//��ʱ��β�ѯ������Ϣ��
#define DF_MQTT_SYNC_REQUEST_QUREY_TER_LOG		"cxzddqrzxx"		//��ѯ�ն˵�ǰ��־��Ϣ
#define DF_MQTT_SYNC_REQUEST_OPEN_SSH_LINE		"kqzdywtd"			//�����ն���άͨ��
#define DF_MQTT_SYNC_REQUEST_CLOSE_SSH_LINE		"jszdywtd"			//�����ն���άͨ��
#define DF_MQTT_SYNC_REQUEST_GET_AUTH_FILE		"cxzddqsqxx"		//��ȡ�ж���Ȩ�ļ�
#define DF_MQTT_SYNC_REQUEST_GET_QUERY_INV_COIL	"asjdlyfpcx"		//��ʱ��β�ѯ��Ʊ����
#define DF_MQTT_SYNC_REQUEST_GET_DOWN_INV_COIL	"asjdlyfpxz"		//��ʱ������ط�Ʊ����
#define DF_MQTT_SYNC_REQUEST_CHECK_SERVER		"jcfwqlj"		    //���˰�������������
#define DF_MQTT_SYNC_REQUEST_MANAGE_PORT_POWER	"adkhgldkdy"		//����˿ڵ�Դ
#define DF_MQTT_SYNC_REQUEST_MANAGE_TER_POWER	"glzddy"			//�����ն˵�Դ
#define DF_MQTT_SYNC_REQUEST_CLEAR_CERT_ERR_ROM	"asphqczshc"		//��˰�̺����֤��������
#define DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH_ALL		"afpdmhmhqhzxxb"	//����Ʊ�������������Ʊ������Ϣ��
#define DF_MQTT_SYNC_REQUEST_GET_AUTH_KEY		"hqkeysqxx"			//��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ
#define DF_MQTT_SYNC_REQUEST_CONECT_PUBSERVER	"sqjrggfwpt"		//���빫������ƽ̨
#define DF_MQTT_SYNC_REQUEST_DISCONECT_PUBSERVER "qxjrggfwpt"		//ȡ�����빫������ƽ̨
#define DF_MQTT_SYNC_REQUEST_GET_STATISTICS		"hqhzxxsj"			//��ȡ˰�̻�����Ϣ����
#define DF_MQTT_SYNC_REQUEST_GET_HISTORY_COILS	"hqlsgpxx"			//��ȡ��ʷ��Ʊ��Ϣ
#define DF_MQTT_SYNC_REQUEST_CHANGE_PORT_SAVE	"qhdksdms"			//�л��˿�ʡ��ģʽ
#define DF_MQTT_SYNC_REQUEST_TER_POWER_MODE_CHANGE	"qhzjsdms"		//�л�����ʡ��ģʽ
#define DF_MQTT_SYNC_REQUEST_TCGXRZFWSJ			"tcgxrzfwsj"		//͸����ѡ��֤��������
#define DF_MQTT_SYNC_REQUSET_KZLXFPSC			"kzlxfpsc"			//�������߷�Ʊ�ϴ�
#define DF_MQTT_SYNC_REQUSET_TZZDDKMQTT			"tzzddkmqtt"		//֪ͨ�ն˶Ͽ�mqtt

#define DF_MQTT_SYNC_REQUSET_QDDLXX	             "qddlxx"			//ȫ���¼  	   0304
#define DF_MQTT_SYNC_REQUSET_QDDLLB				 "qyrylb"			//ȫ���û���Ϣ��ȡ	0423
#define DF_MQTT_SYNC_REQUSET_QDFPCX				 "qdfpcx"		    //ȫ�緢Ʊ��ѯ	    0515

#define DF_MQTT_SYNC_REQUSET_QDFPCXXQ             "qdfpcxxq"		//ȫ�緢Ʊ�����ѯ	
#define DF_MQTT_SYNC_REQUSET_QDFPXZ               "qdfpxz"			//ȫ�緢Ʊ����	
#define DF_MQTT_SYNC_REQUSET_FPKJRPA              "fpkjrpa"			//����������ȫ��APPɨ����ɨ������

#define DF_MQTT_SYNC_REQUSET_FPKJRPAZT			  "fpkjrpastate"	//����������ȫ��APPɨ����ɨ������ ��״̬��ѯ��

#define DF_MQTT_SYNC_REQUSET_QDQYLB				 "qdqylb"		    //ȫ����ҵ�б��ѯ	0526
#define DF_MQTT_SYNC_REQUSET_QDQYQH				 "qdqyqh"		    //ȫ����ҵ�л�	    0526

#define DF_MQTT_SYNC_REQUSET_QDSXED              "qdsxed"			//ȫ�����Ŷ�Ȳ�ѯ

#define DF_MQTT_SYNC_REQUSET_QDLZFPKJ		      "qdfpkj"			//ȫ�����ַ�Ʊ����

//ȫ���Ʊ���
#define DF_MQTT_SYNC_REQUSET_HZQRXXSZZHQUERY  	 "hzqrxxszzhquery"	 //ȫ��ɿ���Ʊ����Ʊ��ѯ
#define DF_MQTT_SYNC_REQUSET_HZQRXXSAVE  	 	 "hzqrxxsave"	 	 //ȫ���Ʊȷ�ϵ�����
#define DF_MQTT_SYNC_REQUSET_HZQRXXQUERY  	 	 "hzqrxxquery"	 	 //ȫ���Ʊȷ�ϵ��б��ѯ
#define DF_MQTT_SYNC_REQUSET_HZQRXXQUERYDETAIL   "hzqrxxquerydetail" //ȫ���Ʊȷ�ϵ������ѯ

#define DF_MQTT_SYNC_REQUSET_HCSTAQUERY           "hcstaquery"		//ȫ�緢Ʊ�������ѯ

#define DF_MQTT_SYNC_REQUSET_FJXXPEIZHI			  "fjxxpeizhi"		//������Ϣ����(�������޸ġ�ɾ��)
#define DF_MQTT_SYNC_REQUSET_CJMBPEIZHI			  "cjmbpeizhi"		//����ģ������(�������޸ġ�ɾ��)

//�����Ľӿ�
#define DF_MQTT_SYNC_REQUSET_QDFPCX_BY_COOKIES				 "qdfpcxbycookies"		    //ȫ�緢Ʊ��ѯ����cookies
#define DF_MQTT_SYNC_REQUSET_QDFPCXDETAIL_BY_COOKIES		 "qdfpcxdetailbycookies"		    //ȫ�緢Ʊ�����ѯ����cookies

#define DF_MQTT_SYNC_REQUSET_QD_GET_COOKIES		 "qdgetcookies"		    //ȫ���ȡcookies

#define DF_MQTT_SYNC_REQUSET_QD_NSRJCXX		 "nsrjcxx"		    //��˰�˻�����Ϣ
#define DF_MQTT_SYNC_REQUSET_QD_NSRFXXX		 "nsrfxxx"		    //��˰�˷�����Ϣ


#define DF_MQTT_SYNC_REQUSET_GET_CLIENTHELLO_BY_PLATE	"mqttgetclienthello"	//mqtt��ȡclienthello
#define DF_MQTT_SYNC_REQUSET_GET_CLIENTAUTH_BY_PLATE	"mqttgetclientauth"		//mqtt��ȡclientauth
#define DF_MQTT_SYNC_REQUSET_READCERT_BY_PLATE			"mqttReadCert"	//��ȡ֤��

#define DF_MQTT_DEVOPS_REQUEST_SSH_CMD			"sshcmd"			//sshָ��
#define DF_MQTT_DEVOPS_REQUEST_FILE_UPLOAD		"uploadfile"		//�ϴ��ļ�
#define DF_MQTT_DEVOPS_REQUEST_FILE_DOWNLOAD	"downloadfile"		//�����ļ�


#define ISspace(x) isspace((int)(x))  
#define DF_ORDER_GET_VERSION							"/api/getVersion"
#define DF_ORDER_READ_CERTINFO							"/api/readCertInfo"
#define DF_ORDER_VERIFRY_PIN							"/api/verifyPin"
#define DF_ORDER_CLIENT_HELLO							"/api/clientHello"
#define DF_ORDER_CLIENT_AUTH							"/api/clientAuth"
#define DF_ORDER_CHECK_KEY								"/api/checkKey"
#define DF_ORDER_GET_DEVICE_NUM							"/api/getDeviceNum"
#define DF_ORDER_READ_CERT								"/api/readCert"

#define DF_LINUX_NET_DIR		"/proc/net"
#define DF_LINUX_DNS			"/etc/resolv.conf"
#define DF_LINUX_DNS_TMP		"/tmp/resolv.conf"
#define DF_TER_INFO_FILE		"/etc/ter.info"
#define DF_TER_ID_INI			"/etc/terid.ini"
#define DF_TER_MAC_ADDR_FILE	"/etc/init.d/mac_address"
	struct _new_usbshare_parm
	{
		char protocol[51];
		char code_type[11];
		char cmd[51];
		char result[2];
		char source_topic[51];
		char random[129];
		char trans_server[100];//ת����·IP
		int trans_port;//ת����·�˿ں�
		int postion;//��ǵ�ǰ�ڴ��±�(���ڹر���·ʹ��)
		int state;//��Ƭ�ڴ�ʹ�ñ�ʶ  0 δʹ��
		int data_lock;//���ṹ��������
		int port;		//��������ת����·���ض˿ڼ�¼�˿ں�
		uint8 mechine_id[13];
		char client_id[50];//�û�Ψһ��ʶ�� ������ת����������ʶƥ��Ψһ��·
		int handle;//��¼ת����·���
		int type;//ת����·������; 1:���� 2:����
	};

	struct mqtt_parm
	{
		char protocol[51];
		char code_type[11];
		char cmd[51];
		char result[2];
		char source_topic[51];
		char random[129];

	};

//�������޸�
//.0.6.26   //���汾���Ƿ�����
//.0.5.26	//���汾���Ƿ�����
//-6-1-16	//���������Ƿ�����
//-5-26-1 //���汾����������Ƿ�����
//-6-1-16	//�����ĸ��汾����include��lib�ļ�
#define DF_TER_VERSION_NUM				"48"				//�������
#define DF_TER_VERSION_TIME				"2024-01-12"	    //���򷢲�ʱ��
//�������޸�

#ifndef MTK_OPENWRT
	#define MAX_MESSAGE_QUEUE_NUM 400	//�����Ϣ�����������		
	#define MAX_DEAL_MESSAGE_TASK 80	//�����Ϣ�߳�����		
#else
	#define MAX_MESSAGE_QUEUE_NUM 400	//�����Ϣ�����������		
	#define MAX_DEAL_MESSAGE_TASK 20	//�����Ϣ�߳�����		
#endif		
#define MAX_HISTROY_DEAL_RESULT 100		//��Ϣ��ʷ������
#define DEAFULT_CONNECT_TIME 	60		//MQTTĬ�������ʱ��


//wang 60�ڶ���
#ifdef DF_2306_1
#define DF_TY_USB_PORT_N                60				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		60				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2306"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402306CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.3"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				11				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				20				//20
#define	DF_FULL_POWER_NUM				30				//30
#endif


#ifdef DF_1902_1
#define DF_TY_USB_PORT_N                2				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		2				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1902"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401902CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.3"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				5				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1  MTK����Ϊ2
#define DF_FULL_LOAD_NUM				2
#define	DF_FULL_POWER_NUM				2
#endif

#ifdef DF_1905_1
#define DF_TY_USB_PORT_N                4				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1905"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401905CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				7				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_MB_HARDWARE_A20_A33			2				//A20����Ϊ0��A33����Ϊ1  MTK����Ϊ2
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

#ifdef DF_1906_1
#define DF_TY_USB_PORT_N                32				//���֧��32���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM		32				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1906"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401906CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.2"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				0				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				32
#define	DF_FULL_POWER_NUM				32
#endif

#ifdef DF_1908_1
#define DF_TY_USB_PORT_N                68				//���֧��68���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM		68				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1908"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401908CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				6				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				68
#define	DF_FULL_POWER_NUM				68
#endif

#ifdef DF_1910_1
#define DF_TY_USB_PORT_N                100				//���֧��100���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		100				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1910"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401910CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.1"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				4				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				100
#define	DF_FULL_POWER_NUM				100
#endif

#ifdef DF_2106_1
#define DF_TY_USB_PORT_N                32				//���֧��32���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		32				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2106"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402106CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.2"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				0				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				32
#define	DF_FULL_POWER_NUM				32
#endif

#ifdef DF_2108_1
#define DF_TY_USB_PORT_N                68				//���֧��68���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM		68				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2108"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402108CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				6				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif
	
#ifdef DF_2110_1
#define DF_TY_USB_PORT_N                100				//���֧��100���˿�   
#define DF_TCP_SER_CONNECT_LINE_NUM		100				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2110"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402110CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				4				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif

#ifdef DF_2202_1
#define DF_TY_USB_PORT_N                20				//���֧��20���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		20				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2202"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402202CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				8				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				20
#endif

#ifdef DF_2204_1
#define DF_TY_USB_PORT_N                4				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2204"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402204CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				7				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

#ifdef DF_2212_1
#define DF_TY_USB_PORT_N                120				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		120				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2212"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402212CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				9				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				15
#define	DF_FULL_POWER_NUM				25
#endif

#ifdef DF_2216_1
#define DF_TY_USB_PORT_N                240				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		240				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_2216"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2402216CX"	//��������
#define DF_TER_VERSION_NAME				"1.1.6.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				10				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				20
#define	DF_FULL_POWER_NUM				30
#endif

#ifdef DF_1201_1
#define DF_TY_USB_PORT_N                4				//���֧��4���˿�    
#define DF_TCP_SER_CONNECT_LINE_NUM		4				//TCP������·����
#define DF_TER_TYPE_NAME				"MB_1201"		//�ͺ�����
#define DF_TER_TYPE_EXE					"MB2401201CX"	//��������
#define DF_TER_VERSION_NAME				"1.2.0.1"		//����汾
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"		//mb1602ΪY1.0.1 mb1603A��mb1603BλY1.0.2 mb1603CΪ3.0.1 mb1602aΪ3.0.2 mb1805Ϊ3.03 mb1806Ϊ3.04 mb1905Ϊ3.04
#define DF_TY_MACHINE_TYPE				7				//��0��32�ڻ���  ��1��60�ڻ���,100�ڻ���(����ṹ) ��3��8��(δ����)  ��4��100�ڻ��ͣ�һ���ṹ ��5��2��  ��6��68��  ��7��4��  ��8��20��    ��9��120��
#define DF_MB_HARDWARE_A20_A33			3				//A20����Ϊ0��A33����Ϊ1	MTK����Ϊ2  A33��wifi����Ϊ3
#define DF_FULL_LOAD_NUM				4
#define	DF_FULL_POWER_NUM				4
#endif

	//typedef enum {

	//	false_mb, true_mb
	//} bool;

	//typedef enum {

	//	FALSE_MB, TRUE_MB
	//} BOOL;

	/*=================================����������=========================================*/
	
	

	
	#define DF_CODE_GB18030						0
	#define DF_CODE_UTF8						1
	
	#define DF_CODE_SELF						DF_CODE_GB18030


#endif	



























