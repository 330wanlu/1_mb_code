#define _tran_data_c
#include "_tran_data.h"
int tran_data_add(int port, const _so_note    *note)
{
	struct _tran_data *stream;
	int result;
	stream = malloc(sizeof(struct _tran_data));
	if (stream == NULL)
		return DF_ERR_MEM_ERR;
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
			out("[_socket_sev_add]����ʧ��\n");
			return -1;
		}
		memset(name, 0, sizeof(name));
		sprintf(name, "/dev/socket_sev/sever/%d", stream->tran_port);
		stream->sock_fd = ty_open(name, 0, 0, NULL);
		if (stream->sock_fd<0)
		{
			out("���ļ�:/dev/socket_sev/sever/3240 ʧ��\n");
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
			out("���ļ�[%s]ʧ��:[%d]\n", "/dev/ty_usb", stream->ty_usb_fd);
			return stream->ty_usb_fd;
		}
		stream->usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		if ((stream->usb_port_numb < 0) || (stream->usb_port_numb > DF_TY_USB_PORT_N))
		{
			out("�˿ڴ���\n");
			return stream->ty_usb_fd;
		}
		for (i = 0; i < stream->usb_port_numb; i++)
		{
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, i + 1, stream->data[i].busid);
			if (result<0)
			{
				out("[open_port]��ȡ�˿�[%d]busidʧ��\n", i);
				return result;
			}
			stream->data[i].port = i + 1;
			stream->data[i].state = 1;
		}

		init_parm(stream->note);
		//out("======socket tran server file open success======\n");		
		stream->lock = _lock_open("tran_data", NULL);
		if (stream->lock<0)
		{
			out("������ʧ��\n");
			return stream->lock;
		}
		bulk_lock = _lock_open("bulk_lock", NULL);
		if (bulk_lock<0)
		{
			out("������ʧ��\n");
			return bulk_lock;
		}
		control_lock = _lock_open("control_lock", NULL);
		if (control_lock<0)
		{
			out("������ʧ��\n");
			return control_lock;
		}
		interrupt_lock = _lock_open("interrupt_lock", NULL);
		if (interrupt_lock<0)
		{
			out("������ʧ��\n");
			return interrupt_lock;
		}
		for (i = 0; i<sizeof(stream->line) / sizeof(stream->line[0]); i++)
			stream->line[i].lock = _lock_open("s", NULL);
		//out("==============================data transmit server start success,going to monitor mode=================================\n");
		//stream->task = _task_open("tran_data",_deal_line_heart_task,stream,1000,NULL);
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
	out("��tran_data�ɹ�:j=%d\n", j);
	return j + 1;
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
	//out("ty_usb_m_ctl:cm=%d\n",cm);
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

/**********׼��ת����·������***********/
static int prepare_connect_packet(char *sbuf, int *len, struct _new_usbshare_parm  *parm)
{
	char sendbuf[100] = { 0 };
	//char port[10] = { 0 };
	//int result;
	long u_sec = 0;
	u_sec = get_time_sec();
	memcpy(sendbuf, "\x01\x06", 2);//�汾��Ϣ

	memset(parm->client_id, 0, sizeof(parm->client_id));
	sprintf(parm->client_id, "%s%ld", parm->mechine_id, u_sec);

	*len += 2;
	sendbuf[2] = 0x00;
	sendbuf[3] = (strlen(parm->client_id) + 6) & 0xff;//2λ���� 0x5e
	*len += 2;

	memcpy(sendbuf + *len, "\x00\x01", 2);//������-00 01 ������������
	*len += 2;

	memcpy(sendbuf + *len, parm->client_id, strlen(parm->client_id));
	*len += strlen(parm->client_id);
	printf("################key is:%s---len is:%d------------################\n", parm->client_id, *len);
	memcpy(sbuf, sendbuf, *len);
	return 0;

}

/*********����ת����·�˿ڼ��ؾ��嶯��*************/
static int trans_usb_share(struct _tran_data    *stream, int port, int fd)
{
	int result, i;
	//int deal_way=0;
	int port_num;
	//int result1,result2;
	uint32 ver_code;
	uint8 busid[32];
	memset(busid, 0, sizeof(busid));
	int client_fd;
WAIT_8003:
	ver_code = recv_midware_ver_code(fd);
	if (ver_code == DF_MID_TER_8005)
	{
		//out("����8005 0005���\n");
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
		out("8003�������\n");
	}
	else
		return 0;

	port_num = port;

	struct _transmit_sign	line1, line2;
	int task2;
	sem_t	sem;

	sem_init(&sem, 0, 0);
	memset(&line1, 0, sizeof(line1));
	memset(&line2, 0, sizeof(line2));
	line1.sem = &sem;
	line1.socket_fd = client_fd;
	line2.sem = &sem;
	line2.socket_fd = fd;
	port_num--;
	out("��ȡ�������ͺͶ˵�\n");
	result = usb_dev_init(stream, port_num + 1, busid);
	if (result < 0)
	{
		out("USB�豸��ʼ��ʧ��\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}
	out("trans_usb_share ��������Ϊ��%d��in_ep = %02x out_ep = %02x,in�������֡����= %d,out�������֡����= %d\n", stream->line[port_num].tran_type, stream->line[port_num].in_ep, stream->line[port_num].out_ep, stream->line[port_num].in_max_len, stream->line[port_num].out_max_len);
	memcpy(stream->line[port_num].busid, busid, strlen((char *)busid));
	get_usb_vid_pid(port_num + 1, &stream->line[port_num].vid, &stream->line[port_num].pid);
	out("�����м�����ݼ���߳�\n");
	task2 = _task_open("main_line", line_task, &line2, 1, NULL);
	_lock_set(stream->lock);
	stream->line[port_num].state_mem = 1;
	stream->line[port_num].out_close = 0;
	stream->line[port_num].unlink_flag = 0;
	stream->line[port_num].sem = &sem;
	_lock_un(stream->lock);
	struct _train_data_oper	line1_data, line2_data;

	long out_q_seqnum_array[OUT_Q_LEN];
	memset(&line1_data, 0, sizeof(line1_data));
	memset(out_q_seqnum_array, 0, sizeof(out_q_seqnum_array));
	memset(&line2_data, 0, sizeof(line2_data));
	for (;;)
	{
		sem_wait(&sem);
		if (stream->line[port_num].out_close != 0 && stream->line[port_num].out_close != 2)
		{
			out("ִ���ⲿ�ر�����\n");
			break;
		}


		if ((result = oper_data(fd, client_fd, &line1_data, out_q_seqnum_array, stream, port_num + 1))<0)
		{
			out("oper_data��⵽�м����·����,result = %d\n", result);
			break;
		}
		set_port_usb_data_tran(port_num + 1);


		_task_week_up(task2);


	}
	out("�˳�ת��ѭ��,ִ�йر�����\n");
	if (stream->line[port_num].tran_type == 3)
	{
		out("�ȴ������жϽ��ն����˳�\n");
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
			out("libusb_release_interface result = %d\n", result);
		}

		if (stream->line[port_num].interface_decs[1].state != 0)//��ʾΪ˫�ӿ��豸
		{
			result = libusb_release_interface(stream->line[port_num].devh, 1);
		}
	}

	for (i = 0; i < 2; i++)
		memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));
	stream->line[port_num].out_close = 1;

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_in_thread_num == 0)
		{
			out("���ƴ���in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_out_thread_num == 0)
		{
			out("���ƴ���out�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_in_thread_num == 0)
		{
			out("�鴫��in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_out_thread_num == 0)
		{
			out("�鴫��out�߳�ȫ���˳����\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_out_thread_num == 0)
		{
			out("�жϴ���out�߳�ȫ���˳����\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_in_thread_num == 0)
		{
			out("�жϴ���in�߳�ȫ���˳����\n");
			break;
		}
	}
	//�ȴ��߳��˳�

	if (stream->line[port_num].bulk_in_thread_num != 0 || \
		stream->line[port_num].bulk_out_thread_num != 0 || \
		stream->line[port_num].control_in_thread_num != 0 || \
		stream->line[port_num].control_out_thread_num != 0 || \
		stream->line[port_num].interrupt_out_thread_num != 0 || \
		stream->line[port_num].interrupt_in_thread_num != 0)
	{
		write_log("���¼���devhʱ���߳�δ�˳�:bulk_in:%d\tbulk_out:%d\tctl_in:%d\tctrl_out:%d\t!\n", stream->line[port_num].bulk_in_thread_num, stream->line[port_num].bulk_out_thread_num, \
			stream->line[port_num].control_in_thread_num, stream->line[port_num].control_out_thread_num);
	}

	out("trans_usb_share����usbshareʹ��\n");
	if (stream->line[port_num].devh != NULL)
	{
		out("trans_usb_share����usbshareʹ��,�ر�%d�Ŷ˿ھ��\n", port_num+1);
		libusb_close(stream->line[port_num].devh);
		stream->line[port_num].devh = NULL;
	}
	out("����libusb���óɹ�,%08x \n", stream->line[port_num].devh);
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
	_task_close(task2);
	printf("\n\nexit trans_usb_share over !\n\n");
	out("trans_usb_share,ִ�йر�������ɣ��˿ں�Ϊ��%d��\n\n\n\n", port_num + 1);
	return 0;

}
/********�ȴ�����8007�Ļ�ȡbusid����*********/
static int wait_for_8007_busid(int fd, struct _tran_data        *tran_stream)
{
	uint32 ver_code;
	int result;
	uint32 len_data;
	int port = 0;
	uint8 recv_port[10] = { 0 };
	uint8 busid[20] = { 0 };
	uint8 send_tmp[1024] = { 0 };
	ver_code = recv_midware_ver_code(fd);
	if (ver_code != 0X01068007)
	{
		out("not 8007\n");
		return -1;
	}
	len_data = recv_data_len(fd);
	if (len_data <= 0)
	{
		//���ش���
		out("recv_data_len err\n");
		return -1;
	}
	//����ʵ��port
	if (ty_socket_read(fd, recv_port, len_data)<0)
	{
		out("��ȡ����\n");
		return -1;
	}

	port = atoi((char*)recv_port);
	out("recv port is :%d\n", port);
	ty_ctl(tran_stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, busid);
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
/********����ת����·����ҵ��************/
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
	//out("")
	transportline = arg;
	fd = transportline->fd;
	tran_stream = transportline->stream;
	parm = &transportline->parm;


	//����Э��ͷ
	ver_code = recv_midware_ver_code(fd);
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
	//����json����
	len_data = recv_data_len(fd);
	if (len_data <= 0)
	{
		//���ش���
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
	//����ʵ��json
	if (ty_socket_read(fd, recvjson, len_data)<0)
	{
		out("��ȡport_str����\n");
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
	out("recv data:%s\n", recvjson);
	result = deal_cmd_newshare(fd, recvjson, str_port, 0);
	if (result <0)
	{
		out("deal_cmd_newshare err");
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
	out("deal_cmd_newshare result:%d--datalock :%d\n", result, parm->data_lock);
	if (result == RESULT_OK_FOR_APPLY_PORT_USE_GW)//��������
	{

		//char bus_id[20] = { 0 }, send_tmp[200] = { 0 };
		//out("����������000������--port is :%d\n", dev_port);
		dev_port = atoi(str_port);
		_lock_set(parm->data_lock);
		parm->type = 1;
		parm->handle = fd;
		parm->port = dev_port;
		_lock_un(parm->data_lock);
		out("���������У�����--port is :%d\n", dev_port);

		result = wait_for_8007_busid(fd, tran_stream);//�ȴ����մ�����ȡbusid����
		if (result < 0)
		{
			out("##############################wait_for_8007_busid##########################################\n");
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

		result = trans_usb_share(tran_stream, dev_port, fd);
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
/**********��ȡһ��ת����·�������*********/
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

	for (i = 0; i < DF_TCP_SER_CONNECT_LINE_NUM; i++)
	{
		if (stream->transport_line[i].state == 0)
			break;
	}
	if (i >= DF_TCP_SER_CONNECT_LINE_NUM)
	{
		out("��·����!\n");
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
	prepare_connect_packet((char*)c_buf, &len, &parm);
	memset(parmtmp->client_id, 0, sizeof(parmtmp->client_id));//new add by whl
	memcpy(parmtmp->client_id, parm.client_id, strlen(parm.client_id));
	memcpy(ip, parm.trans_server, strlen(parm.trans_server));

	//stream->transport_line[i].fd = socket_tcp_open_send("103.27.4.61", 999, c_buf, len);
	stream->transport_line[i].fd = socket_tcp_open_send(ip, 999, c_buf, len);
	if (stream->transport_line[i].fd < 0)
	{
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
	result = ty_socket_read(stream->transport_line[i].fd, conn_recv, 8);
	if (result<0)
	{
		out("-----��ת��������������ʧ��\n");
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
	if (_queue_add("transport", deal_trans_line_oper, &stream->transport_line[i], 0)<0)
	{
		out("��������ʧ��\n");
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
	//out("��ȡ:[%s]��usbip��Ϣ\n",busid);
	result = _usb_file_get_infor(busid, &file->dev);
	if (result<0)
	{
		//out("���豸��������λ\n");
		return -1;
	}
	memcpy(file->busid, busid, strlen(busid));

	return 0;
}
static int deal_8009_0009(struct _tran_data *stream, int fd, int *port, uint32 ip)
{

	uint8 state[4];
	uint8 port_str[4];
	//int client_fd;
	int result;
	//int buf_len;
	struct op_devlist_reply reply;
	struct _usbip_file  *usbip[DF_TY_USB_PORT_N];
	struct usb_device   device;
	struct usb_interface    interface;
	int i, n, j, err = -1;
	uint8 s_buf[81920];
	memset(s_buf, 0, sizeof(s_buf));

	uint8 abuf[2000];
	uint8 bbuf[81920];
	char busid[50] = { 0 };
	memset(abuf, 0, sizeof(abuf));
	memset(bbuf, 0, sizeof(bbuf));
	memset(state, 0, sizeof(state));
	//out("����·Ϊ3240�״λ�ȡ��Ϣ����,��ȡ�����ֽ�\n");
	//��������������̶ֹ�102���ֽ�
	if (ty_socket_read(fd, state, 4)<0)
	{
		out("��ȡ״̬����\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		out("״̬�ֲ���\n");
		return -1;
	}

	if (ty_socket_read(fd, port_str, 4)<0)
	{
		out("��ȡ�˿ڴ���\n");
		return -1;
	}
	*port = port_str[3];

	printf("########################RECV PORT IS :%d########################\n", *port);
	int app_fd;
	char client[128] = {0};
	logout(INFO, "tcp", "attach", "deal_8009_0009,����USB�˿�,%d\r\n", *port);
	result = open_port_attach(*port, ip, client, &app_fd);
	if (result < 0)
	{
		if (result == DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION)
		{
			ty_socket_write(fd, (uint8 *)"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01", 8);
		}
		return result;
	}


	get_usb_busid(*port, busid);//��ȡbusid

	_lock_set(stream->lock);
	out("��ȡ�ɼ��ص��豸����\n");
	for (i = 0, memset(&reply, 0, sizeof(reply)); i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		//out("�����ڴ� ndev = %d\n",reply.ndev);
		usbip[reply.ndev] = malloc(sizeof(struct _usbip_file));
		//out("�����ڴ�ɹ� ndev = %d\n",reply.ndev);
		if (usbip[reply.ndev] == NULL)
		{
			out("�����ڴ�ʧ��\n");
			_lock_un(stream->lock);
			n = reply.ndev;
			goto _ERRO;
		}
		memset(usbip[reply.ndev], 0, sizeof(struct _usbip_file));
		if (_usbip_file_read(stream->data[i].busid, usbip[reply.ndev])<0)
		{
			//out("��ȡusb��Ϣʧ��\n");
			free(usbip[reply.ndev]);
			continue;
		}
		//out("�ɼ����豸busid = %s,�˿ں� = %d\n", stream->data[i].busid,0);
		reply.ndev++;
	}
	_lock_un(stream->lock);
	n = reply.ndev;
	out("һ����:%d���豸��Ҫ���ظ��Է�\n", n);
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
		get_usb_dev_pdu(&usbip[i]->dev, usbip[i]->busid, &device);
		if (memcmp(usbip[i]->busid, busid, strlen(busid)) != 0)//ƥ�䵽ͬһ��busid��ʱ��ſ���
		{

			continue;
		}
		out("�ҵ�ƥ���busid:%s\n", busid);
		//out("����usb�豸��Ϣ\n");
		pack_usb_device(0, &device);
		memcpy(s_buf + seek, &device, sizeof(struct usb_device));
		seek = seek + sizeof(struct usb_device);
		//if(ty_socket_write(fd,&device,sizeof(device))<0)
		//    goto _ERRO;
		//out("USB�ӿ��豸��Ϣ:%d\n",usbip[i]->dev.bNumInterfaces);
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
	//out("seek len:%d\n", seek);
	//for (i = 0; i < seek; i++)
	//{
	//	printf("%02x ", s_buf[i]);
	//}
	//printf("\n\n");
	if (ty_socket_write(fd, s_buf, seek)<0)
		goto _ERRO;
	err = 0;
_ERRO:
	for (i = 0; i<n; i++)
	{  //out("�ͷ�USBIP�ڴ�\n"); 
		free(usbip[i]);
	}
	return err;

}
static int deal_8005_0005(struct _tran_data    *stream, int fd)
{
	uint8 state[4];
	struct op_devlist_reply reply;
	struct _usbip_file  *usbip[DF_TY_USB_PORT_N];
	struct usb_device   device;
	struct usb_interface    interface;
	int i, n, j, err = -1;
	uint8 s_buf[81920];
	memset(s_buf, 0, sizeof(s_buf));

	if (ty_socket_read(fd, state, 4)<0)
	{
		out("��ȡ״̬����\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		out("״̬�ֲ���\n");
		return -1;
	}


	_lock_set(stream->lock);
	out("��ȡ�ɼ��ص��豸����\n");
	for (i = 0, memset(&reply, 0, sizeof(reply)); i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		//out("�����ڴ� ndev = %d\n",reply.ndev);
		usbip[reply.ndev] = malloc(sizeof(struct _usbip_file));
		//out("�����ڴ�ɹ� ndev = %d\n",reply.ndev);
		if (usbip[reply.ndev] == NULL)
		{
			out("�����ڴ�ʧ��\n");
			_lock_un(stream->lock);
			n = reply.ndev;
			goto _ERRO;
		}
		memset(usbip[reply.ndev], 0, sizeof(struct _usbip_file));
		if (_usbip_file_read(stream->data[i].busid, usbip[reply.ndev])<0)
		{
			//out("��ȡusb��Ϣʧ��\n");
			free(usbip[reply.ndev]);
			continue;
		}
		reply.ndev++;
	}
	_lock_un(stream->lock);
	n = reply.ndev;
	out("һ����:%d���豸��Ҫ���ظ��Է�\n", n);
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
		memset(&device, 0, sizeof(device));
		get_usb_dev_pdu(&usbip[i]->dev, usbip[i]->busid, &device);
		//out("����usb�豸��Ϣ\n");
		pack_usb_device(0, &device);
		memcpy(s_buf + seek, &device, sizeof(device));
		seek = seek + sizeof(device);
		//if(ty_socket_write(fd,&device,sizeof(device))<0)
		//    goto _ERRO;
		//out("USB�ӿ��豸��Ϣ:%d\n",usbip[i]->dev.bNumInterfaces);
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
_ERRO:
	for (i = 0; i<n; i++)
	{  //out("�ͷ�USBIP�ڴ�\n"); 
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
	memset(abuf, 0, sizeof(abuf));
	memset(bbuf, 0, sizeof(bbuf));
	memset(state, 0, sizeof(state));
	//memset(busid,0,sizeof(busid));
	if (ty_socket_read(fd, state, 4)<0)
	{
		out("��ȡ״̬����\n");
		return -1;
	}
	if ((state[0] != 0x00) && (state[1] != 0x00) && (state[2] != 0x00) && (state[3] != 0x00))
	{
		out("״̬�ֲ���\n");
		return -1;
	}
	struct usb_device   dev;
	int i;
	//int num;
	out("��ȡ[busid]\n");
	if (ty_socket_read(fd, (uint8 *)busid, DF_TY_USB_BUSID)<0)
	{
		out("net��ȡbusidʧ��\n");
		return -1;
	}
	out("���busid=%s\n", busid);
	_lock_set(stream->lock);
	for (i = 0; i<sizeof(stream->data) / sizeof(stream->data[0]); i++)
	{
		if (stream->data[i].state == 0)
			continue;
		if (memcmp(stream->data[i].busid, busid, DF_TY_USB_BUSID) != 0)
			continue;

		break;
	}
	if (i == sizeof(stream->data) / sizeof(stream->data[0]))
	{
		_lock_un(stream->lock);
		out("û�����USB��\n");
		return -1;
	}
	//num = i;
	memset(&dev, 0, sizeof(dev));
	out("��ȡusb�豸����Ϣ\n");
	if (get_usbip_dev((char *)busid, &dev)<0)
	{
		_lock_un(stream->lock);
		out("��ȡ[%s]��Ϣʧ��\n", busid);
		return -1;
	}
	out("д�豸��socket=%d\n", fd);


	_lock_un(stream->lock);
	out("�豸��ʼö��\n");
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
		out("���ļ�ʧ��\n");
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
	//����json����
	len_data = recv_data_len(sockfd);
	if (len_data <= 0)
	{
		//���ش���
		return -1;
	}
	//����ʵ��json
	recvjson = (unsigned char *)malloc(len_data + 10);
	if (recvjson == NULL)
	{
		return -1;
	}
	memset(recvjson, 0, len_data + 10);
	if (ty_socket_read(sockfd, recvjson, len_data)<0)
	{
		out("��ȡjson����\n");
		free(recvjson);
		return -2;
	}
	//out("recv data:%s\n", recvjson);
	if (strstr((char *)recvjson, "ter_restart") != NULL)
	{
		ip2ip_asc(ip, ip_asc);
		write_log("�ͻ��˷���������IP:%s\n", ip_asc);
	}
	result = deal_cmd_newshare(sockfd, recvjson, output, 1);
	if (result <0)
	{
		out("deal_cmd_newshare err");
		free(recvjson);
		return -1;
	}
	free(recvjson);
	return 0;
}
#endif

//���̻߳ص�
static int _transmit_order_hook(int fd, uint32 ip, uint16 port, void *arg)
{
	struct _tran_data    *stream;
	int result, i;
	int port_num;
	uint32 ver_code;
	uint8 busid[32];
	stream = arg;
	memset(busid, 0, sizeof(busid));
	int client_fd;
	//out("�жϴ�������Ϊ��һ����·���ǵڶ�����·\n");
	ver_code = recv_midware_ver_code(fd);
	if (ver_code == DF_MID_TER_8005)
	{
		//out("����8005 0005���\n");

		return	deal_8005_0005(stream, fd);

	}
	else if (ver_code == DF_MID_TER_8003)
	{
		client_fd = deal_8003_0003(stream, fd, busid);
		if (client_fd < 0)
		{
			return client_fd;
		}
		out("8003�������\n");
	}
	else if (ver_code == DF_DLL_TER_8009)//3240 ֱ��ͨѶ��
	{
		int port = 0;

		result = deal_8009_0009(stream, fd, &port,ip);
		return result;
	}
#ifdef NEW_USB_SHARE
	else if (ver_code == 0X01068007)//3240 �����
		return deal_new_protocol(fd, ip);
#endif
	else
		return 0;

	port_num = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_BUSID_PORT, busid);
	if (port_num<1 || port_num>DF_TY_USB_PORT_N)
	{
		out("��ȡ���Ķ˿ںŴ���\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}


	struct _transmit_sign	line1, line2;
	int task2;
	sem_t	sem;

	sem_init(&sem, 0, 0);
	memset(&line1, 0, sizeof(line1));
	memset(&line2, 0, sizeof(line2));
	line1.sem = &sem;
	line1.socket_fd = client_fd;
	line2.sem = &sem;
	line2.socket_fd = fd;
	//port_num--;


	out("��ȡ�������ͺͶ˵�\n");
	result = usb_dev_init(stream, port_num, busid);
	if (result < 0)
	{
		out("USB�豸��ʼ��ʧ��\n");
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
		return -1;
	}
	out("_transmit_order_hook��ʼusbshareʹ��,��%d�Ŷ˿ھ��,�����%08x\n", port_num, stream->line[port_num - 1].devh);
	out("_transmit_order_hook ��������Ϊ��%d��in_ep = %02x out_ep = %02x,in�������֡����= %d,out�������֡����= %d\n", 
		stream->line[port_num-1].tran_type, stream->line[port_num-1].in_ep, stream->line[port_num-1].out_ep, stream->line[port_num-1].in_max_len, stream->line[port_num-1].out_max_len);
	memcpy(stream->line[port_num-1].busid, busid, strlen((char *)busid));
	get_usb_vid_pid(port_num, &stream->line[port_num-1].vid, &stream->line[port_num-1].pid);

	out("�����м�����ݼ���߳�\n");
	task2 = _task_open("main_line", line_task, &line2, 1, NULL);
	_lock_set(stream->lock);
	stream->line[port_num-1].state_mem = 1;
	stream->line[port_num-1].unlink_flag = 0;
	stream->line[port_num-1].out_close = 0;
	stream->line[port_num-1].sem = &sem;
	_lock_un(stream->lock);
	struct _train_data_oper	line1_data, line2_data;

	long out_q_seqnum_array[OUT_Q_LEN];
	memset(&line1_data, 0, sizeof(line1_data));
	memset(out_q_seqnum_array, 0, sizeof(out_q_seqnum_array));
	memset(&line2_data, 0, sizeof(line2_data));
	for (;;)
	{
		sem_wait(&sem);
		if (stream->line[port_num-1].out_close != 0 && stream->line[port_num-1].out_close != 2)
		{
			out("ִ���ⲿ�ر�����\n");
			break;
		}


		if ((result = oper_data(fd, client_fd, &line1_data, out_q_seqnum_array, stream, port_num))<0)
		{
			out("oper_data��⵽�м����·����,result = %d\n", result);
			break;
		}
		set_port_usb_data_tran(port_num);


		_task_week_up(task2);


	}
	out("�˳�ת��ѭ��,ִ�йر�����\n");
	if (stream->line[port_num-1].tran_type == 3)
	{
		out("�ȴ������жϽ��ն����˳�\n");
		for (i = 0; i<3000; i++, usleep(1000))
		{
			if (stream->line[port_num-1].queue_num == 0)
				break;
		}

	}
	if (stream->line[port_num-1].devh != NULL)
	{
		result = libusb_release_interface(stream->line[port_num-1].devh, 0);
		if (result < 0)
		{
			out("libusb_release_interface result = %d\n", result);
		}

		if (stream->line[port_num-1].interface_decs[1].state != 0)//��ʾΪ˫�ӿ��豸
		{
			result = libusb_release_interface(stream->line[port_num-1].devh, 1);
		}
	}

	for (i = 0; i < 2; i++)
		memset(&stream->line[port_num-1].interface_decs[i], 0, sizeof(struct _interface_dec));
	stream->line[port_num-1].out_close = 1;

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].control_in_thread_num == 0)
		{
			out("���ƴ���in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].control_out_thread_num == 0)
		{
			out("���ƴ���out�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].bulk_in_thread_num == 0)
		{
			out("�鴫��in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num-1].bulk_out_thread_num == 0)
		{
			out("�鴫��out�߳�ȫ���˳����\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num - 1].interrupt_out_thread_num == 0)
		{
			out("�鴫��out�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num - 1].interrupt_in_thread_num == 0)
		{
			out("�鴫��in�߳�ȫ���˳����\n");
			break;
		}
	}
	//�ȴ��߳��˳�
	out("_transmit_order_hook����usbshareʹ��\n");
	if (stream->line[port_num-1].devh != NULL)
	{
		out("_transmit_order_hook����usbshareʹ��,�ر�%d�Ŷ˿ھ��,�����%08x\n", port_num, stream->line[port_num - 1].devh);
		libusb_close(stream->line[port_num-1].devh);
		out("_transmit_order_hook����usbshareʹ��,�ر�%d�Ŷ˿ھ���ɹ�\n", port_num);
		stream->line[port_num-1].devh = NULL;
	}
	out("_transmit_order_hook����usbshareʹ��,close end!\n");
	//libusb_exit(stream->line[port_num-1].context);
	out("_transmit_order_hook����usbshareʹ��,libusb_exit end!\n");
	out("����libusb���óɹ� \n");
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
	_task_close(task2);
	out("_transmit_order_hook,ִ�йر�������ɣ��˿ں�Ϊ��%d��\n\n\n\n", port_num);
	return 0;

}




static int oper_data(int src_line, int dest_line, struct _train_data_oper	*data, long *out_q_seqnum_array, struct _tran_data	*stream, int port_num)
{
	int len;
	int result;
	//int i;
	int ret = 0;
	uint8 *buf_1;
	uint8 usbip_data[204800];
	int usbip_len;
	int len1;
	int direction;
	int ep;


	if (data->buf_len<48)
	{
		len = recv(src_line, data->buf + data->buf_len, 48 - data->buf_len, MSG_DONTWAIT);
		if (len == 0)
		{
			out("��·�ر�1\n");
			return -1;
		}
		if (len == -1)
		{
			//	out("����Ϊ err again:%d!\n", errno);
			return 0;
		}
		if (len<0)
		{
			out("��·�ر�2\n");
			return -1;

		}
		data->buf_len += len;
		if (data->buf_len < 48)
		{
			out("���յ���ͷ����48�ֽ�!\n");
			return data->buf_len;
		}
	}
	len = get_head_size(data->buf);
	buf_1 = data->buf;
	if (get_inlen_par(data->buf, out_q_seqnum_array) < 0)
	{
		out("get_inlen_par error\n");
		//out("data->buf_len = %d\n", data->buf_len);
		printf_array((char *)data->buf, data->buf_len);
		return -2;
	}

	int transfer_flags = data->buf[20] * 0x1000000 + data->buf[21] * 0x10000 + data->buf[22] * 0x100 + data->buf[23];
	int command = data->buf[3];
	if (command == USBIP_CMD_UNLINK)
	{
		if (stream->line[port_num - 1].unlink_num < 2)
		{
			out("################unlink cancel urb request!##############\n");//Ŀǰ���ֽ�˰�̹������ص�ʱ�򾭳������������󣬲�ȡ�ô�ʩ���Խ��
			//Unlink = 1;
			unsigned char unlink_data[1000] = { 0 };
			memcpy(unlink_data, "\x00\x00\x00\x04", 4);//ret unlink
			memcpy(unlink_data + 4, data->buf + 4, 16);
			memcpy(unlink_data + 20, "\x00\x00\x00\x01", 4);
			memcpy(unlink_data + 24, data->buf + 24, 24);

			stream->line[port_num - 1].unlink_flag = 1;

			//stream->line[port_num - 1].unlink_num++;
			//sleep(2);//�ȴ��̴߳���
			//reload_devh(stream, port_num - 1);
			if (send(src_line, unlink_data, 48, MSG_NOSIGNAL)<0)
			{
				return -1;
			}
			ret = 0;
			goto end;
		}
		else
		{
			out("unlink too many times!\n");
			return -1;
		}
	}
	if ((len + 48)>sizeof(data->buf))
	{
		out("oper_data,err need relloc mem:%d\n", len);
		
		printf_array((char *)data->buf,48);

		//if (data->buf[15] == USBIP_DIR_OUT && ((transfer_flags & URB_ZERO_PACKET) != URB_ZERO_PACKET) && data->buf[19] != 0x00)//��ʾout�����豸д���豸,�Ҳ���0�ֽڰ�,��������urb����
		if (data->buf[15] == USBIP_DIR_OUT && ((transfer_flags & URB_ZERO_PACKET) != URB_ZERO_PACKET))//ĳһ���ֱ���caֻ�п��ƴ���0�˵� 
		{

			{ out("oper_data,�������ڴ�\n");
			if (data->re_buf == NULL)
			{
				data->re_buf = malloc(len + 48);
				if (data->re_buf == NULL)
				{
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
			goto oper;
		}



		

		if (data->re_buf == NULL)
		{
			data->re_buf = malloc(len + 48);

			if (data->re_buf == NULL)
			{
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
		len1 = recv(src_line, buf_1 + data->buf_len, len + 48 - data->buf_len, MSG_DONTWAIT);
		if ((len1 == 0) || (len1<-1))
		{
			if (data->re_buf != NULL)
				free(data->re_buf);
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
		return data->buf_len;
	}

oper:


	if ((buf_1[0] == 0x00) && (buf_1[1] == 0x00) && (buf_1[2] == 0x00) && (buf_1[3] == 0x05))
	{
		//out("�������ݲ�����ֱ�ӽ���\n");
		ret = 0;
		goto data_end;
	}
	//out("�жϸ����ݵĴ���˿�\n");
	get_ep_direction((char *)buf_1, &ep, &direction);
	//out("��ȡ����ep = %d,direction = %d\n",ep,direction);
	memset(usbip_data, 0, sizeof(usbip_data));
	//if (stream->line[port_num - 1].vid == 0X096e && stream->line[port_num - 1].pid == 0X070b)
	{
		//out("��������epΪ:%02x\n", ep);
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
			//out("��֡����Ϊ���ƴ���\n");
			result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
			if (result < 0)
			{
				ret = -1;
			}
			goto end;
		}
		else
		{
			//out("�ǿ��ƴ���ɸѡ:ep:%02x\n",ep);
			int i;
			int j;
			int find = 0;
			for (i = 0; i < 2 && find == 0; i++)
			{
				if (stream->line[port_num - 1].interface_decs[i].state == 1)
				{
					for (j = 0; j < 3; j++)
					{

						if (stream->line[port_num - 1].interface_decs[i].point_decs[j].state == 1 && ep == stream->line[port_num - 1].interface_decs[i].point_decs[j].bEndpointAddress)
						{
							stream->line[port_num - 1].tran_type = stream->line[port_num - 1].interface_decs[i].point_decs[j].bmAttributes & 0x03;
							stream->line[port_num - 1].in_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
							stream->line[port_num - 1].out_max_len = stream->line[port_num - 1].interface_decs[i].point_decs[j].wMaxPacketSize;
							if (stream->line[port_num - 1].interrupt_in_maxlen == 0x00 && stream->line[port_num - 1].tran_type == 0x03)//�жϴ�������ֵδ��ֵ����ʼ״̬
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
				out("δ�ҵ�ƥ��Ķ˵㣺ep:%02x \n", ep);
				ret = -1;
				goto end;
			}
		}//end else
	}


	if (ep == 0)
	{
		//out("��֡����Ϊ���ƴ���\n");
		result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
		if (result < 0)
		{
			ret = -1;
		}
	}
	else
	{
		//out("�˵��0�����жϴ�������\n");
		switch (stream->line[port_num - 1].tran_type)
		{
		case 0:
			//out("��������Ϊ���ƴ���\n");
			result = control_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
			if (result < 0)
			{
				out("��0�˵���ƴ���ʧ��\n");
				ret = -11;
			}
			break;
		case 1:
			//out("��������Ϊͬ������\n");
			ret = -12;
			break;
		case 2:
			//out("��������Ϊ�鴫��\n");
			result = bulk_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
			if (result < 0)
			{
				out("��0�˵�鴫��ʧ��\n");
				ret = -13;
			}
			break;
		case 3:
			//out("��������Ϊ�жϴ���\n");
			result = interrupt_transfer(stream, port_num, (unsigned char *)buf_1, data->buf_len, (unsigned char *)usbip_data, &usbip_len, src_line);
			if (result < 0)
			{
				out("��0�˵�����жϴ���\n");
				ret = -14;
			}
			break;
		default:
			out("�������ʹ���\n");
			ret = -15;
			break;
		}
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
data_end:
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





/*======================================����ִ��===============================================*/
static int tran_data_close_tran_task(struct _tran_data_fd *id, va_list args)
{
	struct _tran_data        *stream;
	int port;
	int count;
	stream = id->dev;
	port = va_arg(args, int);
	out("�ر��ն˺��м���߳�,port = %d\n", port);
	if (stream->line[port - 1].state_mem == 0)
		return 0;
	_lock_set(stream->lock);
	stream->line[port - 1].out_close = 1;
	_lock_un(stream->lock);
	sem_post(stream->line[port - 1].sem);
	for (; stream->line[port - 1].state_mem != 0; usleep(10000))
	{
		count++;
		_lock_set(stream->lock);
		stream->line[port - 1].out_close = 1;
		_lock_un(stream->lock);
		if (count == 100 * 10)//10 S
		{
			write_log("����10S��δ�ɹ��ر�%d�˿���·\n", port);
			break;
		}
	}
	out("tran_data_close_tran_task �رճɹ�����\n");
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
	if (stream->line[port - 1].state_mem == 0)
		return -1;
	return 0;
}
/*======================================�ײ���==================================================*/

static uint32 recv_midware_ver_code(int fd)
{
	int result;
	uint32 ver;
	uint8 ver_code[4];
	memset(ver_code, 0, sizeof(ver_code));
	result = ty_socket_read(fd, ver_code, sizeof(ver_code));
	if (result<0)
	{
		out("���ļ�ʧ��\n");
		return result;
	}
	ver = (uint32)(ver_code[0] * 0x1000000) + (uint32)(ver_code[1] * 0x10000) + (uint32)(ver_code[2] * 0x100) + (uint32)ver_code[3];
	return ver;
}







//////////////////////////////////////////////USBIP��غ���//////////////////////////////////////////////////
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
//	//out("seqnum ���飺");
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
		while (arr[j] >= key && i<j)--j; //��������1 �Ӻ���ȥǰɨ��ֱ���ҵ�һ��a[j]<key�������
		arr[i] = arr[j];
		while (arr[i] <= key && i<j)++i; //��������2 �Ӻ���ȥǰɨ��ֱ���ҵ�һ��a[i]>key�������
		arr[j] = arr[i];
	}
	arr[i] = key;
	if (i - 1>startPos) quickSort(arr, startPos, i - 1); //��������1 ���keyǰ�������������ϵ�������keyǰ��������һ���Ļ���Ȼ�Ͳ������ˣ�
	if (endPos>i + 1) quickSort(arr, i + 1, endPos);//��������2 ���key�������������ϵ�������key�����
}





/*==================================socket ���ͺͽ���===============================================*/
/*==================================================================================================*/
static int ty_socket_read(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{
			out("read data time out or err \n");
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
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			continue;
		if (libusb_get_bus_number(dev) == nBusNum && libusb_get_device_address(dev) == nDevNum)
		{
			devRet = dev;
			break;
		}
	}
	return devRet;
}

static int usb_dev_init(struct _tran_data	*stream, int port_num, uint8 *busid)
{
	//int ret = 0;
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

	//out("��ʼ����̬��ɹ�\n");
	//out("��ʼ����̬��ɹ�,stream->line[port_num - 1].context = %08x\n", stream->line[port_num - 1].context);
	if (libusb_get_device_list(NULL, &devs) < 0)
	{
		out("libusb_get_device_list error\n");
		return -1;
	}
	busnum = get_busid_busnum((char *)busid);
	devnum = get_busid_devnum((char *)busid);
	//out("BUSID : %s,���ߺ�Ϊ��%d,�豸��Ϊ��%d\n", busid, busnum, devnum);
	libusb_device *dev = FindDeviceByID(devs, busnum, devnum);
	if (dev == NULL)
	{
		out("FindDeviceByID error\n");
		return -2;
	}
	//out("����libusb_open\n");
	if (libusb_open(dev, &devh) < 0)
	{
		//out("ret = %d\n", ret);
		return -3;
	}



	//out("����libusb_open�ɹ�\n");
	if (libusb_kernel_driver_active(devh, 0))
	{
		if (libusb_detach_kernel_driver(devh, 0) < 0)
		{
			return -4;
		}
	}


	if (libusb_kernel_driver_active(devh, 1))//��˫�ӿ��豸�ļ���
	{
		if (libusb_detach_kernel_driver(devh, 1) < 0)
		{
			return -4;
		}
	}

	//out("ж���ں������ɹ�\n");

	//out("��λ��USB�豸\n");
	if (libusb_reset_device(devh) < 0)
	{
		out("libusb_reset_device��λʧ��\n");
		return -6;
	}
	//out("��λ�ɹ�port= %d\n", port_num);
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
	//out("��ȡ�豸������\n");
	//memcpy(usb_data, "\x80\x06\x00\x01\x00\x00\x12\x00", 8);
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	{
		out("��ȡ�豸������ʧ��\n");
	}
	//out("��ȡ�豸�������ɹ�\n");
	//out("��ȡ����������\n");
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, 0x09, 3000) < 0)
	{
		out("��һ�λ�ȡ����������ʧ��\n");
	}
	//out("��ȡ�����������ɹ�\n");
	wToalLength = usb_data[2];
	//out("��ȡ�������������ܳ���Ϊ��%d\n", wToalLength);
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, wToalLength, 3000) < 0)
	{
		out("�ڶ��λ�ȡ����������ʧ��\n");
	}
	out("��ӡ��ȡ����������\n");

	printf_array((char *)usb_data, wToalLength);
	int count = 0;
	for (i = 0; i < wToalLength; i++)
	{
		if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
		{
			bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bNumEndpoints = usb_data[i + 4];
			//out("��ȡ���Ķ˵�� bInterfaceNumber Ϊ��%02x,bNumEndpoints Ϊ��%02x\n", bInterfaceNumber, stream->line[port_num - 1].bNumEndpoints);
			if (libusb_claim_interface(devh, bInterfaceNumber) < 0)
			{
				return -7;
			}
			count++;
		}
	}
	stream->line[port_num - 1].AllInterfaceNumber = count;
	int i_count = 0;
	//if (stream->line[port_num - 1].vid == 0x096e && stream->line[port_num - 1].pid == 0x070b)//Ŀǰֻ���ƽ�����е�key������ĳ�ͨ�õ�
	{
		//09 02 32 00 
		out("�����豸\n");
		for (i = 0; i < wToalLength; i++)
		{
			if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x02)))//09 02
			{
				if ((usb_data[i + 4] == 0x02))//��ʾΪ˫�ӿ�//02 01 00 80 32 09 04 00 00 00 03 00 00 00 09 21 10 01 00 22 96 01 09 04 01 00 02 08 06 50 00 07 05 83 02 40 00 00 07 05 04 02 40 00 00
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

							out("�ҵ��ڡ�%d�����ӿ�--�ýӿڶ˵�0����Ķ˵�����%d��\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//ƫ�Ƶ��ӿ�����������˵�������
							out("���ڵ�λ��:��%d��--ֵ:%02x\n", i, usb_data[i]);
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//�ҵ�һ���˵�������

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
									{//�ҵ�һ���˵�������

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
									{//�ҵ���һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("�ڡ�%d�����ӿ�--�ڡ�%d�����˵�,�˵��:��%02x��,�˵�����:��%02x��,������С:��%d��\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										out("�ڡ�%d�����ӿ�--�ڡ�%d�����˵�,�˵��:��%02x��,�˵�����:��%02x��,������С:��%d��\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//��¼��һ���ӿڵĶ˵�0����Ķ˵���

						}
					}
				}
				else if ((usb_data[i + 4] == 0x01))//��ʾΪ���ӿ�
				{
					out("���ӿ��豸!\n");
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))//�ӿ�������
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)//�˵�0����������˵���Ϊ0,��ֻ�п��ƴ���
							{
								out("ֻ���ڿ��ƴ���!\n");
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = 0x00;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = 0;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = 64;
								//stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								i_count++;
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)//�˵�0����������˵���Ϊ1
							{
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										p_count++;
										i += 5;
										i_count++;
										out("1�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)//�˵�0����������˵���Ϊ2
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("2�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										out("3�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 3)//�˵�0����������˵���Ϊ3
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("4�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("5�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);

										p_count++;
										i += 7;


										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 7;
										out("6�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
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

	libusb_free_device_list(devs, 1);


	return 0;
}

static int get_ep_direction(char *buf, int *ep, int *direction)
{
	*direction = buf[15];
	//out("direction = %02x\n", *direction);

	*ep = buf[19];
	//out("ep = %02x\n", *ep);
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

	out("usb_dev_init_new ��ʼ����̬��ɹ�\n");
	if (libusb_get_device_list(NULL, &devs) < 0)
	{
		out("libusb_get_device_list error\n");
		return -1;
	}
	busnum = get_busid_busnum((char *)busid);
	devnum = get_busid_devnum((char *)busid);
	//out("BUSID : %s,���ߺ�Ϊ��%d,�豸��Ϊ��%d\n", busid, busnum, devnum);
	libusb_device *dev = FindDeviceByID(devs, busnum, devnum);
	if (dev == NULL)
	{
		out("FindDeviceByID error\n");
		return -2;
	}
	//out("����libusb_open\n");
	if (libusb_open(dev, &devh) < 0)
	{
		//out("ret = %d\n", ret);
		return -3;
	}
	//out("����libusb_open�ɹ�\n");
	if (libusb_kernel_driver_active(devh, 0))
	{
		if (libusb_detach_kernel_driver(devh, 0) < 0)
		{
			return -4;
		}
	}


	if (libusb_kernel_driver_active(devh, 1))//��˫�ӿ��豸�ļ���
	{
		if (libusb_detach_kernel_driver(devh, 1) < 0)
		{
			return -4;
		}
	}

	//out("ж���ں������ɹ�\n");

	//out("��λ��USB�豸\n");
	if (libusb_reset_device(devh) < 0)
	{
		out("usb_dev_init_new libusb_reset_device��λʧ��\n");
		return -6;
	}
	//out("��λ�ɹ�port= %d\n", port_num);
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
	//out("��ȡ�豸������\n");
	//memcpy(usb_data, "\x80\x06\x00\x01\x00\x00\x12\x00", 8);
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0100, 0, usb_data, 0x12, 3000) < 0)
	{
		out("��ȡ�豸������ʧ��\n");
	}
	//out("��ȡ�豸�������ɹ�\n");
	//out("��ȡ����������\n");
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, 0x09, 3000) < 0)
	{
		out("��һ�λ�ȡ����������ʧ��\n");
	}
	//out("��ȡ�����������ɹ�\n");
	wToalLength = usb_data[2];
	//out("��ȡ�������������ܳ���Ϊ��%d\n", wToalLength);
	memset(usb_data, 0, sizeof(usb_data));
	if (libusb_control_transfer(devh, 0x80, 0x06, 0x0200, 0, usb_data, wToalLength, 3000) < 0)
	{
		out("�ڶ��λ�ȡ����������ʧ��\n");
	}
	//out("��ӡ��ȡ����������\n");
	//int i;
	//for (i = 0; i < wToalLength; i++)
	//{
	//	printf("%02x ", (unsigned char)usb_data[i]);
	//}
	//printf("\n");
	int count = 0;
	for (i = 0; i < wToalLength; i++)
	{
		if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))
		{
			bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bInterfaceNumber = usb_data[i + 2];
			stream->line[port_num - 1].bNumEndpoints = usb_data[i + 4];
			//out("��ȡ���Ķ˵�� bInterfaceNumber Ϊ��%02x,bNumEndpoints Ϊ��%02x\n", bInterfaceNumber, stream->line[port_num - 1].bNumEndpoints);
			if (libusb_claim_interface(devh, bInterfaceNumber) < 0)
			{
				return -7;
			}
			count++;
		}
	}
	stream->line[port_num - 1].AllInterfaceNumber = count;
	int i_count = 0;
	//if (stream->line[port_num - 1].vid == 0x096e && stream->line[port_num - 1].pid == 0x070b)//Ŀǰֻ���ƽ�����е�key������ĳ�ͨ�õ�
	{
		out("�����豸\n");
		for (i = 0; i < wToalLength; i++)
		{
			if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x02)))
			{
				if ((usb_data[i + 4] == 0x02))//��ʾΪ˫�ӿ�
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

							out("�ҵ��ڡ�%d�����ӿ�--�ýӿڶ˵�0����Ķ˵�����%d��\n", i_count + 1, stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints);
							i += 4;//ƫ�Ƶ��ӿ�����������˵�������
							out("���ڵ�λ��:��%d��--ֵ:%02x\n", i, usb_data[i]);

							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)
							{
								//while (i < wToalLength)
								//{
								//	if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
								//	{//�ҵ�һ���˵�������

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
									{//�ҵ�һ���˵�������

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
									{//�ҵ���һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("�ڡ�%d�����ӿ�--�ڡ�%d�����˵�,�˵��:��%02x��,�˵�����:��%02x��,������С:��%d��\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);

										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										i += 5;
										out("�ڡ�%d�����ӿ�--�ڡ�%d�����˵�,�˵��:��%02x��,�˵�����:��%02x��,������С:��%d��\n", i_count + 1, p_count, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress
											, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							//bNumEndppoints0 = usb_data[i + 4];//��¼��һ���ӿڵĶ˵�0����Ķ˵���

						}
					}
				}
				else if ((usb_data[i + 4] == 0x01))//��ʾΪ���ӿ�
				{
					i += 4;
					for (; i < wToalLength; i++)
					{
						if ((usb_data[i] == 0x09) && ((usb_data[i + 1] == 0x04)))//�ӿ�������
						{
							int p_count = 0;

							stream->line[port_num - 1].interface_decs[i_count].state = 1;
							stream->line[port_num - 1].interface_decs[i_count].bInterfaceNumber = usb_data[i + 2];
							stream->line[port_num - 1].interface_decs[i_count].bAlterateSetting = usb_data[i + 3];
							stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints = usb_data[i + 4];
							if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 0)//�˵�0����������˵���Ϊ0,��ֻ�п��ƴ���
							{
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = 0x00;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = 0;
								stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = 64;
								//stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
								i_count++;
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 1)//�˵�0����������˵���Ϊ1
							{
								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

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
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 2)//�˵�0����������˵���Ϊ2
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

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
										out("7�˵�����Ϊ:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes);
										i_count++;
										break;
									}
									else
										i++;
								}
								continue;
							}
							else if (stream->line[port_num - 1].interface_decs[i_count].bNumEndpoints == 3)//�˵�0����������˵���Ϊ3
							{

								while (i < wToalLength)
								{
									if ((usb_data[i] == 0x07) && ((usb_data[i + 1] == 0x05)))
									{//�ҵ�һ���˵�������

										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("�˵�:%02x -����Ϊ:%d - ���䳤��:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes, \
											stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize);
										p_count++;
										i += 7;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].state = 1;
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress = usb_data[i + 2];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bmAttributes = usb_data[i + 3];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].wMaxPacketSize = usb_data[i + 4];
										stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bInterval = usb_data[i + 5];
										out("�˵�:%02x -����Ϊ:%d - ���䳤��:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
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
										out("�˵�:%02x -����Ϊ:%d - ���䳤��:%d!\n", stream->line[port_num - 1].interface_decs[i_count].point_decs[p_count].bEndpointAddress, \
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

	//out("bmAttributes = %02x\n", bmAttributes);
	switch (bmAttributes & 0x03)
	{
	case 0x00:
		out("��������Ϊ�����ƴ���\n");
		stream->line[port_num - 1].tran_type = 0x00;
		break;
	case 0x01:
		out("��������Ϊ��ͬ������,�����Ͳ�֧��\n");
		stream->line[port_num - 1].tran_type = 0x01;
		break;
	case 0x02:
		out("��������Ϊ���鴫��\n");
		stream->line[port_num - 1].tran_type = 0x02;
		break;
	case 0x03:
		out("��������Ϊ���жϴ���\n");
		stream->line[port_num - 1].tran_type = 0x03;
		break;
	default:
		break;
	}



	if ((stream->line[port_num - 1].tran_type == 0x03) && (stream->line[port_num - 1].bNumEndpoints == 1))
	{
		out("���豸Ϊ����һ���˵���жϴ��������豸�����ƴ���CA��ǿ���޸�IN�˵�������ݳ���\n");
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
		out("�ȴ������жϽ��ն����˳�\n");
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
			out("���ƴ���in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].control_out_thread_num == 0)
		{
			out("���ƴ���out�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_in_thread_num == 0)
		{
			out("�鴫��in�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].bulk_out_thread_num == 0)
		{
			out("�鴫��out�߳�ȫ���˳����\n");
			break;
		}
	}

	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_out_thread_num == 0)
		{
			out("�жϴ���out�߳�ȫ���˳����\n");
			break;
		}
	}
	for (i = 0; i<8000; i++, usleep(1000))
	{
		if (stream->line[port_num].interrupt_in_thread_num == 0)
		{
			out("�жϴ���in�߳�ȫ���˳����\n");
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
		write_log("���¼���devhʱ���߳�δ�˳�:bulk_in:%d\tbulk_out:%d\tctl_in:%d\tctrl_out:%d\t!\n", stream->line[port_num].bulk_in_thread_num, stream->line[port_num].bulk_out_thread_num, \
			stream->line[port_num].control_in_thread_num, stream->line[port_num].control_out_thread_num);
	}
	//�ȴ��߳��˳�
	if (stream->line[port_num].devh != NULL)
	{

		result = libusb_release_interface(stream->line[port_num].devh, 0);
		if (result < 0)
		{
			out("libusb_release_interface result = %d\n", result);
		}

		if (stream->line[port_num].interface_decs[1].state != 0)//��ʾΪ˫�ӿ��豸
		{
			result = libusb_release_interface(stream->line[port_num].devh, 1);
		}
	}
	for (i = 0; i < 2; i++)
		memset(&stream->line[port_num].interface_decs[i], 0, sizeof(struct _interface_dec));

	out("reload_devh �رվ�����´�\n");
	if (stream->line[port_num].devh != NULL)
	{
		out("reload_devh����usbshareʹ�ùر�%d�Ŷ˿ھ��\n", port_num+1);
		libusb_close(stream->line[port_num].devh);
		stream->line[port_num].devh = NULL;
	}
	stream->line[port_num].out_close = 0;
	out("������Ҫ���»�ȡ���\n");
	result = usb_dev_init_new(stream, port_num + 1, stream->line[port_num].busid);

	//out("reload_devh��������Ϊ��%d��in_ep = %02x out_ep = %02x,in�������֡����= %d,out�������֡����= %d\n", stream->line[port_num].tran_type, stream->line[port_num].in_ep, stream->line[port_num].out_ep, stream->line[port_num].in_max_len, stream->line[port_num].out_max_len);
	out("unlink !reload dev success!\n");
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
			out("�鴫���߳��ⲿҪ���˳�!\n");
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
		if (*line->devh == NULL)//
		{

			write_log("bulk_data_to_dev_out ����devh�Ѿ��ͷ� enter1\n");
			_lock_set(bulk_lock);
			if (line->bulk_in_data != NULL)
			{
				free(line->bulk_in_data);
				line->bulk_in_data = NULL;
			}
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			return;//break->return 10 29 change
		}

		if (*line->unlink_flag == 1)
		{
			out("bulk_data_to_dev_out unlink!\n");
			break;//�������ص�ʱ���ֿ��ܴ���unlink�����
		}


		result = libusb_bulk_transfer(*line->devh, line->ep, line->bulk_in_data, line->Wlength, &size, 2000);
		if (result < 0)
		{
			if (*line->devh == NULL)
			{
				write_log("bulk_data_to_dev_out ����devh�Ѿ��ͷ�\n");
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
			out("bulk out err wait next -result:%d!\n", result);
			if (*line->unlink_flag == 1)
			{
				out("bulk_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, line->ep);
				line->stream->line[port_num].unlink_flag = 0;
				break;//�������ص�ʱ���ֿ��ܴ���unlink�����
			}
			if (result != -7)
			{
				if (result == LIBUSB_ERROR_PIPE)//��Ϊ-9��ʱ���ʾ��ʱ��ָ֡�����Ϊ����ģ���stall pid reset�ȣ����Խ��е�һ163C 0620�豸ʱ����,���ǵ���ũ�е�һ���豸��ʱ��Ҫ����˵�ȴ�096e 0705
				{
					out("�鴫���߳�err pipe--wlength:%d -thread_num:%d!\n", line->Wlength, *thread_num);
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
			out("�鴫���߳�д�뵽���ݳ���Ϊ:%d\n", line->Wlength);
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
	//out("�鴫��in�̷߳������ݰ�\n");



	if (send(line->sock, out_buf, 48, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
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
	_lock_un(line->sock_lock);
	//�ͷŴ�����·��Դ
	_lock_set(bulk_lock);
	if (line->bulk_in_data != NULL)
	{
		free(line->bulk_in_data);
		line->bulk_in_data = NULL;
	}
	memset(line, 0, sizeof(struct _bulk_info));
	(*thread_num)--;

	_lock_un(bulk_lock);

}

static void bulk_data_to_dev(void *arg)
{
	struct _bulk_info *line;
	line = arg;
	unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[204800] = { 0 };
	int result;
	int size = 0;
	int *thread_num;
	int port_num;
	int retry_num = 0;
	port_num = line->port_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;



	for (;;)
	{
		if (*line->out_close != 0)
		{
			out("�鴫���߳��ⲿҪ���˳�!\n");
			_lock_set(bulk_lock);
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			return;
		}
		if (((line->vid == 0x1677) && (line->pid == 0x0024)) && line->Wlength >= 65535)
		{
			out("bulk in len too long :%d\n", line->Wlength);
			line->Wlength = 1024;//����1677 0024����65536�����
		}
		if (*line->devh == NULL)//
		{
			//size = line->Wlength;//new add 0719
			//break;

			write_log("bulk_data_to_dev ����devh�Ѿ��ͷ� enter1\n");
			_lock_set(bulk_lock);
			memset(line, 0, sizeof(struct _bulk_info));
			(*thread_num)--;
			_lock_un(bulk_lock);
			return;
		}

		if (*line->unlink_flag == 1)
		{
			out("bulk_data_to_dev unlink!\n");
			libusb_clear_halt(*line->devh, line->ep);
			line->stream->line[port_num].unlink_flag = 0;
			size = line->Wlength;
			break;//�������ص�ʱ���ֿ��ܴ���unlink�����
		}


		result = libusb_bulk_transfer(*line->devh, line->ep, usb_data, line->Wlength - size, &size, 2000);
		if (result < 0)
		{
			if (*line->devh == NULL)
			{
				write_log("bulk_data_to_dev ����devh�Ѿ��ͷ�\n");
				_lock_set(bulk_lock);
				memset(line, 0, sizeof(struct _bulk_info));
				(*thread_num)--;
				_lock_un(bulk_lock);
				return;
			}
			//out("�鴫���̵߳ȴ��´ζ�ȡ --- result:%d---ep:%02x\n", result, line->ep);
			//out("bulk err wait next:ep:%02x --result:%d!--len:%d -size:%d\n", line->ep, result, line->Wlength, size);
			if (*line->unlink_flag == 1)
			{
				out("bulk_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, line->ep);
				line->stream->line[port_num].unlink_flag = 0;
				size = line->Wlength;//new add 0719
				break;//�������ص�ʱ���ֿ��ܴ���unlink�����
			}
			if (size > 0)
			{
				out("size is not zore ,break!\n");
				break;
			}
			if (result != -7)
			{
				if (result == LIBUSB_ERROR_PIPE)//��Ϊ-9��ʱ���ʾ��ʱ��ָ֡�����Ϊ����ģ���stall pid reset�ȣ����Խ��е�һ163C 0620�豸ʱ����,���ǵ���ũ�е�һ���豸��ʱ��Ҫ����˵�ȴ�096e 0705
				{
					out("�鴫���߳�err pipe--wlength:%d -thread_num:%d!\n", line->Wlength, *thread_num);
					if (((line->vid == 0x163c) && (line->pid == 0x0620)) || ((line->vid == 0x163c) && (line->pid == 0x5462)) \
						)//21-0312 -���� 163c 5462 
					{
						libusb_clear_halt(*line->devh, line->ep);
						size = line->Wlength;
						break;
					}

					if (retry_num == 5)
					{
						_lock_set(bulk_lock);
						memset(line, 0, sizeof(struct _bulk_info));
						(*thread_num)--;
						_lock_un(bulk_lock);
						return;
					}
					/*if ((line->vid == 0x096e) && (line->pid == 0x0705))
					{*/
					libusb_clear_halt(*line->devh, line->ep);
					sleep(1);
					continue;
					//}



				}
				if (result == LIBUSB_ERROR_OVERFLOW)
				{
					out("overflow wait next:%d! break\n", size);//���Է��ֹ������ؽ�˰�̵�ʱ�������������  ???�Ƿ��´ε�ʱ��line->Wlength ��Ϊpoint��maxpacket  ����֤
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
				return;
			}
			//2021/12/22 Ҽ�w���ػ������ж�d449 0004 �����и����� 

			//{
			//	//if ((line->vid == 0xd449) && (line->pid == 0x0004))
			//	size = line->Wlength;
			//	out("HXBANK SPECIAL DEAL!\n");
			//	break;
			//}
			{
				//size = line->Wlength;
				//out("SPECIAL DEAL!\n");
				//break;
			}

			continue;//2021/12/22 Ҽ�w���ػ������ж�d449 0004 �����и����� --2022/02/15���ֹ�������˰��Ukey��¼��ʱ����ܻ����-7���� Ҫ�ȴ�
		}
		else
		{
#ifdef BULK_OUT
			out("�鴫���̶߳�ȡ�����ݳ���Ϊ:%d\n", size);
#endif
			break;
		}
	}

	memcpy(out_buf, line->head_data, 20);
	out_buf[0] = 0x00;							out_buf[1] = 0x00;
	out_buf[2] = 0x00;							out_buf[3] = 0x03;
	out_buf[4] = line->head_data[4];						out_buf[5] = line->head_data[5];
	out_buf[6] = line->head_data[6];						out_buf[7] = line->head_data[7];
	out_buf[24] = ((size >> 24) & 0xff);		out_buf[25] = ((size >> 16) & 0xff);//wLength
	out_buf[26] = ((size >> 8) & 0xff);		out_buf[27] = ((size >> 0) & 0xff);
	memcpy(out_buf + 48, usb_data, size);




	_lock_set(line->sock_lock);
	//out("�鴫��in�̷߳������ݰ�\n");



	if (send(line->sock, out_buf, 48 + size, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(bulk_lock);
		memset(line, 0, sizeof(struct _bulk_info));
		(*thread_num)--;
		_lock_un(bulk_lock);
		return;
	}
	_lock_un(line->sock_lock);
	//�ͷŴ�����·��Դ
	_lock_set(bulk_lock);
	memset(line, 0, sizeof(struct _bulk_info));
	(*thread_num)--;

	_lock_un(bulk_lock);

}
static int bulk_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd)
{
	unsigned char usb_data[204800];
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
	memset(usb_data, 0, sizeof(usb_data));
	memset(tmp, 0, sizeof(tmp));
#ifdef BULK_OUT
	int i;
	out("���鴫�䡿���뵽�豸�����ݳ���Ϊ��%d\n", buf_len);

	printf_array(buf,buf_len);
#endif

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
		//	printf("bulk out buff too large:%d\n!", buf_len - 48);
		//	return -1;
		//}
		for (i = 0; i < sizeof (stream->line[port_num - 1].bulk_info_out) / sizeof(struct _bulk_info); i++)
		{
			if (stream->line[port_num - 1].bulk_info_out[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N)
		{
			out("�鴫��in �̴߳ﵽ���ޣ�\n");
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
			out("�鴫��mallocʧ��!\n");
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
		_lock_un(bulk_lock);
		//out("�鴫�䴴���߳�\n");
		if (_queue_add("0", bulk_data_to_dev_out, &stream->line[port_num - 1].bulk_info_out[i], 0)<0)
		{
			out("��������ʧ��\n");
		}
		return 0;

	}
	else if (dir_ep == stream->line[port_num - 1].in_ep)
	{
		memset(usb_data, 0, sizeof(usb_data));

		//out("���鴫�䡿�����豸����������in�˵�wLength = %d\n", wLength);

		int i;


		for (i = 0; i < sizeof (stream->line[port_num - 1].bulk_info) / sizeof(struct _bulk_info); i++)
		{
			if (stream->line[port_num - 1].bulk_info[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N)
		{
			out("�鴫��in �̴߳ﵽ���ޣ�\n");
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
		//out("�鴫�䴴���߳�\n");
		if (_queue_add("0", bulk_data_to_dev, &stream->line[port_num - 1].bulk_info[i], 0)<0)
		{
			out("��������ʧ��\n");
		}
		return 0;

	}

	return 0;

}
/*
���ƴ�����豸�ж�ȡ���� (in)
*/
static void control_data_from_dev(void *arg)
{
	struct _control_info *line;
	line = arg;
	unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[204800] = { 0 };
	int result;
	int *thread_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	int reload_times = 0;
	out("control in thread_num :%d\n", *thread_num);
	for (;;)
	{
		if (*line->out_close != 0)
		{
			//out("���ƴ����߳��ⲿҪ���˳�!\n");
			_lock_set(control_lock);
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			return;
		}
		if (*line->devh == NULL)//
		{
			//result = line->Wlength;//new add 0719
			//break;

			write_log("control_data_from_dev ����devh�Ѿ��ͷ� enter1\n");
			_lock_set(control_lock);
			memset(line, 0, sizeof(struct _control_info));
			(*thread_num)--;
			_lock_un(control_lock);
			return;
		}
		result = libusb_control_transfer(*line->devh, line->request_type, line->bRequest, line->wValue, line->wIndex, usb_data, line->Wlength, 2000);//5000->2000  10 29
		if (result < 0)
		{
			//out("���ƴ����߳�ִ���´ζ�ȡ!\n");
			out("control_data_from_dev err :%d .wait next!\n", result);
			if (*line->devh == NULL)
			{
				write_log("control_data_from_dev ����devh�Ѿ��ͷ�\n");
				_lock_set(control_lock);
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				return;
			}
			if (*line->unlink_flag == 1)
			{
				out("control_data_from_dev unlink!\n");
				libusb_clear_halt(*line->devh, 0);
				//line->stream->line[port_num].unlink_flag = 0;//new chg 0714
				*line->unlink_flag = 0;//new add by whl 1025
				result = line->Wlength;//new add 0719
				break;//�������ص�ʱ���ֿ��ܴ���unlink�����
			}

			if ((line->vid == 0X1ea8 && line->pid == 0Xc102) || (line->vid == 0X1d99 && line->pid == 0X0013))//Э������ca����  ����Ϊ -9 err pipe ��Ҫ���»�ȡ ԭ����
			{
				if (reload_times >= 2)//new change by whl
				{
					out("reload_times too many times!\n");
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

			if (result != LIBUSB_ERROR_TIMEOUT)//���ǳ�ʱ-7 �����������˳�
			{
				out("���ƴ���in�߳�ִ��ʧ��--result :%d --len:%d!\n", result, line->Wlength);
				if (line->vid == 0X1a56 && line->pid == 0Xdd01)//���ӿڰ��豸����--��֪���Ƿ���ȷ������֤��
				{
					result = 0;
					break;
				}
				_lock_set(control_lock);
				memset(line, 0, sizeof(struct _control_info));
				(*thread_num)--;
				_lock_un(control_lock);
				return;
			}
			continue;
		}
		else
		{
			/*out("���ƴ���in�̶߳�ȡ�����ݳ���Ϊ:%d\n", result);
			int i;
			printf("\n");
			for (i = 0; i < result; i++)
			{
			printf("%02x ", usb_data[i]);
			}
			printf("\n\n");*/
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




	_lock_set(line->sock_lock);
	//out("���ƴ���in�̷߳������ݰ�\n");
	if (send(line->sock, out_buf, 48 + result, MSG_NOSIGNAL)<0)
	{
		_lock_un(line->sock_lock);
		_lock_set(control_lock);
		memset(line, 0, sizeof(struct _control_info));
		(*thread_num)--;
		_lock_un(control_lock);
		return;
	}
	_lock_un(line->sock_lock);

	//�ͷŴ�����·��Դ
	_lock_set(control_lock);
	memset(line, 0, sizeof(struct _control_info));
	(*thread_num)--;
	_lock_un(control_lock);
	out("���ƴ���in�̷߳������ݰ����� -- %08x   -- %d\n", thread_num, *thread_num);
	return;
}
/*
���ƴ�����usb�豸д������  (out)
*/
static void control_data_to_dev(void *arg)
{
	struct _control_info *line;
	line = arg;
	//unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[204800] = { 0 };
	int result;
	int *thread_num;
	int port_num;
	port_num = line->port_num;
	//memcpy(usb_data, line->ctl_in_data, line->ctl_in_data_len);
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	int reload_times = 0;
	out("control out thread_num:%d --unlink_flag:%d\n", *thread_num, *line->unlink_flag);
	for (;;)
	{
		if (*line->out_close != 0)
		{
			out("���ƴ����߳��ⲿҪ���˳�!\n");
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
		if (*line->devh == NULL)//
		{
			write_log("control_data_to_dev ����devh�Ѿ��ͷ� enter1\n");
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
		result = libusb_control_transfer(*line->devh, line->request_type, line->bRequest, line->wValue, line->wIndex, line->ctl_in_data, line->ctl_in_data_len, 2000);//5000->2000  10 29
		if (result < 0)
		{
			out("libusb_control_transfer err :%d .wait next or unlink !\n", result);
			if (*line->devh == NULL)
			{
				write_log("control_data_to_dev ����devh�Ѿ��ͷ�\n");
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

			if (*line->unlink_flag == 1)
			{
				out("control_data_to_dev unlink!\n");
				libusb_clear_halt(*line->devh, 0);
				//*line->unlink_flag = 0;//new add by  whl 1025
				break;//�������ص�ʱ���ֿ��ܴ���unlink�����
			}


			if ((line->vid == 0X1ea8 && line->pid == 0Xc102) || (line->vid == 0X1d99 && line->pid == 0X0013) || (line->vid == 0X0525 && line->pid == 0Xa4a2))//Э������ca ����  ����Ϊ -9 err pipe ��Ҫ���»�ȡ ԭ����  (plc usb)
			{

				if (reload_times >= 2)//new change by whl
				{
					out("reload_times too many times!\n");
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
				//������Ͽ���к��й���������
				if ((result == LIBUSB_ERROR_PIPE && (line->vid == 0Xd449 && line->pid == 0X0004)) || (result == LIBUSB_ERROR_PIPE && (line->vid == 0X163c && line->pid == 0X548c)))
				{
					libusb_clear_halt(*line->devh, 0);

					line->stream->line[port_num].unlink_flag = 0;
					break;
				}
				if (result == LIBUSB_ERROR_PIPE && (line->vid == 0X163c && line->pid == 0X548c))//�й���������һ���豸
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
					return;
				}
				out("���ƴ���out�߳�ִ��ʧ��--result:%d- len:%d!\n", result, line->ctl_in_data_len);
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

			continue;
		}
		else
		{
			//out("���ƴ����߳�д�����ݳ���Ϊ:%d\n", result);
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
	//memcpy(out_buf + 48, usb_data, result);

	_lock_set(line->sock_lock);
	//out("���ƴ���out�̷߳������ݰ�\n");
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

		return;
	}
	_lock_un(line->sock_lock);

	//�ͷŴ�����·��Դ
	_lock_set(control_lock);
	if (line->ctl_in_data != NULL)
	{
		free(line->ctl_in_data);
		line->ctl_in_data = NULL;
	}
	memset(line, 0, sizeof(struct _control_info));
	(*thread_num)--;
	_lock_un(control_lock);

	out(" out  control out thread_num:%d\n", *thread_num);
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


#ifdef CONTROL_OUT
	//int i;
	out("�����ƴ��䡿���뵽�豸������\n");

	printf_array(buf,buf_len);
#endif

	request_type = buf[40];
	bRequest = buf[41];
	wValue = buf[43] * 0x100 + buf[42];
	wIndex = buf[45] * 0x100 + buf[44];
	wLength = buf[47] * 0x100 + buf[46];
	//out("�����ƴ��䡿wLength = %d\n", wLength);
	usb_data = malloc(buf_len - 48 + 10);
	if (usb_data == NULL)
		return -1;
	memset(usb_data, 0, buf_len - 48 + 10);
	memcpy(usb_data, buf + 48, buf_len - 48);

	int direction = 0;
	direction = buf[15];


	if (direction == 1)//��ʾҪ��in�˵��ȡ����
	{

		// int ret;
		free(usb_data);
		for (i = 0; i < sizeof (stream->line[port_num - 1].control_info) / sizeof(struct _control_info); i++)
		{
			if (stream->line[port_num - 1].control_info[i].state == 0)
				break;
		}
		if (i == DF_TY_USB_PORT_N)
		{
			out("���ƴ���in �̴߳ﵽ���ޣ�\n");
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
		//out("���ƴ��䴴���߳�--in\n");
		if (_queue_add("0", control_data_from_dev, &stream->line[port_num - 1].control_info[i], 0) < 0)
		{
			out("��������ʧ��\n");
		}
		//out("���ƴ��䴴���߳�--in -- over!\n");
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
		if (i == DF_TY_USB_PORT_N)
		{
			out("���ƴ���out �̴߳ﵽ���ޣ�\n");
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
		if (buf_len - 48 > 2000)//new add by whl 10 29
		{
			write_log("���ƴ������ݹ���:%d,ʧ��!\n", buf_len - 48);
			_lock_un(control_lock);
			free(usb_data);
		}

		stream->line[port_num - 1].control_info_out[i].ctl_in_data = malloc(buf_len - 48 + 20);
		if (stream->line[port_num - 1].control_info_out[i].ctl_in_data == NULL)
		{
			out("�鴫��mallocʧ�ܣ�\n");
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
		//out("���ƴ��䴴���߳�--out\n");
		if (_queue_add("0", control_data_to_dev, &stream->line[port_num - 1].control_info_out[i], 0) < 0)
		{
			out("��������ʧ��\n");
		}
		return 0;
	}
	else
	{
		out("���ƶ˵㷽����inҲ����out\n");
		return -1;
	}
}

//�жϴ�������־��ӡ��Ҫע��  ���ӿڰ��豸���ִ�ӡ��־Ӱ��ʱ���µ�¼��ʧ��
static void interr_data_from_dev(void *arg)//��ҵ���з�����Ҫ�첽����
{
	struct _interrupt_info *line;
	line = arg;
	//unsigned char usb_data[204800] = { 0 };
	unsigned char usb_data[10000] = { 0 };
	unsigned char out_buf[1024] = { 0 };
	int result;
	int *thread_num;
	int size = 0;
	//int count = 0;
	int len = 0;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;

	if ((line->vid == 0X163c && line->pid == 0X070b)||(line->vid == 0X163c && line->pid == 0X0704))//���������豸   ����������maxpacketsize��16 ʵ��Ҫ��32(��vid pid����������  ����һ���������еľ���32)
	{
		len = 32;
	}
	else if ((line->vid == 0X096e && line->pid == 0X0727) || ((line->vid == 0x1780 && line->pid == 0x0410)))//��ҵ���У����죩�����豸   ����������maxpacketsize��16 ʵ��Ҫ��64
	{
		out("interr_data_from_dev in :%d --ep:%02x!\n", line->Wlength, line->ep);
		len = 64;
	}
	else if ((line->vid == 0x096e && line->pid == 0x070b) || (line->vid == 0x1780 && line->pid == 0x0410))
	{
		len = 64;
	}
	else
	{
		len = line->Wlength;
	}
	//printf("interr_data_from_dev in :%d --ep:%02x!\n", len, line->ep);
	for (;;)
	{

		if (*line->out_close != 0)
		{
			out("�жϴ����߳�in�ⲿҪ���˳�!\n");
			_lock_set(interrupt_lock);
			memset(line, 0, sizeof(struct _interrupt_info));
			(*thread_num)--;
			_lock_un(interrupt_lock);
			return;
		}
		result = libusb_interrupt_transfer(line->devh, line->ep, usb_data, len, &size, 1000);
		if (result < 0)
		{
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
					libusb_clear_halt(line->devh, line->ep);
					continue;
				}
				else
				{
					_lock_set(interrupt_lock);
					*line->out_close = 1;
					memset(line, 0, sizeof(struct _interrupt_info));
					(*thread_num)--;
					//*line->out_close = 1;
					_lock_un(interrupt_lock);
					return;
				}
			}
		}
		else
		{
			//printf("interrupt read data:%d\n", size);//�����ڴ˴�����־   Ŀǰ���Ե��ӿڰ��豸������־��ᵼ�µ�¼����ȥ!
			//	sleep(1);
			//printf("1\n");
			break;
		}


	}
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

static void interr_data_to_dev(void *arg)//��ҵ���з�����Ҫ�첽����
{
	struct _interrupt_info *line;
	line = arg;
	//unsigned char usb_data[204800] = { 0 };
	unsigned char out_buf[1024] = { 0 };
	int result;
	int *thread_num;
	int size = 0;
	//int port_num;
	//int count = 0;
	//port_num = line->port_num;
	*(line->thread_num) += 1;
	thread_num = line->thread_num;
	//memcpy(usb_data, line->in_data, line->Wlength);
	//printf("interr_data_to_dev in :%d--ep:%02x!\n", line->Wlength, line->ep);
	for (;;)
	{

		if (*line->out_close != 0)
		{
			out("�жϴ����߳��ⲿҪ���˳�!\n");
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
		result = libusb_interrupt_transfer(line->devh, line->ep, line->inter_in_data, line->Wlength, &size, 1000);
		if (result < 0)
		{
			if (result == LIBUSB_ERROR_TIMEOUT)
			{
				/*	if (count == 3)
				{
				out("�ж�out��ʱ��������!\n");
				result = line->Wlength;
				break;
				}
				count++;*/

				continue;
			}
			else
			{
				out("�ж�out err,code is:%d!\n", result);
				_lock_set(interrupt_lock);
				if (line->inter_in_data != NULL)
				{
					free(line->inter_in_data);
					line->inter_in_data = NULL;
				}
				*line->out_close = 1;
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
static int interrupt_transfer(struct _tran_data	*stream, int port_num, unsigned char *buf, int buf_len, unsigned char *out_buf, int *out_len, int socket_fd)
{
	unsigned char usb_data[204800];
	//int i;
	//int result;
	int dirction;
	int ep;
	int dir_ep;
	memset(usb_data, 0, sizeof(usb_data));


	//#ifdef INTERRUPT_OUT
	//	int i;
	//	out("���жϴ��䡿���뵽�豸�����ݳ���Ϊ��%d\n", buf_len);
	//	for (i = 0; i < buf_len; i++)
	//	{
	//		printf("%02x ", buf[i]);
	//	}
	//	printf("\n\n\n");
	//#endif
	dirction = buf[12] * 0x1000000 + buf[13] * 0x10000 + buf[14] * 0x100 + buf[15];
	ep = buf[16] * 0x1000000 + buf[17] * 0x10000 + buf[18] * 0x100 + buf[19];
	//

	if ((dirction == 1) || (stream->line[port_num - 1].bNumEndpoints == 1))
	{
		dir_ep = ep | 0x80;
	}
	else
	{
		dir_ep = ep;
	}
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
		if (i == DF_TY_USB_PORT_N)
		{
			out("�жϴ���out �̴߳ﵽ���ޣ�\n");
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
			out("�жϴ���mallocʧ�ܣ�\n");
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
		_lock_un(interrupt_lock);
		//out("���ƴ��䴴���߳�--out\n");
		if (_queue_add("0", interr_data_to_dev, &stream->line[port_num - 1].interrupt_info_out[i], 0) < 0)
		{
			out("��������ʧ��\n");
		}
		return 0;
#else

		int len_t = buf_len - 48;
		usleep(5000);
		memcpy(usb_data, buf + 48, buf_len - 48);
		result = libusb_interrupt_transfer(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep, usb_data, len_t, &size, timeout);
		if (result < 0)
		{
			out("���жϴ��䡿����������out�˵�:%02xʧ��,result = %d\n", stream->line[port_num - 1].out_ep, result);
			if (result == LIBUSB_ERROR_TIMEOUT)
			{
				out("���жϴ��䡿����˵㣬�ٴη��ͻ�ȡ����\n");
				libusb_clear_halt(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep);
				result = libusb_interrupt_transfer(stream->line[port_num - 1].devh, stream->line[port_num - 1].out_ep, usb_data, len_t, &size, timeout);
				if (result < 0)
				{
					out("���жϴ��䡿����˵㣬�ٴη��ͻ�ȡ����ʧ��\n");
					return -3;
				}
			}
			else
			{
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
		out("���жϴ��䡿�������м��������,���ȣ�%d\n", 48);

		printf_array(out_buf,48);
#endif
		if (send(socket_fd, out_buf, 48, MSG_NOSIGNAL)<0)
		{
			_lock_un(stream->line[port_num - 1].lock);
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
		if (i == DF_TY_USB_PORT_N * 2)
		{
			out("�жϴ���in �̴߳ﵽ���ޣ�\n");
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
		//out("���ƴ��䴴���߳�--out\n");
		if (_queue_add("0", interr_data_from_dev, &stream->line[port_num - 1].interrupt_info_in[i], 0) < 0)
		{
			out("��������ʧ��\n");
		}
		return 0;
#else
		//stream->line[port_num - 1].seqnum = buf[4] * 0x1000000 + buf[5] * 0x10000 + buf[6] * 0x100 + buf[7];
		//stream->line[port_num - 1].socket_fd = socket_fd;
		//if (stream->line[port_num - 1].interrupt_in_ep == 0x00)//��in�˵㸳ֵһ��
		//{
		//	stream->line[port_num - 1].interrupt_in_ep = dir_ep;
		//}
		//record_out_l(stream->line[port_num - 1].seqnum, stream->line[port_num - 1].out_q_seqnum_array);
		//if (_queue_add("0", interrupt_data_to_dev, &stream->line[port_num - 1], 0)<0)
		//{
		//	out("��������ʧ��\n");
		//}
#endif

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
//	out("����һ���߳�\n");
//	out("��interrupt_data_to_dev�������豸in_ep = %02x,in_max_len = %d\n", in_ep, in_maxlen);
//	for (;;)
//	{
//
//		if (line->out_close != 0)
//		{
//			out("��interrupt_data_to_dev���ⲿ��Ҫ��رգ�ֹͣ��USB�豸�Ķ�ȡ����,seqnum = %d, line->queue_num = %d\n", seqnum, line->queue_num);
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
//				break;//�������ص�ʱ���ֿ��ܴ���unlink�����
//			}
//
//			if (result == LIBUSB_ERROR_TIMEOUT)
//			{
//				//out("��interrupt_data_to_dev������ʧ��,��ʱ��result = %d\n", result);
//				if (size != 0)
//				{
//					out("�жϳ�ʱ��transferedΪ:%d\n", size);
//					break;
//				}
//				continue;
//			}
//			out("��interrupt_data_to_dev������ʧ��,��ոö˵��ٴη���,result = %d  Ҫ��ȡ�ĳ���:%d,size:%d\n", result, in_maxlen, size);
//
//			if (result == LIBUSB_ERROR_PIPE)
//			{
//				out("�жϴ��� in LIBUSB_ERROR_PIPE\n");
//				libusb_clear_halt(line->devh, in_ep);
//				continue;
//			}
//			else
//			{
//				line->queue_num = line->queue_num - 1;
//				out("�쳣�˳��̣߳�\n");
//				return;
//			}
//
//
//		}
//		else
//		{
//			out("��interrupt_data_to_dev���жϴ�����ɣ�\n");
//			break;
//		}
//	}
//	if (size > 0)
//		out("�жϴ���in��ȡ��%d������\n", size);
//	if (line->vid == 0x1780 && line->pid == 0x0410)
//		size = 64;
//	get_out((unsigned long *)&seqnum, line->out_q_seqnum_array);
//	check_out(seqnum, line->out_q_seqnum_array);
//	line->queue_num = line->queue_num + 1;
//	//out("���к�Ϊ:%d\n", seqnum);
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
//	out("��interrupt_data_to_dev��---------!!!!!!!�������м��������:size:%d\n", size);
//	for (i = 0; i < 48 + size; i++)
//	{
//		printf("%02x ", out_buf[i]);
//	}
//	printf("\n\n\n");
//#endif
//	_lock_un(line->lock);
//
//	line->queue_num = line->queue_num - 1;
//	//_task_week_up(line->interrupt_task);
//	return;
//}
//






