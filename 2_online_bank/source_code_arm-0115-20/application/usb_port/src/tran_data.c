#include "../inc/tran_data.h"

int bulk_lock;
int control_lock;
int interrupt_lock;
int all_num;

struct _tran_data *my_tran_data = NULL;

int re_attach_data[DF_TY_USB_PORT_N_MAX] = {0x00};
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
static int deal_8009_0009(struct _tran_data *stream, int fd, int *port, uint32 ip,int reattach_flag);
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
static int interrupt_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd,int response_flag,int *end_flag);
static int bulk_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd,int response_flag,int *end_flag);
//static void interrupt_data_to_dev(void *arg);

static int reload_devh(struct _tran_data	*stream, int port_num);
int usb_dev_init_new(struct _tran_data	*stream, int port_num, uint8 *busid);

static uint32 recv_data_len(int fd);
static int trans_usb_share(struct _tran_data    *stream, int port, int fd);
static int tran_date_open_trans_line(struct _tran_data_fd *id, va_list args);
static void deal_trans_line_oper(void *arg);
static int prepare_connect_packet(char *sbuf, int *len, struct _new_usbshare_parm  *parm);
static int wait_for_8007_busid(int fd, struct _tran_data        *tran_stream);

static const struct _usbip_speed_n _usbip_speed[] =
{ { USB_SPEED_UNKNOWN_A, "unknown" },
{ USB_SPEED_LOW_A, "1.5" },
{ USB_SPEED_FULL_A, "12" },
{ USB_SPEED_HIGH_A, "480" }
};

static const struct _tran_data_ctl_fu ctl_fun[] =
{ { DF_CLOSE_LINE_TRAN_TASK, tran_data_close_tran_task },
{ DF_LOCK_UNLOCK_ALL_LINE, tran_data_lock_all_line },
{ DF_CHECK_PORT_LINE_STATUS, tran_data_check_line_status },
{ DF_OPEN_TRANSPORT_LINE, tran_date_open_trans_line },//公网加载
};

static const struct _file_fuc	tran_data_fuc =
{ .open = tran_data_open,
.read = NULL,
.write = NULL,
.ctl = tran_data_ctl,
.close = tran_data_close,
.del = NULL
};

int tran_data_add(int port, const _so_note    *note)
{
	struct _tran_data *stream;
	int result;
	stream = malloc(sizeof(struct _tran_data));
	if (stream == NULL)
		return DF_ERR_MEM_ERR;
	my_tran_data = stream;
	memset(stream, 0, sizeof(struct _tran_data));
	stream->tran_port = port;
	stream->note = (_so_note    *)note;
	result = ty_file_add(DF_DEV_TRAN_DATA, stream, "tran_data", (struct _file_fuc *)&tran_data_fuc);
	if (result<0)
		free(stream);
	return result;
}


static int tran_data_open(struct ty_file	*file)
{
	struct _tran_data *stream;
	char name[DF_FILE_NAME_MAX_LEN];
	int j;
	int i;
	int result;
	stream = file->pro_data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if (stream->state == 0)
	{  
		if (_socket_sev_add(stream->tran_port)<0)
		{
			tran_data_out("[_socket_sev_add]申请失败\n");
			return -1;
		}
		memset(name, 0, sizeof(name));
		sprintf(name, "/dev/socket_sev/sever/%d", stream->tran_port);
		stream->sock_fd = ty_open(name, 0, 0, NULL);
		if (stream->sock_fd<0)
		{
			tran_data_out("打开文件:/dev/socket_sev/sever/3240 失败\n");
			return stream->sock_fd;
		}
		if (libusb_init(NULL) < 0)
		{
			return -2;
		}
		ty_ctl(stream->sock_fd, DF_SOCKET_SEV_CM_HOOK, stream, _transmit_order_hook);
		ty_ctl(stream->sock_fd, DF_SOCKET_SEV_CM_OPEN_KEEP, 1);
		ty_ctl(stream->sock_fd, DF_SOCKET_SEV_CM_NODELAY, 1);
		ty_ctl(stream->sock_fd, DF_SOCKET_SEV_CM_CLOSE_TYPE, 0);
		ty_ctl(stream->sock_fd, DF_SOCKET_SEV_CM_SEND_TIMER, 5);
		stream->ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
		if (stream->ty_usb_fd<0)
		{
			tran_data_out("打开文件[%s]失败:[%d]\n", "/dev/ty_usb", stream->ty_usb_fd);
			return stream->ty_usb_fd;
		}
		stream->usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		if ((stream->usb_port_numb < 0) || (stream->usb_port_numb > DF_TY_USB_PORT_N_MAX))
		{
			tran_data_out("端口错误\n");
			return stream->ty_usb_fd;
		}
		for (i = 0; i < stream->usb_port_numb; i++)
		{
			//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, i + 1, stream->data[i].busid);
			result = my_get_busid_only(i + 1,stream->data[i].busid,stream->note);
			if (result<0)
			{
				tran_data_out("[open_port]获取端口[%d]busid失败\n", i);
				return result;
			}
			stream->data[i].port = i + 1;
			stream->data[i].state = 1;
		}

		init_parm(stream->note);
		//tran_data_out("======socket tran server file open success======\n");		
		stream->lock = _lock_open("tran_data", NULL);
		if (stream->lock<0)
		{
			tran_data_out("申请锁失败\n");
			return stream->lock;
		}
		bulk_lock = _lock_open("bulk_lock", NULL);
		if (bulk_lock<0)
		{
			tran_data_out("申请锁失败\n");
			return bulk_lock;
		}
		control_lock = _lock_open("control_lock", NULL);
		if (control_lock<0)
		{
			tran_data_out("申请锁失败\n");
			return control_lock;
		}
		interrupt_lock = _lock_open("interrupt_lock", NULL);
		if (interrupt_lock<0)
		{
			tran_data_out("申请锁失败\n");
			return interrupt_lock;
		}
		for (i = 0; i<sizeof(stream->line) / sizeof(stream->line[0]); i++)
			stream->line[i].lock = _lock_open("s", NULL);
		//tran_data_out("==============================data transmit server start success,going to monitor mode=================================\n");
		//stream->task = _scheduled_task_open("tran_data",_deal_line_heart_task,stream,1000,NULL);
	}
	for (j = 0; j<sizeof(stream->fd) / sizeof(stream->fd[0]); j++)
	{
		if (stream->fd[j].state == 0)
			break;
	}
	if (j == sizeof(stream->fd) / sizeof(stream->fd[0]))
	{
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j], 0, sizeof(stream->fd[0]));
	stream->fd[j].dev = stream;
	stream->fd[j].state = 1;
	stream->fd[j].mem_fd = &file->fd[j];
	stream->state++;
	tran_data_out("打开tran_data成功:j=%d\n", j);
	return j + 1;
}

//获取USb vid pid
int get_usb_vid_pid(int port,uint16 *vid,uint16 *pid)
{   
	int effect_port = 0;
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针
	if (vid == NULL || pid == NULL || usb_port_class == NULL)
	{
		tran_data_out("USB端口参数错误\n");\
        return -1;
	}
	if(usb_port_class->machine_type == TYPE_2303)
	{
		effect_port = port/2 - 1;
	}
	else
	{
		effect_port = port - 1;
	}
	printf("get_usb_vid_pid port = %d,effect_port = %d\n",port,effect_port);
	if (port != 0 && usb_port_class->judge_usbport_exist.port_exist_state[port-1].now_port_state != WITHOUT_DEV)
    {   //memset(&dev,0,sizeof(dev));
		*vid = usb_port_class->get_dev_basic_action.usb_dev_basic_data[effect_port].port_info.vid;
		*pid = usb_port_class->get_dev_basic_action.usb_dev_basic_data[effect_port].port_info.pid;		
		return 0;
    }
    return -1;
}

static int tran_data_ctl(void *data, int fd, int cm, va_list args)
{
	struct _tran_data        *stream;
	struct _tran_data_fd     *id;
	int i;
	stream = data;
	if ((stream == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if ((fd == 0) || (fd>sizeof(stream->fd) / sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &stream->fd[fd];
	if (id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	//tran_data_out("ty_usb_m_ctl:cm=%d\n",cm);
	for (i = 0; i<sizeof(ctl_fun) / sizeof(ctl_fun[0]); i++)
	{
		if (cm == ctl_fun[i].cm)
			return ctl_fun[i].ctl(id, args);
	}
	return DF_ERR_PORT_CORE_CM;
}
static int tran_data_close(void *data, int fd)
{
	libusb_exit(NULL);
	return 0;
}
long open_num = 0;
long close_num = 0;



#ifdef NEW_USB_SHARE

/**********准备转发链路联机包***********/
static int prepare_connect_packet(char *sbuf, int *len, struct _new_usbshare_parm  *parm)
{
	char sendbuf[100] = { 0 };
	//char port[10] = { 0 };
	//int result;
	long u_sec = 0;
	u_sec = get_time_sec();
	memcpy(sendbuf, "\x01\x06", 2);//版本信息

	memset(parm->client_id, 0, sizeof(parm->client_id));
	sprintf(parm->client_id, "%s%ld%d", parm->mechine_id, u_sec,rand() % 100 + 1);

	*len += 2;
	sendbuf[2] = 0x00;
	sendbuf[3] = (strlen(parm->client_id) + 6) & 0xff;//2位长度 0x5e
	*len += 2;

	memcpy(sendbuf + *len, "\x00\x01", 2);//命令字-00 01 代表机柜链接
	*len += 2;

	memcpy(sendbuf + *len, parm->client_id, strlen(parm->client_id));
	*len += strlen(parm->client_id);
	printf("################key is:%s---len is:%d------------################\n", parm->client_id, *len);
	memcpy(sbuf, sendbuf, *len);
	return 0;

}

/*********处理转发链路端口加载具体动作*************/
static int trans_usb_share(struct _tran_data    *stream, int port, int fd)
{
	int result, i;
	LOAD_DATA load_data = {0x00};
	//int deal_way=0;
	int port_num;
	//int result1,result2;
	uint32 ver_code;
	uint8 busid[32];
	memset(busid, 0, sizeof(busid));
	int client_fd;
	struct _transmit_sign	line1, line2;
	int task2 = 0;
	sem_t	sem;
	struct _train_data_oper	line1_data, line2_data;

	long out_q_seqnum_array[OUT_Q_LEN];

	memset(busid, 0, sizeof(busid));
	sem_init(&sem, 0, 0);

WAIT_8003:
	ver_code = recv_midware_ver_code(fd);
	if (ver_code == DF_MID_TER_8005)					//8005 获得USB设备列表信息（OP_REQ_DEVLIST）
	{
		//tran_data_out("处理8005 0005完成\n");
		result = deal_8005_0005(stream, fd);
		if (result != 0)
		{
			return -1;
		}
		goto WAIT_8003;
	}
	else if (ver_code == DF_MID_TER_8003)
	{
		client_fd = deal_8003_0003(stream, fd, busid);
		if (client_fd < 0)
		{
			return client_fd;
		}
		tran_data_out("8003处理完成\n");
	}
	else
		return 0;

	port_num = port;

	
	memset(&line1, 0, sizeof(line1));
	memset(&line2, 0, sizeof(line2));
	line1.sem = &sem;
	line1.socket_fd = client_fd;
	line2.sem = &sem;
	line2.socket_fd = fd;
	port_num--;
	// tran_data_out("获取传输类型和端点\n");
	result = usb_dev_init_new(stream, port_num + 1, busid);
	if (result < 0)
	{
		tran_data_out("USB设备初始化失败\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}
	// tran_data_out("trans_usb_share 传输类型为：%d，in_ep = %02x out_ep = %02x,in最大数据帧长度= %d,out最大数据帧长度= %d\n", stream->line[port_num].tran_type, stream->line[port_num].in_ep, stream->line[port_num].out_ep, stream->line[port_num].in_max_len, stream->line[port_num].out_max_len);
	memcpy(stream->line[port_num].busid, busid, strlen((char *)busid));

	get_usb_vid_pid(port_num + 1, &stream->line[port_num].vid, &stream->line[port_num].pid);
	tran_data_out("开启中间件数据检测线程\n");
	task2 = _scheduled_task_open("main_line", line_task, &line2, 1, NULL);
	if(task2 < 0)
	{
		tran_data_out("####################### scheduled_task_open 失败\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}
	_lock_set(stream->lock);
	printf("lbc 22222222222222222 port_num = %d\n",port_num);
	stream->line[port_num].state_mem = 1;
	stream->line[port_num].out_close = 0;
	stream->line[port_num].unlink_flag = 0;
	stream->line[port_num].sem = &sem;
	_lock_un(stream->lock);
	
	memset(&line1_data, 0, sizeof(line1_data));
	memset(out_q_seqnum_array, 0, sizeof(out_q_seqnum_array));
	memset(&line2_data, 0, sizeof(line2_data));
	for (;;)
	{
		sem_wait(&sem);
		if (stream->line[port_num].out_close != 0 && stream->line[port_num].out_close != 2)
		{
			tran_data_out("执行外部关闭命令,port_num = %d,out_close = %d\n",port_num,stream->line[port_num].out_close);
			break;
		}
		if(re_attach_data[port_num] == 1)
		{
			re_attach_data[port_num] = 0;
			printf("goto WAIT_8003 \n");
			stream->data[port_num].state = 1;
			if (stream->line[port_num].devh != NULL)
			{
				result = libusb_release_interface(stream->line[port_num].devh, 0);
				if (result < 0)
				{
					tran_data_out("libusb_release_interface_0 result = %d\n", result);
				}
			}

			for (i = 0; i < 3; i++)
				memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));
			if (stream->line[port_num].devh != NULL)
			{
				tran_data_out("trans_usb_share结束usbshare使用,关闭%d号端口句柄\n", port_num+1);
				libusb_close(stream->line[port_num].devh);
				stream->line[port_num].devh = NULL;
			}

			stream->line[port_num].unlink_num = 0;
			//stream->line[port_num].state_mem = 0;
			stream->line[port_num].out_close = 0;
			stream->line[port_num].unlink_flag = 0;
			stream->line[port_num].sem = NULL;
			stream->line[port_num].queue_num = 0;
			stream->line[port_num].test_num = 0;
			memset(&stream->line[port_num].out_q_seqnum_array, 0, sizeof(stream->line[port_num].out_q_seqnum_array));
			_scheduled_task_close(task2);
			goto WAIT_8003;
		}
		if ((result = oper_data(fd, client_fd, &line1_data, out_q_seqnum_array, stream, port_num + 1))<0)
		{
			tran_data_out("oper_data检测到中间件链路故障,result = %d\n", result);
			break;
		}
		//待完善
		//set_port_usb_data_tran(port_num + 1);
		_scheduled_task_week_up(task2);
	}
	tran_data_out("退出转发循环,执行关闭流程\n");
	if (stream->line[port_num].tran_type == 3)
	{
		tran_data_out("等待所有中断接收队列退出\n");
		for (i = 0; i<3000; i++, usleep(1000))
		{
			if (stream->line[port_num].queue_num == 0)
				break;
		}

	}
	if (stream->line[port_num].devh != NULL)
	{
		result = libusb_release_interface(stream->line[port_num].devh, 0);
		if (result < 0)
		{
			tran_data_out("libusb_release_interface_0 result = %d\n", result);
		}
		else
		{
			if (stream->line[port_num].interface_decs[1].state != 0)//表示为双接口设备
			{
				tran_data_out("libusb_release_interface_1\n");
				result = libusb_release_interface(stream->line[port_num].devh, 1);
				if (result < 0)
				{
					tran_data_out("libusb_release_interface_1 result = %d\n", result);
				}
			}
		}
		
	}

	for (i = 0; i < 3; i++)
		memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));
	stream->line[port_num].out_close = 1;

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_in_thread_num == 0)
		{
			tran_data_out("控制传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_out_thread_num == 0)
		{
			tran_data_out("控制传输out线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_in_thread_num == 0)
		{
			tran_data_out("块传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_out_thread_num == 0)
		{
			tran_data_out("块传输out线程全部退出完毕\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_out_thread_num == 0)
		{
			tran_data_out("中断传输out线程全部退出完毕\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_in_thread_num == 0)
		{
			tran_data_out("中断传输in线程全部退出完毕\n");
			break;
		}
	}
	//等待线程退出

	// if (stream->line[port_num].bulk_in_thread_num != 0 || \
	// 	stream->line[port_num].bulk_out_thread_num != 0 || \
	// 	stream->line[port_num].control_in_thread_num != 0 || \
	// 	stream->line[port_num].control_out_thread_num != 0 || \
	// 	stream->line[port_num].interrupt_out_thread_num != 0 || \
	// 	stream->line[port_num].interrupt_in_thread_num != 0)
	// {
	// 	write_log("重新加载devh时有线程未退出:bulk_in:%d\tbulk_out:%d\tctl_in:%d\tctrl_out:%d\t!\n", stream->line[port_num].bulk_in_thread_num, stream->line[port_num].bulk_out_thread_num, \
	// 		stream->line[port_num].control_in_thread_num, stream->line[port_num].control_out_thread_num);
	// }

	tran_data_out("trans_usb_share结束usbshare使用\n");
	if (stream->line[port_num].devh != NULL)
	{
		tran_data_out("trans_usb_share结束usbshare使用,关闭%d号端口句柄\n", port_num+1);
		libusb_close(stream->line[port_num].devh);
		stream->line[port_num].devh = NULL;
	}
	tran_data_out("结束libusb调用成功,%08x \n", stream->line[port_num].devh);
	_lock_set(stream->lock);
	stream->line[port_num].unlink_num = 0;
	stream->line[port_num].state_mem = 0;
	stream->line[port_num].out_close = 0;
	stream->line[port_num].unlink_flag = 0;
	stream->line[port_num].sem = NULL;
	stream->line[port_num].queue_num = 0;
	stream->line[port_num].test_num = 0;
	memset(&stream->line[port_num].out_q_seqnum_array, 0, sizeof(stream->line[port_num].out_q_seqnum_array));
	_lock_un(stream->lock);
	shutdown(fd, SHUT_RDWR);
	close(fd);

	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	_scheduled_task_close(task2);
	printf("\n\nexit trans_usb_share over !\n\n");

	load_data.port = port_num;
	get_usb_port_class()->load_action.fun_load_stop((void *)(&load_data));
	tran_data_out("trans_usb_share,执行关闭流程完成，端口号为【%d】\n\n\n\n", port_num + 1);
	return 0;

}
/********等待处理8007的获取busid请求*********/
static int wait_for_8007_busid(int fd, struct _tran_data        *tran_stream)
{
	uint32 ver_code;
	int result;
	uint32 len_data;
	int port = 0;
	uint8 recv_port[10] = { 0 };
	uint8 busid[20] = { 0 };
	uint8 send_tmp[1024] = { 0 };
	int with_robot_arm = -1;
	char tmp_busid[64] = {0x00};
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针
	with_robot_arm = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
	
	
	ver_code = recv_midware_ver_code(fd);
	if (ver_code != 0X01068007)
	{
		tran_data_out("not 8007\n");
		return -1;
	}
	len_data = recv_data_len(fd);
	if (len_data <= 0 || len_data > sizeof(recv_port))
	{
		//返回错误
		tran_data_out("recv_data_len err,len_data = %d\n",len_data);
		return -1;
	}
	//接收实体port
	if (ty_socket_read(fd, recv_port, len_data)<0)
	{
		tran_data_out("获取错误\n");
		return -1;
	}

	port = atoi((char*)recv_port);
	tran_data_out("recv port is :%d,len_data = %d,recv_port = %s\n", port,len_data,recv_port);
	if(port < 1 || port > usb_port_class->usb_port_numb)
	{
		//port = tran_stream->dev_port;
		tran_data_out("port错误,port = %d,tran_stream->dev_port = %d\n",port,tran_stream->dev_port);
		return -1;
	}
	if(*((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //先判断是否是机械臂的机柜
	{
		tran_data_out("my_get_busid_only 1111\n");
		my_get_busid_only(port*2,busid,tran_stream->note);
		tran_data_out("my_get_busid_only 2222,busid = %s\n",busid);
	}
		// ty_ctl(tran_stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port*2, busid);
	else
	{
		tran_data_out("my_get_busid_only 3333\n");
		// ty_ctl(tran_stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, busid);
		my_get_busid_only(port,busid,tran_stream->note);
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[port - 1].port_info.arm_version))  //插着独立的机械臂
		{
			strcat(busid,".2");
		}
	}
		
	memcpy(send_tmp, "\x01\x06\x00\x07", 4);
	send_tmp[4] = strlen((char*)busid) / 0x1000000; send_tmp[5] = strlen((char*)busid) / 0x10000;
	send_tmp[6] = strlen((char*)busid) / 0x100; send_tmp[7] = strlen((char*)busid) % 0x100;
	memcpy(send_tmp + 8, busid, strlen((char*)busid));
	result = ty_socket_write(fd, send_tmp, strlen((char*)busid) + 8);
	if (result < 0)
	{
		return -1;
	}
	return 0;
}


/********处理转发链路具体业务************/
static void deal_trans_line_oper(void *arg)
{
	struct _tran_data        *tran_stream;
	struct _new_usbshare_parm* parm;
	struct line_transport * transportline;
	int fd;
	unsigned char recvjson[1024] = { 0 };
	int len_data = 0;
	//int client_fd;
	int result;
	uint32 ver_code;
	char str_port[10] = { 0 };
	int dev_port = 0;
	//tran_data_out("")
	transportline = arg;
	fd = transportline->fd;
	tran_stream = transportline->stream;
	parm = &transportline->parm;

	// my_tran_stream = transportline->stream;

    struct timeval timeout;
    timeout.tv_sec = 15;  // 设置超时时间为10秒
    timeout.tv_usec = 0;

    // if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    //     // perror("setsockopt");
	// 	printf("		setsockopt   RCV\r\n");
    //     // exit(EXIT_FAILURE);
    // }
    // if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    //     // perror("setsockopt");
	// 	printf("		setsockopt   SND\r\n");
	// 	// exit(EXIT_FAILURE);
    // }

	//接收协议头
	ver_code = recv_midware_ver_code(fd);		///socket 接收
	if (ver_code != 0X01068007)
	{
		shutdown(fd, SHUT_RDWR);
		close(fd);

		_lock_set(tran_stream->lock);
		transportline->state = 0;
		_lock_un(tran_stream->lock);

		_lock_set(parm->data_lock);
		parm->state = 0;
		_lock_un(parm->data_lock);

		return;
	}
	// else
	// {
	// 	struct timeval timeout1;
	// 	timeout1.tv_sec = 0;  // 设置为不超时
	// 	timeout1.tv_usec = 0;
	// 	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout1, sizeof(timeout1)) < 0) {
	// 		printf("setsockopt   RCV\r\n");
	// 	}
	// // 	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
	// // 		printf("setsockopt   SND\r\n");
	// // 	}
	// }
	//接收json长度
	len_data = recv_data_len(fd);
	if (len_data <= 0)
	{
		//返回错误
		shutdown(fd, SHUT_RDWR);
		close(fd);

		_lock_set(tran_stream->lock);
		transportline->state = 0;
		_lock_un(tran_stream->lock);

		_lock_set(parm->data_lock);
		parm->state = 0;
		_lock_un(parm->data_lock);
		return;
	}
	//接收实体json
	if (ty_socket_read(fd, recvjson, len_data)<0)
	{
		tran_data_out("获取port_str错误\n");
		shutdown(fd, SHUT_RDWR);
		close(fd);

		_lock_set(tran_stream->lock);
		transportline->state = 0;
		_lock_un(tran_stream->lock);

		_lock_set(parm->data_lock);
		parm->state = 0;
		_lock_un(parm->data_lock);
		return;
	}
	// tran_data_out("recv data:%s\n", recvjson);
	result = deal_cmd_newshare(fd, recvjson, str_port, 0);		//根据不同指令处理响应请求
	if (result <0)
	{
		tran_data_out("deal_cmd_newshare err");
		shutdown(fd, SHUT_RDWR);
		close(fd);

		_lock_set(tran_stream->lock);
		transportline->state = 0;
		_lock_un(tran_stream->lock);

		_lock_set(parm->data_lock);
		parm->state = 0;
		_lock_un(parm->data_lock);
		return;
	}
	tran_data_out("deal_cmd_newshare result:%d--datalock :%d\n", result, parm->data_lock);
	if (result == RESULT_OK_FOR_APPLY_PORT_USE_GW)				//公网加载
	{

		//char bus_id[20] = { 0 }, send_tmp[200] = { 0 };
		//tran_data_out("公网加载中000！！！--port is :%d\n", dev_port);
		dev_port = atoi(str_port);
		_lock_set(parm->data_lock);
		parm->type = 1;
		parm->handle = fd;
		parm->port = dev_port;
		_lock_un(parm->data_lock);
		tran_data_out("公网加载中！！！--port is :%d\n", dev_port);

		// tran_stream->dev_port = dev_port/2;

		result = wait_for_8007_busid(fd, tran_stream);				//等待接收处理获取busid请求
		if (result < 0)
		{
			tran_data_out("##############################wait_for_8007_busid##########################################\n");
			shutdown(fd, SHUT_RDWR);
			close(fd);

			_lock_set(tran_stream->lock);
			transportline->state = 0;
			_lock_un(tran_stream->lock);

			_lock_set(parm->data_lock);
			parm->state = 0;
			_lock_un(parm->data_lock);

			return;
		}

		result = trans_usb_share(tran_stream, dev_port, fd);		//处理转发链路端口加载具体动作
		if (result != 0)
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);

			_lock_set(tran_stream->lock);
			transportline->state = 0;
			_lock_un(tran_stream->lock);

			_lock_set(parm->data_lock);
			parm->state = 0;
			_lock_un(parm->data_lock);

			return;
		}
	}

	shutdown(fd, SHUT_RDWR);
	close(fd);

	_lock_set(tran_stream->lock);
	transportline->state = 0;
	_lock_un(tran_stream->lock);

	_lock_set(parm->data_lock);
	parm->state = 0;
	_lock_un(parm->data_lock);

	return;
}
/**********获取一条转发链路具体操作*********/
static int tran_date_open_trans_line(struct _tran_data_fd *id, va_list args)
{
	int i;
	struct _new_usbshare_parm  parm;
	struct _new_usbshare_parm  *parmtmp;
	struct _tran_data        *stream;
	char ip[100] = { 0 };

	stream = id->dev;
	parmtmp = va_arg(args, struct _new_usbshare_parm *);
	memcpy(&parm, parmtmp, sizeof(struct _new_usbshare_parm));

	for (i = 0; i < DF_TCP_SER_CONNECT_LINE_NUM_MAX; i++)		//tcp最大传输链路上限
	{
		if (stream->transport_line[i].state == 0)
			break;
	}
	tran_data_out("tran_date_open_trans_line,i = %d!\n",i);
	if (i >= DF_TCP_SER_CONNECT_LINE_NUM_MAX)
	{
		tran_data_out("线路已满!\n");
		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL;
	}
	unsigned char c_buf[100] = { 0 };
	int len = 0;
	_lock_set(stream->lock);
	stream->transport_line[i].state = 1;
	_lock_un(stream->lock);
	prepare_connect_packet((char*)c_buf, &len, &parm);		  ///组包
	memset(parmtmp->client_id, 0, sizeof(parmtmp->client_id));//new add by whl
	memcpy(parmtmp->client_id, parm.client_id, strlen(parm.client_id));
	memcpy(ip, parm.trans_server, strlen(parm.trans_server));

	//stream->transport_line[i].fd = socket_tcp_open_send("103.27.4.61", 999, c_buf, len);
	stream->transport_line[i].fd = socket_tcp_open_send(ip, 999, c_buf, len);		//tcp 数据发送
	if (stream->transport_line[i].fd < 0)
	{
		tran_data_out("stream->transport_line[i].fd < 0\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);

		return MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR;
	}
	unsigned char conn_recv[50] = { 0 };
	int result = 0;
	result = ty_socket_read(stream->transport_line[i].fd, conn_recv, 8);			//socket 接收
	if (result<0)
	{
		tran_data_out("-----中转服务器联机返回失败\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		shutdown(stream->transport_line[i].fd, SHUT_RDWR);
		close(stream->transport_line[i].fd);
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR;
	}

	stream->transport_line[i].stream = stream;
	memcpy(&stream->transport_line[i].parm, &parm, sizeof(struct _new_usbshare_parm));
	//stream->transport_line[i].parm = &parm;
	if (_delay_task_add("transport", deal_trans_line_oper, &stream->transport_line[i], 0)<0)
	{
		tran_data_out("创建队列失败\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		shutdown(stream->transport_line[i].fd, SHUT_RDWR);
		close(stream->transport_line[i].fd);
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL;

	}
	return stream->transport_line[i].fd;
}


int my_tran_date_open_trans_line(struct _new_usbshare_parm  *parmtmp)
{
	int i;
	struct _new_usbshare_parm  parm;
	struct _tran_data        *stream;
	char ip[100] = { 0 };
	unsigned char c_buf[100] = { 0 };
	int len = 0;

	stream = my_tran_data;
	memcpy(&parm, parmtmp, sizeof(struct _new_usbshare_parm));

	_lock_set(stream->lock);
	for (i = 0; i < DF_TCP_SER_CONNECT_LINE_NUM_MAX; i++)		//tcp最大传输链路上限
	{
		if (stream->transport_line[i].state == 0)
			break;
	}
	tran_data_out("-----------------------------------------tran_date_open_trans_line,i = %d!\n",i);
	if (i >= DF_TCP_SER_CONNECT_LINE_NUM_MAX)
	{
		_lock_un(stream->lock);
		tran_data_out("线路已满!\n");
		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL;
	}
	// _lock_set(stream->lock);
	stream->transport_line[i].state = 1;
	_lock_un(stream->lock);
	prepare_connect_packet((char*)c_buf, &len, &parm);		  ///组包
	memset(parmtmp->client_id, 0, sizeof(parmtmp->client_id));//new add by whl
	memcpy(parmtmp->client_id, parm.client_id, strlen(parm.client_id));
	memcpy(ip, parm.trans_server, strlen(parm.trans_server));

	//stream->transport_line[i].fd = socket_tcp_open_send("103.27.4.61", 999, c_buf, len);
	stream->transport_line[i].fd = socket_tcp_open_send(ip, 999, c_buf, len);		//tcp 数据发送
	if (stream->transport_line[i].fd < 0)
	{
		tran_data_out("stream->transport_line[i].fd < 0\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);

		return MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR;
	}
	unsigned char conn_recv[50] = { 0 };
	int result = 0;
	result = ty_socket_read(stream->transport_line[i].fd, conn_recv, 8);			//socket 接收
	if (result<0)
	{
		tran_data_out("-----中转服务器联机返回失败\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		shutdown(stream->transport_line[i].fd, SHUT_RDWR);
		close(stream->transport_line[i].fd);
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_CONNECT_TRANS_SERVER_ERR;
	}

	stream->transport_line[i].stream = stream;
	memcpy(&stream->transport_line[i].parm, &parm, sizeof(struct _new_usbshare_parm));
	//stream->transport_line[i].parm = &parm;
	if (_delay_task_add("transport", deal_trans_line_oper, &stream->transport_line[i], 0)<0)
	{
		tran_data_out("创建队列失败\n");
		_lock_set(stream->lock);
		stream->transport_line[i].state = 0;
		shutdown(stream->transport_line[i].fd, SHUT_RDWR);
		close(stream->transport_line[i].fd);
		_lock_un(stream->lock);

		_lock_set(parm.data_lock);
		parmtmp->state = 0;
		_lock_un(parm.data_lock);
		return MQTT_GW_USBSHARE_OPEN_PORT_ERR_SPACE_FULL;

	}
	return stream->transport_line[i].fd;
}


#endif
int _usbip_file_read(const char *busid, struct _usbip_file *file)
{
	int result;
	//char path[DF_TY_PATH_MAX];
	//tran_data_out("读取:[%s]的usbip信息\n",busid);
	result = _usb_file_get_infor(busid, &file->dev);
	if (result<0)
	{
		//tran_data_out("该设备物理不在位\n");
		return -1;
	}
	memcpy(file->busid, busid, strlen(busid));

	return 0;
}

static int deal_8009_0009(struct _tran_data *stream, int fd, int *port, uint32 ip,int reattach_flag)
{

	uint8 state[4];
	uint8 port_str[4];
	//int client_fd;
	int result,with_robot_arm = -1;
	//int buf_len;
	struct op_devlist_reply reply;
	struct _usbip_file  usbip[DF_TY_USB_PORT_N_MAX];
	struct usb_device   device;
	struct usb_interface    interface;
	int i, n, j, err = -1;
	uint8 s_buf[81920];
	memset(s_buf, 0, sizeof(s_buf));

	uint8 abuf[2000];
	uint8 bbuf[81920];
	char busid[50] = { 0 },tmp_busid[64] = {0};

	int app_fd;
	char oper_id[128] = {0x00},user[128] = {0x00};

	LOAD_DATA load_data = {0x00};
	tran_data_out("此链路为3240首次获取信息请求,读取后续字节\n");
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针

	
	if(NULL == usb_port_class->load_action.fun_load_start)
	{
		tran_data_out("NULL == usb_port_class->load_action.fun_load_start=%x\n",&usb_port_class->load_action);
		return -1;
	}
		


	memset(abuf, 0, sizeof(abuf));
	memset(bbuf, 0, sizeof(bbuf));
	memset(state, 0, sizeof(state));
	//tran_data_out("此链路为3240首次获取信息请求,读取后续字节\n");
	//联机命令参数部分固定102个字节
	int flags;
	if (flags = fcntl(fd, F_GETFL, 0) < 0)
	{
		return -1;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
	{
		return -1;
	}
	if (ty_socket_read(fd, state, 4)<0)
	{
		tran_data_out("获取状态错误\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		tran_data_out("状态字不对\n");
		return -1;
	}

	if (ty_socket_read(fd, port_str, 4)<0)
	{
		tran_data_out("获取端口错误\n");
		return -1;
	}
	*port = port_str[3];

	if (ty_socket_read(fd, user, 32)<0)
	{
		tran_data_out("没有user\n");
	}
	if (ty_socket_read(fd, oper_id, 32)<0)
	{
		tran_data_out("没有client_id\n");
	}

	if (flags = fcntl(fd, F_GETFL, 0) < 0)
	{
		return -1;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
	{
		return -1;
	}
	printf("########################RECV PORT IS :%d########################\n", *port);
	
	
	
	//load_data.port = *port;
	if(1 == *((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //先判断是否是机械臂的机柜  load_data.port = ((*port*2) - 1) > 0?((*port*2)-1):0;
		load_data.port = (2*(*port) - 1) > 0?(2*(*port)-1):0;
	else
		load_data.port = (*port - 1) > 0?(*port-1):0;

	with_robot_arm = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
	if(with_robot_arm)
	{
		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(load_data.port))  //判断是否在升级中
		{
			result = DF_ERR_PORT_CORE_TY_USB_PORT_USED;
			logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,机械臂正在升级中\r\n", *port);
			return result;
		}
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(load_data.port))   //是机械臂
		{
			result = NEW_PROTOCOL_ERR_PORT_IS_ROBOIARM;
			ty_socket_write(fd, (uint8 *)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02", 8);
			logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,此端口是机械臂\r\n", *port);
			return result;
			
		}
	}
	else  //不是自带机械臂的机柜，判断是否有插独立的机械臂
	{
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[load_data.port].port_info.arm_version))
		{
			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(load_data.port))  //判断是否在升级中
			{
				result = DF_ERR_PORT_CORE_TY_USB_PORT_USED;
				logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,机械臂正在升级中\r\n", *port);
				return result;
			}
		}
	}
	logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口,%d,user = %s,client_id = %s\r\n", *port,user,oper_id);
	memcpy(load_data.oper_id,oper_id,strlen(oper_id));
	memcpy(load_data.user,user,strlen(user));
	load_data.ip = ip;
	load_data.port_fd = &app_fd;
	load_data.net_type = INTRANET;
	if(reattach_flag == 0)
	{
	result = *((int *)(usb_port_class->load_action.fun_load_start((void *)(&load_data)))); //执行一次加载
	printf("result = %d \n",result);
	//result = open_port_attach(*port, ip, client, &app_fd);
	if (result < 0)
	{
		if (result == DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION)
		{
			ty_socket_write(fd, (uint8 *)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01", 8);
		}
		return result;
		}
	}

	//get_usb_busid(*port, busid);//获取busid
	// ty_ctl(usb_port_class->module->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, load_data.port + 1, busid);
	my_get_busid_only(load_data.port + 1,busid,stream->note);
	if(1 != with_robot_arm)  //非内置机械臂机柜
	{
		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[load_data.port].port_info.arm_version))  //插着独立的机械臂
		{
			strcat(busid,".2");
		}
	}

	_lock_set(stream->lock);
	tran_data_out("获取可加载的设备数量\n");
	for (i = 0, memset(&reply, 0, sizeof(reply)); i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		//tran_data_out("申请内存 ndev = %d\n",reply.ndev);
		// usbip[reply.ndev] = malloc(sizeof(struct _usbip_file));
		// //tran_data_out("申请内存成功 ndev = %d\n",reply.ndev);
		// if (usbip[reply.ndev] == NULL)
		// {
		// 	tran_data_out("申请内存失败\n");
		// 	_lock_un(stream->lock);
		// 	n = reply.ndev;
		// 	goto _ERRO;
		// }
		tran_data_out("i = %d,reply.ndev= %d,busid_len = %d\n",i,reply.ndev,strlen(stream->data[i].busid));
		memset(&usbip[reply.ndev], 0, sizeof(struct _usbip_file));
		memset(tmp_busid,0x00,sizeof(tmp_busid));
		strcpy(tmp_busid,stream->data[i].busid);
		if(1 != with_robot_arm)  //非内置机械臂机柜
		{
			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))  //插着独立的机械臂
			{
				strcat(tmp_busid,".2");
			}
		}
		if (_usbip_file_read(tmp_busid, &usbip[reply.ndev])<0)
		{
			tran_data_out("获取usb信息失败,stream->data[%d].busid = %s\n",i,tmp_busid);
			//free(usbip[reply.ndev]);
			continue;
		}
		//tran_data_out("可加载设备busid = %s,端口号 = %d\n", stream->data[i].busid,0);
		reply.ndev++;
	}
	_lock_un(stream->lock);
	n = reply.ndev;
	tran_data_out("一共有:%d个设备需要加载给对方\n", n);
	//if(usbip_send_op_common(fd,OP_REP_DEVLIST,ST_OK)<0)
	//    goto _ERRO;    
	//PACK_OP_DEVLIST_REPLY(0, &reply);
	//if(ty_socket_write(fd,&reply,sizeof(reply))<0)
	//    goto _ERRO;
	s_buf[0] = 0x01; s_buf[1] = 0x06;
	s_buf[2] = 0x00; s_buf[3] = 0x09;
	s_buf[4] = 0x00; s_buf[5] = 0x00;
	s_buf[6] = 0x00; s_buf[7] = 0x00;
	int seek = 8;
	for (i = 0; i<n; i++)
	{
		memset(&device, 0, sizeof(struct usb_device));
		get_usb_dev_pdu(&usbip[i].dev, usbip[i].busid, &device);
		if (memcmp(usbip[i].busid, busid, strlen(busid)) != 0)//匹配到同一个busid的时候才拷贝
		{
			printf("usbip[i].busid = %s,busid = %s\n",usbip[i].busid,busid);
			continue;
		}
		tran_data_out("找到匹配的busid:%s\n", busid);
		//tran_data_out("发送usb设备信息\n");
		pack_usb_device(0, &device);
		memcpy(s_buf + seek, &device, sizeof(struct usb_device));
		seek = seek + sizeof(struct usb_device);
		//if(ty_socket_write(fd,&device,sizeof(device))<0)
		//    goto _ERRO;
		//tran_data_out("USB接口设备信息:%d\n",usbip[i]->dev.bNumInterfaces);
		for (j = 0; j<usbip[i].dev.bNumInterfaces; j++)
		{
			memset(&interface, 0, sizeof(interface));
			get_usb_interface(&usbip[i].dev.interface[j], &interface);
			//if(ty_socket_write(fd,&interface,sizeof(interface))<0)
			//    goto _ERRO;
			memcpy(s_buf + seek, &interface, sizeof(interface));
			seek = seek + sizeof(interface);
		}
	}
	printf("seek len:%d\n", seek);
	for (i = 0; i < seek; i++)
	{
		printf("%02x ", s_buf[i]);
	}
	printf("\n\n");
	if (ty_socket_write(fd, s_buf, seek)<0)
		goto _ERRO;
	err = 0;
_ERRO:
	// for (i = 0; i<n; i++)
	// {  //tran_data_out("释放USBIP内存\n"); 
	// 	free(usbip[i]);
	// }
	printf("lbc err = %d\n",err);
	return err;

}

// static int deal_8009_0009(struct _tran_data *stream, int fd, int *port, uint32 ip,int reattach_flag)
// {

// 	uint8 state[4];
// 	uint8 port_str[4];
// 	//int client_fd;
// 	int result,with_robot_arm = -1;
// 	//int buf_len;
// 	struct op_devlist_reply reply;
// 	struct _usbip_file  *usbip[DF_TY_USB_PORT_N_MAX];
// 	struct usb_device   device;
// 	struct usb_interface    interface;
// 	int i, n, j, err = -1;
// 	uint8 s_buf[81920];
// 	memset(s_buf, 0, sizeof(s_buf));

// 	uint8 abuf[2000];
// 	uint8 bbuf[81920];
// 	char busid[50] = { 0 },tmp_busid[64] = {0};

// 	int app_fd;
// 	char oper_id[128] = {0x00},user[128] = {0x00};

// 	LOAD_DATA load_data = {0x00};
// 	tran_data_out("此链路为3240首次获取信息请求,读取后续字节\n");
// 	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针

	
// 	if(NULL == usb_port_class->load_action.fun_load_start)
// 	{
// 		tran_data_out("NULL == usb_port_class->load_action.fun_load_start=%x\n",&usb_port_class->load_action);
// 		return -1;
// 	}
		


// 	memset(abuf, 0, sizeof(abuf));
// 	memset(bbuf, 0, sizeof(bbuf));
// 	memset(state, 0, sizeof(state));
// 	//tran_data_out("此链路为3240首次获取信息请求,读取后续字节\n");
// 	//联机命令参数部分固定102个字节
// 	int flags;
// 	if (flags = fcntl(fd, F_GETFL, 0) < 0)
// 	{
// 		return -1;
// 	}
// 	flags |= O_NONBLOCK;
// 	if (fcntl(fd, F_SETFL, flags) < 0)
// 	{
// 		return -1;
// 	}
// 	if (ty_socket_read(fd, state, 4)<0)
// 	{
// 		tran_data_out("获取状态错误\n");
// 		return -1;
// 	}
// 	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
// 	{
// 		tran_data_out("状态字不对\n");
// 		return -1;
// 	}

// 	if (ty_socket_read(fd, port_str, 4)<0)
// 	{
// 		tran_data_out("获取端口错误\n");
// 		return -1;
// 	}
// 	*port = port_str[3];

// 	if (ty_socket_read(fd, user, 32)<0)
// 	{
// 		tran_data_out("没有user\n");
// 	}
// 	if (ty_socket_read(fd, oper_id, 32)<0)
// 	{
// 		tran_data_out("没有client_id\n");
// 	}

// 	if (flags = fcntl(fd, F_GETFL, 0) < 0)
// 	{
// 		return -1;
// 	}
// 	flags &= ~O_NONBLOCK;
// 	if (fcntl(fd, F_SETFL, flags) < 0)
// 	{
// 		return -1;
// 	}
// 	printf("########################RECV PORT IS :%d########################\n", *port);
	
	
	
// 	//load_data.port = *port;
// 	if(1 == *((int *)(usb_port_class->fun_check_with_robot_arm(NULL))))   //先判断是否是机械臂的机柜  load_data.port = ((*port*2) - 1) > 0?((*port*2)-1):0;
// 		load_data.port = (2*(*port) - 1) > 0?(2*(*port)-1):0;
// 	else
// 		load_data.port = (*port - 1) > 0?(*port-1):0;

// 	with_robot_arm = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));
// 	if(with_robot_arm)
// 	{
// 		if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(load_data.port))  //判断是否在升级中
// 		{
// 			result = DF_ERR_PORT_CORE_TY_USB_PORT_USED;
// 			logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,机械臂正在升级中\r\n", *port);
// 			return result;
// 		}
// 		if(0 == get_robot_arm_class()->fun_judge_robot_arm(load_data.port))   //是机械臂
// 		{
// 			result = NEW_PROTOCOL_ERR_PORT_IS_ROBOIARM;
// 			ty_socket_write(fd, (uint8 *)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02", 8);
// 			logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,此端口是机械臂\r\n", *port);
// 			return result;
			
// 		}
// 	}
// 	else  //不是自带机械臂的机柜，判断是否有插独立的机械臂
// 	{
// 		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[load_data.port].port_info.arm_version))
// 		{
// 			if(0 > usb_port_class->online_bank_action.mechanical_arm_action.class->fun_arm_get_port_update_state(load_data.port))  //判断是否在升级中
// 			{
// 				result = DF_ERR_PORT_CORE_TY_USB_PORT_USED;
// 				logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口【%d】失败,机械臂正在升级中\r\n", *port);
// 				return result;
// 			}
// 		}
// 	}
// 	logout(INFO, "tcp", "attach", "deal_8009_0009,加载USB端口,%d,user = %s,client_id = %s\r\n", *port,user,oper_id);
// 	memcpy(load_data.oper_id,oper_id,strlen(oper_id));
// 	memcpy(load_data.user,user,strlen(user));
// 	load_data.ip = ip;
// 	load_data.port_fd = &app_fd;
// 	load_data.net_type = INTRANET;
// 	if(reattach_flag == 0)
// 	{
// 	result = usb_port_class->load_action.fun_load_start((void *)(&load_data)); //执行一次加载
// 	//result = open_port_attach(*port, ip, client, &app_fd);
// 	if (result < 0)
// 	{
// 		if (result == DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION)
// 		{
// 			ty_socket_write(fd, (uint8 *)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01", 8);
// 		}
// 		return result;
// 		}
// 	}

// 	//get_usb_busid(*port, busid);//获取busid
// 	// ty_ctl(usb_port_class->module->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, load_data.port + 1, busid);
// 	my_get_busid_only(load_data.port + 1,busid,stream->note);
// 	if(1 != with_robot_arm)  //非内置机械臂机柜
// 	{
// 		if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[load_data.port].port_info.arm_version))  //插着独立的机械臂
// 		{
// 			strcat(busid,".2");
// 		}
// 	}

// 	_lock_set(stream->lock);
// 	tran_data_out("获取可加载的设备数量\n");
// 	for (i = 0, memset(&reply, 0, sizeof(reply)); i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
// 	{
// 		if (stream->data[i].state == 0)
// 			continue;
// 		//tran_data_out("申请内存 ndev = %d\n",reply.ndev);
// 		usbip[reply.ndev] = malloc(sizeof(struct _usbip_file));
// 		//tran_data_out("申请内存成功 ndev = %d\n",reply.ndev);
// 		if (usbip[reply.ndev] == NULL)
// 		{
// 			tran_data_out("申请内存失败\n");
// 			_lock_un(stream->lock);
// 			n = reply.ndev;
// 			goto _ERRO;
// 		}
// 		memset(usbip[reply.ndev], 0, sizeof(struct _usbip_file));
// 		memset(tmp_busid,0x00,sizeof(tmp_busid));
// 		strcpy(tmp_busid,stream->data[i].busid);
// 		if(1 != with_robot_arm)  //非内置机械臂机柜
// 		{
// 			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))  //插着独立的机械臂
// 			{
// 				strcat(tmp_busid,".2");
// 			}
// 		}
// 		if (_usbip_file_read(tmp_busid, usbip[reply.ndev])<0)
// 		{
// 			tran_data_out("获取usb信息失败,stream->data[%d].busid = %s\n",i,tmp_busid);
// 			free(usbip[reply.ndev]);
// 			continue;
// 		}
// 		//tran_data_out("可加载设备busid = %s,端口号 = %d\n", stream->data[i].busid,0);
// 		reply.ndev++;
// 	}
// 	_lock_un(stream->lock);
// 	n = reply.ndev;
// 	tran_data_out("一共有:%d个设备需要加载给对方\n", n);
// 	//if(usbip_send_op_common(fd,OP_REP_DEVLIST,ST_OK)<0)
// 	//    goto _ERRO;    
// 	//PACK_OP_DEVLIST_REPLY(0, &reply);
// 	//if(ty_socket_write(fd,&reply,sizeof(reply))<0)
// 	//    goto _ERRO;
// 	s_buf[0] = 0x01; s_buf[1] = 0x06;
// 	s_buf[2] = 0x00; s_buf[3] = 0x09;
// 	s_buf[4] = 0x00; s_buf[5] = 0x00;
// 	s_buf[6] = 0x00; s_buf[7] = 0x00;
// 	int seek = 8;
// 	for (i = 0; i<n; i++)
// 	{
// 		memset(&device, 0, sizeof(struct usb_device));
// 		get_usb_dev_pdu(&usbip[i]->dev, usbip[i]->busid, &device);
// 		if (memcmp(usbip[i]->busid, busid, strlen(busid)) != 0)//匹配到同一个busid的时候才拷贝
// 		{
// 			printf("usbip[i]->busid = %s,busid = %s\n",usbip[i]->busid,busid);
// 			continue;
// 		}
// 		tran_data_out("找到匹配的busid:%s\n", busid);
// 		//tran_data_out("发送usb设备信息\n");
// 		pack_usb_device(0, &device);
// 		memcpy(s_buf + seek, &device, sizeof(struct usb_device));
// 		seek = seek + sizeof(struct usb_device);
// 		//if(ty_socket_write(fd,&device,sizeof(device))<0)
// 		//    goto _ERRO;
// 		//tran_data_out("USB接口设备信息:%d\n",usbip[i]->dev.bNumInterfaces);
// 		for (j = 0; j<usbip[i]->dev.bNumInterfaces; j++)
// 		{
// 			memset(&interface, 0, sizeof(interface));
// 			get_usb_interface(&usbip[i]->dev.interface[j], &interface);
// 			//if(ty_socket_write(fd,&interface,sizeof(interface))<0)
// 			//    goto _ERRO;
// 			memcpy(s_buf + seek, &interface, sizeof(interface));
// 			seek = seek + sizeof(interface);
// 		}
// 	}
// 	printf("seek len:%d\n", seek);
// 	for (i = 0; i < seek; i++)
// 	{
// 		printf("%02x ", s_buf[i]);
// 	}
// 	printf("\n\n");
// 	if (ty_socket_write(fd, s_buf, seek)<0)
// 		goto _ERRO;
// 	err = 0;
// _ERRO:
// 	for (i = 0; i<n; i++)
// 	{  //tran_data_out("释放USBIP内存\n"); 
// 		free(usbip[i]);
// 	}
// 	printf("lbc err = %d\n",err);
// 	return err;

// }





static int deal_8005_0005(struct _tran_data    *stream, int fd)
{
	uint8 state[4];
	struct op_devlist_reply reply;
	struct _usbip_file  *usbip[DF_TY_USB_PORT_N_MAX];
	struct usb_device   device;
	struct usb_interface    interface;
	int i, n, j, err = -1;
	int result,with_robot_arm = -1;
	uint8 s_buf[81920];
	char tmp_busid[64] = {0};
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针
	memset(s_buf, 0, sizeof(s_buf));

	if (ty_socket_read(fd, state, 4)<0)
	{
		tran_data_out("获取状态错误\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		tran_data_out("状态字不对\n");
		return -1;
	}


	_lock_set(stream->lock);
	tran_data_out("deal_8005_0005 获取可加载的设备数量\n");

	with_robot_arm = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));

	for (i = 0, memset(&reply, 0, sizeof(reply)); i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		//tran_data_out("申请内存 ndev = %d\n",reply.ndev);
		usbip[reply.ndev] = malloc(sizeof(struct _usbip_file));
		//tran_data_out("申请内存成功 ndev = %d\n",reply.ndev);
		if (usbip[reply.ndev] == NULL)
		{
			tran_data_out("申请内存失败\n");
			_lock_un(stream->lock);
			n = reply.ndev;
			goto _ERRO;
		}
		memset(usbip[reply.ndev], 0, sizeof(struct _usbip_file));
		memset(tmp_busid,0x00,sizeof(tmp_busid));
		strcpy(tmp_busid,stream->data[i].busid);
		if(1 != with_robot_arm)  //非内置机械臂机柜
		{
			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))  //插着独立的机械臂
			{
				strcat(tmp_busid,".2");
			}
		}
		// printf("tmp_busid=%s\n",tmp_busid);
		if (_usbip_file_read(tmp_busid, usbip[reply.ndev])<0)
		{
			//tran_data_out("获取usb信息失败\n");
			free(usbip[reply.ndev]);
			continue;
		}
		reply.ndev++;
	}
	_lock_un(stream->lock);
	n = reply.ndev;
	tran_data_out("一共有:%d个设备需要加载给对方\n", n);
	//if(usbip_send_op_common(fd,OP_REP_DEVLIST,ST_OK)<0)
	//    goto _ERRO;    
	//PACK_OP_DEVLIST_REPLY(0, &reply);
	//if(ty_socket_write(fd,&reply,sizeof(reply))<0)
	//    goto _ERRO;
	s_buf[0] = 0x01; s_buf[1] = 0x06;
	s_buf[2] = 0x00; s_buf[3] = 0x05;
	s_buf[4] = 0x00; s_buf[5] = 0x00;
	s_buf[6] = 0x00; s_buf[7] = 0x00;
	s_buf[8] = 0x00; s_buf[9] = 0x00;
	s_buf[10] = 0x00; s_buf[11] = n;
	int seek = 12;
	for (i = 0; i<n; i++)
	{
		printf("i = %d,busid = %s\n",i,usbip[i]->busid);
		memset(&device, 0, sizeof(device));
		get_usb_dev_pdu(&usbip[i]->dev, usbip[i]->busid, &device);		//获取usb dev信息主要是usb描述符中的结构
		//tran_data_out("发送usb设备信息\n");
		pack_usb_device(0, &device);
		memcpy(s_buf + seek, &device, sizeof(device));
		seek = seek + sizeof(device);
		//if(ty_socket_write(fd,&device,sizeof(device))<0)
		//    goto _ERRO;
		//tran_data_out("USB接口设备信息:%d\n",usbip[i]->dev.bNumInterfaces);
		for (j = 0; j<usbip[i]->dev.bNumInterfaces; j++)
		{
			memset(&interface, 0, sizeof(interface));
			get_usb_interface(&usbip[i]->dev.interface[j], &interface);
			//if(ty_socket_write(fd,&interface,sizeof(interface))<0)
			//    goto _ERRO;
			memcpy(s_buf + seek, &interface, sizeof(interface));
			seek = seek + sizeof(interface);
		}
	}
	if (ty_socket_write(fd, s_buf, seek)<0)
		goto _ERRO;
	err = 0;
	tran_data_out("deal_8005_0005 exit\n");
_ERRO:
	for (i = 0; i<n; i++)
	{  //tran_data_out("释放USBIP内存\n"); 
		free(usbip[i]);
	}
	return err;
}
static void pack_uint16(int pack, uint16 *val)
{
	uint8 buf[2];
	uint16 hex;
	if (pack != 0)
		return;
	hex = *val;
	buf[0] = ((hex >> 8) & 0xff); buf[1] = (hex & 0xff);
	hex = (uint16)buf[1] * 256 + buf[0];
	*val = hex;
}

static void pack_uint32(int pack, uint32 *val)
{
	uint8 buf[4];
	uint32 hex;
	if (pack != 0)
		return;
	hex = *val;
	buf[0] = ((hex >> 24) & 0xff); buf[1] = ((hex >> 16) & 0xff);
	buf[2] = ((hex >> 8) & 0xff); buf[3] = ((hex >> 0) & 0xff);
	hex = ((uint32)buf[3] << 24) + ((uint32)buf[2] << 16) + ((uint32)buf[1] << 8) + ((uint32)buf[0] << 0);
	*val = hex;
}

static int get_usb_interface(const struct _usb_file_interface   *usb_file_interface, struct usb_interface    *interface)
{
	interface->bInterfaceClass = usb_file_interface->bInterfaceClass;
	interface->bInterfaceSubClass = usb_file_interface->bInterfaceSubClass;
	interface->bInterfaceProtocol = usb_file_interface->bInterfaceProtocol;
	return 0;
}
static void pack_usb_device(int pack, struct usb_device *udev)
{
	pack_uint32(pack, &udev->busnum);
	pack_uint32(pack, &udev->devnum);
	pack_uint32(pack, &udev->speed);

	pack_uint16(pack, &udev->idVendor);
	pack_uint16(pack, &udev->idProduct);
	pack_uint16(pack, &udev->bcdDevice);
}

static int get_usb_dev_pdu(const struct _usb_file_dev *usb_dev, const char *busid, struct usb_device    *device)
{
	int i, len;
	device->busnum = usb_dev->busnum;
	device->devnum = usb_dev->devnum;
	memcpy(device->busid, busid, strlen(busid));
	device->idVendor = usb_dev->idVendor;
	device->idProduct = usb_dev->idProduct;
	device->bcdDevice = usb_dev->bcdDevice;

	device->bDeviceClass = usb_dev->bDeviceClass;
	device->bDeviceSubClass = usb_dev->bDeviceSubClass;
	device->bDeviceProtocol = usb_dev->bDeviceProtocol;
	device->bConfigurationValue = usb_dev->bConfigurationValue;
	device->bNumConfigurations = usb_dev->bNumConfigurations;
	device->bNumInterfaces = usb_dev->bNumInterfaces;
	for (i = 0, len = strlen(usb_dev->speed); i<sizeof(_usbip_speed) / sizeof(struct _usbip_speed_n); i++)
	{
		if (len != strlen(_usbip_speed[i].speed))
			continue;
		if (memcmp(usb_dev->speed, _usbip_speed[i].speed, len) == 0)
		{
			device->speed = _usbip_speed[i].num;
			break;

		}
	}
	if (i == sizeof(_usbip_speed) / sizeof(struct _usbip_speed_n))
	{
		device->speed = USB_SPEED_UNKNOWN_A;
	}
	//memcpy(device->path,DF_USB_FILE_SYS_DIR,strlen(DF_USB_FILE_SYS_DIR));
	sprintf(device->path, "%s/%s", DF_USB_FILE_SYS_DIR, busid);
	memcpy(device->busid, busid, strlen(busid));
	return 0;
}
static int get_usbip_dev(const char *busid, struct usb_device *device)
{
	struct _usb_file_dev    usb_dev;
	//int i,len;
	memset(&usb_dev, 0, sizeof(usb_dev));
	if (_usb_file_get_infor(busid, &usb_dev)<0)
		return -1;
	return get_usb_dev_pdu(&usb_dev, busid, device);
}
static int usbip_send_op_common(int sockfd, uint32 code, uint32 status)
{   //int result;
	struct op_common op_common;
	memset(&op_common, 0, sizeof(op_common));
	op_common.version = DF_TY_USBIP_VERSION;
	op_common.code = code;
	op_common.status = status;
	PACK_OP_COMMON(0, &op_common);
	return ty_socket_write(sockfd, (uint8 *)&op_common, sizeof(op_common));
}

static int deal_8003_0003(struct _tran_data    *stream, int fd, uint8 *busid)
{
	uint8 state[4];
	//int result;
	uint8 abuf[2000];
	//int buf_len;
	//int client_fd; 
	uint8 bbuf[81920];
	int with_robot_arm = -1;
	char tmp_busid[64] = {0x00};
	USB_PORT_CLASS *usb_port_class = get_usb_port_class();  //获取usb_port的操作指针

	with_robot_arm = *((int *)(usb_port_class->fun_check_with_robot_arm(NULL)));

	memset(abuf, 0, sizeof(abuf));
	memset(bbuf, 0, sizeof(bbuf));
	memset(state, 0, sizeof(state));
	//memset(busid,0,sizeof(busid));
	if (ty_socket_read(fd, state, 4)<0)
	{
		tran_data_out("获取状态错误\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		tran_data_out("状态字不对\n");
		return -1;
	}
	struct usb_device   dev;
	int i;
	//int num;
	tran_data_out("获取[busid]\n");
	if (ty_socket_read(fd, (uint8 *)busid, DF_TY_USB_BUSID)<0)
	{
		tran_data_out("net获取busid失败\n");
		return -1;
	}
	tran_data_out("获得busid=%s\n", busid);
	_lock_set(stream->lock);
	for(i = 0; i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		memset(tmp_busid,0x00,sizeof(tmp_busid));
		strcpy(tmp_busid,stream->data[i].busid);
		if(1 != with_robot_arm)  //非内置机械臂机柜
		{
			if(0 < strlen(usb_port_class->online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))  //插着独立的机械臂
			{
				strcat(tmp_busid,".2");
			}
		}
			
		if (memcmp(tmp_busid, busid, DF_TY_USB_BUSID) != 0)
			continue;

		break;
	}
	if (i == sizeof(stream->data) / sizeof(stream->data[0]))
	{
		_lock_un(stream->lock);
		tran_data_out("没有这个USB口\n");
		return -1;
	}
	//num = i;
	memset(&dev, 0, sizeof(dev));
	tran_data_out("获取usb设备的信息\n");
	if (get_usbip_dev((char *)busid, &dev)<0)
	{
		_lock_un(stream->lock);
		tran_data_out("获取[%s]信息失败\n", busid);
		return -1;
	}
	tran_data_out("写设备的socket=%d\n", fd);


	_lock_un(stream->lock);
	tran_data_out("设备开始枚举\n");
	usbip_send_op_common(fd, OP_REP_IMPORT, ST_OK);
	pack_usb_device(0, &dev);
	return ty_socket_write(fd, (uint8 *)&dev, sizeof(dev));
}

uint8 seqnum[4];
uint8 devid[4];


#ifdef NEW_USB_SHARE
static uint32 recv_data_len(int fd)
{
	int result;
	uint32 len;
	uint8 len_code[4];
	memset(len_code, 0, sizeof(len_code));
	result = ty_socket_read(fd, len_code, sizeof(len_code));
	if (result<0)
	{
		tran_data_out("读文件失败\n");
		return result;
	}
	len = (uint32)(len_code[0] * 0x1000000) + (uint32)(len_code[1] * 0x10000) + (uint32)(len_code[2] * 0x100) + (uint32)len_code[3];
	return len;
}




static int deal_new_protocol(int sockfd, uint32 ip)
{
	int len_data, result;
	char ip_asc[100] = { 0 };
	char output[2000] = { 0 };
	unsigned char *recvjson = NULL;
	//接收json长度
	len_data = recv_data_len(sockfd);
	if (len_data <= 0)
	{
		tran_data_out("len_data <= 0\n");
		//返回错误
		return -1;
	}
	//接收实体json
	recvjson = (unsigned char *)malloc(len_data + 10);
	if (recvjson == NULL)
	{
		tran_data_out("recvjson == NULL\n");
		return -1;
	}
	memset(recvjson, 0, len_data + 10);
	if (ty_socket_read(sockfd, recvjson, len_data)<0)
	{
		tran_data_out("获取json错误\n");
		free(recvjson);
		return -2;
	}
	// tran_data_out("recv data:%s\n", recvjson);
	if (strstr((char *)recvjson, "ter_restart") != NULL)
	{
		ip2ip_asc(ip, ip_asc);
		write_log("客户端发起重启，IP:%s\n", ip_asc);
	}
	result = deal_cmd_newshare(sockfd, recvjson, output, 1);
	if (result <0)
	{
		tran_data_out("deal_cmd_newshare err");
		free(recvjson);
		return -1;
	}
	free(recvjson);
	return 0;
}
#endif

//主线程回调
static int _transmit_order_hook(int fd, uint32 ip, uint16 port, void *arg)
{
	struct _tran_data    *stream;
	LOAD_DATA load_data = {0x00};
	int result, i;
	int port_num = 0;
	uint32 ver_code;
	uint8 busid[32];

	struct _transmit_sign	line1, line2;
	struct _train_data_oper	line1_data, line2_data;
	long out_q_seqnum_array[OUT_Q_LEN];
	int task2;
	sem_t	sem;

	sem_init(&sem, 0, 0);

	stream = arg;
	memset(busid, 0, sizeof(busid));
	int client_fd;
	//  tran_data_out("_transmit_order_hook 11111111111111111111111111111\n");
WAIT_8003:
	ver_code = recv_midware_ver_code(fd);
	if (ver_code == DF_MID_TER_8005)
	{
		//tran_data_out("处理8005 0005完成\n");
		result = deal_8005_0005(stream, fd);
		if (result != 0)
		{
			return -1;
		}
		goto WAIT_8003;
		//  return	deal_8005_0005(stream, fd);
	}
	else if (ver_code == DF_MID_TER_8003)
	{
		client_fd = deal_8003_0003(stream, fd, busid);
		if (client_fd < 0)
		{
			return client_fd;
		}
		tran_data_out("8003处理完成\n");
	}
	else if (ver_code == DF_DLL_TER_8009)//3240 直接通讯用
	{
		int port = 0;
		if(port_num > 0)
		{
			result = deal_8009_0009(stream, fd, &port,ip,re_attach_data[port_num - 1]);
			re_attach_data[port_num - 1] = 0;
		}
		else
		{
			result = deal_8009_0009(stream, fd, &port,ip,0);
		}
		return result;
	}
#ifdef NEW_USB_SHARE
	else if (ver_code == 0X01068007)//3240 命令处理
		return deal_new_protocol(fd, ip);
#endif
	else
		return 0;

	port_num = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_BUSID_PORT, busid);
	if (port_num<1 || port_num>DF_TY_USB_PORT_N_MAX)
	{
		tran_data_out("获取到的端口号错误\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}


	
	memset(&line1, 0, sizeof(line1));
	memset(&line2, 0, sizeof(line2));
	line1.sem = &sem;
	line1.socket_fd = client_fd;
	line2.sem = &sem;
	line2.socket_fd = fd;
	//port_num--;

	tran_data_out("获取传输类型和端点\n");
	result = usb_dev_init(stream, port_num, busid);
	if (result < 0)
	{
		tran_data_out("USB设备初始化失败\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}
	tran_data_out("_transmit_order_hook开始usbshare使用,打开%d号端口句柄,句柄：%08x\n", port_num, stream->line[port_num - 1].devh);
	tran_data_out("_transmit_order_hook 传输类型为：%d，in_ep = %02x out_ep = %02x,in最大数据帧长度= %d,out最大数据帧长度= %d\n", 
		stream->line[port_num-1].tran_type, stream->line[port_num-1].in_ep, stream->line[port_num-1].out_ep, stream->line[port_num-1].in_max_len, stream->line[port_num-1].out_max_len);
	memcpy(stream->line[port_num-1].busid, busid, strlen((char *)busid));

	get_usb_vid_pid(port_num, &stream->line[port_num-1].vid, &stream->line[port_num-1].pid);

	tran_data_out("开启中间件数据检测线程\n");
	task2 = _scheduled_task_open("main_line", line_task, &line2, 1, NULL);
	_lock_set(stream->lock);
	stream->line[port_num-1].state_mem = 1;
	stream->line[port_num-1].unlink_flag = 0;
	stream->line[port_num-1].out_close = 0;
	stream->line[port_num-1].sem = &sem;
	_lock_un(stream->lock);



	memset(&line1_data, 0, sizeof(line1_data));
	memset(out_q_seqnum_array, 0, sizeof(out_q_seqnum_array));
	memset(&line2_data, 0, sizeof(line2_data));
	for (;;)
	{
		sem_wait(&sem);
		if (stream->line[port_num-1].out_close != 0 && stream->line[port_num-1].out_close != 2)
		{
			tran_data_out("执行外部关闭命令\n");
			break;
		}
		if(re_attach_data[port_num - 1] == 1)
		{
			printf("goto WAIT_8003 \n");
			stream->data[port_num - 1].state = 1;
			if (stream->line[port_num - 1].devh != NULL)
			{
				result = libusb_release_interface(stream->line[port_num - 1].devh, 0);
				if (result < 0)
				{
					tran_data_out("libusb_release_interface_0 result = %d\n", result);
				}
			}

			for (i = 0; i < 3; i++)
				memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));
			if (stream->line[port_num - 1].devh != NULL)
			{
				tran_data_out("trans_usb_share结束usbshare使用,关闭%d号端口句柄\n", port_num);
				libusb_close(stream->line[port_num - 1].devh);
				stream->line[port_num - 1].devh = NULL;
			}

			stream->line[port_num - 1].unlink_num = 0;
			stream->line[port_num - 1].state_mem = 0;
			stream->line[port_num - 1].out_close = 0;
			stream->line[port_num - 1].unlink_flag = 0;
			stream->line[port_num - 1].sem = NULL;
			stream->line[port_num - 1].queue_num = 0;
			stream->line[port_num - 1].test_num = 0;
			memset(&stream->line[port_num - 1].out_q_seqnum_array, 0, sizeof(stream->line[port_num - 1].out_q_seqnum_array));
			_scheduled_task_close(task2);
			goto WAIT_8003;
		}

		if ((result = oper_data(fd, client_fd, &line1_data, out_q_seqnum_array, stream, port_num))<0)			//!!!!!!!!!!!!!!!!!!!!!
		{
			tran_data_out("oper_data检测到中间件链路故障,result = %d\n", result);
			break;
		}
		//待完善
		// set_port_usb_data_tran(port_num);


		_scheduled_task_week_up(task2);


	}
	tran_data_out("退出转发循环,执行关闭流程\n");
	if (stream->line[port_num-1].tran_type == 3)
	{
		tran_data_out("等待所有中断接收队列退出\n");
		for (i = 0; i<3000; i++, usleep(1000))
		{
			if (stream->line[port_num-1].queue_num == 0)
				break;
		}

	}
	if (stream->line[port_num-1].devh != NULL)
	{
		result = libusb_release_interface(stream->line[port_num-1].devh, 0);		//释放之前为指定的设备申请接口，注意这个函数只是释放接口，不会重新加载内核驱动
		if (result < 0)
		{
			tran_data_out("libusb_release_interface result = %d\n", result);
		}

		if (stream->line[port_num-1].interface_decs[1].state != 0)//表示为双接口设备
		{
			result = libusb_release_interface(stream->line[port_num-1].devh, 1);
		}
	}

	for (i = 0; i < 3; i++)
		memset(&stream->line[port_num-1].interface_decs[i], 0, sizeof(struct _interface_dec));
	stream->line[port_num-1].out_close = 1;

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].control_in_thread_num == 0)
		{
			tran_data_out("控制传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].control_out_thread_num == 0)
		{
			tran_data_out("控制传输out线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].bulk_in_thread_num == 0)
		{
			tran_data_out("块传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].bulk_out_thread_num == 0)
		{
			tran_data_out("块传输out线程全部退出完毕\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num - 1].interrupt_out_thread_num == 0)
		{
			tran_data_out("块传输out线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num - 1].interrupt_in_thread_num == 0)
		{
			tran_data_out("块传输in线程全部退出完毕\n");
			break;
		}
	}
	//等待线程退出
	tran_data_out("_transmit_order_hook结束usbshare使用\n");
	if (stream->line[port_num-1].devh != NULL)
	{
		tran_data_out("_transmit_order_hook结束usbshare使用,关闭%d号端口句柄,句柄：%08x\n", port_num, stream->line[port_num - 1].devh);
		libusb_close(stream->line[port_num-1].devh);
		tran_data_out("_transmit_order_hook结束usbshare使用,关闭%d号端口句柄成功\n", port_num);
		stream->line[port_num-1].devh = NULL;
	}
	tran_data_out("_transmit_order_hook结束usbshare使用,close end!\n");
	//libusb_exit(stream->line[port_num-1].context);
	tran_data_out("_transmit_order_hook结束usbshare使用,libusb_exit end!\n");
	tran_data_out("结束libusb调用成功 \n");
	_lock_set(stream->lock);
	stream->line[port_num - 1].state_mem = 0;
	stream->line[port_num - 1].out_close = 0;
	stream->line[port_num - 1].unlink_flag = 0;
	stream->line[port_num - 1].sem = NULL;
	stream->line[port_num - 1].queue_num = 0;
	stream->line[port_num - 1].unlink_num = 0;
	stream->line[port_num - 1].test_num = 0;
	memset(&stream->line[port_num - 1].out_q_seqnum_array, 0, sizeof(stream->line[port_num - 1].out_q_seqnum_array));
	_lock_un(stream->lock);
	shutdown(fd, SHUT_RDWR);
	close(fd);

	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
	_scheduled_task_close(task2);

	load_data.port = (port_num - 1) > 0?(port_num-1):0;
	get_usb_port_class()->load_action.fun_load_stop((void *)(&load_data));
	tran_data_out("_transmit_order_hook,执行关闭流程完成，端口号为【%d】\n\n\n\n", port_num);
	return 0;

}

static int oper_data(int src_line, int dest_line, struct _train_data_oper	*data, long *out_q_seqnum_array, struct _tran_data	*stream, int port_num)
{
	char heart_data[64] = {0x00,0x00,0x00,'B'};
	int len;
	int result;
	//int i;
	int ret = 0;
	int buf_len,data_num = 0,total_len = 0,break_flag = 1,data_num_count = 0,end_flag = 0;
	uint8 read_cmd_data[48] = {0x00};
	uint8 tmp_buff[3] = {0x00};
	uint8 *buf_1;
	uint8 *usbip_data = (uint8*)malloc(TRAN_DATA_LEN);
	
	int usbip_len;
	int len1;
	int direction;
	int ep,i = 0,special_handle_flag = 0;

	if(NULL == usbip_data)
	{
		return -1;
	}

	if (data->buf_len<48)
	{
		// printf("enetr oper_data read\n");
		len = recv(src_line, data->buf + data->buf_len, 48 - data->buf_len, MSG_DONTWAIT);
		// printf("enetr oper_data read,len = %d\n",len);
		if (len == 0)
		{
			tran_data_out("线路关闭1\n");
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -1;
		}
		if (len == -1)
		{
			// tran_data_out("接收为 err again:%d!\n", errno);
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return 0;
		}
		if (len < 0)
		{
			tran_data_out("线路关闭2\n");
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -1;

		}
		data->buf_len += len;
		if (data->buf_len < 48)
		{
			tran_data_out("接收到的头少于48字节!\n");
			if(memcmp(heart_data,data->buf,strlen(heart_data)) == 0)
			{
				send(src_line, heart_data,strlen(heart_data), MSG_NOSIGNAL);
				memset(data->buf,0x00,data->buf_len);
				data->buf_len = 0;
				tran_data_out("response heart_data\n");
			}
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return data->buf_len;
		}
	}
	len = get_head_size(data->buf);
	// printf("111111111 get_head_size len = %d\n",len);
	if(data->buf[0] == 0x00 && data->buf[1] == 0x00 && data->buf[2] == 0x00 && data->buf[3] == 'B')   //自定义头，用于保活
	{
		tran_data_out("保活信息!\n");
		if(send(src_line, heart_data,48, MSG_NOSIGNAL) <= 0)
		{
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -1;
		}
		memset(data->buf,0x00,data->buf_len);
		data->buf_len = 0;
		if(NULL != usbip_data)
		{
			free(usbip_data);
		}
		tran_data_out("response heart_data!\n");
		return 0;

	}
	if(data->buf[0] == 0x00 && data->buf[1] == 0x00 && data->buf[2] == 0x00 && ((data->buf[3]&0x80) == 0x80))   //自定义头，用于区分长度
	{
		// printf("data->buf[3] = %.02x\n",data->buf[3]);
		special_handle_flag = 1;
		data->buf[3] == data->buf[3]&0x7f;
		len = 0x03;
		len1 = recv(src_line, tmp_buff, 3, MSG_DONTWAIT);
		if ((len1 == 0) || (len1<-1))
		{
			tran_data_out("线路关闭oper_read\n");
			if (data->re_buf != NULL)
				free(data->re_buf);
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -4;
		}
		else if (len1 == -1)
			len1 = 0;
		
		// printf("11111111111111 tmp_buff = \n");
		// printf_array((char *)tmp_buff,3);
		data_num = tmp_buff[0];
		len = tmp_buff[1];
		len = (len << 8) | tmp_buff[2];
		// data->buf_len += len1;
		// printf("11111111111111 data_num = %d,len = %d,len1 = %d\n",data_num,len,len1);

	}
	// printf("222222222222 get_head_size len = %d\n",len);
	buf_1 = data->buf;
	if (get_inlen_par(data->buf, out_q_seqnum_array) < 0)
	{
		tran_data_out("get_inlen_par error\n");
		//tran_data_out("data->buf_len = %d\n", data->buf_len);
		// printf_array((char *)data->buf, data->buf_len);
		if(NULL != usbip_data)
		{
			free(usbip_data);
		}
		return -2;
	}

	int transfer_flags = data->buf[20] * 0x1000000 + data->buf[21] * 0x10000 + data->buf[22] * 0x100 + data->buf[23];
	int command = data->buf[3];
	if (command == USBIP_CMD_UNLINK)
	{
		if (stream->line[port_num - 1].unlink_num < 2)
		{
			tran_data_out("################unlink cancel urb request!##############\n");//目前发现金税盘公网加载的时候经常会出现这个错误，采取该措施可以解决
			//Unlink = 1;
			unsigned char unlink_data[1000] = { 0 };
			memcpy(unlink_data, "\x00\x00\x00\x04", 4);//ret unlink
			memcpy(unlink_data + 4, data->buf + 4, 16);
			memcpy(unlink_data + 20, "\x00\x00\x00\x01", 4);
			memcpy(unlink_data + 24, data->buf + 24, 24);

			stream->line[port_num - 1].unlink_flag = 1;

			//stream->line[port_num - 1].unlink_num++;
			//sleep(2);//等待线程处理
			//reload_devh(stream, port_num - 1);
			if (send(src_line, unlink_data, 48, MSG_NOSIGNAL)<0)
			{
				if(NULL != usbip_data)
				{
					free(usbip_data);
				}
				return -1;
			}
			ret = 0;
			goto end;
		}
		else
		{
			tran_data_out("unlink too many times!\n");
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -1;
		}
	}
	if ((len + 48)>sizeof(data->buf))
	{
		tran_data_out("oper_data,err need relloc mem:%d\n", len);
		
		//printf_array((char *)data->buf,48);

		//if (data->buf[15] == USBIP_DIR_OUT && ((transfer_flags & URB_ZERO_PACKET) != URB_ZERO_PACKET) && data->buf[19] != 0x00)//标示out，往设备写入设备,且不是0字节包,否则则无urb数据
		if (data->buf[15] == USBIP_DIR_OUT && ((transfer_flags & URB_ZERO_PACKET) != URB_ZERO_PACKET))//某一个种北京ca只有控制传输0端点 
		{
			{ tran_data_out("oper_data,需申请内存\n");
			if (data->re_buf == NULL)
			{
				data->re_buf = malloc(len + 48);
				if (data->re_buf == NULL)
				{
					if(NULL != usbip_data)
					{
						free(usbip_data);
					}
					return -3;
				}
				memset(data->re_buf, 0, len + 48);
				memcpy(data->re_buf, data->buf, data->buf_len);
			}
			buf_1 = data->re_buf;
			}
			goto oper_read;
		}
		//len = data->buf[47] * 0x100 + data->buf[46];
		else
		{
			data->buf_len = 48;
			printf("oper recv buflen = %d\r\n",data->buf_len);
			goto oper;
		}



		

		if (data->re_buf == NULL)
		{
			data->re_buf = malloc(len + 48);

			if (data->re_buf == NULL)
			{
				if(NULL != usbip_data)
				{
					free(usbip_data);
				}
				return -3;
			}
			memset(data->re_buf, 0, len + 48);
			memcpy(data->re_buf, data->buf, data->buf_len);
		}
		buf_1 = data->re_buf;
	}
oper_read:
	if (len>0)
	{
		// printf("!!!!!!!!!!!!!! oper_read,len = %d\r\n",len);
		len1 = recv(src_line, buf_1 + data->buf_len, len + 48 - data->buf_len, MSG_DONTWAIT);
		// printf("oper_read_recv_buflen = %d\r\n",len1);
		if ((len1 == 0) || (len1<-1))
		{
			tran_data_out("线路关闭oper_read\n");
			if (data->re_buf != NULL)
				free(data->re_buf);
			if(NULL != usbip_data)
			{
				free(usbip_data);
			}
			return -4;

		}
		else if (len1 == -1)
			len1 = 0;
		data->buf_len += len1;
	}
	if (data->buf_len != (len + 48))
	{
		//if (data->re_buf != NULL)
		//free(data->re_buf);//new add 0926
		if(NULL != usbip_data)
		{
			free(usbip_data);
		}
		return data->buf_len;
	}

oper:

	if ((buf_1[0] == 0x00) && (buf_1[1] == 0x00) && (buf_1[2] == 0x00) && (buf_1[3] == 0x05))
	{
		tran_data_out("心跳数据不处理直接结束\n");
		ret = 0;
		goto data_end;
	}
	// tran_data_out("判断该数据的传输端口\n");

	memset(usbip_data, 0, TRAN_DATA_LEN);
	//if (stream->line[port_num - 1].vid == 0X096e && stream->line[port_num - 1].pid == 0X070b)
	// lbc_count++;
	get_ep_direction((char *)buf_1, &ep, &direction);
	//if(ep > 0)
	// {
	// 	printf("lbc ---------------,ep = %.02x,direction = %d\n",ep,direction);
	// 	printf_array((unsigned char *)buf_1, data->buf_len);
	// }
	
	//while(break_flag)
	{
		tran_data_out("获取到的ep = %02x,direction = %d,data->buf_len = %d\n",ep,direction,data->buf_len);
		if (direction == USBIP_DIR_IN && data->buf_len > 48 && special_handle_flag == 1)//in的方向时却有数据，说明in的包里面合并了out包,需要处理
		{
			special_handle_flag = 0;
			memcpy(read_cmd_data,buf_1,48);   //保存真正有用的读取数据的usbip的头
			buf_1 += 48;					  //偏移到对应的数据段
			//data_num = *buf_1;                //数据条目数
			// total_len = *(buf_1+1);
			// total_len = total_len<<8 | *(buf_1+2);  //数据总长度
			// printf("\n\ndata_num = %d,total_len = %d,data->buf_len = %d\n\n",data_num,total_len,data->buf_len);
			// if(total_len != data->buf_len - 48 - 3)
			// {
			// 	printf("len is not right !!!!!!!!!!!!!!!!!!!!!\n");
			// 	goto data_end;
			// }
			// buf_1 += 3;
			for(data_num_count = 0;data_num_count<data_num;++data_num_count)
			{
				buf_len = *buf_1;		//数据长度
				buf_len = buf_len << 8 | (*(buf_1 + 1));
				if(buf_len < 48)
				{
					printf("buf_len is not right !!!!!!!!!!!!!!!!!!!!!\n");
					goto data_end;
				}
				buf_1 += 2;				//偏移到数据段
				get_ep_direction((char *)buf_1, &ep, &direction);
				tran_data_out("循环中,获取到的ep = %02x,direction = %d,buf_len = %d\n",ep,direction,buf_len);
				if (ep != 0)
				{
					if (direction == USBIP_DIR_IN)//in
					{
						ep = ep | 0x80;
						stream->line[port_num - 1].in_ep = ep;
					}
					else
					{
						ep = ep;
						stream->line[port_num - 1].out_ep = ep;
					}
				}
				if (ep == 0)
				{
					tran_data_out("此帧传输为控制传输_1\n");
					result = control_transfer(stream, port_num, (unsigned char *)buf_1, buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
					if (result < 0)
					{
						ret = -1;
					}
					goto end;
				}
				else
				{
					//tran_data_out("非控制传输筛选:ep:%02x\n",ep);
					int i;
					int j;
					int find = 0;
					for (i = 0; i < 3 && find == 0; i++)
					{
						if (stream->line[port_num - 1].interface_decs[i].state == 1)
						{
							for (j = 0; j < 3; j++)
							{
								if (stream->line[port_num - 1].interface_decs[i].point_decs[j].state == 1 && ep == stream->line[port_num - 1].interface_decs[i].point_decs[j].bEndpointAddress)
								{
									stream->line[port_num - 1].tran_type = stream->line[port_num - 1].interface_decs[i].point_decs[j].bmAttributes & 0x03;		//非控制传输获取传输方式
									stream->line[port_num - 1].in_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
									stream->line[port_num - 1].out_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
									if (stream->line[port_num - 1].interrupt_in_maxlen == 0x00 && stream->line[port_num - 1].tran_type == 0x03)					//中断传输包最大值未赋值，初始状态
									{
										stream->line[port_num - 1].interrupt_in_maxlen = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
									}
									find = 1;
									break;
								}
							}// end j for
						}
					}//end i for
					if (find == 0)
					{
						tran_data_out("未找到匹配的端点：ep:%02x \n", ep);
						ret = -1;
						goto end;
					}
					// printf("lbc 11111111111111111111 find interface i = %d\n",i);
					// libusb_claim_interface(stream->line[port_num - 1].devh,i);
				}//end else
				if (ep == 0)
				{
					tran_data_out("此帧传输为控制传输_2\n");
					result = control_transfer(stream, port_num, (unsigned char *)buf_1, buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
					if (result < 0)
					{
						ret = -1;
					}
				}
				else
				{
					tran_data_out("端点非0，需判断传输类型\n");
					switch (stream->line[port_num - 1].tran_type)
					{
					case 0:
						tran_data_out("传输类型为控制传输\n");
						result = control_transfer(stream, port_num, (unsigned char *)buf_1, buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
						if (result < 0)
						{
							tran_data_out("非0端点控制传输失败\n");
							ret = -11;
						}
						break;
					case 1:
						tran_data_out("传输类型为同步传输\n");
						ret = -12;
						break;
					case 2:
						tran_data_out("传输类型为块传输\n");
						stream->line[port_num - 1].end_flag = 0;
						// printf("000000 end_flag = %x\n",&end_flag);
						result = bulk_transfer(stream, port_num, (unsigned char *)buf_1, buf_len, (unsigned char *)usbip_data, &usbip_len, src_line,0,&end_flag);
						if (result < 0)
						{
							tran_data_out("非0端点块传输失败\n");
							ret = -13;
						}
						break;
					case 3:
						tran_data_out("传输类型为中断传输\n");
						result = interrupt_transfer(stream, port_num, (unsigned char *)buf_1, buf_len, (unsigned char *)usbip_data, &usbip_len, src_line,0,&end_flag);
						if (result < 0)
						{
							tran_data_out("非0端点控制中断传输\n");
							ret = -14;
						}
						break;
					default:
						tran_data_out("传输类型错误\n");
						ret = -15;
						break;
					}
				}
				//需要等待上一帧数据结束
				// printf("stream->line[port_num - 1].end_flag = %d\n",stream->line[port_num - 1].end_flag);
				if(stream->line[port_num - 1].pid != 0x5477 || stream->line[port_num - 1].vid != 0x163c)
				{
					while(stream->line[port_num - 1].end_flag == 0)
					{
						usleep(1000);
					}
				}
				stream->line[port_num - 1].end_flag = 0;
				buf_1 += buf_len;
			}

			get_ep_direction((char *)read_cmd_data, &ep, &direction);
			tran_data_out("获取到的ep = %02x,direction = %d\n",ep,direction);
			if (ep != 0)
			{
				if (direction == USBIP_DIR_IN)//in
				{
					ep = ep | 0x80;
					stream->line[port_num - 1].in_ep = ep;
				}
				else
				{
					ep = ep;
					stream->line[port_num - 1].out_ep = ep;
				}
			}
			if (ep == 0)
			{
				tran_data_out("此帧传输为控制传输_1\n");
				result = control_transfer(stream, port_num, (unsigned char *)read_cmd_data, 48, (unsigned char *)usbip_data, &usbip_len, src_line);
				if (result < 0)
				{
					ret = -1;
				}
				goto end;
			}
			else
			{
				//tran_data_out("非控制传输筛选:ep:%02x\n",ep);
				int i;
				int j;
				int find = 0;
				for (i = 0; i < 3 && find == 0; i++)
				{
					if (stream->line[port_num - 1].interface_decs[i].state == 1)
					{
						for (j = 0; j < 3; j++)
						{
							if (stream->line[port_num - 1].interface_decs[i].point_decs[j].state == 1 && ep == stream->line[port_num - 1].interface_decs[i].point_decs[j].bEndpointAddress)
							{
								stream->line[port_num - 1].tran_type = stream->line[port_num - 1].interface_decs[i].point_decs[j].bmAttributes & 0x03;		//非控制传输获取传输方式
								stream->line[port_num - 1].in_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								stream->line[port_num - 1].out_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								if (stream->line[port_num - 1].interrupt_in_maxlen == 0x00 && stream->line[port_num - 1].tran_type == 0x03)					//中断传输包最大值未赋值，初始状态
								{
									stream->line[port_num - 1].interrupt_in_maxlen = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								}
								find = 1;
								break;
							}
						}// end j for
					}
				}//end i for
				if (find == 0)
				{
					tran_data_out("未找到匹配的端点：ep:%02x \n", ep);
					ret = -1;
					goto end;
				}
				// libusb_claim_interface(stream->line[port_num - 1].devh,i+1);
			}//end else
			if (ep == 0)
			{
				tran_data_out("此帧传输为控制传输_2\n");
				result = control_transfer(stream, port_num, (unsigned char *)read_cmd_data, 48, (unsigned char *)usbip_data, &usbip_len, src_line);
				if (result < 0)
				{
					ret = -1;
				}
			}
			else
			{
				tran_data_out("端点非0，需判断传输类型\n");
				switch (stream->line[port_num - 1].tran_type)
				{
				case 0:
					tran_data_out("传输类型为控制传输\n");
					result = control_transfer(stream, port_num, (unsigned char *)read_cmd_data, 48, (unsigned char *)usbip_data, &usbip_len, src_line);
					if (result < 0)
					{
						tran_data_out("非0端点控制传输失败\n");
						ret = -11;
					}
					break;
				case 1:
					tran_data_out("传输类型为同步传输\n");
					ret = -12;
					break;
				case 2:
					tran_data_out("传输类型为块传输\n");
					result = bulk_transfer(stream, port_num, (unsigned char *)read_cmd_data, 48, (unsigned char *)usbip_data, &usbip_len, src_line,1,&end_flag);
					if (result < 0)
					{
						tran_data_out("非0端点块传输失败\n");
						ret = -13;
					}
					break;
				case 3:
					tran_data_out("传输类型为中断传输\n");
					result = interrupt_transfer(stream, port_num, (unsigned char *)read_cmd_data, 48, (unsigned char *)usbip_data, &usbip_len, src_line,1,&end_flag);
					if (result < 0)
					{
						tran_data_out("非0端点控制中断传输\n");
						ret = -14;
					}
					break;
				default:
					tran_data_out("传输类型错误\n");
					ret = -15;
					break;
				}
			}
		}
		else   //兼容之前的协议处理
		{
			break_flag = 0;
			if (ep != 0)
			{
				if (direction == USBIP_DIR_IN)//in
				{
					ep = ep | 0x80;
					stream->line[port_num - 1].in_ep = ep;
				}
				else
				{
					ep = ep;
					stream->line[port_num - 1].out_ep = ep;
				}
			}
			if (ep == 0)
			{
				tran_data_out("此帧传输为控制传输_1\n");
				result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
				if (result < 0)
				{
					ret = -1;
				}
				goto end;
			}
			else
			{
				//tran_data_out("非控制传输筛选:ep:%02x\n",ep);
				int i;
				int j;
				int find = 0;
				for (i = 0; i < 3 && find == 0; i++)
				{
					if (stream->line[port_num - 1].interface_decs[i].state == 1)
					{
						for (j = 0; j < 3; j++)
						{
							if (stream->line[port_num - 1].interface_decs[i].point_decs[j].state == 1 && ep == stream->line[port_num - 1].interface_decs[i].point_decs[j].bEndpointAddress)
							{
								stream->line[port_num - 1].tran_type = stream->line[port_num - 1].interface_decs[i].point_decs[j].bmAttributes & 0x03;		//非控制传输获取传输方式
								stream->line[port_num - 1].in_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								stream->line[port_num - 1].out_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								if (stream->line[port_num - 1].interrupt_in_maxlen == 0x00 && stream->line[port_num - 1].tran_type == 0x03)					//中断传输包最大值未赋值，初始状态
								{
									stream->line[port_num - 1].interrupt_in_maxlen = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
								}
								find = 1;
								break;
							}
						}// end j for
					}
				}//end i for
				if (find == 0)
				{
					tran_data_out("未找到匹配的端点：ep:%02x \n", ep);
					ret = -1;
					goto end;
				}
				// libusb_claim_interface(stream->line[port_num - 1].devh,i+1);
			}//end else
			if (ep == 0)
			{
				tran_data_out("此帧传输为控制传输_2\n");
				result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
				if (result < 0)
				{
					ret = -1;
				}
			}
			else
			{
				tran_data_out("端点非0，需判断传输类型\n");
				switch (stream->line[port_num - 1].tran_type)
				{
				case 0:
					tran_data_out("传输类型为控制传输\n");
					result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
					if (result < 0)
					{
						tran_data_out("非0端点控制传输失败\n");
						ret = -11;
					}
					break;
				case 1:
					tran_data_out("传输类型为同步传输\n");
					ret = -12;
					break;
				case 2:
					tran_data_out("传输类型为块传输\n");
					// printf("00000000 end_flag = %x\n",&end_flag);
					result = bulk_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line,1,&end_flag);
					if (result < 0)
					{
						tran_data_out("非0端点块传输失败\n");
						ret = -13;
					}
					break;
				case 3:
					tran_data_out("传输类型为中断传输\n");
					result = interrupt_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line,1,&end_flag);
					if (result < 0)
					{
						tran_data_out("非0端点控制中断传输\n");
						ret = -14;
					}
					break;
				default:
					tran_data_out("传输类型错误\n");
					ret = -15;
					break;
				}
			}
		}
	}
	if(NULL != usbip_data)
	{
		free(usbip_data);
		usbip_data = NULL;
	}
end:
	memset(seqnum, 0, sizeof(seqnum));
	memcpy(seqnum, buf_1 + 4, 4);
	memset(devid, 0, sizeof(devid));
	memcpy(devid, buf_1 + 8, 4);
	if (data->re_buf != NULL)
	{
		free(data->re_buf);
	}
	if(NULL != usbip_data)
	{
		free(usbip_data);
		usbip_data = NULL;
	}
data_end:
	if(NULL != usbip_data)
	{
		free(usbip_data);
		usbip_data = NULL;
	}
	memset(data, 0, sizeof(struct _train_data_oper));
	return ret;
}








static int get_head_size(uint8 *buf)
{
	struct usbip_header	u;
	int out_len, iso_len;
	memset(&u, 0, sizeof(u));
	memcpy(&u, buf, sizeof(u));
	if (!u.base.direction)
		out_len = ntohl(u.u.cmd_submit.transfer_buffer_length);
	else
		out_len = 0;
	if (u.u.cmd_submit.number_of_packets)
		iso_len = sizeof(struct usbip_iso_packet_descriptor)*ntohl(u.u.cmd_submit.number_of_packets);
	else
		iso_len = 0;
	return out_len + iso_len;
}

static int get_inlen_par(uint8 *buf1, long *out_q_seqnum_array)
{
	return 0;
}








static void line_task(void *arg, int o_timer)
{
	char c;
	struct _transmit_sign *transmit_sign;
	transmit_sign = arg;
	recv(transmit_sign->socket_fd, &c, 1, MSG_PEEK);
	sem_post(transmit_sign->sem);
}





/*======================================命令执行===============================================*/
static int tran_data_close_tran_task(struct _tran_data_fd *id, va_list args)
{
	struct _tran_data        *stream;
	int port;
	int *flag_ptr = NULL;
	int count;
	stream = id->dev;
	port = va_arg(args, int);
	flag_ptr = va_arg(args, int *);
	tran_data_out("关闭终端和中间件线程,port = %d\n", port);
	if (stream->line[port - 1].state_mem == 0)
	{
		//printf("(stream->line[port - 1].state_mem == 0)\r\n");
		return 0;
	}
	_lock_set(stream->lock);
	stream->line[port - 1].out_close = 1;
	_lock_un(stream->lock);
	if(stream->line[port - 1].sem != NULL)
		sem_post(stream->line[port - 1].sem);

	for (; stream->line[port - 1].state_mem != 0; usleep(10000))
	{
		count++;
		_lock_set(stream->lock);
		stream->line[port - 1].out_close = 1;
		_lock_un(stream->lock);
		if (count == 100 * 10)//10 S
		{
			//write_log("超过10S都未成功关闭%d端口线路\n", port);
			printf("tran_data_close_tran_task3333333333333\r\n");
			break;
		}
	}
	*flag_ptr = 1;
	//tran_data_out("tran_data_close_tran_task 关闭成功返回\r\n");
	return 0;
}


static int tran_data_lock_all_line(struct _tran_data_fd *id, va_list args)
{
	struct _tran_data        *stream;
	int lock_type;
	int i;
	stream = id->dev;
	lock_type = va_arg(args, int);
	if (lock_type == 1)
	{
		for (i = 0; i<sizeof(stream->line) / sizeof(stream->line[0]); i++)
		{
			_lock_set(stream->line[i].lock);

		}
	}
	else if (lock_type == 0)
	{
		for (i = 0; i<sizeof(stream->line) / sizeof(stream->line[0]); i++)
		{
			_lock_un(stream->line[i].lock);

		}
	}
	return 0;
}


static int tran_data_check_line_status(struct _tran_data_fd *id, va_list args)
{
	struct _tran_data        *stream;
	int port;
	//int i;
	stream = id->dev;
	
	port = va_arg(args, int);
	// printf("tran_data_check_line_status port = %d\n",port);
	if (stream->line[port - 1].state_mem == 0)
		return -1;
	return 0;
}
/*======================================底层打包==================================================*/

static uint32 recv_midware_ver_code(int fd)
{
	int result;
	uint32 ver;
	uint8 ver_code[4];
	memset(ver_code, 0, sizeof(ver_code));
	result = ty_socket_read(fd, ver_code, sizeof(ver_code));
	if (result<0)
	{
		tran_data_out("111读文件失败\n");
		return result;
	}
	ver = (uint32)(ver_code[0] * 0x1000000) + (uint32)(ver_code[1] * 0x10000) + (uint32)(ver_code[2] * 0x100) + (uint32)ver_code[3];
	return ver;
}







//////////////////////////////////////////////USBIP相关函数//////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


//static int record_out_l(unsigned long num, long *out_q_seqnum_array)
//{
//	int i;
//	for (i = 0; i<OUT_Q_LEN; i++)
//	{
//		if (out_q_seqnum_array[i])
//			continue;
//		out_q_seqnum_array[i] = num;
//		quickSort((int *)out_q_seqnum_array, 0, OUT_Q_LEN - 1);
//		return 1;
//	}
//	return 0;
//}

//static int check_out(unsigned long num, long *out_q_seqnum_array)
//{
//	int i;
//	for (i = 0; i<OUT_Q_LEN; i++)
//	{
//		if (out_q_seqnum_array[i] != num)
//			continue;
//		out_q_seqnum_array[i] = 0;
//		return 1;
//	}
//	return 0;
//}
//static int get_out(unsigned long *num, long *out_q_seqnum_array)
//{
//	int i;
//	//tran_data_out("seqnum 数组：");
//	//for (i = 0; i < 25; i++)
//	//{
//	//	printf("%02x ", (unsigned char )out_q_seqnum_array[i]);
//	//}
//	//printf("\n");
//	for (i = 0; i<OUT_Q_LEN; i++)
//	{
//		if (out_q_seqnum_array[i] != 0)
//		{
//			*num = out_q_seqnum_array[i];
//			return 0;
//		}
//	}
//	return -1;
//}
void quickSort(int* arr, int startPos, int endPos)
{
	int i, j;
	int key;
	key = arr[startPos];
	i = startPos;
	j = endPos;
	while (i<j)
	{
		while (arr[j] >= key && i<j)--j; //————1 从后往去前扫，直到找到一个a[j]<key或遍历完
		arr[i] = arr[j];
		while (arr[i] <= key && i<j)++i; //————2 从后往去前扫，直到找到一个a[i]>key或遍历完
		arr[j] = arr[i];
	}
	arr[i] = key;
	if (i - 1>startPos) quickSort(arr, startPos, i - 1); //————1 如果key前还有两个及以上的数，排key前的数（有一个的话自然就不用排了）
	if (endPos>i + 1) quickSort(arr, i + 1, endPos);//————2 如果key后还有两个及以上的数，排key后的数
}





/*==================================socket 发送和接受===============================================*/
/*==================================================================================================*/
static int ty_socket_read(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{
			tran_data_out("read data time tran_data_out or err \n");
			return result;
		}
	}
	return i;
}

static int ty_socket_write(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{   //result=write(sock,buf+i,buf_len-i);
		result = send(sock, buf + i, buf_len - i, MSG_NOSIGNAL);
		if (result<0)
			return result;
	}
	return i;
}

/*===================================================================================================*/
////////////////////////////////////////////////libusb/////////////////////////////////////////////////
/*===================================================================================================*/

static libusb_device *FindDeviceByID(libusb_device **devs, int nBusNum, int nDevNum)
{
	libusb_device *dev = NULL;
	libusb_device *devRet = NULL;
	int i = 0;
	while ((dev = devs[i++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);			//获取USB设备的设备描述符	//dev libusb_device的指针，是要读取的设备 desc 设备描述符的指针，用来带回设备描述符的结构
		if (r < 0)
			continue;
		if (libusb_get_bus_number(dev) == nBusNum && libusb_get_device_address(dev) == nDevNum)		//判断总线号和设备号
		{
			devRet = dev;
			break;
		}
	}
	return devRet;
}

static int usb_dev_init(struct _tran_data	*stream, int port_num, uint8 *busid)
{
	int ret = 0;
	int out_max_len = 1024;
	int in_max_len = 1024;
	//int bDescriptorType = 0;
	//int bmAttributes = 0;
	int bInterfaceNumber = 0;
	int wToalLength;
	libusb_device **devs = NULL;
	struct libusb_device_handle *devh = NULL;
	//struct libusb_device_descriptor desc;
	int busnum;
	int devnum;

	//tran_data_out("初始化动态库成功\n");
	//tran_data_out("初始化动态库成功,stream->line[port_num - 1].context = %08x\n", stream->line[port_num - 1].context);
	if (libusb_get_device_list(NULL, &devs) < 0)
	{
		tran_data_out("libusb_get_device_list error\n");
		return -1;
	}
	busnum = get_busid_busnum((char *)busid);
	devnum = get_busid_devnum((char *)busid);
	//tran_data_out("BUSID : %s,总线号为：%d,设备号为：%d\n", busid, busnum, devnum);
	libusb_device *dev = FindDeviceByID(devs, busnum, devnum);
	if (dev == NULL)
	{
		tran_data_out("FindDeviceByID error\n");
		return -2;
	}
	//tran_data_out("开启libusb_open\n");
	if (libusb_open(dev, &devh) < 0)		//通过libusb_device的指针打开一个USB设备，并返回设备句柄libusb_device_handle(devh)的指针
	{
		//tran_data_out("ret = %d\n", ret);
		return -3;
	}


	//tran_data_out("开启libusb_open成功\n");
	if (libusb_kernel_driver_active(devh, 0))			//确定指定接口的内核驱动程序是否已经激活。如果一个内核驱动程序是激活的，libusb_claim_interface调用的会失败
	{
		if (libusb_detach_kernel_driver(devh, 0) < 0)	//卸载指定接口的内核驱动程序。如果一个内核驱动程序是激活的，必须先调用这个函数，再调用libusb_claim_interface
		{
			return -4;
		}
	}


	if (libusb_kernel_driver_active(devh, 1))			//对双接口设备的兼容
	{
		if (libusb_detach_kernel_driver(devh, 1) < 0)
		{
			return -4;
		}
	}

	if (libusb_kernel_driver_active(devh, 2))			//对双接口设备的兼容
	{
		if (libusb_detach_kernel_driver(devh, 2) < 0)
		{
			return -4;
		}
	}

	//tran_data_out("卸载内核驱动成功\n");
	/*
	 * \param dev_handle a handle of the device to reset
	 * \returns 0 on success
	 * \returns LIBUSB_ERROR_NOT_FOUND(-5) if re-enumeration is required, or if the
	 * device has been disconnected
	 * \returns another LIBUSB_ERROR code on other failure
	 */

	if(devh == NULL)
	{
		tran_data_out("libusb_reset_device devh == NULL\n");
	}
	ret = libusb_reset_device(devh);
	if (ret < 0)
	{
		usleep(50000);								//wang 
		tran_data_out("libusb_reset_device复位失败_1,ret = %d\n",ret);
		if (libusb_reset_device(devh) < 0)
		{
			tran_data_out("libusb_reset_device复位失败_11,ret = %d\n",ret);
			return -6;
		}
	}

	//tran_data_out("复位成功port= %d\n", port_num);
	////usb_claim_interface(devh, 0);
	////GetEndPointAddress(devh);

	if (libusb_claim_interface(devh, 0) < 0)		//为指定的设备申请接口	devh：句柄  bInterfaceNumber：对应接口描述符
	{
		return -5;
	}


	stream->line[port_num - 1].devh = devh;
	unsigned char usb_data[2048];
	int outpoint = 0, inpoint = 0;
	int i;

	memset(usb_data, 0, sizeof(usb_data));
	//tran_data_out("获取设备描述符\n");
	//memcpy(usb_data, "\x80\x06\x00\x01\x00\x00\x12\x00", 8);
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	{
		tran_data_out("获取设备描述符失败\n");
	}
	tran_data_out("打印设备描述符\n");
	printf_array((char *)usb_data, 0x12);
	//tran_data_out("获取设备描述符成功\n");
	//tran_data_out("获取配置描述符\n");
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, 0x09, 3000) < 0)
	{
		tran_data_out("第一次获取配置描述符失败\n");
	}
	tran_data_out("打印获取配置描述符\n");

	printf_array((char *)usb_data, 0x09);
	//tran_data_out("获取配置描述符成功\n");
	wToalLength = usb_data[2];
	//tran_data_out("获取到配置描述符总长度为：%d\n", wToalLength);
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, wToalLength, 3000) < 0)
	{
		tran_data_out("第二次获取配置描述符失败\n");
	}
	tran_data_out("打印获取到的描述符\n");

	printf_array((char *)usb_data, wToalLength);
	int count = 0;
	for (i = 0; i < wToalLength; i++)
	{
		if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
		{
			bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bNumEndpoints = usb_data[i + 4];
			//tran_data_out("获取到的端点号 bInterfaceNumber 为：%02x,bNumEndpoints 为：%02x\n", bInterfaceNumber, stream->line[port_num - 1].bNumEndpoints);
			if (libusb_claim_interface(devh, bInterfaceNumber) < 0)			//为指定的设备申请接口
			{
				return -7;
			}
			count++;
		}
	}
	stream->line[port_num - 1].AllInterfaceNumber = count;
	int i_count = 0;
	//if (stream->line[port_num - 1].vid == 0x096e && stream->line[port_num - 1].pid == 0x070b)//目前只针对平安银行的key，后面改成通用的
	{
		//09 02 32 00 
		tran_data_out("解析设备\n");
		for (i = 0; i < wToalLength; i++)
		{
			if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x02)))//09 02
			{
				if ((usb_data[i + 4] == 0x02))//标示为双接口//02 01 00 80 32 09 04 00 00 00 03 00 00 00 09 21 10 01 00 22 96 01 09 04 01 00 02 08 06 50 00 07 05 83 02 40 00 00 07 05 04 02 40 00 00
				{
					i += 4;//01 00 80 32 09 04 00 00 00 03 00 00 00 09 21 10 01 00 22 96 01 09 04 01 00 02 08 06 50 00 07 05 83 02 40 00 00 07 05 04 02 40 00 00
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))//09 04 01 00 02 08 06 50 00 07 05 83 02 40 00 00 07 05 04 02 40 00 00
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];

							tran_data_out("找到第【%d】个接口--该接口端点0以外的端点数【%d】\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//偏移掉接口描述符到达端点描述符
							tran_data_out("现在的位置:【%d】--值:%02x\n", i, usb_data[i]);
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//找到一个端点描述符

								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								//		p_count++;
								//		i += 5;
								//		i_count++;
								//		break;
								//	}
								//	else
								//		i++;
								//}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)
							{
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)
							{

								//usb_data[i] == 0X07; usb_data[i+1] == 0X05;
								while (i < wToalLength)//08 06 50 00 07 05 83 02 40 00 00 07 05 04 02 40 00 00
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到第一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//记录第一个接口的端点0以外的端点数

						}
					}
				}
				else if ((usb_data[i + 4] == 0x01))//标示为单接口
				{
					tran_data_out("单接口设备!\n");
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))//接口描述符
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)//端点0以外的其他端点数为0,则只有控制传输
							{
								tran_data_out("只存在控制传输!\n");
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = 0x00;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = 0;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = 64;
								//stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								i_count++;
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)//端点0以外的其他端点数为1
							{
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										tran_data_out("1端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)//端点0以外的其他端点数为2
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("2端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("3端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 3)//端点0以外的其他端点数为3
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("4端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("5端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);

										p_count++;
										i += 7;


										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 7;
										tran_data_out("6端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}

						}
					}
				}
				else if ((usb_data[i + 4] == 0x03))//标示为三接口
				{
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];

							tran_data_out("找到第【%d】个接口--该接口端点0以外的端点数【%d】\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//偏移掉接口描述符到达端点描述符
							tran_data_out("现在的位置:【%d】--值:%02x\n", i, usb_data[i]);

							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//找到一个端点描述符

								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								//		p_count++;
								//		i += 5;
								//		i_count++;
								//		break;
								//	}
								//	else
								//		i++;
								//}
								continue;

							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)
							{

								//usb_data[i] == 0X07; usb_data[i+1] == 0X05;
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到第一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//记录第一个接口的端点0以外的端点数
						}
					}
				}
			}
		}
	}
	stream->line[port_num - 1].out_ep = outpoint;
	stream->line[port_num - 1].in_ep = 0x80 | inpoint;
	stream->line[port_num - 1].out_max_len = out_max_len;
	stream->line[port_num - 1].in_max_len = in_max_len;

	libusb_free_device_list(devs, 1);							//释放以前使用的设备列表 list 要释放的设备列表的指针  unref_devices 如果该参数置1 列表中的每个设备的引用计数减1

	return 0;
}

static int get_ep_direction(char *buf, int *ep, int *direction)
{
	*direction = buf[15];
	//tran_data_out("direction = %02x\n", *direction);

	*ep = buf[19];
	//tran_data_out("ep = %02x\n", *ep);
	return 0;
}
int usb_dev_init_new(struct _tran_data	*stream, int port_num, uint8 *busid)
{
	int out_max_len = 1024;
	int in_max_len = 1024;
	//int bDescriptorType = 0;
	int bmAttributes = 0;
	int bInterfaceNumber = 0;
	int wToalLength;
	libusb_device **devs = NULL;
	struct libusb_device_handle *devh = NULL;
	//struct libusb_device_descriptor desc;
	int busnum;
	int devnum;

	tran_data_out("usb_dev_init_new 初始化动态库成功\n");
	if (libusb_get_device_list(NULL, &devs) < 0)			//usb lib库函数 获取当前的设备列表,devs:USB设备列表
	{
		tran_data_out("libusb_get_device_list error\n");
		return -1;
	}
	busnum = get_busid_busnum((char *)busid);				//?busnum: 总线号 devnum: 设备号
	devnum = get_busid_devnum((char *)busid);
	tran_data_out("BUSID : %s,总线号为：%d,设备号为：%d\n", busid, busnum, devnum);
	libusb_device *dev = FindDeviceByID(devs, busnum, devnum);
	if (dev == NULL)
	{
		tran_data_out("FindDeviceByID error\n");
		return -2;
	}
	//tran_data_out("开启libusb_open\n");
	if (libusb_open(dev, &devh) < 0)
	{
		//tran_data_out("ret = %d\n", ret);
		return -3;
	}
	//tran_data_out("开启libusb_open成功\n");
	if (libusb_kernel_driver_active(devh, 0))
	{
		if (libusb_detach_kernel_driver(devh, 0) < 0)
		{
			return -4;
		}
	}


	if (libusb_kernel_driver_active(devh, 1))//对双接口设备的兼容
	{
		if (libusb_detach_kernel_driver(devh, 1) < 0)
		{
			return -4;
		}
	}

	if (libusb_kernel_driver_active(devh, 2))//对双接口设备的兼容
	{
		if (libusb_detach_kernel_driver(devh, 2) < 0)
		{
			return -4;
		}
	}

	// if (libusb_kernel_driver_active(devh, 3))//对三接口设备的兼容
	// {
	// 	if (libusb_detach_kernel_driver(devh, 3) < 0)
	// 	{
	// 		tran_data_out("1111 三接口初始化失败\n");
	// 		return -4;
	// 	}
	// }

	//tran_data_out("卸载内核驱动成功\n");

	//tran_data_out("复位该USB设备\n");
	/*
	Perform an USB port reset for an usb device. Returns 0 on success, 
	LIBUSB_ERROR_NOT_FOUND if re-enumeration is required or if the device has been disconnected and a LIBUSB_ERROR code on failure.
	*/
	//可以具体看下返回的值是多少 wang 11/29
	if (libusb_reset_device(devh) < 0)
	{
		tran_data_out("libusb_reset_device复位失败2\n");
		return -6;
	}
	//tran_data_out("复位成功port= %d\n", port_num);
	////usb_claim_interface(devh, 0);
	////GetEndPointAddress(devh);


	if (libusb_claim_interface(devh, 0) < 0)
	{
		return -5;
	}


	stream->line[port_num - 1].devh = devh;
	unsigned char usb_data[2048];
	int outpoint = 0, inpoint = 0;
	int i;

	memset(usb_data, 0, sizeof(usb_data));
	//tran_data_out("获取设备描述符\n");
	//memcpy(usb_data, "\x80\x06\x00\x01\x00\x00\x12\x00", 8);
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	{
		tran_data_out("获取设备描述符失败\n");
	}
	//tran_data_out("获取设备描述符成功\n");
	//tran_data_out("获取配置描述符\n");
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, 0x09, 3000) < 0)
	{
		tran_data_out("第一次获取配置描述符失败\n");
	}
	//tran_data_out("获取配置描述符成功\n");
	wToalLength = usb_data[2];
	//tran_data_out("获取到配置描述符总长度为：%d\n", wToalLength);
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, wToalLength, 3000) < 0)
	{
		tran_data_out("第二次获取配置描述符失败\n");
		return -8;
	}
	tran_data_out("打印获取到的描述符\n");
	// int i;
	for (i = 0; i < wToalLength; i++)
	{
		printf("%02x ", (unsigned char)usb_data[i]);
	}
	printf("\n");
	int count = 0;
	for (i = 0; i < wToalLength; i++)
	{
		if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
		{
			bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bNumEndpoints = usb_data[i + 4];
			tran_data_out("获取到的端点号 bInterfaceNumber 为：%02x,bNumEndpoints 为：%02x\n", bInterfaceNumber, stream->line[port_num - 1].bNumEndpoints);
			if (libusb_claim_interface(devh, bInterfaceNumber) < 0)
			{
				return -7;
			}
			count++;
		}
	}
	stream->line[port_num - 1].AllInterfaceNumber = count;
	int i_count = 0;
	//if (stream->line[port_num - 1].vid == 0x096e && stream->line[port_num - 1].pid == 0x070b)//目前只针对平安银行的key，后面改成通用的
	{
		tran_data_out("解析设备\n");
		for (i = 0; i < wToalLength; i++)
		{
			if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x02)))
			{
				printf("1111111111 interfance = %d\n",usb_data[i + 4]);
				if ((usb_data[i + 4] == 0x02))//标示为双接口
				{
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];

							tran_data_out("找到第【%d】个接口--该接口端点0以外的端点数【%d】\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//偏移掉接口描述符到达端点描述符
							tran_data_out("现在的位置:【%d】--值:%02x\n", i, usb_data[i]);

							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//找到一个端点描述符

								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								//		p_count++;
								//		i += 5;
								//		i_count++;
								//		break;
								//	}
								//	else
								//		i++;
								//}
								continue;

							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)
							{

								//usb_data[i] == 0X07; usb_data[i+1] == 0X05;
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到第一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//记录第一个接口的端点0以外的端点数

						}
					}
				}
				else if ((usb_data[i + 4] == 0x03))//标示为三接口
				{
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];

							tran_data_out("找到第【%d】个接口--该接口端点0以外的端点数【%d】\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//偏移掉接口描述符到达端点描述符
							tran_data_out("现在的位置:【%d】--值:%02x\n", i, usb_data[i]);

							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//找到一个端点描述符

								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
								//		stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								//		p_count++;
								//		i += 5;
								//		i_count++;
								//		break;
								//	}
								//	else
								//		i++;
								//}
								continue;

							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)
							{

								//usb_data[i] == 0X07; usb_data[i+1] == 0X05;
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到第一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("第【%d】个接口--第【%d】个端点,端点号:【%02x】,端点特性:【%02x】,最大传输大小:【%d】\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//记录第一个接口的端点0以外的端点数
						}
					}
				}
				else if ((usb_data[i + 4] == 0x01))//标示为单接口
				{
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))//接口描述符
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)//端点0以外的其他端点数为0,则只有控制传输
							{
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = 0x00;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = 0;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = 64;
								//stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								i_count++;
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)//端点0以外的其他端点数为1
							{
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)//端点0以外的其他端点数为2
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										tran_data_out("端点特性为:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 3)//端点0以外的其他端点数为3
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//找到一个端点描述符

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("端点:%02x -特性为:%d - 传输长度:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										tran_data_out("端点:%02x -特性为:%d - 传输长度:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;


										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 7;
										tran_data_out("端点:%02x -特性为:%d - 传输长度:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}

						}
					}
				}

			}
		}
	}
	stream->line[port_num - 1].out_ep = outpoint;
	stream->line[port_num - 1].in_ep = 0x80 | inpoint;
	stream->line[port_num - 1].out_max_len = out_max_len;
	stream->line[port_num - 1].in_max_len = in_max_len;

	// bmAttributes = stream->line[port_num - 1].interface_decs[0].point_decs[0].bmAttributes;

	//tran_data_out("bmAttributes = %02x,stream->line[port_num - 1].bNumEndpoints=%d,stream->line[port_num - 1].tran_type=%d\n", bmAttributes,stream->line[port_num - 1].bNumEndpoints,stream->line[port_num - 1].tran_type);
	switch (bmAttributes & 0x03)
	{
	case 0x00:
		tran_data_out("传输类型为：控制传输\n");
		stream->line[port_num - 1].tran_type = 0x00;
		break;
	case 0x01:
		tran_data_out("传输类型为：同步传输,此类型不支持\n");
		stream->line[port_num - 1].tran_type = 0x01;
		break;
	case 0x02:
		tran_data_out("传输类型为：块传输\n");
		stream->line[port_num - 1].tran_type = 0x02;
		break;
	case 0x03:
		tran_data_out("传输类型为：中断传输\n");
		stream->line[port_num - 1].tran_type = 0x03;
		break;
	default:
		break;
	}

	tran_data_out("bmAttributes = %02x,stream->line[port_num - 1].bNumEndpoints=%d,stream->line[port_num - 1].tran_type=%d\n", bmAttributes,stream->line[port_num - 1].bNumEndpoints,stream->line[port_num - 1].tran_type);

	if ((stream->line[port_num - 1].tran_type == 0x03) && (stream->line[port_num - 1].bNumEndpoints == 1))
	{
		tran_data_out("该设备为仅用一个端点的中断传输类型设备，类似大明CA，强制修改IN端点最大数据长度\n");
		stream->line[port_num - 1].in_max_len = 1024;
	}
	libusb_free_device_list(devs, 1);


	return 0;
}


static int reload_devh(struct _tran_data*stream, int port_num)
{

	int result;

	int i = 0;
	stream->line[port_num].out_close = 2;
	if (stream->line[port_num].tran_type == 3)
	{
		tran_data_out("等待所有中断接收队列退出\n");
		for (i = 0; i<3000; i++, usleep(1000))
		{
			if (stream->line[port_num].queue_num == 0)
				break;
		}

	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_in_thread_num == 0)
		{
			tran_data_out("控制传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_out_thread_num == 0)
		{
			tran_data_out("控制传输out线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_in_thread_num == 0)
		{
			tran_data_out("块传输in线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_out_thread_num == 0)
		{
			tran_data_out("块传输out线程全部退出完毕\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_out_thread_num == 0)
		{
			tran_data_out("中断传输out线程全部退出完毕\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_in_thread_num == 0)
		{
			tran_data_out("中断传输in线程全部退出完毕\n");
			break;
		}
	}
	if (stream->line[port_num].bulk_in_thread_num != 0 || \
		stream->line[port_num].bulk_out_thread_num != 0 || \
		stream->line[port_num].control_in_thread_num != 0 || \
		stream->line[port_num].control_out_thread_num != 0 || \
		stream->line[port_num].interrupt_out_thread_num != 0 || \
		stream->line[port_num].interrupt_in_thread_num != 0)
	{
		write_log("重新加载devh时有线程未退出:bulk_in:%d\tbulk_out:%d\tctl_in:%d\tctrl_out:%d\t!\n", stream->line[port_num].bulk_in_thread_num, stream->line[port_num].bulk_out_thread_num, \
			stream->line[port_num].control_in_thread_num, stream->line[port_num].control_out_thread_num);
	}
	//等待线程退出
	if (stream->line[port_num].devh != NULL)
	{

		result = libusb_release_interface(stream->line[port_num].devh, 0);
		if (result < 0)
		{
			tran_data_out("libusb_release_interface result = %d\n", result);
		}

		if (stream->line[port_num].interface_decs[1].state != 0)//表示为双接口设备
		{
			result = libusb_release_interface(stream->line[port_num].devh, 1);
		}
	}
	for (i = 0; i < 2; i++)
		memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));

	tran_data_out("reload_devh 关闭句柄重新打开\n");
	if (stream->line[port_num].devh != NULL)
	{
		tran_data_out("reload_devh结束usbshare使用关闭%d号端口句柄\n", port_num+1);
		libusb_close(stream->line[port_num].devh);
		stream->line[port_num].devh = NULL;
	}
	stream->line[port_num].out_close = 0;
	tran_data_out("重载需要重新获取句柄\n");
	result = usb_dev_init_new(stream, port_num + 1, stream->line[port_num].busid);

	//tran_data_out("reload_devh传输类型为：%d，in_ep = %02x out_ep = %02x,in最大数据帧长度= %d,out最大数据帧长度= %d\n", stream->line[port_num].tran_type, stream->line[port_num].in_ep, stream->line[port_num].out_ep, stream->line[port_num].in_max_len, stream->line[port_num].out_max_len);
	tran_data_out("unlink !reload dev success!\n");
	return result;

}

static void bulk_data_to_dev_out(void *arg)
{
	struct _bulk_info *line;
	line = (struct _bulk_info *) arg;
	unsigned char out_buf[1000] = { 0 };
	int result;
	int size;
	int *thread_num;
	int port_num;
	port_num = line->port_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	int retry_num = 0;
	for (;;)
	{
		if (*line->out_close != 0)
		{
			tran_data_out("块传输线程外部要求退出!\n");
			_lock_set(bulk_lock);
			if (line->bulk_in_data != NULL)
			{
				free(line->bulk_in_data);
				line->bulk_in_data = NULL;
			}
			*(line->end_flag) = 1;
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			return;
		}
		if (*line->devh == NULL)//
		{

			write_log("bulk_data_to_dev_out 错误devh已经释放 enter1\n");
			_lock_set(bulk_lock);
			if (line->bulk_in_data != NULL)
			{
				free(line->bulk_in_data);
				line->bulk_in_data = NULL;
			}
			*(line->end_flag) = 1;
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			
			return;//break->return 10 29 change
		}

		if (*line->unlink_flag == 1)
		{
			tran_data_out("bulk_data_to_dev_out unlink!\n");
			break;//公网加载的时候发现可能存在unlink的情况
		}

		// count_1++;
		// printf("count_1 = %d\n",count_1);
		// printf("line->ep = %d,line->Wlength = %d\n",line->ep,line->Wlength);
		// printf_array((unsigned char *)line->bulk_in_data, line->Wlength);
		result = libusb_bulk_transfer(*line->devh, line->ep, line->bulk_in_data, line->Wlength, &size, 2000);
		if (result < 0)
		{
			printf("bulk_data_to_dev_out result = %d\n",result);
			if (*line->devh == NULL)
			{
				write_log("bulk_data_to_dev_out 错误devh已经释放\n");
				_lock_set(bulk_lock);
				if (line->bulk_in_data != NULL)
				{
					free(line->bulk_in_data);
					line->bulk_in_data = NULL;
				}
				*(line->end_flag) = 1;
				memset(line, 0, sizeof(struct _bulk_info));
				(*thread_num)--;
				_lock_un(bulk_lock);
				
				return;
			}
			tran_data_out("bulk tran_data_out err wait next -result:%d!\n", result);
			if (*line->unlink_flag == 1)
			{
				tran_data_out("bulk_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, line->ep);
				line->stream->line[port_num].unlink_flag = 0;
				break;//公网加载的时候发现可能存在unlink的情况
			}
			if (result != -7)
			{
				if (result == LIBUSB_ERROR_PIPE)//当为-9的时候表示此时这帧指令可能为错误的，如stall pid reset等，调试建行的一163C 0620设备时发现,但是调试农行的一个设备的时候要清除端点等待096e 0705
				{
					tran_data_out("块传输线程err pipe--wlength:%d -thread_num:%d!\n", line->Wlength, *thread_num);
					if (((line->vid == 0x163c) && (line->pid == 0x0620)) || ((line->vid == 0x163c) && (line->pid == 0x5462)))
					{
						libusb_clear_halt(*line->devh, line->ep);
						size = line->Wlength;
						break;
					}
					if (retry_num == 5)
					{
						_lock_set(bulk_lock);
						if (line->bulk_in_data != NULL)
						{
							free(line->bulk_in_data);
							line->bulk_in_data = NULL;
						}
						memset(line, 0, sizeof(struct _bulk_info));
						(*thread_num)--;
						_lock_un(bulk_lock);
						return;
					}
					libusb_clear_halt(*line->devh, line->ep);
					sleep(1);
					retry_num++;
					continue;

				}
				_lock_set(bulk_lock);
				if (line->bulk_in_data != NULL)
				{
					free(line->bulk_in_data);
					line->bulk_in_data = NULL;
				}
				*(line->end_flag) = 1;
				memset(line, 0, sizeof(struct _bulk_info));
				(*thread_num)--;
				_lock_un(bulk_lock);
				return;
			}
			continue;
		}
		else
		{
#ifdef BULK_OUT
			tran_data_out("块传输线程写入到数据长度为:%d\n", line->Wlength);
#endif
			break;
		}
	}
	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];						out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];						out_buf[7] = line->head_data[7];
	out_buf[24] = ((line->Wlength >> 24) & 0xff);		out_buf[25] = ((line->Wlength >> 16) & 0xff);//wLength
	out_buf[26] = ((line->Wlength >> 8) & 0xff);		out_buf[27] = ((line->Wlength >> 0) & 0xff);
	//memcpy(out_buf + 48, usb_data, size);

	


	_lock_set(line->sock_lock);
	//tran_data_out("块传输in线程发送数据包\n");


	if(line->response_flag == 1)
	{
		// printf("lbc ###########################\n");
		// printf_array((unsigned char *)out_buf, 48);
		if (send(line->sock, out_buf, 48, MSG_NOSIGNAL)<0)
		{
			_lock_un(line->sock_lock);
			_lock_set(bulk_lock);
			if (line->bulk_in_data != NULL)
			{
				free(line->bulk_in_data);
				line->bulk_in_data = NULL;
			}
			*(line->end_flag) = 1;
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			
			return;
		}
	}
	_lock_un(line->sock_lock);
	//释放此条链路资源
	_lock_set(bulk_lock);
	if (line->bulk_in_data != NULL)
	{
		free(line->bulk_in_data);
		line->bulk_in_data = NULL;
	}
	// printf("2222222222 line->end_flag = %x\n",*line->end_flag);
	if(line->end_flag != NULL)
		(*(line->end_flag)) = 1;
	memset(line, 0, sizeof(struct _bulk_info));
	(*thread_num)--;

	_lock_un(bulk_lock);
	

}

static void bulk_data_to_dev(void *arg)
{
	struct _bulk_info *line;
	line = arg;
	
	int result;
	int size = 0;
	int *thread_num;
	int port_num;
	int retry_num = 0;
	port_num = line->port_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;

	unsigned char *usb_data = (unsigned char *)calloc(1,TRAN_DATA_LEN);
	if(NULL == usb_data)
	{
		return;
	}
	unsigned char *out_buf= (unsigned char *)calloc(1,TRAN_DATA_LEN);
	if(NULL == out_buf)
	{
		free(usb_data);
		return;
	}

	for (;;)
	{
		if (*line->out_close != 0)
		{
			tran_data_out("块传输线程外部要求退出!\n");
			_lock_set(bulk_lock);
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			if(NULL != out_buf)
			{
				free(out_buf);
				out_buf = NULL;
			}
			return;
		}
		if (((line->vid == 0x1677) && (line->pid == 0x0024)) && line->Wlength >= 65535)
		{
			tran_data_out("bulk in len too long :%d\n", line->Wlength);
			line->Wlength = 1024;//建行1677 0024出现65536的情况
		}
		if (*line->devh == NULL)//
		{
			//size = line->Wlength;//new add 0719
			//break;

			write_log("bulk_data_to_dev 错误devh已经释放 enter1\n");
			_lock_set(bulk_lock);
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			if(NULL != out_buf)
			{
				free(out_buf);
				out_buf = NULL;
			}
			return;
		}

		if (*line->unlink_flag == 1)
		{
			tran_data_out("bulk_data_to_dev unlink!\n");
			libusb_clear_halt(*line->devh, line->ep);
			line->stream->line[port_num].unlink_flag = 0;
			size = line->Wlength;
			break;//公网加载的时候发现可能存在unlink的情况
		}

		// count_2++;
		// printf("count_2 = %d\n",count_2);
		// printf("111111111 line->ep = %d,line->Wlength = %d\n",line->ep,line->Wlength);
		result = libusb_bulk_transfer(*line->devh, line->ep, usb_data, line->Wlength - size, &size, 2000);
		if (result < 0)
		{
			printf("bulk_data_to_dev result = %d\n",result);
			if (*line->devh == NULL)
			{
				tran_data_out("bulk_data_to_dev 错误devh已经释放\n");
				_lock_set(bulk_lock);
				memset(line, 0, sizeof(struct _bulk_info));
				(*thread_num)--;
				_lock_un(bulk_lock);
				if(NULL != usb_data)
				{
					free(usb_data);
					usb_data = NULL;
				}
				if(NULL != out_buf)
				{
					free(out_buf);
					out_buf = NULL;
				}
				return;
			}
			//tran_data_out("块传输线程等待下次读取 --- result:%d---ep:%02x\n", result, line->ep);
			//tran_data_out("bulk err wait next:ep:%02x --result:%d!--len:%d -size:%d\n", line->ep, result, line->Wlength, size);
			if (*line->unlink_flag == 1)
			{
				tran_data_out("bulk_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, line->ep);
				line->stream->line[port_num].unlink_flag = 0;
				size = line->Wlength;//new add 0719
				break;//公网加载的时候发现可能存在unlink的情况
			}
			if (size > 0)
			{
				tran_data_out("size is not zore ,break!\n");
				break;
			}
			if (result != -7)
			{
				if (result == LIBUSB_ERROR_PIPE)//当为-9的时候表示此时这帧指令可能为错误的，如stall pid reset等，调试建行的一163C 0620设备时发现,但是调试农行的一个设备的时候要清除端点等待096e 0705
				{
					tran_data_out("块传输线程err pipe--wlength:%d -thread_num:%d!\n", line->Wlength, *thread_num);
					if (((line->vid == 0x163c) && (line->pid == 0x0620)) || ((line->vid == 0x163c) && (line->pid == 0x5462)) \
						)//21-0312 -中行 163c 5462 
					{
						libusb_clear_halt(*line->devh, line->ep);
						size = line->Wlength;
						break;
					}
					if(((line->vid == 0x096e) && (line->pid == 0x0808)))
					{
						libusb_clear_halt(*line->devh, line->ep);
						libusb_reset_device(*line->devh);
						result = libusb_bulk_transfer(*line->devh, line->ep, usb_data, line->Wlength - size, &size, 2000);
						size = line->Wlength;
						break;
					}

					if (retry_num == 5)
					{
						_lock_set(bulk_lock);
						memset(line, 0, sizeof(struct _bulk_info));
						(*thread_num)--;
						_lock_un(bulk_lock);
						if(NULL != usb_data)
						{
							free(usb_data);
							usb_data = NULL;
						}
						if(NULL != out_buf)
						{
							free(out_buf);
							out_buf = NULL;
						}
						return;
					}
					/*if ((line->vid == 0x096e) && (line->pid == 0x0705))
					{*/
					libusb_clear_halt(*line->devh, line->ep);
					sleep(1);
					continue;
					//}



				}
				if (result == -1 || result == -4)
				{
					// tran_data_out("line->vid = %x,line->pid = %x\n",line->vid,line->pid); //my_tran_stream
					
					if(((line->vid == 0x4344) && (line->pid == 0x0000)))
					{
						 sleep(1);
						re_attach_data[port_num] = 1;
						break;
					}
				}
				if (result == LIBUSB_ERROR_OVERFLOW)
				{
					tran_data_out("overflow wait next:%d! break\n", size);//测试发现公网加载金税盘的时候会出现这个问题  ???是否下次的时候将line->Wlength 置为point的maxpacket  待验证
					//libusb_clear_halt(*line->devh, line->ep);
					continue;
					/*if (size > 0)
					break;
					else
					{

					size = line->Wlength;
					break;
					}*/
				}
				_lock_set(bulk_lock);
				memset(line, 0, sizeof(struct _bulk_info));
				(*thread_num)--;
				_lock_un(bulk_lock);
				
				if(NULL != usb_data)
				{
					free(usb_data);
					usb_data = NULL;
				}
				if(NULL != out_buf)
				{
					free(out_buf);
					out_buf = NULL;
				}
				return;
			}
			//2021/12/22 壹誻加载华夏银行盾d449 0004 发现有该问题 

			//{
			//	//if ((line->vid == 0xd449) && (line->pid == 0x0004))
			//	size = line->Wlength;
			//	tran_data_out("HXBANK SPECIAL DEAL!\n");
			//	break;  
			//}
			
			if (((line->vid == 0x163c) && (line->pid == 0x5489))||((line->vid == 0x2ec9) && (line->pid == 0x1001)))
			{
				size = line->Wlength;
				tran_data_out("SPECIAL DEAL!\n");
				break;
			}

			continue;//2021/12/22 壹誻加载华夏银行盾d449 0004 发现有该问题 --2022/02/15发现公网加载税务Ukey登录的时候可能会出现-7错误 要等待
		}
		else
		{
#ifdef BULK_OUT
			tran_data_out("块传输线程读取到数据长度为:%d\n", size);
#endif
			break;
		}
	}

	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];						out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];
							out_buf[7] = line->head_data[7];
	if (result == -1 || result == -4)
	{
		if(((line->vid == 0x4344) && (line->pid == 0x0000)))
		{
			printf("-----lbc    need attached again\n");
			tran_data_out("重新枚举\n");
			size = strlen("need attached again");
			memcpy(usb_data,"need attached again",size);
		}

	}


	out_buf[24] = ((size >> 24) & 0xff);		out_buf[25] = ((size >> 16) & 0xff);//wLength
	out_buf[26] = ((size >> 8) & 0xff);		out_buf[27] = ((size >> 0) & 0xff);
	memcpy(out_buf + 48, usb_data, size);

	// tran_data_out("【块传输读取】数据长度为：%d\n", size);
	// printf_array(usb_data,size);

	// printf("lbc $$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	// printf_array((unsigned char *)out_buf, 48 + size);

	_lock_set(line->sock_lock);
	//tran_data_out("块传输in线程发送数据包\n");



	if (send(line->sock, out_buf, 48 + size, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(bulk_lock);
		memset(line, 0, sizeof(struct _bulk_info));
		(*thread_num)--;
		_lock_un(bulk_lock);
		if(NULL != usb_data)
		{
			free(usb_data);
			usb_data = NULL;
		}
		if(NULL != out_buf)
		{
			free(out_buf);
			out_buf = NULL;
		}
		return;
	}
	_lock_un(line->sock_lock);
	//释放此条链路资源
	_lock_set(bulk_lock);
	memset(line, 0, sizeof(struct _bulk_info));
	(*thread_num)--;


	_lock_un(bulk_lock);
	if(NULL != usb_data)
	{
		free(usb_data);
		usb_data = NULL;
	}
	if(NULL != out_buf)
	{
		free(out_buf);
		out_buf = NULL;
	}
	return ;

}
static int bulk_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd,int response_flag,int *end_flag)
{
	
	//int i;
	int dirction;
	int ep;
	int dir_ep;
	char tmp[20];
	//uint8 request_type;
	//uint8 bRequest;
	//uint16 wValue;
	//uint16 wIndex;
	uint32 wLength;
	
	memset(tmp, 0, sizeof(tmp));
	//wang 11/17 打印usbip数据
	// tran_data_out("【块传输】数据长度为：%d\n", buf_len);
	// printf_array(buf,buf_len);

// #ifdef BULK_OUT
// 	int i;
// 	tran_data_out("【块传输】传入到设备的数据长度为：%d\n", buf_len);
// 	printf_array(buf,buf_len);
// #endif

	unsigned char *usb_data = (unsigned char *)calloc(1,TRAN_DATA_LEN);
	if(NULL == usb_data)
	{
		return -1;
	}

	wLength = buf[24] * 0x1000000 + buf[25] * 0x10000 + buf[26] * 0x100 + buf[27];
	dirction = buf[12] * 0x1000000 + buf[13] * 0x10000 + buf[14] * 0x100 + buf[15];
	ep = buf[16] * 0x1000000 + buf[17] * 0x10000 + buf[18] * 0x100 + buf[19];



	if (dirction == 1)
	{
		dir_ep = ep | 0x80;
	}
	else
	{
		dir_ep = ep;
	}
	//int len_t = buf_len - 48;
	if (dir_ep == stream->line[port_num - 1].out_ep)
	{
		memcpy(usb_data, buf + 48, buf_len - 48);
		int i;

		//if (buf_len - 48 > sizeof(stream->line[port_num - 1].bulk_info_out[i].in_data))
		//{
		//	printf("bulk tran_data_out buff too large:%d\n!", buf_len - 48);
		//	return -1;
		//}
		for (i = 0; i < sizeof (stream->line[port_num - 1].bulk_info_out) / sizeof(struct _bulk_info); i++)
		{
			if (stream->line[port_num - 1].bulk_info_out[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX)
		{
			tran_data_out("块传输in 线程达到上限！\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		_lock_set(bulk_lock);
		stream->line[port_num - 1].bulk_info_out[i].devh = &stream->line[port_num - 1].devh;
		stream->line[port_num - 1].bulk_info_out[i].Wlength = wLength;
		stream->line[port_num - 1].bulk_info_out[i].ep = dir_ep;
		stream->line[port_num - 1].bulk_info_out[i].sock = socket_fd;
		stream->line[port_num - 1].bulk_info_out[i].sock_lock = stream->line[port_num - 1].lock;
		memcpy(stream->line[port_num - 1].bulk_info_out[i].head_data, buf, 48);

		stream->line[port_num - 1].bulk_info_out[i].bulk_in_data = malloc(buf_len - 48 + 20);
		if (stream->line[port_num - 1].bulk_info_out[i].bulk_in_data == NULL)
		{
			tran_data_out("块传输malloc失败!\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		memset(stream->line[port_num - 1].bulk_info_out[i].bulk_in_data, 0, buf_len - 48 + 20);

		memcpy(stream->line[port_num - 1].bulk_info_out[i].bulk_in_data, usb_data, buf_len - 48);
		stream->line[port_num - 1].bulk_info_out[i].Wlength = buf_len - 48;
		stream->line[port_num - 1].bulk_info_out[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].bulk_info_out[i].thread_num = &(stream->line[port_num - 1].bulk_out_thread_num);
		stream->line[port_num - 1].bulk_info_out[i].state = 1;
		stream->line[port_num - 1].bulk_info_out[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].bulk_info_out[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].bulk_info_out[i].stream = stream;
		stream->line[port_num - 1].bulk_info_out[i].port_num = port_num - 1;
		stream->line[port_num - 1].bulk_info_out[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		stream->line[port_num - 1].bulk_info_out[i].end_flag = &stream->line[port_num - 1].end_flag;
		_lock_un(bulk_lock);
		//tran_data_out("块传输创建线程\n");
		stream->line[port_num - 1].bulk_info_out[i].response_flag = response_flag;
		// stream->line[port_num - 1].bulk_info_out[i].end_flag = end_flag;
		// printf("1111111111 end_flag = %x,stream->line[port_num - 1].bulk_info_out[i].end_flag = %x\n",end_flag,stream->line[port_num - 1].bulk_info_out[i].end_flag);
		if (_delay_task_add("0", bulk_data_to_dev_out, &stream->line[port_num - 1].bulk_info_out[i], 0)<0)
		{
			tran_data_out("创建队列失败\n");
		}
		if(NULL != usb_data)
		{
			free(usb_data);
			usb_data = NULL;
		}
		return 0;

	}
	else if (dir_ep == stream->line[port_num - 1].in_ep)
	{
		memset(usb_data, 0, TRAN_DATA_LEN);

		//tran_data_out("【块传输】读出设备发送数据至in端点wLength = %d\n", wLength);

		int i;


		for (i = 0; i < sizeof (stream->line[port_num - 1].bulk_info) / sizeof(struct _bulk_info); i++)
		{
			if (stream->line[port_num - 1].bulk_info[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX)
		{
			tran_data_out("块传输in 线程达到上限！\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		_lock_set(bulk_lock);
		stream->line[port_num - 1].bulk_info[i].devh = &stream->line[port_num - 1].devh;
		stream->line[port_num - 1].bulk_info[i].Wlength = wLength;
		stream->line[port_num - 1].bulk_info[i].ep = dir_ep;
		stream->line[port_num - 1].bulk_info[i].sock = socket_fd;
		stream->line[port_num - 1].bulk_info[i].sock_lock = stream->line[port_num - 1].lock;
		memcpy(stream->line[port_num - 1].bulk_info[i].head_data, buf, 48);
		stream->line[port_num - 1].bulk_info[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].bulk_info[i].thread_num = &(stream->line[port_num - 1].bulk_in_thread_num);
		stream->line[port_num - 1].bulk_info[i].state = 1;
		stream->line[port_num - 1].bulk_info[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].bulk_info[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].bulk_info[i].stream = stream;
		stream->line[port_num - 1].bulk_info[i].port_num = port_num - 1;
		stream->line[port_num - 1].bulk_info[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		_lock_un(bulk_lock);
		//tran_data_out("块传输创建线程\n");
		if (_delay_task_add("0", bulk_data_to_dev, &stream->line[port_num - 1].bulk_info[i], 0)<0)
		{
			tran_data_out("创建队列失败\n");
		}
		if(NULL != usb_data)
		{
			free(usb_data);
			usb_data = NULL;
		}
		return 0;

	}
	if(NULL != usb_data)
	{
		free(usb_data);
		usb_data = NULL;
	}
	return 0;

}
/*
控制传输从设备中读取数据 (in)
*/
/*
libusb_device_handle *dev_handle：  设备句柄。
uint8_t  bmRequestType：			对应Setup Packet包中的bmRequestType字段。
uint8_t  bRequest： 				对应Setup Packet包中的bRequest字段。
uint16_t   wValue： 				对应Setup Packet包中的wValue字段。
uint16_t wIndex： 					对应Setup Packet包中的wIndex字段。
unsigned char  *data： 				数据。
uint16_t  wLength： 				数据大小。
unsigned int  timeout： 			超时阀值
*/

static void control_data_from_dev(void *arg)
{
	struct _control_info *line;
	line = arg;
	
	int result,i = 0;
	int *thread_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	int reload_times = 0;
	// tran_data_out("control in thread_num :%d\n", *thread_num);
	unsigned char *usb_data = (unsigned char *)calloc(1,TRAN_DATA_LEN);
	if(NULL == usb_data)
	{
		return;
	}
	unsigned char *out_buf = (unsigned char *)calloc(1,TRAN_DATA_LEN);
	if(NULL == out_buf)
	{
		free(usb_data);
		return;
	}
	for (;;)
	{
		if (*line->out_close != 0)
		{
			tran_data_out("控制传输线程外部要求退出!\n");
			_lock_set(control_lock);
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
			}
			if(NULL != out_buf)
			{
				free(out_buf);
			}
			return;
		}
		if (*line->devh == NULL)//
		{
			//result = line->Wlength;//new add 0719
			//break;
			write_log("control_data_from_dev 错误devh已经释放 enter1\n");
			_lock_set(control_lock);
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
			}
			if(NULL != out_buf)
			{
				free(out_buf);
			}
			return;
		}
		result = libusb_control_transfer(*line->devh, line->request_type, line->bRequest, line->wValue, line->wIndex, usb_data, line->Wlength, 2000);//5000->2000  10 29
		if (result < 0)
		{
			tran_data_out("控制传输线程执行下次读取!\n");
			tran_data_out("control_data_from_dev err :%d .wait next!\n", result);
			if (*line->devh == NULL)
			{
				write_log("control_data_from_dev 错误devh已经释放\n");
				_lock_set(control_lock);
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				if(NULL != usb_data)
				{
					free(usb_data);
				}
				if(NULL != out_buf)
				{
					free(out_buf);
				}
				
				return;
			}
			if (*line->unlink_flag == 1)
			{
				tran_data_out("control_data_from_dev unlink!\n");
				libusb_clear_halt(*line->devh, 0);
				//line->stream->line[port_num].unlink_flag = 0;//new chg 0714
				*line->unlink_flag = 0;//new add by whl 1025
				result = line->Wlength;//new add 0719
				break;//公网加载的时候发现可能存在unlink的情况
			}
			if ((line->vid == 0X1ea8 && line->pid == 0Xc102) || (line->vid == 0X1d99 && line->pid == 0X0013))//协卡助手ca问题  可能为 -9 err pipe 需要重新获取 原因不明
			{
				if (reload_times >= 2)//new change by whl
				{
					tran_data_out("reload_times too many times!\n");
					libusb_clear_halt(*line->devh, 0);
					result = line->Wlength;
					//line->stream->line[port_num].unlink_flag = 0;
					break;
				}

				if (reload_devh(line->stream, line->port_num) < 0)
				{
					_lock_set(control_lock);
					memset(line, 0, sizeof(struct _control_info));
					(*thread_num)--;
					_lock_un(control_lock);
					if(NULL != usb_data)
					{
						free(usb_data);
					}
					if(NULL != out_buf)
					{
						free(out_buf);
					}
					return;
				}
				reload_times++;
				continue;
			}
			if (line->vid == 0X096e && line->pid == 0X0801)//bj bank
			{
				result = line->Wlength;
				break;
			}

			if (result != LIBUSB_ERROR_TIMEOUT)//不是超时-7 其他错误则退出
			{
				tran_data_out("控制传输in线程执行失败--result :%d --len:%d!\n", result, line->Wlength);
				if (line->vid == 0X1a56 && line->pid == 0Xdd01)//电子口岸设备处理--不知道是否正确，待验证！
				{
					result = 0;
					break;
				}
				_lock_set(control_lock);
				
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				if(NULL != usb_data)
				{
					free(usb_data);
				}
				if(NULL != out_buf)
				{
					free(out_buf);
				}
				return;
			}
			continue;
		}
		else
		{
			break;
		}
	}
	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];						out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];						out_buf[7] = line->head_data[7];
	out_buf[24] = ((result >> 24) & 0xff);		out_buf[25] = ((result >> 16) & 0xff);
	out_buf[26] = ((result >> 8) & 0xff);		out_buf[27] = ((result >> 0) & 0xff);
	memcpy(out_buf + 48, usb_data, result);


	// printf("lbc **********************************\n");
	// printf_array((unsigned char *)out_buf, 48 + result);

	_lock_set(line->sock_lock);
	//tran_data_out("控制传输in线程发送数据包\n");
	if (send(line->sock, out_buf, 48 + result, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(control_lock);
		
		memset(line, 0, sizeof(struct _control_info));
		(*thread_num)--;
		_lock_un(control_lock);
		if(NULL != usb_data)
		{
			free(usb_data);
		}
		if(NULL != out_buf)
		{
			free(out_buf);
		}
		return;
	}
	_lock_un(line->sock_lock);

	if(NULL != usb_data)
	{
		free(usb_data);
		usb_data = NULL;
	}
	if(NULL != out_buf)
	{
		free(out_buf);
		out_buf = NULL;
	}

	//释放此条链路资源
	_lock_set(control_lock);
	memset(line, 0, sizeof(struct _control_info));
	(*thread_num)--;
	_lock_un(control_lock);
	tran_data_out("控制传输in线程发送数据包结束 -- %08x   -- %d\n", thread_num, *thread_num);
	return;
}
/*
控制传输往usb设备写入数据  (tran_data_out)
*/
static void control_data_to_dev(void *arg)
{
	struct _control_info *line;
	line = arg;
	int result;
	int *thread_num;
	int port_num;
	port_num = line->port_num;
	
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	int reload_times = 0;
	// tran_data_out("control tran_data_out thread_num:%d --unlink_flag:%d\n", *thread_num, *line->unlink_flag);
	unsigned char *usb_data = (unsigned char *)malloc(TRAN_DATA_LEN);
	if(NULL == usb_data)
	{
		return;
	}
	memcpy(usb_data, line->ctl_in_data, line->ctl_in_data_len);
	unsigned char *out_buf = (unsigned char *)malloc(TRAN_DATA_LEN);
	if(NULL == out_buf)
	{
		free(usb_data);
		return;
	}

	// int i;
	// printf("control_data_to_dev,len = %d\n",line->ctl_in_data_len);
	// for (i = 0; i < line->ctl_in_data_len; i++)
	// {
	// printf("%02x ", usb_data[i]);
	// }
	// printf("\n\n");
	for (;;)
	{
		if (*line->out_close != 0)
		{
			tran_data_out("控制传输线程外部要求退出!\n");
			_lock_set(control_lock);
			if (line->ctl_in_data != NULL)
			{
				free(line->ctl_in_data);
				line->ctl_in_data = NULL;
			}
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
			}
			if(NULL != out_buf)
			{
				free(out_buf);
			}
			return;
		}
		if (*line->devh == NULL)//
		{
			write_log("control_data_to_dev 错误devh已经释放 enter1\n");
			_lock_set(control_lock);
			if (line->ctl_in_data != NULL)
			{
				free(line->ctl_in_data);
				line->ctl_in_data = NULL;
			}
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			if(NULL != usb_data)
			{
				free(usb_data);
			}
			if(NULL != out_buf)
			{
				free(out_buf);
			}
			return;
		}
		result = libusb_control_transfer(*line->devh, line->request_type, line->bRequest, line->wValue, line->wIndex, usb_data, line->ctl_in_data_len, 5000);//5000->2000  10 29
		if (result < 0)
		{
			tran_data_out("libusb_control_transfer err :%d .wait next or unlink !\n", result);
			if (*line->devh == NULL)
			{
				write_log("control_data_to_dev 错误devh已经释放\n");
				_lock_set(control_lock);
				if (line->ctl_in_data != NULL)
				{
					free(line->ctl_in_data);
					line->ctl_in_data = NULL;
				}
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				if(NULL != usb_data)
				{
					free(usb_data);
				}
				if(NULL != out_buf)
				{
					free(out_buf);
				}
				return;
			}

			if (*line->unlink_flag == 1)
			{
				tran_data_out("control_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, 0);
				//*line->unlink_flag = 0;//new add by  whl 1025
				break;//公网加载的时候发现可能存在unlink的情况
			}


			if ((line->vid == 0X1ea8 && line->pid == 0Xc102) || (line->vid == 0X1d99 && line->pid == 0X0013) || (line->vid == 0X0525 && line->pid == 0Xa4a2))//协卡助手ca 问题  可能为 -9 err pipe 需要重新获取 原因不明  (plc usb)
			{

				if (reload_times >= 2)//new change by whl
				{
					tran_data_out("reload_times too many times!\n");
					libusb_clear_halt(*line->devh, 0);
					//line->stream->line[port_num].unlink_flag = 0;
					break;
				}

				if (reload_devh(line->stream, line->port_num) < 0)
				{
					_lock_set(control_lock);
					if (line->ctl_in_data != NULL)
					{
						free(line->ctl_in_data);
						line->ctl_in_data = NULL;
					}
					memset(line, 0, sizeof(struct _control_info));
					(*thread_num)--;
					_lock_un(control_lock);
					return;
				}
				reload_times++;
				continue;
			}

			if (result != -7)
			{
				//for test
				//福建海峡银行和中国建设银行
				if ((result == LIBUSB_ERROR_PIPE && (line->vid == 0Xd449 && line->pid == 0X0004)) || (result == LIBUSB_ERROR_PIPE && (line->vid == 0X163c && line->pid == 0X548c)))
				{
					libusb_clear_halt(*line->devh, 0);

					line->stream->line[port_num].unlink_flag = 0;
					break;
				}
				//if (result == LIBUSB_ERROR_PIPE && (line->vid == 0X163c && line->pid == 0X548c))//中国建设银行一种设备
				if (result == LIBUSB_ERROR_PIPE && ((line->vid == 0X163c && line->pid == 0X548c)||(line->vid == 0X287f && line->pid == 0Xf409)))//中国建设银行一种设备
				{
					libusb_clear_halt(*line->devh, 0);

					line->stream->line[port_num].unlink_flag = 0;
					break;
				}
				//
				if (result == -2 || result == -9)
				{
					_lock_set(control_lock);
					if (line->ctl_in_data != NULL)
					{
						free(line->ctl_in_data);
						line->ctl_in_data = NULL;
					}
					memset(line, 0, sizeof(struct _control_info));
					(*thread_num)--;
					_lock_un(control_lock);
					if(NULL != usb_data)
					{
						free(usb_data);
					}
					if(NULL != out_buf)
					{
						free(out_buf);
					}
					return;
				}
				tran_data_out("控制传输out线程执行失败--result:%d- len:%d!\n", result, line->ctl_in_data_len);
				_lock_set(control_lock);
				if (line->ctl_in_data != NULL)
				{
					free(line->ctl_in_data);
					line->ctl_in_data = NULL;
				}
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				if(NULL != usb_data)
				{
					free(usb_data);
				}
				if(NULL != out_buf)
				{
					free(out_buf);
				}
				return;
			}

			continue;
		}
		else
		{
			// tran_data_out("控制传输线程写入数据长度为:%d\n", result);
			// int i;
			// printf("2222222222222222222222222222222222\n");
			// for (i = 0; i < result; i++)
			// {
			// printf("%02x ", usb_data[i]);
			// }
			// printf("\n\n");
			break;
		}
	}
	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];			out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];			out_buf[7] = line->head_data[7];
	out_buf[24] = 0x00;//((result >> 24) & 0xff);		
	out_buf[25] = 0x00;//((result >> 16) & 0xff);
	out_buf[26] = 0x00;//((result >> 8) & 0xff);		
	out_buf[27] = 0x00;//((result >> 0) & 0xff);
	//memcpy(out_buf + 48, usb_data, result);

	// printf("lbc +++++++++++++++++++++++++++++\n");
	// printf_array((unsigned char *)out_buf,  48);

	_lock_set(line->sock_lock);
	//tran_data_out("控制传输out线程发送数据包\n");
	if (send(line->sock, out_buf, 48, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(control_lock);
		if (line->ctl_in_data != NULL)
		{
			free(line->ctl_in_data);
			line->ctl_in_data = NULL;
		}
		memset(line, 0, sizeof(struct _control_info));
		(*thread_num)--;
		_lock_un(control_lock);
		if(NULL != usb_data)
		{
			free(usb_data);
		}
		if(NULL != out_buf)
		{
			free(out_buf);
		}
		return;
	}
	_lock_un(line->sock_lock);

	if(NULL != usb_data)
	{
		free(usb_data);
		usb_data = NULL;
	}
	if(NULL != out_buf)
	{
		free(out_buf);
		out_buf = NULL;
	}

	//释放此条链路资源
	_lock_set(control_lock);
	if (line->ctl_in_data != NULL)
	{
		free(line->ctl_in_data);
		line->ctl_in_data = NULL;
	}
	memset(line, 0, sizeof(struct _control_info));
	(*thread_num)--;
	_lock_un(control_lock);

	// tran_data_out(" tran_data_out  control tran_data_out thread_num:%d\n", *thread_num);
	return;
}

static int control_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd)
{
	//unsigned char usbip_data[204800];
	unsigned char* usb_data = NULL;
	int i;
	//int result;
	//int r_len;
	uint8 request_type;
	uint8 bRequest;
	uint16 wValue;
	uint16 wIndex;
	uint16 wLength;
	//unsigned int timeout = 5000;
	//memset(usb_data, 0, sizeof(usb_data));


//wang 11/17 打印usbip数据
	  //tran_data_out("【控制传输】传入到设备的数据\n");
// 	  printf("control_transfer data\n");
// 	  printf_array(buf,buf_len);
// #ifdef CONTROL_OUT
// 	//int i;
// 	tran_data_out("【控制传输】传入到设备的数据\n");

// 	printf_array(buf,buf_len);
// #endif

	request_type = buf[40];
	bRequest = buf[41];
	wValue = buf[43] * 0x100 + buf[42];
	wIndex = buf[45] * 0x100 + buf[44];
	wLength = buf[47] * 0x100 + buf[46];
	// tran_data_out("【控制传输】wLength = %d\n", wLength);
	usb_data = malloc(buf_len - 48 + 10);
	if (usb_data == NULL)
		return -1;
	memset(usb_data, 0, buf_len - 48 + 10);
	memcpy(usb_data, buf + 48, buf_len - 48);

	int direction = 0;
	direction = buf[15];


	if (direction == 1)//表示要从in端点读取数据
	{

		// int ret;
		free(usb_data);
		for (i = 0; i < sizeof (stream->line[port_num - 1].control_info) / sizeof(struct _control_info); i++)
		{
			if (stream->line[port_num - 1].control_info[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX)
		{
			tran_data_out("控制传输in 线程达到上限！\n");
			return -1;
		}
		_lock_set(control_lock);
		stream->line[port_num - 1].control_info[i].devh = &stream->line[port_num - 1].devh;

		stream->line[port_num - 1].control_info[i].sock = socket_fd;
		stream->line[port_num - 1].control_info[i].bRequest = bRequest;
		stream->line[port_num - 1].control_info[i].wValue = wValue;
		stream->line[port_num - 1].control_info[i].wIndex = wIndex;
		stream->line[port_num - 1].control_info[i].request_type = request_type;
		stream->line[port_num - 1].control_info[i].Wlength = wLength;
		memcpy(stream->line[port_num - 1].control_info[i].head_data, buf, 48);
		stream->line[port_num - 1].control_info[i].sock_lock = stream->line[port_num - 1].lock;
		stream->line[port_num - 1].control_info[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].control_info[i].thread_num = &(stream->line[port_num - 1].control_in_thread_num);
		stream->line[port_num - 1].control_info[i].state = 1;

		stream->line[port_num - 1].control_info[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].control_info[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].control_info[i].stream = stream;
		stream->line[port_num - 1].control_info[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		stream->line[port_num - 1].control_info[i].port_num = port_num - 1;
		_lock_un(control_lock);
		//tran_data_out("控制传输创建线程--in\n");
		if (_delay_task_add("0", control_data_from_dev, &stream->line[port_num - 1].control_info[i], 0) < 0)
		{
			tran_data_out("创建队列失败\n");
		}
		//tran_data_out("控制传输创建线程--in -- over!\n");
		return 0;
	}
	else if (direction == 0)
	{

		//int ret;


		for (i = 0; i < sizeof (stream->line[port_num - 1].control_info_out) / sizeof(struct _control_info); i++)
		{
			if (stream->line[port_num - 1].control_info_out[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX)
		{
			tran_data_out("控制传输out 线程达到上限！\n");
			free(usb_data);
			return -1;
		}
		_lock_set(control_lock);
		stream->line[port_num - 1].control_info_out[i].devh = &stream->line[port_num - 1].devh;

		stream->line[port_num - 1].control_info_out[i].sock = socket_fd;
		stream->line[port_num - 1].control_info_out[i].bRequest = bRequest;
		stream->line[port_num - 1].control_info_out[i].wValue = wValue;
		stream->line[port_num - 1].control_info_out[i].wIndex = wIndex;
		stream->line[port_num - 1].control_info_out[i].request_type = request_type;
		stream->line[port_num - 1].control_info_out[i].Wlength = wLength;
		memcpy(stream->line[port_num - 1].control_info_out[i].head_data, buf, 48);
		// if (buf_len - 48 > 2000)//new add by whl 10 29
		// {
		// 	write_log("控制传输数据过大:%d,失败!\n", buf_len - 48);
		// 	_lock_un(control_lock);
		// 	free(usb_data);
		// }

		stream->line[port_num - 1].control_info_out[i].ctl_in_data = malloc(buf_len - 48 + 20);
		if (stream->line[port_num - 1].control_info_out[i].ctl_in_data == NULL)
		{
			tran_data_out("块传输malloc失败！\n");
			return -1;
		}
		memset(stream->line[port_num - 1].control_info_out[i].ctl_in_data, 0, buf_len - 48 + 20);
		memcpy(stream->line[port_num - 1].control_info_out[i].ctl_in_data, usb_data, buf_len - 48);


		//memcpy(stream->line[port_num - 1].control_info_out[i].ctl_in_data, usb_data, buf_len - 48);
		stream->line[port_num - 1].control_info_out[i].ctl_in_data_len = buf_len - 48;
		stream->line[port_num - 1].control_info_out[i].sock_lock = stream->line[port_num - 1].lock;
		stream->line[port_num - 1].control_info_out[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].control_info_out[i].thread_num = &(stream->line[port_num - 1].control_out_thread_num);
		stream->line[port_num - 1].control_info_out[i].state = 1;

		stream->line[port_num - 1].control_info_out[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].control_info_out[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].control_info_out[i].stream = stream;

		stream->line[port_num - 1].control_info_out[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		stream->line[port_num - 1].control_info_out[i].port_num = port_num - 1;
		_lock_un(control_lock);
		free(usb_data);
		//tran_data_out("控制传输创建线程--tran_data_out\n");
		if (_delay_task_add("0", control_data_to_dev, &stream->line[port_num - 1].control_info_out[i], 0) < 0)
		{
			tran_data_out("创建队列失败\n");
		}
		return 0;
	}
	else
	{
		tran_data_out("控制端点方向不是in也不是out\n");
		return -1;
	}
}

//中断传输中日志打印需要注意  电子口岸设备发现打印日志影响时序导致登录会失败
static void interr_data_from_dev(void *arg)//兴业银行发现需要异步处理
{
	struct _interrupt_info *line;
	line = arg;
	//unsigned char usb_data[204800] = { 0 };
	unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[1024] = { 0 };
	int result;
	int *thread_num;
	int size = 0;
	//int count = 0;
	int len = 0;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;

	 

	if ((line->vid == 0X163c && line->pid == 0X070b) || (line->vid == 0X163c && line->pid == 0X0704))//招行特殊设备   描述符返回maxpacketsize是16 实际要传32(该vid pid招行有两种  其中一种正常运行的就是32)
	{
		len = 32;
	}
	else if ((line->vid == 0X096e && line->pid == 0X0727) || ((line->vid == 0x1780 && line->pid == 0x0410)))//兴业银行（飞天）特殊设备   描述符返回maxpacketsize是16 实际要传64
	{
		tran_data_out("interr_data_from_dev in :%d --ep:%02x!\n", line->Wlength, line->ep);
		len = 64;
	}
	//lib1780_0491
	// (line->vid == 0x1780 && line->pid == 0x0491) wang 02 02招行盾返回是32实际时64
	else if ((line->vid == 0x096e && line->pid == 0x070b) || (line->vid == 0x1780 && line->pid == 0x0410) || (line->vid == 0x1780 && line->pid == 0x0491) || (line->vid == 0x096e && line->pid == 0xa010))
	{
		len = 64;
	}
	else
	{
		len = line->Wlength;
	}
	if ((line->vid == 0X5188 && line->pid == 0X1801 && line->ep == 0x83))  //幽灵键鼠   
	{ 
		len = 0x24;
	}
	else if ((line->vid == 0X5188 && line->pid == 0X1801 && line->ep == 0x82))  //幽灵键鼠   
	{ 
		len = 0x08;
	}
	else if ((line->vid == 0X5188 && line->pid == 0X1801 && line->ep == 0x81))  //幽灵键鼠   
	{ 
		len = 0x08;
	}
	// len = 8;
	
	for (;;)
	{

		if (*line->out_close != 0)
		{
			tran_data_out("中断传输线程in外部要求退出!\n");
			_lock_set(interrupt_lock);
			memset(line, 0, sizeof(struct _interrupt_info));
			(*thread_num)--;
			_lock_un(interrupt_lock);
			return;
		}
		// printf("line->vid = %x,line->pid = %x,line->Wlength = %d,len = %d,line->ep = %.02x\n", line->vid, line->pid,line->Wlength,len,line->ep);

		//if ((line->vid == 0X5188 && line->pid == 0X1801))  //0X1801
		{
			result = libusb_interrupt_transfer(line->devh, line->ep, usb_data, len, &size, 0);	//执行USB中断传输。该函数可以处理输入和输出，根据端点地址的方向位推断传输方向，该函数采用同步模式，数据传输完毕才返回
			// printf("libusb_interrupt_transfer result = %d\n",result);
		}
		// else
		// {
		// 	result = libusb_interrupt_transfer(line->devh, line->ep, usb_data, len, &size, 2000);
		// }
		if (result < 0)
		{
			//  printf("libusb_interrupt_transfer error interr_data_from_dev,result = %d,size = %d,line->ep = %.02x\r\n",result,size,line->ep);
			if (result == LIBUSB_ERROR_TIMEOUT)
			{
				if (size != 0)
				{
					break;
				}
				continue;
			}
			else
			{
				if (result == LIBUSB_ERROR_PIPE)
				{
					// printf("---------------- libusb_interrupt_transfer 111111111111111111111111111111\n");
					libusb_clear_halt(line->devh, line->ep);
					// if ((line->vid == 0X5188 && line->pid == 0X1801 && line->ep == 0x83))
					// {
					// 	break;
					// }
					continue;
				}
				else
				{
					printf("1111111111111111 interr_data_from_dev out_close = 1\n");
					_lock_set(interrupt_lock);
					*line->out_close = 1;
					memset(line, 0, sizeof(struct _interrupt_info));
					(*thread_num)--;
					_lock_un(interrupt_lock);
					
					return;
				}
			}
		}
		else
		{
			break;
		}
	}
	// printf("libusb_interrupt_transfer interr_data_from_dev exit\n");
	if (line->vid == 0x1780 && line->pid == 0x0410)
		size = 64;
	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];			out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];			out_buf[7] = line->head_data[7];
	out_buf[24] = ((size >> 24) & 0xff);		out_buf[25] = ((size >> 16) & 0xff);
	out_buf[26] = ((size >> 8) & 0xff);		out_buf[27] = ((size >> 0) & 0xff);
	memcpy(out_buf + 48, usb_data, size);

	_lock_set(line->sock_lock);

	// if(line->ep == 0x83)
	// {
	// 	printf("lbc $$$$$$$$$$$$$$$$$$$$$$$$$$$,line->ep = %.02x\n",line->ep);
	// 	printf_array((unsigned char *)out_buf, 48 + size);
	// }	
	if (send(line->sock, out_buf, 48 + size, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(interrupt_lock);
		memset(line, 0, sizeof(struct _interrupt_info));
		(*thread_num)--;
		_lock_un(interrupt_lock);
		return;
	}
	_lock_un(line->sock_lock);
	_lock_set(interrupt_lock);
	memset(line, 0, sizeof(struct _interrupt_info));
	(*thread_num)--;
	_lock_un(interrupt_lock);
	return;
}

static void interr_data_to_dev(void *arg)//兴业银行发现需要异步处理
{
	struct _interrupt_info *line;
	line = arg;
	unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[1024] = { 0 };
	int result;
	int *thread_num;
	int size = 0;

	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	// printf("interr_data_to_dev in :%d--ep:%02x!\n", line->Wlength, line->ep);
	// int i;
	// printf("libusb_interrupt_transfer interr_data_from_dev\n");
	// for (i = 0; i < line->Wlength; i++)
	// {
	// printf("%02x ", line->inter_in_data[i]);
	// }
	// printf("\n\n");
	for (;;)
	{

		if (*line->out_close != 0)
		{
			tran_data_out("中断传输线程外部要求退出!\n");
			_lock_set(interrupt_lock);
			if (line->inter_in_data != NULL)
			{
				free(line->inter_in_data);
				line->inter_in_data = NULL;
			}
			if(line->end_flag != NULL)
			(*(line->end_flag)) = 1;
			memset(line, 0, sizeof(struct _interrupt_info));
			(*thread_num)--;
			_lock_un(interrupt_lock);
			return;
		}
		result = libusb_interrupt_transfer(line->devh, line->ep, line->inter_in_data, line->Wlength, &size, 1000);
		if (result < 0)
		{
			tran_data_out("libusb_interrupt_transfer error!result = %d\r\n",result);
			if (result == LIBUSB_ERROR_TIMEOUT)
			{
				/*	if (count == 3)
				{
				tran_data_out("中断out超时超过三次!\n");
				result = line->Wlength;
				break;
				}
				count++;*/
				continue;
			}
			else
			{
				tran_data_out("中断out err,code is:%d!\n", result);
				_lock_set(interrupt_lock);
				if (line->inter_in_data != NULL)
				{
					free(line->inter_in_data);
					line->inter_in_data = NULL;
				}
				*line->out_close = 1;
				if(line->end_flag != NULL)
				(*(line->end_flag)) = 1;
				memset(line, 0, sizeof(struct _interrupt_info));
				(*thread_num)--;

				//*line->out_close = 1;
				_lock_un(interrupt_lock);
				return;
			}
		}
		else
		{
			break;
		}
	}

	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];			out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];			out_buf[7] = line->head_data[7];
	out_buf[24] = ((result >> 24) & 0xff);		out_buf[25] = ((result >> 16) & 0xff);
	out_buf[26] = ((result >> 8) & 0xff);		out_buf[27] = ((result >> 0) & 0xff);
	_lock_set(line->sock_lock);

	if(line->response_flag == 1)
	{
		// printf("lbc ###########################,line->ep = %.02x\n",line->ep);
		// printf_array((unsigned char *)out_buf, 48);
		if (send(line->sock, out_buf, 48, MSG_NOSIGNAL)<0)
		{
			_lock_un(line->sock_lock);
			_lock_set(interrupt_lock);

			if (line->inter_in_data != NULL)
			{
				free(line->inter_in_data);
				line->inter_in_data = NULL;
			}
			memset(line, 0, sizeof(struct _interrupt_info));
			(*thread_num)--;
			_lock_un(interrupt_lock);
			return;
		}
	}
	_lock_un(line->sock_lock);

	_lock_set(interrupt_lock);

	if (line->inter_in_data != NULL)
	{
		free(line->inter_in_data);
		line->inter_in_data = NULL;
	}
	if(line->end_flag != NULL)
		(*(line->end_flag)) = 1;
	memset(line, 0, sizeof(struct _interrupt_info));
	(*thread_num)--;
	_lock_un(interrupt_lock);
	return;
}
static int interrupt_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd,int response_flag,int *end_flag)
{
	unsigned char *usb_data = (unsigned char *)calloc(1,TRAN_DATA_LEN);
	//int i;
	//int result;
	int dirction;
	int ep;
	int dir_ep;

	//wang 11/17 打印usbip数据
	/*int i;
	tran_data_out("【中断传输】传入到设备的数据长度为：%d\n", buf_len);
	for (i = 0; i < buf_len; i++)
	{
		printf("%02x ", buf[i]);
	}
	printf("\n\n\n");*/
	//#ifdef INTERRUPT_OUT
		// int i;
		// tran_data_out("【中断传输】传入到设备的数据长度为：%d\n", buf_len);
		// for (i = 0; i < buf_len; i++)
		// {
		// 	printf("%02x ", buf[i]);
		// }
		// printf("\n\n\n");
	//#endif
	dirction = buf[12] * 0x1000000 + buf[13] * 0x10000 + buf[14] * 0x100 + buf[15];
	ep = buf[16] * 0x1000000 + buf[17] * 0x10000 + buf[18] * 0x100 + buf[19];
	//
	// printf("stream->line[port_num - 1].AllInterfaceNumber = %d\n",stream->line[port_num - 1].AllInterfaceNumber);
	if (((dirction == 1) || (stream->line[port_num - 1].bNumEndpoints == 1)))
	{
		if(((stream->line[port_num - 1].vid == 0X163c && stream->line[port_num - 1].pid == 0X070b && stream->line[port_num - 1].AllInterfaceNumber == 2))) //金智维双接口
		{
			dir_ep = ep;
		}
		else
		{
			dir_ep = ep | 0x80;
		}
	}
	else
	{
		dir_ep = ep;
	}
	// printf("22222222222222 ep=%.02x, dir_ep = %.02x,out_ep = %.02x,in_ep = %.02x\n",ep,dir_ep,stream->line[port_num - 1].out_ep,stream->line[port_num - 1].in_ep);
	if (dir_ep == stream->line[port_num - 1].out_ep)
	{
#if 1
		memcpy(usb_data, buf + 48, buf_len - 48);
		int i;
		for (i = 0; i < sizeof (stream->line[port_num - 1].interrupt_info_out) / sizeof(struct _interrupt_info); i++)
		{
			if (stream->line[port_num - 1].interrupt_info_out[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX)
		{
			tran_data_out("中断传输out 线程达到上限！\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		_lock_set(interrupt_lock);
		stream->line[port_num - 1].interrupt_info_out[i].devh = stream->line[port_num - 1].devh;
		stream->line[port_num - 1].interrupt_info_out[i].ep = dir_ep;
		stream->line[port_num - 1].interrupt_info_out[i].sock = socket_fd;
		memcpy(stream->line[port_num - 1].interrupt_info_out[i].head_data, buf, 48);


		stream->line[port_num - 1].interrupt_info_out[i].inter_in_data = malloc(buf_len - 48 + 20);
		if (stream->line[port_num - 1].interrupt_info_out[i].inter_in_data == NULL)
		{
			tran_data_out("中断传输malloc失败！\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		memset(stream->line[port_num - 1].interrupt_info_out[i].inter_in_data, 0, buf_len - 48 + 20);
		memcpy(stream->line[port_num - 1].interrupt_info_out[i].inter_in_data, usb_data, buf_len - 48);


		stream->line[port_num - 1].interrupt_info_out[i].Wlength = buf_len - 48;
		stream->line[port_num - 1].interrupt_info_out[i].sock_lock = stream->line[port_num - 1].lock;
		stream->line[port_num - 1].interrupt_info_out[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].interrupt_info_out[i].thread_num = &(stream->line[port_num - 1].interrupt_out_thread_num);
		stream->line[port_num - 1].interrupt_info_out[i].state = 1;

		stream->line[port_num - 1].interrupt_info_out[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].interrupt_info_out[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].interrupt_info_out[i].stream = stream;

		stream->line[port_num - 1].interrupt_info_out[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		stream->line[port_num - 1].interrupt_info_out[i].port_num = port_num - 1;
		stream->line[port_num - 1].interrupt_info_out[i].end_flag = &stream->line[port_num - 1].end_flag;
		_lock_un(interrupt_lock);
		stream->line[port_num - 1].interrupt_info_out[i].response_flag = response_flag;
		//tran_data_out("控制传输创建线程--tran_data_out\n");
		if (_delay_task_add("0", interr_data_to_dev, &stream->line[port_num - 1].interrupt_info_out[i], 0) < 0)
		{
			tran_data_out("创建队列失败\n");
		}
		if(NULL != usb_data)
		{
			free(usb_data);
			usb_data = NULL;
		}
		return 0;
#else

		int len_t = buf_len - 48;
		usleep(5000);
		memcpy(usb_data, buf + 48, buf_len - 48);
		result = libusb_interrupt_transfer(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep, usb_data, len_t, &size, timeout);
		if (result < 0)
		{
			tran_data_out("【中断传输】发送数据至out端点:%02x失败,result = %d\n", stream->line[port_num - 1].out_ep, result);
			if (result == LIBUSB_ERROR_TIMEOUT)
			{
				tran_data_out("【中断传输】清除端点，再次发送获取数据\n");
				libusb_clear_halt(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep);
				result = libusb_interrupt_transfer(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep, usb_data, len_t, &size, timeout);
				if (result < 0)
				{
					tran_data_out("【中断传输】清除端点，再次发送获取数据失败\n");
					if(NULL != usb_data)
					{
						free(usb_data);
						usb_data = NULL;
					}
					return -3;
				}
			}
			else
			{
				if(NULL != usb_data)
				{
					free(usb_data);
					usb_data = NULL;
				}
				return result;
			}
		}

		out_buf[0] = 0x00;			out_buf[1] = 0x00;				out_buf[2] = 0x00;				out_buf[3] = 0x03;
		out_buf[4] = buf[4];							out_buf[5] = buf[5];
		out_buf[6] = buf[6];							out_buf[7] = buf[7];
		out_buf[24] = ((len_t >> 24) & 0xff);						out_buf[25] = ((len_t >> 16) & 0xff);
		out_buf[26] = ((len_t >> 8) & 0xff);							out_buf[27] = ((len_t >> 0) & 0xff);
		_lock_set(stream->line[port_num - 1].lock);
#ifdef INTERRUPT_OUT
		tran_data_out("【中断传输】传出到中间件的数据,长度：%d\n", 48);

		printf_array(out_buf,48);
#endif
		if (send(socket_fd, out_buf, 48, MSG_NOSIGNAL)<0)
		{
			_lock_un(stream->line[port_num - 1].lock);
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		_lock_un(stream->line[port_num - 1].lock);
#endif

	}
	else if (dir_ep == stream->line[port_num - 1].in_ep)
	{

#if 1
		int i;
		for (i = 0; i < sizeof (stream->line[port_num - 1].interrupt_info_in) / sizeof(struct _interrupt_info); i++)
		{
			if (stream->line[port_num - 1].interrupt_info_in[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N_MAX * 2)
		{
			tran_data_out("中断传输in 线程达到上限！\n");
			if(NULL != usb_data)
			{
				free(usb_data);
				usb_data = NULL;
			}
			return -1;
		}
		_lock_set(interrupt_lock);
		stream->line[port_num - 1].interrupt_info_in[i].devh = stream->line[port_num - 1].devh;
		stream->line[port_num - 1].interrupt_info_in[i].ep = dir_ep;
		stream->line[port_num - 1].interrupt_info_in[i].sock = socket_fd;
		memcpy(stream->line[port_num - 1].interrupt_info_in[i].head_data, buf, 48);

		stream->line[port_num - 1].interrupt_info_in[i].Wlength = stream->line[port_num - 1].interrupt_in_maxlen;
		stream->line[port_num - 1].interrupt_info_in[i].sock_lock = stream->line[port_num - 1].lock;
		stream->line[port_num - 1].interrupt_info_in[i].out_close = &(stream->line[port_num - 1].out_close);
		stream->line[port_num - 1].interrupt_info_in[i].thread_num = &(stream->line[port_num - 1].interrupt_in_thread_num);
		stream->line[port_num - 1].interrupt_info_in[i].state = 1;

		stream->line[port_num - 1].interrupt_info_in[i].vid = stream->line[port_num - 1].vid;
		stream->line[port_num - 1].interrupt_info_in[i].pid = stream->line[port_num - 1].pid;
		stream->line[port_num - 1].interrupt_info_in[i].stream = stream;

		stream->line[port_num - 1].interrupt_info_in[i].unlink_flag = &stream->line[port_num - 1].unlink_flag;
		stream->line[port_num - 1].interrupt_info_in[i].port_num = port_num - 1;
		_lock_un(interrupt_lock);
		//tran_data_out("控制传输创建线程--tran_data_out\n");
		if (_delay_task_add("0", interr_data_from_dev, &stream->line[port_num - 1].interrupt_info_in[i], 0) < 0)
		{
			tran_data_out("创建队列失败\n");
		}
		if(NULL != usb_data)
		{
			free(usb_data);
			usb_data = NULL;
		}
		return 0;
#else
		//stream->line[port_num - 1].seqnum = buf[4] * 0x1000000 + buf[5] * 0x10000 + buf[6] * 0x100 + buf[7];
		//stream->line[port_num - 1].socket_fd = socket_fd;
		//if (stream->line[port_num - 1].interrupt_in_ep == 0x00)//给in端点赋值一次
		//{
		//	stream->line[port_num - 1].interrupt_in_ep = dir_ep;
		//}
		//record_out_l(stream->line[port_num - 1].seqnum, stream->line[port_num - 1].out_q_seqnum_array);
		//if (_delay_task_add("0", interrupt_data_to_dev, &stream->line[port_num - 1], 0)<0)
		//{
		//	tran_data_out("创建队列失败\n");
		//}
#endif

	}
	if(NULL != usb_data)
	{
		free(usb_data);
		usb_data = NULL;
	}
	return 0;
}

//static void interrupt_data_to_dev(void *arg)
//{
//	struct _connect_state *line;
//	line = arg;
//	unsigned char usb_data[204800] = { 0 };
//	unsigned char out_buf[204800] = { 0 };
//	int result;
//	int size = 0;
//	long seqnum = 0;
//	memset(out_buf, 0, sizeof(out_buf));
//	int in_ep;
//	int in_maxlen;
//	//int count = 0;
//	in_ep = line->interrupt_in_ep;
//	if ((line->vid == 0x096e && line->pid == 0x070b) || (line->vid == 0x1780 && line->pid == 0x0410))
//	{
//		in_maxlen = 64;
//	}
//	else
//	{
//		in_maxlen = line->interrupt_in_maxlen;
//	}
//	tran_data_out("创建一个线程\n");
//	tran_data_out("【interrupt_data_to_dev】读出设备in_ep = %02x,in_max_len = %d\n", in_ep, in_maxlen);
//	for (;;)
//	{
//
//		if (line->out_close != 0)
//		{
//			tran_data_out("【interrupt_data_to_dev】外部已要求关闭，停止对USB设备的读取操作,seqnum = %d, line->queue_num = %d\n", seqnum, line->queue_num);
//			line->queue_num = line->queue_num - 1;
//			return;
//		}
//
//		result = libusb_interrupt_transfer(line->devh, in_ep, usb_data, in_maxlen, &size, 1000);//1000
//		if (result < 0)
//		{
//			if (line->unlink_flag == 1)
//			{
//				printf("interrupt_data_to_dev unlink!\n");
//				libusb_clear_halt(line->devh, in_ep);
//				line->unlink_flag = 0;
//				break;//公网加载的时候发现可能存在unlink的情况
//			}
//
//			if (result == LIBUSB_ERROR_TIMEOUT)
//			{
//				//tran_data_out("【interrupt_data_to_dev】发送失败,超时！result = %d\n", result);
//				if (size != 0)
//				{
//					tran_data_out("中断超时，transfered为:%d\n", size);
//					break;
//				}
//				continue;
//			}
//			tran_data_out("【interrupt_data_to_dev】发送失败,清空该端点再次发送,result = %d  要读取的长度:%d,size:%d\n", result, in_maxlen, size);
//
//			if (result == LIBUSB_ERROR_PIPE)
//			{
//				tran_data_out("中断传输 in LIBUSB_ERROR_PIPE\n");
//				libusb_clear_halt(line->devh, in_ep);
//				continue;
//			}
//			else
//			{
//				line->queue_num = line->queue_num - 1;
//				tran_data_out("异常退出线程！\n");
//				return;
//			}
//
//
//		}
//		else
//		{
//			tran_data_out("【interrupt_data_to_dev】中断传输完成！\n");
//			break;
//		}
//	}
//	if (size > 0)
//		tran_data_out("中断传输in读取到%d个数据\n", size);
//	if (line->vid == 0x1780 && line->pid == 0x0410)
//		size = 64;
//	get_out((unsigned long *)&seqnum, line->out_q_seqnum_array);
//	check_out(seqnum, line->out_q_seqnum_array);
//	line->queue_num = line->queue_num + 1;
//	//tran_data_out("序列号为:%d\n", seqnum);
//	//sleep(3);
//	out_buf[0] = 0x00;			out_buf[1] = 0x00;				out_buf[2] = 0x00;				out_buf[3] = 0x03;
//	out_buf[4] = ((seqnum >> 24) & 0xff);						out_buf[5] = ((seqnum >> 16) & 0xff);
//	out_buf[6] = ((seqnum >> 8) & 0xff);						out_buf[7] = ((seqnum >> 0) & 0xff);
//	out_buf[24] = ((size >> 24) & 0xff);						out_buf[25] = ((size >> 16) & 0xff);
//	out_buf[26] = ((size >> 8) & 0xff);							out_buf[27] = ((size >> 0) & 0xff);
//	if (size >0)
//		memcpy(out_buf + 48, usb_data, size);
//	_lock_set(line->lock);
//
//
//	if (send(line->socket_fd, out_buf, 48 + size, MSG_NOSIGNAL)<0)
//	{
//		line->queue_num = line->queue_num - 1;
//		_lock_un(line->lock);
//		return;
//	}
//#ifdef INTERRUPT_OUT
//	int i;
//	tran_data_out("【interrupt_data_to_dev】---------!!!!!!!传出到中间件的数据:size:%d\n", size);
//	for (i = 0; i < 48 + size; i++)
//	{
//		printf("%02x ", out_buf[i]);
//	}
//	printf("\n\n\n");
//#endif
//	_lock_un(line->lock);
//
//	line->queue_num = line->queue_num - 1;
//	//_scheduled_task_week_up(line->interrupt_task);
//	return;
//}
//







