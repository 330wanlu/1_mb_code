#ifndef _USB_PORT_H
#define _USB_PORT_H

#include "opt.h"

#include "power_saving.h"
//#include "module.h"
#include "../../app/inc/module.h"
// #include "define.h"
#include "usb_file.h"
#include "power_saving.h"
#include "get_net_state.h"
#include "ty_usb.h"
#include "robot_arm.h"

#ifdef TEST_MODEL
#include "test.h"
#endif

#define usb_port_out(s,arg...)       				//log_out("all.c",s,##arg)  

#define READ_QUENE_MAX_NUM  50

#define DF_CLIENT_ASC_NUMBER            64      //�ͻ��˱�ʶ��
#define DF_TY_USB_DATA_MAX_TIMER_S      11/2				//��·���ӳ�ʱʱ��
#define DF_TY_USB_MQTT_REPORT_STATUS	15				//MQTT��ʱ�ϱ����ʱ���Ͻӿ�
#define DF_TY_USB_MQTT_M_SERVER_HEART_REPORT	60			//MQTT��ʱ�ϱ����ʱ���½ӿ�
#define DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT	3			//MQTT��ʱ��һ��״̬�ϱ����ʱ���½ӿ�
#define DF_TY_USB_MQTT_OFFLINE_UP_TIME   2000				//�ϴ�����Ʊ�����һ�ο�Ʊ���ʱ��

#define DF_TY_USB_POWER_OFF_TIME 	1
#define DF_TY_USB_CHECK_WITH_DEV 	10
#define DF_TY_USB_CHECK_WITHOUT_DEV 2


#define AUTH_CERT_FILE_PATH		"/etc/auth_cert.json"

typedef int (*Funaction_power)(int port_num,PORT_STATE_ENUM port_state);

//�˿ڵ�Դ������ز�������
#pragma pack(4)
typedef struct ACTION_POWER_DATA_T
{
	uint8 power_status;   //��Դ״̬
	int power_off_time;   //�µ�ʱ��
	int power_control_type;  //1:��е�۷�ʽ    0:��ͳ��ʽ
}ACTION_POWER_DATA;
//�˿ڵ�Դ�ṹ��
#pragma pack(4)
typedef struct ACTION_POWER_T
{
	Funaction power_on_off;  //��Դ����
	Funaction get_power_status;  //��ȡ��Դ��ز���
	ACTION_POWER_DATA power_data[DF_TY_USB_PORT_N_MAX];  //��Դ��������
	Funaction_power fun_power;

}ACTION_POWER;

enum LOAD_STATE
{
	UNATTACHED = 0,//δ����
	ATTACHED //�Ѽ���
		  
};

enum LOAD_NET_TYPE
{
	INTRANET = 0,   //����
	EXTERNAL_NETWORK,//����
	OTHER_NETWORK	 //����
};

//���ز�����ز�������
#pragma pack(4)
typedef struct LOAD_DATA_T
{
	enum LOAD_STATE usbip_state;    //����״̬
	enum LOAD_NET_TYPE net_type;
	int port; 
	unsigned int ip;
	char oper_id[128];
	char user[128];
	int *port_fd;
}LOAD_DATA;
//���ؽṹ��

#pragma pack(4)
typedef struct ACTION_LOAD_T
{
	Funaction fun_load_start;  //����һ�μ���
	Funaction fun_load_stop;  //ֹͣһ�μ���
	LOAD_DATA load_data[DF_TY_USB_PORT_N_MAX]; //���ز�������

}ACTION_LOAD;


//��Ϣ�ϱ���ز�������
#pragma pack(4)
typedef struct REPORT_DATA_T
{
	uint8 report_status;    //�ϱ�״̬
	int report_s_used;//�ϱ��ն�״̬�����Ƿ����
	int report_h_used;//�ϱ��ն����������Ƿ����
	int report_o_used;//�ϱ��ն��Ͻӿڶ����Ƿ����
	int report_p_used;//�ϱ��ն˶˿���Ϣ�����Ƿ����
	int report_i_used;//�ϱ��ն˿���Ϣ�����Ƿ����
	int report_ter_flag;//�ն���Ϣ���ϴ�һ�� 0δ�ɹ��ϴ����ٴ��ϴ� 1�ѳɹ��ϴ������ٴ��ϴ�
	unsigned long last_report_time_o;//���ϱ��ӿ�����ϱ�ʱ��
	unsigned long last_report_time_h;//���ϱ���������ϱ�ʱ��
	unsigned long last_report_time_s;//���ϱ�״̬����ϱ�ʱ��
	unsigned long last_report_time_s_report;//���ϱ�״̬����ϱ�ʱ��
	int report_ter_flag_report;
	uint16 report_counts;
}REPORT_DATA;
//��Ϣ�ϱ��ṹ��
#pragma pack(4)
typedef struct ACTION_REPORT_T
{
	Funaction report;  //�ϱ���Ϣ
	REPORT_DATA report_data; //�ϱ���Ϣ��������

}ACTION_REPORT;


//��ȡ�豸��Ϣ��ز�������
#pragma pack(4)
typedef struct GET_DEV_INFO_DATA_T
{
	uint8 get_dev_info_status;    //״̬

}GET_DEV_INFO_DATA;
//��ȡ�豸��Ϣ�ṹ��
#pragma pack(4)
typedef struct ACTION_GET_DEV_INFO_T
{
	Funaction get_dev_info;  //��ȡ�豸��Ϣ
	GET_DEV_INFO_DATA get_dev_info_data; //��ȡ�豸��Ϣ��������

}ACTION_GET_DEV_INFO;

#pragma pack(4)
typedef struct KEY_AUTH_STATE_T
{
	unsigned char key_s[4096];
	char *cert_json;
	char start_time[20];
	char end_time[20];
	int keyfile_load_err;//��Կ���ݶ�ȡʧ��
	int lib_load_err;//���ܿ����ʧ��
	int lib_dec_err;//���ܿ����ʧ��
	int auth_dec_err;//��Ȩ�ļ�����ʧ��
	Funaction upload_auth_cert_file;  //������Ȩ��Ϣ
	Funaction get_auth_cert_file;  //��ȡ��Ȩ��Ϣ
}KEY_AUTH_STATE;

#pragma pack(4)
struct _port_last_info
{
	unsigned short usb_app;// 0δ֪	1��˰��	2˰����	3ģ����	4 ca / ukey	5 ����
	int cert_err;				//Ĭ�ϳ�ʼΪ0  ���һ����֤�������Ϊ-1  ��֤�ɹ���ֵΪ1
	uint8 ca_name[110];
	uint8 ca_serial[20];
	uint8 sn[20]; //��ȡ˰�̻�����Ϣʱ��˰��ʱ��
	unsigned short extension;//�ֻ��� ����Ϊ0
	uint16 vid;
	uint16 pid;
};

#pragma pack(4)
struct _usb_port_infor
{
	/*USB����*/
	char arm_version[32];  //��е��ר�ã��汾��
	int arm_status;
	int arm_with_dev;
	char port_str[200];
	unsigned short port;
	unsigned short port_power;					//�˿ڵ�Դ 0�ص�״̬	1����״̬
	unsigned short with_dev;					///�����豸
	char busid[DF_TY_USB_BUSID];
	uint8 subclass;
	uint8 dev_type;
	uint8 protocol_ver;
	uint16 vid;
	uint16 pid;
	uint8 bnuminterface;
	int port_used;					//�˿��Ƿ�ʹ�� 1ʹ����  0δʹ��
	unsigned short system_used;		//ϵͳʹ�� 1ϵͳʹ��
	long used_time;					//�˿�ʹ��ʱ��
	int used_level;					//1usb_shareʹ��  2֤�鼰������Ϣ��ȡ  3��Ʊ���ߡ����ϡ����ܷ�д���ϴ�  4˰�����ݲ�ѯ  5��Ʊ���ݲ�ѯ  6��Ʊ���ݲ�ѯ���ϴ�
	char used_info[1024];
	unsigned short usb_app; // 0δ֪	1��˰��	2˰����	3ģ����	4 ca / ukey	5 ����
	unsigned short usb_share;//�Ƿ�usb-share����ʹ��0��		1��
	unsigned short usb_share_mode;//usb-share����ʹ��ģʽ 0����ʹ��		1����ת��ʹ��			2������ʽ
	int check_num;				//���������رն˿ں��豸���ļ�ϵͳ����ʧ��������
	//long port_insert_time;			//�˿�USB�豸����ʱ���
	unsigned short plate_type;
	int tran_data;					//USB�����ݶ�д
	/*USBIP����*/
	unsigned short port_status;            //״̬,usbip��״̬  0δʹ��  2������
	uint32 ip;
	long last_open_time;		//���һ�ο���ʹ��ʱ��
	int usbip_state;			//�˿��Ƿ�ʹ��  1usbip��״̬   0usbip�ر�״̬
	int tran_line_closed_flag;
	char client[DF_CLIENT_ASC_NUMBER];      //�ͻ��˱�ʶ
	char oper_id[128];
	char user[128];
	//int connect_timer;          //����ʱ�䣨�����򿪶˿ں��¼������·����ʱ����
	int app_fd;                 //Ӧ��ʹ�þ��
	/*֤������*/
	//unsigned short ca_read;	//���֤�����ƺ�˰�Ŷ�ȡ--��־λ
	uint8 ca_name[110];
	uint8 ca_serial[20];
	unsigned short encording;
	unsigned short ca_ok;
	unsigned short in_read_base_tax;
	/*˰����Ϣ����*/
	unsigned short tax_read;//���˰����Ϣ��ȡ--��־λ
	unsigned short need_chaoshui;//��Ҫ��˰
	unsigned short need_huizong;//��Ҫ�ϱ�����
	unsigned short need_fanxiejiankong;//��Ҫ��д���
	unsigned short inv_query;				//��Ʊ��ѯ������0�ر� 1��������Ϊֹͣ��Ʊ��ѯʹ�ã�
	uint8 sn[16];//˰�̱��
	unsigned char plate_time[20]; //��ȡ˰�̻�����Ϣʱ��˰��ʱ��
	unsigned short extension;//�ֻ��� ����Ϊ0
	int plate_sys_time;			//˰���뵱ǰϵͳʱ���
	uint8 month_fp_num[6]; //��¼�ϴβ�ѯ�·ݷ�Ʊ����
	int fp_num;				//��¼�ϴβ�ѯ���ķ�Ʊ����
	unsigned long long last_kp_time;		//���Ʊʱ��,����2�뷽�����ϴ�����Ʊ
	int offinv_num_exit;	//����Ʊ�������Ƿ��������Ʊ
	int offinv_stop_upload;	//ֹͣ����Ʊ�ϴ����ܣ� 0��ֹͣ 1ֹͣ
	int off_up_state;		//�����ϴ���־ 0δ���ϴ�  1�ϴ���
	int upfailed_count;		//�ϴ�ʧ�ܴ���
	struct _upload_inv_num upload_inv_num;
	int cb_state;		//����״̬  0 δ֪  1 �����ɹ�  -1ʧ��
	int hz_state;		//����״̬  0 δ֪  1 ���ܳɹ�  -1ʧ��
	int qk_state;		//�忨״̬  0 δ֪  1 �忨�ɹ�  -1ʧ��
	int m_inv_state;	//��Ʊ������M����ͬ��״̬	0δͬ��  1����ͬ��  2ͬ�����	-1����
	int m_tzdbh_state;	//������Ϣ����M����ͬ��״̬ 0δͬ��  1����ͬ��  2ͬ�����	-1����
	//int m_netinv_state; //������ƱԴ��M����ͬ��״̬ 0δͬ��  1����ͬ��  2ͬ�����	-1����
	int e_tzdbh_c;	//������Ϣ���ȡʧ�ܴ���
	//int e_netinv_c; //������ƱԴ��ȡʧ�ܴ���
	struct _plate_infos plate_infos;
	int connect_success_count;
	int connect_failed_count;
	int connect_success_time;
	int connect_failed_time;
	char start_time_s[30];
	char end_time_s[30];
	//char connect_errinfo[1024];
	char tax_business[3000];
	int ad_status;//ad״̬
	unsigned short ca_read;		//��1�����һ��CA֤��˰�Ŷ�ȡ
	int fail_num;
	unsigned int close_port_flag;
};

#pragma pack(4)
//��ȡ�豸������Ϣ���ݣ����ƣ�˰�ţ�
typedef struct DEV_BASIC_DATA_T
{
	int port;                   //�˿ں�
	int lock;                   //�˿���
	unsigned short power;		//�˿ڵ�Դ 0�ص�״̬	1����״̬
	int port_led_status;		//1û���豸��δ��Ȩ   2�豸�ڱ���    3�豸��Զ��ʹ�� 4�ڱ��ص�֤���쳣 5ϵͳʹ���� 6Զ�̼��غ������ݶ�д 7˰��Ĭ�Ͽ����쳣 8˰���豸������� 9�˿�û���豸����Ȩ��
	long insert_time;			//USB����ʱ��
	int usb_err;				//0δ���ֹ����� 1���ֹ�����
	//int cert_open;				//0δ��Ȩ  1��Ȩ
	int aisino_sup;
	int nisec_sup;
	int cntax_sup;
	int mengbai_sup;
	char aisino_end_time[20];
	char nisec_end_time[20];
	char cntax_end_time[20];
	char mengbai_end_time[20];
	char start_time[20];
	char end_time[20];
	char frozen_ago_time[20];
	int retry_forbidden;		//��ֹ���Զ˿���Ϣ��ȡ��  0����ֹ  1��ֹ
	int off_report;//�����ϱ� 0����Ҫ  1��Ҫ
	int up_report;//�����ϱ� 0����Ҫ  1��Ҫ
	int last_act;//���һ�ζ���Ϊ 1����  2����
	int read_success_flag;  //��ȡ��־
	unsigned short in_read_base_tax;
	long last_without_time;
	struct _port_last_info port_last_info;//��һ��USB�豸�Ļ�����Ϣ
	struct _usb_port_infor port_info;//�˿���Ϣ
	struct HandlePlate h_plate;
	int sys_reload; //����ĳ˰��ukey����ǰ��ϵ������˿ڵ��ֲ������¶�ȡ����˰�ŵ����
	int power_mode;// 0 ����ģʽ�˿�  1��ʡ��ģʽ�˿�
	int status_counts;
	int fail_num;
	unsigned int now_report_counts; 		//wang 11/10
	unsigned int nongye_report;				//wang 11/18

}DEV_BASIC_DATA;
//��ȡ�豸������Ϣ�ṹ��
#pragma pack(4)
typedef struct ACTION_GET_DEV_BASIC_T
{
	Funaction get_dev_basic;  //��ȡ�豸������Ϣ
	DEV_BASIC_DATA dev_basic_data[DF_TY_USB_PORT_N_MAX]; //�豸������Ϣ
	DEV_BASIC_DATA usb_dev_basic_data[DF_TY_USB_PORT_N_MAX]; //usb�豸������Ϣ,���������ݴ��ڵط�

}ACTION_GET_DEV_BASIC;

//��е�۲�����ز�������
#pragma pack(4)
typedef struct MECHANICAL_ARM_DATA_T
{
	uint8 arm_status;    //��е��״̬

}MECHANICAL_ARM_DATA;
//��е�۽ṹ��

typedef struct
{
	int arm_with_dev_result;
	int arm_with_dev_fd;
	int port;

}ARM_WITH_DEV;

#pragma pack(4)
typedef struct ACTION_MECHANICAL_ARM_T
{
	Funaction arm_start;  //����һ�λ�е��
	Funaction arm_update;  //����һ�λ�е������
	Funaction get_arm_update_state;  //��ȡ��е������״̬
	// Funaction get_arm_status; //��ȡ��е��״̬
	// MECHANICAL_ARM_DATA arm_data; //��е�۲�������
	ROBOT_ARM_CALSS *class;
	ARM_WITH_DEV arm_with_dev[DF_TY_USB_PORT_N_MAX];
	DEV_BASIC_DATA arm_data[DF_TY_USB_PORT_N_MAX];
	// char arm_with_dev_result[DF_TY_USB_PORT_N_MAX/2];   //��е�����Ƿ�����豸
	// int arm_with_dev_fd[DF_TY_USB_PORT_N_MAX/2];

}ACTION_MECHANICAL_ARM;

typedef struct
{
	char arm_version[32];  //��е�۰汾��
	int arm_status;
	int arm_with_dev;
}REBOT_ARM_DATA_STRUCT;

//U�ܲ�����ز�������
#pragma pack(4)
typedef struct U_SHIELD_DATA_T
{
	uint8 u_shield_status;    //U��״̬

}U_SHIELD_DATA;
//U�ܽṹ��
#pragma pack(4)
typedef struct ACTION_U_SHIELD_T
{
	Funaction u_shield_get_data;  //U�ܻ�ȡ����
	U_SHIELD_DATA u_shield_data; //U�ܲ�������

}ACTION_U_SHIELD;
//������ؽṹ��
#pragma pack(4)
typedef struct ACTION_ONLINE_BANK_T
{
	ACTION_MECHANICAL_ARM mechanical_arm_action;  //��е�����
	ACTION_U_SHIELD u_shield_action;  //U�����

}ACTION_ONLINE_BANK;

//�˿ڼ���õ������ݲ���������
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_FUN_T
{
	Funaction fun_start_step;  	 //��ʼ��ִ�ж���
	Funaction fun_recheck_step;  //���¼���ִ�ж���
	Funaction fun_conform_step;  //ȷ�ϵ�ִ�ж���
	Funaction fun_end_step;  	 //������ִ�ж���
	Funaction fun_init;			 //�豸�γ������

}PROCESS_JUDGE_USBPORT_EXIST_FUN;

enum IGNORE_FLAG  //�Թ�����־
{
	NO_IGNORE = 0, //���Թ�
	IGNORE	//�Թ�
};

//ȷ�������豸���̲���ö��
enum PROCESS_JUDGE_USBPORT_EXIST_STEPS
{
	WAIT_STEP = 0,
	START_STEP,		//��ʼ���裬��ȡ�˿������豸���������ݽ����ο�����Ҫrecheck
	RECHECK_STEP,	//���¼��
	CONFROM_STEP, 	//����ȷ�����

};

enum USBPORT_EXIST_STATE
{
	WITHOUT_DEV = 0, //���豸
	WITH_DEV,		 //���豸
};

enum USBPORT_ACTION
{
	NO_ACTON = 0, //�޶���
	DEV_INSERT,	  //�豸����
	DEV_EXTRACT	  //�豸�γ�
};

enum USBPORT_RECHECK_RESULT
{
	RECHECK_RESULT_NOT_EXIST = 0, //���¼�ⲻ����
	RECHECK_RESULT_EXIST	  //���¼�����
};
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_DATA_T
{
	char busid[32];		//���ڼ���豸��������busid
	enum PROCESS_JUDGE_USBPORT_EXIST_STEPS process_step;   //����ȷ�ϲ���
	enum USBPORT_EXIST_STATE now_port_state;			   //��ǰ��¼�Ķ˿��豸�������
	enum USBPORT_EXIST_STATE target_port_state;			   //�˴μ�⵽��״̬����Ҫȥ����ȷ�ϵ�״̬
	enum USBPORT_ACTION usbport_action;					   //�˿ڵĶ���
	enum USBPORT_EXIST_STATE recheck_result;			   //���¼��Ľ��
	int with_dev_recheck_failed_count;					 //�൱�ڼ��ʱ������˲�
	int without_dev_recheck_failed_count;				 //�൱�ڼ��ʱ������˲�
	int without_dev_count;
	int without_robot_arm_count;
	enum IGNORE_FLAG ignore_judge_flag;								//�Թ�����־
	int start_step_time;

}PROCESS_JUDGE_USBPORT_EXIST_DATA;
#pragma pack(4)
typedef struct PROCESS_JUDGE_USBPORT_EXIST_T
{
	PROCESS_JUDGE_USBPORT_EXIST_FUN judge_exist_fun;
	PROCESS_JUDGE_USBPORT_EXIST_DATA port_exist_state[DF_TY_USB_PORT_N_MAX];

}PROCESS_JUDGE_USBPORT_EXIST;

//usb�˿ڽṹ�壬�������в���
#pragma pack(4)
typedef struct GET_PORT_STATUS_T
{
	Funaction fun_get_port_ad_status; //��ȡ�˿������豸�������ο�ֵ��
	char port_status_buff[DF_TY_USB_PORT_N_MAX];
	char last_port_status_buff[DF_TY_USB_PORT_N_MAX]; //��һ�εĶ˿�״̬
}GET_PORT_STATUS;

//��ȡ˰���������ݲ���
enum READ_TAX_NAME_STATE
{
	WAIT_READ = 0,
	NEED_READ,
	READING,
	READ_COMPLATED,
	READ_FAILED
};

#pragma pack(4)
typedef struct READ_TAX_NAME_LIST_T
{
	int port_num;
	enum READ_TAX_NAME_STATE read_state;
	struct READ_TAX_NAME_LIST_T *next;

}READ_TAX_NAME_LIST;

#pragma pack(4)
typedef struct READ_TAX_NAME_T
{
	enum READ_TAX_NAME_STATE read_state[DF_TY_USB_PORT_N_MAX];
	Funaction fun_read_tax_name;
	int need_read_list_lock;	//����ȡ�б���
	READ_TAX_NAME_LIST *need_read_tax_name_list_head; //����ȡ�б�
	int reading_list_lock;		//���ڶ�ȡ�б���
	READ_TAX_NAME_LIST *reading_tax_name_list_head;	  //���ڶ�ȡ�б�
	int read_queue_num;
	int ca_read_lock;
	int first_start;
}READ_TAX_NAME;

#pragma pack(4)
typedef struct TASK_FD_T
{
	int get_port_status_fd;
	int usbport_exist_recheck_fd;
	int port_check_fd;
	int read_tax_name_fd;
	int led_manage_fd;
	int report_fd;
	int upgrade_strategy_fd;
	int attach_fd;
	
}TASK_FD;

//����״̬  0��δ����   1��������    2�������ɹ�
typedef enum
{
	UPGRADE_STRATEGY_NO_UPDATE = 0,
	UPGRADE_STRATEGY_UPDATING,
	UPGRADE_STRATEGY_UPDATE_SUCCESS,
	UPGRADE_STRATEGY_UPDATE_FAILED
}UPGRADE_STRATEGY_STATE;

//�������Խṹ��
typedef struct 
{
	char old_version[32];	 //���еĻ�е�۰汾��
	UPGRADE_STRATEGY_STATE  update_state;		 

}UPGRADE_STRATEGY_T;

typedef struct
{
	char file_version[32];	 //�����ļ��汾��
	int  need_update_port_num[DF_TY_USB_PORT_N_MAX]; //��Ҫ�����Ļ�е�ۺ�
	UPGRADE_STRATEGY_T upgrade_strategy_struct[DF_TY_USB_PORT_N_MAX];   //�������Խṹ
}UPGRADE_STRATEGY;

//usb�˿ڽṹ�壬�������в���
#pragma pack(4)
typedef struct USB_PORT_CLASS_T
{
	int data_lock; 			//����������ֹ���̵߳��������
	GET_PORT_STATUS get_port_status;
	ACTION_POWER power_action; //�˿ڵ�Դ���
	ACTION_ONLINE_BANK online_bank_action; //��е�����
	ACTION_LOAD load_action; //�������
	PROCESS_JUDGE_USBPORT_EXIST judge_usbport_exist; //���˿ڴ������
	ACTION_REPORT report_action; //�ϱ����
	ACTION_GET_DEV_INFO get_dev_info_action; //��ȡ�豸��Ϣ���
	ACTION_GET_DEV_BASIC get_dev_basic_action; //��ȡ�豸������Ϣ
	READ_TAX_NAME read_tax_name;
	Fun_get_ter_all_data_json fun_get_ter_all_data_json;              //�������ݴ��������һ ��Ϊʲô��ֱ�Ӹ�һ������tm������
	Fun_get_ter_all_data_json fun_get_ter_all_data_json_without_port; //�������ݴ����������
	Fun_get_ter_all_data_json fun_get_ter_all_data_json_without_tax;  //�������ݴ����������
	Funaction fun_check_port_status;  //������޶˿���ʹ��
	Funaction fun_check_port_with_num;  //���˿��Ƿ�����
	Funaction fun_get_machine_type;
	Funaction fun_check_with_robot_arm;
	MODULE *module; //ģ�����ָ��
	KEY_AUTH_STATE key_auth_state;
	TASK_FD task_fd;
	int timer_fd;
	int usb_port_numb;
	int machine_type;  //�����ͺ�,1Ϊ���û�е�۵�
	float intranet_delay_ms;
	float external_delay_ms;
	_so_note *app_note;
	UPGRADE_STRATEGY upgrade_strategy;   //�������Խṹ
}USB_PORT_CLASS;

//USB_PORT����ֵ
enum USB_PORT_RESPONSE_CODE
{
	USB_PORT_PARAMETER_ERROR = -5, //���δ���
	USB_PORT_NULL_FUN,  	  //����ָ��Ϊ��
	USB_SYS_ERROR,		      //ϵͳ�������
	USB_PARAMETER_NO_MATCH,	  //�Ҳ���ƥ��Ĳ���
	USB_COMMON_ERROR,		  //һ�����
	USB_PORT_NO_ERROR = 0,
};

int usb_port_init(MODULE *module,_so_note *app_note);
USB_PORT_CLASS *get_usb_port_class(void);

#endif
