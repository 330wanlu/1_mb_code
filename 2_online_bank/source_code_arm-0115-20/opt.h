#ifndef OPT_H
#define OPT_H

#include "ol_bank_include.h"

#define RELEASE_SO

#define DF_TY_PATH_MAX                  256                //·�����ֵ
#define DF_TY_USB_BUSID                 32                 //BUSID��ֵ

#define DF_TRAN_OPEN

#define DF_MQ_QOS0		0
#define DF_MQ_QOS1		1
#define DF_MQ_QOS2		2
#define DF_TY_TRAN_PORT						3240
#define DF_TY_TRAN_OPEN						1

#define DF_TY_SEV_ORDER_PORT				10001
#define DF_TY_UDP_SER_PORT					10002	
/*===============================����ʹ�û���========================================*/
#define NEW_USB_SHARE
#define TAX_SPECIAL				//��ֵ˰ר��

#define MONITOR_CLOSE

/*===============================ϵͳ��Сʱ�䶨��=====================================*/
#define SYSTEM_LEAST_TIME	"2021-05-25 00:00:00"

#define RELEASE					//release�汾���� RELEASE

//====�����汾����====//�����汾��VER_ALPHA ˫���汾��VER_BETA
#define VER_ALPHA

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

typedef signed char sint8;
typedef unsigned char uint8;
typedef short int16;
typedef signed short sint16;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed int sint32;

typedef void* (*Funaction)(void* args);

typedef int (*Fun_get_ter_all_data_json)(cJSON *data_layer);

#define PORT_START_NUM 	1	 //��ʼ�˿ں�
#define MAX_PORT_NUM 	DF_TY_USB_PORT_N   //���˿���

#ifdef MCHTYPE_2303
#define MAX_ONLINE_NUM 60  //������߶˿���
#else
#define MAX_ONLINE_NUM 20  //������߶˿���
#endif

/*---------------------------------ԭ��------------------------------------------*/
#define DF_EVENT_REASON_NO_HEART			1                               //��λʱ����û������
#define DF_EVENT_REASON_NET					100                             //��������
#define DF_EVENT_REASON_MAIN_HUB			(DF_EVENT_REASON_NET+1)         //��HUBû�з��� 
#define DF_EVENT_REASON_SET_MA				(DF_EVENT_REASON_MAIN_HUB+1)    //���û������
#define DF_EVENT_REASON_SOFT				(DF_EVENT_REASON_SET_MA+1)      //�������   
#define DF_EVENT_REASON_SET_BUSID			(DF_EVENT_REASON_SOFT+1)        //����busid
#define DF_EVENT_REASON_FILE_USB			(DF_EVENT_REASON_SET_BUSID+1)   //�豸�Ѿ��ر�,���ļ�ϵͳ������   
#define	DF_EVENT_REASON_CLOSE_SOCKET_FAILED	(DF_EVENT_REASON_FILE_USB+1)
#define DF_EVENT_REASON_CHECK_SYS_ERR		(DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)
#define DF_EVENT_REASON_GET_M3_FLASH_ERROR  (DF_EVENT_REASON_CLOSE_SOCKET_FAILED+1)


#define MAX_MESSAGE_QUEUE_NUM 400	//�����Ϣ�����������
#define MAX_DEAL_MESSAGE_TASK 80	//�����Ϣ�߳�����

#define MAX_HISTROY_DEAL_RESULT 100	//��Ϣ��ʷ������
#define DEAFULT_CONNECT_TIME 60		//MQTTĬ�������ʱ��

/*--------------------------------�¼�����----------------------------------------*/
#define DF_EVENT_NAME_RESET             1                               //��������
#define DF_EVENT_NAME_POWER_OFF         (DF_EVENT_NAME_RESET+1)         //�رյ�Դ   
#define DF_EVENT_NAME_IMPORT_RECORD		(DF_EVENT_NAME_POWER_OFF+1)		//��Ҫ��¼

enum RESPONSE_CODE
{
	NO_ERROR = 0,
	PARAMETER_ERROR,
	POWER_LOWER_PERMISSION_ERROR
};

enum MACHINE_TYPE
{
	RESVER = 0,
	TYPE_2303 = 1,
	TYPE_2110,
	TYPE_2108,
	TYPE_2202,
	TYPE_2306,
	TYPE_2212
};

typedef struct
{
	char *name;						//ģ������
	char *r_date;					//��������
	char *ver;						//�����汾
	char *v_num;					//˳������汾��
	char *abdr;						//��ǰ�汾���ͣ�alpha beta debug release
	char *cpy;						//��˾
	char *writer;					//��д��
	char *note;						//��ע
	char *type;						//�ͺ�
	char *code;						//��������
	int machine_type;				//��������
}_so_note;

#define CA_READ_ERR_NOT_FOUND_SO		-200

#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED		801
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND		802
#define MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL		803
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START				804
#define MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE				805
#define MQTT_GW_USBSHARE_OPERA_ERR			806
#define MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR			807
#define MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_WITHOUT_DEV		808
#define MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_NO_SATRT			809
#define MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED			810
#define MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING			811
#define MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE			812
#define MQTT_GW_USBSHARE_ROBOTARM_NOT_WORK			813

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
#define DF_MQTT_SYNC_REQUEST_START_ROBOT_ARM	"start_robot_arm"	//����һ�λ�е��
#define DF_MQTT_SYNC_REQUEST_UPDATE_ROBOT_ARM   "update_robot_arm"  //������е��

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
	char random[512];

};

struct _upload_inv_num
{
	int state;
	int in_used;			//0δʹ��  1��ʹ��
	char invoice_month[10];	//�·�
	char summary_month[10];	//���������ϴ��·�
	char *dmhmzfsbstr;//�������з�Ʊ����
	int just_single_month;		//��ͬ��һ���·�Ʊ
};

typedef struct MODULE_T
{
	int switch_fd;
	int ty_usb_fd;
	int ty_usb_m_fd;
	int ty_pd_fd;
	int deploy_fd;
	int net_time_fd;
	int event_file_fd;
	int machine_fd;
	int serial_fd;
	int get_net_fd;
	int lcd_fd;
	int mq_sync_fd;
	int m_task_fd;
	int vpnfd;
	int opsfd;
	int ble_fd;
	int device_manage_fd;//add by whl
	int task;
	int mqtt_lock;
	int sql_lock;
	int inv_read_lock;
	int data_lock;
	int uart_ok;
	int sock_fd;
	int udp_fd;
	int tran_fd;
	int update_file;
	char mqtt_server[200];
	int mqtt_port;
	char user_name[20];
	char user_passwd[50];
	char topic[200];
	int mqtt_reg_flag;
	int connect_time;
	int mqtt_state;		//0��δ��¼					1���Ѿ���¼
	int boot_update;
	char address[200];
	int port;
	char topic_gw[100];
	char user_name_gw[20];
	char user_passwd_gw[20];
	char user_id_sub_gw[100];
	char user_id_pub_gw[100];
	int log_enable;
	char log_path[200];
	struct mqtt_parm parm[100];
	char ter_id[13];
}MODULE;

#define DF_EVENT_FILE_NAME                  "event"
#define DF_TY_PD_NAME                       "ty_pd"

#define DF_TY_PD_CM_RESET                   100
#define DF_TY_PC_CM_POWER_OFF               (DF_TY_PD_CM_RESET+1)
#define DF_TY_PD_CM_ADD_NAME                (DF_TY_PC_CM_POWER_OFF+1)
#define DF_TY_PD_CM_DEL_NAME                (DF_TY_PD_CM_ADD_NAME+1)

#define DF_TY_M_CM_PORT				100
#define DF_TY_M_CM_ORDER			(DF_TY_M_CM_PORT+1)
#define DF_TY_M_CM_HOOK				(DF_TY_M_CM_ORDER+1)
#define DF_TY_M_CM_READ				(DF_TY_M_CM_HOOK+1)
#define DF_TY_M_CM_WRITE			(DF_TY_M_CM_READ+1)
/*------------------------��������������ֵ---------------------------*/
#define DF_TY_MAX_PAR				8

#define DF_TER_VERSION_NUM				"20"				//�������
#define DF_TER_VERSION_TIME				"2024-02-04"	//���򷢲�ʱ��
#define DF_MB_HARDWARE_A20_A33			1				//A20����Ϊ0��A33����Ϊ1  MTK����Ϊ2
#define DF_TY_1502_HARD_EDIT			"Y3.0.4"

#define DF_TY_USB_PORT_N_MAX 					120+32   //���˿���
#define DF_TCP_SER_CONNECT_LINE_NUM_MAX         120+32   //���������


//2303����
#define DF_TER_VERSION_NAME_MB2303				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2303				11				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2303               	60				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2303		60				//TCP������·����
#define DF_TER_TYPE_NAME_MB2303					"MB_2303"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2303					"MB2402303CX"	//��������

//2110����
#define DF_TER_VERSION_NAME_MB2110				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2110				4				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2110                 100				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2110		100				//TCP������·����
#define DF_TER_TYPE_NAME_MB2110					"MB_2110"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2110					"MB2402110CX"	//��������

//2108����
#define DF_TER_VERSION_NAME_MB2108				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2108				6				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2108                 68				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2108		68				//TCP������·����
#define DF_TER_TYPE_NAME_MB2108					"MB_2108"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2108					"MB2402108CX"	//��������

//2202����
#define DF_TER_VERSION_NAME_MB2202				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2202				8				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2202                 20				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2202		20				//TCP������·����
#define DF_TER_TYPE_NAME_MB2202					"MB_2202"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2202					"MB2402202CX"	//��������

//2306����
#define DF_TER_VERSION_NAME_MB2306				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2306				12				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2306                 60				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2306		60				//TCP������·����
#define DF_TER_TYPE_NAME_MB2306					"MB_2306"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2306					"MB2402306CX"	//��������

//2212����
#define DF_TER_VERSION_NAME_MB2212				"4.0.0.3"		//����汾
#define DF_TY_MACHINE_TYPE_MB2212				9				//mb1601��mb1603��һ��HUB����Ϊ1��mb1602��һ��HUB����Ϊ0,mb1702Ϊ8���豸3,mb1603C�˿����з�ʽ��ͬΪ4,mb1805Ϊ5�����豸,mb1806Ϊ6 mb1806aΪ6 mb_1905=7
#define DF_TY_USB_PORT_N_MB2212                 60				//���֧��60���˿�  
#define DF_TCP_SER_CONNECT_LINE_NUM_MB2212		60				//TCP������·����
#define DF_TER_TYPE_NAME_MB2212					"MB_2212"		//�ͺ�����
#define DF_TER_TYPE_EXE_MB2212					"MB2402212CX"	//��������

#define MAX_MESSAGE_QUEUE_NUM 400	//�����Ϣ�����������
#define MAX_DEAL_MESSAGE_TASK 80	//�����Ϣ�߳�����
	
#define MAX_HISTROY_DEAL_RESULT 100	//��Ϣ��ʷ������
#define DEAFULT_CONNECT_TIME 60		//MQTTĬ�������ʱ��

#define DF_CODE_GB18030						0
#define DF_CODE_UTF8						1

#define DF_CODE_SELF						DF_CODE_GB18030

#endif	
