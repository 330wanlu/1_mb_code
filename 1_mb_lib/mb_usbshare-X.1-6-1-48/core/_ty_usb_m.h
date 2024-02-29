#ifndef _TY_USB_M_DEFINE 
#define _TY_USB_M_DEFINE
#include "_ty_usb.h"
#include "_ty_pd.h"
#include "_event_file.h"
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include "_tran_data.h"
#include "../manage/_process_manage.h"
#include "../linux_sys/mqtt_client.h"
#include "../linux_sys/_http.h"
#include "../manage/_process_manage.h"
#include "_lcd_state.h"
#ifndef MTK_OPENWRT
#include "../linux_sys/_mb_sql.h"
#endif
#include "_deploy.h"
#include "_get_net_state.h"
#include "_machine_infor.h"
#include "_m_server.h"
#include "_invoice_transmit.h"

/*====================================����������==============================================*/

#define DF_CLIENT_ASC_NUMBER            64      //�ͻ��˱�ʶ��
#ifdef _ty_usb_m_c
#ifdef DEBUG
#define out(s,arg...)       				log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						log_out("all.c",s,##arg)
#endif	

#define e_ty_usb_m
#define DF_TY_USB_DATA_MAX_TIMER_S      5				//��·���ӳ�ʱʱ��
#define DF_TY_USB_MQTT_REPORT_STATUS	15				//MQTT��ʱ�ϱ����ʱ���Ͻӿ�
#define DF_TY_USB_MQTT_M_SERVER_HEART_REPORT	60			//MQTT��ʱ�ϱ����ʱ���½ӿ�
#define DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT	3			//MQTT��ʱ��һ��״̬�ϱ����ʱ���½ӿ�
#define DF_TY_USB_MQTT_OFFLINE_UP_TIME   2000				//�ϴ�����Ʊ�����һ�ο�Ʊ���ʱ��

#define POWER_ERR_FULL_LOAD	-1008
#define POWER_ERR_NO_DEV 	-1009

#define DF_PLATE_USB_PORT_NOT_AUTH -106

#else
#define e_ty_usb_m                      extern 
#endif
/*====================================�ṹ�嶨��==============================================*/


struct	_queue_state
{
	uint8 use_flag;         //�Ƿ���ʹ��
	char summary_month[64]; //��Ҫ�ϴ��·�
	char now_month[64]; 	//Ŀǰ�ϴ��·�
	char start_time[64];    //��ʼʱ��
	char end_time[64];    //��ʼʱ��
};


struct _usb_port_infor
{
	/*USB����*/
	char port_str[200];
	unsigned short port;
	unsigned short port_power;					//�˿ڵ�Դ 0�ص�״̬	1����״̬
	unsigned short with_dev;
	char busid[DF_TY_USB_BUSID];
	uint8 subclass;
	uint8 dev_type;
	uint8 protocol_ver;
	uint16 vid;
	uint16 pid;
	uint8 bnuminterface;
	int port_used;		//�˿��Ƿ�ʹ�� 1ʹ����  0δʹ��
	unsigned short system_used;		//ϵͳʹ�� 1ϵͳʹ��
	long used_time;					//�˿�ʹ��ʱ��

	int used_level;					//1usb_shareʹ��  2֤�鼰������Ϣ��ȡ  3��Ʊ���ߡ����ϡ����ܷ�д���ϴ�  4˰�����ݲ�ѯ  5��Ʊ���ݲ�ѯ  6��Ʊ���ݲ�ѯ���ϴ�
	char used_info[1024];

	unsigned short usb_app; // 0δ֪	1��˰��	2˰����	3ģ����	4 ca / ukey	5 ����
	unsigned short usb_share;//�Ƿ�usb-share����ʹ��0��		1��
	unsigned short usb_share_mode;//usb-share����ʹ��ģʽ 0����ʹ��		1����ת��ʹ��			2������ʽ


	int check_num;				//���������رն˿ں��豸���ļ�ϵͳ����ʧ��������
	//long port_insert_time;			//�˿�USB�豸����ʱ���
	int tran_data;					//USB�����ݶ�д



	/*USBIP����*/
	unsigned short port_status;            //״̬,usbip��״̬  0δʹ��  2������
	uint32 ip;
	long last_open_time;		//���һ�ο���ʹ��ʱ��
	int usbip_state;			//�˿��Ƿ�ʹ��  1usbip��״̬   0usbip�ر�״̬
	char client[DF_CLIENT_ASC_NUMBER];      //�ͻ��˱�ʶ
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

	struct _queue_state queue_state; //��Ʊͬ������ʹ�����


	int connect_success_count;
	int connect_failed_count;
	int connect_success_time;
	int connect_failed_time;

	char start_time_s[30];
	char end_time_s[30];
	//char connect_errinfo[1024];

	char tax_business[3000];

	int ad_status;//ad״̬
#ifdef ALL_SHARE
	unsigned short ca_read;
	int fail_num;
#endif
	
};
#define REFER_TO_TAX_ADDRESS 1
#define NOT_REFER_TO_TAX_ADDRESS 0



#ifdef _ty_usb_m_c   
enum POWER_MODE
{
	Normal_Mode = 0,
	Saving_Mode,	
};


struct _ty_usb_m_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _ty_usb_m    *dev;
};

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

//�˿����Խṹ��
struct _port_state
{
	int port;                   //�˿ں�
	int lock;                   //�˿���
	unsigned short power;					//�˿ڵ�Դ 0�ص�״̬	1����״̬
	int port_led_status;		//1û���豸��δ��Ȩ   2�豸�ڱ���    3�豸��Զ��ʹ�� 4�ڱ��ص�֤���쳣 5ϵͳʹ���� 6Զ�̼��غ������ݶ�д 7˰��Ĭ�Ͽ����쳣 8˰���豸������� 9�˿�û���豸����Ȩ��
	long insert_time;			//USB����ʱ��
	int usb_err;				//0δ���ֹ����� 1���ֹ�����
	int usb_err_cs;				//�������ֹ�����Ĵ���
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

	int need_getinv;			//0����Ҫ��ȡ���ϴ��ķ�Ʊ�·���Ϣ 1��Ҫ��ȡ���ϴ��ķ�Ʊ�·���Ϣ
	//int need_sync_tzdbh;		//0����Ҫ��ȡ��ͬ��������Ϣ�� 1��Ҫ��ȡ��ͬ��������Ϣ��
	//int need_sync_netinv;		//0����Ҫ��ȡ��ͬ��������ƱԴ 1��Ҫ��ȡ��ͬ��������ƱԴ
	unsigned short in_read_base_tax;

	int inuse_sync;				//�߳�����ͬ������
	long last_without_time;
	struct _port_last_info port_last_info;//��һ��USB�豸�Ļ�����Ϣ
	struct _usb_port_infor port_info;//�˿���Ϣ
	//struct _ty_usb_m		*dev_stream;
	struct HandlePlate h_plate;
	int sys_reload; //����ĳ˰��ukey����ǰ��ϵ������˿ڵ��ֲ������¶�ȡ����˰�ŵ����
	int power_mode;// 0 ����ģʽ�˿�  1��ʡ��ģʽ�˿�
#ifndef DF_OLD_MSERVER
	unsigned long last_report_time_t;//��Ʊͬ����Ϣ��ѯʱ��
	uint16 now_report_counts;
	int in_report;
	int off_up_flag; //1�����ϱ� 2�����ϱ�
#endif
	//�ڵ����¶���
	int port_init;	//�˿�һ��ʹ�û��߳�������¿�ʼʹ�ã����¶�ȡ������Ϣ��  0��ʼʹ��  1׼����ȡ����˰��  2��ȡ����˰�������ɳ�ʼ
	int need_up_power; //0����Ҫ�ϵ�   1��Ҫ�ϵ�
	int need_down_power; //0����Ҫ�µ�   1��Ҫ�µ�
	int ad_repair_check;
	long ad_repair_up_time;//��¼���һ�β����ϵ��ʱ��
	long ad_repair_down_time;//��¼���һ�β����µ��ʱ��
	int status;// 0���豸 1���豸  2��·
	int status_counts;
	
#ifdef ALL_SHARE
	int fail_num;
#endif
};

//ȫ�ֽṹ��
struct _ty_usb_m
{
	int state;                                  //
	int lock;                                   //
	unsigned char key_s[4096];
	struct _port_state  port[DF_TY_USB_PORT_N];
	struct _ty_usb_m_fd fd[DF_FILE_OPEN_NUMBER];
	char ty_usb_name[DF_FILE_NAME_MAX_LEN];
	int ty_usb_fd;
	int lcd_fd;
	int timer_fd;
	int tran_fd;
	int deploy_fd;
	int event_file_fd;
	int switch_fd;
	int get_net_fd;
	int usb_port_numb;
	int frist_start;
	int frist_up_inv;
	int machine_fd;
	char ter_id[20];
	char *cert_json;
	char tax_so_ver[50];
	char mbc_so_ver[50];
	int up_queue_num;	//�ϴ���Ʊ������
	int sync_queue_num;	//��Ʊͬ����ѯ����
	int base_read_queue_num;	//������Ϣ��ȡ������
	int onlineport;//�����Ѿ������豸����
	int poweredport;//�ѿ���˿�����
	sem_t	cb_qk_sem;//����Ʊ�ϴ��ź���

	//״̬��
	int stop_aisino_up; //ֹͣ��Ʊ�ϴ�
	int stop_nisec_up;//ֹͣ��Ʊ�ϴ�
	int stop_cntax_up;//ֹͣ��Ʊ�ϴ�
	int stop_mengbai_up;//ֹͣ��Ʊ�ϴ�


	unsigned long last_report_time_o;//���ϱ��ӿ�����ϱ�ʱ��
	unsigned long last_report_time_h;//���ϱ���������ϱ�ʱ��
	unsigned long last_report_time_s;//���ϱ�״̬����ϱ�ʱ��
	uint8 ad_status[4098];
	uint8 all_port_status[4096];
	int all_port_status_len;
	int all_port_updata;	//02 24 ����ֶ�

	int report_s_used;//�ϱ��ն�״̬�����Ƿ����
	int report_h_used;//�ϱ��ն����������Ƿ����
	int report_o_used;//�ϱ��ն��Ͻӿڶ����Ƿ����
#ifndef DF_OLD_MSERVER
	int report_t_used;//��Ʊͬ����ѯ�����Ƿ����
#else
	int report_p_used;//�ϱ��ն˶˿���Ϣ�����Ƿ����
	int report_i_used;//�ϱ��ն˿���Ϣ�����Ƿ����
#endif

	int report_ter_flag;//�ն���Ϣ���ϴ�һ�� 0δ�ɹ��ϴ����ٴ��ϴ� 1�ѳɹ��ϴ������ٴ��ϴ�	

	int lcd_share_state;//10usbshare��ʹ��  20usbshareʹ��
	int lcd_tax_state;//10usbshare��ʹ��  20usbshareʹ��
	int keyfile_load_err;//��Կ���ݶ�ȡʧ��
	int lib_load_err;//���ܿ����ʧ��
	int lib_dec_err;//���ܿ����ʧ��
	int auth_dec_err;//��Ȩ�ļ�����ʧ��

#ifndef DF_OLD_MSERVER
	uint16 report_counts;
#endif
	struct _dev_support dev_support;

};

struct _ty_usb_m_ctl_fu
{
	int cm;
	int(*ctl)(struct _ty_usb_m_fd   *id, va_list args);
};
void *ty_usb_m_id;








#endif        

#ifdef RELEASE_SO

//==ͨ�ýӿ�==//
//��ʼ����̬��
typedef int(*_so_common_init_tax_lib)(uint8 *, char *);
_so_common_init_tax_lib							so_common_init_tax_lib;

//������Ȩ�ļ�
typedef int(*_so_common_reload_tax_auth)(uint8 *);
_so_common_reload_tax_auth						so_common_reload_tax_auth;

//�ж���ʶ��˰�̵���Ȩ��Ϣ
typedef int(*_so_common_jude_plate_auth)(uint8, char *,char *);
_so_common_jude_plate_auth						so_common_jude_plate_auth;

//��ʼ����̬���ڲ���־
typedef int(*_so_common_init_tax_lib_log_hook)(void *, void *);
_so_common_init_tax_lib_log_hook				so_common_init_tax_lib_log_hook;

//��ȡƱ����Ϣ��Ӧ�ṹ���ڴ��
typedef int(*_so_common_find_inv_type_mem)(uint8, int *);
_so_common_find_inv_type_mem					so_common_find_inv_type_mem;

//��ȡ��Ȩ�ļ���Ϣ
typedef int(*_so_common_get_cert_json_file)(uint8 *, char **);
_so_common_get_cert_json_file					so_common_get_cert_json_file;

//���ش�����Ϣ�ϱ���ά��̨
typedef int(*_so_common_report_event)(char *, char *, char *, int);
_so_common_report_event							so_common_report_event;

//��Ʊ����Ԥ��ӿ�
typedef int(*_so_common_analyze_json_buff)(uint8 , char *, struct Fpxx *, char *);
_so_common_analyze_json_buff					so_common_analyze_json_buff;

//��ȡ������Ϣ��˰����Ϣ����Ʊ����Ϣ��֧�ֽ�˰�̡�˰���̡�ģ���̡�˰��ukey�����Ź�ѡkey��������ѡkey
typedef int(*_so_common_get_basic_tax_info)(struct HandlePlate *, struct _plate_infos *);
_so_common_get_basic_tax_info					so_common_get_basic_tax_info;

//�ر�USB�豸���
typedef int(*_so_common_close_usb_device)(struct HandlePlate *);
_so_common_close_usb_device						so_common_close_usb_device;

typedef int(*_so_common_get_basic_simple)(struct HandlePlate *, struct _plate_infos *);
_so_common_get_basic_simple					so_common_get_basic_simple;



//==����Ҫ��Ȩ�Ҳ���Ҫ������֤ͨ����ɵ��õĽӿ�==//
//��ȡ˰�̵�ǰʱ��
typedef int(*_so_aisino_get_tax_time)(struct HandlePlate *, char *);
_so_aisino_get_tax_time							so_aisino_get_tax_time;
typedef int(*_so_nisec_get_tax_time)(struct HandlePlate *, char *);
_so_nisec_get_tax_time							so_nisec_get_tax_time;
typedef int(*_so_cntax_get_tax_time)(struct HandlePlate *, char *);
_so_cntax_get_tax_time							so_cntax_get_tax_time;
typedef int(*_so_mb_get_plate_time)(struct HandlePlate *, char *);
_so_mb_get_plate_time							so_mb_get_plate_time;

//mqtt�ӿڻ�ȡ��Ʊ��ϸ
typedef int(*_so_aisino_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_aisino_get_inv_details_mqtt					so_aisino_get_inv_details_mqtt;
typedef int(*_so_nisec_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_nisec_get_inv_details_mqtt					so_nisec_get_inv_details_mqtt;
typedef int(*_so_cntax_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_cntax_get_inv_details_mqtt					so_cntax_get_inv_details_mqtt;
typedef int(*_so_mb_get_inv_details_mqtt)(struct HandlePlate *, unsigned char *, void *, void *, char *, int, unsigned short *, int);
_so_mb_get_inv_details_mqtt						so_mb_get_inv_details_mqtt;

//��Ʊ�ϴ�M����
typedef int(*_so_aisino_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_aisino_upload_m_server						so_aisino_upload_m_server;
typedef int(*_so_nisec_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_nisec_upload_m_server						so_nisec_upload_m_server;
typedef int(*_so_cntax_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_cntax_upload_m_server						so_cntax_upload_m_server;
typedef int(*_so_mb_upload_m_server)(struct HandlePlate *, unsigned short *, struct _upload_inv_num *, void *, void *,struct _inv_sum_data *);
_so_mb_upload_m_server							so_mb_upload_m_server;

//��ȡ�·�Ʊ����
typedef int(*_so_aisino_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_aisino_get_month_invoice_num				so_aisino_get_month_invoice_num;
typedef int(*_so_nisec_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_nisec_get_month_invoice_num					so_nisec_get_month_invoice_num;
typedef int(*_so_cntax_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_cntax_get_month_invoice_num					so_cntax_get_month_invoice_num;
typedef int(*_so_mb_get_month_invoice_num)(struct HandlePlate *, unsigned char *, unsigned int  *, unsigned long *);
_so_mb_get_month_invoice_num					so_mb_get_month_invoice_num;

//����Ʊ��������ȡ��Ʊ��Ϣ
typedef int(*_so_aisino_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_aisino_fpdm_fphm_get_invs					so_aisino_fpdm_fphm_get_invs;
typedef int(*_so_nisec_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_nisec_fpdm_fphm_get_invs					so_nisec_fpdm_fphm_get_invs;
typedef int(*_so_cntax_fpdm_fphm_get_invs)(struct HandlePlate *, char  *, char *, int , char **);
_so_cntax_fpdm_fphm_get_invs					so_cntax_fpdm_fphm_get_invs;

//����Ʊ��������ȡ��Ʊ�������ļ���Ϣ
typedef int(*_so_aisino_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_aisino_fpdm_fphm_get_invs_kpstr				so_aisino_fpdm_fphm_get_invs_kpstr;
typedef int(*_so_nisec_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_nisec_fpdm_fphm_get_invs_kpstr				so_nisec_fpdm_fphm_get_invs_kpstr;
typedef int(*_so_cntax_fpdm_fphm_get_invs_kpstr)(struct HandlePlate *, char  *, char *, int, char **, char **, char **);
_so_cntax_fpdm_fphm_get_invs_kpstr				so_cntax_fpdm_fphm_get_invs_kpstr;

//==��Ҫ��Ȩδ��֤�����������ɵ��õĽӿ�==//
//����������֤֤�����
typedef int(*_so_aisino_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_aisino_get_auth_passwd						so_aisino_get_auth_passwd;
typedef int(*_so_nisec_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_nisec_get_auth_passwd						so_nisec_get_auth_passwd;
typedef int(*_so_cntax_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_cntax_get_auth_passwd						so_cntax_get_auth_passwd;
typedef int(*_so_mb_get_auth_passwd)(struct HandlePlate *, struct _plate_infos *);
_so_mb_get_auth_passwd						so_mb_get_auth_passwd;

//�ӿڵ�����֤Ĭ��֤������Ƿ���ȷ
typedef int(*_so_aisino_verify_cert_passwd)(struct HandlePlate *, char *);
_so_aisino_verify_cert_passwd					so_aisino_verify_cert_passwd;
typedef int(*_so_nisec_verify_cert_passwd)(struct HandlePlate *, char *);
_so_nisec_verify_cert_passwd					so_nisec_verify_cert_passwd;
typedef int(*_so_cntax_verify_cert_passwd)(struct HandlePlate *, char *);
_so_cntax_verify_cert_passwd					so_cntax_verify_cert_passwd;
typedef int(*_so_mb_verify_cert_passwd)(struct HandlePlate *, char *);
_so_mb_verify_cert_passwd						so_mb_verify_cert_passwd;

//�޸�֤�����
typedef int(*_so_aisino_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_aisino_change_certpassword					so_aisino_change_certpassword;
typedef int(*_so_nisec_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_nisec_change_certpassword					so_nisec_change_certpassword;
typedef int(*_so_cntax_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_cntax_change_certpassword					so_cntax_change_certpassword;
typedef int(*_so_mb_change_certpassword)(struct HandlePlate *, char *, char *, char *);
_so_mb_change_certpassword						so_mb_change_certpassword;

//��ȡ��ǰ��Ʊ�������
typedef int(*_so_aisino_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_aisino_get_current_invcode					so_aisino_get_current_invcode;
typedef int(*_so_nisec_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_nisec_get_current_invcode					so_nisec_get_current_invcode;
typedef int(*_so_cntax_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_cntax_get_current_invcode					so_cntax_get_current_invcode;
typedef int(*_so_mb_get_current_invcode)(struct HandlePlate *, unsigned char, char*, char*);
_so_mb_get_current_invcode						so_mb_get_current_invcode;


//======��Ҫ��Ȩ����֤������ȷ�󷽿ɲ����Ľӿ�======//
//��˰����
typedef int(*_so_aisino_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_aisino_copy_report_data						so_aisino_copy_report_data;
typedef int(*_so_nisec_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_nisec_copy_report_data						so_nisec_copy_report_data;
typedef int(*_so_cntax_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_cntax_copy_report_data						so_cntax_copy_report_data;
typedef int(*_so_mb_copy_report_data)(struct HandlePlate *, uint8, char *);
_so_mb_copy_report_data							so_mb_copy_report_data;

//�����忨
typedef int(*_so_aisino_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_aisino_report_summary_write_back			so_aisino_report_summary_write_back;
typedef int(*_so_nisec_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_nisec_report_summary_write_back				so_nisec_report_summary_write_back;
typedef int(*_so_cntax_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_cntax_report_summary_write_back				so_cntax_report_summary_write_back;
typedef int(*_so_mb_report_summary_write_back)(struct HandlePlate *, uint8, char *);
_so_mb_report_summary_write_back				so_mb_report_summary_write_back;

//���߷�Ʊ�ϴ�
typedef int(*_so_aisino_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_aisino_inv_upload_server					so_aisino_inv_upload_server;
typedef int(*_so_nisec_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_nisec_inv_upload_server						so_nisec_inv_upload_server;
typedef int(*_so_cntax_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_cntax_inv_upload_server						so_cntax_inv_upload_server;
typedef int(*_so_mb_inv_upload_server)(struct HandlePlate *, struct _offline_upload_result *);
_so_mb_inv_upload_server						so_mb_inv_upload_server;

//����Ʊ��������ϴ����߷�Ʊ
typedef int(*_so_aisino_fpdm_fphm_update_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_aisino_fpdm_fphm_update_invs				so_aisino_fpdm_fphm_update_invs;
typedef int(*_so_nisec_fpdm_fphm_update_invs)(struct HandlePlate *, char  *, char *, int, char **);
_so_nisec_fpdm_fphm_update_invs					so_nisec_fpdm_fphm_update_invs;

//����˰�������������
typedef int(*_so_aisino_test_server_connect)(struct HandlePlate *, char **,char *);
_so_aisino_test_server_connect					so_aisino_test_server_connect;
typedef int(*_so_nisec_test_server_connect)(struct HandlePlate *, char **,char *);
_so_nisec_test_server_connect					so_nisec_test_server_connect;
typedef int(*_so_cntax_test_server_connect)(struct HandlePlate *,char **, char *);
_so_cntax_test_server_connect					so_cntax_test_server_connect;
typedef int(*_so_mb_test_server_connect)(struct HandlePlate *, char **,char *);
_so_mb_test_server_connect						so_mb_test_server_connect;

//������������Ʊ����
typedef int(*_so_aisino_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_aisino_make_invoice							so_aisino_make_invoice;
typedef int(*_so_nisec_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_nisec_make_invoice							so_nisec_make_invoice;
typedef int(*_so_cntax_make_invoice)(struct HandlePlate *, unsigned char , char* , struct _plate_infos *, struct _make_invoice_result *);
_so_cntax_make_invoice							so_cntax_make_invoice;
typedef int(*_so_mb_make_invoice)(struct HandlePlate *, unsigned char, char*, struct _plate_infos *, struct _make_invoice_result *);
_so_mb_make_invoice								so_mb_make_invoice;

//�ѿ���Ʊ����
typedef int(*_so_aisino_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_aisino_waste_invoice						so_aisino_waste_invoice;
typedef int(*_so_nisec_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_nisec_waste_invoice							so_nisec_waste_invoice;
typedef int(*_so_cntax_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_cntax_waste_invoice							so_cntax_waste_invoice;
typedef int(*_so_mb_waste_invoice)(struct HandlePlate *, unsigned char, char  *, char *, char *, struct _cancel_invoice_result *);
_so_mb_waste_invoice							so_mb_waste_invoice;

//δ����Ʊ����
typedef int(*_so_aisino_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_aisino_waste_null_invoice						so_aisino_waste_null_invoice;
typedef int(*_so_nisec_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_nisec_waste_null_invoice						so_nisec_waste_null_invoice;
typedef int(*_so_cntax_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_cntax_waste_null_invoice						so_cntax_waste_null_invoice;
typedef int(*_so_mb_waste_null_invoice)(struct HandlePlate *,struct _plate_infos *, struct _cancel_invoice_result *);
_so_mb_waste_null_invoice						so_mb_waste_null_invoice;

//������ַ�Ʊ��������Ƿ���Կ��ߺ�Ʊ
typedef int(*_so_aisino_check_lzfpdm_lzfphm_allow)(struct HandlePlate *, unsigned char, char *, char *, uint32, char *);
_so_aisino_check_lzfpdm_lzfphm_allow			so_aisino_check_lzfpdm_lzfphm_allow;

//������Ϣ������
typedef int(*_so_aisino_upload_hzxxb)(struct HandlePlate *, uint8,char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_aisino_upload_hzxxb						so_aisino_upload_hzxxb;
typedef int(*_so_nisec_upload_hzxxb)(struct HandlePlate *, uint8, char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_nisec_upload_hzxxb						so_nisec_upload_hzxxb;
typedef int(*_so_cntax_upload_hzxxb)(struct HandlePlate *, uint8,char*, struct _plate_infos *, struct _askfor_tzdbh_result *);
_so_cntax_upload_hzxxb						so_cntax_upload_hzxxb;

//ͨ��������Ϣ���Ų�ѯ������Ϣ
typedef int(*_so_aisino_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_aisino_download_hzxxb_from_rednum				so_aisino_download_hzxxb_from_rednum;
typedef int(*_so_nisec_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_nisec_download_hzxxb_from_rednum				so_nisec_download_hzxxb_from_rednum;
typedef int(*_so_cntax_download_hzxxb_from_rednum)(struct HandlePlate *, char *, uint8 **, char *);
_so_cntax_download_hzxxb_from_rednum				so_cntax_download_hzxxb_from_rednum;

//ͨ��ʱ��β�ѯ������Ϣ��������Ϣ
typedef int(*_so_aisino_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_aisino_download_hzxxb_from_date_range				so_aisino_download_hzxxb_from_date_range;
typedef int(*_so_aisino_download_hzxxb_from_date_range_new)(struct HandlePlate *, char *, char *,uint8 **, char *);
_so_aisino_download_hzxxb_from_date_range_new				so_aisino_download_hzxxb_from_date_range_new;
typedef int(*_so_nisec_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_nisec_download_hzxxb_from_date_range				so_nisec_download_hzxxb_from_date_range;
typedef int(*_so_cntax_download_hzxxb_from_date_range)(struct HandlePlate *, char *, uint8 **, char *);
_so_cntax_download_hzxxb_from_date_range				so_cntax_download_hzxxb_from_date_range;

//����������Ϣ������
typedef int(*_so_aisino_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_aisino_cancel_hzxxb							so_aisino_cancel_hzxxb;
typedef int(*_so_nisec_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_nisec_cancel_hzxxb							so_nisec_cancel_hzxxb;
typedef int(*_so_cntax_cancel_hzxxb)(struct HandlePlate *, char *, char *);
_so_cntax_cancel_hzxxb							so_cntax_cancel_hzxxb;

////���߷�Ʊһ������//�ӿ�ͣ��
//typedef int(*_so_aisino_get_download_net_invoice_coil)(struct HandlePlate *, char *, char *);
//_so_aisino_get_download_net_invoice_coil		so_aisino_get_download_net_invoice_coil;

//���߷�Ʊ���ò�ѯ
typedef int(*_so_aisino_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_aisino_query_net_invoice_coil				so_aisino_query_net_invoice_coil;
typedef int(*_so_nisec_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_nisec_query_net_invoice_coil				so_nisec_query_net_invoice_coil;
typedef int(*_so_cntax_query_net_invoice_coil)(struct HandlePlate *, char *, char *, char *);
_so_cntax_query_net_invoice_coil				so_cntax_query_net_invoice_coil;
//���߷�Ʊ����
typedef int(*_so_aisino_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_aisino_net_invoice_coil_download_unlock		so_aisino_net_invoice_coil_download_unlock;
typedef int(*_so_nisec_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_nisec_net_invoice_coil_download_unlock		so_nisec_net_invoice_coil_download_unlock;
typedef int(*_so_cntax_net_invoice_coil_download_unlock)(struct HandlePlate *, char *, uint8, char *, char *, int, char *);
_so_cntax_net_invoice_coil_download_unlock		so_cntax_net_invoice_coil_download_unlock;

//������빫������ƽ̨
typedef int(*_so_aisino_connect_pubservice)(struct HandlePlate *, uint8, char *);
_so_aisino_connect_pubservice					so_aisino_connect_pubservice;
typedef int(*_so_nisec_connect_pubservice)(struct HandlePlate *, uint8, char *);
_so_nisec_connect_pubservice					so_nisec_connect_pubservice;

//�������빫������ƽ̨
typedef int(*_so_aisino_disconnect_pubservice)(struct HandlePlate *, uint8, char *);
_so_aisino_disconnect_pubservice				so_aisino_disconnect_pubservice;
typedef int(*_so_nisec_disconnect_pubservice)(struct HandlePlate *, uint8, char *);
_so_nisec_disconnect_pubservice					so_nisec_disconnect_pubservice;

//��������������--hello
typedef int(*_so_aisino_client_hello)(struct HandlePlate *, char *, char *);
_so_aisino_client_hello				so_aisino_client_hello;
typedef int(*_so_nisec_client_hello)(struct HandlePlate *, char *, char *);
_so_nisec_client_hello				so_nisec_client_hello;
typedef int(*_so_cntax_client_hello)(struct HandlePlate *, char *, char *);
_so_cntax_client_hello				so_cntax_client_hello;

//������������֤--auth
typedef int(*_so_aisino_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_aisino_client_auth				so_aisino_client_auth;
typedef int(*_so_nisec_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_nisec_client_auth				so_nisec_client_auth;
typedef int(*_so_cntax_client_auth)(struct HandlePlate *, char *, char *,char *);
_so_cntax_client_auth				so_cntax_client_auth;

//��ȡ˰��֤��
typedef int(*_so_aisino_get_cert)(struct HandlePlate *, char *, char *);
_so_aisino_get_cert				so_aisino_get_cert;
typedef int(*_so_nisec_get_cert)(struct HandlePlate *, char *, char *);
_so_nisec_get_cert				so_nisec_get_cert;
typedef int(*_so_cntax_get_cert)(struct HandlePlate *, char *, char *);
_so_cntax_get_cert				so_cntax_get_cert;

//���»�ȡ˰�̻�����Ϣ�ӿ�
typedef int(*_so_aisino_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_aisino_query_invoice_month_all_data				so_aisino_query_invoice_month_all_data;
typedef int(*_so_nisec_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_nisec_query_invoice_month_all_data				so_nisec_query_invoice_month_all_data;
typedef int(*_so_cntax_query_invoice_month_all_data)(struct HandlePlate *, char *, char **, char *);
_so_cntax_query_invoice_month_all_data				so_cntax_query_invoice_month_all_data;

//����ʷƱ����Ϣ

typedef int(*_so_nisec_read_inv_coil_number_history)(struct HandlePlate *, char **, char *);
_so_nisec_read_inv_coil_number_history				so_nisec_read_inv_coil_number_history;
typedef int(*_so_cntax_read_inv_coil_number_history)(struct HandlePlate *, char **, char *);
_so_cntax_read_inv_coil_number_history				so_cntax_read_inv_coil_number_history;

//��ȡ�ɰط�����key������Ϣ
typedef int(*_so_mb_server_key_info)(struct HandlePlate *, char *);
_so_mb_server_key_info							so_mb_server_key_info;


//ȫ���½
typedef int(*_so_etax_login_info)(char *, char *, char *, char *, char *, char *, char *,char *,char *,char *,char *,char *,char *,int,char *,char *);
_so_etax_login_info						so_etax_login_info;


//ȫ���û���Ϣ��ȡ
typedef int(*_so_etax_user_query_info)(char *, char *,char **, char *, char *,int, char *, char *);
_so_etax_user_query_info				so_etax_user_query_info;

//��ҵ�б�
typedef int(*_so_etax_relationlist_info)(char *, char *, char *, char *, char *, char *, char *,char **, char *, char *,int, char *, char *);
_so_etax_relationlist_info						so_etax_relationlist_info;

//��ҵ�л�
typedef int(*_so_etax_relationchange_info)(char *, char *, char *, char *, char *, char *, char *,char *,char *,char *,char *,char *,char *, char *, char *,int, char *, char *);
_so_etax_relationchange_info						so_etax_relationchange_info;

//ȫ�緢Ʊ��ѯ
typedef int(*_so_etax_invoice_query_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_info				so_etax_invoice_query_info;

//ȫ���ȡcookie
typedef int(*_so_etax_get_cookie_info)(char *,  char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char **,char *,char *,int,char *,char *,char *);
_so_etax_get_cookie_info				so_etax_get_cookie_info;

// typedef int(*_so_etax_get_cookie_info_login)(char *,  char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char **,char *,char *,int,char *,char *,char *);
// _so_etax_get_cookie_info_login				so_etax_get_cookie_info_login;

//ȫ�緢Ʊ����
typedef int(*_so_etax_invoice_query_detail_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_detail_info				so_etax_invoice_query_detail_info;

//ȫ�緢Ʊ����
typedef int(*_so_etax_invoice_query_down_info)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_invoice_query_down_info				so_etax_invoice_query_down_info;

//ȫ��rpaɨ��ӿ�
typedef int(*_so_etax_rpa_invoice_issuance_info)(char *, char *, char *, char *, char *, char *,char *,char *,int, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_rpa_invoice_issuance_info				so_etax_rpa_invoice_issuance_info;

//ȫ�������֤״̬�ӿ�
typedef int(*_so_etax_rpa_invoice_issuance_status_info)(char *,char *, int ,char *, char *, char *, char *, char *,char *,char *,int, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_rpa_invoice_issuance_status_info				so_etax_rpa_invoice_issuance_status_info;

//ȫ�����Ŷ�Ƚӿ�
typedef int(*_so_etax_credit_limit_info)(char *, char *, char *, char *, char *, char *,char *,char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_credit_limit_info			so_etax_credit_limit_info;

//ȫ�����ַ�Ʊ����
typedef int(*_so_etax_blue_ink_invoice_issuance_info)(char *, char *, char *, char *, char *, char *,int ,char *, char *, char *,char **,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_blue_ink_invoice_issuance_info			so_etax_blue_ink_invoice_issuance_info;

//ȫ��ɿ���Ʊ����Ʊ��ѯ
typedef int(*_so_etax_hzqrxxSzzhQuery)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxSzzhQuery				so_etax_hzqrxxSzzhQuery_info;

//ȫ���Ʊȷ�ϵ�����
typedef int(*_so_etax_hzqrxxSave)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxSave				so_etax_hzqrxxSave_info;

//ȫ���Ʊȷ�ϵ��б��ѯ
typedef int(*_so_etax_hzqrxxQuery)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxQuery				so_etax_hzqrxxQuery_info;

//ȫ���Ʊȷ�ϵ������ѯ
typedef int(*_so_etax_hzqrxxQueryDetail)(char *, char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hzqrxxQueryDetail				so_etax_hzqrxxQueryDetail_info;


typedef int(*_so_etax_hcstaquery_info)(char *, char *, char *, char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_hcstaquery_info				so_etax_hcstaquery_info;


typedef int(*_so_etax_fjxxpeizhi_info)(char *, char *, char *, char *,char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_fjxxpeizhi_info				so_etax_fjxxpeizhi_info;


typedef int(*_so_etax_cjmbpeizhi_info)(char *, char *, char *, char *,char *, char *, char *,int, char *, char *, char *,char **,char *, char *, char *, char *, char *, char *, char *,int);
_so_etax_cjmbpeizhi_info				so_etax_cjmbpeizhi_info;

typedef int(*_so_etax_get_nsrjcxx_info)(char *, char *, char *, char *,char *,int, char *, char *, char *,char **,char *, char *, char *, char *, int);
_so_etax_get_nsrjcxx_info				so_etax_get_nsrjcxx_info;

typedef int(*_so_etax_get_nsrfxxx_info)(char *, char *, char *, char *,char *,int, char *, char *, char *,char **,char *, char *, char *, char *, int);
_so_etax_get_nsrfxxx_info				so_etax_get_nsrfxxx_info;

#endif 

/*======================================ȫ�纯���ӿ�================================================*/

//ȫ���½
int mbi_etax_login_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//ȫ���û���Ϣ��ȡ
int mbi_etax_user_query_Interface(int port,char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//��ҵ�б��ѯ
int mbi_etax_relationlist_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//��ҵ�л�
int mbi_etax_relationchange_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code);

//ȫ�緢Ʊ��ѯ
//int mbi_etax_invoice_query_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout);
int mbi_etax_invoice_query_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_get_cookie_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);

// int mbi_etax_get_cookie_Interface_login(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data);


// int mbi_etax_invoice_query_detail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout);
int mbi_etax_invoice_query_detail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

// int mbi_etax_invoice_query_down_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname);
int mbi_etax_invoice_query_down_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

// int mbi_etax_rpa_invoice_issuance_Interface(int port,char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata);
int mbi_etax_rpa_invoice_issuance_Interface(int port,char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_rpa_invoice_issuance_status_Interface(int port,char *busid,char *rzid,char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);


// int mbi_etax_credit_limit_Interface(int port,char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata);
int mbi_etax_credit_limit_Interface(int port,char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_blue_ink_invoice_issuance_Interface(int port,char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxSzzhQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxSave_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hzqrxxQueryDetail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_hcstaquery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_fjxxpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);

int mbi_etax_cjmbpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie);
int mbi_etax_get_nsrjcxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie);
int mbi_etax_get_nsrfxxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie);

/*======================================��������================================================*/
e_ty_usb_m  int ty_usb_m_add(const char *usb_name, int dev_type);
e_ty_usb_m int open_port_attach(int port, uint32 ip, char *client, int *port_fd);
e_ty_usb_m int close_port_dettach(int port);
e_ty_usb_m int get_usb_busid(int port, char*busid);
e_ty_usb_m int change_port_gw_state(struct _new_usbshare_parm* parm, int port);
e_ty_usb_m int change_port_power_quandian(int port,char *inbusid);
e_ty_usb_m int change_all_port_power_mode(int mode);
e_ty_usb_m int get_reg_ble_name(char *ble_name);
e_ty_usb_m int get_module_state(void);
e_ty_usb_m int get_transport_line(struct _new_usbshare_parm *parm);
//***TCP�ӿ�****//
//���»�ȡ��Ʊ����JSON����
e_ty_usb_m int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info);
e_ty_usb_m int set_port_usb_data_tran(int port);
//���ݷ�Ʊ��������ѯ��Ʊ��Ϣ
e_ty_usb_m int fun_get_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, char **inv_data, int dzsyh, char *errinfo);
e_ty_usb_m int fun_get_inv_from_fpdm_fphm_get_bin_str(char *plate_num, char *fpdm, char *fphm, int dzsyh, char **inv_data, char **kp_bin, char **kp_str, char *errinfo);
//���²�ѯ��ѯ��Ʊ����
e_ty_usb_m int fun_get_palte_month_inv_num(char *plate_num, char *month, unsigned int *fp_num, unsigned long *fp_len, char *errinfo);
//���²�ѯ��Ʊ��ϸ��Ϣ
e_ty_usb_m int fun_get_invoice_detail_mqtt(char *plate_num, unsigned char *month, void * function, void * arg, char *errinfo, int old_new, int sfxqqd);
//ֹͣ���ڲ�ѯ�ķ�Ʊ
e_ty_usb_m int fun_stop_invoice_query(char *plate_num, char *errinfo);
e_ty_usb_m int get_usb_vid_pid(int port, uint16 *vid, uint16 *pid);

e_ty_usb_m int fun_deal_all_port_report(void);

//***MQTT�ӿ�****//
//�̺Ż�ȡUSB�����Ϣ

//�̺Ż�ȡ˰��λ��
e_ty_usb_m int fun_get_plate_usb_port(unsigned char *plate_num);
//��ȡ˰�̵�ǰʱ��
e_ty_usb_m int fun_get_plate_now_time(unsigned char *plate_num, char *time);
//��Ʊ����
e_ty_usb_m int fun_make_invoice_to_plate(unsigned char *plate_num, int inv_type, unsigned char *inv_data, struct _make_invoice_result *make_invoice_result);
//��Ʊ����
e_ty_usb_m int fun_cancel_invoice_to_plate(unsigned char *plate_num, int inv_type, int zflx, unsigned char *fpdm, unsigned char *fphm, unsigned char *zfr, struct _cancel_invoice_result *cancel_invoice_result);
//�����ϱ�Ƶ��
e_ty_usb_m int function_set_report_time(int report_time);
//��֤֤����Կ�Ƿ���ȷ
e_ty_usb_m int fun_check_cert_passwd(unsigned char *plate_num, unsigned char *cert_passwd);
//��ȡ��ǰ��Ʊ�������
e_ty_usb_m int fun_check_now_fpdm_fphm(unsigned char *plate_num, int inv_type, char *fpdm, char *fphm, char *errinfo);
//ִ�г����忨�����ؼ����Ϣ����
e_ty_usb_m int fun_cb_qingka_get_usb_info(unsigned char *plate_num, struct _usb_port_infor *port_info);
//����Ʊ��������ѯ���ϴ���Ʊ
e_ty_usb_m int fun_update_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, int dzsyh, char **inv_data, char *errinfo);
//���������Ϣ��
e_ty_usb_m int fun_upload_hzxxb(char *plate_num,uint8 int_type, char* inv_data, struct _askfor_tzdbh_result *askfor_tzdbh_result);
//��Ʊ����ǰ����ƱУ��
e_ty_usb_m int fun_check_lzfpdm_lzfphm_allow(char *plate_num, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo);
//ͨ��������Ϣ���ѯ����ĺ�Ʊ��Ϣ
e_ty_usb_m int fun_get_redinv_online_from_rednum(char *plate_num, char *rednum, uint8 **inv_json, char *errinfo);
////��Ʊ����//ͣ��
//e_ty_usb_m int fun_get_download_inv_coils(char *plate_num, char *date_range, char *errinfo);
//֪ͨ˰���ϴ��·�Ʊ
e_ty_usb_m int fun_notice_plate_update_inv_to_mserver(char *plate_num, char *month, char * summary_month,char *errinfo);
//�޸�֤�����
e_ty_usb_m int fun_change_cert_passwd(char *plate_num, char *old_passwd, char *new_passwd, char *errinfo);
//����������Ϣ������
e_ty_usb_m int fun_cancel_hzxxb_online(char *plate_num, char *rednum, char *errinfo);
//��ʱ��β�ѯ������Ϣ��
e_ty_usb_m int fun_query_rednum_from_date_range(char *plate_num, char *date_range, uint8 **rednum_data, int *rednum_count, char *errinfo);
e_ty_usb_m int fun_query_rednum_from_date_range_new(char *plate_num, char *date_range,char *gfsh,uint8 **rednum_data, int *rednum_count,char *errinfo);
//��ѯ��Ȩ��Ϣ
e_ty_usb_m int fun_query_auth_file(char **auth_file);
//��Ʊ���ò�ѯ
e_ty_usb_m int fun_query_net_inv_coils(char *plate_num, char *date_range, char *inv_data, char *errinfo);
//��Ʊ��������
e_ty_usb_m int fun_net_inv_coils_download(char *plate_num, char *date_range, uint8 fplxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo);
//������������
e_ty_usb_m int fun_test_plate_server_connect(char *plate_num,char **splxxx, char *errinfo);
//����USB�˿ڵ�Դ
e_ty_usb_m int fun_deal_usb_port_power(int usb_port, int power_state, char *errinfo);
//��ȡ��汾��
e_ty_usb_m int fun_deal_get_so_version(char *tax_ver, char *mbc_ver,char *tax_load, char *auth_load);
//���֤�������󻺴�
e_ty_usb_m int fun_deal_clear_cert_err(char *plate_num, int mode);
//��ȡ��Ȩ��Կ
e_ty_usb_m int fun_auth_key_info(char *plate_num, char *key_info, char *errinfo);
//���빫������ƽ̨
e_ty_usb_m int fun_connect_pubservice(char *plate_num, uint8 inv_type, char *errinfo);
//ȡ�����빫������ƽ̨
e_ty_usb_m int fun_disconnect_pubservice(char *plate_num, uint8 inv_type, char *errinfo);
//��������������--hello
e_ty_usb_m int fun_client_hello(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag);
//������������֤--auth
e_ty_usb_m int fun_client_auth(char *plate_num, char* server_hello, char *client_auth, char *errinfo);

//��������������--hello
e_ty_usb_m int fun_client_hello_d(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag);
//������������֤--auth
e_ty_usb_m int fun_client_auth_d(char *plate_num, char* server_hello, char *client_auth, char *errinfo);

//��ֵ˰��Ʊ�ۺϷ���ƽ̨����͸��
e_ty_usb_m int fun_zzsfpzhfwpt_auth_hello_data_tran(char *plate_num, char *https_data, int https_size);
//���»�ȡ˰�̻�����Ϣ�ӿ�
e_ty_usb_m int fun_query_invoice_month_all_data(char *plate_num, char *month, char **data_json, char *errinfo);
//��ʷ��Ʊ��Ϣ��ѯ
e_ty_usb_m int fun_query_history_invoice_coils(char *plate_num, char **data_json, char *errinfo);
//��ȡ����ȫ����Ϣjson����
e_ty_usb_m int fun_get_ter_all_data_json(cJSON *data_layer);
//��ȡ������˿���ȫ����Ϣjson����
e_ty_usb_m int fun_get_ter_all_data_json_without_port(cJSON *data_layer);
//��ȡ�����˰����Ϣ��ȫ����Ϣjson����
e_ty_usb_m int fun_get_ter_all_data_json_without_tax(cJSON *data_layer);
//��ȡ���������Ϣjson����
e_ty_usb_m int fun_get_ter_base_data_json(cJSON *data_layer);
//��ȡ˰����Ϣjson����
e_ty_usb_m int fun_get_port_data_json(cJSON *data_layer, int port,int is_common);
//�л��˿ڽڵ�ģʽ
e_ty_usb_m int change_port_power_mode(int port, int mode);
//��ȡmqtt����MD5��Ϣ
e_ty_usb_m int function_get_mqtt_connect_data_md5_jude_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type);
e_ty_usb_m int function_get_mqtt_connect_data_md5_jude_update_1201(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type);
//��ȡmqtt����֤����Ϣ
e_ty_usb_m int function_get_mqtt_connect_data_info_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd, int mqtt_type);
//������Ȩ�ļ�
e_ty_usb_m int fun_upload_auth_cert_file(void);
//ֹͣ������Ʊ�ϴ�����
e_ty_usb_m int fun_stop_plate_upload_offline_invoice(char *plate_num,int mode, char *errinfo);

#ifdef _ty_usb_m_c
static int ty_usb_m_open(struct ty_file	*file);
static int ty_usb_m_ctl(void *data, int fd, int cm, va_list args);
static int ty_usb_m_close(void *data, int fd);

static void pack_report_m_server_status_data_linux64(void *arg);

static int jude_port_auth_passwd(struct _port_state   *state,char *errinfo);
static int update_tax_business_info(struct _port_state   *state,char *use_info ,int if_defer_tax,int result ,char *errinfo);
static void task_port_led_manage(void *arg, int timer);
static int update_plate_info_cbqkzt(struct _port_state   *state);
static int close_port_socket(struct _ty_usb_m *stream, int port);
static int open_port(struct _ty_usb_m  *stream, int port, uint32 ip, uint8 *client_id);
static int open_port_ty_usb(struct _ty_usb_m  *stream, int port);
static int close_port_ty_usb(struct _ty_usb_m  *stream, int port);
static int check_stop_upload(int *stop_aisino_up, int *stop_nisec_up, int *stop_cntax_up, int *stop_mengbai_up);
static int timer_read_now_time_add_hour_asc(int hour, uint8 *timer_asc);
static void task_m(void *data, int timer);
static void task_check_port_repair_51err(void *arg, int timer);
static void task_check_port(void *arg, int timer);
static void reset_systerm(void *arg);

static int all_port_power_open(struct _ty_usb_m  *stream);
static int check_port(struct _port_state   *state, struct _ty_usb_m  *stream);
static int check_port_used_time(struct _port_state   *state);
static void deal_plate_upload_m_server_queue(void *arg);


static int check_port_auth_info(struct _ty_usb_m  *stream);

static int common_close_usb_device(struct _port_state   *state,struct HandlePlate *h_plate);
static int find_usb_port_by_plate_num(struct _ty_usb_m  *stream, unsigned char *palte_num);
#ifdef RELEASE_SO
static int load_tax_lib_so(unsigned char *key_file);
static int load_tax_lib_so_fun(void);
static int load_tax_lib_so_fun_wang(void);
#endif
static uint16 crc16_MAXIM(uint8 *addr, int num);

#ifdef PRODUCE_PROGRAM
static void task_read_port_base_tax_info_simple(void *arg, int timer);
#endif

static int get_dev_passwd_err_port(int port, int *left_num);
static int get_cert_passwd_err_port(char *plate_num, int *left_num);
static void update_dev_passwd_err_port(int port, int left_num);
static void update_cert_passwd_err_port(char *plate_num,int left_num);

static int vid_pid_jude_plate(uint16 vid, uint16 pid);

static void timely_tigger_report_status(void *arg, int timer);
static void pack_report_json_data(void *arg);
static void pack_report_m_server_heart_data(void *arg);
static void pack_report_m_server_status_data(void *arg);

static void timely_tigger_report_port_status(void *arg, int timer);


static void pack_request_upload_inv_num(void *arg);
static int analysis_get_inv_num_response(struct _port_state   *state, char *json_data);

static void timely_tigger_deal_plate(void *arg, int timer);
#ifndef DF_OLD_MSERVER
static int pack_report_plate_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
static int pack_report_other_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
#ifdef ALL_SHARE
static int pack_report_ca_info_json(struct _port_state   *state, int off_up_flag,char **g_buf);
#endif

static int single_inv_upload(struct _port_state   *state, char *scfpsj, char *kpsj);

static int analysis_ports_report_response(struct _port_state   *state, char *json_data);
static void timely_tigger_get_plate_invoice_db_info(void *arg, int timer);
static int mqtt_pub_hook(void *arg, char *month, char *s_data,int inv_sum);
#else
static int mengbai_plate_inv_upload(struct _port_state   *state, char *scfpsj, char *kpsj);
static int pack_report_plate_info_json(char **g_buf, int off_up_flag, char *random, struct _port_state   *state);
static void pack_report_plate_info(void *arg);
static void pack_report_other_info(void *arg);
static int mqtt_pub_hook(unsigned char *s_data, int total_num, int now_count, void *arg, char *plate_num, unsigned char *month,char *statistics,int complete_err);
#endif

static int judge_plate_allow_kp(struct _port_state   *state, int inv_type, char *inv_data, char *errinfo);

static int inv_change_notice_upload(struct _port_state   *state, int act_type, char *fpdm, char *fphm, char *scfpsj, char *kpsj);

static int make_invoice_ok_update_monitor_status(struct _port_state   *state, int read_flag);
static int make_invoice_ok_update_monitor_status_and_notice_server(struct _port_state   *state, struct _plate_infos *plate_infos);
static void report_offline_inv_to_server(void *arg, int timer);
static void auto_cb_jkxxhc_thread(void *arg, int timer);
static void timely_tigger_auto_cb_jkxxhc(void *arg, int timer);
//static void timely_tigger_sync_plate_cpy_info(void *arg, int timer);
static void get_offline_inv_and_upload_aisino(void *arg);
static void get_offline_inv_and_upload_nisec(void *arg);
static void get_offline_inv_and_upload_cntax(void *arg);
static void get_offline_inv_and_upload_mengbai(void *arg);
static int mengbai_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int nisec_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int cntax_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);
static int aisino_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l);

static int waiting_for_plate_install(uint8 *plate_num, struct _port_state   *state, int outtimes);
static int waiting_for_plate_uninstall(uint8 *plate_num, struct _port_state   *state, int outtimes);
static int restart_plate_power(struct _port_state   *state, uint8 *plate_num, int port, int wait_install);
static int plate_useinfo_errinfo(struct _port_state   *state, char *errinfo);
#ifndef ALL_SHARE
static int jude_port_allow_attach(struct _port_state  *state);
#endif
static int jude_port_led_auth(struct _port_state  *state);
static int port_lock(struct _port_state   *state, int second, int used_level, char *used_info);
static int port_unlock(struct _port_state   *state);

static void task_read_port_base_tax_info(void *arg, int timer);
static void deal_read_plate_base_tax_info(void *arg);




static void task_check_port_ad_status(void *arg, int timer);
static void task_port_power_up_down(void *arg,int timer);
static void task_port_repair_ad_error(void *arg, int timer);
static void task_check_power_down_dev_remove(void *arg, int timer);
static int free_h_plate(struct _port_state   *state);
static int port_power_up(struct _port_state   *state);
static void port_power_down(struct _port_state   *state);
#endif   


struct _port_used_level_info
{
	int level;
	char info[200];
};

/*usb-share������*///һ��
#define DF_PORT_USED_USB_SHARE		"usb-share����"
/*˰�̡�ca������Ϣ��ȡ��*///����
#define DF_PORT_USED_DQMCSHXX		"��ȡ����˰�ŵȻ�����Ϣ"
/*˰�̶�д������*///����
#define DF_PORT_USED_FPKJ			"˰�̷�Ʊ����"
#define DF_PORT_USED_FPZF			"˰�̷�Ʊ����"
#define DF_PORT_USED_CXDQFPHM		"����Ʊ���Ͳ�ѯ��ǰ��Ʊ�������"
#define DF_PORT_USED_MRKLRZ			"Ĭ�Ͽ�����֤���޸�"
#define DF_PORT_USED_HQSPDQSJ		"��ȡ˰�̵�ǰʱ��"
#define DF_PORT_USED_HQFWSJMXX		"��ȡ�ɰط�����key������Ϣ"
#define DF_PORT_USED_FWQLJWS		"��������������"
#define DF_PORT_USED_FWQLJRZ		"������������֤"
#define DF_PORT_USBD_QD_LOCK         "ȫ��ӿڵ���"
/*�漰�ֶ˽����ӿ�--��ʱ*///����
#define DF_PORT_USED_LXSC			"��ȡ���߷�Ʊ���ϴ��ֶ�"
#define DF_PORT_USED_HZSC			"��˰�������ϴ�"
#define DF_PORT_USED_FXJK			"��д�����Ϣ"
#define DF_PORT_USED_FPHM_SCFP		"����Ʊ��������ϴ���Ʊ"
#define DF_PORT_USED_FPYJCXBLJ		"��Ʊһ����ѯ������"
#define DF_PORT_USED_DLYFPCX		"�����÷�Ʊ��ѯ"
#define DF_PORT_USED_DLYFPXZAZ		"�����÷�Ʊ���ذ�װ"
#define DF_PORT_USED_HPKJLZHMJY		"��Ʊ����ǰ����ƱУ��"
#define DF_PORT_USED_SQHZXXB		"���������Ϣ��"
#define DF_PORT_USED_CXHZXXB		"����������Ϣ��"
#define DF_PORT_USED_ASJDCXHZXXB	"��ʱ��β�ѯ������Ϣ����"
#define DF_PORT_USED_AXXBBHCXHZXXB	"����Ϣ���Ų�ѯ������Ϣ����"
#define DF_PORT_USED_CSLJ			"���������������"
#define DF_PORT_USED_SQJRGGFWPT		"������빫������ƽ̨"
#define DF_PORT_USED_CXJRGGFWPT		"�������빫������ƽ̨"
/*˰��ֻ��������--��ʱ*///����
#define DF_PORT_USED_AYCXFPSLMQTT	"���²�ѯ��Ʊ����MQTT�ӿ�"
#define DF_PORT_USED_AYCXFPSJMQTT	"���²�ѯ��Ʊ����MQTT����"
#define DF_PORT_USED_AFPHMCXFP		"����Ʊ��������ѯ��Ʊ����"
#define DF_PORT_USED_CXHZSJ			"��ѯ��������"
/*˰��ֻ�����ϴ��ɰ���--��ʱ*///����
#define DF_PORT_USED_FPCX_SCMB		"˰�̷�Ʊ��ѯ���ϴ�M_SERVER"

static const struct _port_used_level_info used_info[] =
{
	/*usb-share������*/
	{ 1, DF_PORT_USED_USB_SHARE },
	/*˰�̡�ca������Ϣ��ȡ��*/
	{ 2, DF_PORT_USED_DQMCSHXX },
	/*˰�̶�д������*/
	{ 3, DF_PORT_USED_FPKJ },
	{ 3, DF_PORT_USED_FPZF },
	{ 3, DF_PORT_USED_CXDQFPHM },
	{ 3, DF_PORT_USED_MRKLRZ },
	{ 3, DF_PORT_USED_HQSPDQSJ },
	{ 3, DF_PORT_USED_HQFWSJMXX },
	{ 3, DF_PORT_USED_FWQLJWS },
	{ 3, DF_PORT_USED_FWQLJRZ },
	{ 3, DF_PORT_USBD_QD_LOCK },
	/*�漰�ֶ˽����ӿ�*/
	{ 4, DF_PORT_USED_LXSC },
	{ 4, DF_PORT_USED_HZSC },
	{ 4, DF_PORT_USED_FXJK },
	{ 4, DF_PORT_USED_FPHM_SCFP },
	{ 4, DF_PORT_USED_FPYJCXBLJ },
	{ 4, DF_PORT_USED_DLYFPCX },
	{ 4, DF_PORT_USED_DLYFPXZAZ },
	{ 4, DF_PORT_USED_HPKJLZHMJY },
	{ 4, DF_PORT_USED_SQHZXXB },
	{ 4, DF_PORT_USED_CXHZXXB },
	{ 4, DF_PORT_USED_ASJDCXHZXXB },
	{ 4, DF_PORT_USED_AXXBBHCXHZXXB },
	{ 4, DF_PORT_USED_CSLJ },
	{ 4, DF_PORT_USED_SQJRGGFWPT },
	{ 4, DF_PORT_USED_CXJRGGFWPT },
	/*˰��ֻ��������*/
	{ 5, DF_PORT_USED_AYCXFPSLMQTT },
	{ 5, DF_PORT_USED_AYCXFPSJMQTT },
	{ 5, DF_PORT_USED_AFPHMCXFP },
	{ 5, DF_PORT_USED_CXHZSJ },
	/*˰��ֻ�����ϴ��ɰ���*/
	{ 6, DF_PORT_USED_FPCX_SCMB }
};


	struct _plate_function
	{
		char *usb_app;		//˰�����ͣ�1��˰�� 2˰���� 3ģ����  101˰��ukey
		char *plate_type;	//˰����������
		char *jcxxdq;		//������Ϣ��ȡ
		char *jkxxdq;		//�����Ϣ��ȡ
		char *kcxxdq;		//�����Ϣ��ȡ
		char *cxspdqsj;		//��ѯ˰�̵�ǰʱ��
		char *cxdqfpdmhm;	//��ѯ��ǰ��Ʊ�������
		char *aycxykfpsj;	//���²�ѯ�ѿ���Ʊ����
		char *aycxykfpzs;	//���²�ѯ�ѿ���Ʊ����
		char *afpdmhmcxykfp;//����Ʊ��������ѯ�ѿ���Ʊ
		char *yzzskl;		//��֤֤�����
		char *xgzskl;		//�޸�֤�����
		char *zsfpkj;		//������Ʊ����
		char *fsfpkj;		//������Ʊ����
		char *ykzsfpzf;		//�ѿ�������Ʊ����
		char *ykfsfpzf;		//�ѿ�������Ʊ����
		char *wkfpzf;		//δ����Ʊ����
		char *yqsbfpzf;		//��ǩʧ�ܷ�Ʊ����
		char *asjdcxhzxxb;	//��ʱ��β�ѯ������Ϣ��
		char *axxbbhcxhzxxb;//����Ϣ���Ų�ѯ������Ϣ��
		char *xfsqhzxxb;	//�������������Ϣ��
		char *gfsqhzxxb;	//�������������Ϣ��
		char *cxhzxxb;		//����������Ϣ��
		char *zdcshz;		//�Զ���˰����
		char *zdqk;			//�Զ��忨
		char *lxfpzdsc;		//���߷�Ʊ�Զ��ϴ�
		char *afpdmhmscfp;	//����Ʊ��������ϴ���Ʊ
		char *csspyfwqlj;	//����˰�������������
		char *dlyfpcx;		//�����÷�Ʊ��ѯ
		char *dlyfpxzaz;	//�����÷�Ʊ���ذ�װ
		char *jrggfwpt;		//���빫������ƽ̨
		char *qxggfwpt;		//ȡ����������ƽ̨
	};

	static const struct _plate_function plate_function[] =
	{
		{
			.usb_app = "1",//˰�����ͣ�1��˰�� 2˰���� 3ģ����  101˰��ukey
			.plate_type = "��˰��",//˰����������
			.jcxxdq = "1",//������Ϣ��ȡ
			.jkxxdq = "1",//�����Ϣ��ȡ
			.kcxxdq = "1",//�����Ϣ��ȡ
			.cxspdqsj = "1",//��ѯ˰�̵�ǰʱ��
			.cxdqfpdmhm = "1",//��ѯ��ǰ��Ʊ�������
			.aycxykfpsj = "1",//���²�ѯ�ѿ���Ʊ����
			.aycxykfpzs = "1",//���²�ѯ�ѿ���Ʊ����
			.afpdmhmcxykfp = "1",//����Ʊ��������ѯ�ѿ���Ʊ
			.yzzskl = "1",//��֤֤�����
			.xgzskl = "1",//�޸�֤�����
			.zsfpkj = "1",//������Ʊ����
			.fsfpkj = "1",//������Ʊ����
			.ykzsfpzf = "1",//�ѿ�������Ʊ����
			.ykfsfpzf = "1",//�ѿ�������Ʊ����
			.wkfpzf = "1",//δ����Ʊ����
			.yqsbfpzf = "0",//��ǩʧ�ܷ�Ʊ����
			.asjdcxhzxxb = "1",//��ʱ��β�ѯ������Ϣ��
			.axxbbhcxhzxxb = "1",//����Ϣ���Ų�ѯ������Ϣ��
			.xfsqhzxxb = "1",//�������������Ϣ��
			.gfsqhzxxb = "1",//�������������Ϣ��
			.cxhzxxb = "1",//����������Ϣ��
			.zdcshz = "1",//�Զ���˰����
			.zdqk = "1",//�Զ��忨
			.lxfpzdsc = "1",//���߷�Ʊ�Զ��ϴ�
			.afpdmhmscfp = "1",//����Ʊ��������ϴ���Ʊ
			.csspyfwqlj = "1",//����˰�������������
			.dlyfpcx = "1",//�����÷�Ʊ��ѯ
			.dlyfpxzaz = "1",//�����÷�Ʊ���ذ�װ
			.jrggfwpt = "1",//���빫������ƽ̨
			.qxggfwpt = "1",//ȡ����������ƽ̨

		},
		{
			.usb_app = "2",//˰�����ͣ�1��˰�� 2˰���� 3ģ����  101˰��ukey
			.plate_type = "˰����",//˰����������
			.jcxxdq = "1",//������Ϣ��ȡ
			.jkxxdq = "1",//�����Ϣ��ȡ
			.kcxxdq = "1",//�����Ϣ��ȡ
			.cxspdqsj = "1",//��ѯ˰�̵�ǰʱ��
			.cxdqfpdmhm = "1",//��ѯ��ǰ��Ʊ�������
			.aycxykfpsj = "1",//���²�ѯ�ѿ���Ʊ����
			.aycxykfpzs = "1",//���²�ѯ�ѿ���Ʊ����
			.afpdmhmcxykfp = "1",//����Ʊ��������ѯ�ѿ���Ʊ
			.yzzskl = "1",//��֤֤�����
			.xgzskl = "1",//�޸�֤�����
			.zsfpkj = "1",//������Ʊ����
			.fsfpkj = "1",//������Ʊ����
			.ykzsfpzf = "1",//�ѿ�������Ʊ����
			.ykfsfpzf = "1",//�ѿ�������Ʊ����
			.wkfpzf = "1",//δ����Ʊ����
			.yqsbfpzf = "0",//��ǩʧ�ܷ�Ʊ����
			.asjdcxhzxxb = "1",//��ʱ��β�ѯ������Ϣ��
			.axxbbhcxhzxxb = "1",//����Ϣ���Ų�ѯ������Ϣ��
			.xfsqhzxxb = "1",//�������������Ϣ��
			.gfsqhzxxb = "1",//�������������Ϣ��
			.cxhzxxb = "1",//����������Ϣ��
			.zdcshz = "1",//�Զ���˰����
			.zdqk = "1",//�Զ��忨
			.lxfpzdsc = "1",//���߷�Ʊ�Զ��ϴ�
			.afpdmhmscfp = "1",//����Ʊ��������ϴ���Ʊ
			.csspyfwqlj = "1",//����˰�������������
			.dlyfpcx = "1",//�����÷�Ʊ��ѯ
			.dlyfpxzaz = "1",//�����÷�Ʊ���ذ�װ
			.jrggfwpt = "1",//���빫������ƽ̨
			.qxggfwpt = "1",//ȡ����������ƽ̨
		},
		{
			.usb_app = "3",//˰�����ͣ�1��˰�� 2˰���� 3ģ����  101˰��ukey
			.plate_type = "ģ����",//˰����������
			.jcxxdq = "1",//������Ϣ��ȡ
			.jkxxdq = "1",//�����Ϣ��ȡ
			.kcxxdq = "1",//�����Ϣ��ȡ
			.cxspdqsj = "1",//��ѯ˰�̵�ǰʱ��
			.cxdqfpdmhm = "1",//��ѯ��ǰ��Ʊ�������
			.aycxykfpsj = "1",//���²�ѯ�ѿ���Ʊ����
			.aycxykfpzs = "1",//���²�ѯ�ѿ���Ʊ����
			.afpdmhmcxykfp = "0",//����Ʊ��������ѯ�ѿ���Ʊ
			.yzzskl = "1",//��֤֤�����
			.xgzskl = "1",//�޸�֤�����
			.zsfpkj = "1",//������Ʊ����
			.fsfpkj = "1",//������Ʊ����
			.ykzsfpzf = "1",//�ѿ�������Ʊ����
			.ykfsfpzf = "1",//�ѿ�������Ʊ����
			.wkfpzf = "1",//δ����Ʊ����
			.yqsbfpzf = "0",//��ǩʧ�ܷ�Ʊ����
			.asjdcxhzxxb = "0",//��ʱ��β�ѯ������Ϣ��
			.axxbbhcxhzxxb = "0",//����Ϣ���Ų�ѯ������Ϣ��
			.xfsqhzxxb = "0",//�������������Ϣ��
			.gfsqhzxxb = "0",//�������������Ϣ��
			.cxhzxxb = "0",//����������Ϣ��
			.zdcshz = "1",//�Զ���˰����
			.zdqk = "1",//�Զ��忨
			.lxfpzdsc = "1",//���߷�Ʊ�Զ��ϴ�
			.afpdmhmscfp = "0",//����Ʊ��������ϴ���Ʊ
			.csspyfwqlj = "1",//����˰�������������
			.dlyfpcx = "0",//�����÷�Ʊ��ѯ
			.dlyfpxzaz = "0",//�����÷�Ʊ���ذ�װ
			.jrggfwpt = "0",//���빫������ƽ̨
			.qxggfwpt = "0",//ȡ����������ƽ̨
		},
		{
			.usb_app = "101",//˰�����ͣ�1��˰�� 2˰���� 3ģ����  101˰��ukey
			.plate_type = "˰��UKEY",//˰����������
			.jcxxdq = "1",//������Ϣ��ȡ
			.jkxxdq = "1",//�����Ϣ��ȡ
			.kcxxdq = "1",//�����Ϣ��ȡ
			.cxspdqsj = "1",//��ѯ˰�̵�ǰʱ��
			.cxdqfpdmhm = "1",//��ѯ��ǰ��Ʊ�������
			.aycxykfpsj = "1",//���²�ѯ�ѿ���Ʊ����
			.aycxykfpzs = "1",//���²�ѯ�ѿ���Ʊ����
			.afpdmhmcxykfp = "1",//����Ʊ��������ѯ�ѿ���Ʊ
			.yzzskl = "1",//��֤֤�����
			.xgzskl = "1",//�޸�֤�����
			.zsfpkj = "1",//������Ʊ����
			.fsfpkj = "1",//������Ʊ����
			.ykzsfpzf = "1",//�ѿ�������Ʊ����
			.ykfsfpzf = "1",//�ѿ�������Ʊ����
			.wkfpzf = "1",//δ����Ʊ����
			.yqsbfpzf = "0",//��ǩʧ�ܷ�Ʊ����
			.asjdcxhzxxb = "1",//��ʱ��β�ѯ������Ϣ��
			.axxbbhcxhzxxb = "1",//����Ϣ���Ų�ѯ������Ϣ��
			.xfsqhzxxb = "1",//�������������Ϣ��
			.gfsqhzxxb = "1",//�������������Ϣ��
			.cxhzxxb = "1",//����������Ϣ��
			.zdcshz = "1",//�Զ���˰����
			.zdqk = "1",//�Զ��忨
			.lxfpzdsc = "1",//���߷�Ʊ�Զ��ϴ�
			.afpdmhmscfp = "1",//����Ʊ��������ϴ���Ʊ
			.csspyfwqlj = "1",//����˰�������������
			.dlyfpcx = "1",//�����÷�Ʊ��ѯ
			.dlyfpxzaz = "1",//�����÷�Ʊ���ذ�װ
			.jrggfwpt = "1",//���빫������ƽ̨
			.qxggfwpt = "1",//ȡ����������ƽ̨
		}
	};


/*======================================��������===============================================*/
#ifdef _ty_usb_m_c
static const struct _ty_usb_m_ctl_fu ctl_fun[] = {
};
static const struct _file_fuc	ty_usb_m_fuc =
{ .open = ty_usb_m_open,
.read = NULL,
.write = NULL,
.ctl = ty_usb_m_ctl,
.close = ty_usb_m_close,
.del = NULL
};


#endif



#endif
