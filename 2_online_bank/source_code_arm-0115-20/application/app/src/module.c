#include "../inc/module.h"

MODULE module;

struct _message_callback message_callback;
struct _message_history	message_history;

// struct _linux_s linux_s;

static int _app_order(int fd,uint32 ip,uint16 port,void *arg);
void recvSignal(int sig)  ;
static int set_start_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len); 
static int set_ip_ip(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_timer(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_update_address(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_tryout_date(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_vpn_server_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int set_close_update(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int get_sev_pro(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_sev_status(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_port_pro(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_sev_par(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_check_all_port(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_all_port_status(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_all_port_ca_name(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_all_port_type(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_ter_soft_version(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int get_vpn_server_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);		
static int get_close_update(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int get_area_code(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int get_tax_infos(int fd,uint32 ip,MODULE	*par,uint8 *buf,int buf_len);
static int get_version_describe(int fd,uint32 ip,MODULE	*par,uint8 *buf,int buf_len);
static int open_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int close_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int open_port_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int close_port_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int close_port_now(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int power_off(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int heart(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int reset(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int regdit(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len);
static int open_port_no_active(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len);
static int get_log(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int get_net_par(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int keep_open_port(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int check_version_updata(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int close_port_by_dettach(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int get_usb_dev_info(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int get_update_address(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len);
static int answer_no_order(int fd,MODULE    *par,uint8 *buf,int buf_len);
static int err_out_tcp(int fd,int err);
static int err_out_s(int fd,uint8 err,uint8 *name);
static int write_ty1502_order(int fd,int order,uint8 *buf,int buf_len);
static int unpack(int fd,uint8 *out_buf,int *out_len);
static int ty_socket_write(int sock,uint8 *buf,int buf_len);
static int ty_socket_read(int sock,uint8 *buf,int buf_len);
// void update_task_hook(void *arg, int timer);
//////////////////////UDP////////////////////////////
static void udp_ser_main_task(void *arg,int o_timer);
static int udp_unpack(uint8 *in_buf,int in_len,uint8 *out_buf,int *out_len);
static int udp_get_sev_pro(int fd,MODULE  *par,uint8 *buf,int buf_len);
static int udp_write_order(int fd,int order,uint8 *buf,int buf_len);
static int udp_order_sev_restart(int fd,MODULE  *par,uint8 *buf,int buf_len);
static int Key_Check_Http(struct http_parm * parm);
static int check_port_status(MODULE  *par);
static int udp_unpack_new_json(uint8 *in_buf, int in_len);
static int udp_get_sev_pro_new(int fd, MODULE  *par);
//////////////////////MQTT//////////////////////////
static void task_deal_message_queue(void *arg, int o_timer);
static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num);
static int find_available_queue_mem(MODULE     *stream);
static int find_deal_queue_mem(MODULE     *stream);

static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum);
static int analysis_json_head(char *inbuf, struct mqtt_parm *parm);
static int mqtt_response_errno(int errnum, struct mqtt_parm *parm, char *errinfo);
static void deal_with_cmd(MODULE *stream, int mem_i, void *fucntion);

//�����������ѯ������Ϣ
static void analysis_request_cxdlxx(void *arg);
//�����������ѯ��ʷ������
static void analysis_request_cxlscljg(void *arg);
//�����������������쳣����
static void analysis_request_cmd_not_support(void *arg);
//�����������ѯ�ն�ȫ����Ϣ
static void analysis_request_cxzdqbxx(void *arg);
//�����������ѯ�ն˻�����Ϣ
static void analysis_request_cxzdjcxx(void *arg);
//֪ͨ�ն˻�ȡ��������Ȩ�ļ�
static void analysis_request_tzzdsjsq(void *arg);
//��Ȩ�ն˵Ķ˿�
static void analysis_request_sqzddk(void *arg);
//�����������ѯ�ն���־�ļ�
static void analysis_request_cxzddqrzxx(void *arg);
//��������������άͨ��
static void analysis_request_kqzdywtd(void *arg);
//�������������ά��ͨ��
static void analysis_request_jszdywtd(void *arg);
//�����������ѯ�ն���Ȩ�ļ�
static void analysis_request_cxzddqsqxx(void *arg);
//�������������˿ڵ�Դ
static void analysis_request_adkhgldkdy(void *arg);
//���������������ն�
static void analysis_request_glzddy(void *arg);
//�л��˿�ʡ��ģʽ
static void analysis_request_qhdksdms(void *arg);
//�л�����ʡ��ģʽ
static void analysis_request_qhzjsdms(void *arg);
static void mqtt_respoonse_port(char *sendbuf, struct _new_usbshare_parm *parm);
//static int get_vid_pid(struct _usb_pro *usb, char *outpid, char *outvid);
static int mqtt_response_errno_gw(struct _new_usbshare_parm *parm, int err_code);
static int prepare_mqtt_gw_close_packet(char *sendbuf, struct _new_usbshare_parm *parm);
static int prepare_mqtt_gw_robot_arm_packet(char *sendbuf, struct _new_usbshare_parm *parm,char *cmd);
static int mqtt_open_share_response(struct _new_usbshare_parm *parm, int fd);
static void analysis_request_openport(void *arg);
static void analysis_request_closeport(void *arg);
static void analysis_request_start_robot_arm(void *arg);
static void analysis_request_update_robot_arm(void *arg);

_so_note app_note;

struct err_no_infos app_err_no_infos[] =
{
	//Э�����ݻ��ն˴���
	{ DF_TAX_ERR_CODE_TER_SYSTEM_ERROR, 301, "Terminal system error." },									//�ն�ϵͳ����
	{ DF_TAX_ERR_CODE_COMMAND_UNSUPPORTED, 302, "This command word is not supported." },					//�����ֲ�֧��
	{ DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, 303, "The terminal memory is insufficient." },				//ϵͳ�ڴ治��
	{ DF_TAX_ERR_CODE_DATA_PARSING_FAILED, 304, "The JSON data parsing failure." },							//json����ʧ��
	{ DF_TAX_ERR_CODE_SYSTEM_TIME_ERROR,312,"The terminal system time error."},								//��ǰ�ն�ϵͳʱ�����
	{ DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR,325,"Mysql database connection error."},							//���ݿ����Ӵ���
	//������Ʊ����
	{ DF_TAX_ERR_CODE_EXCEED_KP_END_DATE, 305, "Exceeding the billing deadline." },							//������Ʊ��ֹ����
	{ DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT, 306, "Exceeding a single invoice billing limit." },				//�������ŷ�Ʊ��Ʊ�޶�
	{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT, 307, "Exceeding the cumulative billing limit." },				//���������޶�
	{ DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE, 308, "This invoice type invoice is not supported." },				//��֧�ָ÷�Ʊ����
	{ DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR, 309, "Invoice data parsing error." },							//��Ʊ���ݽ���ʧ��
	{ DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED, 310, "Authorized tax rate does not exist." },					//��Ȩ˰�ʲ�֧��
	{ DF_TAX_ERR_CODE_HAVE_NO_INVOICE, 311, "Have no invoice available." },									//û�п��÷�Ʊ			
	{ DF_TAX_ERR_CODE_INV_DATA_TOO_LARGE, 313, "Invoice data too large." },									//��Ʊ��������			
	{ DF_TAX_ERR_CODE_INV_DATA_DISACCORD,315,"Invoice data is inconsistent with plate infomation."},		//��Ʊ������˰����Ϣ��һ��
	{ DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME,318,"Exceeding offline invoice time."},						//��������ʱ��
	{ DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING,326,"The invoice serial number is inusing."},						//��ǰ��ˮ�����ڿ�Ʊ��
	//˰�̲���ͨ�ô���
	{ DF_TAX_ERR_CODE_PLATE_IN_USED,314,"The plate is in used."},											//˰�����ڱ�ʹ��
	{ DF_TAX_ERR_CODE_PLATE_OPER_FAILED,316,"The plate operation failed."},									//˰�̲���ʧ��
	{ DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR,317,"The plate cert passwd failed."},							//֤��������			
	{ DF_TAX_ERR_CODE_PLATE_NON_EXISTENT,319,"The plate is not on this terminal."},							//˰�̲��ڸ��ն���
	{ DF_TAX_ERR_CODE_MONTH_HAVE_NO_INVPICE,320,"No invoice for this month's inquiry."},					//��ǰҪ��ѯ�޷�Ʊ
	{ DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8,321,"Illegal character or UTF8 encoding is used."},				//ʹ���˷Ƿ��ַ���utf8����
	{ DF_TAX_ERR_CODE_CAN_NOT_USE_IOZSV,322,"Illegal characters cannot be used in tax numbers.(Example : I O Z S V)"},//˰���в���ʹ��IOZSV�Ƿ��ַ�
	{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST, 323, "Invoice serial number does not exist." },				//��Ʊ��ˮ�Ų�����
	{ DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED, 324, "Invoice serial number has already been issued. " },//�÷�Ʊ��ˮ���Ѿ����߹���
	{ DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH, 327, "This USB port is not authorized." },//��USB�˿�δ��Ȩ
	{ DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_UPDATING, 328, "The robot arm is updating..." },//��USB�˿ڶ�Ӧ�Ļ�е����������
	{ DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_NO_POWERE, 329, "The port is robot arm,can not manage power!"}

};

struct err_info mqttgw_share_err_n[] =
{
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED, "The port is already used!" },
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_NOT_FOUND, "The port no device!" },
	{ MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL, "The space is full!" },
	{ MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START, "Get start port err!" },
	{ MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE, "Close Port err!" },
	{ MQTT_GW_USBSHARE_OPERA_ERR, "The operation failure!" },
	{ MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR, "Connect trans server error!" },
	{ MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_WITHOUT_DEV, "Mechanical arm press failed, there may be no equipment available!"},
	{ MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_NO_SATRT, "Mechanical arm not activated!"},
	{ MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED,"The port is not aollowed!"},
	{ MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING,"The robot arm is updating..."},
	{ MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE,"The machine type is not robot arm!"},
	{ MQTT_GW_USBSHARE_ROBOTARM_NOT_WORK,"The robot arm is not working!"}

};

struct _ty_err_table err_n[]=
{   {   1,	DF_ERR_PORT_CORE_TY_USB_INIT            },//stm32��ʼ����
	{   2,	DF_ERR_PORT_CORE_TY_USB_PORT_OVER       },//����USB����˿�����
	{   3,	DF_ERR_PORT_CORE_TY_USB_PCB_NO          },//�ÿ�PCB��ʼ��ʱ,������
	{   4,	DF_ERR_PORT_CORE_TY_USB_STM32_OPEN      },//stm32�򿪶˿�ʧ��
	{   5,	DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER      },//usb bus���豸����
	{   6,	DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV     },//usb�˿��Ѿ���,û�з��ָ��豸
	{	7,	DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION	},//�޲���Ȩ��
	{   8,	DF_ERR_PORT_CORE_TY_USB_STM32_ANSWER    },//stm32�ڹ涨ʱ����û��Ӧ��
	{   9,	DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE     },//stm32Ӧ��رն˿�ʧ��
	{   10,	DF_ERR_PORT_CORE_TY_USB_STM32_PCB       },//stm32δ�ҵ�PCB
	{   11,	DF_ERR_PORT_CORE_TY_USB_STM32_DEV       },//stm32δ�ҵ�DEV
	{   12,	DF_ERR_PORT_CORE_TY_USB_TREE_READ       },//��usb��ʧ��
	{   13,	DF_ERR_PORT_CORE_TY_USB_TREE_PCB        },//û���ҵ��ð���
	{	14,	DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO	},//��ʼ�Ų���Ϊ0
	{   15,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED  },//�ö˿��Ѿ���ʹ����  
	{   16,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_NO_CLOSE    },//��Ȩ�رոö˿�
	{   17,	DF_ERR_PORT_CORE_USB_IP_ROOT            },//��������root�û���¼
	{   18,	DF_ERR_PORT_CORE_USB_IP_KO              },//usbipû���ҵ�����
	{   19,	DF_ERR_PORT_CORE_REGIT_ERR              },//ע��Э��ջʧ��
	{   20,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_USB_BUSY},//port_manage���˿���
	{   21,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_CHECK_PAR},//���������
	{   22,	DF_ERR_PORT_CORE_TY_USB_OPER_ERR            },//��·�����й����з�������,USB HUB��ʧ��,ϵͳҪ����
	{   23,	DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_LINUX_FILE  },//����ϵͳUSB�ļ�ϵͳ������������,������Ҫ����������
	{   24,	DF_ERR_PORT_CORE_SOCKET_SEV_BIND            },//socket�󶨶˿�ʧ��
	{   25,	DF_ERR_PORT_CORE_SOCKET_LISTER              },//socket�����˿�ʧ��
	{   26,	DF_ERR_PORT_CORE_SWITCH_SERIA               },//seria��������
	{   27,	DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID          },//û�п��õ���busid
	{   28,	DF_ERR_PORT_CORE_TY_USB_MAIN_HUB            },//��hub��ʧ,Ҫ����������
	{   29,	DF_ERR_PORT_CORE_TY_USB_PCB_HUB             },//����hub��ʧ
	{   30,	DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM        },//û���ҵ���Ӧ���豸ö��
	{   31,	DF_ERR_PORT_CORE_TY_USB_CLOSE               },//�豸�Ѿ��ر�,���ļ�ϵͳ�����豸��Ϣ,ϵͳ��������
	{   32,	DF_ERR_PORT_CORE_USB_IP_BUSID_RE            },//��busid�Ѿ�����
	{   33,	DF_ERR_PORT_CORE_USB_IP_WRITE				},//дusbipʧ��
	{	34,	DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR			},//���ظ���ʧ��
	{	35,	DF_ERR_PORT_CORE_FTP_DOWNLOAD_PROGRAM_IS_NEW},//����Ѿ������°汾
	{	36,	DF_ERR_PORT_CORE_PORT_ISNOT_SHUIPAN			},//�ö˿ڷ�֧�ֵ�˰��
	{	37, DF_ERR_PORT_CORE_GET_INVOICE_ERR            },//��ȡ˰�̷�Ʊ��Ϣ����
	{	38, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA		},//��֧�ִ��෢Ʊ��ѯ
	{	39, DF_ERR_PORT_CORE_GET_FP_PARAMETERS			},//��ȡ˰�̷�Ʊ��������
	{	40, DF_ERR_PORT_CORE_GET_FP_OPEER_USB			},//��ȡ˰����Ϣ����USBʧ��
	{	41, DF_ERR_PORT_CORE_GET_FP_INVIOCE_NUM			},//��ȡ˰�̷�Ʊ��������
	{	42, DF_ERR_PORT_CORE_GET_FP_INVOICE_LEN			}//��ȡ��Ʊ���ݴ�С����
	
};

struct _app_order order_name[]=
{   
	{   DF_TY_1502_SEV_ORDER_SET_USB_START,			"[������]:���÷�������USB��ʼ��",    		set_start_port		},
	{   DF_TY_1502_SEV_ORDER_SET_IP,				"[������]:���÷�����IP",             		set_ip_ip			},
	{   DF_TY_1502_SEV_ORDER_SET_ID,				"[������]:�����豸�������",           		set_id        		},
	{   DF_TY_1502_SEV_ORDER_SET_TIMER,				"[������]:���÷�����ʱ��",             		set_timer       	},
	{   DF_TY_1502_SEV_ORDER_SET_PAR,				"[������]:���÷���������",             		set_par           	},
	{	DF_TY_1502_SEV_ORDER_SET_TRYOUT_TIME,		"[������]:������������",					set_tryout_date		},
	{	DF_TY_1502_SEV_ORDER_SET_UPDATE_ADDR,		"[������]:����������������ַ",				set_update_address	},
	//{	DF_TY_1502_SEV_ORDER_SET_VPN_PAR,			"[������]:����VPN����������",				set_vpn_server_par	},
	//{	DF_TY_1502_SEV_ORDER_SET_CLOSE_AUTO_UPDATE,	"[������]:�����Ƿ�ر��Զ�����",			set_close_update	},
	{   DF_TY_1502_SEV_ORDER_SEE_SEV_PRO,			"[��ѯ��]:����������",             			get_sev_pro       	},
	{	DF_TY_1502_SEV_ORDER_SEE_USB_PORT_INFOR,	"[��ѯ��]:��ȡUSB�豸��ϸ��Ϣ",				get_usb_dev_info	},
	{   DF_TY_1502_SEV_ORDER_SEE_SEV_STATE,			"[��ѯ��]:������״̬",             			get_sev_status      },
	{   DF_TY_1502_SEV_ORDER_SEE_USB_PORT_PRO,		"[��ѯ��]:��USB�˿ڲ�ѯ�˿�����",   		get_port_pro    	},
	{   DF_TY_1502_SEV_ORDER_SEE_PAR,				"[��ѯ��]:����������",             			get_sev_par       	},
	{   DF_TY_1502_SEV_ORDER_SEE_LOG,				"[��ѯ��]:�������쳣��־",          		get_log           	},
	{   DF_TY_1502_SEV_ORDER_SEE_NET_PAR,			"[��ѯ��]:�������ò���",					get_net_par    		},
	{	DF_TY_1502_SEV_ORDER_SEE_CHECK,				"[��ѯ��]:����ն������Ƿ�����",			get_check_all_port	},
	{	DF_TY_1502_SEV_ORDER_SEE_UPDATE_ADDR,		"[��ѯ��]:��ѯ������������ַ",				get_update_address	},
	{	DF_TY_1502_SEV_ORDER_SEE_ALL_PORT_STATE,	"[��ѯ��]:��ѯ�ն�����USB�˿�״̬",			get_all_port_status	},
	{	DF_TY_1502_SEV_ORDER_SEE_TER_SOFT_VERSION,	"[��ѯ��]:��ѯ�ն�����汾��Ϣ",			get_ter_soft_version},
	//{	DF_TY_1502_SEV_ORDER_SEE_VPN_PAR,			"[��ѯ��]:��ѯVPN����������",				get_vpn_server_par	},
	//{	DF_TY_1502_SEV_ORDER_SEE_CLOSE_AUTO_UPDATE,	"[��ѯ��]:��ѯ�Ƿ�ر��Զ�����",			get_close_update	},
	{	DF_TY_1502_SEV_ORDER_SEE_VERSION_DESCRIBE,	"[��ѯ��]:��ѯ�汾��������",				get_version_describe},
	{   DF_TY_1502_SEV_ORDER_OPER_OPEN_USB,			"[������]:��USB�˿ں�����ʹ��Ȩ",			open_port    		},
	{   DF_TY_1502_SEV_ORDER_OPER_CLOSE_USB,		"[������]:��USB�˿ں��ͷ�ʹ��Ȩ",			close_port    		},
	// {   DF_TY_1502_SEV_ORDER_OPER_ID_OPEN_USB,		"[������]:������������ʹ��Ȩ",        	open_port_id     	},
	// {   DF_TY_1502_SEV_ORDER_OPER_ID_CLOSE_USB,		"[������]:���������ͷſ���Ȩ",        	close_port_id     	},
	{   DF_TY_1502_SEV_ORDER_OPER_CLOSE_NOW,		"[������]:ǿ�ƹرն˿�",             		close_port_now     	},
	{   DF_TY_1502_SEV_ORDER_OPER_POWER_OFF,		"[������]:�ػ�",            	 			power_off          	},
	{   DF_TY_1502_SEV_ORDER_OPER_HEART,			"[������]:�ͻ����Ѵ򿪶˿��ϱ�",			heart     			},
	{   DF_TY_1502_SEV_ORDER_OPER_RESET,			"[������]:��������",             			reset     			},
	{   DF_TY_1502_SEV_ORDER_OPER_REDIT,			"[������]:ע������",             			regdit              },
	{   DF_TY_1502_SEV_ORDER_OPER_OPEN_NOACTIVE,	"[������]:��USB�˿ڵ������USBIP",		open_port_no_active	},
	{   DF_TY_1502_SEV_ORDER_SEE_OPEN_PORT,			"[������]:ά�ֲ鿴�򿪵��豸��������",	keep_open_port		},
	{	DF_TY_1502_SEV_ORDER_OPER_UPDATA,			"[������]:���汾���²�����",				check_version_updata},
	{	DF_TY_1502_SEV_ORDER_OPER_DETTACH_PORT,		"[������]:ʹ��Dettach�ر�USB�˿�",			close_port_by_dettach}
};


MODULE *get_module(void)
{
	return &module;
}

int add_soft_module()
{
	//module_out("��ʼ���ظ�ģ��\n");
	if (switch_dev_add(DF_MB_HARDWARE_A20_A33, app_note.type)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��Ƭ������ģ�����ʧ��\r\n");
		return -1;
	}
	//module_out("���USBִ���ļ�\n");
	if (ty_usb_add("/dev/switch", app_note.machine_type)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�˿�ģ��ʧ��\r\n");
		return -2;
	}
	//module_out("����¼��ļ�\n");
	// if (event_file_add("/dev/switch")<0)
	// {
	// 	logout(INFO, "SYSTEM", "INIT", "����¼�ģ��ʧ��\r\n");
	// 	return -3;
	// }
	//module_out("��������ʱ�ļ�\n");
	if (_get_net_time_add()<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��������ʱģ��ʧ��\r\n");
		return -4;
	}
	//module_out("���ת���ļ�\n");
	if (tran_data_add(DF_TY_TRAN_PORT, &app_note) < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ת��ģ��ʧ��\r\n");
		return -5;
	}
	//module_out("���UDP�����ļ�\n");
	if (_udp_sev_add(DF_TY_UDP_SER_PORT)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���UDP����ģ��ʧ��\r\n");
		return -7;
	}
	//module_out("��������ļ�\n");
	if (deploy_add((_so_note *)&app_note, "/dev/switch", "/dev/ty_usb", DF_MB_HARDWARE_A20_A33)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�������ģ��ʧ��\r\n");
		return -8;
	}
	//module_out("����߳��ػ��ļ�\n");
	// if (ty_pd_add("/dev/switch")<0)
	// {
	// 	logout(INFO, "SYSTEM", "INIT", "����ػ�����ģ��ʧ��\r\n");
	// 	return -9;
	// }
	//module_out("����豸��Ϣ�����ļ�\n");
	if (machine_infor_add("/dev/switch", &app_note, DF_MB_HARDWARE_A20_A33)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��ӻ�����Ϣģ��ʧ��\r\n");
		return -11;
	}
	//module_out("���socket�����������ļ�\n");
	if (_socket_sev_add(DF_TY_SEV_ORDER_PORT)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��ӷ���˿�ʧ��\r\n");
		return -12;
	}
	if (get_net_state_add() <0)
	{
		logout(INFO, "SYSTEM", "INIT", "�������״̬ģ��ʧ��\r\n");
		return -13;
	}
	// if (vpn_set_add() <0)
	// {
	// 	logout(INFO, "SYSTEM", "INIT", "���VPNģ��ʧ��\r\n");
	// 	return -14;
	// }
	//module_out("���LCDģ��\n");
	if (lcd_state_add())
	{
		logout(INFO, "SYSTEM", "INIT", "���LCD��ʾģ��ʧ��\r\n");
		return -15;
	}
	if (_m_server_add())
	{
		logout(INFO, "SYSTEM", "INIT", "���mqttͬ��ģ��ʧ��\r\n");
		return -16;
	}

	// if (_devops_add())
	// {
	// 	logout(INFO, "SYSTEM", "INIT", "����ն���ά����ģ��ʧ��\r\n");
	// 	return -17;
	// }
	if (bluetooth_add("/dev/ttyS2", &app_note, "/dev/switch"))
	{
		return -18;
	}
	return 0;
}

void recvSignal(int sig)
{  
	printf("================================received signal %d !!!================================\n",sig);  
}

void load(void)
{   //_linux_load();
	//memset(&linux_s, 0, sizeof(struct _linux_s));
	_lock_load();
	_scheduled_task_load();
	_delay_task_load();
	mosquitto_lib_init();

    ty_file_load();
    _linux_usart_load(DF_MB_HARDWARE_A20_A33);
	//_err_add("port_core", &port_core_err_buf);//����޸��޳�err
}

int set_by_ter_info_file(void)
{
	char ter_id[20] = "068330";
	char *ptr = NULL;
	char src_data[2000] = {0};
	read_file(DF_TER_INFO_FILE, src_data, sizeof(src_data));
	ptr = strstr(src_data,"terid=");
	if(ptr != NULL)
	{
		memset(ter_id,0x00,sizeof(ter_id));
		memcpy(ter_id,ptr + strlen("terid="),12);
	}
	printf("set_by_ter_info_file ter_id = %s\n",ter_id);
	if(0 == memcmp(ter_id,"068330",6)) //��е�ۻ���30��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2303;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2303;
		app_note.name = DF_TER_TYPE_EXE_MB2303;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2303;
	}
	else if(0 == memcmp(ter_id,"068360",6)) //2306���ͣ�60��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2306;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2306;
		app_note.name = DF_TER_TYPE_EXE_MB2306;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2306;
	}
	else if(0 == memcmp(ter_id,"068191",6)) //2212���ͣ�120��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2212;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2212;
		app_note.name = DF_TER_TYPE_EXE_MB2212;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2212;
	}
	else if(0 == memcmp(ter_id,"068161",6)) //2110���ͣ�100��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2110;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2110;
		app_note.name = DF_TER_TYPE_EXE_MB2110;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2110;
	}
	else if(0 == memcmp(ter_id,"068181",6)) //2108���ͣ�68��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2108;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2108;
		app_note.name = DF_TER_TYPE_EXE_MB2108;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2108;
	}
	else if(0 == memcmp(ter_id,"068171",6)) //2202���ͣ�20��
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2202;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2202;
		app_note.name = DF_TER_TYPE_EXE_MB2202;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2202;
	}
	else
	{
		app_note.r_date = DF_TER_VERSION_TIME;//�汾��������
		app_note.ver = DF_TER_VERSION_NAME_MB2303;//�汾��
		app_note.v_num = DF_TER_VERSION_NUM;//˳������汾��
		app_note.abdr = "Beta";
		app_note.cpy = "mengbaiinfo";
		app_note.writer = "mengbai";
		app_note.note = "usb_share";		
		app_note.code = "gbk";
		app_note.type = DF_TER_TYPE_NAME_MB2303;
		app_note.name = DF_TER_TYPE_EXE_MB2303;
		app_note.machine_type = DF_TY_MACHINE_TYPE_MB2303;
	}
}

int module_init(void)
{
	int result = 0;
	char name[DF_FILE_NAME_MAX_LEN] = { 0 };
	struct _lcd_print_info *lcd_print_info = NULL;
	lcd_print_info = get_lcd_printf_info();

	set_by_ter_info_file();

	load();
	logout(INFO, "SYSTEM", "INIT", "\n\n");
	logout(INFO, "SYSTEM", "INIT", "����ʼ����,���ظ�Ӧ��ģ��\r\n");




	printf("enter main_start_init\n");
	main_start_init();
	printf("exit main_start_init\n");
    memset(&module,0x00,sizeof(MODULE));
	signal(SIGPIPE, recvSignal);
	//����ϵͳ����
	signal(SIGSEGV, SIG_IGN); 
    //module_out("��ӿ������ļ�ϵͳ\n");
	if (add_soft_module() < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ģ�����ʧ��,�����˳�����\r\n");
		return -1;
	}
	logout(INFO, "system", "����", "ϵͳ���������,���ģ���������,��������ģ�鼰Ӳ��\r\n");

	

	module.switch_fd=ty_open("/dev/switch",0,0,NULL);
	if (module.switch_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��Ƭ������ģ���ʧ��\r\n");
		return -2;
	}
	//module_out("��LCDģ�鲢��ʼ����ʾ\n");

	module.lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
	if (module.lcd_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "lcdģ���ʧ��\r\n");
		return -3;
	}
	memset(&lcd_print_info->version, 0, sizeof(lcd_print_info->version));
	memcpy(&lcd_print_info->version, app_note.ver,8);
	lcd_print_info->usbshare_en = 0;
	lcd_print_info->sd_en = 1;
	lcd_print_info->udp_en = 0;
	lcd_print_info->mqtt_en = 1;
	lcd_print_info->mqtt_state = 1;
	//����lcd��ʾ����ʾ
	ty_ctl(module.lcd_fd, DF_LCD_STATE_UPDATE_STATE);

	module.ty_usb_fd=ty_open("/dev/ty_usb",0,0,NULL); 
	if(module.ty_usb_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��/dev/ty_usbʧ��\r\n");
		return -5;
	}

	module.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	if (module.machine_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ն���Ϣģ��ʧ��\r\n");
		return -6;
	}

	module.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
	if (module.deploy_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "������ģ��ʧ��\r\n");
		return -7;
	}

	module.net_time_fd = ty_open("/dev/get_net_time", 0, 0, NULL);
	if (module.net_time_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�������ʱģ��ʧ��\r\n");
		return -8;
	}

	module.mq_sync_fd = ty_open("/dev/mq_sync", 0, 0, NULL);
	if (module.mq_sync_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��MQTTͬ��ģ��ʧ��\r\n");
		return -9;
	}

	module.mqtt_lock = _lock_open("_app.c", NULL);
	if (module.mqtt_lock<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���̻߳�������ʧ��\r\n");
		return -11;
	}
	module.data_lock = _lock_open("data_gw", NULL);
	if (module.data_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��mqtt������ʧ��\r\n");
		return -12;
	}

	module.sql_lock = _lock_open("_app.c", NULL);
	if (module.sql_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��mysql��ʧ��\r\n");
		return -13;
	}

	module.inv_read_lock = _lock_open("_app.c", NULL);
	if (module.inv_read_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪶�ȡ��Ʊ��ʧ��\r\n");
		return -14;
	}
	memset(&message_callback, 0, sizeof(struct _message_callback));
	memset(&message_history, 0, sizeof(struct _message_history));
	module.m_task_fd = _scheduled_task_open("deal_message", task_deal_message_queue, &module, 10, NULL);
	if (module.m_task_fd < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����Ϣ�����߳�ʧ��\r\n");
		return -15;
	}
	module.connect_time = 1;
	//usb����ģ���ʼ��
	//power_saving_init(&module);
	usb_port_init(&module,&app_note);

	result = ty_ctl(module.machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, module.mqtt_server, &module.mqtt_port, module.user_name, module.user_passwd);
	if (result < 0)
	{
		module_out("��ȡ��¼�õ�MQTT�û�����ʧ��\n");
	}
	else
	{
		module_out("��ȡ��¼�õ�MQTT�û�����ɹ�,server %s,port %d,usr_name %s,passwd %s\n", module.mqtt_server, module.mqtt_port, module.user_name, module.user_passwd);
		char id_sub[200] = { 0 };
		char id_pub[200] = { 0 };
		memset(id_sub, 0, sizeof(id_sub));
		memset(id_pub, 0, sizeof(id_pub));
		sprintf(module.topic, "mb_fpkjxt_%s", module.user_name);
		sprintf(id_sub, "sub%s", module.user_name);
		sprintf(id_pub, "pub%s", module.user_name);
		_mqtt_client_init(sub_message_callback_function_sync, sub_err_callback_deal_function, module.mqtt_server, module.mqtt_port, module.topic, module.user_name, module.user_passwd, id_sub, id_pub);
	}

    memset(name,0,sizeof(name));
	sprintf(name, "/dev/socket_sev/sever/%d", DF_TY_SEV_ORDER_PORT);
    module.sock_fd=ty_open(name,0,0,NULL);
    if(module.sock_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ļ�:/dev/socket_sev/sever/10001 ʧ��\r\n");
        return module.sock_fd;
    }
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_HOOK,&module,_app_order);
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_CLOSE_TYPE,1);
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_OPEN_KEEP,1);
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_NODELAY,1);
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_REV_TIMER,10);
    ty_ctl(module.sock_fd,DF_SOCKET_SEV_CM_SEND_TIMER,10);
	//printf("======socket server file open success======\n");

	memset(name,0,sizeof(name));
	sprintf(name, "/dev/udp_sev/%d", DF_TY_UDP_SER_PORT);
	module.udp_fd=ty_open(name,0,0,NULL);
	if(module.udp_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ļ���/dev/udp_sev/10002ʧ��\r\n");
		return module.udp_fd;
	}
	//module_out("UDP �ļ��򿪳ɹ�\n");
	module.task=_scheduled_task_open("udp_ser",udp_ser_main_task,&module,DF_UDP_SEV_MAIN_TASK_TIMER,NULL);
	if (module.task<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��udp����ģ��ʧ��\r\n");
		return module.udp_fd;
	}
	//module_out("��update file �ɹ�\n");
	module.get_net_fd=ty_open("/dev/get_net_state",0,0,NULL); 
	if(module.get_net_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪻�ȡ����״̬ģ��ʧ��\r\n");
		return module.get_net_fd;
	}


	
	// module.opsfd = ty_open("/dev/devops", 0, 0, NULL);
	// if (module.opsfd<0)
	// {
	// 	logout(INFO, "SYSTEM", "INIT", "���ն���ά����ʧ��\r\n");
	// 	return module.opsfd;
	// }

	 module.ble_fd = ty_open("/dev/bluetooth", 0, 0, NULL);



	result = _scheduled_task_open("update_task", update_task_hook, &module, 1000 * 20, NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪳�����Ȩ�����߳�ʧ��\r\n");
		return result;
	}

	//���������������
	result = _scheduled_task_open("intranet_update_task", intranet_update_task_hook, &module, 1000 * 20, NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��[����]������Ȩ�����߳�ʧ��\r\n");
		return result;
	}

	//������������
	struct http_parm_m parm;
	memset(&parm, 0, sizeof(struct http_parm_m));
	result = _scheduled_task_open("intranet_update_thread", intranet_update_thread, &parm, 1000 * 2, NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��������ʧ��\r\n");
		return result;
	}

	module.tran_fd = ty_open("/dev/tran_data",0,0,NULL);
	if(module.tran_fd<0)
	{
		logout(INFO, "system", "init", "��ת��ģ��ʧ��\r\n");
		return module.tran_fd;
	}

	


	printf("========================Start success,going to monitor mode version %s date %s============================\n", app_note.ver, app_note.r_date);

	logout(INFO, "system", "����", "�ն�����������,������ţ�%s,�汾:%s,����%s,����:%s\r\n", module.user_name, app_note.ver, app_note.v_num, app_note.r_date);


    for(;;sleep(10)); 
}



_so_note    *get_application_version(void)
{
    
	return (_so_note    *)&app_note;
    
}


static int _app_order(int fd,uint32 ip,uint16 port,void *arg)
{   
	MODULE    *par;
	uint8 buf[2048] = { 0 };
    int result,buf_len,i;
    par=arg;
    buf_len=sizeof(buf);
    memset(buf,0,sizeof(buf));
    result=unpack(fd,buf,&buf_len);
    if(result<0)
    {
		// module_out("���ݰ�����,ip = %08x\n",ip);
		if(result!=-1)
        {   
			answer_no_order(fd,par,NULL,0);
        }
        return result;
    }
    for(i=0;i<sizeof(order_name)/sizeof(order_name[0]);i++)
    {   
		if(order_name[i].order==result)
        {
			module_out("ִ��:[%s]\n",order_name[i].name);
            order_name[i].answer(fd,ip,par,buf,buf_len);
            return 0;
        }
    }
    answer_no_order(fd,par,NULL,0);
    return -1;
}
/*======================================����ִ��===============================================*/
    /*----------------------------------������--------------------------------------------*/
		//���÷�������USB��ʼ��
    static int set_start_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)    
    {   
		int start;
        int result;
        module_out("[set_start_port]:���÷�����USB��ʼ��\n");
        ip=ip;
        start=buf[1]*0x10000+buf[2]*0x100+buf[3];
        if(start==0)
        {   
			return err_out_tcp(fd,DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO);
        }
        module_out("��ʼ��Ϊ:%d\n",start);
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_START_PORT,start);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_USB_START,NULL,0); 
    }
		//���÷�����IP
	static int set_ip_ip(int fd, uint32 ip, MODULE    *par, uint8 *buf, int buf_len)
	{
		int result;
		uint32 t_ip, mask, getway, DNS;
		int type;
		type = buf[0];
		t_ip = (uint32)(buf[1] * 0x1000000) + (uint32)(buf[2] * 0x10000) + (uint32)(buf[3] * 0x100) + (uint32)buf[4];
		mask = (uint32)(buf[5] * 0x1000000) + (uint32)(buf[6] * 0x10000) + (uint32)(buf[7] * 0x100) + (uint32)buf[8];
		getway = (uint32)(buf[9] * 0x1000000) + (uint32)(buf[10] * 0x10000) + (uint32)(buf[11] * 0x100) + (uint32)buf[12];
		DNS = (uint32)(buf[13] * 0x1000000) + (uint32)(buf[14] * 0x10000) + (uint32)(buf[15] * 0x100) + (uint32)buf[16];
		module_out("[set_ip_ip]:����ip=0X%08X,mask=0X%08X,getway=0X%08X,DNS=0X%08X\n", t_ip, mask, getway, DNS);
		logout(INFO, "TCP", "10001", "��IP%08x��������������ն�IP����type=%d ip=0x%08x netmask=0x%08x gateway=0x%08x dns=0x%08x\r\n", ip, type, t_ip, mask, getway, DNS);

		result = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_SET_IP, t_ip, mask, getway, DNS, type);
		module_out("IP���ý���,result = %d\n\n\n", result);
		if (result<0)
		{
			return err_out_tcp(fd, result);
		}
		ty_ctl(par->get_net_fd, DF_GET_NET_STATE_CM_CLEAN_STATUS);
		write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SET_IP, NULL, 0);
		return 0;
	}
		//�����豸�������
    static int set_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		struct _switch_dev_id   id;
        int result;
        module_out("[set_id]:�����豸ID�Լ���������\n");
        memset(&id,0,sizeof(id));
        asc2bcd(buf,id.id,12);
        asc2bcd(buf+12,id.date,4);
        asc2bcd(buf+17,id.date+2,2);
        asc2bcd(buf+20,id.date+3,2);
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն˻�����Ų���\r\n", ip);
        if((result=ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_SET_ID,&id))<0)
        {   
			module_out("���û������ʧ��\n");
			logout(INFO, "SYSTEM", "10001", "���û������ʧ��,�������%d\r\n", result);
			return err_out_tcp(fd,result);
        }
		module_out("���û�����ųɹ������������ն�\n");
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_ID,NULL,0); 
		//return err_out_tcp(fd,DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION);
		
    }
		//���÷�����ʱ��
    static int set_timer(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		module_out("[set_timer]:���÷�����ʱ��\n");
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն�ʱ�����\r\n", ip);
		SetSystemTime(buf);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_TIMER,NULL,0);
    }
		//���÷���������
    static int set_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {
        int result;
		char net_a[100] = { 0 };
        memset(net_a,0,sizeof(net_a));
        memcpy(net_a,buf+22,100);
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն˲�������\r\n", ip);
        module_out("Զ�̸��¶˿�:%s\n",net_a);
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_NET_SOFT,net_a);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_PAR,NULL,0); 
    }
	
	//����������������ַ���û�������
    static int set_update_address(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {
		char addr[65] = { 0 };
		char user[25] = { 0 };
		char passwd[25] = { 0 };
		char net_a[200] = { 0 };
        //port_n=buf[0];
		
		module_out("�°����÷�����������ַ\n");
        memset(net_a,0,sizeof(net_a));
		memset(addr,0,sizeof(addr));	
		memset(user,0,sizeof(user));
		memset(passwd,0,sizeof(passwd));
		memcpy(addr,buf,60);
		memcpy(user,buf+60,20);
		memcpy(passwd,buf+80,20);
		sprintf(net_a,"host=[%s],user=[%s],pwd=[%s],timer=[0]",addr,user,passwd);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_UPDATE_ADDR,NULL,0); 
    }

	static int set_tryout_date(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
	{
		uint8 date[4] = { 0 };
		module_out("[[set_tryout_date]:��ʼ����\n");
        asc2bcd(buf,date,4);
        asc2bcd(buf+5,date+2,2);
        asc2bcd(buf+8,date+3,2);
        module_out("[set_tryout_date]:������������\n");
		ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_SET_TRY_TIME,date);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_TRYOUT_TIME,NULL,0); 
	}


    // static int check_port_status(MODULE *par)
	// {
	// 	return 0;
	// }

	// static int set_vpn_server_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
	// {
	// 	int enable;
	// 	int mode;
	// 	int result;
	// 	uint8 server[50] = { 0 };
	// 	int port;
	// 	uint8 user[20] = { 0 };
	// 	uint8 passwd[20] = { 0 };
	// 	int ftp_port;
	// 	uint8 ftp_user[20] = { 0 };
	// 	uint8 ftp_passwd[20] = { 0 };
	// 	uint8 hold[20] = { 0 };
	// 	int i;
	// 	module_out("���յ�������:buf_len = %d\n",buf_len);
	// 	for(i=0;i<156;i++)
	// 	{
	// 		printf("%02x ",buf[i]);
	// 	}
	// 	printf("\n");
	// 	memset(server,0,sizeof(server));
	// 	memset(user,0,sizeof(user));
	// 	memset(passwd,0,sizeof(passwd));
	// 	memset(ftp_user,0,sizeof(ftp_user));
	// 	memset(ftp_passwd,0,sizeof(passwd));
	// 	memset(hold,0,sizeof(hold));

	// 	enable = buf[0];
	// 	mode = buf[1];		
	// 	memcpy(server,buf+2,50);
	// 	port = buf[52]*0x100+buf[53];		
	// 	memcpy(user,buf+54,20);		
	// 	memcpy(passwd,buf+74,20);
	// 	ftp_port=buf[94]*0x100+buf[95];		
	// 	memcpy(ftp_user,buf+96,20);		
	// 	memcpy(ftp_passwd,buf+116,20);		
	// 	memcpy(hold,buf+136,20);
	// 	module_out("�����Ƿ�����VPNΪ%d\n",enable);
	// 	module_out("����VPN��¼ģʽΪ%d\n",mode);
	// 	module_out("����VPN��������ַΪ%s\n",server);
	// 	module_out("����VPN�������˿ں�%d\n",port);
	// 	module_out("����VPN�ͻ����û���Ϊ%s\n",user);
	// 	module_out("����VPN�ͻ�������Ϊ%s\n",passwd);
	// 	module_out("����VPN������FTP���ض˿�Ϊ%d\n",ftp_port);
	// 	module_out("����VPN������FTP�����û���Ϊ%s\n",ftp_user);
	// 	module_out("����VPN������FTP�����û�����Ϊ%s\n",ftp_passwd);

	// 	result=ty_ctl(par->vpnfd,DF_VPN_SET_CM_SET_VPN,enable,mode,server,port,user,passwd,ftp_port,ftp_user,ftp_passwd);
	// 	if(result<0)
    //     {   
	// 		return err_out_tcp(fd,result);
    //     }
	// 	module_out("����VPN ���,Ӧ��ͻ���\n");
	// 	return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_VPN_PAR,NULL,0); 
	// }

	// static int set_close_update(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
	// {	
	// 	int result;
	// 	int close_status;
	// 	close_status = buf[0];
	// 	module_out("�����Ƿ�ر��Զ�����״̬��Ϊ��%d\n",close_status);
	// 	result = ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_CLOSE_UPDATE,close_status);
	// 	if(result < 0)
	// 	{
	// 		return err_out_tcp(fd,result);
	// 	}
	// 	module_out("�����Զ��������\n");
	// 	return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_CLOSE_AUTO_UPDATE,NULL,0); 
	// }


    /*----------------------------------��ѯ��-------------------------------------------*/
	//��ȡ����������
    // static int get_sev_pro(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)    
	// {
	// 	uint8 s_buf[1024] = { 0 };
	// 	uint8 trydate[10] = { 0 };
	// 	int result;
    //     int start;
	// 	//int i;
	// 	int sys_state = 0;
	// 	int usb_sum;
	// 	struct _switch_dev_id   id;
	// 	struct _machine_s   machine;
	// 	uint8 s_name[50] = { 0 };
	// 	//int s_type;
	// 	uint8 s_date[10] = { 0 };
	// 	uint8 s_version[100] = { 0 };
	// 	uint8 s_writer[50] = { 0 };
	// 	uint8 s_note[50] = { 0 };
	// 	uint8 s_code[20] = { 0 };
	// 	//char mach_type[12] = { 0 };
	// 	char machine_id[16] = { 0 };
	// 	//_lock_set(par->lock);
    //     memset(s_buf,0,sizeof(s_buf));
    //     //module_out("=====================��ȡ����������========================\n");
	// 	//sys_state = ty_read(par->ty_pd_fd,NULL,0);
	// 	if(sys_state == 0)
	// 	{
	// 		s_buf[0]=0x00;
	// 		memcpy(s_buf+1,"ϵͳ����",8);      
	// 	}
    //     else
    //     {   
	// 		s_buf[0]=0x01;
	// 		memcpy(s_buf+1,"ϵͳ�쳣",8);
    //     }
	// 	memcpy(s_buf + 51, DF_TY_1502_HARD_EDIT, strlen(DF_TY_1502_HARD_EDIT));
    //     result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_KERNEL,s_buf+51+8);
	// 	if(result < 0)
	// 	{	
	// 		module_out("����ں���Ϣresult = %d\n",result);
	// 	}

    //     //module_out("�������汾\n");
    //     result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_SOFT,s_buf+51+38);
	// 	if(result < 0)
	// 	{	
	// 		module_out("�������汾result = %d\n",result);
	// 	}

    //     //module_out("����������\n");       
    //     memset(&id,0,sizeof(id));
    //     result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
    //     bcd2asc(id.id,s_buf+46+51,6);
	// 	if(result < 0)
	// 	{	
	// 		module_out("get_sev_pro����������result = %d\n",result);
	// 	}

    //     //module_out("���ip\n");
    //     ip=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_IP);
    //     s_buf[58+51]=((ip>>24)&0xff);s_buf[58+52]=((ip>>16)&0xff);
    //     s_buf[58+53]=((ip>>8)&0xff);s_buf[58+54]=((ip>>0)&0xff);


    //     //module_out("����豸��ʼ��\n");
    //     start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
    //     s_buf[62+51]=((start>>24)&0xff);s_buf[63+51]=((start>>16)&0xff);
    //     s_buf[64+51]=((start>>8)&0xff);s_buf[65+51]=((start>>0)&0xff);
    //     //module_out("���USB�˿�����\n");
        
    //     usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
    //     s_buf[66+51]=((usb_sum>>24)&0xff);s_buf[67+51]=((usb_sum>>16)&0xff);
    //     s_buf[68+51]=((usb_sum>>8)&0xff);s_buf[69+51]=((usb_sum>>0)&0xff);           
    //     //module_out("�������˿�\n");
	// 	s_buf[70 + 51] = ((DF_TY_SEV_ORDER_PORT >> 8) & 0xff);
	// 	s_buf[71 + 51] = ((DF_TY_SEV_ORDER_PORT >> 0) & 0xff);
    //     //module_out("���USBIP�˿�\n");
    //     int usbip_port = 3240;
	// 	s_buf[72+51]=((usbip_port>>8)&0xff);
	// 	s_buf[73+51]=((usbip_port>>0)&0xff);
    //     //module_out("�����������\n");
    //     time_asc(id.date,s_buf+51+74,2);
	// 	/////////////////////////////////////////////////////////////////////////////////
    //     //module_out("�������״̬\n");
    //     result=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_REGISTER_WHETHER);
	// 	if(result == 0)
	// 	{	
	// 		module_out("δע��������\n");
	// 		s_buf[135]=0x00;
	// 		memcpy(s_buf+136,"δע��",6);
	// 	}
	// 	else if(result == 1)
	// 	{	
	// 		module_out("��ע�����������Ѽ���\n");
	// 		s_buf[135]=0x01;
	// 		memcpy(s_buf+136,"������",6);
	// 	}
	// 	else if(result == 2)
	// 	{	
	// 		module_out("��ע�������ŵ�δ����\n");
	// 		s_buf[135]=0x00;
	// 		result= ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_TRYOUT_WHETHER);
	// 		if(result == 0)
	// 		{	module_out("û�г�����������\n");
	// 			memset(trydate,0x00,sizeof(trydate));
	// 			ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_TRY_TIME,trydate);
	// 			memcpy(s_buf+136,trydate,10);
	// 		}
	// 		else
	// 		{	
	// 			memcpy(s_buf+136,"�ѹ���",6);
	// 			module_out("������������\n");
	// 		}	
			
	// 	}
	// 	////////////////////////////////////////////////////////////////////////////////////

	// 	memset(&machine,0,sizeof(machine));
	// 	ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_MACHINE_S,&machine);
	// 	memcpy(s_buf+100+50,machine.hard,sizeof(machine.hard));
	// 	memcpy(s_buf+100+8+50,machine.soft,sizeof(machine.soft));
	// 	bcd2asc(machine.id,s_buf+100+8+8+50,6);
	// 	machine.n=usb_sum;
	// 	s_buf[100+8+8+12+50]=((machine.n>>8)&0xff);
	// 	s_buf[100+8+8+12+1+50]=((machine.n>>0)&0xff); 
	// 	//	_lock_un(par->lock);
	// 	//memset(s_buf,0,sizeof(s_buf));
	// 	memset(s_name,0,sizeof(s_name));
	// 	memset(s_date,0,sizeof(s_date));
	// 	memset(s_version,0,sizeof(s_version));
	// 	memset(s_writer,0,sizeof(s_writer));
	// 	memset(s_note,0,sizeof(s_note));
	// 	memset(s_code,0,sizeof(s_code));
	// 	sprintf((char *)s_name, "%s", app_note.name);
	// 	module_out("�������Ϊ��%s\n",s_name);
	// 	sprintf((char *)s_date, "%s", app_note.r_date);
	// 	module_out("�������Ϊ��%s\n",s_date);
	// 	sprintf((char *)s_version, "%s_%s_%s", app_note.ver, app_note.r_date, app_note.abdr);
	// 	module_out("����汾Ϊ��%s\n",s_version);
	// 	sprintf((char *)s_writer, "%s_%s", app_note.cpy, app_note.writer);
	// 	module_out("�������Ϊ��%s\n",s_writer);
	// 	sprintf((char *)s_note, "%s %s", app_note.note, app_note.type);
	// 	module_out("�������Ϊ��%s\n",s_note);
	// 	sprintf((char *)s_code, "%s", app_note.code);
	// 	module_out("�������Ϊ��%s\n",s_code);
	// 	//s_type=0x01;
	// 	sprintf((char *)s_buf + 180, "%s", s_name);
	// 	//sprintf(s_buf+50,"%d",s_type);
	// 	s_buf[230]=0x01;
	// 	sprintf((char *)s_buf + 231, "%s", s_date);
	// 	sprintf((char *)s_buf + 241, "%s", s_version);
	// 	sprintf((char *)s_buf + 341, "%s", s_writer);
	// 	sprintf((char *)s_buf + 391, "%s", s_note);
	// 	sprintf((char *)s_buf+441,"%s",s_code);

	// 	bcd2asc(id.id,(uint8 *)machine_id,6);
	// 	memcpy(s_buf + 461, app_note.type, strlen(app_note.type));
    //     return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_SEV_PRO,s_buf,471);  
    // }
   
       static int get_sev_pro(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)    
	{
		uint8 s_buf[1024] = { 0 };
		uint8 trydate[10] = { 0 };
		int result;
        int start;
		//int i;
		int sys_state = 0;
		int usb_sum;
		struct _switch_dev_id   id;
		struct _machine_s   machine;
		uint8 s_name[50] = { 0 };
		//int s_type;
		uint8 s_date[10] = { 0 };
		uint8 s_version[100] = { 0 };
		uint8 s_writer[50] = { 0 };
		uint8 s_note[50] = { 0 };
		uint8 s_code[20] = { 0 };
		//char mach_type[12] = { 0 };
		char machine_id[16] = { 0 };
		//_lock_set(par->lock);
        memset(s_buf,0,sizeof(s_buf));
        //module_out("=====================��ȡ����������========================\n");
		//sys_state = ty_read(par->ty_pd_fd,NULL,0);
		if(sys_state == 0)
		{
			s_buf[0]=0x00;
			memcpy(s_buf+1,"ϵͳ����",8);      
		}
        else
        {   
			s_buf[0]=0x01;
			memcpy(s_buf+1,"ϵͳ�쳣",8);
        }
		//module_out("���Ӳ���汾\n");
		memcpy(s_buf + 51, DF_TY_1502_HARD_EDIT, strlen(DF_TY_1502_HARD_EDIT));
        /*result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_HARD,s_buf+51);
		if(result < 0)
		{	module_out("���Ӳ���汾ʧ��result = %d\n",result);
		}*/

        //module_out("����ں���Ϣ\n");
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_KERNEL,s_buf+51+8);
		if(result < 0)
		{	
			module_out("����ں���Ϣresult = %d\n",result);
		}

        //module_out("�������汾\n");
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_SOFT,s_buf+51+38);
		if(result < 0)
		{	
			module_out("�������汾result = %d\n",result);
		}

        //module_out("����������\n");       
        memset(&id,0,sizeof(id));
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
        bcd2asc(id.id,s_buf+46+51,6);
		if(result < 0)
		{	
			module_out("get_sev_pro����������result = %d\n",result);
		}

        //module_out("���ip\n");
        ip=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_IP);
        s_buf[58+51]=((ip>>24)&0xff);s_buf[58+52]=((ip>>16)&0xff);
        s_buf[58+53]=((ip>>8)&0xff);s_buf[58+54]=((ip>>0)&0xff);


        //module_out("����豸��ʼ��\n");
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        s_buf[62+51]=((start>>24)&0xff);s_buf[63+51]=((start>>16)&0xff);
        s_buf[64+51]=((start>>8)&0xff);s_buf[65+51]=((start>>0)&0xff);
        //module_out("���USB�˿�����\n");
        
        usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[66+51]=((usb_sum>>24)&0xff);s_buf[67+51]=((usb_sum>>16)&0xff);
        s_buf[68+51]=((usb_sum>>8)&0xff);s_buf[69+51]=((usb_sum>>0)&0xff);           
        //module_out("�������˿�\n");
		s_buf[70 + 51] = ((DF_TY_SEV_ORDER_PORT >> 8) & 0xff);
		s_buf[71 + 51] = ((DF_TY_SEV_ORDER_PORT >> 0) & 0xff);
        //module_out("���USBIP�˿�\n");
        int usbip_port = 3240;
	s_buf[72+51]=((usbip_port>>8)&0xff);
	s_buf[73+51]=((usbip_port>>0)&0xff);
        //module_out("�����������\n");
        time_asc(id.date,s_buf+51+74,2);
		/////////////////////////////////////////////////////////////////////////////////
        //module_out("�������״̬\n");
        result=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_REGISTER_WHETHER);
		if(result == 0)
		{	
			//module_out("δע��������\n");
			s_buf[135]=0x00;
			memcpy(s_buf+136,"δע��",6);
		}
		else if(result == 1)
		{	
			//module_out("��ע�����������Ѽ���\n");
			s_buf[135]=0x01;
			memcpy(s_buf+136,"������",6);
		}
		else if(result == 2)
		{	
			//module_out("��ע�������ŵ�δ����\n");
			s_buf[135]=0x00;
			result= ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_TRYOUT_WHETHER);
			if(result == 0)
			{	//module_out("û�г�����������\n");
				memset(trydate,0x00,sizeof(trydate));
				ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_TRY_TIME,trydate);
				memcpy(s_buf+136,trydate,10);
			}
			else
			{	
				memcpy(s_buf+136,"�ѹ���",6);
			}	
			
		}
		////////////////////////////////////////////////////////////////////////////////////

		memset(&machine,0,sizeof(machine));
		//ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_MACHINE_S,&machine);
		memcpy(s_buf+100+50,machine.hard,sizeof(machine.hard));
		memcpy(s_buf+100+8+50,machine.soft,sizeof(machine.soft));
		bcd2asc(machine.id,s_buf+100+8+8+50,6);
		machine.n=usb_sum;
		s_buf[100+8+8+12+50]=((machine.n>>8)&0xff);
		s_buf[100+8+8+12+1+50]=((machine.n>>0)&0xff); 
		//	_lock_un(par->lock);



		
		//memset(s_buf,0,sizeof(s_buf));
		memset(s_name,0,sizeof(s_name));
		memset(s_date,0,sizeof(s_date));
		memset(s_version,0,sizeof(s_version));
		memset(s_writer,0,sizeof(s_writer));
		memset(s_note,0,sizeof(s_note));
		memset(s_code,0,sizeof(s_code));
		sprintf((char *)s_name, "%s", app_note.name);
		//module_out("�������Ϊ��%s\n",s_name);
		sprintf((char *)s_date, "%s", app_note.r_date);
		//module_out("�������Ϊ��%s\n",s_date);
		sprintf((char *)s_version, "%s_%s_%s", app_note.ver, app_note.r_date, app_note.abdr);
		//module_out("����汾Ϊ��%s\n",s_version);
		sprintf((char *)s_writer, "%s_%s", app_note.cpy, app_note.writer);
		//module_out("�������Ϊ��%s\n",s_writer);
		sprintf((char *)s_note, "%s %s", app_note.note, app_note.type);
		//module_out("�������Ϊ��%s\n",s_note);
		sprintf((char *)s_code, "%s", app_note.code);
		//module_out("�������Ϊ��%s\n",s_code);
		//s_type=0x01;
		sprintf((char *)s_buf + 180, "%s", s_name);
		//sprintf(s_buf+50,"%d",s_type);
		s_buf[230]=0x01;
		sprintf((char *)s_buf + 231, "%s", s_date);
		sprintf((char *)s_buf + 241, "%s", s_version);
		sprintf((char *)s_buf + 341, "%s", s_writer);
		sprintf((char *)s_buf + 391, "%s", s_note);
		sprintf((char *)s_buf+441,"%s",s_code);


		bcd2asc(id.id,(uint8 *)machine_id,6);
		//if(result < 0)
		//{	module_out("err ����������result = %d\n",result);
		//}
		//module_out("�������Ϊ��%s\n",machine_id);
		/*memset(mach_type,0,sizeof(mach_type));
		
		if ((machine_id[3] == 0x30) && (machine_id[4] == 0x31 || machine_id[4] == 0x32 || machine_id[4] == 0x33))
		{
			sprintf(mach_type,"MB_16");
			memcpy(mach_type+5,machine_id+3,2);
		}
		else if((machine_id[3] == 0x30) && (machine_id[4] == 0x34))
		{
			sprintf(mach_type,"MB_1702");
		}
		else if((machine_id[3] == 0x30) && (machine_id[4] == 0x35))
		{
			sprintf(mach_type,"MB_1703");
		}		
		else if((machine_id[3] == 0x30) && (machine_id[4] == 0x37))
		{
			sprintf(mach_type,"MB_1805");
		}
		else if((machine_id[3] == 0x30) && (machine_id[4] == 0x38))
		{
			sprintf(mach_type,"MB_1806");
		}
		else if ((machine_id[3] == 0x31) && (machine_id[4] == 0x31))
		{
			sprintf(mach_type, "MB_1902");
		}
		else if ((machine_id[3] == 0x31) && (machine_id[4] == 0x32))
		{
			sprintf(mach_type, "MB_1906");
		}
		if(machine_id[5]!=0x30)
		{
			memset(mach_type+7,machine_id[5]+0x10,1);
		}
		memcpy(s_buf+461,mach_type,10);*/
		memcpy(s_buf + 461, app_note.type, strlen(app_note.type));
		// module_out("Ҫ���͵�����Ϊ��");
		// int i = 0;
		// for(i=0;i<471;i++)
		// {
		// 	printf("%02x ",s_buf[i]);
		// }
		// printf("\n");


        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_SEV_PRO,s_buf,471);  
    }
   
   
    //��ȡ������״̬
    static int get_sev_status(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
    {   
		//struct _usb_port_state      state;
		uint8 s_buf[2048] = { 0 };
        int len;
		int sys_state = 0;
        //memset(&state,0,sizeof(state));
        memset(s_buf,0,sizeof(s_buf));
        module_out("================���ϵͳ״̬�Լ�״̬����=======================\n");
		//sys_state = ty_read(par->ty_pd_fd,NULL,0);
		if(sys_state == 0)
		{
			s_buf[0]=0x00;
			memcpy(s_buf+1,"ϵͳ����",8);      
		}
        else
        {   
			s_buf[0]=0x01;
			memcpy(s_buf+1,"ϵͳ�쳣",8);
        }
        //ty_ctl(par->ty_usb_m_fd,DF_TY_USB_M_CM_GET_STATE,&state);
        s_buf[51]=((0>>8)&0xff);s_buf[52]=((0>>0)&0xff);      //�Ѿ������ϵĿͻ�����


        len=53;
        s_buf[len++]=((0>>8)&0xff);s_buf[len++]=((0>>0)&0xff);    //�Ѿ��򿪵�USB��

        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_SEV_STATE,s_buf,len); 
    }

	int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info)
	{
		DEV_BASIC_DATA *state = NULL;
		USB_PORT_CLASS *usb_port_class = get_usb_port_class();
		int tmp_port_num = port_num > 0?(port_num - 1):0;
		state = &(usb_port_class->get_dev_basic_action.usb_dev_basic_data[tmp_port_num]);
		//update_plate_info_cbqkzt(state);
		//ty_usb_m_out("�˿ںţ�%d,��Դ״̬��%d,�Ƿ�����豸��%d\n", state->port_info.port, state->port_info.power, state->port_info.with_dev);
		memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
		port_info->port_power = state->power;
		port_info->port = state->port;//�˿ڽṹ����Ϣ����״̬�ṹ����ܲ�ͬ��
		//  _lock_set(state->lock);
		if (state->h_plate.hDev != NULL)
		{
			port_info->connect_success_count = state->h_plate.hDev->connect_success_count;
			port_info->connect_failed_count = state->h_plate.hDev->connect_failed_count;
			port_info->connect_success_time = state->h_plate.hDev->connect_success_time;
			port_info->connect_failed_time = state->h_plate.hDev->connect_failed_time;
			memset(port_info->start_time_s, 0, sizeof(port_info->start_time_s));
			memset(port_info->end_time_s, 0, sizeof(port_info->end_time_s));
			//memset(port_info->connect_errinfo, 0, sizeof(port_info->connect_errinfo));
			memset(port_info->tax_business, 0, sizeof(port_info->tax_business));
			strcpy(port_info->start_time_s, state->h_plate.hDev->DevHttp.start_time_s);
			strcpy(port_info->end_time_s, state->h_plate.hDev->DevHttp.end_time_s);
			//if ((state->h_plate.hDev->hUSB != NULL) && (state->h_plate.hDev->hUSB->errcode == ErrorNET))
			//	strcpy(port_info->connect_errinfo, state->h_plate.hDev->hUSB->errinfo);
			strcpy(port_info->tax_business, state->port_info.tax_business);
		}
		else
		{
			port_info->connect_success_count = 0;
			port_info->connect_failed_count = 0;
			port_info->connect_success_time = 0;
			port_info->connect_failed_time = 0;
			memset(port_info->start_time_s, 0, sizeof(port_info->start_time_s));
			memset(port_info->end_time_s, 0, sizeof(port_info->end_time_s));
			//memset(port_info->connect_errinfo, 0, sizeof(port_info->connect_errinfo));
			memset(port_info->tax_business, 0, sizeof(port_info->tax_business));
			strcpy(port_info->tax_business, state->port_info.tax_business);
		 }
		//  _lock_un(state->lock);
		port_info->ad_status = state->port_info.ad_status;

		return 0;
	}
	//��ѯ�˿�״̬
    static int get_port_pro(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
    {   
        int result,start,port;
		uint8 s_buf[1024] = { 0 };
		uint8 dev_id[6];
		uint8 sub_id[6];
		char drivers[DF_TY_PATH_MAX];
		int port_dev;               //�ڻ�������ı��
		int port_sub;               //��������ı��
		struct _usb_file_dev    dev;
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        port=(port-start+1);

		struct _usb_port_infor port_info;
		memset(&port_info, 0, sizeof(struct _usb_port_infor));
		result = fun_get_plate_usb_info(port, &port_info);

        if(result<0)
        {   
			module_out("result=%d\n",result);
            return err_out_tcp(fd,result);
        }
        memset(s_buf,0,sizeof(s_buf));
		s_buf[0] = port_info.port_status;//USB�˿�״̬

		port_dev = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_GET_PORT_MA_ID, port, dev_id);
        bcd2asc(dev_id,s_buf+1,6);
        s_buf[13]=((port_dev>>8)&0xff);s_buf[14]=((port_dev>>0)&0xff);

		port_sub = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_GET_PORT_PCB_ID, port, sub_id);
        bcd2asc(sub_id,s_buf+15,6);
        s_buf[27]=((port_sub>>8)&0xff);s_buf[28]=((port_sub>>0)&0xff);

		memcpy(s_buf + 29, port_info.busid, strlen(port_info.busid));

		
		if (port_info.with_dev != 0)
		{
			memset(&dev, 0, sizeof(dev));
			_usb_file_get_infor(port_info.busid, &dev);
			memcpy(drivers, dev.interface[0].driver, strlen(dev.interface[0].driver));
		}

        memcpy(s_buf+49,drivers,strlen(drivers));
		s_buf[99] = ((port_info.ip >> 24) & 0xff);
		s_buf[100] = ((port_info.ip >> 16) & 0xff);
		s_buf[101] = ((port_info.ip >> 8) & 0xff);
		s_buf[102] = ((port_info.ip >> 0) & 0xff);
		memcpy(s_buf + 103, port_info.client, sizeof(port_info.client));
		return write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_PRO, s_buf, 103 + sizeof(port_info.client));
    }

	//��ѯ�ն�������֤����˰������
	static int get_all_port_ca_name(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		int result;
		uint8 s_buf[15600] = { 0 };
		int i,usb_sum;
		int offset=0;
		int len;
		memset(s_buf,0,sizeof(s_buf));
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		for(i=1;i<=usb_sum;i++)
		{
			s_buf[offset+1]=i;
			//module_out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
			memset(&usb_t,0,sizeof(struct _usb_port_infor));
			result = fun_get_plate_usb_info(i, &usb_t);
			if(result < 0)
			{
				s_buf[offset+2]=0;
				offset+=153;
				continue;
			}
			if(usb_t.with_dev == 0)
			{	
				s_buf[offset+2]=0;
				offset+=153;
				continue;
			}
			if(usb_t.port_status == 0)
			{	
				s_buf[offset+2]=1;
			}
			else
			{	
				s_buf[offset+2]=2;
			}			
			s_buf[offset+3]=usb_t.ca_ok;
			memcpy(s_buf+offset+4,usb_t.ca_name,sizeof(usb_t.ca_name));
			memcpy(s_buf+offset+4+130,usb_t.ca_serial,sizeof(usb_t.ca_serial));
			offset+=153;
			continue;
		}
		len = usb_sum*153+1;
		//module_out("Ҫ���͵����ݳ���Ϊ%d��\n",len);
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_CA_NAME,s_buf,len); 		
	}

	//��ѯ���ж˿��豸���ͺ�֤��
	static int get_all_port_type(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		int result;
		uint8 s_buf[15600] = { 0 };
		int i,usb_sum;
		int offset=0;
		int len;
		struct _switch_dev_id   id;
		memset(s_buf,0,sizeof(s_buf));
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		 //module_out("����������\n");       
        memset(&id,0,sizeof(id));
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
        bcd2asc(id.id,s_buf+1,6);
		if(result < 0)
		{	
			module_out("get_sev_pro����������result = %d\n",result);
		}
		for(i=1;i<=usb_sum;i++)
		{
			s_buf[offset+13]=i;
			//module_out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
			result = fun_get_plate_usb_info(i, &usb_t);
			if(result < 0)
			{
				offset+=157;
				continue;
			}
			s_buf[offset+14]=usb_t.dev_type;
			s_buf[offset+15]=((usb_t.vid>>8)&0xff);s_buf[offset+16]=((usb_t.vid>>0)&0xff);
			s_buf[offset+17]=((usb_t.pid>>8)&0xff);s_buf[offset+18]=((usb_t.pid>>0)&0xff);
			s_buf[offset+19]=usb_t.ca_ok;
			memcpy(s_buf+offset+20,usb_t.ca_name,sizeof(usb_t.ca_name));
			s_buf[offset + 130] = ((usb_t.usb_app >> 8) & 0xff); s_buf[offset + 131] = ((usb_t.usb_app >> 0) & 0xff);
			memcpy(s_buf+offset+132,usb_t.sn,sizeof(usb_t.sn));
			s_buf[offset+148]=((usb_t.extension>>8)&0xff);s_buf[offset+149]=((usb_t.extension>>0)&0xff);
			memcpy(s_buf+offset+150,usb_t.ca_serial,sizeof(usb_t.ca_serial));
			offset+=157;
			continue;
		}
		len = usb_sum*157+1+12;
		//module_out("Ҫ���͵����ݳ���Ϊ%d��\n",len);
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_TYPE,s_buf,len); 		
	}

	static int get_all_port_status(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		USB_PORT_CLASS *usb_port_class = get_usb_port_class();
		int result;
		uint8 s_buf[1024] = { 0 };
		int i,usb_sum;
		memset(s_buf,0,sizeof(s_buf));
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		// printf("------------------- lbc usb_sum=%d\n",usb_sum);
		for(i=1;i<=usb_sum;i++)
		{
			//module_out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
			result = fun_get_plate_usb_info(i, &usb_t);
			if(result<0)
			{   //module_out("��ȡUSB�˿���Ϣʧ��result=%d\n",result);
				s_buf[i]=0;
				continue;
			}
			if(WITHOUT_DEV == usb_port_class->judge_usbport_exist.port_exist_state[i - 1].now_port_state)
			{
				s_buf[i]=0;
				continue;
			}
			else
			{
				s_buf[i]=1;
				continue;
			}
			// if(usb_t.with_dev == 0)
			// {	
			// 	s_buf[i]=0;
			// 	continue;
			// }
			// if (usb_t.port_status == 0)
			// {	
			// 	s_buf[i]=1;
			// 	continue;
			// }
			// else
			// {	
			// 	s_buf[i]=2;
			// 	continue;
			// }
			s_buf[i]=0;			
		}

		// printf("get_all_port_status\n");
		// for(i = 0;i<256;++i)
		// {
		// 	printf(" %02x",s_buf[i]);
		// }
		// printf("\n");
		// module_out("Ҫ���͵�����Ϊ��s_b");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR,s_buf,256); 		
	}

	static int get_ter_soft_version(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{	//int result;
		uint8 s_buf[1024] = { 0 };
		//int i,usb_sum;
		uint8 s_name[50] = { 0 };
		//int s_type;
		uint8 s_date[10] = { 0 };
		uint8 s_version[100] = { 0 };
		uint8 s_writer[50] = { 0 };
		uint8 s_note[50] = { 0 };
		uint8 s_code[20] = { 0 };
		memset(s_buf,0,sizeof(s_buf));
		memset(s_name,0,sizeof(s_name));
		memset(s_date,0,sizeof(s_date));
		memset(s_version,0,sizeof(s_version));
		memset(s_writer,0,sizeof(s_writer));
		memset(s_note,0,sizeof(s_note));
		memset(s_code,0,sizeof(s_code));
		sprintf((char *)s_name, "%s", app_note.name);
		module_out("�������Ϊ��%s\n",s_name);
		sprintf((char *)s_date, "%s", app_note.r_date);
		module_out("�������Ϊ��%s\n",s_date);
		sprintf((char *)s_version, "%s_%s_%s", app_note.ver, app_note.r_date, app_note.abdr);
		module_out("����汾Ϊ��%s\n",s_version);
		sprintf((char *)s_writer, "%s_%s", app_note.cpy, app_note.writer);
		module_out("�������Ϊ��%s\n",s_writer);
		sprintf((char *)s_note, "%s %s", app_note.note, app_note.type);
		module_out("�������Ϊ��%s\n",s_note);
		sprintf((char *)s_code, "%s", app_note.code);
		module_out("�������Ϊ��%s\n",s_code);
		//s_type=0x01;
		sprintf((char *)s_buf, "%s", s_name);
		//sprintf(s_buf+50,"%d",s_type);
		s_buf[50]=0x01;
		sprintf((char *)s_buf + 51, "%s", s_date);
		sprintf((char *)s_buf + 61, "%s", s_version);
		sprintf((char *)s_buf + 161, "%s", s_writer);
		sprintf((char *)s_buf + 211, "%s", s_note);
		sprintf((char *)s_buf + 261, "%s", s_code);
		//module_out("Ҫ���͵�����Ϊ��");
		//for(i=0;i<281;i++)
		//{
		//	printf("%02x ",s_buf[i]);
		//}
		//printf("\n");
		////module_out("Ҫ���͵�����Ϊ��");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR,s_buf,281); 		
	}

	//��ѯUSB�豸��ϸ��Ϣ
	static int get_usb_dev_info(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
	{
		struct _usb_port_infor usb_t;
		int result,port;
		uint8 s_buf[1024] = { 0 };
		uint8 code_num[2] = { 0 };
		uint8 guid[36] = { 0 };
		//int i;
		port=buf[1]*0x10000+buf[2]*0x100+buf[3];    
		memcpy(code_num,buf+4,2);
		memcpy(guid,buf+6,36);
		memset(&usb_t,0,sizeof(usb_t));
		//module_out("get_usb_dev_info ��ѯ�ն�port��%d�Ŷ˿���Ϣ\n",port);
		result = fun_get_plate_usb_info(port, &usb_t);
		if(result<0)
		{   
			module_out("get_usb_dev_info ��ȡUSB�˿���Ϣʧ��result=%d\n",result);
			return err_out_tcp(fd,result);
		} 
		memset(s_buf,0,sizeof(s_buf));
		s_buf[0]=usb_t.with_dev;//USB�˿�״̬
		s_buf[1] = usb_t.port_status;//USB�˿�״̬
		memcpy(s_buf+2,usb_t.busid,strlen(usb_t.busid));
		s_buf[22]=usb_t.dev_type;
		s_buf[23]=((usb_t.protocol_ver>>8)&0xff);s_buf[24]=((usb_t.protocol_ver>>0)&0xff);
		s_buf[25]=((usb_t.vid>>8)&0xff);s_buf[26]=((usb_t.vid>>0)&0xff);
		s_buf[27]=((usb_t.pid>>8)&0xff);s_buf[28]=((usb_t.pid>>0)&0xff);
		s_buf[29]=usb_t.bnuminterface;
		//memcpy(s_buf+30,usb_t.dev_name,sizeof(usb_t.dev_name));
		s_buf[80]=((usb_t.ip>>24)&0xff);s_buf[81]=((usb_t.ip>>16)&0xff);
		s_buf[82]=((usb_t.ip>>8)&0xff);s_buf[83]=((usb_t.ip>>0)&0xff);
		memcpy(s_buf+84,usb_t.client,38);
	/*	module_out("Ҫ���͵�����Ϊ��");
		for(i=0;i<148;i++)
		{
			printf("%02x ",s_buf[i]);
		}
		printf("\n");*/
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR,s_buf,148); 
	}

    //��ȡ����������
    static int get_sev_par(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[1024] = { 0 }, t_timer[8] = { 0 };
		module_out("===============��ȡ�������=============\n");
        memset(s_buf,0,sizeof(s_buf));
        memset(t_timer,0,sizeof(t_timer));
		module_out("��ȡ��ǰʱ��\n");
        clock_get(t_timer);
		module_out("ת����ǰʱ��\n");
        time_asc(t_timer,s_buf,5);
        s_buf[19]=1;
        module_out("�Ƿ�ע��:%d\n",s_buf[19]);
        s_buf[20]=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_PORT_N);
        module_out("�˿����й���:%d\n",s_buf[20]);
        s_buf[21]=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_ENUM_TIMER);
        module_out("USBö��ʱ��:%d\n",s_buf[21]);
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_BUSID,s_buf+22);
        module_out("USB������ַ:%s\n",s_buf+22);
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_SOFT,s_buf+42);
        module_out("HTTP������ַ:%s\n",s_buf+42);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_PAR,s_buf,142);  
    }
	 //��ȡ������������ַ
    static int get_update_address(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[1024] = { 0 };
		//module_out("===============��ȡ������������ַ=============\n");
		memset(s_buf,0,sizeof(s_buf));
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_ADDR,s_buf);
        //module_out("HTTP������ַ:%s\n",s_buf);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_UPDATE_ADDR,s_buf,100);  
    }
    //��ȡ�������쳣��־
    static int get_log(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[10240] = { 0 };
        struct _switch_event_all    all;
        int i;
        memset(&all,0,sizeof(all));
        memset(s_buf,0,sizeof(s_buf));
        //ty_ctl(par->event_file_fd,DF_EVENT_CM_READ,&all);
        module_out("��־����:all.n=%d\n",all.n);
        // for(i=0;i<all.n;i++)
		// {
		// 	sprintf((char *)s_buf + strlen((char *)s_buf), "%d.[ʱ��]:", i + 1);
		// 	time_asc(all.event[i].clock, s_buf + strlen((char *)s_buf), 4);
		// 	sprintf((char *)s_buf + strlen((char *)s_buf), "[�¼�]:");
        //     module_out("�¼�:%d\n",all.event[i].event);
		// 	ty_ctl(par->event_file_fd, DF_EVENT_CM_EVENT_NOTE, all.event[i].event, s_buf + strlen((char *)s_buf));
		// 	sprintf((char *)s_buf + strlen((char *)s_buf), "[ԭ��]:");
        //     module_out("ԭ��:%d\n",all.event[i].reason);
		// 	ty_ctl(par->event_file_fd, DF_EVENT_CM_REASON_NOTE, all.event[i].reason, s_buf + strlen((char *)s_buf));
		// 	sprintf((char *)s_buf + strlen((char *)s_buf), "\n");
        //     //module_out("��־:[%s]\n",s_buf);    
        // }
		// module_out("��־:[%s]\n",s_buf);   
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_LOG,s_buf,strlen((char *)s_buf)); 
    }
	//��ȡ�������
    static int get_net_par(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[256] = { 0 };
        uint32 mask,getway,dns;
		int type;
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_PAR,&ip,&mask,&getway,&dns,&type);
        //module_out("��ѯ��:ip=0X%08X,mask=0X%08X,getway=0X%08X\n,dns=0x%08x,type=%d\n",ip,mask,getway,dns,type);
        memset(s_buf,0,sizeof(s_buf));
		s_buf[0]=type;
        s_buf[1]=((ip>>24)&0xff);s_buf[2]=((ip>>16)&0xff);
        s_buf[3]=((ip>>8)&0xff);s_buf[4]=((ip>>0)&0xff);
        s_buf[5]=((mask>>24)&0xff);s_buf[6]=((mask>>16)&0xff);
        s_buf[7]=((mask>>8)&0xff);s_buf[8]=((mask>>0)&0xff);
        s_buf[9]=((getway>>24)&0xff);s_buf[10]=((getway>>16)&0xff);
        s_buf[11]=((getway>>8)&0xff);s_buf[12]=((getway>>0)&0xff);
		s_buf[13]=((dns>>24)&0xff);s_buf[14]=((dns>>16)&0xff);
        s_buf[15]=((dns>>8)&0xff);s_buf[16]=((dns>>0)&0xff);
        //module_out("����ִ�����\n");
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_NET_PAR,s_buf,17); 
		
    }
    //����ն��Ƿ�����
	static int Key_Check_Http(struct http_parm * parm)
	{
		int sock = 0;
		int result;
		struct timeval tv;
		result = http_socket_open(&sock, parm);
		if (result <0)
		{
			printf("socket create failed\n");//socket ����ʧ��ֱ�ӷ���

			return -1;
		}
		result = http_get_token(sock, parm);
		if (result < 0)
		{
			printf("http get token err!\n");
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -1;
		}

		gettimeofday(&tv, NULL);
		memset(parm->timestamp, 0, sizeof(parm->timestamp));
		sprintf((char*)parm->timestamp, "%ld", tv.tv_sec);


		result = http_send_sign_enc(sock, parm);
		if (result < 0)
		{
			printf("http_send_sign_enc err!\n");
			shutdown(sock, SHUT_RDWR);
			close(sock);
			return -1;
		}
		shutdown(sock, SHUT_RDWR);
		close(sock);
		return 0;
	}

	static int get_check_all_port(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
	{
		//struct _usb_port_state      state;
		uint8 s_buf[2048] = { 0 };
		//uint8 a_buf[50];
        int i,j=0;
		int port_num;
		int result = 0;
		int err_num=0;
		int len; 
		struct http_parm parm;
		char baseudid[100] = { 0 };
		struct _switch_dev_only 	udid;
		struct _usb_port_infor port_info;
        //memset(&state,0,sizeof(state));
        memset(s_buf,0,sizeof(s_buf));
		memset(&parm, 0, sizeof(struct http_parm));
        module_out("================���ϵͳ״̬�Լ�״̬����=======================\n");
		// port_num=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);//��ȡ�ն˶˿�����
		port_num = get_usb_port_class()->usb_port_numb;
		module_out("���ն�һ����%d��USB�˿�\n", port_num);
		for(i=0;i<port_num;i++)
		{
			result = *(int *)(get_usb_port_class()->fun_check_port_with_num((void *)(&i)));
			if (result != 0)
			{	
				module_out("%d�Ŷ˿�û���豸\n",i+1);
				sprintf((char *)s_buf+2+j*20,"%d��USB�˿��쳣",i+1);
				j++;
				err_num = err_num+1;
			}
		}
		result = get_DNS_ip("www.njmbxx.com", parm.ip);
		if (result < 0)
		{
			module_out("get_DNS_ip  failed\n");
			return -1;
		}

		//module_out("get_check_all_port  11111\n");
		ty_ctl(par->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, parm.key_s);
		//module_out("get_check_all_port  22222\n");
		result = ty_ctl(par->switch_fd, DF_SWITCH_CM_GET_SERIAL_NUM, &udid);
		//module_out("get_check_all_port  33333\n");
		if (result<0)
		{
			module_out("��ȡΨһ��ʶ��ʧ��\n");
			sprintf((char *)s_buf + 2 + j * 20, "��ȡΨһ��ʶ��ʧ��");
			err_num = err_num + 1;
		}
		//memcpy(udid_num, udid.serial_num, sizeof(udid_num));//��ȡ��Ψһ��ʶ��
		memcpy(parm.ter_id, "TERREGENCCKECK", 14);
		memcpy(parm.appid, "TERREGENCCKECK", 14);
		memset(baseudid, 0, sizeof(baseudid));
		encode((char *)udid.serial_num, 12, (char *)baseudid);
		memcpy(parm.cpu_id, baseudid, 16);
		memcpy(parm.app_ver, app_note.ver, strlen(app_note.ver));
		//module_out("get_check_all_port  44444\n");
		result = Key_Check_Http(&parm);
		//module_out("get_check_all_port  55555\n");
		//result = 0;
		if (result<0)
		{
			sprintf((char *)s_buf + 2 + j * 20, "��ԿУ��ʧ��");
			err_num = err_num + 1;
		}
		if(err_num==0)
		{	
			module_out("err_num=0,û���쳣\n");
			s_buf[0]=0x00;
			s_buf[1]=0x00;
			write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CHECK,s_buf,2);
		}
		s_buf[0]=0x01;
		s_buf[1]=err_num;
		len = err_num*20+2;
		//module_out("get_check_all_port  66666\n");
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CHECK,s_buf,len); 
		//module_out("get_check_all_port  77777\n");
	}


	// static int get_vpn_server_par(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
	// {	//int result;
	// 	int enable;
	// 	int mode;		
	// 	uint8 server[50] = { 0 };
	// 	int port;
	// 	uint8 user[20] = { 0 };
	// 	uint8 passwd[20] = { 0 };
	// 	//uint8 hold[20];
	// 	uint8 s_buf[256] = { 0 };
	// 	int ftp_port;
	// 	uint8 ftp_user[20] = { 0 };
	// 	uint8 ftp_passwd[20] = { 0 };



    //     ty_ctl(par->vpnfd,DF_VPN_GET_CM_GET_VPN,&enable,&mode,server,&port,user,passwd,&ftp_port,ftp_user,ftp_passwd);
	// 	module_out("enable = %d,mode = %d,server = %s,port = %d,user = %s,passwd = %s,ftp_port = %d,ftp_user = %s,ftp_passwd = %s\n",enable,mode,server,port,user,passwd,ftp_port,ftp_user,ftp_passwd);
    //     memset(s_buf,0,sizeof(s_buf));
	// 	s_buf[0]=enable;
	// 	s_buf[1]=mode;
	// 	memcpy(s_buf+2,server,50);
	// 	s_buf[52]=((port>>8)&0xff);s_buf[53]=((port>>0)&0xff);
	// 	memcpy(s_buf+54,user,20);
	// 	memcpy(s_buf+74,passwd,20);
	// 	s_buf[94]=((ftp_port>>8)&0xff);s_buf[95]=((ftp_port>>0)&0xff);
	// 	memcpy(s_buf+96,ftp_user,20);
	// 	memcpy(s_buf+116,ftp_passwd,20);
    //     module_out("����ִ�����\n");
    //     return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_VPN_PAR,s_buf,156); 
	// }

	// static int get_close_update(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
	// {
	// 	uint8 s_buf[256] = { 0 };
	// 	int close_status;
	// 	close_status = ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_CLOSE_UPDATE);
	// 	//module_out("�Ƿ�ر��Զ�����״̬��Ϊ��%d\n",close_status);
	// 	s_buf[0]= close_status;
	// 	return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CLOSE_AUTO_UPDATE,s_buf,1); 
	// }
	static int get_version_describe(int fd, uint32 ip, MODULE	*par, uint8 *buf, int buf_len)
	{
		char *g_buf;
		char s_buf[1000] = { 0 };
		cJSON *json = cJSON_CreateObject();
		cJSON_AddStringToObject(json, "version", app_note.ver);
		cJSON_AddStringToObject(json, "heartbeat", "y");
		g_buf = cJSON_Print(json);
		memset(s_buf, 0, sizeof(s_buf));
		memcpy(s_buf,g_buf,strlen(g_buf));
		free(g_buf);
		return  write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SEE_TAX_INFOS, (uint8 *)s_buf, strlen(s_buf));
	}
    /*----------------------------------������-------------------------------------------*/
	int count_o=0;
	int count_c=0;
		//���˿ںŴ򿪶˿�
    static int open_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {
		//struct _usb_pro usb;
        int port,result,app_fd,start;
		uint8 s_buf[200] = { 0 };
		char client[64] = { 0 };
		struct _usb_port_infor port_info = {0x00};
		char drivers[DF_TY_PATH_MAX];
		struct _usb_file_dev    dev;
		LOAD_DATA load_data = {0x00};
		USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

		if(NULL == usb_port_class->load_action.fun_load_start)
			return err_out_tcp(fd,result);
		
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);//��ȡ�������ʼ�˿ں�
        if(start<0)
        {
			return err_out_tcp(fd,start);
        }
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];//�˿ں�
        port=port-start+1;//��ȡ����˿ںž���λ��
        app_fd=0;
		module_out("[_app.c]���˿ںŴ򿪶˿�\n");
        memset(client,0,sizeof(client));//�ͻ��˱�ʶ��GUID
        if(buf_len>4)//Ϊ���ݲ����ͻ��˱�ʶ���汾��GUIDΪ64λ
        {   //module_out("���ͻ��˵�client\n");
            result=sizeof(client);
            buf_len-=4;
            if(result>buf_len)
                result=buf_len;
            memcpy(client,buf+4,result);
        }
		// load_data.port = port;
		load_data.port = (port - 1) > 0?(port-1):0;
		memcpy(load_data.oper_id,client,strlen(client));
		load_data.ip = ip;
		load_data.port_fd = &app_fd;
		load_data.net_type = INTRANET;
		result = *((int *)usb_port_class->load_action.fun_load_start((void *)(&load_data))); //ִ��һ�μ���
		if (result<0)
        {   
			module_out("�򿪶˿�ʧ��\n");
            return err_out_tcp(fd,result);
        }
		memset(&port_info, 0, sizeof(struct _usb_port_infor));
		fun_get_plate_usb_info(port, &port_info);
		if (port_info.with_dev != 0)
		{
			memset(&dev, 0, sizeof(dev));
			_usb_file_get_infor(port_info.busid, &dev);
			memcpy(drivers, dev.interface[0].driver, strlen(dev.interface[0].driver));
		}
        memset(s_buf,0,sizeof(s_buf));
        s_buf[0]=((app_fd>>24)&0xff);s_buf[1]=((app_fd>>16)&0xff);
        s_buf[2]=((app_fd>>8)&0xff);s_buf[3]=((app_fd>>0)&0xff);
        s_buf[4]=((port>>24)&0xff);s_buf[5]=((port>>16)&0xff);
        s_buf[6]=((port>>8)&0xff);s_buf[7]=((port>>0)&0xff);
		memcpy(s_buf + 8, port_info.busid, strlen(port_info.busid));
        memcpy(s_buf+28,drivers,strlen(drivers));
		result = write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_OPEN_USB,s_buf,82);
		count_o +=1;
		module_out("[���˿ںŴ򿪶˿�] �򿪶˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ],\n\n\n\n\n\n\n\n\n", port, count_o);
        return result;
    }
    //���˿ںŹرն˿�
    static int close_port(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		int port,result,start;
		LOAD_DATA load_data = {0x00};
		USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
		if(NULL == usb_port_class->load_action.fun_load_stop)
			return err_out_tcp(fd,result);
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        //app_fd=buf[0]*0x1000000+buf[1]*0x10000+buf[2]*0x100+buf[3];
        port=buf[5]*0x10000+buf[6]*0x100+buf[7];
        port=port-start+1;
		load_data.port = (port - 1) > 0?(port-1):0;
		// load_data.port = port;
		result = *((int *)usb_port_class->load_action.fun_load_stop((void *)(&load_data))); //ִ��һ���ͷ�
        if(result<0)
        {   
			return err_out_tcp(fd,result);  
        }
		count_c +=1;
		module_out("[���˿ںŹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_USB,NULL,0);
    }
    // //�������Ŵ򿪶˿�
    // static int open_port_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    // {   
	// 	int result,port,app_fd;
	// 	uint8 id[6] = { 0 }, s_buf[200] = { 0 };
    //     //struct _usb_pro usb;
	// 	char client[64] = { 0 };
    //     memset(id,0,sizeof(id));
    //     asc2bcd(buf,id,12);
    //     port=buf[13]*0x10000+buf[14]*0x100+buf[15];    

    //     memset(client,0,sizeof(client));
    //     if(buf_len>16)
    //     {   //module_out("���ͻ��˵�client\n");
    //         result=sizeof(client);
    //         buf_len-=16;
    //         if(result>buf_len)
    //             result=buf_len;
    //         memcpy(client,buf+16,result);
    //     }
	// 	if ((open_port_attach(port, ip, client, &app_fd))<0)
    //     {   
	// 		module_out("�򿪶˿�ʧ��\n");
    //         return err_out_tcp(fd,result);
    //     }

	// 	struct _usb_port_infor port_info;
	// 	memset(&port_info, 0, sizeof(struct _usb_port_infor));
	// 	fun_get_plate_usb_info(port, &port_info);
	// 	char drivers[DF_TY_PATH_MAX];
	// 	struct _usb_file_dev    dev;
	// 	if (port_info.with_dev != 0)
	// 	{
	// 		memset(&dev, 0, sizeof(dev));
	// 		_usb_file_get_infor(port_info.busid, &dev);
	// 		memcpy(drivers, dev.interface[0].driver, strlen(dev.interface[0].driver));
	// 	}
    //     memset(s_buf,0,sizeof(s_buf));
    //     s_buf[0]=((app_fd>>24)&0xff);s_buf[1]=((app_fd>>16)&0xff);
    //     s_buf[2]=((app_fd>>8)&0xff);s_buf[3]=((app_fd>>0)&0xff);
    //     s_buf[4]=((port>>24)&0xff);s_buf[5]=((port>>16)&0xff);
    //     s_buf[6]=((port>>8)&0xff);s_buf[7]=((port>>0)&0xff);
	// 	memcpy(s_buf + 8, port_info.busid, strlen(port_info.busid));
    //     memcpy(s_buf+28,drivers,strlen(drivers));
    //     return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_ID_OPEN_USB,s_buf,82); 
    // }
    // //�������Źرն˿�
    // static int close_port_id(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    // {   
	// 	int port,result;
	// 	uint8 id[6] = { 0 };
    //     //struct _usb_pro usb;
    //     memset(id,0,sizeof(id));
    //     asc2bcd(buf,id,12);
    //     //app_fd=buf[12]*0x1000000+buf[13]*0x10000+buf[14]*0x100+buf[15];
    //     port=buf[17]*0x10000+buf[18]*0x100+buf[19];
    //     //if(memcmp(id,usb.dev_id,sizeof(id))!=0)
    //     //{   
	// 	//	module_out("û�иû������\n");
    //     //    return err_out_tcp(fd,-1);
    //     //}
	// 	result = close_port_dettach(port);
    //     if(result<0)
    //     {   
	// 		return err_out_tcp(fd,result);  
    //     }
	// 	count_c +=1;
	// 	module_out("[��������Źرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
    //     return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_ID_CLOSE_USB,NULL,0);
    // }
	//ǿ�ƹرն˿�
    static int close_port_now(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		int port,result,start;		
		LOAD_DATA load_data = {0x00};
		USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
		if(NULL == usb_port_class->load_action.fun_load_stop)
			return err_out_tcp(fd,result);
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];
        port=port-start+1;
		module_out("ǿ�ƹرն˿ڡ�%d��\n",port);
		
		// load_data.port = port;
		load_data.port = (port - 1) > 0?(port-1):0;
		result = *((int*)usb_port_class->load_action.fun_load_stop((void *)(&load_data))); //ִ��һ���ͷ�
        if(result<0)
        {   
			return err_out_tcp(fd,result);  
        }
		usb_port_class->power_action.fun_power(port-1,RETART_ACTION);
		module_out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n",port);
		count_c +=1;
		module_out("[���˿ں�ǿ�ƹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_NOW,NULL,0);
    }
    //�ػ�
    static int power_off(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
    {   
		int result;
		module_out("����ػ�����\n");
        result=ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_POWER_OFF,DF_EVENT_REASON_NET);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_POWER_OFF,NULL,0);    
    }
	//����
    static int heart(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		module_out("������������\n");
		return err_out_tcp(fd, -1);
    }
		//����
    static int reset(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		module_out("������������\n");
		ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_RESET_POWER,DF_EVENT_REASON_NET); 
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_RESET,NULL,0);    
    }
		//ע��
    static int regdit(int fd,uint32 ip,MODULE  *par,uint8 *buf,int buf_len)
    {   
		int result;
		module_out("ע��\n");
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_REDIT);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_REDIT,NULL,0);  
    }
		
	//�򿪶˿ڵ�������USBip
    static int open_port_no_active(int fd,uint32 ip,MODULE    *par,uint8 *buf,int buf_len)
    {   
		return err_out_tcp(fd, -1);
    }
    
	//ά�ִ򿪶˿�
    static int keep_open_port(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
    {   
		//struct _usb_pro usb;
        int port,result,app_fd,start;
		uint8 s_buf[200] = { 0 };
		char client[64] = { 0 };
		module_out("ά�ִ򿪶˿�\n");
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];
        port=port-start+1;
        app_fd=0;
        memset(client,0,sizeof(client));
        if(buf_len>4)
        {   //module_out("���ͻ��˵�client\n");
            result=sizeof(client);
            buf_len-=4;
            if(result>buf_len)
                result=buf_len;
            memcpy(client,buf+4,result);
        }
		// if ((result = open_port_attach(port, ip, client, &app_fd))<0)
        // //if((result=ty_ctl(par->ty_usb_m_fd,DF_TY_USB_M_CM_SEE_OPEN_PORT,port,ip,client,&app_fd))<0)
        // {   
		// 	module_out("�򿪶˿�ʧ��\n");
        //     return err_out_tcp(fd,result);
        // }
        //memset(&usb,0,sizeof(usb));
        module_out("���Ժ󷵻�fd=%d\n",app_fd);

		struct _usb_port_infor port_info;
		memset(&port_info, 0, sizeof(struct _usb_port_infor));
		fun_get_plate_usb_info(port, &port_info);
		char drivers[DF_TY_PATH_MAX];
		struct _usb_file_dev    dev;
		if (port_info.with_dev != 0)
		{
			memset(&dev, 0, sizeof(dev));
			_usb_file_get_infor(port_info.busid, &dev);
			memcpy(drivers, dev.interface[0].driver, strlen(dev.interface[0].driver));
		}


        memset(s_buf,0,sizeof(s_buf));
        s_buf[0]=((app_fd>>24)&0xff);s_buf[1]=((app_fd>>16)&0xff);
        s_buf[2]=((app_fd>>8)&0xff);s_buf[3]=((app_fd>>0)&0xff);
        s_buf[4]=((port>>24)&0xff);s_buf[5]=((port>>16)&0xff);
        s_buf[6]=((port>>8)&0xff);s_buf[7]=((port>>0)&0xff);
		memcpy(s_buf + 8, port_info.busid, strlen(port_info.busid));
        memcpy(s_buf+28,drivers,strlen(drivers));
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_OPEN_PORT,s_buf,82); 
    }
    

	static int check_version_updata(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
	{
		int result;
		result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_FTP_DATA);
        if(result<0)
        {   

			if (detect_process("vsftpd", "tmp11.txt") > 0)
			{
				module_out("%s���̴���,�����\n", "vsftpd");
				system("killall vsftpd");
			}

			if (access("/etc/vsftpd.conf-back", 0) == 0)
			{
				system("vsftpd /etc/vsftpd.conf-back &");
			}

			module_out("����ʧ�ܣ�����ʧ��ԭ��\n");
			return err_out_tcp(fd,result);
        }
		module_out("�����ɹ�\n");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_UPDATA,NULL,0);
	}

	static int close_port_by_dettach(int fd,uint32 ip,MODULE *par,uint8 *buf,int buf_len)
	{
		int result,start,port;
		module_out("[_app.c]ʹ��Dettach�رն˿�\n");
		start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
		//app_fd=buf[0]*0x1000000+buf[1]*0x10000+buf[2]*0x100+buf[3];
		port=buf[5]*0x10000+buf[6]*0x100+buf[7];
		port=port-start+1;
		//result = close_port_dettach(port);
		if(result<0)
		{
			return err_out_tcp(fd,result);
		}
		module_out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
		count_c += 1;
		module_out("[���˿ں�ǿ�ƹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n", port, count_c);
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_DETTACH_PORT,NULL,0);
	}

/*========================================���󷵻�=============================================*/
static int answer_no_order(int fd,MODULE    *par,uint8 *buf,int buf_len)
{   
	par=par;buf=buf;buf_len=buf_len;
    return err_out_s(fd,254,(uint8 *)"��֧�ָ�����");  
}
static int err_out_tcp(int fd,int err)
{
	int i;
	char o_name[256] = { 0 }, note[256] = { 0 }, name[256] = { 0 };
    for(i=0;i<sizeof(err_n)/sizeof(err_n[0]);i++)
    {   
		if(err_n[i].err_ty==err)
            break;
    }
    if(i==sizeof(err_n)/sizeof(err_n[0]))
    {   
		module_out("���������δ�������ô�������\n");
        return err_out_s(fd,255,(uint8 *)"δ֪����");
    }
    memset(o_name,0,sizeof(o_name));
    memset(note,0,sizeof(note));
    memset(name,0,sizeof(name));
    //_err_out(err,o_name,note);
    sprintf(name,"[%s]:\"%s\"",o_name,note);
    module_out("����:[%s]\n",name);
    return err_out_s(fd,err_n[i].err,(uint8 *)name);
}

static int err_out_s(int fd,uint8 err,uint8 *name)
{
	uint8 buf[1024] = { 0 };
    int len;
    memset(buf,0,sizeof(buf));
    buf[0]=err;
    len=strlen((char *)name);
    memcpy(buf+1,name,len);
    len++;
    return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_ERR,buf,len);
}
/*======================================�ײ���==================================================*/
static int write_ty1502_order(int fd,int order,uint8 *buf,int buf_len)
{
	uint8 s_buf[2000000] = { 0 };
    int len,result;
    memset(s_buf,0,sizeof(s_buf));
    len=6+buf_len;
	if (len <= 0xffff)
	{
		s_buf[0] = DF_TY_1502_COM_EDIT / 256; s_buf[1] = (DF_TY_1502_COM_EDIT & 0xff);
		s_buf[2] = len / 256; s_buf[3] = (len & 0xff);
		s_buf[4] = order / 256; s_buf[5] = (order & 0xff);
		if ((buf_len - 6) < sizeof(s_buf))
		{
			memcpy(s_buf + 6, buf, buf_len);
			return ty_socket_write(fd, s_buf, len);
		}
		else
		{
			if ((result = ty_socket_write(fd, s_buf, 6)) < 0)
				return result;
			return ty_socket_write(fd, buf, buf_len);
		}
	}
	else//�����ֽڼ�ͷ�ֽڴ���65535����ò��������ȷ�ʽ
	{
		s_buf[0] = DF_TY_1502_COM_EDIT / 256;			s_buf[1] = (DF_TY_1502_COM_EDIT & 0xff);
		s_buf[2] = 0;									s_buf[3] = 0;
		s_buf[4] = order / 256;							s_buf[5] = (order & 0xff);
		s_buf[6] = (((len + 4) >> 24) & 0xff);			s_buf[7] = (((len + 4) >> 16) & 0xff);//���ӳ���4���ֽ�
		s_buf[8] = (((len + 4) >> 8) & 0xff);			s_buf[9] = (((len + 4) >> 0) & 0xff);
		memcpy(s_buf + 10, buf, buf_len);
		return ty_socket_write(fd, s_buf, len+4);
	}
}



static int unpack(int fd,uint8 *out_buf,int *out_len)
{
	uint8 edit[2] = { 0 };
	uint8 len_buf[2] = { 0 };
	uint8 order[2] = { 0 };
    int buf_len;
    module_out("��ȡ�汾��Ϣ\n");
	memset(edit,0,sizeof(edit));
	memset(len_buf,0,sizeof(len_buf));
	memset(order,0,sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
    if(ty_socket_read(fd,edit,sizeof(edit))<0)
    {   module_out("��ȡ�汾��Ϣ����\n");
		return -1;
    }
    if((edit[0]!=0x01)&&(edit[1]!=0x10))
    {   module_out("��ȡ���İ汾��Ϣ����%02x %02x\n",edit[0],edit[1]);
        return -2;
    }
	//module_out("�汾��Ϣ�ɹ� %02x %02x\n",edit[0],edit[1]);
    //module_out("��ȡ������Ϣ\n");
    if(ty_socket_read(fd,len_buf,sizeof(len_buf))<0)
    {   
		module_out("������Ϣ��ȡʧ��\n");
        return -1;
    }
    buf_len=len_buf[0]*256+len_buf[1];
    //module_out("��֡����:%d\n",buf_len);
    if(buf_len<6)
    {   
		module_out("����С��6����\n");
        return -3;
    }
    if(buf_len-6>(*out_len))
    {   
		module_out("���ȹ���\n");
        return -4;
    } 
    if(ty_socket_read(fd,order,sizeof(order))<0)
    {   
		module_out("��ȡ������ʧ��\n");
        return -1;
    }
    if(ty_socket_read(fd,out_buf,buf_len-6)<0)
    {   
		module_out("��ȡ����ʧ��\n");
        return -1;
    }
    (*out_len)=(buf_len-6);
    return (order[0]*256+order[1]);
}

/*==================================socket ���ͺͽ���===============================================*/
static int ty_socket_read(int sock,uint8 *buf,int buf_len)
{   
	int i,result;
    for(i=0;i<buf_len;i+=result)
    {   //module_out("read data by read i = %d\n",i);
		result=read(sock,buf+i,buf_len-i);
        if(result<=0)
		{	//module_out("read data time module_out or err \n\n\n\n");
            return result;
		}
    }
    return i;
}

static int ty_socket_write(int sock,uint8 *buf,int buf_len)
{   
	int i,result;
    for(i=0;i<buf_len;i+=result)
    {   
		result=write(sock,buf+i,buf_len-i);
        if(result<=0)
            return result;
    }
    return i;   
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////UDP�������к���/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void udp_ser_main_task(void *arg,int o_timer)
{
	MODULE *stream;
	int result;
	uint8 buf[512] = { 0 };
	int in_len;
	uint8 rcvbuf[512] = { 0 };
	int rcvlen;
	stream=arg;
	//module_out("stream->udp_fd = %d\n",stream->udp_fd);
	result = ty_read(stream->udp_fd,buf,sizeof(buf));
	if(result < 0)
	{
		module_out("��ȡUDP����ʧ��\n");
		return ;
	}
	in_len = result;
	//module_out("main task result =%d in_len =%d\n",result,in_len);
	result = udp_unpack(buf,in_len,rcvbuf,&rcvlen);
	// module_out("main task result =%d in_len =%d\n",result,in_len);
	if(result<0)
	{
		//module_out("UDP���ݽ�������\n");
		result = udp_unpack_new_json(buf, in_len);
		if (result == 0)//�½ӿ�Э��������豸����
		{
			udp_get_sev_pro_new(stream->udp_fd, stream);
		}
		return ;
	}
	if(result == DF_MB_ORDER_SEE_USB_SHARE_DEV)
	{
		//module_out("������Ϊ��ȡ�ն���Ϣ stream->udp_fd=%d\n",stream->udp_fd);
		udp_get_sev_pro(stream->udp_fd,stream,rcvbuf,rcvlen);
	}
	else if(result == DF_MB_ORDER_OPER_RESTART)
	{	
		//module_out("������Ϊ�����ն� stream->udp_fd=%d\n",stream->udp_fd);
		udp_order_sev_restart(stream->udp_fd,stream,rcvbuf,rcvlen);
	}
	return ;
}
static int udp_unpack_new_json(uint8 *in_buf, int in_len)
{
	cJSON *root = NULL;
	cJSON*item = NULL;
	root = cJSON_Parse((char*)in_buf);
	if (root == NULL)
		return -1;
	cJSON *object = cJSON_GetObjectItem(root, "head");
	if (object == NULL)
	{
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(object, "protocol");
	if (item != NULL)
	{
		//module_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		if (memcmp(item->valuestring, "mb_usbshare_20191024", 20) != 0)
		{
			cJSON_Delete(root);
			return -1;
		}
			
	}
	item = cJSON_GetObjectItem(object, "cmd");
	if (item != NULL)
	{
		//module_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		if (memcmp(item->valuestring, "search_ters", 11) != 0)
		{
			cJSON_Delete(root);
			return -1;
		}

	}
	cJSON_Delete(root);
	return 0;
}

static int udp_unpack(uint8 *in_buf,int in_len,uint8 *out_buf,int *out_len)
{   
	uint8 order[2] = { 0 };
    int buf_len;
    //module_out("��ȡ�汾��Ϣ\n");
    if((in_buf[0]!=0x01)&&(in_buf[1]!=0x10))
    {   //module_out("�汾��Ϣ����\n");
        return -2;
    }
    //module_out("��ȡ������Ϣ\n");
    buf_len=in_buf[2]*256+in_buf[3];
    //module_out("��֡����:%d\n",buf_len);
    if(buf_len<6)
    {   
		module_out("����С��6����\n");
        return -3;
    }

    if(buf_len!=in_len)
    {   
		module_out("���ݳ��ȴ���buf=%d in_len=%d\n",buf_len,in_len);
        return -4;
    } 
    order[0]=in_buf[4];
	order[1]=in_buf[5];

	memcpy(out_buf,in_buf+6,buf_len-6);
    (*out_len)=(buf_len-6);
    return (order[0]*256+order[1]);
}

//upd��������Ҫ���ն�����
static int udp_order_sev_restart(int fd,MODULE  *par,uint8 *buf,int buf_len)    
{
	uint8 s_buf[1024] = { 0 };
	//uint8 trydate[10];
	int result;
    //int start;
	//uint32 ip;
	//int i;
	char oper_id[16] = { 0 };
	char machine_id[16] = { 0 };
	//int usb_sum;
	struct _switch_dev_id   id;
	//struct _machine_s   machine;
	//_lock_set(par->lock);
    memset(s_buf,0,sizeof(s_buf));
    module_out("=====================���������Ϣ========================\n");
   
	module_out("����������\n");
	memset(oper_id,0,sizeof(oper_id));
	memset(machine_id,0,sizeof(machine_id));
    memset(&id,0,sizeof(id));
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
	bcd2asc(id.id, (uint8 *)machine_id, 6);
	if(result < 0)
	{	
		module_out("����������result = %d\n",result);
	}
	module_out("��ȡ�����������Ϊ%s\n",machine_id);
	memcpy(oper_id,buf,12);
	if(memcmp(machine_id,oper_id,12)!=0)
	{
		module_out("��������Ҫ������ID��һ��,��ִ���κβ���\n");
		return 0;
	}
	module_out("������������\n");
	logout(INFO, "SYSTEM", "udp", "�ɿͻ���UDP��������������������\r\n");

	ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_RESET_POWER,DF_EVENT_REASON_NET); 
	udp_write_order(par->udp_fd,DF_MB_ANSWER_OPER_RESTART,NULL,0);
	module_out("�ɹ��������������ʱ10����˳��ó���\n");
	sleep(10);
	logout(INFO, "SYSTEM", "udp", "UDP�ɹ�������������,�˳�����\r\n");
	exit(0);

	return 0;  
}


static int udp_get_sev_pro_new(int fd, MODULE  *par)
{
	uint8 s_buf[1024] = { 0 };
	//uint8 trydate[10];
	int result;
	//int start;
	uint32 ip;
	char ip_str[20] = { 0 };
	char order_port[10] = { 0 };
	char usb_sum_str[10] = { 0 };
	char soft_ver[50] = { 0 };
	//int i;
	char mach_type[12] = { 0 };
	char machine_id[16] = { 0 };
	int usb_sum;
	struct _switch_dev_id   id;

	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
	
	//struct _machine_s   machine;
	//_lock_set(par->lock);
	memset(s_buf, 0, sizeof(s_buf));
	//module_out("=====================���������Ϣ========================\n");
	ip = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_GET_IP);
	//module_out("���ip = %08x\n",ip);
	sprintf(ip_str, "%d.%d.%d.%d", ((ip >> 24) & 0xff), ((ip >> 16) & 0xff), ((ip >> 8) & 0xff), ((ip >> 0) & 0xff));
	
	//module_out("�������˿�= %d\n",par->order_port);
	sprintf(order_port, "%d", DF_TY_SEV_ORDER_PORT);

	//module_out("���USBIP�˿�= %d\n",par->usbip_port);
	//int usbip_port = 3240;
	
	//module_out("����������\n");
	memset(machine_id, 0, sizeof(machine_id));
	memset(&id, 0, sizeof(id));
	result = ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, (uint8 *)machine_id, 6);
	if (result < 0)
	{	//module_out("����������result = %d\n",result);
	}
	
	// module_out("���USB�˿�����usb_sum = %s\n",usb_sum);
	usb_sum = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //���ж��Ƿ��ǻ�е�۵Ļ���
		sprintf(usb_sum_str, "%d", usb_sum/2);
	else
		sprintf(usb_sum_str, "%d", usb_sum);
	//module_out("�������汾\n");
	result = ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_GET_SOFT, soft_ver);
	if (result < 0)
	{
		module_out("�������汾result = %d\n", result);
	}
	//module_out("��������ͺ�\n");

	//memset(mach_type, 0, sizeof(mach_type));
	//if (memcmp(machine_id, "068130", 6) == 0)
	//{
	//	sprintf(mach_type, "MB_1905");
	//}
	//else if (machine_id[4] == 0x31 || machine_id[4] == 0x32 || machine_id[4] == 0x33)
	//{
	//	sprintf(mach_type, "MB_16");
	//	memcpy(mach_type + 5, machine_id + 3, 2);
	//}
	//else if (machine_id[4] == 0x34)
	//{
	//	sprintf(mach_type, "MB_1702");
	//	//memcpy(mach_type+5,sev_id+3,2);
	//}
	//else if (machine_id[4] == 0x35)
	//{
	//	sprintf(mach_type, "MB_1703");
	//	//memcpy(mach_type+5,sev_id+3,2);
	//}
	//else if (machine_id[4] == 0x37)
	//{
	//	sprintf(mach_type, "MB_1805");
	//}
	//else if (machine_id[4] == 0x38)
	//{
	//	sprintf(mach_type, "MB_1806");
	//}
	//else if (machine_id[5] != 0x30)
	//{
	//	memset(mach_type + 7, machine_id[5] + 0x10, 1);
	//}
	memset(mach_type, 0, sizeof(mach_type));
	memcpy(mach_type, app_note.type, strlen(app_note.type));
	cJSON  *dir2,*root = NULL;
	char *tmp = NULL;
	root = cJSON_CreateObject();
	if (root == NULL)
		return -1;
	cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
	cJSON *p_layer;
	cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(p_layer, "ter_ip", ip_str);
	cJSON_AddStringToObject(p_layer, "ter_id", machine_id);
	cJSON_AddStringToObject(p_layer, "usb_port_num", usb_sum_str);
	cJSON_AddStringToObject(p_layer, "ter_version", soft_ver);
	cJSON_AddStringToObject(p_layer, "ter_type", mach_type);
	cJSON_AddStringToObject(p_layer, "order_port", order_port);
	cJSON_AddStringToObject(p_layer, "cmd_port","3240");
	tmp = cJSON_Print(root);
	memset(s_buf, 0, sizeof(s_buf));
	memcpy(s_buf, tmp, strlen(tmp));
	free(tmp);
	cJSON_Delete(root);

	// printf("udp_get_sev_pro s_buf=%s\n",s_buf);

	// int i = 0;
	// printf("udp_get_sev_pro\n");
	// for(i = 0;i<strlen((char*)s_buf);++i)
	// {
	// 	printf(" %02x",s_buf[i]);
	// }
	// printf("\n");

	return ty_write(fd, s_buf, strlen((char*)s_buf));
	//return udp_write_order(par->udp_fd, DF_MB_ANSWER_SEE_USB_SHARE_DEV, s_buf, 42);
}

//��ȡ����������
static int udp_get_sev_pro(int fd,MODULE  *par,uint8 *buf,int buf_len)    
{
	uint8 s_buf[1024] = { 0 };
	//uint8 trydate[10];
	int result;
    //int start;
	uint32 ip;
	//int i;
	//char mach_type[12];
	char machine_id[16] = { 0 };
	int usb_sum;
	struct _switch_dev_id   id;
	//struct _machine_s   machine;
	//_lock_set(par->lock);
    memset(s_buf,0,sizeof(s_buf));
    //module_out("=====================���������Ϣ========================\n");
    ip=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_IP);
	//module_out("���ip = %08x\n",ip);
    s_buf[0]=((ip>>24)&0xff);s_buf[1]=((ip>>16)&0xff);
    s_buf[2]=((ip>>8)&0xff);s_buf[3]=((ip>>0)&0xff);       
    //module_out("�������˿�= %d\n",par->order_port);
	s_buf[4] = ((DF_TY_SEV_ORDER_PORT >> 8) & 0xff);
	s_buf[5] = ((DF_TY_SEV_ORDER_PORT >> 0) & 0xff);
    //module_out("���USBIP�˿�= %d\n",par->usbip_port);
	int usbip_port = 3240;
	s_buf[6]=((usbip_port>>8)&0xff);
	s_buf[7]=((usbip_port>>0)&0xff);
	//module_out("����������\n");
	memset(machine_id,0,sizeof(machine_id));
    memset(&id,0,sizeof(id));
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
    bcd2asc(id.id,(uint8 *)machine_id,6);
	if(result < 0)
	{	//module_out("����������result = %d\n",result);
	}
	memcpy(s_buf+8,machine_id,12);
	//module_out("���USB�˿�����\n");
    usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
    s_buf[20]=((usb_sum>>24)&0xff);s_buf[21]=((usb_sum>>16)&0xff);
    s_buf[22]=((usb_sum>>8)&0xff);s_buf[23]=((usb_sum>>0)&0xff);     
	//module_out("�������汾\n");
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_SOFT,s_buf+24);
	if(result < 0)
	{	
		module_out("�������汾result = %d\n",result);
	}
	//module_out("��������ͺ�\n");

	//memset(mach_type,0,sizeof(mach_type));
	//if(machine_id[4]==0x31||machine_id[4]==0x32||machine_id[4]==0x33)
	//{
	//	sprintf(mach_type,"MB_16");
	//	memcpy(mach_type+5,machine_id+3,2);
	//}
	//else if(machine_id[4]==0x34)
	//{
	//	sprintf(mach_type,"MB_1702");
	//	//memcpy(mach_type+5,sev_id+3,2);
	//}
	//else if(machine_id[4]==0x35)
	//{
	//	sprintf(mach_type,"MB_1703");
	//	//memcpy(mach_type+5,sev_id+3,2);
	//}
	//else if(machine_id[4]==0x37)
	//{
	//	sprintf(mach_type,"MB_1805");
	//}
	//else if(machine_id[4]==0x38)
	//{
	//	sprintf(mach_type,"MB_1806");
	//}
	//else if(machine_id[5]!=0x30)
	//{
	//	memset(mach_type+7,machine_id[5]+0x10,1);
	//}
	//memcpy(s_buf+32,mach_type,10);

	memcpy(s_buf + 32, app_note.type, strlen(app_note.type));
	//	_lock_un(par->lock);
    //return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_SEV_PRO,s_buf,180); 
	//module_out("par->udp_fd = %d\n",par->udp_fd);

	// int i = 0;
	// printf("udp_get_sev_pro\n");
	// for(i = 0;i<42;++i)
	// {
	// 	printf(" %02x",s_buf[i]);
	// }
	// printf("\n");

	return udp_write_order(par->udp_fd,DF_MB_ANSWER_SEE_USB_SHARE_DEV,s_buf,42);  
}


static int udp_write_order(int fd,int order,uint8 *buf,int buf_len)
{
	uint8 s_buf[4096] = { 0 };
    int len,result;
    memset(s_buf,0,sizeof(s_buf));
    len=6+buf_len;
    s_buf[0]=DF_TY_1502_COM_EDIT/256;s_buf[1]=(DF_TY_1502_COM_EDIT&0xff);
    s_buf[2]=len/256;s_buf[3]=(len&0xff);
    s_buf[4]=order/256;s_buf[5]=(order&0xff);
    if((buf_len-6)<sizeof(s_buf))
    {   
		memcpy(s_buf+6,buf,buf_len);
		//module_out("ʹ��UDP�������� fd=%d\n",fd);
        return ty_write(fd,s_buf,len);
    }
    else
    {   
		if((result=ty_write(fd,s_buf,6))<0)
            return result;
        return ty_write(fd,buf,buf_len);
    }
}


//����jsonͷ��Ϣ
static int analysis_json_head(char *inbuf, struct mqtt_parm *parm)
{
	int result;
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object = cJSON_GetObjectItem(root, "head");
	if (object == NULL)
	{
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	//module_out("��ȡЭ��汾\n");
	item = cJSON_GetObjectItem(object, "protocol");
	if (get_json_value_can_not_null(item, parm->protocol, 1, 50) < 0)
	{
		module_out("Э��汾��ȡʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	result = strcmp(parm->protocol, DF_MQTT_SYNC_PROTOCOL_VER);
	if (result != 0)
	{
		module_out("Э��汾��һ��,protocol = %s\n", parm->protocol);
		cJSON_Delete(root);
		return -1;
	}

	///////////////////////////////////////////////////////////////////
	item = cJSON_GetObjectItem(object, "code_type");
	if (get_json_value_can_not_null(item, parm->code_type, 1, 10) < 0)
	{
		module_out("��ȡ��������ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	if ((strcmp(parm->code_type, DF_MQTT_SYNC_CODE_REQUEST) != 0) && (strcmp(parm->code_type, DF_MQTT_SYNC_CODE_RESPONSE) != 0))
	{
		module_out("��Ϣ���ʹ���,parm->code_type = %s\n", parm->code_type);
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(object, "cmd");
	if (get_json_value_can_not_null(item, parm->cmd, 1, 50) < 0)
	{
		module_out("��ȡ������ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	//module_out("��ȡ��������Ϊ%s\n", parm->cmd);

	//module_out(��ȡԴ��Ϣ����\n);
	item = cJSON_GetObjectItem(object, "source_topic");
	if (get_json_value_can_not_null(item, parm->source_topic, 1, 50) < 0)
	{
		module_out("��ȡԴ��Ϣ����ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}


	//module_out(��ȡ��������\n);
	item = cJSON_GetObjectItem(object, "random");
	if (get_json_value_can_not_null(item, parm->random, 1, 128) < 0)
	{
		module_out("��ȡ��������\n");   //inbuf
		module_out("\n\nlbc\ninbuf = %s\n\n",inbuf);
		cJSON_Delete(root);
		return -1;
	}


	//module_out(��ȡ�������\n);
	item = cJSON_GetObjectItem(object, "result");
	if (get_json_value_can_not_null(item, parm->result, 1, 1) < 0)
	{
		//module_out("��ȡ���������ʧ��\n");
	}

	cJSON_Delete(root);
	return 0;
}
//����Ӧ��
static int mqtt_response_errno(int errnum, struct mqtt_parm *parm,char *errinfo)
{
	MODULE *stream;
	int i;
	char msg[200] = { 0 };
	char err[10] = { 0 };
	int result;
	stream = &module;
	//module_out("errnum = %d\n", errnum);
	for (i = 0; i < sizeof(app_err_no_infos) / sizeof(app_err_no_infos[0]); i++)
	{
		if (errnum == app_err_no_infos[i].err)
		{
			memset(msg, 0, sizeof(msg));
			memcpy(msg, app_err_no_infos[i].errinfo, sizeof(msg));

			memset(err, 0, sizeof(err));
			sprintf(err, "%d", app_err_no_infos[i].errnum);
			break;
		}
	}
	if (i == sizeof(app_err_no_infos) / sizeof(app_err_no_infos[0]))
	{
		module_out("δ�ҵ������������������Ϊδ֪����\n");
		sprintf(err, "0");
		memset(msg, 0, sizeof(msg));
		sprintf(msg, "Unknown error.");
	}

	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", parm->cmd);
	cJSON_AddStringToObject(dir1, "source_topic", stream->topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "f");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir3, "err", err);
	cJSON_AddStringToObject(dir3, "msg", msg);
	if (errinfo != NULL)
	{
		cJSON_AddStringToObject(dir3, "errinfo", errinfo);
	}
	else
	{
		cJSON_AddStringToObject(dir3, "errinfo", "");
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	module_out("���ش������ %s\n", g_buf);

	//module_out("MQTT��������,���ݳ���%d\n",strlen(g_buf));
	//_lock_set(stream->lock);
	result = _mqtt_client_message_pub(parm->source_topic, g_buf,5);
	//_lock_un(stream->lock);
	free(g_buf);
	cJSON_Delete(json);
	return result;
}

static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num)
{
	int i;
	int result;
#ifndef NGINX_TRAN
	MODULE *stream;
	stream = &module;
	if (strcmp(stream->topic, topic) == 0)
	{
		logout(INFO, "SYSTEM", "mqtt_pub", "��Ϣ����topic���󣬲���Ϊ����topic\r\n", message_callback.insert_num);
		return 0;
	}
#endif

	for (i = 0; i < try_num; i++)
	{
		result = _mqtt_client_pub(topic, sbuf);
		if (result != 0)
		{
			usleep(500000);
			continue;
		}
		break;
	}
	if (i == try_num)
	{
		//logout(INFO, "SYSTEM", "mqtt_pub", "MQTTӦ����Ϣ�����쳣\r\n");
		module_out("����%d�ξ�ʧ��\n",try_num);
		return -1;
	}
	return 0;
}

//���ҿ�����Ϣ���к�
static int find_available_queue_mem(MODULE     *stream)
{
	int mem_i;
	//module_out("���ҿ�����Ϣ����\n");
	if (message_callback.insert_num == MAX_MESSAGE_QUEUE_NUM)
		message_callback.insert_num = 0;
	mem_i = message_callback.insert_num;
	if (message_callback.message_line[mem_i].state == 1)
	{
		if (message_callback.message_line[mem_i].inuse == 1)
		{
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ʱ,��ǰҪ����Ķ������Ϊ�ϴ�ѭ��δ���������,�жϺ������޿��ö���,��ǰ�����ڴ�ţ�%d\r\n", message_callback.insert_num);
			mem_i++;
			for (; mem_i < MAX_MESSAGE_QUEUE_NUM; mem_i++)
			{
				if (message_callback.message_line[mem_i].state == 0)
				{					
					message_callback.insert_num = mem_i;
					//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ʱ,�ҵ������ڴ�ţ�%d\r\n", message_callback.insert_num);
					return mem_i;
				}
			}
		}
		return -1;
	}
	message_callback.insert_num += 1;
	return mem_i;
}

//����Ҫ�������Ϣ���к�
static int find_deal_queue_mem(MODULE     *stream)
{
	int wait_deal_num=0;
	int i=0;
	int mem_i;
	//module_out("���ҿ�����Ϣ����\n");
	if (message_callback.deal_num == MAX_MESSAGE_QUEUE_NUM)
		message_callback.deal_num = 0;
	mem_i = message_callback.deal_num;
	if (message_callback.message_line[mem_i].state == 0)
	{
		
		for (i = 0; i < MAX_MESSAGE_QUEUE_NUM; i++)
		{
			if ((message_callback.message_line[i].state == 1) && (message_callback.message_line[i].inuse == 0))
				wait_deal_num += 1;
		}
		if (wait_deal_num>=1)
		{
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ڴ�����������,��ǰ�������Ϊ��,�������������������Ҫ����,������ǰ�������\r\n");
			message_callback.deal_num += 1;
		}
		return -1;
	}
	if (message_callback.message_line[mem_i].inuse != 0)
	{
		//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ڴ�����������,�ϸ�ѭ�����ڴ���,������ǰ�������\r\n");
		message_callback.deal_num += 1;
		return -1;
	}
	message_callback.message_line[mem_i].inuse = 1;
	return mem_i;
}
//��������������
static void  deal_with_cmd(MODULE *stream, int mem_i, void *fucntion)
{
	unsigned int time_now;
	time_now = get_time_sec();
	if ((time_now - message_callback.message_line[mem_i].message_time) > 120)
	{
		logout(INFO, "SYSTEM", "message_queue", "������Ϣ����ʱ���ֵ�ǰ��Ϣ�ѳ�ʱ,���ٴ������Ϣ\r\n");
		message_callback.message_line[mem_i].state = 0;
		message_callback.message_line[mem_i].inuse = 0;
		memset(message_callback.message_line[mem_i].fpqqlsh, 0, sizeof(message_callback.message_line[mem_i].fpqqlsh));
		memset(&message_callback.message_line[mem_i].parm, 0, sizeof(struct mqtt_parm));
		free(message_callback.message_line[mem_i].message);
		message_callback.message_line[mem_i].message = NULL;
		message_callback.message_line[mem_i].message_time = 0;
		message_callback.deal_num += 1;
		return ;
	}
	if (_delay_task_add("0", fucntion, &message_callback.message_line[mem_i], 0)<0)
	{
		message_callback.message_line[mem_i].inuse = 0;
		message_callback.err_count += 1;
		logout(INFO, "SYSTEM", "mqtt_err", "������Ϣ��������ʧ��,�Ժ��ٴδ���,���������%d\r\n", message_callback.err_count);
		if (message_callback.err_count > 100)
		{
			logout(INFO, "SYSTEM", "mqtt_err", "�����߳��쳣�ۻ�����100��,������������\r\n");
			sleep(5);
			sync();
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
			sleep(10);
		}
		return ;
	}

	//logout(INFO, "SYSTEM", "message_queue", "�������������ɣ���ˮ�ţ�%s\r\n", message_callback.message_line[mem_i].parm.random);
	message_callback.err_count = 0;
	message_callback.deal_num += 1;
	message_callback.task_num += 1;
	return ;
	//module_out("������Ϣ���гɹ�\n");
}

//MQTT�ͻ��˶����쳣����ص�
static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum)
{
	MODULE *stream;
	struct _lcd_print_info *lcd_print_info = NULL;
	lcd_print_info = get_lcd_printf_info();
	stream = &module;
	logout(INFO, "SYSTEM", "mqtt_err", "���붩���쳣����ص�,�������Ϊ%d\r\n", errnum);

	switch (errnum)
	{
		case 1:
			module_out("֤�鲻����");
			break;
		case 8:
			module_out("֤����û����������\n");
			break;
		case 14:
			module_out("����������ʧ��\n");
			break;
#ifdef MTK_OPENWRT
		case DF_MQTT_ERR_RED_OFF_GREEN_ON:
			printf("MQTT CONNECT ERR! LIGHT CHANGING on!!\n");
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_ON, 1001);
			return;
		case DF_MQTT_ERR_RED_OFF_GREEN_TWI:
			printf("MQTT CONNECT ERR! LIGHT CHANGING  twi!!\n");
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI, 1001);
			return;
#endif
		default:
			break;
	}
#ifdef MTK_OPENWRT
	printf("MQTT CONNECT ERR! LIGHT CHANGING  twi!!\n");
	ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI, 1001);
#endif
	lcd_print_info->mqtt_state = 0;
	ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	sleep(10);
	module_out("���ӷ������쳣,�û�����������֤�����\n");
#ifndef MQTT_CTL
	int result;
	if (stream->mqtt_reg_flag < 10)//��ǰ�����������ע������ȡ10��
	{
		module_out("�����쳣����һ��ȥ��ȡע����Ϣ\n");
		memset(stream->mqtt_server, 0, sizeof(stream->mqtt_server));
		memset(stream->user_name, 0, sizeof(stream->user_name));
		memset(stream->user_passwd, 0, sizeof(stream->user_passwd));
		result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_INFO, stream->mqtt_server, &stream->mqtt_port, stream->user_name, stream->user_passwd);
		if (result < 0)
		{
			module_out("�豸δע�ᵽmqtt������,��ʱһ����,result = %d\n",result);
			stream->mqtt_reg_flag += 1;
		}
		else
		{
			module_out("�����ڷ�����ע��ɹ�,ʹ����������\n");
			//char topic[200];
			char id_sub[200] = { 0 };
			char id_pub[200] = { 0 };
			//memset(topic, 0, sizeof(topic));
			memset(id_sub, 0, sizeof(id_sub));
			memset(id_pub, 0, sizeof(id_pub));
			module_out("stream.user_passwd = %s\n", stream->user_passwd);
			sprintf(stream->topic, "mb_fpkjxt_%s", stream->user_name);
			sprintf(id_sub, "sub%s", stream->user_name);
			sprintf(id_pub, "pub%s", stream->user_name);

			mqtt_client->server_port = stream->mqtt_port;
			memset(mqtt_client->server_address, 0, sizeof(mqtt_client->server_address));
			memcpy(mqtt_client->server_address, stream->mqtt_server, strlen(stream->mqtt_server));
			memset(mqtt_client->topic, 0, sizeof(mqtt_client->topic));
			memcpy(mqtt_client->topic, stream->topic, strlen(stream->topic));
			memset(mqtt_client->user_name, 0, sizeof(mqtt_client->user_name));
			memcpy(mqtt_client->user_name, stream->user_name, strlen(stream->user_name));
			memset(mqtt_client->user_passwd, 0, sizeof(mqtt_client->user_passwd));
			memcpy(mqtt_client->user_passwd, stream->user_passwd, strlen(stream->user_passwd));
			memset(mqtt_client->user_id_sub, 0, sizeof(mqtt_client->user_id_sub));
			memcpy(mqtt_client->user_id_sub, id_sub, strlen(id_sub));
			memset(mqtt_client->user_id_pub, 0, sizeof(mqtt_client->user_id_pub));
			memcpy(mqtt_client->user_id_pub, id_pub, strlen(id_pub));
			stream->mqtt_reg_flag = 0;
			mqtt_client->init_ok = 1;

			lcd_print_info->mqtt_state = 1;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			return;
		}
	}
#endif
	sleep(stream->connect_time);
	if ((stream->connect_time > DEAFULT_CONNECT_TIME*20)||(stream->connect_time < DEAFULT_CONNECT_TIME))//����20���ӣ�������ʱ���ٴθ�Ϊ60��һ��
	{
		stream->connect_time = DEAFULT_CONNECT_TIME;
	}
	stream->connect_time += 10;
	lcd_print_info->mqtt_state = 1;
	ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	return;

}

void task_deal_message_queue(void *arg, int o_timer)
{
	MODULE *stream;
	//char name[200];
	//char plate_num[20];
	//char fplxdm[6];
	//char json_buf[1024*1024*2];
	int mem_i;
	//int result;
	stream = arg;
	if (message_callback.task_num > MAX_DEAL_MESSAGE_TASK + 20)
	{
		logout(INFO, "SYSTEM", "mqtt_err", "��ǰ����ִ�е���Ϣ�����������쳣,������������\r\n");
		sleep(5);
		sync();
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
		return;
	}

	//module_out("===============================================================\n���뵽��Ϣ�����߳�\n\n\n\n\n\n");
	if (message_callback.task_num > MAX_DEAL_MESSAGE_TASK)
	{
		//module_out("�̶߳��г���%d,�ȴ��Ժ󴴽�\n", MAX_DEAL_MESSAGE_TASK);
		return;
	}
	_lock_set(stream->mqtt_lock);
	mem_i = find_deal_queue_mem(stream);
	if (mem_i < 0)
	{
		//module_out("����Ҫ��������\n");
		_lock_un(stream->mqtt_lock);
		return;
	}
	_lock_un(stream->mqtt_lock);
	//module_out("��%d���ڴ�������Ϣ������,������Ϊ%s,��ˮ�ţ�%s\n", mem_i, message_callback.message_line[mem_i].parm.cmd, message_callback.message_line[mem_i].parm.random);
	//module_out("cmd %s\n", parm.cmd);
	//module_out("�ж������ֲ�ִ����Ӧ����\n");
	if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXDLXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ��Ϣ������Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxdlxx);
	}
	else if(strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXLSCLJG) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxlscljg);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXZDQBXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxzdqbxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXZDJCXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxzdjcxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_TZZDSJSQ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_tzzdsjsq);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_SQZDDK) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˶˿�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_sqzddk);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_QUREY_TER_LOG) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxzddqrzxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_OPEN_SSH_LINE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������άͨ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_kqzdywtd);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CLOSE_SSH_LINE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������άͨ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_jszdywtd);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_AUTH_FILE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxzddqsqxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_MANAGE_PORT_POWER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_adkhgldkdy);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_MANAGE_TER_POWER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����ն�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_glzddy);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHANGE_PORT_SAVE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�˿�ʡ��ģʽ�л�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qhdksdms);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_TER_POWER_MODE_CHANGE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����ʡ��ģʽ�л�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qhzjsdms);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_OPEN_PORT) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��USB�˿�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_openport);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CLOSE_PORT) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_closeport);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_START_ROBOT_ARM) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������е��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_start_robot_arm);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_UPDATE_ROBOT_ARM) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������е��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_update_robot_arm);
	}
	else
	{
		//module_out("�����ֲ�֧��,�ڴ����Ϊ%d\n",mem_i);
		deal_with_cmd(stream, mem_i, analysis_request_cmd_not_support);
	}
	
	
	return;
}

static void mqtt_respoonse_port(char *sendbuf, struct _new_usbshare_parm *parm)
{
	//char pub_topic[30] = { 0 };
	//sprintf(pub_topic, "")
	module_out("MQTT ��������Ϊ��%s\n", parm->source_topic);
	_mqtt_client_pub(parm->source_topic, sendbuf);

}
static int mqtt_response_errno_gw(struct _new_usbshare_parm *parm, int err_code)
{
	//cJSON *dir1, *dir2;		//���ʹ��
	//char source_topic[30] = { 0 };
	cJSON *data_array;
	cJSON *head_layer;
	cJSON *root = cJSON_CreateObject();
	int i = 0;
	char err_str[20] = { 0 };
	//char o_name[256], note[256], name[256];
	for (i = 0; i<sizeof(mqttgw_share_err_n) / sizeof(mqttgw_share_err_n[0]); i++)
	{
		if (mqttgw_share_err_n[i].err_code == err_code)
			break;
	}
	if (i == sizeof(mqttgw_share_err_n) / sizeof(mqttgw_share_err_n[0]))
	{
		module_out("���������δ�������ô�������\n");
		//return err_out_s(fd, 255, (uint8 *)"δ֪����");
		return -1;
	}
	sprintf(err_str, "%d", err_code);


	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", parm->cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(head_layer, "random", parm->random);
	cJSON_AddStringToObject(head_layer, "result", "f");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "err_code", err_str);
	cJSON_AddStringToObject(data_layer, "err_info", mqttgw_share_err_n[i].err_info);

	char *g_buf;
	g_buf = cJSON_Print(root);
	//printf("g_buf = %s\n", g_buf);

	module_out("MQTT��������err gw:%s\n", g_buf);
	_mqtt_client_pub(parm->source_topic, g_buf);
	free(g_buf);
	return 0;
}
static int mqtt_open_share_response(struct _new_usbshare_parm *parm, int fd)
{
	char *g_buf = NULL;
	//char handle[10] = { 0 };
	char trans_port[20] = { 0 };
	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", "openport");
	cJSON_AddStringToObject(dir1, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(dir1, "random", parm->random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON *dir3;
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	cJSON_AddStringToObject(dir3, "tran_server", parm->trans_server);
	sprintf(trans_port, "%d", parm->port);
	cJSON_AddStringToObject(dir3, "tran_port", trans_port);
	/*sprintf(handle, "%d", fd);
	cJSON_AddStringToObject(dir3, "line_handle", handle);*/

	cJSON_AddStringToObject(dir3, "clientid", parm->client_id);
	g_buf = cJSON_Print(json);
	cJSON_Delete(json);

	module_out("MQTT��������gw:%s\n", g_buf);
	_mqtt_client_pub(parm->source_topic, g_buf);
	free(g_buf);
	return 0;

}


static int prepare_mqtt_gw_close_packet(char *sendbuf, struct _new_usbshare_parm *parm)
{

	char *p_buf;
	char source_topic[30] = { 0 };
	//char portstr[4] = { 0 };
	cJSON *root = cJSON_CreateObject();
	//cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON *data_array;		 //���ʹ��
	sprintf(source_topic, "gw/%s/usbshare", parm->mechine_id);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", "closeport");
	cJSON_AddStringToObject(head_layer, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(head_layer, "random", parm->random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());


	cJSON *data_layer;
	char port_str[5] = { 0 };
	sprintf(port_str, "%03d", parm->port);
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "port", port_str);
	cJSON_AddStringToObject(data_layer, "msg", "s");
	/*cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());




	sprintf(portstr, "%03d", port);
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "port", portstr);
	cJSON_AddStringToObject(data_layer, "busid", usb->busid);
	cJSON_AddStringToObject(data_layer, "drivers", usb->drivers);*/
	p_buf = cJSON_Print(root);
	memcpy(sendbuf, p_buf, strlen(p_buf));
	free(p_buf);

	return 0;
}

static int prepare_mqtt_gw_robot_arm_packet(char *sendbuf, struct _new_usbshare_parm *parm,char *cmd)
{

	char *p_buf;
	char source_topic[30] = { 0 };
	//char portstr[4] = { 0 };
	cJSON *root = cJSON_CreateObject();
	//cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON *data_array;		 //���ʹ��
	sprintf(source_topic, "gw/%s/usbshare", parm->mechine_id);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", parm->source_topic);
	cJSON_AddStringToObject(head_layer, "random", parm->random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());


	cJSON *data_layer;
	char port_str[5] = { 0 };
	sprintf(port_str, "%03d", parm->port);
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "port", port_str);
	cJSON_AddStringToObject(data_layer, "msg", "s");

	p_buf = cJSON_Print(root);
	memcpy(sendbuf, p_buf, strlen(p_buf));
	free(p_buf);

	return 0;
}



//char start_time[50];
//char end_time[50];
//MQTT�ͻ�����Ϣ���ջص�
static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	MODULE     *stream;
	struct mqtt_parm parm;
	int result;
	int mem_i;
	stream = &module;
	//module_out("����MQTT�ͻ�����Ϣ���ջص�\n");
	assert(obj);
	if (!message->payloadlen)
	{
		module_out("û�����ݻ����ݴ���\n");
		return;
	}
	//module_out("MQTT PUB [%s:%s]\n", message->topic, message->payload);
	//mqtt_len += (strlen(message->payloadlen) + strlen(message->topic));
	//module_out("MQTT�շ��ֽ���:%lld\n", mqtt_len);
	//module_out("topic %s \n", message->topic);
	//printf("message %s\n",(char *) message->payload);
	memset(&parm, 0, sizeof(struct mqtt_parm));
	result = analysis_json_head(message->payload, &parm);
	if (result < 0)
	{
		module_out("����json����ͷʧ��\n");
		return;
	}

	if ((strcmp(parm.cmd, "zdsbspxx") == 0))
	{
		return;
	}

	//logout(INFO, "MQTT", "mqtt_message", "��ˮ�ţ�%s,���յ���Ϣ���������\r\n", parm.random);
	mem_i = find_available_queue_mem(stream);
	if (mem_i < 0)
	{
		module_out("��Ϣ��������,�޷�����\n");
		logout(INFO, "MQTT", "mqtt_err", "��Ϣ%s,��Ϣ���������޷��洢,����������\r\n", parm.random);
		return;
	}
	message_callback.message_line[mem_i].message = malloc(strlen(message->payload) + 100);
	if (message_callback.message_line[mem_i].message == NULL)
	{
		module_out("�ڴ�����ʧ��,Ҫ������ڴ��СΪ%d,������%s\n", strlen(message->payload) + 100, DF_MQTT_SYNC_REPORT_TER_STATUS);
		return;
	}
	memset(&message_callback.message_line[mem_i].parm, 0, sizeof(struct mqtt_parm));
	memcpy(&message_callback.message_line[mem_i].parm, &parm, sizeof(struct mqtt_parm));
	memset(message_callback.message_line[mem_i].message, 0, strlen(message->payload) + 100);
	memcpy(message_callback.message_line[mem_i].message, message->payload, strlen(message->payload));
	message_callback.message_line[mem_i].inuse = 0;
	message_callback.message_line[mem_i].state = 1;	
	message_callback.message_line[mem_i].message_time = get_time_sec();
	//logout(INFO, "MQTT", "mqtt_message", "��ˮ�ţ�%s,�����%d����Ϣ����,��ǰ�����߳�����Ϊ%d��\r\n", parm.random, mem_i, message_callback.task_num);
	return;
}
//�����������������쳣����
static void analysis_request_cmd_not_support(void *arg)
{
	MODULE     *stream;
	//int size;
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	//cJSON *arrayItem, *item; //����ʹ��
	//cJSON *data_array;		 //���ʹ��
	//char errinfo[2048] = { 0 };

	//module_out("message_line->message = %s\n", message_line->message);
	//module_out("cmd  = %s\n", message_line->parm.cmd);
	//module_out("������%s���ⲿ����������,����Ϊ�ⲿӦ��������,����M����ģ�鴦��\n", message_line->parm.cmd);
	
	
	message_callback_deal_answer(&message_line->parm, message_line->message);

	//module_out("Mģ�鴦�����\n");
	
	free(message_line->message);	
	//mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
	goto End;

End:
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//module_out("�����ѯ˰��˰��ʱ�����\n");
	return;
}

//�����������ѯ������Ϣ
static void analysis_request_cxdlxx(void *arg)
{
	MODULE     *stream;
	int size;
	char random[150] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��

	char errinfo[2048] = {0};
	//module_out("message_line->message = %s\n", message_line->message);
	//module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "random");
	if (get_json_value_can_not_null(item, (char *)random, 0, 130) < 0) 
	{
		sprintf(errinfo, "�����JSON������data�ڵ���random����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	module_out("��ȡ����randomΪ%s\n", random);

	int i=0;
	int find = 0;
	for (i = 0; i < MAX_MESSAGE_QUEUE_NUM; i++)
	{
		if (strcmp(message_callback.message_line[i].parm.random, random) == 0)
		{
			find = 1;
			break;
		}
	}




	if (find != 1)
	{
		sprintf(errinfo, "δ�ҵ�random�ڵ�ǰ�����е�ʹ�ü�¼");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm,errinfo);
		goto End;
	}

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());

	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());
	char num_s[100] = { 0 };
	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", message_callback.insert_num);
	cJSON_AddStringToObject(data, "message_insert_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", message_callback.deal_num);
	cJSON_AddStringToObject(data, "message_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", message_callback.task_num);
	cJSON_AddStringToObject(data, "message_task_num", num_s);

	int wait_deal_num = 0;
	int now_deal_num = 0;

	for (i = 0; i < MAX_MESSAGE_QUEUE_NUM; i++)
	{
		if (message_callback.message_line[i].state == 1)
		{
			if (message_callback.message_line[i].inuse == 0)
				wait_deal_num += 1;
			else
				now_deal_num += 1;
		}
	}

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", MAX_MESSAGE_QUEUE_NUM);
	cJSON_AddStringToObject(data, "total_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", wait_deal_num);
	cJSON_AddStringToObject(data, "wait_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", now_deal_num);
	cJSON_AddStringToObject(data, "now_deal_num", num_s);
	


	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	//module_out("MQTT��������\n");
	//_lock_set(stream->lock);
	//_mqtt_client_pub(message_line->parm.source_topic, g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	//_lock_un(stream->lock);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,���,��ѯ��random��%s\r\n", message_line->parm.random, random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//module_out("�����ѯ˰�̷�Ʊ��Ϣ���\n");
	return;
}

//�����������ѯ��ʷ������
static void analysis_request_cxlscljg(void *arg)
{
	MODULE     *stream;
	int size;
	char random[150] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��

	char errinfo[2048] = { 0 };
	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "random");
	if (get_json_value_can_not_null(item, (char *)random, 0, 130) < 0)
	{
		sprintf(errinfo, "�����JSON������data�ڵ���random����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	module_out("��ȡ����randomΪ%s\n", random);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());

	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());

	int i = 0;
	int success_flag=0;
	for (i = 0; i < MAX_HISTROY_DEAL_RESULT; i++)
	{
		if (message_history.history_line[i].state == 0)
		{
			continue;
		}
		if (strcmp(message_history.history_line[i].random, random) == 0)
		{
			if (message_history.history_line[i].result == 0)
			{
				cJSON_AddStringToObject(data, "result", "f");
				cJSON_AddStringToObject(data, "result_data", "");
			}
			else
			{
				cJSON_AddStringToObject(data, "result", "s");
				cJSON_AddStringToObject(data, "result_data", message_history.history_line[i].data);
			}
			if (strcmp(message_history.history_line[i].random, random) == 0)//�ٴ��ж��ڴ�δ���
			{
				success_flag = 1;
			}
			break;
		}
	}
	if (success_flag != 1)
	{
		sprintf(errinfo, "δ�ҵ�random��ʷ��ʹ�ü�¼");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,%s,��ѯ��random��%s\r\n", message_line->parm.random, errinfo,random);
		cJSON_Delete(json);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ������,���,��ѯ��random��%s\r\n", message_line->parm.random, random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//module_out("�����ѯ˰�̷�Ʊ��Ϣ���\n");
	return;
}


//�����������ѯ�ն�ȫ����Ϣ
static void analysis_request_cxzdqbxx(void *arg)
{
	MODULE     *stream;	
	struct _message_line *message_line;
	int size;

	USB_PORT_CLASS *usb_port_class = get_usb_port_class();
	//int result;
	message_line = arg;
	stream = &module;
	cJSON *data_array;		 //���ʹ��

	char errinfo[2048] = {0};
	//module_out("message_line->message = %s\n", message_line->message);
	//module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm,errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	char version[64] = {0x00};
	

    // usb_port_class->online_bank_action.mechanical_arm_action.class->fun_set_terid(80,"068331001017");


	//  usb_port_class->online_bank_action.mechanical_arm_action.class->fun_get_terid(80,version);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());
	cJSON_AddStringToObject(data, "use_compress", "1");
	cJSON_AddStringToObject(data, "compress_type", "zlib");



	cJSON *data_layer=cJSON_CreateObject();
	//cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	char num_s[100] = {0};
	memset(num_s,0,sizeof(num_s));
	sprintf(num_s, "%d", message_callback.insert_num);
	cJSON_AddStringToObject(data_layer, "message_insert_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", message_callback.deal_num);
	cJSON_AddStringToObject(data_layer, "message_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", message_callback.task_num);
	cJSON_AddStringToObject(data_layer, "message_task_num", num_s);


	int wait_deal_num = 0;
	int now_deal_num = 0;
	int count;
	for (count = 0; count < MAX_MESSAGE_QUEUE_NUM; count++)
	{
		if (message_callback.message_line[count].state == 1)
		{
			if (message_callback.message_line[count].inuse == 0)
				wait_deal_num += 1;
			else
				now_deal_num += 1;
		}
	}

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", MAX_MESSAGE_QUEUE_NUM);
	cJSON_AddStringToObject(data_layer, "total_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", wait_deal_num);
	cJSON_AddStringToObject(data_layer, "wait_deal_num", num_s);

	memset(num_s, 0, sizeof(num_s));
	sprintf(num_s, "%d", now_deal_num);
	cJSON_AddStringToObject(data_layer, "now_deal_num", num_s);
	
	if(NULL != usb_port_class->fun_get_ter_all_data_json)
		usb_port_class->fun_get_ter_all_data_json(data_layer);

	char *data_buf;
	char *compress;
	int data_len;
	int compress_len;
	data_buf = cJSON_Print(data_layer);

	str_replace(data_buf, "\\\\", "\\");
	
	data_len = strlen(data_buf);
	compress = calloc(1, data_len + 2048);
	if (compress == NULL)
	{
		sprintf(errinfo,"�ڴ�����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(data_buf);
		cJSON_Delete(root);
		cJSON_Delete(json);
		cJSON_Delete(data_layer);		
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo);
		goto End;
	}

	asc_compress_base64((const unsigned char *)data_buf, data_len, (unsigned char *)compress, &compress_len);
	cJSON_AddStringToObject(data, "zdqbxx", compress);
	free(data_buf);
	free(compress);
	//module_out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);


	str_replace(g_buf, "\\\\", "\\");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	cJSON_Delete(data_layer);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}



//��ѯ�ն˻�����Ϣ
static void analysis_request_cxzdjcxx(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;

	//int result;
	message_line = arg;
	stream = &module;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	//module_out("message_line->message = %s\n", message_line->message);
	//module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}



	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());



	fun_get_ter_base_data_json(data);


	

	//module_out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);

	str_replace(g_buf, "\\\\", "\\");

	//module_out("�ն˻�����Ϣ���ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);

	//module_out("�������ж˿��ٴ��ϱ�\n");

	//fun_deal_all_port_report();


	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}


//֪ͨ�ն˻�ȡ��������Ȩ�ļ�
static void analysis_request_tzzdsjsq(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;
	//char tmp[50];
	char ter_id[20] = {0};
	char notice_type[20] = { 0 };
	//int result;
	message_line = arg;
	stream = &module;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	cJSON *item;
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	
	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ter_id����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(ter_id, 0, sizeof(ter_id));
		memcpy(ter_id, item->valuestring, strlen(item->valuestring));
	}
	module_out("��ȡ���Ļ������Ϊ%s\n", ter_id);


	item = cJSON_GetObjectItem(arrayItem, "notice_type");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���notice_type����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(notice_type, 0, sizeof(notice_type));
		memcpy(notice_type, item->valuestring, strlen(item->valuestring));
	}
	module_out("��ȡ����֪ͨ����Ϊ%s\n", notice_type);

	
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	
	if (strcmp(ter_info.ter_id, ter_id) != 0)
	{
		sprintf(errinfo, "������Ų�һ�£��ն˻����ܾ�");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	if (strcmp(notice_type, "auth") == 0)
		;//write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ������Ȩ�ļ�", strlen("������Ҫ��ϵͳ������Ȩ�ļ�"));
	else if (strcmp(notice_type, "update") == 0)
		write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ���������ļ�", strlen("������Ҫ��ϵͳ���������ļ�"));
	else
	{
		sprintf(errinfo, "֪ͨ���ʹ���,�޷�֧��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());


	//module_out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);
	module_out("�ն�Ӧ�����ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

//��Ȩ�ն˵Ķ˿�
static void analysis_request_sqzddk(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;
	//char tmp[50];
	char ter_id[20] = { 0 };
	char auth_data[10*1024] = { 0 };
	char auth_md5[200] = { 0 };
	//int result;
	message_line = arg;
	stream = &module;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	cJSON *item;
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}


	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ter_id����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(ter_id, 0, sizeof(ter_id));
		memcpy(ter_id, item->valuestring, strlen(item->valuestring));
	}
	module_out("��ȡ���Ļ������Ϊ%s\n", ter_id);


	item = cJSON_GetObjectItem(arrayItem, "auth_data");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���auth_data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(auth_data, 0, sizeof(auth_data));
		memcpy(auth_data, item->valuestring, strlen(item->valuestring));
	}
	module_out("��ȡ���ļ�����Ȩ�ļ���ϢΪ%s\n", auth_data);


	item = cJSON_GetObjectItem(arrayItem, "auth_md5");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���auth_md5����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(auth_md5, 0, sizeof(auth_md5));
		memcpy(auth_md5, item->valuestring, strlen(item->valuestring));
	}
	module_out("��ȡ���ļ�����Ȩ�ļ���Ϣauth_md5Ϊ:%s\n", auth_md5);


	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);


	if (strcmp(ter_info.ter_id, ter_id) != 0)
	{
		sprintf(errinfo, "������Ų�һ�£��ն˻����ܾ�");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	char auth_file[10 * 1024] = {0};

	int auth_file_len = sizeof(auth_file);
	uncompress_asc_base64((const uint8 *)auth_data, strlen((const char *)auth_data), (uint8 *)auth_file, &auth_file_len);


	char auth_file_md5[200] = { 0 };
	md5_value(auth_file, strlen(auth_file), auth_file_md5);


	module_out("��Ȩ�����ļ�md5ֵΪ:%s\n", auth_file_md5);

	if (strcmp(auth_file_md5, auth_md5) != 0)
	{
		module_out("�ļ�MD5ֵ�봫���json�ֶ�MD5ֵ��һ��\n");
		sprintf(errinfo, "MD5ֵУ���쳣ֹͣ��Ȩ����");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	if (access("/etc/auth_cert.json", 0) == 0)
	{
		system("rm /etc/auth_cert.json");
	}
	write_file("/etc/auth_cert.json", auth_file, strlen(auth_file));

	(get_usb_port_class())->key_auth_state.upload_auth_cert_file(NULL);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data;
	cJSON_AddItemToObject(data_array, "dira", data = cJSON_CreateObject());


	//module_out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);
	module_out("�ն�Ӧ�����ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

//�����������л�����ʡ��ģʽ
static void analysis_request_qhzjsdms(void *arg)
{
	MODULE     *stream;
	int size;
	int save_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��


	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	//������ݽ���
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	else
	{
		//module_out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л�����ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "ter_save_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���usb_port����ʧ��");
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л�����ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			save_state = atoi(item->valuestring);
			module_out("��ȡ����������Դ״̬Ϊ%d\n", save_state);


		}
	}

	//result = change_all_port_power_mode(save_state);
	if (result < 0)
	{
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_TER_SYSTEM_ERROR, &message_line->parm, errinfo);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л�����ʡ��ģʽ:[%d]�ɹ�,��������\r\n", message_line->parm.random, save_state);
	sleep(2);
	sync();
	exit(0);
	system("reboot");
	return;
}
//�����������л��˿�ʡ��ģʽ
static void analysis_request_qhdksdms(void *arg)
{
	MODULE     *stream;
	int size;
	int usb_port;
	int save_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��


	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	//������ݽ���
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	else
	{
		//module_out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}


			char usb_port_string[100] = { 0 };
			char save_state_string[100] = { 0 };
			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���usb_port����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			if (get_json_value_can_not_null(item, usb_port_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���usb_port����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			usb_port = atoi(usb_port_string);
			module_out("��ȡ����USB�˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "save_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			if (get_json_value_can_not_null(item, save_state_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			save_state = atoi(save_state_string);
			module_out("��ȡ����USB�˿ںŵ�Դ״̬Ϊ%d\n", save_state);


		}
	}

	//result = change_port_power_mode(usb_port, save_state);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,��%d��USB�˿�,�ײ������Ϣ��%s\r\n", message_line->parm.random, usb_port, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_PLATE_OPER_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,��%d��USB�˿�,���\r\n", message_line->parm.random, usb_port);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

//�ر�USB�˿�
static void analysis_request_closeport(void *arg)
{

	MODULE     *stream;
	char sendbuf[1024] = { 0 };
	struct _message_line *message_line;
	int port = 0;
	char port_str[10] = { 0 };
	int result;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};

	struct _new_usbshare_parm parm;

	LOAD_DATA load_data = {0x00};
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
	if(NULL == usb_port_class->load_action.fun_load_stop)
		goto End;

	memset(&parm, 0, sizeof(struct _new_usbshare_parm));

	memcpy(parm.source_topic, message_line->parm.source_topic, strlen(message_line->parm.source_topic));
	memcpy(parm.random, message_line->parm.random, strlen(message_line->parm.random));

	cJSON *item;
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random,errinfo);
		goto End;
	}
	int size;
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		module_out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "port");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���port����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(port_str, 0, sizeof(port_str));
			memcpy(port_str, item->valuestring, strlen(item->valuestring));
		}
	}
	int start;
	start = ty_ctl(stream->deploy_fd, DF_DEPLOY_CM_GET_START_PORT);
	if (start<0)
	{
		sprintf(errinfo, "��ȡϵͳ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		int err_code;
		err_code = MQTT_GW_USBSHARE_CLOSE_PORT_ERR_GET_START;
		mqtt_response_errno_gw(&parm, err_code);
		cJSON_Delete(root);
		goto End;
	}
	port = atoi(port_str);
	if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //���ж��Ƿ��ǻ�е�۵Ļ���
		parm.port = port * 2;
	else	
		parm.port = port;
	port = port - start + 1;
	module_out("ǿ�ƹرն˿ڡ�%d��\n", port);

	load_data.port = (parm.port - 1) > 0?(parm.port-1):0;
	result = *((int *)usb_port_class->load_action.fun_load_stop((void *)(&load_data))); //ִ��һ���ͷ�
	//result = close_port_dettach(port);
	if (result<0)
	{
		sprintf(errinfo, "ǿ�ƹر�ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		int err_code;
		err_code = MQTT_GW_USBSHARE_CLOSE_PORT_ERR_CLOSE;
		mqtt_response_errno_gw(&parm, err_code);
		cJSON_Delete(root);
		goto End;
	}
	struct _switch_dev_id   id;

	_lock_set(stream->data_lock);
	memset(&id, 0, sizeof(id));
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, (uint8*)(parm.mechine_id), 6);
	_lock_un(stream->data_lock);

	module_out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
	
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,�ɹ�,�˿ںţ�%d\r\n", message_line->parm.random, port);

	prepare_mqtt_gw_close_packet(sendbuf, &parm);
	module_out("MQTT ���ر���Ϊ:%s\n", sendbuf);
	mqtt_respoonse_port(sendbuf, &parm);//ͨ��MQTT ����

	cJSON_Delete(root);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
}

//����һ�λ�е��
static void analysis_request_start_robot_arm(void *arg)
{
	MODULE     *stream;
	char sendbuf[1024] = { 0 };
	struct _message_line *message_line;
	char port_str[10] = { 0 };
	int result = -1;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};

	struct _new_usbshare_parm parm;

	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	memset(&parm, 0, sizeof(struct _new_usbshare_parm));
	memcpy(parm.source_topic, message_line->parm.source_topic, strlen(message_line->parm.source_topic));
	memcpy(parm.random, message_line->parm.random, strlen(message_line->parm.random));

	// if(1 != *((int *)usb_port_class->fun_check_with_robot_arm(NULL)))  //�ǻ�е�ۻ��ͣ�����������е��
	// {
	// 	sprintf(errinfo, "�ǻ�е�ۻ���");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
	// 	strcpy(parm.cmd,"start_robot_arm");
	// 	mqtt_response_errno_gw(&parm, MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE);
	// 	goto End;
	// }

	if(NULL == usb_port_class->online_bank_action.mechanical_arm_action.arm_start)
		goto End;

	

	cJSON *item;
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random,errinfo);
		goto End;
	}
	int size;
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		module_out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "port");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���port����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(port_str, 0, sizeof(port_str));
			memcpy(port_str, item->valuestring, strlen(item->valuestring));
		}
	}
	int port;

	parm.port = atoi(port_str);

	port = parm.port;

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������е��,�˿ںţ�%d\r\n", message_line->parm.random, parm.port);
	module_out("������е�ۡ�%d��\n", parm.port);

	port = (port - 1) > 0?(port-1):0;

	printf("lbc222 %d\n",(port));

	// result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
	// if(result)
	{
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port_info.arm_version))
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(port))  //�ж��Ƿ���������
			{
				result = MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING;
			}
			else
			{
				result = 0;
			}
		}
		else
		{
			result = MQTT_GW_USBSHARE_ROBOTARM_NOT_WORK;
		}
	}
	// else
	// {
	// 	result = MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE;
	// }
	
	

	// if(result >= 0)
	// {
	// 	printf("push_value = %d\n",get_test_model_calss()->fun_get_push_value(port));
	// }

	//printf("delay us = %d\n",get_net_delay("103.27.4.61"));
	//ip_conflict_check("eth0","192.168.0.31");

	//result = close_port_dettach(port);
	if (result<0 || result >= MQTT_GW_USBSHARE_OPEN_PORT_ERR_USED)
	{
		sprintf(errinfo, "������е��ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		int err_code;
		if(result == -1)
			err_code = MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_WITHOUT_DEV;//MQTT_GW_USBSHARE_OPERA_ERR;
		else if(result == -2)
			err_code = MQTT_GW_USBSHARE_ROBOT_ARM_FAILED_NO_SATRT;
		else
			err_code = result;

		strcpy(parm.cmd,"start_robot_arm");
		mqtt_response_errno_gw(&parm, err_code);
		cJSON_Delete(root);
		goto End;
	}
	// struct _switch_dev_id   id;

	// _lock_set(stream->data_lock);
	// memset(&id, 0, sizeof(id));
	// result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	// bcd2asc(id.id, (uint8*)(parm.mechine_id), 6);
	// _lock_un(stream->data_lock);

	// module_out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
	
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,�ɹ�,�˿ںţ�%d\r\n", message_line->parm.random, parm.port);

	prepare_mqtt_gw_robot_arm_packet(sendbuf, &parm,"start_robot_arm");
	module_out("MQTT ���ر���Ϊ:%s\n", sendbuf);
	mqtt_respoonse_port(sendbuf, &parm);//ͨ��MQTT ����
	cJSON_Delete(root);

	if(result == 0)
	{
		result = *((int *)(usb_port_class->online_bank_action.mechanical_arm_action.arm_start((void *)(&port))));
		if(result == 0)
			result = 1;
		else if(result == -1)
			result = 0;
		else
			result = 0;
		if(usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result != result)
		{
			
			//usb_port_class->get_dev_basic_action.usb_dev_basic_data[(port+1)/2*2+1].up_report = 1;   //�ϱ�һ�λ�е���������豸
			usb_port_class->online_bank_action.mechanical_arm_action.arm_with_dev[port].arm_with_dev_result = result;  //״̬���µ�����
			//�����ϱ�������Ϣ
			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].port = port + 1;
			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].up_report = 1;
			usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port].last_act = 1;
			usb_port_class->report_action.report_data.report_ter_flag_report = 0;
		}
	}

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
}

/**********************************************************************
 *
 * File name    : base64.cpp / base64.c
 * Function     : base64 encoding and decoding of data or file.
 * Created time : 2020-08-04
 *
 *********************************************************************/

//base64 ����ת������64��
static const char base64_encode_table[] = {
    'A','B','C','D','E','F','G','H','I','J',
    'K','L','M','N','O','P','Q','R','S','T',
    'U','V','W','X','Y','Z','a','b','c','d',
    'e','f','g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v','w','x',
    'y','z','0','1','2','3','4','5','6','7',
    '8','9','+','/'
};

//base64 �����
static const unsigned char base64_decode_table[] = {
    //ÿ��16��
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                //1 - 16
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                //17 - 32
    0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,              //33 - 48
    52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,      //49 - 64
    0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,           //65 - 80
    15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,     //81 - 96
    0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, //97 - 112
    41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0      //113 - 128
};

/**
 * @brief base64_encode     base64����
 * @param indata            ����������
 * @param inlen             ���������ݴ�С
 * @param outdata           ��������������
 * @param outlen            �������������ݴ�С
 * @return  int             0���ɹ�    -1����Ч����
 */
int base64_encode(const char *indata, int inlen, char *outdata, int *outlen)
{
    if(indata == NULL || inlen <= 0) {
        return -1;
    }
/*
    //����һ��
    int i, j;
    char ch;
    int add_len = (inlen % 3 == 0 ? 0 : 3 - inlen % 3); //ԭ�ַ����貹����ַ�����
    int in_len = inlen + add_len; //Դ�ַ��������ַ���ĳ��ȣ�Ϊ3�ı���
    if(outdata != NULL) {
        //���룬����Ϊ����֮��ĳ��ȣ�3�ֽ�һ��
        for(i=0, j=0; i<in_len; i+=3, j+=4) {
            //��indata��һ���ַ������ƶ�2bit������2bit��
            ch = base64_encode_table[(unsigned char)indata[i] >> 2]; //��Ӧbase64ת������ַ�
            outdata[j] = ch; //��ֵ

            //�������һ�飨���3���ֽڣ�������
            if(i == in_len - 3 && add_len != 0) {
                if(add_len == 1) {
                    outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
                    outdata[j + 2] = base64_encode_table[((unsigned char)indata[i + 1] & 0x0f) << 2];
                    outdata[j + 3] = '=';
                }
                else if(add_len == 2) {
                    outdata[j + 1] = base64_encode_table[((unsigned char)indata[i] & 0x03) << 4];
                    outdata[j + 2] = '=';
                    outdata[j + 3] = '=';
                }
            }
            //����������3�ֽ�����
            else {
                outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
                outdata[j + 2] = base64_encode_table[(((unsigned char)indata[i + 1] & 0x0f) << 2) | ((unsigned char)indata[i + 2] >> 6)];
                outdata[j + 3] = base64_encode_table[(unsigned char)indata[i + 2] & 0x3f];
            }
        }
    }
    if(outlen != NULL) {
        *outlen = in_len * 4 / 3; //�����ĳ���
    }
*/
    //��������
    int i, j;
    unsigned char num = inlen % 3;
    if(outdata != NULL) {
        //���룬3���ֽ�һ�飬�������ܳ��Ȳ���3�ı��������������� num ���ֽ�����
        for(i=0, j=0; i<inlen - num; i+=3, j+=4) {
            outdata[j] = base64_encode_table[(unsigned char)indata[i] >> 2];
            outdata[j + 1] = base64_encode_table[(((unsigned char)indata[i] & 0x03) << 4) | ((unsigned char)indata[i + 1] >> 4)];
            outdata[j + 2] = base64_encode_table[(((unsigned char)indata[i + 1] & 0x0f) << 2) | ((unsigned char)indata[i + 2] >> 6)];
            outdata[j + 3] = base64_encode_table[(unsigned char)indata[i + 2] & 0x3f];
        }
        //������������ num ���ֽڵ�����
        if(num == 1) { //����Ϊ1���貹�������ֽ�'='
            outdata[j] = base64_encode_table[(unsigned char)indata[inlen - 1] >> 2];
            outdata[j + 1] = base64_encode_table[((unsigned char)indata[inlen - 1] & 0x03) << 4];
            outdata[j + 2] = '=';
            outdata[j + 3] = '=';
        }
        else if(num == 2) { //����Ϊ2���貹��һ���ֽ�'='
            outdata[j] = base64_encode_table[(unsigned char)indata[inlen - 2] >> 2];
            outdata[j + 1] = base64_encode_table[(((unsigned char)indata[inlen - 2] & 0x03) << 4) | ((unsigned char)indata[inlen - 1] >> 4)];
            outdata[j + 2] = base64_encode_table[((unsigned char)indata[inlen - 1] & 0x0f) << 2];
            outdata[j + 3] = '=';
        }
    }
    if(outlen != NULL) {
        *outlen = (inlen + (num == 0 ? 0 : 3 - num)) * 4 / 3; //�����ĳ���
    }

    return 0;
}

/**
 * @brief base64_decode     base64����
 * @param indata            ����������
 * @param inlen             ���������ݴ�С
 * @param outdata           ��������������
 * @param outlen            �������������ݴ�С
 * @return  int             0���ɹ�    -1����Ч����
 * ע�⣺��������ݵĴ�С�������4������4�ı���
 */
int base64_decode(const char *indata, int inlen, char *outdata, int *outlen)
{
    if(indata == NULL || inlen <= 0 || (outdata == NULL && outlen == NULL)) {
        return -1;
    }
    if(inlen < 4 ||inlen % 4 != 0) { //��Ҫ��������ݳ��Ȳ���4�ı���  //inlen < 4 ||
        return -1;
    }

    int i, j;

    //����������ַ�������
    int len = inlen / 4 * 3;
    if(indata[inlen - 1] == '=') {
        len--;
    }
    if(indata[inlen - 2] == '=') {
        len--;
    }

    if(outdata != NULL) {
        for(i=0, j=0; i<inlen; i+=4, j+=3) {
            outdata[j] = (base64_decode_table[(unsigned char)indata[i]] << 2) | (base64_decode_table[(unsigned char)indata[i + 1]] >> 4);
            outdata[j + 1] = (base64_decode_table[(unsigned char)indata[i + 1]] << 4) | (base64_decode_table[(unsigned char)indata[i + 2]] >> 2);
            outdata[j + 2] = (base64_decode_table[(unsigned char)indata[i + 2]] << 6) | (base64_decode_table[(unsigned char)indata[i + 3]]);
        }
    }
    if(outlen != NULL) {
        *outlen = len;
    }
    return 0;
}

/**
 * @brief base64_encode_file    base64�����ļ�
 * @param src                   �������ļ�·��
 * @param dst                   �����������ļ�·��
 * @return  int                 0���ɹ�    -1����Ч����     -2���ļ�����ʧ��
 */
int base64_encode_file(const char *src, const char *dst)
{
    if(src == NULL || dst == NULL) {
        return -1;
    }

    FILE *src_fp, *dst_fp;
    char rdata[128*3+3]; //��Ŷ�ȡ�����ļ����ݣ�+3��ʾԤ��3���ֽڿռ���������������
    size_t rmemb; //���ļ����ݷ���ֵ����ȡ���Ŀ���
    size_t nmemb = sizeof(rdata) - 3; //ÿ�ζ�ȡ�ļ����ݵĿ����������3�ı���
    char encode_data[(nmemb+(nmemb%3==0?0:3-nmemb%3))*4/3+1]; //��ű���������
    int encode_datalen; //���������ݴ�С
    unsigned char num = 0, lastnum = 0;

    src_fp = fopen(src, "rb");
    if(NULL == src_fp) {
        perror("open src file failed");
        return -2;
    }
    dst_fp = fopen(dst, "wb");
    if(NULL == dst_fp) {
        fclose(src_fp);
        perror("open dst file failed");
        return -2;
    }
    while(1) {
        //memset(rdata, 0, sizeof(rdata));
        //memset(encode_data, 0, sizeof(encode_data));
        encode_datalen = 0;
        rmemb = fread(rdata + lastnum, 1, nmemb, src_fp);
        if((lastnum + rmemb) % 3 == 0 || rmemb < nmemb) { //��ȡ�����������ϴ��������������ܴ�С��3�ı��� �� �ļ��Ѷ��꣨�����
            base64_encode(rdata, lastnum + rmemb, encode_data, &encode_datalen);
            fwrite(encode_data, 1, encode_datalen, dst_fp);
            lastnum = 0;
        }
        else {
            num = (lastnum + rmemb) % 3; //���������ֽ���
            base64_encode(rdata, lastnum + rmemb - num, encode_data, &encode_datalen);
            fwrite(encode_data, 1, encode_datalen, dst_fp);
            //���������������ƶ�����������ǰ��
            if(num == 1) {
                rdata[0] = rdata[lastnum + rmemb - 1];
            }
            else if(num == 2) {
                rdata[0] = rdata[lastnum + rmemb - 2];
                rdata[1] = rdata[lastnum + rmemb - 1];
            }
            lastnum = num;
        }
        if(rmemb < nmemb) { //�ļ��Ѷ��� �� ����
            break;
        }
    }
    fclose(src_fp);
    fclose(dst_fp);
    return 0;
}

/**
 * @brief base64_decode_file    base64�����ļ�
 * @param src                   �������ļ�·��
 * @param dst                   �����������ļ�·��
 * @return  int                 0���ɹ�    -1����Ч����     -2���ļ�����ʧ��
 */
int base64_decode_file(const char *src, const char *dst)
{
    if(src == NULL || dst == NULL) {
        return -1;
    }

    FILE *src_fp, *dst_fp;
    char rdata[128*4]; //��Ŷ�ȡ�����ļ�����
    size_t rmemb; //���ļ����ݷ���ֵ����ȡ���Ŀ���
    size_t nmemb = sizeof(rdata); //ÿ�ζ�ȡ�ļ����ݵĿ����������4�ı���
    char decode_data[nmemb/4*3+1]; //��Ž��������ݣ���С����
    int decode_datalen; //���������ݴ�С
    unsigned char num = 0, lastnum = 0;

    src_fp = fopen(src, "rb");
    if(NULL == src_fp) {
        perror("open src file failed");
        return -2;
    }
    dst_fp = fopen(dst, "wb");
    if(NULL == dst_fp) {
        fclose(src_fp);
        perror("open dst file failed");
        return -2;
    }
    while(1) {
        //memset(rdata, 0, sizeof(rdata));
        //memset(encode_data, 0, sizeof(encode_data));
        decode_datalen = 0;
        rmemb = fread(rdata + lastnum, 1, nmemb, src_fp);
        if((lastnum + rmemb) % 4 == 0 || rmemb < nmemb) { //��ȡ�����������ϴ��������������ܴ�С��4�ı��� �� �ļ��Ѷ��꣨�����
            base64_decode(rdata, lastnum + rmemb, decode_data, &decode_datalen);
            fwrite(decode_data, 1, decode_datalen, dst_fp);
            lastnum = 0;
        }
        else {
            num = (lastnum + rmemb) % 4; //���������ֽ���
            base64_decode(rdata, lastnum + rmemb - num, decode_data, &decode_datalen);
            fwrite(decode_data, 1, decode_datalen, dst_fp);
            //���������������ƶ�����������ǰ��
            if(num == 1) {
                rdata[0] = rdata[lastnum + rmemb - 1];
            }
            else if(num == 2) {
                rdata[0] = rdata[lastnum + rmemb - 2];
                rdata[1] = rdata[lastnum + rmemb - 1];
            }
            else if(num == 3) {
                rdata[0] = rdata[lastnum + rmemb - 3];
                rdata[1] = rdata[lastnum + rmemb - 2];
                rdata[2] = rdata[lastnum + rmemb - 1];
            }
            lastnum = num;
        }
        if(rmemb < nmemb) { //�ļ��Ѷ��� �� ����
            break;
        }
    }
    fclose(src_fp);
    fclose(dst_fp);
    return 0;
}


void update_robot_arm_read_callback(struct evhttp_request* remote_rsp, void* arg)
{
	MD5_CTX ctx;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��
	int i = 0,n = 0,file_len = 0,file_data_len = 0;
	char *buf = NULL,*ptr = NULL,*file_data = NULL;
	cJSON *recv_root = NULL, *item = NULL, *res = NULL;
	char md5_value[64] = {0x00},my_md5_value[64] = {0x00},my_md5_value_string[64] = {0x00},tmpdata[12] = {0x00};
	struct evbuffer* evbuf = evhttp_request_get_input_buffer(remote_rsp);
	buf = (char *)malloc(500*1024);
	if(buf == NULL || usb_port_class == NULL)
	{
		return;
	}
	ptr = buf;
	while ((n = evbuffer_remove(evbuf, ptr, 500*1024)) > 0)
	{
		//fwrite(buf, n, 1, stdout);
		ptr += n;
		usleep(1000);
		evbuf = evhttp_request_get_input_buffer(remote_rsp);
	}
	// printf("update_robot_arm_read_callback buf = %s\n",buf);
	recv_root = cJSON_Parse((const char *)buf);
	if (recv_root == NULL)
	{
		printf("err recv get ter update,is not a json !\n");
		free(buf);
		return;
	}
	printf("update_robot_arm_read_callback 1111111111111111111111111\n");
	item = cJSON_GetObjectItem(recv_root, "result");
	if (item == NULL)
	{
		printf("err recv get ter update:json err1\n");
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	printf("update_robot_arm_read_callback item->valuestring=%s\n",item->valuestring);
	if (memcmp(item->valuestring, "s", 1) != 0)
	{
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	item = cJSON_GetObjectItem(recv_root, "response");
	if (item == NULL)
	{
		printf("err recv get ter update:json err2\n");
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	res = cJSON_GetObjectItem(item, "roboticArmfilelen");
	if (res == NULL)
	{
		printf("err recv get ter update:roboticArmfilelen not found\n");
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	file_len = (res->valueint);
	printf("get file len:%ld!\n", file_len);
	res = cJSON_GetObjectItem(item, "roboticArmfileMd5");
	if (res == NULL)
	{
		printf("err recv get ter update:upgradefile not found\n");
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	
	strcpy(md5_value,res->valuestring);
	printf("md5_value:%s!\n", md5_value);

	res = cJSON_GetObjectItem(item, "roboticArmfileVersion");
	if (res == NULL)
	{
		printf("err recv get ter update:roboticArmfileVersion not found\n");
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	strcpy(usb_port_class->upgrade_strategy.file_version,res->valuestring);
	printf("usb_port_class->upgrade_strategy.file_version:%s!\n", usb_port_class->upgrade_strategy.file_version);
	write_file(ROBOT_ARM_UPDATE_VERSION_FILE_PATH,usb_port_class->upgrade_strategy.file_version,strlen(usb_port_class->upgrade_strategy.file_version));
	res = cJSON_GetObjectItem(item, "roboticArmfile");
	if (res == NULL)
	{
		printf("err recv get ter update:roboticArmfile not found\n");//usb_port_class
		cJSON_Delete(recv_root);
		free(buf);
		return;
	}
	memset(buf,0x00,sizeof(buf));
	strcpy(buf,res->valuestring);
	// printf("file:%s!\n", buf);
	file_data = (char *)malloc(100*1024);
	if(file_data == NULL)
	{
		free(buf);
		return;
	}
	base64_decode((uint8 *)buf, strlen(buf),(uint8 *)file_data,&file_data_len);
	printf("file_data : file_data_len=%d,strlen(buf)=%d\n",file_data_len,strlen(buf));
	// for(i = 0;i<file_data_len;++i)
	// {
	// 	printf("%02x ",file_data[i]);
	// }
	printf("\n");
	MD5_Init(&ctx);
	MD5_Update(&ctx, file_data, file_data_len);
	MD5_Final(my_md5_value, &ctx);
	printf("my_md5_value\n");
	for(i = 0;i<16;++i)
	{
		printf("%02x ",my_md5_value[i]);
	}
	printf("\n");
	for(i = 0;i<16;++i)
	{
		memset(tmpdata,0x00,sizeof(tmpdata));
		sprintf(tmpdata,"%02x",my_md5_value[i]);
		strcat(my_md5_value_string,tmpdata);
	}
	printf("roboticArmfileMd5 = %s,my_md5_value_string =%s\n",md5_value,my_md5_value_string);
	// if(0 != memcmp(my_md5_value_string,md5_value,strlen(md5_value)))   //md5ֵ��ͬ������������
	// {
	// 	free(buf);
	// 	free(file_data);
	// 	cJSON_Delete(recv_root);
	// 	return;
	// }
	write_file(ROBOT_ARM_UPDATE_FILE_PATH,file_data,file_len);
	free(buf);
	free(file_data);
	cJSON_Delete(recv_root);
	event_base_loopexit((struct event_base*)(arg), NULL);
}

//������е��
static void analysis_request_update_robot_arm(void *arg)
{
	MODULE	*stream = NULL;
	char sendbuf[1024] = { 0 };
	struct _message_line *message_line = NULL;
	char port_str[1024] = { 0 };
	int port_array[ROBOT_ARM_NUM] = {0x00};
	char *p = NULL;
	char down_load_addr[1024] = { 0 };
	char down_load_addr_base[1024] = { 0 };
	int result,count = 0,i = 0;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};

	struct _new_usbshare_parm parm;

	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	memset(&parm, 0, sizeof(struct _new_usbshare_parm));
	memcpy(parm.source_topic, message_line->parm.source_topic, strlen(message_line->parm.source_topic));
	memcpy(parm.random, message_line->parm.random, strlen(message_line->parm.random));

	// if(1 != *((int *)usb_port_class->fun_check_with_robot_arm(NULL)))  //�ǻ�е�ۻ��ͣ�����������е��
	// {
	// 	sprintf(errinfo, "�ǻ�е�ۻ���");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
	// 	strcpy(parm.cmd,"start_robot_arm");
	// 	mqtt_response_errno_gw(&parm, MQTT_GW_USBSHARE_ISNOT_ROBOTARM_TYPE);
	// 	goto End;
	// }
	if(NULL == usb_port_class->online_bank_action.mechanical_arm_action.arm_start)
		goto End;

	

	cJSON *item;
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random,errinfo);
		goto End;
	}
	int size;
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		module_out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "ports");  //down_load_addr
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ports����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(port_str, 0, sizeof(port_str));
			memcpy(port_str, item->valuestring, strlen(item->valuestring));
			printf("port_str = %s\n",port_str);
			write_file(ROBOT_ARM_UPDATE_FLAG_FILE_PATH,port_str,strlen(port_str));
		}
	}
	item = cJSON_GetObjectItem(arrayItem, "download");  //down_load_addr
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���download����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(down_load_addr, 0, sizeof(down_load_addr));
			memcpy(down_load_addr, item->valuestring, strlen(item->valuestring));
		}
	}

	module_out("������е��\n");
	//�����ļ�����  down_load_addr


	//�Ȼظ�
	struct _switch_dev_id   id;

	_lock_set(stream->data_lock);
	memset(&id, 0, sizeof(id));
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, (uint8*)(parm.mechine_id), 6);
	_lock_un(stream->data_lock);
	prepare_mqtt_gw_robot_arm_packet(sendbuf, &parm,"update_robot_arm");
	module_out("MQTT ���ر���Ϊ:%s\n", sendbuf);
	mqtt_respoonse_port(sendbuf, &parm);//ͨ��MQTT ����

	//������
	base64_dec((uint8 *)down_load_addr_base, (uint8 *)down_load_addr, strlen(down_load_addr));
	printf("down_load_addr_base = %s\n",down_load_addr_base);

	result = https_get(down_load_addr_base,NULL,0,NULL,NULL,update_robot_arm_read_callback,NULL,&result);

	//�ȴ�5s
	//sleep(5);

	if(result < 0)
		goto End;

	// char*temp = strtok(buf,"@");
    // while(temp)
    // {
    //     printf("%s ",temp);
    //     temp = strtok(NULL,"@");
    // }

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,�˿ڣ�%s\r\n", message_line->parm.random, port_str);
	
	for (p = strtok(port_str, ",");p != NULL;p=strtok(NULL,",")) {
		printf("%s\n", p);
		port_array[count++] = atoi(p);
	}

	for(i = 0;i<count;++i)
	{
		printf("port[%d] = %d\n",i,port_array[i]);
		if(port_array[i] >=0 && port_array[i] < DF_TY_USB_PORT_N_MAX/2)
		{
			usb_port_class->upgrade_strategy.need_update_port_num[port_array[i]] = 1;
		}
		//usb_port_class->online_bank_action.mechanical_arm_action.arm_update((void *)(&port_array[i]));
	}
	int port;
	if (result<0)
	{
		sprintf(errinfo, "������е��ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,%s\r\n", message_line->parm.random, errinfo);
		int err_code;
		err_code = MQTT_GW_USBSHARE_OPERA_ERR;
		mqtt_response_errno_gw(&parm, err_code);
		cJSON_Delete(root);
		goto End;
	}
	

	// module_out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
	
	

	

	cJSON_Delete(root);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������е��,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
}


int get_transport_line(struct _new_usbshare_parm *parm)
{
	int result;
	module_out("get_transport_line host:%s--port:%d-- clientid:%s\n", parm->trans_server, parm->port, parm->client_id);
	//result = ty_ctl(module.tran_fd, DF_OPEN_TRANSPORT_LINE, parm);//������ع���usbָ��
	result = my_tran_date_open_trans_line(parm);
	if (result < 0)
	{
		printf("open_port_transport error DF_MQTT_REQUEST_SHARE_USB_PORT,result = %d!\n",result);
		return result;
		
	}
	return result;
}

//��USB�˿�
static void analysis_request_openport(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;
	//struct mqtt_parm_newshare parm;
	int port = 0;
	char port_str[10] = { 0 };
	char trans_server[100] = { 0 };
	char client_id[200] = { 0 };
	int result;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};
	cJSON *item;
	cJSON *root = cJSON_Parse(message_line->message);
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	struct _ter_info ter_info;

	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random,errinfo);
		goto End;
	}

	int size;
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		module_out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		module_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "tran_port");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���tran_port����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(port_str, 0, sizeof(port_str));
			memcpy(port_str, item->valuestring, strlen(item->valuestring));
		}
	}
	item = cJSON_GetObjectItem(arrayItem, "tran_server");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���tran_server����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(trans_server, 0, sizeof(trans_server));
			memcpy(trans_server, item->valuestring, strlen(item->valuestring));
		}
	}
	item = cJSON_GetObjectItem(arrayItem, "clientid");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���clientid����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (item->valuestring != NULL)
		{
			memset(client_id, 0, sizeof(client_id));
			memcpy(client_id, item->valuestring, strlen(item->valuestring));
		}
	}
	struct _new_usbshare_parm parm;
	memset(&parm, 0, sizeof(struct _new_usbshare_parm));
	parm.data_lock = stream->data_lock;
	memcpy(parm.trans_server, trans_server, strlen(trans_server));
	memcpy(parm.client_id, client_id, strlen(client_id));

	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port_class->module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	memcpy(parm.mechine_id, ter_info.ter_id, strlen(ter_info.ter_id));

	port = atoi(port_str);
	parm.port = atoi(port_str);

	memcpy(parm.source_topic, message_line->parm.source_topic, strlen(message_line->parm.source_topic));
	memcpy(parm.random, message_line->parm.random, strlen(message_line->parm.random));
	/*
	message_line->parm.data_lock = stream->data_lock;
	message_line->parm.port = atoi(port_str);
	memcpy(message_line->parm.trans_server, trans_server, strlen(trans_server));
	memcpy(message_line->parm.client_id, client_id, strlen(client_id));
	*/
	result = jude_port_allow_attach();
	printf("result1111 = %d\n",result);
	if (result < 0)
	{
		result = MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED;
	}
	else
	{
		// result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
		// printf("result fun_check_with_robot_arm = %d��parm.port=%d\n",result,parm.port);
		// if(result)
		// {
		// 	if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state((parm.port > 0?(parm.port - 1):0)))  //�ж��Ƿ���������
		// 	{
		// 		result = MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING;
		// 	}
		// 	else
		// 	{
		// 		result = get_transport_line(&parm);
		// 	}
		// }
		// else
		
		{ 
			result = get_transport_line(&parm);
		}
	}
	// result = MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED;
	printf("result2222 = %d\n",result);
	//result = get_transport_line(&parm);
	if (result < 0 || result == MQTT_GW_USBSHARE_PORT_WITHOUT_AOLLOWED || result == MQTT_GW_USBSHARE_PORT_ROBOT_ARM_UODATING)
	{
		sprintf(errinfo, "�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		mqtt_response_errno_gw(&parm, result);
		cJSON_Delete(root);
		goto End;
	}
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,�ɹ�,�˿ںţ�%d,client ID��%s\r\n", message_line->parm.random, port, parm.client_id);
	mqtt_open_share_response(&parm, result);
	cJSON_Delete(root);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
}

//�����������ѯ�ն���־�ļ�
static void analysis_request_cxzddqrzxx(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};
	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	char *log_data;
	log_data = (char *)malloc(2*1024*1024);
	memset(log_data, 0, 2 * 1024 * 1024);

	int log_len;
	log_len = read_file("/etc/terminal.log", log_data, 2 * 1024 * 1024);

	char *log_zlib;
	int log_zlib_len = 2 * 1024 * 1024;
	log_zlib = (char *)malloc(log_zlib_len);
	memset(log_zlib, 0, log_zlib_len);


	asc_compress_base64((const unsigned char *)log_data, log_len, (unsigned char *)log_zlib, &log_zlib_len);

	module_out("ѹ��ǰ���ݳ���%d��ѹ�������ݳ���%d\n", log_len, strlen(log_zlib));
	free(log_data);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "log_data", log_zlib);
	free(log_zlib);
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

#ifdef MTK_OPENWRT
static void ssh_tunnel_start(void *arg)
{
	int result, i;

	for (i = 0; i < 20; i++)//����ű���mtk�Ͽ�����Ҫ���ж�Σ�ԭ��δ֪
	{
		system("/etc/ssh_tunnel.exp");
		result = detect_process("\"ssh -CNfg -R\"", "tmpssh.txt");
		if (result > 0)
		{
			break;
		}
	}
	return;
}
#endif

//��������������άͨ��
static void analysis_request_kqzdywtd(void *arg)
{
	MODULE     *stream;
	int size;
	int i;
	int result = -1;
	char server_addr[200] = { 0 };
	char server_port[10] = { 0 };
	char server_user[20] = { 0 };
	char server_passwd[50] = { 0 };
	char ssh_port[10] = {0};
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = { 0 };

	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	
	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());

	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "server_addr");
		if (get_json_value_can_not_null(item, (char *)server_addr, 0, 200) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_addr����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		module_out("��ȡ���ķ�������ַΪ%s\n", server_addr);

		item = cJSON_GetObjectItem(arrayItem, "server_port");
		if (get_json_value_can_not_null(item, (char *)server_port, 0, 6) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_port����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		module_out("��ȡ���ķ������˿�Ϊ%s\n", server_port);

		item = cJSON_GetObjectItem(arrayItem, "server_user");
		if (get_json_value_can_not_null(item, (char *)server_user, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_user����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		module_out("��ȡ���ķ������û���Ϊ%s\n", server_user);

		item = cJSON_GetObjectItem(arrayItem, "server_passwd");
		if (get_json_value_can_not_null(item, (char *)server_passwd, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_passwd����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		module_out("��ȡ���ķ������û�����Ϊ%s\n", server_passwd);

		item = cJSON_GetObjectItem(arrayItem, "ssh_port");
		if (get_json_value_can_not_null(item, (char *)ssh_port, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���ssh_port����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
			goto End;
		}
		module_out("��ȡ���ķ������˿�Ϊ%s\n", ssh_port);

	
		module_out("�жϽ����Ƿ��Ѿ����������Ѿ��������Ƚ���\n");
		result = detect_process("\"ssh -CNfg -R\"", "tmpssh.txt");
		if (result > 0)
		{
			module_out("%s���̴���,�����\n", "\"ssh -CNfg -R\"");
			check_and_close_process("\"ssh -CNfg -R\"");
		}

		result = detect_process("/usr/local/sbin/sshd", "tmpssh.txt");
		if (result > 0)
		{
			module_out("%s���̴���,�����\n", "/usr/local/sbin/sshd");
			check_and_close_process("/usr/local/sbin/sshd");
		}

		char sh_txt[2048] = { 0 };
		if (access("/lib/tcl8.6", 0) != 0)//Ŀ¼��û��tcl8.6Ŀ¼��������ļ�������tar.gz�İ� ���н�ѹ
		{
			if (access("/lib/tcl.tar.gz", 0) == 0)
			{
				system("chmod 777 /lib/tcl.tar.gz");
				system("tar -xf /lib/tcl.tar.gz -C /lib");
			}

		}
		system("chmod 777 /bin/expect");
#ifdef MTK_OPENWRT
		system("chmod 777 /bin/stty");
		sprintf(sh_txt, "#! /bin/expect -f\nset timeout 3\nspawn ssh -CNfg -R %s:127.0.0.1:22 %s@%s -p %s\nexpect {\n\"yes/no\" { send \"yes\\r\"; exp_continue }\n\"password:\" {send \"%s\\r\";exp_continue }\neof\n}\nwait", \
			ssh_port, server_user, server_addr, server_port, server_passwd);

		printf("%s\n", sh_txt);
		write_file("/etc/ssh_tunnel.exp", sh_txt, strlen(sh_txt));
		//MTK��Ҫ�����߳���������
		system("chmod 777 /etc/ssh_tunnel.exp");
		_delay_task_add("ssh_tuunel", ssh_tunnel_start, NULL, 0);
#else
		sprintf(sh_txt,"#! /bin/sh\nexpect -c \"\nset timeout 10\nspawn /usr/local/bin/ssh -CNfg -R %s:127.0.0.1:22 %s@%s -p %s\nexpect {\n\t\t\\\"yes/no\\\" {send \\\"yes\\n\\\";exp_continue;}\n\t\t\\\"password:\\\" {send \\\"%s\\n\\\";sleep 3;exit;}\n\t}\nexpect eof\"",\
			ssh_port,server_user,server_addr,server_port,server_passwd);
		printf("%s\n",sh_txt);
		write_file("/etc/ssh_tunnel.sh", sh_txt, strlen(sh_txt));
		system("chmod 777 /etc/ssh_tunnel.sh");

		result = detect_process("/usr/local/sbin/sshd", "tmpsshs.txt");
		if (result < 0)
		{
			system("/usr/local/sbin/sshd &");
			sleep(3);
		}


		system("/etc/ssh_tunnel.sh &");
		//system("/etc/ssh_tunnel.sh &");
		//system("/etc/ssh_tunnel.sh &");
#endif



	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	module_out("��������������άͨ�����\n");
	return;
}

//�������������ά��ͨ��
static void analysis_request_jszdywtd(void *arg)
{
	MODULE     *stream;
	int result = -1;
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	char errinfo[2048] = {0};

	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	module_out("�жϽ����Ƿ��Ѿ����������Ѿ��������Ƚ���\n");
	result = detect_process("ssh -CNfg -R", "tmpssh.txt");
	if (result > 0)
	{
		module_out("%s���̴���,�����\n", "ssh -CNfg -R");
		check_and_close_process("ssh -CNfg -R");
	}


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());


	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);

	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

//�����������ѯ�ն���Ȩ�ļ�
static void analysis_request_cxzddqsqxx(void *arg)
{
	MODULE     *stream;
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	int result;
	char errinfo[1024];
	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	char *auth_data = NULL;

	result = *((int *)((get_usb_port_class())->key_auth_state.get_auth_cert_file((void *)(&auth_data))));

	if ((result < 0) || (auth_data == NULL))
	{
		sprintf(errinfo, "�ն���Ȩ��Ϣ�ļ���ȡʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,�ײ������Ϣ��%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}

	char *auth_base;
	int auth_base_len = 2 * 1024 * 1024;
	auth_base = (char *)malloc(auth_base_len);
	memset(auth_base, 0, auth_base_len);

	base64_enc((uint8 *)auth_base, (uint8 *)auth_data, strlen(auth_data));



	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "auth_json", auth_base);
	free(auth_base);
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

//�������������˿ڵ�Դ
static void analysis_request_adkhgldkdy(void *arg)
{
	MODULE     *stream;
	int size;
	int usb_port = 0,dis_port = 0;
	int power_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	cJSON *arrayItem, *item; //����ʹ��
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //��ȡusb_port�Ĳ���ָ��

	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	//������ݽ���
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	else
	{
		//module_out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}


			char usb_port_string[100] = {0};
			char power_state_string[100] = { 0 };
			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���usb_port����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			if (get_json_value_can_not_null(item, usb_port_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���usb_port����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			usb_port = atoi(usb_port_string);
			dis_port = usb_port;
			module_out("��ȡ����USB�˿ں�Ϊ%d\n", usb_port);
			if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //���ж��Ƿ��ǻ�е�۵Ļ���
				usb_port = usb_port * 2;

			item = cJSON_GetObjectItem(arrayItem, "power_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			if (get_json_value_can_not_null(item, power_state_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
				goto End;
			}
			power_state = atoi(power_state_string);
			module_out("��ȡ����USB�˿ںŵ�Դ״̬Ϊ%d\n", power_state);


		}
	}

	result = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
	printf("result fun_check_with_robot_arm = %d\n",result);
	if(result)
	{
		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(usb_port-1))  //�ж��Ƿ���������
		{
			result = DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_UPDATING;
		}
		else
		{
			if(0 == get_robot_arm_class()->fun_judge_robot_arm((usb_port-1)>=0?(usb_port-1):0))   //�ǻ�е��
			{
				result = DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_NO_POWERE;
				
			}
			// else
			// {
			// 	result = (get_usb_port_class())->power_action.fun_power(usb_port-1,(PORT_STATE_ENUM)power_state);
			// }
			
		}
	}
	else
	{
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[usb_port-1].port_info.arm_version))
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(usb_port-1))  //�ж��Ƿ���������
			{
				result = DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_UPDATING;
			}
			// else
			// {
			// 	result = (get_usb_port_class())->power_action.fun_power(usb_port-1,(PORT_STATE_ENUM)power_state);
			// }
		}
		// else
		// 	result = (get_usb_port_class())->power_action.fun_power(usb_port-1,(PORT_STATE_ENUM)power_state);
	}

	if (result < 0)
	{
		if(result == DF_TAX_ERR_CODE_USB_PORT_ROBOT_ARM_NO_POWERE)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,��%d��USB�˿�,�ײ������Ϣ���˶˿��ǻ�е��\r\n", message_line->parm.random, dis_port);
		}
		else
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,��%d��USB�˿�,ʧ��\r\n", message_line->parm.random, dis_port);
		}
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(result, &message_line->parm, errinfo);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˿ڵ�Դ,��%d��USB�˿�,���\r\n", message_line->parm.random, dis_port);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);

	if(result >= 0)
	{
		(get_usb_port_class())->power_action.fun_power(usb_port-1,(PORT_STATE_ENUM)power_state);
	}
	return;
}



//���������������ն�
static void analysis_request_glzddy(void *arg)
{
	MODULE     *stream;
	char errinfo[2048] = {0};
	struct _message_line *message_line;
	message_line = arg;
	stream = &module;
	//cJSON *arrayItem, *item; //����ʹ��


	module_out("message_line->message = %s\n", message_line->message);
	module_out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ն�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo);
		goto End;
	}
	free(message_line->message);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	//������ݽ���
	
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ն�,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}
