#ifndef _TRAN_DATA_DEFINE
#define _TRAN_DATA_DEFINE
/*======================================包含文件================================================*/
#include "_switch_dev.h"
#include "../core/_seria.h"
#include <libusb-1.0/libusb.h>
#ifdef MTK_OPENWRT
#include "../linux_sys/_linux_ip_mtk.h"
#else
#include "../linux_sys/_linux_ip.h"
#endif
#include "_socket_sev.h"
#include "_socket_client.h"
#include <arpa/inet.h>
#include	"_ty_usb_m.h"
#include <signal.h>
#ifdef NEW_USB_SHARE
#include "_deal_cmd_newshare.h"
#endif

/*=====================================立即数定义===============================================*/
/*++++++++++++++++++++++++++++++++++命令参数+++++++++++++++++++++++++++++++++++++*/
int bulk_lock;
int control_lock;
int interrupt_lock;

#define DF_CLOSE_LINE_TRAN_TASK				200
#define DF_LOCK_UNLOCK_ALL_LINE				(DF_CLOSE_LINE_TRAN_TASK+1)
#define	DF_CHECK_PORT_LINE_STATUS			(DF_LOCK_UNLOCK_ALL_LINE+1)

#ifdef NEW_USB_SHARE
#define DF_OPEN_TRANSPORT_LINE	DF_CHECK_PORT_LINE_STATUS+1
#define	DF_MQTT_ERRNO_CONNECT_SERV_ERR					300
#endif
/*---------------------------------STM32-----------------------------------------*/
#ifdef _tran_data_c    
/*+++++++++++++++++++++++++++++++命令头定义++++++++++++++++++++++++++++++++++++++*/



#define DF_GET_IP_STATE_TASK_TIMER		 1000					//获取IP线程时间
#define DF_MID_LINE_TIME_OUT				10					//中间件心跳超时时间
#define DF_TY_SEV_TRAN_PORT			 3240
#define DF_TRANSMIT_COM_EDIT                0X0110

#define DF_SELF_IP							0X7F000001

#define DF_DLL_TER_8009	0X01068009
#define DF_TER_DLL_8009	0X01060009

#define DF_MID_TER_8005	0X01068005
#define	DF_TER_MID_8005	0X01060005
#define DF_MID_TER_8003	0X01068003
#define DF_TER_MID_8003	0X01060003
#define DF_MID_TER_URB  0X00000001
#define DF_TER_MID_URB	0X00000003
/*-----------------------------上行命令--------------------------------------*/
#define DF_SEV_ORDER_MID_8005_REQUEST				0X8005
#define	DF_SEV_ORDER_MID_8003_REQUEST				0X8003
#define DF_SEV_ORDER_TER_NOTICE                     0X8006
#define DF_SEV_ORDER_MID_NOTICE                     0X8007
#define DF_SEV_ORDER_NOTICE_STATE					0X00000000		
/*-----------------------------下行命令--------------------------------------*/
#define DF_ANSWER_ORDER_TER_8005_RESPONSE			0X0005
#define DF_ANSWER_ORDER_TER_8003_RESPONSE			0X0003
#define DF_ANSWER_ORDER_TER_NOTICE                  0X0006
#define DF_ANSWER_ORDER_MID_NOTICE                  0X0007
#define DF_ANSWER_ORDER_NOTICE_STATE				0X00000000	
/*------------------------------应答错误命令---------------------------------*/
#define DF_TRANSMIT_ANSWER_ORDER_ERR                             0XFF
#endif
/*--------------------------------------内部数据定义------------------------------------------*/
#ifdef _tran_data_c
#ifdef DEBUG
#define out(s,arg...)       				//log_out("all.c",s,##arg) 
#else
#define out(s,arg...)						//log_out("all.c",s,##arg) 
#endif	
#define e_tran_data
#else
#define e_tran_data                    extern
#endif
#ifdef _tran_data_c


#endif    

/*======================================结构体定义============================================*/

int all_num;
#ifdef _tran_data_c

#pragma pack(push,1)
#ifdef __GNUC__
#define __attribute__(x) __attribute__(x)
#else
#define __attribute__(x) /* x */
#endif
#define USBIP_VERSION 0x000106
#define OUT_Q_LEN 25
#define BIG_SIZE 1000000
#define USBIP_BUS_ID_SIZE 32
#define USBIP_DEV_PATH_MAX 256

#define URB_SHORT_NOT_OK			0x1
#define URB_ISO_ASAP				0x2
#define URB_NO_TRANSFER_DMA_MAP		0x4
#define URB_NO_FSBR					0x20
#define URB_ZERO_PACKET				0x40
#define URB_NO_INTERRUPT			0x80
#define URB_FREE_BUFFER				0x100
#define URB_DIR_MASK				0x200
struct _usbip_file
{
	char busid[DF_TY_USB_BUSID];
	int status;
	int socket;
	struct _usb_file_dev    dev;
};

struct usbip_header_cmd_submit
{
	/* these values are basically the same as in a URB. */
	/* the same in a URB. */
	unsigned int transfer_flags;
	/* set the following data size (out),
	* or expected reading data size (in) */
	int transfer_buffer_length;
	/* it is difficult for usbip to sync frames (reserved only?) */
	int start_frame;
	/* the number of iso descriptors that follows this header */
	int number_of_packets;
	/* the maximum time within which this request works in a host
	* controller of a server side */
	int interval;
	/* set setup packet data for a CTRL request */
	unsigned char setup[8];
}__attribute__((packed));



/*
* An additional header for a RET_SUBMIT packet.
*/
struct usbip_header_ret_submit
{
	int status;
	int actual_length; /* returned data length */
	int start_frame; /* ISO and INT */
	int number_of_packets;  /* ISO only */
	int error_count; /* ISO only */
}__attribute__((packed));

/*
* An additional header for a CMD_UNLINK packet.
*/
struct usbip_header_cmd_unlink
{
	unsigned int seqnum; /* URB's seqnum which will be unlinked */
}__attribute__((packed));


/*
* An additional header for a RET_UNLINK packet.
*/
struct usbip_header_ret_unlink
{
	int status;
}__attribute__((packed));


/* the same as usb_iso_packet_descriptor but packed for pdu */
struct usbip_iso_packet_descriptor
{
	unsigned int offset;
	unsigned int length;            /* expected length */
	unsigned int actual_length;
	unsigned int status;
}__attribute__((packed));


struct usbip_header_basic
{
#define USBIP_CMD_SUBMIT	0x0001
#define USBIP_CMD_UNLINK	0x0002
#define USBIP_RET_SUBMIT	0x0003
#define USBIP_RET_UNLINK	0x0004
#define USBIP_RESET_DEV		0xFFFF
	unsigned int command;
	/* sequencial number which identifies requests.
	* incremented per connections */
	unsigned int seqnum;
	/* devid is used to specify a remote USB device uniquely instead
	* of busnum and devnum in Linux. In the case of Linux stub_driver,
	* this value is ((busnum << 16) | devnum) */
	unsigned int devid;

#define USBIP_DIR_OUT	0
#define USBIP_DIR_IN 	1
	unsigned int direction;
	unsigned int ep;     /* endpoint number */
} __attribute__((packed));

struct usbip_header
{
	struct usbip_header_basic base;
	union
	{
		struct usbip_header_cmd_submit	cmd_submit;
		struct usbip_header_ret_submit	ret_submit;
		struct usbip_header_cmd_unlink	cmd_unlink;
		struct usbip_header_ret_unlink	ret_unlink;
	} u;
}__attribute__((packed));


struct op_devlist_reply
{
	unsigned int ndev;
	/* followed by reply_extra[] */
} __attribute__((packed));

struct usb_device
{
	char path[USBIP_DEV_PATH_MAX];
	char busid[USBIP_BUS_ID_SIZE];
	uint32 busnum;
	uint32 devnum;
	uint32 speed;

	uint16 idVendor;
	uint16 idProduct;
	uint16 bcdDevice;

	uint8 bDeviceClass;
	uint8 bDeviceSubClass;
	uint8 bDeviceProtocol;
	uint8 bConfigurationValue;
	uint8 bNumConfigurations;
	uint8 bNumInterfaces;
} __attribute__((packed));

struct usb_interface
{
	uint8 bInterfaceClass;
	uint8 bInterfaceSubClass;
	uint8 bInterfaceProtocol;
	uint8 padding;	/* alignment */
} __attribute__((packed));


#ifdef NEW_USB_SHARE

struct _err_no_infos
{
	int errnum;
	char errinfo[200];
};
static const struct _err_no_infos err_no_infos[] =
{
	{ DF_MQTT_ERRNO_CONNECT_SERV_ERR, "CONNECT SERVER ERROR!" },

};
struct line_transport
{
	int fd;
	struct _new_usbshare_parm parm;
	struct _tran_data        *stream;
	int state;
};
#endif


struct _point_dec
{
	int state; //标识状态是否可用0 NO 1 YES
	int bEndpointAddress;//端点号，传输方向
	int bmAttributes;//端点特性
	int wMaxPacketSize;//最大数据包长度
	int bInterval;//访问间隔
};
struct _interface_dec//设备描述符结构体
{
	int state;//标识状态是否可用0 NO 1YES
	int bInterfaceNumber;//接口号
	int bAlterateSetting;//可替换设置值
	int bNumEndpoints;//端点0以外端点数
	int bInterfaceClass;//类代码
	int bInterfaceSubClass;//子类代码
	int bInterfaceProtocol;//协议代码
	int iInterface;//字符串描述符的索引值

	struct _point_dec point_decs[3];

};
struct _interrupt_info{
	libusb_device_handle *devh;
	int port_num;
	struct _tran_data	*stream;
	uint16 vid;
	uint16 pid;
	int ep;
	int sock_lock;
	int data_lock;
	int sock;
	int state;
	int Wlength;
	int* out_close;
	unsigned char head_data[50];
	unsigned char *inter_in_data;//此处后面要改成动态内存，不然可能存在问题10000
	int * thread_num;
	int *unlink_flag;//是否存在unlink情况，如果存在则传输timeout的时候break
};

struct _bulk_info{
	libusb_device_handle **devh;
	int port_num;
	struct _tran_data	*stream;
	uint16 vid;
	uint16 pid;
	int ep;
	int sock_lock;
	int data_lock;
	int sock;
	int state;
	int Wlength;
	int* out_close;
	unsigned char head_data[50];
	unsigned char *bulk_in_data;//此处后面要改成动态内存，不然可能存在问题10000
	int * thread_num;
	int *unlink_flag;//是否存在unlink情况，如果存在则传输timeout的时候break
};

struct _control_info{
	libusb_device_handle **devh;
	int sock_lock;
	int data_lock;
	int sock;
	int state;
	unsigned char head_data[50];
	unsigned char *ctl_in_data;
	int ctl_in_data_len;
	uint8 bRequest;
	uint8 request_type;
	uint16 wValue;
	uint16 wIndex;
	uint16 Wlength;
	int * out_close;
	int * thread_num;
	int port_num;
	struct _tran_data	*stream;
	uint16 vid;
	uint16 pid;
	int *unlink_flag;//是否存在unlink情况，如果存在则传输timeout的时候break
};

struct _connect_state
{

	int state_mem;			//此片内存使用状态
	int main_task;			//转发处理主线程
	int ter_task;			//终端数据处理线程句柄
	int mid_task;			//中间件数据处理线程句柄
	int heart_task;			//与中间件的心跳
	int ter_fd;				//链路句柄
	int mid_fd;				//链路句柄
	unsigned char busid[32];
	unsigned long ter_time;	//终端链路最后通讯时间
	unsigned long mid_time;	//中间件链路最后通讯时间
	int ter_sig;
	int mid_sig;
	int lock;
	int out_close;
	sem_t			*sem;
	libusb_context *context;
	libusb_device_handle *devh;
	int tran_type;				//0控制传输	 1同步传输  2块传输 3中断传输（仅支持0、2、3）
	int out_ep;					//输出端点
	int in_ep;					//输入端点
	int out_max_len;			//输出端点最大数据包长度
	int in_max_len;				//输入端点最大数据包长度

	int interrupt_task;
	int bInterfaceNumber;		//接口号
	int bNumEndpoints;			//端点0以外的端点数
	long out_q_seqnum_array[OUT_Q_LEN];
	long seqnum;
	int socket_fd;
	int queue_num;
	int bulk_wait;
	int AllInterfaceNumber;//总共的接口数
	int interrupt_in_ep;
	struct _interface_dec interface_decs[2];
	int current_pack_len;
	int interrupt_len;
	uint16 vid;
	uint16 pid;
	int  interrupt_in_maxlen;
	struct _bulk_info  bulk_info[DF_TY_USB_PORT_N];
	struct _bulk_info  bulk_info_out[DF_TY_USB_PORT_N];
	struct _control_info  control_info[DF_TY_USB_PORT_N];
	struct _control_info  control_info_out[DF_TY_USB_PORT_N];
	struct _interrupt_info  interrupt_info_out[DF_TY_USB_PORT_N];
	struct _interrupt_info  interrupt_info_in[DF_TY_USB_PORT_N * 2];
	int unlink_flag;//是否存在unlink情况，如果存在则传输timeout的时候break
	int port_num;
	int unlink_num;
	struct _tran_data	*stream;
	int control_in_thread_num;
	int control_out_thread_num;
	int bulk_in_thread_num;
	int bulk_out_thread_num;
	int interrupt_out_thread_num;
	int interrupt_in_thread_num;
	int test_num;
};

struct _interrupt_data
{
	struct _tran_data  *dev;
	int socket_fd;
	int port_num;
	char busid[32];
};

struct _tran_data_fd
{
	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _tran_data  *dev;
};

struct _ty_usbip_s
{
	int state;
	int port;
	char busid[DF_TY_USB_BUSID];
};

struct _tran_data
{
	struct _tran_data_fd	fd[DF_FILE_OPEN_NUMBER];
	int state;
	int lock;
	int task;
	int sock_fd;
	int ty_usb_fd;
	int usb_port_numb;
	int tran_port;
	int switch_dev;
	int ty_usb_m;
	_so_note    *note;
	struct _ty_usbip_s data[DF_TY_USB_PORT_N];
	//int net_state;
	struct _connect_state	line[DF_TCP_SER_CONNECT_LINE_NUM];
#ifdef NEW_USB_SHARE
	struct line_transport transport_line[DF_TCP_SER_CONNECT_LINE_NUM];
#endif
	//struct _ter_connect_state  ter_line[DF_TCP_SER_CONNECT_LINE_NUM];
	//struct _mid_connect_state  mid_line[DF_TCP_SER_CONNECT_LINE_NUM];
};
struct _tran_data_ctl_fu
{
	int cm;
	int(*ctl)(struct _tran_data_fd   *id, va_list args);
};

// struct _transmit_order
//{   int order;
//    char *name;  
//    int (*answer)(int fd,uint32 ip,uint16 port,struct _tran_data    *stream,uint8 *buf,int buf_len);
//};

struct _tramsmit_task_data
{
	int line_num;
	struct _tran_data *tran_data;
};

struct _transmit_sign
{
	int socket_fd;
	sem_t			*sem;						//使用触发线程
};

struct _train_data_oper
{
	uint8 buf[1024];
	int buf_len;
	int len;
	uint8 *re_buf;
};

#define DF_TY_USBIP_VERSION         0x000106
#define OP_REQUEST	(0x80 << 8)
#define OP_REPLY	(0x00 << 8)   
#define ST_OK	0x00
#define ST_NA	0x01        

#define OP_UNSPEC	    0x00
#define OP_REQ_UNSPEC	OP_UNSPEC
#define OP_REP_UNSPEC	OP_UNSPEC   
/* ---------------------------------------------------------------------- */
/* Retrieve USB device information. (still not used) */
#define OP_DEVINFO	0x02
#define OP_REQ_DEVINFO	(OP_REQUEST | OP_DEVINFO)
#define OP_REP_DEVINFO	(OP_REPLY   | OP_DEVINFO)  

/* Import a remote USB device. */
#define OP_IMPORT	0x03
#define OP_REQ_IMPORT	(OP_REQUEST | OP_IMPORT)
#define OP_REP_IMPORT   (OP_REPLY   | OP_IMPORT)   

#define OP_DEVLIST	0x05
#define OP_REQ_DEVLIST	(OP_REQUEST | OP_DEVLIST)
#define OP_REP_DEVLIST	(OP_REPLY   | OP_DEVLIST)
struct op_common
{
	uint16 version;
	uint16 code;
	uint32 status;
} __attribute__((packed));

struct _usbip_speed_n
{
	int num;
	char *speed;
};
enum usb_speed
{
	USB_SPEED_UNKNOWN_A = 0,                  /* enumerating */
	USB_SPEED_LOW_A, USB_SPEED_FULL_A,          /* usb 1.1 */
	USB_SPEED_HIGH_A,                         /* usb 2.0 */
	USB_SPEED_VARIABLE_A                      /* wireless (usb 2.5) */
};
static const struct _usbip_speed_n _usbip_speed[] =
{ { USB_SPEED_UNKNOWN_A, "unknown" },
{ USB_SPEED_LOW_A, "1.5" },
{ USB_SPEED_FULL_A, "12" },
{ USB_SPEED_HIGH_A, "480" }
};

#define PACK_OP_COMMON(pack, op_common)  do {\
	pack_uint16(pack, &(op_common)->version); \
	pack_uint16(pack, &(op_common)->code); \
	pack_uint32(pack, &(op_common)->status); \
} while (0)

#endif
/*========================================函数定义=================================================*/
e_tran_data    int tran_data_add(int port, const _so_note    *note);
#ifdef _tran_data_c
static int tran_data_open(struct ty_file	*file);
static int tran_data_ctl(void *data, int fd, int cm, va_list args);
static int tran_data_close(void *data, int fd);

static void line_task(void *arg, int o_timer);
static int get_inlen_par(uint8 *buf, long *out_q_seqnum_array);
static int get_head_size(uint8 *buf);
static int oper_data(int src_line, int dest_line, struct _train_data_oper	*data, long *out_q_seqnum_array, struct _tran_data	*stream, int port_num);

static int tran_data_lock_all_line(struct _tran_data_fd *id, va_list args);
static int tran_data_check_line_status(struct _tran_data_fd *id, va_list args);





static int _transmit_order_hook(int fd, uint32 ip, uint16 port, void *arg);

static uint32 recv_midware_ver_code(int fd);


static int deal_8003_0003(struct _tran_data    *stream, int fd, uint8 *busid);
static int deal_8005_0005(struct _tran_data    *stream, int fd);
static int deal_8009_0009(struct _tran_data *stream, int fd, int *port, uint32 ip);
static int ty_socket_read(int sock, uint8 *buf, int buf_len);
static int ty_socket_write(int sock, uint8 *buf, int buf_len);


static int get_usb_interface(const struct _usb_file_interface   *usb_file_interface, struct usb_interface    *interface);
static void pack_usb_device(int pack, struct usb_device *udev);
static int usbip_send_op_common(int sockfd, uint32 code, uint32 status);
static int get_usbip_dev(const char *busid, struct usb_device *device);
static void pack_uint16(int pack, uint16 *val);
static void pack_uint32(int pack, uint32 *val);
static int get_usb_dev_pdu(const struct _usb_file_dev *usb_dev, const char *busid, struct usb_device    *device);

//static int check_out(unsigned long num, long *out_q_seqnum_array);
//static int get_out(unsigned long *num, long *out_q_seqnum_array);
void quickSort(int* arr, int startPos, int endPos);
static int tran_data_close_tran_task(struct _tran_data_fd *id, va_list args);

/*==================================内核外移植相关函数=================================*/

static libusb_device *FindDeviceByID(libusb_device **devs, int nBusNum, int nDevNum);
static int usb_dev_init(struct _tran_data	*stream, int port_num, uint8 *busid);

static int get_ep_direction(char *buf, int *ep, int *direction);
static int control_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd);
static int interrupt_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd);
static int bulk_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd);
//static void interrupt_data_to_dev(void *arg);

static int reload_devh(struct _tran_data	*stream, int port_num);
int usb_dev_init_new(struct _tran_data	*stream, int port_num, uint8 *busid);

#ifdef NEW_USB_SHARE
static uint32 recv_data_len(int fd);
static int trans_usb_share(struct _tran_data    *stream, int port, int fd);
static int tran_date_open_trans_line(struct _tran_data_fd *id, va_list args);
static void deal_trans_line_oper(void *arg);
static int prepare_connect_packet(char *sbuf, int *len, struct _new_usbshare_parm  *parm);
static int wait_for_8007_busid(int fd, struct _tran_data        *tran_stream);

#endif
#endif    
/*=======================================常数定义==================================================*/
#ifdef _tran_data_c
static const struct _tran_data_ctl_fu ctl_fun[] =
{ { DF_CLOSE_LINE_TRAN_TASK, tran_data_close_tran_task },
{ DF_LOCK_UNLOCK_ALL_LINE, tran_data_lock_all_line },
{ DF_CHECK_PORT_LINE_STATUS, tran_data_check_line_status },
#ifdef NEW_USB_SHARE
{ DF_OPEN_TRANSPORT_LINE, tran_date_open_trans_line },//公网加载
#endif
};

static const struct _file_fuc	tran_data_fuc =
{ .open = tran_data_open,
.read = NULL,
.write = NULL,
.ctl = tran_data_ctl,
.close = tran_data_close,
.del = NULL
};

#endif


#endif
