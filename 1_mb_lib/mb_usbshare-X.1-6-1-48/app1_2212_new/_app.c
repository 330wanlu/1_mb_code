#define _app_c
#include "_app.h"

int add_soft_module()
{
	//out("��ʼ���ظ�ģ��\n");
	if (switch_dev_add(DF_MB_HARDWARE_A20_A33, app_note.type)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��Ƭ������ģ������ʧ��\r\n");
		return -1;
	}
	//out("����USBִ���ļ�\n");
	if (ty_usb_add("/dev/switch", DF_TY_MACHINE_TYPE)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�˿�ģ��ʧ��\r\n");
		return -2;
	}
	//out("�����¼��ļ�\n");
	if (event_file_add("/dev/switch")<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�����¼�ģ��ʧ��\r\n");
		return -3;
	}
	//out("���������ʱ�ļ�\n");
	if (_get_net_time_add()<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���������ʱģ��ʧ��\r\n");
		return -4;
	}
	//out("����ת���ļ�\n");
	if (tran_data_add(DF_TY_TRAN_PORT, &app_note) < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����ת��ģ��ʧ��\r\n");
		return -5;
	}
	//out("����UDP�����ļ�\n");
	if (_udp_sev_add(DF_TY_UDP_SER_PORT)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "����UDP����ģ��ʧ��\r\n");
		return -7;
	}
	//out("���������ļ�\n");
	if (deploy_add((_so_note *)&app_note, "/dev/switch", "/dev/ty_usb", DF_MB_HARDWARE_A20_A33)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��������ģ��ʧ��\r\n");
		return -8;
	}
	//out("�����߳��ػ��ļ�\n");
	if (ty_pd_add("/dev/switch")<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�����ػ�����ģ��ʧ��\r\n");
		return -9;
	}
	//out("���Ӷ˿ڹ����ļ�\n");
	if (ty_usb_m_add("/dev/ty_usb", DF_MB_HARDWARE_A20_A33)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���Ӷ˿ڹ���ģ��ʧ��\r\n");
		return -10;
	}
	//out("�����豸��Ϣ�����ļ�\n");
	if (machine_infor_add("/dev/switch", &app_note, DF_MB_HARDWARE_A20_A33)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ӻ�����Ϣģ��ʧ��\r\n");
		return -11;
	}
	//out("����socket�����������ļ�\n");
	if (_socket_sev_add(DF_TY_SEV_ORDER_PORT)<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ӷ���˿�ʧ��\r\n");
		return -12;
	}
	if (get_net_state_add() <0)
	{
		logout(INFO, "SYSTEM", "INIT", "��������״̬ģ��ʧ��\r\n");
		return -13;
	}
	if (vpn_set_add() <0)
	{
		logout(INFO, "SYSTEM", "INIT", "����VPNģ��ʧ��\r\n");
		return -14;
	}
#ifndef MTK_OPENWRT
	//out("����LCDģ��\n");
	if (lcd_state_add())
	{
		logout(INFO, "SYSTEM", "INIT", "����LCD��ʾģ��ʧ��\r\n");
		return -15;
	}
#endif
	if (_m_server_add())
	{
		logout(INFO, "SYSTEM", "INIT", "����mqttͬ��ģ��ʧ��\r\n");
		return -16;
	}
#ifndef DF_OLD_MSERVER
	if (_invoice_transmit_add())
	{
		logout(INFO, "SYSTEM", "INIT", "�����ն���ά����ģ��ʧ��\r\n");
		return -17;
	}
#endif
#ifndef MTK_OPENWRT
	if (bluetooth_add("/dev/ttyS2", &app_note, "/dev/switch"))
	{
		return -18;
	}
#endif
	return 0;
}

int app_main(void)
{   
	struct _app     stream;
	int result;
	char name[DF_FILE_NAME_MAX_LEN] = { 0 };
	logout(INFO, "SYSTEM", "INIT", "\n\n");
	//out("��ʼ�����ⲿ����\n");
	logout(INFO, "SYSTEM", "INIT", "����ʼ����,���ظ�Ӧ��ģ��\r\n");
	main_start_init();
	//out("�ⲿ�����ʼ�����\n");
    memset(&stream,0,sizeof(stream));
	signal(SIGPIPE, recvSignal);
	signal(SIGSEGV, SIG_IGN); 
    //out("���ӿ������ļ�ϵͳ\n");
	
	if (add_soft_module() < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����ģ�����ʧ��,�����˳�����\r\n");
		return -1;
	}
	
	logout(INFO, "system", "����", "ϵͳ���������,����ģ���������,��������ģ�鼰Ӳ��\r\n");

	stream.switch_fd=ty_open("/dev/switch",0,0,NULL);
	if (stream.switch_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��Ƭ������ģ���ʧ��\r\n");
		return -2;
	}
#ifndef MTK_OPENWRT
	//out("��LCDģ�鲢��ʼ����ʾ\n");
	stream.lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
	if (stream.lcd_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "lcdģ���ʧ��\r\n");
		return -3;
	}
	memset(&lcd_print_info.version, 0, sizeof(lcd_print_info.version));
	memcpy(&lcd_print_info.version, app_note.ver,8);
	lcd_print_info.usbshare_en = 0;
	lcd_print_info.sd_en = 1;
	lcd_print_info.udp_en = 0;
	lcd_print_info.mqtt_en = 1;
	lcd_print_info.mqtt_state = 1;
	//����lcd��ʾ����ʾ
	ty_ctl(stream.lcd_fd, DF_LCD_STATE_UPDATE_STATE);
#endif
	stream.ty_pd_fd=ty_open("/dev/ty_pd",0,0,NULL);
	if (stream.ty_pd_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�ػ�����ģ���ʧ��\r\n");
		return -4;
	}

	stream.ty_usb_fd=ty_open("/dev/ty_usb",0,0,NULL); 
	if(stream.ty_usb_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��/dev/ty_usbʧ��\r\n");
		return -5;
	}

	stream.machine_fd = ty_open("/dev/machine", 0, 0, NULL);
	if (stream.machine_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ն���Ϣģ��ʧ��\r\n");
		return -6;
	}

	stream.deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
	if (stream.deploy_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "������ģ��ʧ��\r\n");
		return -7;
	}

	stream.net_time_fd = ty_open("/dev/get_net_time", 0, 0, NULL);
	if (stream.net_time_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�������ʱģ��ʧ��\r\n");
		return -8;
	}

	stream.mq_sync_fd = ty_open("/dev/mq_sync", 0, 0, NULL);
	if (stream.mq_sync_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��MQTTͬ��ģ��ʧ��\r\n");
		return -9;
	}

	stream.ty_usb_m_fd = ty_open("/dev/ty_usb_m", 0, 0, NULL);
	if (stream.ty_usb_m_fd < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��USB�˿�ģ��ʧ��,֪ͨ��Ƭ�������������\r\n");
		sleep(10);
		ty_ctl(stream.machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
		return -10;
	}
	
	stream.mqtt_lock = _lock_open("_app.c", NULL);
	if (stream.mqtt_lock<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���̻߳�������ʧ��\r\n");
		return -11;
	}
	stream.data_lock = _lock_open("data_gw", NULL);
	if (stream.data_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��mqtt������ʧ��\r\n");
		return -12;
	}

	stream.sql_lock = _lock_open("_app.c", NULL);
	if (stream.sql_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "��mysql��ʧ��\r\n");
		return -13;
	}

	stream.inv_read_lock = _lock_open("_app.c", NULL);
	if (stream.inv_read_lock < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪶�ȡ��Ʊ��ʧ��\r\n");
		return -14;
	}
	memset(&message_callback, 0, sizeof(struct _message_callback));
	memset(&message_history, 0, sizeof(struct _message_history));
	stream.m_task_fd = _task_open("deal_message", task_deal_message_queue, &stream, 1, NULL);
	if (stream.m_task_fd < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����Ϣ�����߳�ʧ��\r\n");
		return -15;
	}
	fd_stream = &stream;
	stream.connect_time = 1;
	result = ty_ctl(stream.machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, stream.mqtt_server, &stream.mqtt_port, stream.user_name, stream.user_passwd);
	if (result < 0)
	{
		out("��ȡ��¼�õ�MQTT�û�����ʧ��\n");
#ifdef MTK_OPENWRT
		ty_ctl(stream.ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI, 1001);//��Դ�ƺ�������
#endif
	}
	else
	{
		out("��ȡ��¼�õ�MQTT�û�����ɹ�,server %s,port %d,usr_name %s,passwd %s\n", stream.mqtt_server, stream.mqtt_port, stream.user_name, stream.user_passwd);
		char id_sub[200] = { 0 };
		char id_pub[200] = { 0 };
		memset(id_sub, 0, sizeof(id_sub));
		memset(id_pub, 0, sizeof(id_pub));
		sprintf(stream.topic, "mb_fpkjxt_%s", stream.user_name);
		sprintf(id_sub, "sub%s", stream.user_name);
		sprintf(id_pub, "pub%s", stream.user_name);
#ifdef MTK_OPENWRT
		ty_ctl(stream.ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_ON, 1001);//
#endif
		_mqtt_client_init(sub_message_callback_function_sync, sub_err_callback_deal_function, stream.mqtt_server, stream.mqtt_port, stream.topic, stream.user_name, stream.user_passwd, id_sub, id_pub);
	}

    stream.event_file_fd=ty_open("/dev/event",0,0,NULL); 
	if (stream.event_file_fd < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "���¼�ģ��ʧ��\r\n");
		return -12;
	}
    memset(name,0,sizeof(name));
	sprintf(name, "/dev/socket_sev/sever/%d", DF_TY_SEV_ORDER_PORT);
    stream.sock_fd=ty_open(name,0,0,NULL);
    if(stream.sock_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ļ�:/dev/socket_sev/sever/10001 ʧ��\r\n");
        return stream.sock_fd;
    }
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_HOOK,&stream,_app_order);
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_CLOSE_TYPE,1);
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_OPEN_KEEP,1);
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_NODELAY,1);
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_REV_TIMER,10);
    ty_ctl(stream.sock_fd,DF_SOCKET_SEV_CM_SEND_TIMER,10);
	//printf("======socket server file open success======\n");

	memset(name,0,sizeof(name));
	sprintf(name, "/dev/udp_sev/%d", DF_TY_UDP_SER_PORT);
	stream.udp_fd=ty_open(name,0,0,NULL);
	if(stream.udp_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ļ���/dev/udp_sev/10002ʧ��\r\n");
		return stream.udp_fd;
	}
	//out("UDP �ļ��򿪳ɹ�\n");
	stream.task=_task_open("udp_ser",udp_ser_main_task,&stream,DF_UDP_SEV_MAIN_TASK_TIMER,NULL);
	if (stream.task<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��udp����ģ��ʧ��\r\n");
		return stream.udp_fd;
	}

	//out("��update file �ɹ�\n");
	stream.get_net_fd=ty_open("/dev/get_net_state",0,0,NULL); 
	if(stream.get_net_fd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪻�ȡ����״̬ģ��ʧ��\r\n");
		return stream.get_net_fd;
	}

	stream.vpnfd=ty_open("/dev/vpn_set",0,0,NULL);
	if(stream.vpnfd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "��vpn����ʧ��\r\n");
		return stream.vpnfd;
	}

#ifndef DF_OLD_MSERVER
	stream.opsfd = ty_open("/dev/invoice_transmit", 0, 0, NULL);
	if (stream.opsfd<0)
	{
		logout(INFO, "SYSTEM", "INIT", "���ն���ά����ʧ��\r\n");
		return stream.opsfd;
	}
#endif
#ifndef MTK_OPENWRT
	stream.ble_fd = ty_open("/dev/bluetooth", 0, 0, NULL);
#endif
	result = _task_open("update_task", update_task_hook, &stream, 1000 * 20, NULL);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "INIT", "�򿪳�����Ȩ�����߳�ʧ��\r\n");
		return result;
	}
	//printf("======network state file open sucess======\n");
	//printf("start Mysql !\n");
	//system("chmod 644 /etc/my.cnf");
	//system("/etc/init.d/mysqld start &");
	printf("========================Start success,going to monitor mode version %s date %s============================\n", app_note.ver, app_note.r_date);

	logout(INFO, "system", "����", "�ն������������,������ţ�%s,�汾:%s,����%s,����:%s\r\n", stream.user_name, app_note.ver, app_note.v_num, app_note.r_date);


    for(;;sleep(10)); 
}

static int check_port_status(struct _app  *par)
{
	struct _usb_port_infor usb_t;
	int result;
	uint8 s_buf[1024] = { 0 };
	int i, usb_sum;
	memset(s_buf, 0, sizeof(s_buf));
	usb_sum = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	s_buf[0] = usb_sum;
	for (i = 1; i <= usb_sum; i++)
	{
		//out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
		result = fun_get_plate_usb_info(i, &usb_t);
		if (result<0)
		{   //out("��ȡUSB�˿���Ϣʧ��result=%d\n",result);
			continue;
		}
		if (usb_t.with_dev == 0)
		{
			continue;
		}
		if (usb_t.port_status == 0)
		{
			continue;
		}
		else
		{//˰��ʹ���з����
			if (usb_t.port_used == 1 && usb_t.system_used == 0)
				return -1;
			if ((strcmp(usb_t.used_info, DF_PORT_USED_USB_SHARE) == 0) || 
				//(strcmp(usb_t.used_info, DF_PORT_USED_DQMCSHXX) == 0) ||
				(strcmp(usb_t.used_info, DF_PORT_USED_FPKJ) == 0) ||
				(strcmp(usb_t.used_info, DF_PORT_USED_FPZF) == 0))
				return -1;
			else
				continue;
		}
	}
	return 0;
}
void get_boot_config_md5(unsigned char *outbuf)
{
	FILE* fp = NULL;
	fp = fopen("/etc/boot_update.info", "r");
	if (fp == NULL)
		return;
	fseek(fp, 0, SEEK_SET);
	fgets((char *)outbuf, 256, fp);
	fclose(fp);
	return;
}

void jugde_boot_update(struct _app  *par)
{
	int result;
	unsigned char jugde_Md5[50] = { 0 };
	unsigned char boott_Md5[50] = { 0 };
	unsigned char mb_bootMd5[50] = { 0 };
	get_boot_config_md5(jugde_Md5);
	//��ȡ���µ�boot MD5ֵ
	result = read_file_md5("/home/share/exe/boot_t", boott_Md5);
	if (result < 0)
	{
		printf("boot_t open or md5 failed!\n ");
		par->boot_update = 1;
		return;
	}
	printf("boot_md5:%s\n", boott_Md5);
	if (memcmp(boott_Md5, jugde_Md5, strlen((char *)boott_Md5)) != 0)//����������boot������ȷ���Ѿ��滻���
	{
		printf("Boot incorrect or Already update!\n ");
		par->boot_update = 1;
		return;
	}
	read_file_md5("/home/share/exe/mb_boot", mb_bootMd5);

	if (memcmp(mb_bootMd5, jugde_Md5, strlen((char *)mb_bootMd5)) == 0)
	{
		printf("Do not need to update boot!\n ");
		par->boot_update = 1;
		return;
	}
	else
	{
		//get_boot_size(&boot_size);
		//if (boot_size < 129584)//�ж�boot_size��С�Ƿ�����
		{
			//�쳣  ���и���
			if (access("/home/share/exe/boot_t", 0) == 0)
			{
				result = detect_process("/home/share/exe/mb_boot", "tmpm.txt");
				if (result > 0)
				{
					write_log("/home/share/exe/mb_boot���̴���,�����\n");
					check_and_close_process("/home/share/exe/mb_boot");
				}

				system("mv /home/share/exe/boot_t /home/share/exe/mb_boot");
				system("chmod 777 /home/share/exe/mb_boot");
				sync();

				logout(INFO, "UPDATE","BOOT_UPDATE", "mb_boot�����滻���\r\n");
			}
		}
	}
}
void update_task_hook(void *arg, int timer)
{
	FILE *fp_app,*fp_auth,*fp_need;
	int result = 0;
	char cmd[200] = { 0 };
	struct _app  *par;
	par = (struct _app  *)arg;
#ifndef MTK_OPENWRT
	if (par->boot_update == 0)
	{
		jugde_boot_update(par);
	}
#endif
	if (access("/etc/update_app_mb.log", 0) == 0)
	{
		fp_app = fopen("/etc/update_app_mb.log", "rb");
		if (fp_app != NULL)
		{
			fclose(fp_app);
			goto update;
		}
	}
	if (access("/etc/update_auth_mb.log", 0) == 0)
	{
		fp_auth = fopen("/etc/update_auth_mb.log", "rb");
		if (fp_auth != NULL)
		{
			fclose(fp_auth);
			goto update;
		}
	}
	if (access("/etc/need_update_auth.log", 0) == 0)
	{
		fp_need = fopen("/etc/need_update_auth.log", "rb");
		if (fp_need != NULL)
		{
			fclose(fp_need);
			goto update;
		}
	}

	return;
update :
	out("��Ҫ������������������������\n");
	result = check_port_status(par);
	if (result < 0)////�ж˿����ڱ�ʹ��
	{
		out("���ڶ˿�ռ���ݲ���������\n");
		return;
	}

	_task_close(par->m_task_fd);
	out("�ر�mqtt��Ϣ���д����̳߳ɹ�\n");


	if (access("/etc/update_auth_mb.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/update_auth_mb.log");
		system(cmd);	
		logout(INFO, "SYSTEM", "UPDATE", "��⵽��Ȩ����,������������\r\n");
		sync();
	}

	if (access("/etc/update_app_mb.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/update_app_mb.log");
		system(cmd);		
		logout(INFO, "SYSTEM", "UPDATE", "��⵽�������,�������³���\r\n");

		sync();
	}

	if (access("/etc/need_update_auth.log", 0) == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm -rf /etc/need_update_auth.log");
		system(cmd);		
		logout(INFO, "SYSTEM", "UPDATE", "��⵽��Ҫ�������,5��󼴽���������\r\n");

		sync();

		sprintf(cmd, "rm -rf /home/share/exe/tar/");
		system(cmd);	
		sync();

		sleep(5);
		ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}

	/*memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "reboot");
	system(cmd);*/
	logout(INFO, "SYSTEM", "UPDATE", "��⵽��Ҫ�������,�˳�����\r\n");

	exit(0);
	return ;


}


#ifdef MTK_OPENWRT
//for mb_boot

void update_boot_task_hook(void *arg, int timer)
{

	int result = 0;
	struct _app  *par;
	par = (struct _app  *)arg;
	result = mb_boot_update((uint8 *)"mb_boot");
	if (result < 0)
	{
		out("boot������»�����ʧ��\n");
		return;
	}

	out("boot������Ҫ������������������������\n");
	result = check_port_status(par);
	if (result < 0)////�ж˿����ڱ�ʹ��
	{
		out("���ڶ˿�ռ���ݲ���������-----boot\n");
		return;
	}
	if (access("/tmp/boot_update", 0) == 0)
	{
		logout(INFO, "SYSTEM", "UPDATE", "��⵽��Ҫboot�������,5��󼴽���������\r\n");

		sync();
		sleep(5);
		int ret = 0;
#ifdef RELEASE_SO
		so_common_report_event(NULL, "����", "����boot���������ɹ�", ret);
#else
		function_common_report_event(NULL, "����", "����boot���������ɹ�", ret);
#endif	

		ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}
	return;
}

int File_Compare_MD5(char *src_file, char *comp_file)
{
	int fds, fdc;
	int sizes = 0, sizec = 0, size_read = 0;
	struct stat statbufs;
	struct stat statbufc;
	char *buff_s = NULL, *buff_c = NULL;
	MD5_CTX ctx;
	char md5_s[16] = { 0 }, md5_c[16] = { 0 };
	if (src_file == NULL || comp_file == NULL)
		return -1;
	if (access(src_file, 0) != 0)
	{

		return -1;
	}
	if (access(comp_file, 0) != 0)
	{
		printf("ԭĿ¼�ļ�������[%s],MD5ֵĬ��Ϊ��ͬ !\n", comp_file);
		return 0;//�����������˵��MD5ֵ��ͬ
	}
	stat(src_file, &statbufs);
	sizes = statbufs.st_size;
	buff_s = malloc(sizes + 10);
	memset(buff_s, 0, sizes + 10);
	if (buff_s == NULL)
	{
		printf("err malloc!\n");
		return -1;
	}


	stat(comp_file, &statbufc);
	sizec = statbufc.st_size;
	buff_c = malloc(sizec + 10);
	memset(buff_c, 0, sizec + 10);
	if (buff_c == NULL)
	{
		printf("err malloc!\n");
		free(buff_s);
		return -1;
	}


	fds = open(src_file, O_RDONLY);
	if (fds < 0)
	{
		printf("tar����%s�ļ���ʧ��!\n", src_file);
		free(buff_c);
		free(buff_s);
		return -1;
	}
	size_read = read(fds, buff_s, sizes);
	if (size_read != sizes)
	{
		printf("tar��%s�ļ���ȡ��С���ļ���С��һ��!\n", src_file);
		free(buff_c);
		free(buff_s);
		close(fds);
		return -1;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_s, sizes);
	MD5_Final((unsigned char *)md5_s, &ctx);

	close(fds);

	size_read = 0;
	fdc = open(comp_file, O_RDONLY);
	if (fdc < 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;
	}
	size_read = read(fdc, buff_c, sizec);
	if (size_read != sizec)
	{
		//printf("read err ! not full!\n");
		free(buff_c);
		free(buff_s);
		close(fdc);
		return 0;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buff_c, sizec);
	MD5_Final((unsigned char *)md5_c, &ctx);

	close(fdc);

	if (memcmp(md5_s, md5_c, 16) != 0)
	{
		free(buff_c);
		free(buff_s);
		return 0;//MD5ֵ��ͬ
	}
	free(buff_c);
	free(buff_s);
	//printf("MD5ֵ��ͬ�������滻!\n");
	return 1;//MD5ֵ��ͬ


}

#define DF_DOWNLOAD_NET_FILE_FTP_ADDR				"www.njmbxx.com"	
#define DF_DOWNLOAD_NET_FILE_FTP_NAME				"njmb"
#define DF_DOWNLOAD_NET_FILE_FTP_PWD				"123456"
#define DF_DOWNLOAD_NET_FILE_FTP_PORT				21
//������ļ���Ҫ����
static int mb_boot_update(uint8 *name)
{
	int result;
	char oper_order[1024] = { 0 };//�ٵ�����
	char down_name[1024] = { 0 };
	struct stat st;
	memset(down_name, 0, sizeof(down_name));
	sprintf(down_name, "/tmp/%s", name);
	if (access("/tmp/boot_update", 0) == 0)//�Ѿ�������  �ȴ�����ر�
	{
		return 0;
	}


	out("ftp:(��[%s],�û���[%s],����[%s])�����ļ�:[%s]��[%s]\n", DF_DOWNLOAD_NET_FILE_FTP_ADDR, DF_DOWNLOAD_NET_FILE_FTP_NAME, DF_DOWNLOAD_NET_FILE_FTP_PWD, name, down_name);
	if (ftp_get_file(DF_DOWNLOAD_NET_FILE_FTP_ADDR, DF_DOWNLOAD_NET_FILE_FTP_NAME, DF_DOWNLOAD_NET_FILE_FTP_PWD, (char *)name, down_name, DF_DOWNLOAD_NET_FILE_FTP_PORT)<0)
	{
		out("ftp:����:%sʧ��\n", down_name);
		return DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR;
	}
	out("���سɹ�\n");
	sync();
	stat(down_name, &st);
	out(" file size = %d\n", st.st_size);
	if (st.st_size == 0)
	{
		out("�ļ���СΪ0\n");
		memset(oper_order, 0, sizeof(oper_order));
		sprintf(oper_order, "rm %s", down_name);
		out("�������%s\n", oper_order);
		system(oper_order);
		return -1;
	}
	//so_report_event()

	result = File_Compare_MD5("/tmp/mb_boot", "/home/share/exe/mb_boot");
	if (result == 0)//�滻boot
	{
		check_and_close_process("/home/share/exe/mb_boot");
		system("rm -rf /home/share/exe/mb_boot");
		sync();
		system("cp /tmp/mb_boot /home/share/exe/.");
		sync();
		system("chmod 777 /home/share/exe/mb_boot");

		system("rm -rf /tmp/mb_boot");
		sync();

		system("touch /tmp/boot_update");
		return 0;
		
	}
	
	return -1;
}
//for mb_boot end
#endif

_so_note    *get_application_version(void)
{
    
	return (_so_note    *)&app_note;
    
}


void recvSignal(int sig)  
{  
	printf("================================received signal %d !!!================================\n",sig);  

}  


static int _app_order(int fd,uint32 ip,uint16 port,void *arg)
{   
	struct _app    *par;
	uint8 buf[2048] = { 0 };
    int result,buf_len,i;
    par=arg;
    buf_len=sizeof(buf);
    memset(buf,0,sizeof(buf));
    result=unpack(fd,buf,&buf_len);
    if(result<0)
    {   //out("���ݰ�����,ip = %08x\n",ip);
		if(result!=-1)
        {   
			answer_no_order(fd,par,NULL,0);
        }
        return result;
    }
    for(i=0;i<sizeof(order_name)/sizeof(order_name[0]);i++)
    {   
		if(order_name[i].order==result)
        {   //out("ִ��:[%s]\n",order_name[i].name);
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
    static int set_start_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)    
    {   
		int start;
        int result;
        out("[set_start_port]:���÷�����USB��ʼ��\n");
        ip=ip;
        start=buf[1]*0x10000+buf[2]*0x100+buf[3];
        if(start==0)
        {   
			return err_out_tcp(fd,DF_ERR_PORT_CORE_TY_USB_SET_START_ZERO);
        }
        out("��ʼ��Ϊ:%d\n",start);
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_START_PORT,start);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_USB_START,NULL,0); 
    }
		//���÷�����IP
	static int set_ip_ip(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
#ifdef MTK_OPENWRT
		int result;
		uint32 t_ip = 0, mask = 0, getway = 0, DNS = 0;
		int type;
		type = buf[0];
		//out("................type:%d\n", type);
		if (type == 3 || type == 4)
		{
			if (type == 4)
			{
				result = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_SET_IP, t_ip, mask, getway, DNS, type);
				if (result<0)
				{
					return err_out_tcp(fd, result);
				}
			}
			else {
				//out("........buf is :%s\n", buf + 1);
				char name[40] = { 0 }, pwd[20] = { 0 }, mode[20] = { 0 };
				/*int len_name = buf[1];
				memcpy(name, buf + 2, len_name);
				int len_pwd = buf[2 + len_name];
				memcpy(pwd, buf + 3+len_name, len_pwd);*/
				uint8 *p = NULL;
				p = (uint8 *)strstr((char *)buf, ":");
				if (p == NULL)
					return err_out_tcp(fd, 43);
				memcpy(name, buf + 5, p - buf - 5);
				//out("........name is :%s\n", name);
				memcpy(pwd, buf + 5 + strlen(name) + 1, strlen((char *)buf) - 5 - strlen(name) - 1);

				//out("........pwd is :%s\n", pwd);
				memcpy(mode, "WPA2PSK", 7);
				result = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_SET_IP_W, name, pwd, mode);
				if (result<0)
				{
					return err_out_tcp(fd, result);
				}
			}
		}
		else {
			t_ip = (uint32)(buf[1] * 0x1000000) + (uint32)(buf[2] * 0x10000) + (uint32)(buf[3] * 0x100) + (uint32)buf[4];
			mask = (uint32)(buf[5] * 0x1000000) + (uint32)(buf[6] * 0x10000) + (uint32)(buf[7] * 0x100) + (uint32)buf[8];
			getway = (uint32)(buf[9] * 0x1000000) + (uint32)(buf[10] * 0x10000) + (uint32)(buf[11] * 0x100) + (uint32)buf[12];
			DNS = (uint32)(buf[13] * 0x1000000) + (uint32)(buf[14] * 0x10000) + (uint32)(buf[15] * 0x100) + (uint32)buf[16];
			out("[set_ip_ip]:����ip=0X%08X,mask=0X%08X,getway=0X%08X,DNS=0X%08X\n", t_ip, mask, getway, DNS);
			logout(INFO, "TCP", "10001","��IP%08x��������������ն�IP����type=%d ip=0x%08x netmask=0x%08x gateway=0x%08x dns=0x%08x\n", ip, type, t_ip, mask, getway, DNS);
			result = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_SET_IP, t_ip, mask, getway, DNS, type);
			if (result<0)
			{
				return err_out_tcp(fd, result);
			}
		}
		return write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SET_IP, NULL, 0);

#else
		int result;
		uint32 t_ip, mask, getway, DNS;
		int type;
		type = buf[0];
		t_ip = (uint32)(buf[1] * 0x1000000) + (uint32)(buf[2] * 0x10000) + (uint32)(buf[3] * 0x100) + (uint32)buf[4];
		mask = (uint32)(buf[5] * 0x1000000) + (uint32)(buf[6] * 0x10000) + (uint32)(buf[7] * 0x100) + (uint32)buf[8];
		getway = (uint32)(buf[9] * 0x1000000) + (uint32)(buf[10] * 0x10000) + (uint32)(buf[11] * 0x100) + (uint32)buf[12];
		DNS = (uint32)(buf[13] * 0x1000000) + (uint32)(buf[14] * 0x10000) + (uint32)(buf[15] * 0x100) + (uint32)buf[16];
		out("[set_ip_ip]:����ip=0X%08X,mask=0X%08X,getway=0X%08X,DNS=0X%08X\n", t_ip, mask, getway, DNS);
		logout(INFO, "TCP", "10001", "��IP%08x��������������ն�IP����type=%d ip=0x%08x netmask=0x%08x gateway=0x%08x dns=0x%08x\r\n", ip, type, t_ip, mask, getway, DNS);

		result = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_SET_IP, t_ip, mask, getway, DNS, type);
		out("IP���ý���,result = %d\n\n\n", result);
		if (result<0)
		{
			return err_out_tcp(fd, result);
		}
		ty_ctl(par->get_net_fd, DF_GET_NET_STATE_CM_CLEAN_STATUS);
		write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SET_IP, NULL, 0);

		//sleep(5);
		//out("��ʼ���\n");
		//
		//out("�������״̬�ɹ�\n");
		///*char ip_asc[20] = { 0 };
		//out("�ٴλ�ȡ����IP\n");
		//ty_ctl(par->deploy_fd, DF_DEPLOY_CM_GET_NET_PAR, &ip, &mask, &getway, &DNS, &type);
		//out("��ѯ��:ip=0X%08X,mask=0X%08X,getway=0X%08X\n,dns=0x%08x,type=%d\n", ip, mask, getway, DNS, type);
		//ip2ip_asc(ip, ip_asc);*/
		//out("����IPΪ%s\n", ip_asc);
		//memset(&lcd_print_info.local_ip, 0, sizeof(lcd_print_info.local_ip));
		//memcpy(&lcd_print_info.local_ip, ip_asc, strlen(ip_asc));
		//ty_ctl(par->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
		return 0;
#endif

	}
		//�����豸�������
    static int set_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		struct _switch_dev_id   id;
        int result;
        out("[set_id]:�����豸ID�Լ���������\n");
        memset(&id,0,sizeof(id));
        asc2bcd(buf,id.id,12);
        asc2bcd(buf+12,id.date,4);
        asc2bcd(buf+17,id.date+2,2);
        asc2bcd(buf+20,id.date+3,2);
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն˻�����Ų���\r\n", ip);
        if((result=ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_SET_ID,&id))<0)
        {   
			out("���û������ʧ��\n");
			logout(INFO, "SYSTEM", "10001", "���û������ʧ��,�������%d\r\n", result);
			return err_out_tcp(fd,result);
        }
		out("���û�����ųɹ������������ն�\n");
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_ID,NULL,0); 
		//return err_out_tcp(fd,DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION);
		
    }
		//���÷�����ʱ��
    static int set_timer(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		out("[set_timer]:���÷�����ʱ��\n");
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն�ʱ�����\r\n", ip);
		SetSystemTime(buf);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_TIMER,NULL,0);
    }
		//���÷���������
    static int set_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   //int port_n;
        int result;
        //int enum_timer;
        //char busid[50];
		char net_a[100] = { 0 };
        //port_n=buf[0];
        //out("�˿����й���:%d\n",port_n);
        //result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_PORT_N,port_n);
        //if(result<0)
        //{   return err_out_tcp(fd,result);
        //}
        //enum_timer=buf[1];
        //out("ö��ʱ��:%d\n",enum_timer);
        //result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_ENUM_TIMER,enum_timer);
        //if(result<0)
        //{   return err_out_tcp(fd,result);
        //}
        //memset(busid,0,sizeof(busid));
        //memcpy(busid,buf+2,20);
        //out("usb���߶˿�:%s\n",busid);
        //result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_BUSID,busid);
        //if(result<0)
        //{   return err_out_tcp(fd,result);
        //}
        memset(net_a,0,sizeof(net_a));
        memcpy(net_a,buf+22,100);
		logout(INFO, "SYSTEM", "10001", "��IP%08x��������������ն˲�������\r\n", ip);
        out("Զ�̸��¶˿�:%s\n",net_a);
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_NET_SOFT,net_a);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_PAR,NULL,0); 
    }
	
	//����������������ַ���û�������
    static int set_update_address(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   //int port_n;
        //int result;
        //int enum_timer;
		char addr[65] = { 0 };
		char user[25] = { 0 };
		char passwd[25] = { 0 };
		char net_a[200] = { 0 };
        //port_n=buf[0];
		
		out("�°����÷�����������ַ\n");
        memset(net_a,0,sizeof(net_a));
		memset(addr,0,sizeof(addr));	
		memset(user,0,sizeof(user));
		memset(passwd,0,sizeof(passwd));
		memcpy(addr,buf,60);
		memcpy(user,buf+60,20);
		memcpy(passwd,buf+80,20);
		sprintf(net_a,"host=[%s],user=[%s],pwd=[%s],timer=[0]",addr,user,passwd);
        //memcpy(net_a,buf+22,100);
		/*
        out("Զ�̸��¶˿�:%s\n",net_a);
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_NET_SOFT,net_a);
        if(result<0)
        {   return err_out_tcp(fd,result);
        }*/
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_UPDATE_ADDR,NULL,0); 
    }

	static int set_tryout_date(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
	{
		uint8 date[4] = { 0 };
		out("[[set_tryout_date]:��ʼ����\n");
        asc2bcd(buf,date,4);
        asc2bcd(buf+5,date+2,2);
        asc2bcd(buf+8,date+3,2);
        out("[set_tryout_date]:������������\n");
		ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_SET_TRY_TIME,date);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_TRYOUT_TIME,NULL,0); 
	}


	static int set_vpn_server_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
	{
		int enable;
		int mode;
		int result;
		uint8 server[50] = { 0 };
		int port;
		uint8 user[20] = { 0 };
		uint8 passwd[20] = { 0 };
		int ftp_port;
		uint8 ftp_user[20] = { 0 };
		uint8 ftp_passwd[20] = { 0 };
		uint8 hold[20] = { 0 };
		int i;
		out("���յ�������:buf_len = %d\n",buf_len);
		for(i=0;i<156;i++)
		{
			printf("%02x ",buf[i]);
		}
		printf("\n");
		memset(server,0,sizeof(server));
		memset(user,0,sizeof(user));
		memset(passwd,0,sizeof(passwd));
		memset(ftp_user,0,sizeof(ftp_user));
		memset(ftp_passwd,0,sizeof(passwd));
		memset(hold,0,sizeof(hold));

		enable = buf[0];
		mode = buf[1];		
		memcpy(server,buf+2,50);
		port = buf[52]*0x100+buf[53];		
		memcpy(user,buf+54,20);		
		memcpy(passwd,buf+74,20);
		ftp_port=buf[94]*0x100+buf[95];		
		memcpy(ftp_user,buf+96,20);		
		memcpy(ftp_passwd,buf+116,20);		
		memcpy(hold,buf+136,20);
		out("�����Ƿ�����VPNΪ%d\n",enable);
		out("����VPN��¼ģʽΪ%d\n",mode);
		out("����VPN��������ַΪ%s\n",server);
		out("����VPN�������˿ں�%d\n",port);
		out("����VPN�ͻ����û���Ϊ%s\n",user);
		out("����VPN�ͻ�������Ϊ%s\n",passwd);
		out("����VPN������FTP���ض˿�Ϊ%d\n",ftp_port);
		out("����VPN������FTP�����û���Ϊ%s\n",ftp_user);
		out("����VPN������FTP�����û�����Ϊ%s\n",ftp_passwd);

		result=ty_ctl(par->vpnfd,DF_VPN_SET_CM_SET_VPN,enable,mode,server,port,user,passwd,ftp_port,ftp_user,ftp_passwd);
		if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
		out("����VPN ���,Ӧ��ͻ���\n");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_VPN_PAR,NULL,0); 
	}

	static int set_close_update(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
	{	
		int result;
		int close_status;
		close_status = buf[0];
		out("�����Ƿ�ر��Զ�����״̬��Ϊ��%d\n",close_status);
		result = ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_CLOSE_UPDATE,close_status);
		if(result < 0)
		{
			return err_out_tcp(fd,result);
		}
		out("�����Զ��������\n");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SET_CLOSE_AUTO_UPDATE,NULL,0); 
	}


    /*----------------------------------��ѯ��-------------------------------------------*/
	//��ȡ����������
    static int get_sev_pro(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)    
	{
		uint8 s_buf[1024] = { 0 };
		uint8 trydate[10] = { 0 };
		int result;
        int start;
		//int i;
		int sys_state;
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
        //out("=====================��ȡ����������========================\n");
		sys_state = ty_read(par->ty_pd_fd,NULL,0);
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
		//out("���Ӳ���汾\n");
		memcpy(s_buf + 51, DF_TY_1502_HARD_EDIT, strlen(DF_TY_1502_HARD_EDIT));
        /*result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_HARD,s_buf+51);
		if(result < 0)
		{	out("���Ӳ���汾ʧ��result = %d\n",result);
		}*/

        //out("����ں���Ϣ\n");
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_KERNEL,s_buf+51+8);
		if(result < 0)
		{	
			out("����ں���Ϣresult = %d\n",result);
		}

        //out("��������汾\n");
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_SOFT,s_buf+51+38);
		if(result < 0)
		{	
			out("��������汾result = %d\n",result);
		}

        //out("����������\n");       
        memset(&id,0,sizeof(id));
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
        bcd2asc(id.id,s_buf+46+51,6);
		if(result < 0)
		{	
			out("get_sev_pro����������result = %d\n",result);
		}

        //out("���ip\n");
        ip=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_IP);
        s_buf[58+51]=((ip>>24)&0xff);s_buf[58+52]=((ip>>16)&0xff);
        s_buf[58+53]=((ip>>8)&0xff);s_buf[58+54]=((ip>>0)&0xff);


        //out("����豸��ʼ��\n");
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        s_buf[62+51]=((start>>24)&0xff);s_buf[63+51]=((start>>16)&0xff);
        s_buf[64+51]=((start>>8)&0xff);s_buf[65+51]=((start>>0)&0xff);
        //out("���USB�˿�����\n");
        
        usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[66+51]=((usb_sum>>24)&0xff);s_buf[67+51]=((usb_sum>>16)&0xff);
        s_buf[68+51]=((usb_sum>>8)&0xff);s_buf[69+51]=((usb_sum>>0)&0xff);           
        //out("�������˿�\n");
		s_buf[70 + 51] = ((DF_TY_SEV_ORDER_PORT >> 8) & 0xff);
		s_buf[71 + 51] = ((DF_TY_SEV_ORDER_PORT >> 0) & 0xff);
        //out("���USBIP�˿�\n");
        int usbip_port = 3240;
	s_buf[72+51]=((usbip_port>>8)&0xff);
	s_buf[73+51]=((usbip_port>>0)&0xff);
        //out("�����������\n");
        time_asc(id.date,s_buf+51+74,2);
		/////////////////////////////////////////////////////////////////////////////////
        //out("�������״̬\n");
        result=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_REGISTER_WHETHER);
		if(result == 0)
		{	
			//out("δע��������\n");
			s_buf[135]=0x00;
			memcpy(s_buf+136,"δע��",6);
		}
		else if(result == 1)
		{	
			//out("��ע�����������Ѽ���\n");
			s_buf[135]=0x01;
			memcpy(s_buf+136,"������",6);
		}
		else if(result == 2)
		{	
			//out("��ע�������ŵ�δ����\n");
			s_buf[135]=0x00;
			result= ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_GET_TRYOUT_WHETHER);
			if(result == 0)
			{	//out("û�г�����������\n");
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
		//out("��������Ϊ��%s\n",s_name);
		sprintf((char *)s_date, "%s", app_note.r_date);
		//out("��������Ϊ��%s\n",s_date);
		sprintf((char *)s_version, "%s_%s_%s", app_note.ver, app_note.r_date, app_note.abdr);
		//out("�����汾Ϊ��%s\n",s_version);
		sprintf((char *)s_writer, "%s_%s", app_note.cpy, app_note.writer);
		//out("��������Ϊ��%s\n",s_writer);
		sprintf((char *)s_note, "%s %s", app_note.note, app_note.type);
		//out("��������Ϊ��%s\n",s_note);
		sprintf((char *)s_code, "%s", app_note.code);
		//out("��������Ϊ��%s\n",s_code);
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
		//{	out("err ����������result = %d\n",result);
		//}
		//out("�������Ϊ��%s\n",machine_id);
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
		//out("Ҫ���͵�����Ϊ��");
		//for(i=0;i<471;i++)
		//{
		//	printf("%02x ",s_buf[i]);
		//}
		//printf("\n");


        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_SEV_PRO,s_buf,471);  
    }
    //��ȡ������״̬
    static int get_sev_status(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
    {   
		//struct _usb_port_state      state;
		uint8 s_buf[2048] = { 0 };
        int len;
		int sys_state;
        //memset(&state,0,sizeof(state));
        memset(s_buf,0,sizeof(s_buf));
        out("================���ϵͳ״̬�Լ�״̬����=======================\n");
		sys_state = ty_read(par->ty_pd_fd,NULL,0);
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
	//��ѯ�˿�״̬
    static int get_port_pro(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
    {   
        int result,start,port;
		uint8 s_buf[1024] = { 0 };
		uint8 dev_id[6];
		uint8 sub_id[6];
		char drivers[DF_TY_PATH_MAX];
		int port_dev;               //�ڻ�������ı��
		int port_sub;               //��������ı��
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        port=(port-start+1);

		struct _usb_port_infor port_info;
		memset(&port_info, 0, sizeof(struct _usb_port_infor));
		result = fun_get_plate_usb_info(port, &port_info);

        if(result<0)
        {   
			out("result=%d\n",result);
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

		struct _usb_file_dev    dev;
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
	static int get_all_port_ca_name(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		int result;
		uint8 *s_buf = calloc(1,1024*1024);

		int i,usb_sum;
		int offset=0;
		int len;
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		for(i=1;i<=usb_sum;i++)
		{
			s_buf[offset+1]=i;
			//out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
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
		//out("Ҫ���͵����ݳ���Ϊ%d��\n",len);
		result = write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_CA_NAME, s_buf, len);
		free(s_buf);
		return	result;
	}

	//��ѯ���ж˿��豸���ͺ�֤��
	static int get_all_port_type(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		int result;
		uint8 *s_buf = calloc(1, 1024 * 1024);
		int i,usb_sum;
		int offset=0;
		int len;
		struct _switch_dev_id   id;
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		 //out("����������\n");       
        memset(&id,0,sizeof(id));
        result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
        bcd2asc(id.id,s_buf+1,6);
		if(result < 0)
		{	
			out("get_sev_pro����������result = %d\n",result);
		}
		for(i=1;i<=usb_sum;i++)
		{
			s_buf[offset+13]=i;
			//out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
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
		//out("Ҫ���͵����ݳ���Ϊ%d��\n",len);
		result = write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SEE_ALL_PORT_TYPE, s_buf, len);
		free(s_buf);
		return	result;
	}

	static int get_all_port_status(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{	
		struct _usb_port_infor usb_t;
		int result;
		uint8 *s_buf = calloc(1, 1024 * 1024);
		int i,usb_sum;
		usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
        s_buf[0]=usb_sum;
		for(i=1;i<=usb_sum;i++)
		{
			//out("��ѯ�ն�%d�Ŷ˿���Ϣ\n",i);
			result = fun_get_plate_usb_info(i, &usb_t);
			if(result<0)
			{   //out("��ȡUSB�˿���Ϣʧ��result=%d\n",result);
				s_buf[i]=0;
				continue;
			}
			if(usb_t.with_dev == 0)
			{	
				s_buf[i]=0;
				continue;
			}
			if (usb_t.port_status == 0)
			{	
				s_buf[i]=1;
				continue;
			}
			else
			{	
				s_buf[i]=2;
				continue;
			}
			s_buf[i]=0;			
		}
		//out("Ҫ���͵�����Ϊ��");
		result = write_ty1502_order(fd, DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR, s_buf, 256);
		free(s_buf);
		return result;
	}

	static int get_ter_soft_version(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
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
		out("��������Ϊ��%s\n",s_name);
		sprintf((char *)s_date, "%s", app_note.r_date);
		out("��������Ϊ��%s\n",s_date);
		sprintf((char *)s_version, "%s_%s_%s", app_note.ver, app_note.r_date, app_note.abdr);
		out("�����汾Ϊ��%s\n",s_version);
		sprintf((char *)s_writer, "%s_%s", app_note.cpy, app_note.writer);
		out("��������Ϊ��%s\n",s_writer);
		sprintf((char *)s_note, "%s %s", app_note.note, app_note.type);
		out("��������Ϊ��%s\n",s_note);
		sprintf((char *)s_code, "%s", app_note.code);
		out("��������Ϊ��%s\n",s_code);
		//s_type=0x01;
		sprintf((char *)s_buf, "%s", s_name);
		//sprintf(s_buf+50,"%d",s_type);
		s_buf[50]=0x01;
		sprintf((char *)s_buf + 51, "%s", s_date);
		sprintf((char *)s_buf + 61, "%s", s_version);
		sprintf((char *)s_buf + 161, "%s", s_writer);
		sprintf((char *)s_buf + 211, "%s", s_note);
		sprintf((char *)s_buf + 261, "%s", s_code);
		//out("Ҫ���͵�����Ϊ��");
		//for(i=0;i<281;i++)
		//{
		//	printf("%02x ",s_buf[i]);
		//}
		//printf("\n");
		////out("Ҫ���͵�����Ϊ��");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR,s_buf,281); 		
	}

	//��ѯUSB�豸��ϸ��Ϣ
	static int get_usb_dev_info(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
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
		//out("get_usb_dev_info ��ѯ�ն�port��%d�Ŷ˿���Ϣ\n",port);
		result = fun_get_plate_usb_info(port, &usb_t);
		if(result<0)
		{   
			out("get_usb_dev_info ��ȡUSB�˿���Ϣʧ��result=%d\n",result);
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
	/*	out("Ҫ���͵�����Ϊ��");
		for(i=0;i<148;i++)
		{
			printf("%02x ",s_buf[i]);
		}
		printf("\n");*/
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_USB_PORT_INFOR,s_buf,148); 
	}

    //��ȡ����������
    static int get_sev_par(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[1024] = { 0 }, t_timer[8] = { 0 };
		out("===============��ȡ�������=============\n");
        memset(s_buf,0,sizeof(s_buf));
        memset(t_timer,0,sizeof(t_timer));
		out("��ȡ��ǰʱ��\n");
        clock_get(t_timer);
		out("ת����ǰʱ��\n");
        time_asc(t_timer,s_buf,5);
        s_buf[19]=1;
        out("�Ƿ�ע��:%d\n",s_buf[19]);
        s_buf[20]=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_PORT_N);
        out("�˿����й���:%d\n",s_buf[20]);
        s_buf[21]=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_ENUM_TIMER);
        out("USBö��ʱ��:%d\n",s_buf[21]);
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_BUSID,s_buf+22);
        out("USB������ַ:%s\n",s_buf+22);
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_SOFT,s_buf+42);
        out("HTTP������ַ:%s\n",s_buf+42);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_PAR,s_buf,142);  
    }
	 //��ȡ������������ַ
    static int get_update_address(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[1024] = { 0 };
		//out("===============��ȡ������������ַ=============\n");
		memset(s_buf,0,sizeof(s_buf));
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_ADDR,s_buf);
        //out("HTTP������ַ:%s\n",s_buf);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_UPDATE_ADDR,s_buf,100);  
    }
    //��ȡ�������쳣��־
    static int get_log(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[10240] = { 0 };
        struct _switch_event_all    all;
        int i;
        memset(&all,0,sizeof(all));
        memset(s_buf,0,sizeof(s_buf));
        ty_ctl(par->event_file_fd,DF_EVENT_CM_READ,&all);
        out("��־����:all.n=%d\n",all.n);
        for(i=0;i<all.n;i++)
		{
			sprintf((char *)s_buf + strlen((char *)s_buf), "%d.[ʱ��]:", i + 1);
			time_asc(all.event[i].clock, s_buf + strlen((char *)s_buf), 4);
			sprintf((char *)s_buf + strlen((char *)s_buf), "[�¼�]:");
            out("�¼�:%d\n",all.event[i].event);
			ty_ctl(par->event_file_fd, DF_EVENT_CM_EVENT_NOTE, all.event[i].event, s_buf + strlen((char *)s_buf));
			sprintf((char *)s_buf + strlen((char *)s_buf), "[ԭ��]:");
            out("ԭ��:%d\n",all.event[i].reason);
			ty_ctl(par->event_file_fd, DF_EVENT_CM_REASON_NOTE, all.event[i].reason, s_buf + strlen((char *)s_buf));
			sprintf((char *)s_buf + strlen((char *)s_buf), "\n");
            //out("��־:[%s]\n",s_buf);    
        }
		out("��־:[%s]\n",s_buf);   
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_LOG,s_buf,strlen((char *)s_buf)); 
    }
	//��ȡ�������
    static int get_net_par(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[256] = { 0 };
        uint32 mask,getway,dns;
		int type;
        ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_NET_PAR,&ip,&mask,&getway,&dns,&type);
        //out("��ѯ��:ip=0X%08X,mask=0X%08X,getway=0X%08X\n,dns=0x%08x,type=%d\n",ip,mask,getway,dns,type);
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
        //out("����ִ�����\n");
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

	static int get_check_all_port(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
	{
		//struct _usb_port_state      state;
		uint8 s_buf[9600] = { 0 };
		//uint8 a_buf[50];
        int i,j=0;
		int port_num;
		int result;
		int err_num=0;
		int len; 
		struct http_parm parm;
		char baseudid[100] = { 0 };
		struct _switch_dev_only 	udid;
		struct _usb_port_infor port_info;
        //memset(&state,0,sizeof(state));
        memset(s_buf,0,sizeof(s_buf));
		
		memset(&parm, 0, sizeof(struct http_parm));
        out("================���ϵͳ״̬�Լ�״̬����=======================\n");

		//result = access(DF_SD_CHECK_FILE,0);
		//if(result < 0)
		//{
		//	out("sd:�����ļ������ڣ�SD��δ���룡����\n");
		//	sprintf((char *)s_buf + 2 + j * 20,"TF������쳣");
		//	j++;
		//	err_num = err_num + 1;
		//}
		/*par->uart_ok = 0;
		a_buf[0]=0x54;
		a_buf[1]=0x59;
		a_buf[2]=0x00;
		a_buf[3]=0x07;
		a_buf[4]=0x60;
		a_buf[5]=0x7F;
		a_buf[6]=0x3B;
		ty_write(par->serial_fd,a_buf,7);
		sleep(1);
		if(par->uart_ok!=1)
		{	out("�����Ի�û���յ�����\n");
			sprintf(s_buf+2+j*20,"���ڼ���쳣");
			j++;
			err_num = err_num+1;			
		}*/
		port_num=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);//��ȡ�ն˶˿�����
		out("���ն�һ����%d��USB�˿�\n", port_num);
		for(i=0;i<port_num;i++)
		{	
			//result = ty_ctl(par->ty_usb_m_fd,DF_TY_USB_M_CM_GET_PORT_ERROR,i);

			memset(&port_info, 0, sizeof(struct _usb_port_infor));
			fun_get_plate_usb_info(i+1, &port_info);			
			if (port_info.with_dev != 1)
			{	
				out("%d�Ŷ˿�û���豸\n",i+1);
				sprintf((char *)s_buf+2+j*20,"%d��USB�˿��쳣",i+1);
				j++;
				err_num = err_num+1;
			}
		}
		result = get_DNS_ip("www.njmbxx.com", parm.ip);
		if (result < 0)
		{
			return -1;
		}
		ty_ctl(par->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, parm.key_s);
		result = ty_ctl(par->switch_fd, DF_SWITCH_CM_GET_SERIAL_NUM, &udid);
		if (result<0)
		{
			out("��ȡΨһ��ʶ��ʧ��\n");
			sprintf((char *)s_buf + 2 + j * 20, "��ԿУ��ʧ��");
			err_num = err_num + 1;
		}
		//memcpy(udid_num, udid.serial_num, sizeof(udid_num));//��ȡ��Ψһ��ʶ��
		memcpy(parm.ter_id, "TERREGENCCKECK", 14);
		memcpy(parm.appid, "TERREGENCCKECK", 14);
		memset(baseudid, 0, sizeof(baseudid));
		encode((char *)udid.serial_num, 12, (char *)baseudid);
		memcpy(parm.cpu_id, baseudid, 16);
		memcpy(parm.app_ver, app_note.ver, strlen(app_note.ver));
		result = Key_Check_Http(&parm);
		if (result<0)
		{
			sprintf((char *)s_buf + 2 + j * 20, "��ԿУ��ʧ��");
			err_num = err_num + 1;
		}
		if(err_num==0)
		{	
			out("err_num=0,û���쳣\n");
			s_buf[0]=0x00;
			s_buf[1]=0x00;
			write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CHECK,s_buf,2);
		}
		s_buf[0]=0x01;
		s_buf[1]=err_num;
		len = err_num*20+2;
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CHECK,s_buf,len); 
	}


	static int get_vpn_server_par(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
	{	//int result;
		int enable;
		int mode;		
		uint8 server[50] = { 0 };
		int port;
		uint8 user[20] = { 0 };
		uint8 passwd[20] = { 0 };
		//uint8 hold[20];
		uint8 s_buf[256] = { 0 };
		int ftp_port;
		uint8 ftp_user[20] = { 0 };
		uint8 ftp_passwd[20] = { 0 };



        ty_ctl(par->vpnfd,DF_VPN_GET_CM_GET_VPN,&enable,&mode,server,&port,user,passwd,&ftp_port,ftp_user,ftp_passwd);
		out("enable = %d,mode = %d,server = %s,port = %d,user = %s,passwd = %s,ftp_port = %d,ftp_user = %s,ftp_passwd = %s\n",enable,mode,server,port,user,passwd,ftp_port,ftp_user,ftp_passwd);
        memset(s_buf,0,sizeof(s_buf));
		s_buf[0]=enable;
		s_buf[1]=mode;
		memcpy(s_buf+2,server,50);
		s_buf[52]=((port>>8)&0xff);s_buf[53]=((port>>0)&0xff);
		memcpy(s_buf+54,user,20);
		memcpy(s_buf+74,passwd,20);
		s_buf[94]=((ftp_port>>8)&0xff);s_buf[95]=((ftp_port>>0)&0xff);
		memcpy(s_buf+96,ftp_user,20);
		memcpy(s_buf+116,ftp_passwd,20);
        out("����ִ�����\n");
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_VPN_PAR,s_buf,156); 
	}

	static int get_close_update(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
	{
		uint8 s_buf[256] = { 0 };
		int close_status;
		close_status = ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_CLOSE_UPDATE);
		//out("�Ƿ�ر��Զ�����״̬��Ϊ��%d\n",close_status);
		s_buf[0]= close_status;
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_SEE_CLOSE_AUTO_UPDATE,s_buf,1); 
	}
    //��ȡĳ�¿�Ʊ����
	static int get_month_fp_num(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}
	//��ȡĳ��Ʊ����
	static int get_fp_details(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

	//��ȡĳ�·�Ʊ����������
	static int get_fp_details_t(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

	//��ȡĳ�·�Ʊjson����
	static int get_fp_details_s(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

    
	//��ȡ���÷�Ʊ����
	static int get_fp_types(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}


	//��ȡ˰�̼����Ϣ
	static int get_monitor_data(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

	//��ȡ��˰�̷�Ʊ����Ϣ
	static int get_fp_counts_info(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

	//��ȡ��˰���������
	static int get_area_code(int fd, uint32 ip, struct _app    *par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}

	//��ѯȫ���˿�˰����Ϣ
	static int get_tax_infos(int fd, uint32 ip, struct _app	*par, uint8 *buf, int buf_len)
	{
		return err_out_tcp(fd, DF_ERR_PORT_CORE_GET_FP_MONITOR_DATA);
	}
	static int get_version_describe(int fd, uint32 ip, struct _app	*par, uint8 *buf, int buf_len)
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
    static int open_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		//struct _usb_pro usb;
        int port,result,app_fd,start;
		uint8 s_buf[200] = { 0 };
		char client[64] = { 0 };
		
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);//��ȡ�������ʼ�˿ں�
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];//�˿ں�
        port=port-start+1;//��ȡ����˿ںž���λ��
        app_fd=0;
		out("[_app.c]���˿ںŴ򿪶˿�\n");
        memset(client,0,sizeof(client));//�ͻ��˱�ʶ��GUID
        if(buf_len>4)//Ϊ���ݲ����ͻ��˱�ʶ���汾��GUIDΪ64λ
        {   //out("���ͻ��˵�client\n");
            result=sizeof(client);
            buf_len-=4;
            if(result>buf_len)
                result=buf_len;
            memcpy(client,buf+4,result);
        }
		if ((result = open_port_attach(port, ip, client, &app_fd))<0)
        {   
			out("�򿪶˿�ʧ��\n");
            return err_out_tcp(fd,result);
        }
		
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
		result = write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_OPEN_USB,s_buf,82);
		count_o +=1;
		out("[���˿ںŴ򿪶˿�] �򿪶˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ],\n\n\n\n\n\n\n\n\n", port, count_o);
        return result;
    }
    //���˿ںŹرն˿�
    static int close_port(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		int port,result,start;
		//out("[_app.c]���˿ںŹرն˿�\n");
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        //app_fd=buf[0]*0x1000000+buf[1]*0x10000+buf[2]*0x100+buf[3];
        port=buf[5]*0x10000+buf[6]*0x100+buf[7];
        port=port-start+1;
		result = close_port_dettach(port);
        if(result<0)
        {   
			return err_out_tcp(fd,result);  
        }
		count_c +=1;
		out("[���˿ںŹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_USB,NULL,0);
    }
    //�������Ŵ򿪶˿�
    static int open_port_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		int result,port,app_fd;
		uint8 id[6] = { 0 }, s_buf[200] = { 0 };
        //struct _usb_pro usb;
		char client[64] = { 0 };
        memset(id,0,sizeof(id));
        asc2bcd(buf,id,12);
        port=buf[13]*0x10000+buf[14]*0x100+buf[15];    

        memset(client,0,sizeof(client));
        if(buf_len>16)
        {   //out("���ͻ��˵�client\n");
            result=sizeof(client);
            buf_len-=16;
            if(result>buf_len)
                result=buf_len;
            memcpy(client,buf+16,result);
        }
		if ((open_port_attach(port, ip, client, &app_fd))<0)
        {   
			out("�򿪶˿�ʧ��\n");
            return err_out_tcp(fd,result);
        }

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
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_ID_OPEN_USB,s_buf,82); 
    }
    //�������Źرն˿�
    static int close_port_id(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		int port,result;
		uint8 id[6] = { 0 };
        //struct _usb_pro usb;
        memset(id,0,sizeof(id));
        asc2bcd(buf,id,12);
        //app_fd=buf[12]*0x1000000+buf[13]*0x10000+buf[14]*0x100+buf[15];
        port=buf[17]*0x10000+buf[18]*0x100+buf[19];
        //if(memcmp(id,usb.dev_id,sizeof(id))!=0)
        //{   
		//	out("û�иû������\n");
        //    return err_out_tcp(fd,-1);
        //}
		result = close_port_dettach(port);
        if(result<0)
        {   
			return err_out_tcp(fd,result);  
        }
		count_c +=1;
		out("[��������Źرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_ID_CLOSE_USB,NULL,0);
    }
	//ǿ�ƹرն˿�
    static int close_port_now(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		int port,result,start;		
		
        start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
        port=buf[1]*0x10000+buf[2]*0x100+buf[3];
        port=port-start+1;
		out("ǿ�ƹرն˿ڡ�%d��\n",port);
		result = close_port_dettach(port);
        if(result<0)
        {   
			return err_out_tcp(fd,result);  
        }	
		out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n",port);
		count_c +=1;
		out("[���˿ں�ǿ�ƹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n",port,count_c);
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_CLOSE_NOW,NULL,0);
    }
    //�ػ�
    static int power_off(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
    {   
		int result;
		out("����ػ�����\n");
        result=ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_POWER_OFF,DF_EVENT_REASON_NET);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_POWER_OFF,NULL,0);    
    }
	//����
    static int heart(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		out("������������\n");
		return err_out_tcp(fd, -1);
    }
		//����
    static int reset(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		out("������������\n");
		ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_RESET_POWER,DF_EVENT_REASON_NET); 
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_RESET,NULL,0);    
    }
		//ע��
    static int regdit(int fd,uint32 ip,struct _app  *par,uint8 *buf,int buf_len)
    {   
		int result;
		out("ע��\n");
        result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_SET_REDIT);
        if(result<0)
        {   
			return err_out_tcp(fd,result);
        }
        return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_REDIT,NULL,0);  
    }
		
	//�򿪶˿ڵ�������USBip
    static int open_port_no_active(int fd,uint32 ip,struct _app    *par,uint8 *buf,int buf_len)
    {   
		return err_out_tcp(fd, -1);
    }
    
	//ά�ִ򿪶˿�
    static int keep_open_port(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
    {   
		//struct _usb_pro usb;
        int port,result,app_fd,start;
		uint8 s_buf[200] = { 0 };
		char client[64] = { 0 };
		out("ά�ִ򿪶˿�\n");
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
        {   //out("���ͻ��˵�client\n");
            result=sizeof(client);
            buf_len-=4;
            if(result>buf_len)
                result=buf_len;
            memcpy(client,buf+4,result);
        }
		if ((result = open_port_attach(port, ip, client, &app_fd))<0)
        //if((result=ty_ctl(par->ty_usb_m_fd,DF_TY_USB_M_CM_SEE_OPEN_PORT,port,ip,client,&app_fd))<0)
        {   
			out("�򿪶˿�ʧ��\n");
            return err_out_tcp(fd,result);
        }
        //memset(&usb,0,sizeof(usb));
        out("���Ժ󷵻�fd=%d\n",app_fd);

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
    

	static int check_version_updata(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
	{
		int result;
		result=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_FTP_DATA);
        if(result<0)
        {   

			if (detect_process("vsftpd", "tmp11.txt") > 0)
			{
				out("%s���̴���,�����\n", "vsftpd");
				system("killall vsftpd");
			}

			if (access("/etc/vsftpd.conf-back", 0) == 0)
			{
				system("vsftpd /etc/vsftpd.conf-back &");
			}

			out("����ʧ�ܣ�����ʧ��ԭ��\n");
			return err_out_tcp(fd,result);
        }
		out("�����ɹ�\n");
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_UPDATA,NULL,0);
	}

	static int close_port_by_dettach(int fd,uint32 ip,struct _app *par,uint8 *buf,int buf_len)
	{
		int result,start,port;
		out("[_app.c]ʹ��Dettach�رն˿�\n");
		start=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_START_PORT);
        if(start<0)
        {   
			return err_out_tcp(fd,start);
        }
		//app_fd=buf[0]*0x1000000+buf[1]*0x10000+buf[2]*0x100+buf[3];
		port=buf[5]*0x10000+buf[6]*0x100+buf[7];
		port=port-start+1;
		result = close_port_dettach(port);
		if(result<0)
		{
			return err_out_tcp(fd,result);
		}
		out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
		count_c += 1;
		out("[���˿ں�ǿ�ƹرն˿�] �رն˿ڳɹ�,�����Ķ˿ں�Ϊ�� %d ��,��������Ϊ[ %d ]\n\n\n\n\n\n\n\n\n", port, count_c);
		return write_ty1502_order(fd,DF_TY_1502_ANSWER_ORDER_OPER_DETTACH_PORT,NULL,0);
	}

/*========================================���󷵻�=============================================*/
static int answer_no_order(int fd,struct _app    *par,uint8 *buf,int buf_len)
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
		out("���������δ�������ô�������\n");
        return err_out_s(fd,255,(uint8 *)"δ֪����");
    }
    memset(o_name,0,sizeof(o_name));
    memset(note,0,sizeof(note));
    memset(name,0,sizeof(name));
    _err_out(err,o_name,note);
    sprintf(name,"[%s]:\"%s\"",o_name,note);
    out("����:[%s]\n",name);
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
    //out("��ȡ�汾��Ϣ\n");
	memset(edit,0,sizeof(edit));
	memset(len_buf,0,sizeof(len_buf));
	memset(order,0,sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
    if(ty_socket_read(fd,edit,sizeof(edit))<0)
    {   //out("��ȡ�汾��Ϣ����\n");
		return -1;
    }
    if((edit[0]!=0x01)&&(edit[1]!=0x10))
    {   //out("��ȡ���İ汾��Ϣ����%02x %02x\n",edit[0],edit[1]);
        return -2;
    }
	//out("�汾��Ϣ�ɹ� %02x %02x\n",edit[0],edit[1]);
    //out("��ȡ������Ϣ\n");
    if(ty_socket_read(fd,len_buf,sizeof(len_buf))<0)
    {   
		out("������Ϣ��ȡʧ��\n");
        return -1;
    }
    buf_len=len_buf[0]*256+len_buf[1];
    //out("��֡����:%d\n",buf_len);
    if(buf_len<6)
    {   
		out("����С��6����\n");
        return -3;
    }
    if(buf_len-6>(*out_len))
    {   
		out("���ȹ���\n");
        return -4;
    } 
    if(ty_socket_read(fd,order,sizeof(order))<0)
    {   
		out("��ȡ������ʧ��\n");
        return -1;
    }
    if(ty_socket_read(fd,out_buf,buf_len-6)<0)
    {   
		out("��ȡ����ʧ��\n");
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
    {   //out("read data by read i = %d\n",i);
		result=read(sock,buf+i,buf_len-i);
        if(result<=0)
		{	//out("read data time out or err \n\n\n\n");
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
	struct _app *stream;
	int result;
	uint8 buf[512] = { 0 };
	int in_len;
	uint8 rcvbuf[512] = { 0 };
	int rcvlen;
	stream=arg;
	//out("stream->udp_fd = %d\n",stream->udp_fd);
	result = ty_read(stream->udp_fd,buf,sizeof(buf));
	if(result < 0)
	{
		out("��ȡUDP����ʧ��\n");
		return ;
	}
	in_len = result;
	//out("main task result =%d in_len =%d\n",result,in_len);
	result = udp_unpack(buf,in_len,rcvbuf,&rcvlen);
	if(result<0)
	{
		//out("UDP���ݽ�������\n");
		result = udp_unpack_new_json(buf, in_len);
		if (result == 0)//�½ӿ�Э��������豸����
		{
			udp_get_sev_pro_new(stream->udp_fd, stream);
		}
		return ;
	}
	if(result == DF_MB_ORDER_SEE_USB_SHARE_DEV)
	{	//out("������Ϊ��ȡ�ն���Ϣ stream->udp_fd=%d\n",stream->udp_fd);
		udp_get_sev_pro(stream->udp_fd,stream,rcvbuf,rcvlen);
	}
	else if(result == DF_MB_ORDER_OPER_RESTART)
	{	
		out("������Ϊ�����ն� stream->udp_fd=%d\n",stream->udp_fd);
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
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(object, "protocol");
	if (item != NULL)
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
		if (memcmp(item->valuestring, "mb_usbshare_20191024", 20) != 0)
		{
			cJSON_Delete(root);
			return -1;
		}
			
	}
	item = cJSON_GetObjectItem(object, "cmd");
	if (item != NULL)
	{
		//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n", item->type, item->string, item->valuestring);
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
    //out("��ȡ�汾��Ϣ\n");
    if((in_buf[0]!=0x01)&&(in_buf[1]!=0x10))
    {   //out("�汾��Ϣ����\n");
        return -2;
    }
    //out("��ȡ������Ϣ\n");
    buf_len=in_buf[2]*256+in_buf[3];
    //out("��֡����:%d\n",buf_len);
    if(buf_len<6)
    {   
		out("����С��6����\n");
        return -3;
    }

    if(buf_len!=in_len)
    {   
		out("���ݳ��ȴ���buf=%d in_len=%d\n",buf_len,in_len);
        return -4;
    } 
    order[0]=in_buf[4];
	order[1]=in_buf[5];

	memcpy(out_buf,in_buf+6,buf_len-6);
    (*out_len)=(buf_len-6);
    return (order[0]*256+order[1]);
}

//upd��������Ҫ���ն�����
static int udp_order_sev_restart(int fd,struct _app  *par,uint8 *buf,int buf_len)    
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
    out("=====================���������Ϣ========================\n");
   
	out("����������\n");
	memset(oper_id,0,sizeof(oper_id));
	memset(machine_id,0,sizeof(machine_id));
    memset(&id,0,sizeof(id));
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
	bcd2asc(id.id, (uint8 *)machine_id, 6);
	if(result < 0)
	{	
		out("����������result = %d\n",result);
	}
	out("��ȡ�����������Ϊ%s\n",machine_id);
	memcpy(oper_id,buf,12);
	if(memcmp(machine_id,oper_id,12)!=0)
	{
		out("��������Ҫ������ID��һ��,��ִ���κβ���\n");
		return 0;
	}
	out("������������\n");
	logout(INFO, "SYSTEM", "udp", "�ɿͻ���UDP��������������������\r\n");

	ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_RESET_POWER,DF_EVENT_REASON_NET); 
	udp_write_order(par->udp_fd,DF_MB_ANSWER_OPER_RESTART,NULL,0);
	out("�ɹ��������������ʱ10����˳��ó���\n");
	sleep(10);
	logout(INFO, "SYSTEM", "udp", "UDP�ɹ�������������,�˳�����\r\n");
	exit(0);

	return 0;  
}


static int udp_get_sev_pro_new(int fd, struct _app  *par)
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
	//struct _machine_s   machine;
	//_lock_set(par->lock);
	memset(s_buf, 0, sizeof(s_buf));
	//out("=====================���������Ϣ========================\n");
	ip = ty_ctl(par->deploy_fd, DF_DEPLOY_CM_GET_IP);
	//out("���ip = %08x\n",ip);
	sprintf(ip_str, "%d.%d.%d.%d", ((ip >> 24) & 0xff), ((ip >> 16) & 0xff), ((ip >> 8) & 0xff), ((ip >> 0) & 0xff));
	
	//out("�������˿�= %d\n",par->order_port);
	sprintf(order_port, "%d", DF_TY_SEV_ORDER_PORT);

	//out("���USBIP�˿�= %d\n",par->usbip_port);
	//int usbip_port = 3240;
	
	//out("����������\n");
	memset(machine_id, 0, sizeof(machine_id));
	memset(&id, 0, sizeof(id));
	result = ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, (uint8 *)machine_id, 6);
	if (result < 0)
	{	//out("����������result = %d\n",result);
	}
	
	//out("���USB�˿�����\n");
	usb_sum = ty_ctl(par->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	sprintf(usb_sum_str, "%d", usb_sum);
	//out("��������汾\n");
	result = ty_ctl(par->machine_fd, DF_MACHINE_INFOR_CM_GET_SOFT, soft_ver);
	if (result < 0)
	{
		out("��������汾result = %d\n", result);
	}
	//out("��������ͺ�\n");

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
	return ty_write(fd, s_buf, strlen((char*)s_buf));
	//return udp_write_order(par->udp_fd, DF_MB_ANSWER_SEE_USB_SHARE_DEV, s_buf, 42);
}

//��ȡ����������
static int udp_get_sev_pro(int fd,struct _app  *par,uint8 *buf,int buf_len)    
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
    //out("=====================���������Ϣ========================\n");
    ip=ty_ctl(par->deploy_fd,DF_DEPLOY_CM_GET_IP);
	//out("���ip = %08x\n",ip);
    s_buf[0]=((ip>>24)&0xff);s_buf[1]=((ip>>16)&0xff);
    s_buf[2]=((ip>>8)&0xff);s_buf[3]=((ip>>0)&0xff);       
    //out("�������˿�= %d\n",par->order_port);
	s_buf[4] = ((DF_TY_SEV_ORDER_PORT >> 8) & 0xff);
	s_buf[5] = ((DF_TY_SEV_ORDER_PORT >> 0) & 0xff);
    //out("���USBIP�˿�= %d\n",par->usbip_port);
	int usbip_port = 3240;
	s_buf[6]=((usbip_port>>8)&0xff);
	s_buf[7]=((usbip_port>>0)&0xff);
	//out("����������\n");
	memset(machine_id,0,sizeof(machine_id));
    memset(&id,0,sizeof(id));
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_ID,&id);
    bcd2asc(id.id,(uint8 *)machine_id,6);
	if(result < 0)
	{	//out("����������result = %d\n",result);
	}
	memcpy(s_buf+8,machine_id,12);
	//out("���USB�˿�����\n");
    usb_sum=ty_ctl(par->ty_usb_fd,DF_TY_USB_CM_PORT_NUMBER);
    s_buf[20]=((usb_sum>>24)&0xff);s_buf[21]=((usb_sum>>16)&0xff);
    s_buf[22]=((usb_sum>>8)&0xff);s_buf[23]=((usb_sum>>0)&0xff);     
	//out("��������汾\n");
    result = ty_ctl(par->machine_fd,DF_MACHINE_INFOR_CM_GET_SOFT,s_buf+24);
	if(result < 0)
	{	
		out("��������汾result = %d\n",result);
	}
	//out("��������ͺ�\n");

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
	//out("par->udp_fd = %d\n",par->udp_fd);
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
		//out("ʹ��UDP�������� fd=%d\n",fd);
        return ty_write(fd,s_buf,len);
    }
    else
    {   
		if((result=ty_write(fd,s_buf,6))<0)
            return result;
        return ty_write(fd,buf,buf_len);
    }
}
static int invoice_fpqqlsh_get_using(struct _app *stream, struct _make_invoice_result *invoice, struct _message_line *message_line)
{
	int mem_i;
	_lock_set(stream->sql_lock);
	for (mem_i = 0; mem_i < MAX_MESSAGE_QUEUE_NUM; mem_i++)
	{
		if (message_callback.message_line[mem_i].state == 0)
			continue;
		if (strcmp((const char *)message_callback.message_line[mem_i].fpqqlsh, (const char *)invoice->fpqqlsh) == 0)
		{
			_lock_un(stream->sql_lock);
			out("��Ʊ��ˮ��%s����,�ڵ�%d����Ϣ����\n", invoice->fpqqlsh, mem_i);
			return DF_TAX_ERR_CODE_FPQQLSH_IS_INUSING;
		}
	}
	memcpy(message_line->fpqqlsh, invoice->fpqqlsh, strlen((const char *)invoice->fpqqlsh));
	_lock_un(stream->sql_lock);
	return 0;
}

#ifndef MTK_OPENWRT
//static int mysql_re_connect_e(struct _app *stream, MYSQL	*mysql_fd)
//{
//	//uint32 ip, mask, getway, dns;
//	//int type;
//	int result;
//	struct _net_status net_status;
//	memset(&net_status,0,sizeof(struct _net_status));
//	//mysql_fun_close(&stream->mysql_fd);
//	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
//	//out("��ѯ��:ip=0X%08X,mask=0X%08X,getway=0X%08X\n,dns=0x%08x,type=%d\n", ip, mask, getway, dns, type);
//	//ip2ip_asc(ip, stream->local_ip);
//	//out("����IPΪ%s\n", stream->local_ip);
//	result = mb_mysql_fun_connect(mysql_fd, net_status.ip_address, "root", "123456", "mbdb", 3306);
//	if (result < 0)
//	{
//		out("======================�ٴ��������ݿ�ʧ��======================\n");
//		//stream->mysql_ok = 0;
//		return -1;
//	}
//	//stream->mysql_ok = 1;
//	return 0;
//}
static int sql_callback(void * notused, int argc, char ** argv, char ** szColName)
{
	//printf("into callback\n");
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("%s = %s\n", szColName[i], argv[i] == 0 ? "NULL" : argv[i]);
		if (argv[i] != NULL)
		{
			strcpy(notused, argv[i]);
			break;
		}
	}

	return 0;
}
static int invoice_fpqqlsh_get_data(struct _app *stream,char *kp_topic, char *plate_num, struct _make_invoice_result *invoice)
{
	int result;
	char cmd[1024] = { 0 };
	//char line_s[10] = { 0 };
	//int line;
	char id_s[10] = {0};
	char str_tmp[100] = { 0 };
	int id;
	//MYSQL	mysql_fd;
	sqlite3 *db = NULL;
	char *errinfo;

	out("�����ȡ��Ʊ��ˮ�����ݿ⺯��\n");
	_lock_set(stream->sql_lock);
	//result = mysql_re_connect_e(stream, &mysql_fd);
	result = sqlite3_open("/mnt/masstore/mbdb.db", &db);
	_lock_un(stream->sql_lock);
	if (result < 0)
	{
		out("���ݿ�����ʧ��\n");
		return -1;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select idtb_invoice from tb_invoice where fpqqlsh = '%s';", invoice->fpqqlsh);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, id_s);
	result = sqlite3_exec(db, cmd, sql_callback, id_s, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(id_s) == 0))
	{
		out("���ݿ����ʧ�� 1\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -2;
	}
	id = atoi(id_s);
	//out("��ȡ���ĸ÷�Ʊ��ˮ��:%s,���ڵļ�¼��Ϊ%d\n", invoice->fpqqlsh,id);


	//out("ͨ����¼�Ż�ȡ��Ʊ��������\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select kp_topic from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, kp_topic);
	result = sqlite3_exec(db, cmd, sql_callback, kp_topic, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(kp_topic) == 0))
	{
		out("���ݿ����ʧ�� 2\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -3;
	}
	//out("ͨ����¼�Ż�ȡ��Ʊ��������:%s\n", kp_topic);



	//out("ͨ����¼�Ż�ȡ�̺�\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select plate_num from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, plate_num);
	result = sqlite3_exec(db, cmd, sql_callback, plate_num, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(plate_num) == 0))
	{
		out("���ݿ����ʧ�� 3\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -4;
	}
	//out("ͨ����¼�Ż�ȡ�̺�:%s\n", plate_num);


	//out("ͨ����¼�Ż�ȡ��Ʊ���ʹ���\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select fplxdm from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd); memset(str_tmp, 0, sizeof(str_tmp));
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, str_tmp);
	result = sqlite3_exec(db, cmd, sql_callback, str_tmp, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(str_tmp) == 0))
	{
		out("���ݿ����ʧ�� 4\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -5;
	}
	invoice->fplxdm = atoi(str_tmp);
	//out("ͨ����¼�Ż�ȡ��Ʊ���ʹ���:%03d\n", invoice->fplxdm);


	//out("ͨ����¼�Ż�ȡ��Ʊ����\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select fpdm from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->fpdm);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->fpdm, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->fpdm) == 0))
	{
		out("���ݿ����ʧ�� 5\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -6;
	}
	//out("ͨ����¼�Ż�ȡ��Ʊ����:%s\n", invoice->fpdm);


	//out("ͨ����¼�Ż�ȡ��Ʊ����\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select fphm from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->fphm);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->fphm, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->fphm) == 0))
	{
		out("���ݿ����ʧ�� 6\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -7;
	}
	//out("ͨ����¼�Ż�ȡ��Ʊ����:%s\n", invoice->fphm);


	//out("ͨ����¼�Ż�ȡ��Ʊʱ��\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select kpsj from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->kpsj);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->kpsj, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->kpsj) == 0))
	{
		out("���ݿ����ʧ�� 7\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -8;
	}
	//out("ͨ����¼�Ż�ȡ��Ʊʱ��:%s\n", invoice->kpsj);


	//out("ͨ����¼�Ż�ȡ��˰�۱�־\n");
	//memset(cmd, 0, sizeof(cmd));
	//sprintf(cmd, "select hsjbz from tb_invoice where idtb_invoice = '%d';", id);
	////out("SQL��䣺%s\n", cmd); memset(str_tmp, 0, sizeof(str_tmp));
	//_lock_set(stream->sql_lock);
	////result = mysql_fun_get_char_res(&mysql_fd, cmd, str_tmp);
	//result = sqlite3_exec(db, cmd, sql_callback, str_tmp, &errinfo);
	//_lock_un(stream->sql_lock);
	//if ((result != SQLITE_OK) || (strlen(str_tmp) == 0))
	//{
	//	out("���ݿ����ʧ�� 8\n");
	//	//mysql_fun_close(&mysql_fd);
	//	sqlite3_close(db);
	//	return -9;
	//}
	//out("ͨ����¼�Ż�ȡ��˰�۱�־:%s\n", str_tmp);


	//out("ͨ����¼�Ż�ȡ�ϼƽ��\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select hjje from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->hjje);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->hjje, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->hjje) == 0))
	{
		out("���ݿ����ʧ�� 9\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -10;
	}
	//out("ͨ����¼�Ż�ȡ�ϼƽ��:%s\n", invoice->hjje);


	//out("ͨ����¼�Ż�ȡ�ϼ�˰��\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select hjse from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->hjse);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->hjse, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->hjse) == 0))
	{
		out("���ݿ����ʧ�� 10\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -11;
	}
	//out("ͨ����¼�Ż�ȡ�ϼ�˰��:%s\n", invoice->hjse);



	//out("ͨ����¼�Ż�ȡ��˰�ϼ�\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select jshj from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->jshj);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->jshj, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->jshj) == 0))
	{
		out("���ݿ����ʧ�� 11\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -12;
	}
	//out("ͨ����¼�Ż�ȡ��˰�ϼ�:%s\n", invoice->jshj);


	//out("ͨ����¼�Ż�ȡУ����\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select jym from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->jym);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->jym, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->jym) == 0))
	{
		out("���ݿ����ʧ�� 12\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -13;
	}
	//out("ͨ����¼�Ż�ȡУ����:%s\n", invoice->jym);


	//out("ͨ����¼�Ż�ȡ����\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select mw from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, (char *)invoice->mwq);
	result = sqlite3_exec(db, cmd, sql_callback, (char *)invoice->mwq, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen((char *)invoice->mwq) == 0))
	{
		out("���ݿ����ʧ�� 13\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -14;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select fpdzsyh from tb_invoice where idtb_invoice = '%d';", id);
	//out("SQL��䣺%s\n", cmd); memset(str_tmp, 0, sizeof(str_tmp));
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, str_tmp);
	result = sqlite3_exec(db, cmd, sql_callback, str_tmp, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(str_tmp) == 0))
	{
		out("���ݿ����ʧ�� 14\n");
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -15;
	}
	invoice->fpdzsyh = atoi(str_tmp);
	//out("ͨ����¼�Ż�ȡ����:%s\n", invoice->mwq);
	//mysql_fun_close(&mysql_fd);
	sqlite3_close(db);
	return id;
}


static int invoice_fpqqlsh_get_exsit(struct _app *stream, char *kp_topic, char *plate_num, struct _make_invoice_result *invoice)
{
	int result;
	char cmd[2048] = { 0 };
	//char line_s[10] = { 0 };
	//int line;
	char id_s[10] = { 0 };
	int id;
	sqlite3 *db = NULL;
	char *errinfo;

	_lock_set(stream->sql_lock);
	//result = mysql_re_connect_e(stream, &mysql_fd);
	result = sqlite3_open("/mnt/masstore/mbdb.db", &db);
	_lock_un(stream->sql_lock);
	if (result <0)
	{
		out("���ݿ�����ʧ��\n");
		return -1;
	}

	memset(cmd, 0, sizeof(cmd));
	//sprintf(cmd, "select idtb_invoice from tb_invoice where fpqqlsh = 'fpqqlsh0000000000000000000000000000000000000000003';");
	sprintf(cmd, "select idtb_invoice from tb_invoice where fpqqlsh = '%s';", invoice->fpqqlsh);
	out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, id_s);
	result = sqlite3_exec(db, cmd, sql_callback, id_s, &errinfo);
	_lock_un(stream->sql_lock);
	out("sqlite3_exec result = %d\n", result);
	if ((result != SQLITE_OK) || (strlen(id_s) == 0))
	{
		out("���ݿ����ʧ��,�ü�¼������,result = %d\n",result);
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -2;
	}
	id = atoi(id_s);
	out("��ȡ���ĸ÷�Ʊ��ˮ��:%s,���ڵļ�¼��Ϊ%d\n", invoice->fpqqlsh, id);
	//mysql_fun_close(&mysql_fd);
	sqlite3_close(db);
	return id;
}

static int invoice_data_to_mysql(struct _app *stream, struct mqtt_parm *parm, char *plate_num,struct _make_invoice_result *invoice)
{
	char cmd[102400] = {0};
	//char line_s[10] = {0};
	char total_s[10] = {0};
	char end_s[10] = {0};
	//int line;
	int total;
	int end;
	int result;
	//MYSQL	mysql_fd;
	sqlite3 *db = NULL;
	char *errinfo;
	_lock_set(stream->sql_lock);
	//result = mysql_re_connect_e(stream, &mysql_fd);
	result = sqlite3_open("/mnt/masstore/mbdb.db", &db);
	_lock_un(stream->sql_lock);
	if (result <0)
	{
		out("���ݿ�����ʧ��\n");
		return -1;
	}
	//out("����������ݿ⺯��\n");
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select total from tb_invoice_mg where idtb_invoice_mg = 1;");
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, total_s);
	result = sqlite3_exec(db, cmd, sql_callback, total_s, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(total_s) == 0))
	{
		out("���ݿ����ʧ��1,result = %d\n",result);
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -2;
	}
	total = atoi(total_s);
	//out("��ȡ����ѭ����¼����Ϊ%d\n", total);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "select end from tb_invoice_mg where idtb_invoice_mg = 1;");
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_get_char_res(&mysql_fd, cmd, end_s);
	result = sqlite3_exec(db, cmd, sql_callback, end_s, &errinfo);
	_lock_un(stream->sql_lock);
	if ((result != SQLITE_OK) || (strlen(end_s) == 0))
	{
		out("���ݿ����ʧ��2,result = %d\n", result);
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -3;
	}
	end = atoi(end_s);
	//out("��ȡ��������¼Ϊ%d\n", end);
	if (end > total)
		end = 1;//��total����ѭ����¼
	//out("��¼���ڸ�������\n");
	sprintf(cmd,\
	"UPDATE tb_invoice SET fpqqlsh = '%s',kp_topic = '%s',plate_num = '%s',fplxdm = '%03d',fpdm = '%s',fphm = '%s',kpsj = '%s',hjje = '%s',hjse = '%s',jshj = '%s',jym = '%s',mw = '%s',fpdzsyh = '%d' where idtb_invoice = '%d'", \
	invoice->fpqqlsh, parm->source_topic, plate_num, invoice->fplxdm, invoice->fpdm, invoice->fphm, invoice->kpsj, invoice->hjje, invoice->hjse, invoice->jshj, invoice->jym, invoice->mwq,  invoice->fpdzsyh, end);//�޸����
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_send(&mysql_fd, cmd);
	result = sqlite3_exec(db, cmd, 0, 0, &errinfo);
	_lock_un(stream->sql_lock);
	if (result != SQLITE_OK)
	{
		out("���ݿ����ʧ��3,result = %d\n", result);
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -4;
	}
	//out("���ݲ������\n");
	sprintf(cmd, "UPDATE tb_invoice_mg SET end = '%d' where idtb_invoice_mg = '1';", end+1);//�޸����
	//out("SQL��䣺%s\n", cmd);
	_lock_set(stream->sql_lock);
	//result = mysql_fun_send(&mysql_fd, cmd);
	result = sqlite3_exec(db, cmd, 0, 0, &errinfo);
	_lock_un(stream->sql_lock);
	if (result != SQLITE_OK)
	{
		out("���ݿ����ʧ��4,result = %d\n", result);
		//mysql_fun_close(&mysql_fd);
		sqlite3_close(db);
		return -5;
	}
	//out("���ݹ������������\n");
	//mysql_fun_close(&mysql_fd);
	sqlite3_close(db);
	return 0;
}
#endif


// ������Ʊ����json
static int analysis_invoice_sfsylsh(char *inbuf, struct _make_invoice_result *invoice)
{
	//int result;
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "SFSYLSH");
	if (item != NULL)
	{
		if (item->valuestring != NULL)
			invoice->sfsylsh = atoi(item->valuestring);
		else
			invoice->sfsylsh = 0;
	}
	else
	{
		invoice->sfsylsh = 0;
	}

	if (invoice->sfsylsh == 1)
	{
		item = cJSON_GetObjectItem(root, "FPQQLSH");
		if (item != NULL)
		{
			if (item->valuestring != NULL)
				memcpy(invoice->fpqqlsh, item->valuestring, strlen(item->valuestring));
			else
			{
				invoice->sfsylsh = 0;
				cJSON_Delete(root);
				return -1;
			}
		}
		else
		{
			invoice->sfsylsh = 0;
			cJSON_Delete(root);
			return -1;
		}
	}
	cJSON_Delete(root);
	return 0;
}

//����jsonͷ��Ϣ
static int analysis_json_head(char *inbuf, struct mqtt_parm *parm)
{
	int result;
	cJSON *item;
	cJSON *root = cJSON_Parse(inbuf);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object = cJSON_GetObjectItem(root, "head");
	if (object == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	//out("��ȡЭ��汾\n");
	item = cJSON_GetObjectItem(object, "protocol");
	if (get_json_value_can_not_null(item, parm->protocol, 1, 50) < 0)
	{
		out("Э��汾��ȡʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	result = strcmp(parm->protocol, DF_MQTT_SYNC_PROTOCOL_VER);
	if (result != 0)
	{
		out("Э��汾��һ��,protocol = %s\n", parm->protocol);
		cJSON_Delete(root);
		return -1;
	}

	///////////////////////////////////////////////////////////////////
	item = cJSON_GetObjectItem(object, "code_type");
	if (get_json_value_can_not_null(item, parm->code_type, 1, 10) < 0)
	{
		out("��ȡ��������ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	if ((strcmp(parm->code_type, DF_MQTT_SYNC_CODE_REQUEST) != 0) && (strcmp(parm->code_type, DF_MQTT_SYNC_CODE_RESPONSE) != 0))
	{
		out("��Ϣ���ʹ���,parm->code_type = %s\n", parm->code_type);
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(object, "cmd");
	if (get_json_value_can_not_null(item, parm->cmd, 1, 50) < 0)
	{
		out("��ȡ������ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}
	//out("��ȡ��������Ϊ%s\n", parm->cmd);

	//out(��ȡԴ��Ϣ����\n);
	item = cJSON_GetObjectItem(object, "source_topic");
	if (get_json_value_can_not_null(item, parm->source_topic, 1, 50) < 0)
	{
		out("��ȡԴ��Ϣ����ʧ��\n");
		cJSON_Delete(root);
		return -1;
	}


	//out(��ȡ��������\n);
	item = cJSON_GetObjectItem(object, "random");
	if (get_json_value_can_not_null(item, parm->random, 1, 128) < 0)
	{
		out("��ȡ��������\n");
		cJSON_Delete(root);
		return -1;
	}


	//out(��ȡ�������\n);
	item = cJSON_GetObjectItem(object, "result");
	if (get_json_value_can_not_null(item, parm->result, 1, 1) < 0)
	{
		//out("��ȡ���������ʧ��\n");
	}

	cJSON_Delete(root);
	return 0;
}
//����Ӧ��
static int mqtt_response_errno(int errnum, struct mqtt_parm *parm,char *errinfo,int qos)
{
	struct _app *stream;
	int i;
	char msg[200] = { 0 };
	char err[10] = { 0 };
	int result;
	stream = fd_stream;
	//out("errnum = %d\n", errnum);
	for (i = 0; i < sizeof(err_no_infos) / sizeof(err_no_infos[0]); i++)
	{
		if (errnum == err_no_infos[i].err)
		{
			memset(msg, 0, sizeof(msg));
			memcpy(msg, err_no_infos[i].errinfo, sizeof(msg));

			memset(err, 0, sizeof(err));
			sprintf(err, "%d", err_no_infos[i].errnum);
			break;
		}
	}
	if (i == sizeof(err_no_infos) / sizeof(err_no_infos[0]))
	{
		out("δ�ҵ������������������Ϊδ֪����\n");
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
	out("���ش������ %s\n", g_buf);

	//out("MQTT��������,���ݳ���%d\n",strlen(g_buf));
	//_lock_set(stream->lock);
	result = _mqtt_client_message_pub(parm->source_topic, g_buf,3,qos);
	//_lock_un(stream->lock);
	free(g_buf);
	cJSON_Delete(json);
	return result;
}

static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num,int qos)
{
	int i;
	int result;

	for (i = 0; i < try_num; i++)
	{
		result = _mqtt_client_pub(topic, sbuf,qos);
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
		out("����%d�ξ�ʧ��\n",try_num);
		return -1;
	}
	return 0;
}

//���ҿ�����Ϣ���к�
static int find_available_queue_mem(struct _app     *stream)
{
	int mem_i;
	//out("���ҿ�����Ϣ����\n");
	if (message_callback.insert_num == MAX_MESSAGE_QUEUE_NUM)
		message_callback.insert_num = 0;
	mem_i = message_callback.insert_num;
	if (message_callback.message_line[mem_i].state == 1)
	{
		if (message_callback.message_line[mem_i].inuse == 1)
		{
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�������ʱ,��ǰҪ����Ķ������Ϊ�ϴ�ѭ��δ����������,�жϺ������޿��ö���,��ǰ�����ڴ�ţ�%d\r\n", message_callback.insert_num);
			mem_i++;
			for (; mem_i < MAX_MESSAGE_QUEUE_NUM; mem_i++)
			{
				if (message_callback.message_line[mem_i].state == 0)
				{					
					message_callback.insert_num = mem_i;
					//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�������ʱ,�ҵ������ڴ�ţ�%d\r\n", message_callback.insert_num);
					return mem_i;
				}
			}
		}
		return -1;
	}
	message_callback.insert_num += 1;
	return mem_i;
}

//����Ҫ��������Ϣ���к�
static int find_deal_queue_mem(struct _app     *stream)
{
	int wait_deal_num=0;
	int i=0;
	int mem_i;
	//out("���ҿ�����Ϣ����\n");
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
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д������ڴ������������,��ǰ�������Ϊ��,�������������������Ҫ����,������ǰ�������\r\n");
			message_callback.deal_num += 1;
		}
		return -1;
	}
	if (message_callback.message_line[mem_i].inuse != 0)
	{
		//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д������ڴ������������,�ϸ�ѭ�����ڴ���,������ǰ�������\r\n");
		message_callback.deal_num += 1;
		return -1;
	}
	message_callback.message_line[mem_i].inuse = 1;
	return mem_i;
}
//��������������
static void  deal_with_cmd(struct _app *stream, int mem_i, void *fucntion)
{
	unsigned int time_now;
	time_now = get_time_sec();
	if ((time_now - message_callback.message_line[mem_i].message_time) > 120)
	{
		logout(INFO, "SYSTEM", "message_queue", "������Ϣ����ʱ���ֵ�ǰ��Ϣ�ѳ�ʱ,���ٴ�������Ϣ\r\n");
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
	if (_queue_add("0", fucntion, &message_callback.message_line[mem_i], 0)<0)
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

	//logout(INFO, "SYSTEM", "message_queue", "��������������ɣ���ˮ�ţ�%s\r\n", message_callback.message_line[mem_i].parm.random);
	message_callback.err_count = 0;
	message_callback.deal_num += 1;
	message_callback.task_num += 1;
	return ;
	//out("������Ϣ���гɹ�\n");
}

//MQTT�ͻ��˶����쳣�����ص�
static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum)
{
	struct _app *stream;
	stream = fd_stream;
	logout(INFO, "SYSTEM", "mqtt_err", "���붩���쳣�����ص�,�������Ϊ%d\r\n", errnum);

	switch (errnum)
	{
		case 1:
			out("֤�鲻����");
			break;
		case 8:
			out("֤����û����������\n");
			break;
		case 14:
			out("����������ʧ��\n");
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
	lcd_print_info.mqtt_state = 0;
	ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	sleep(10);
	out("���ӷ������쳣,�û�����������֤�����\n");

	sleep(stream->connect_time);
	if ((stream->connect_time > DEAFULT_CONNECT_TIME*20)||(stream->connect_time < DEAFULT_CONNECT_TIME))//����20���ӣ�������ʱ���ٴθ�Ϊ60��һ��
	{
		stream->connect_time = DEAFULT_CONNECT_TIME;
	}
	stream->connect_time += 10;
	lcd_print_info.mqtt_state = 1;
	ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
	return;

}

void task_deal_message_queue(void *arg, int o_timer)
{
	struct _app *stream;
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

	//out("===============================================================\n���뵽��Ϣ�����߳�\n\n\n\n\n\n");
	if (message_callback.task_num > MAX_DEAL_MESSAGE_TASK)
	{
		//out("�̶߳��г���%d,�ȴ��Ժ󴴽�\n", MAX_DEAL_MESSAGE_TASK);
		return;
	}
	_lock_set(stream->mqtt_lock);
	mem_i = find_deal_queue_mem(stream);
	if (mem_i < 0)
	{
		//out("����Ҫ��������\n");
		_lock_un(stream->mqtt_lock);
		return;
	}
	_lock_un(stream->mqtt_lock);
	//out("��%d���ڴ�������Ϣ������,������Ϊ%s,��ˮ�ţ�%s\n", mem_i, message_callback.message_line[mem_i].parm.cmd, message_callback.message_line[mem_i].parm.random);
	//out("cmd %s\n", parm.cmd);
	//out("�ж������ֲ�ִ����Ӧ����\n");
	if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXDLXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ��Ϣ������Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxdlxx);
	}
	else if(strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXLSCLJG) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxlscljg);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXZDQBXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_cxzdqbxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_AYCXYKFPZS) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_aycxykfpzs);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_AYCXYKFPXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_aycxykfpxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_AFPDMHMCXFP) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_afpdmhmcxfp);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_AFPDMHMCXFPBS) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_afpdmhmcxfpbs);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_TZYKFPCX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_tzykfpcx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_DQFPDMHM) == 0)
	{
		//logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_fpzl_fpdm_fphm);
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
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_TIME) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_time);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_FPKJ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_fpkj);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_FPZF) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_fpzf);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_TAX_INFO) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_tax_info);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_INVOICE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_invoice_details);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_PLATE_CBKQ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����忨\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_plate_cbqk);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_SET_REPORT_TIME) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_set_report_time);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHECK_CERT_PASSWD) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_check_cert_passwd);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_LSH_INV_DATA) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_fpqqlsh_inv_data);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_AFPDMHMSCFP) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_afpdmhmscfp);	
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,���������Ϣ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hqhzxxbbh);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHECK_LZFPDMHM) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_jylzfpdmhm);	
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_INV_REDNUM) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hzxxbhqfp);
	}
	//else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_NET_INV_COIL) == 0)//�ӿ�ͣ��
	//{
	//	logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
	//	deal_with_cmd(stream, mem_i, analysis_request_zxfply);
	//}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_UPDATE_INV_MSERVER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_tzzdscmyfp);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHANGE_CERT_PASSWD) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_xgmrzsmmyz);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHANCEL_TZDBH) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����������Ϣ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hzxxbhcx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_QUERY_TZDBH_DATE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_asjdcxhzxxb);
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
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_QUERY_INV_COIL) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_asjdlyfpcx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_DOWN_INV_COIL) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_asjdlyfpxz);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CHECK_SERVER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��������������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_jcfwqlj);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_MANAGE_PORT_POWER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_adkhgldkdy);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_MANAGE_TER_POWER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,�����ն�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_glzddy);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CLEAR_CERT_ERR_ROM) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_asphqczshc);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_ZYFP_TZDBH_ALL) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_afpdmhmhqhzxxb);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_AUTH_KEY) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hqkeysqxx);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CONECT_PUBSERVER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_sqjrggfwpt);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_DISCONECT_PUBSERVER) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qxjrggfwpt);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_STATISTICS) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hqhzxxsj);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_GET_HISTORY_COILS) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hqlsgpxx);
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
	else if ((strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_TCGXRZFWSJ) == 0) && strcmp(message_callback.message_line[mem_i].parm.code_type, DF_MQTT_SYNC_CODE_RESPONSE)!=0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_tcgxrzfwsj);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_KZLXFPSC) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_kzlxfpsc);
	}	
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_TZZDDKMQTT) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_tzzddkmqtt);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDDLXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���¼\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qddengl);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDDLLB) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���û���Ϣ��ȡ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdyongyxxhq);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDQYLB) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л��б���ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdqiyeqiehuanlbcx);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDQYQH) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdqiyeqiehuan);
	}


	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDFPCX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ�б���ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfpcx);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDFPCXXQ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ�����ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfpcxxq);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDFPXZ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfpcxxz);
	}


	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_FPKJRPA) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ����ɨ������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfpkjrpasl);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_FPKJRPAZT) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ��������֤�ӿ�\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfpkjrpasl_zt);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDSXED) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�����Ŷ�Ȳ�ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdsxedcx);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QDLZFPKJ) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�����ַ�Ʊ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdlzfpkj);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_HZQRXXSZZHQUERY) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ��ɿ���Ʊ����Ʊ��ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdhzqrxxszzhquery);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_HZQRXXSAVE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���Ʊȷ�ϵ�����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hzqrxxsave);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_HZQRXXQUERY) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���Ʊȷ�ϵ��б���ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hzqrxxquery);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_HZQRXXQUERYDETAIL) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���Ʊȷ�ϵ������ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_hzqrxxquerydetail);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_HCSTAQUERY) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ�������ѯ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdhcstaquery);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_FJXXPEIZHI) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�總����Ϣ����\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdfjxxpeizhi);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_CJMBPEIZHI) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ�糡��ģ������\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qdcjmbpeizhi);
	}	

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QD_GET_COOKIES) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ���ȡcookies\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qd_get_cookie);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QD_NSRJCXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ����˰�˻�����Ϣ��ȡ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qd_get_nsrjcxx);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_QD_NSRFXXX) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,ȫ����˰�˷�����Ϣ��ȡ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_qd_get_nsrfxxx);
	}

	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_GET_CLIENTHELLO_BY_PLATE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡclienthello\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_get_clienthello);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_GET_CLIENTAUTH_BY_PLATE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡclientauth\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_get_clientauth);
	}
	else if (strcmp(message_callback.message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUSET_READCERT_BY_PLATE) == 0)
	{
		logout(INFO, "mqtt", message_callback.message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ȡ֤����Ϣ\r\n", message_callback.message_line[mem_i].parm.random);
		deal_with_cmd(stream, mem_i, analysis_request_read_cert);
	}



#ifdef NEW_USB_SHARE
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
#endif

	else
	{
		//out("�����ֲ�֧��,�ڴ����Ϊ%d\n",mem_i);
		deal_with_cmd(stream, mem_i, analysis_request_cmd_not_support);
	}
	
	
	return;
}

//��Ʊ���ݻص����ͺ���
static int mqtt_pub_hook(unsigned char *s_data, int total_num, int now_count, void *arg, char *plate_num)
{
	struct _message_line *message_line;
	struct _app     *stream;
	stream = fd_stream;
	message_line = arg;
	int i;
	out("��ƱJSON totalnum = %d,nowcount = %d\n", total_num, now_count);
	char str_tmp[10] = { 0 };
	int now_num;
	int invlen;
	long index = 0;

	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON *data_array;		 //���ʹ��
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());

	if (total_num == 0)
	{
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(data_layer, "total_num", "0");
		cJSON_AddStringToObject(data_layer, "now_num", "0");
		cJSON_AddStringToObject(data_layer, "plate_num", plate_num);
		cJSON_AddStringToObject(data_layer, "invoice_data", "");
	}
	else
	{
		for (i = 0; i < now_count; i++)
		{
			cJSON *data_layer;
			char *inv_data = NULL;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", total_num);
			cJSON_AddStringToObject(data_layer, "total_num", str_tmp);

			now_num = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("��ǰ��Ʊ���%d\n",now_num);
			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", now_num);
			cJSON_AddStringToObject(data_layer, "now_num", str_tmp);
			cJSON_AddStringToObject(data_layer, "plate_num", plate_num);
			index += 4;
			invlen = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("��ǰ��Ʊ���ݳ���%d\n", invlen);
			index += 4;
			inv_data = (char *)malloc(invlen + 100);
			memset(inv_data, 0, invlen + 100);
			memcpy(inv_data, s_data + index, invlen);
			index += invlen;
			//out("��ǰ��Ʊ����%s\n", inv_data);
			cJSON_AddStringToObject(data_layer, "invoice_data", inv_data);

			free(inv_data);

			//out("�鷢ƱӦ������\n");
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	//out("mqtt ��Ʊ�������� = %s\n", g_buf);
	cJSON_Delete(json);

	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	out("MQTT�ص������������\n");
	free(g_buf);

	return 0;
}

#ifdef NEW_USB_SHARE



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
		out("���������δ�������ô�������\n");
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

	//cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	//cJSON *dir3;
	//cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	//memset(err, 0, sizeof(err));
	//sprintf(err, "%d", errnum);
	//cJSON_AddStringToObject(dir3, "err", err);
	//cJSON_AddStringToObject(dir3, "msg", msg);

	char *g_buf;
	g_buf = cJSON_Print(root);
	//printf("g_buf = %s\n", g_buf);

	out("MQTT��������err gw:%s\n", g_buf);


	_mqtt_client_message_pub(parm->source_topic, g_buf, 3, DF_MQ_QOS0);



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

	out("MQTT��������gw:%s\n", g_buf);

	_mqtt_client_message_pub(parm->source_topic, g_buf, 3, DF_MQ_QOS0);
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


#endif

//char start_time[50];
//char end_time[50];
//MQTT�ͻ�����Ϣ���ջص�
static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct _app     *stream;
	struct mqtt_parm parm;
	int result;
	int mem_i;
	stream = fd_stream;
	//out("����MQTT�ͻ�����Ϣ���ջص�\n");
	assert(obj);
	if (!message->payloadlen)
	{
		out("û�����ݻ����ݴ���\n");
		return;
	}
	//out("MQTT PUB [%s:%s]\n", message->topic, message->payload);
	//mqtt_len += (strlen(message->payloadlen) + strlen(message->topic));
	//out("MQTT�շ��ֽ���:%lld\n", mqtt_len);
	//out("topic %s \n", message->topic);
	//printf("message %s\n",(char *) message->payload);
	memset(&parm, 0, sizeof(struct mqtt_parm));
	result = analysis_json_head(message->payload, &parm);
	if (result < 0)
	{
		out("����json����ͷʧ��\n");
		return;
	}
#ifdef DF_OLD_MSERVER
	if ((strcmp(parm.cmd, "zdsbspxx") == 0))
	{
		return;
	}
#endif
	//logout(INFO, "MQTT", "mqtt_message", "��ˮ�ţ�%s,���յ���Ϣ���������\r\n", parm.random);
	mem_i = find_available_queue_mem(stream);
	if (mem_i < 0)
	{
		out("��Ϣ��������,�޷�����\n");
		logout(INFO, "MQTT", "mqtt_err", "��Ϣ%s,��Ϣ���������޷��洢,����������\r\n", parm.random);
		return;
	}
	message_callback.message_line[mem_i].message = malloc(strlen(message->payload) + 100);
	if (message_callback.message_line[mem_i].message == NULL)
	{
		out("�ڴ�����ʧ��,Ҫ������ڴ��СΪ%d,������%s\n", strlen(message->payload) + 100, DF_MQTT_SYNC_REPORT_TER_STATUS);
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
	struct _app     *stream;
	//int size;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	//cJSON *arrayItem, *item; //����ʹ��
	//cJSON *data_array;		 //���ʹ��
	//char errinfo[2048] = { 0 };

	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	//out("������%s���ⲿ����������,����Ϊ�ⲿӦ��������,����M����ģ�鴦��\n", message_line->parm.cmd);
	
	
	message_callback_deal_answer(&message_line->parm, message_line->message);

	//out("Mģ�鴦�����\n");
	
	free(message_line->message);	
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
	//out("������ѯ˰��˰��ʱ�����\n");
	return;
}

//������������ѯ������Ϣ
static void analysis_request_cxdlxx(void *arg)
{
	struct _app     *stream;
	int size;
	char random[150] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��

	char errinfo[2048] = {0};
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "random");
	if (get_json_value_can_not_null(item, (char *)random, 0, 130) < 0) 
	{
		sprintf(errinfo, "�����JSON������data�ڵ���random����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ����randomΪ%s\n", random);

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
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	//out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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
	//out("������ѯ˰�̷�Ʊ��Ϣ���\n");
	return;
}

//������������ѯ��ʷ�������
static void analysis_request_cxlscljg(void *arg)
{
	struct _app     *stream;
	int size;
	char random[150] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��

	char errinfo[2048] = { 0 };
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "random");
	if (get_json_value_can_not_null(item, (char *)random, 0, 130) < 0)
	{
		sprintf(errinfo, "�����JSON������data�ڵ���random����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ����randomΪ%s\n", random);

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
#ifndef DF_OLD_SERVER
				cJSON_AddStringToObject(data, "result_data", "");
#else 
				cJSON_AddStringToObject(data, "result_data", message_history.history_line[i].inv_data);
#endif
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,%s,��ѯ��random��%s\r\n", message_line->parm.random, errinfo,random);
		cJSON_Delete(json);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ��ʷ�������,���,��ѯ��random��%s\r\n", message_line->parm.random, random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//out("������ѯ˰�̷�Ʊ��Ϣ���\n");
	return;
}


//������������ѯ�ն�ȫ����Ϣ
static void analysis_request_cxzdqbxx(void *arg)
{
	struct _app     *stream;	
	struct _message_line *message_line;
	int size;
	//int result;
	message_line = arg;
	stream = fd_stream;
	cJSON *data_array;		 //���ʹ��

	char errinfo[2048] = {0};
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն�ȫ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	
	fun_get_ter_all_data_json(data_layer);
	
	
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
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	asc_compress_base64((const unsigned char *)data_buf, data_len, (unsigned char *)compress, &compress_len);
	cJSON_AddStringToObject(data, "zdqbxx", compress);
	free(data_buf);
	free(compress);
	//out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);


	str_replace(g_buf, "\\\\", "\\");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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

//�������������²�ѯ�ѿ���Ʊ����
static void analysis_request_aycxykfpzs(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char plate_num[13] = { 0 };
	char month[20] = { 0 };
	char total_num[20] = {0};
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON������data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	//out("Ҫ���ѯȫ��USB���ϵ�˰��ʱ��\n");

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

	//out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "month");
		if (get_json_value_can_not_null(item, (char *)month, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���month����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,%s\r\n", message_line->parm.random, errinfo);
			sprintf(errinfo, "��ƱJSON����data���·�ʧ��");
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ�����·�Ϊ%s\n", month);
		unsigned int num = 0;
		unsigned long size = 0;
		result = fun_get_palte_month_inv_num(plate_num, month,&num,&size,errinfo);
		if (result < 0)
		{
			//sprintf(errinfo, "��ѯ%s�·�Ʊ����ʧ��",month);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		sprintf(total_num, "%d", num);
		
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(data_layer, "total_num", (const char *)total_num);
	

	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	//out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return ;
}
//�������������²�ѯ�ѿ���Ʊ��Ϣ
static void analysis_request_aycxykfpxx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = {0};
	struct _message_line *message_line;
	unsigned char month[20] = {0};
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	int sfxqqd;
	unsigned char sfxqqd_s[20] = {0};
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
	{
		sprintf(errinfo, "�����JSON������data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	item = cJSON_GetObjectItem(arrayItem, "month");
	if (get_json_value_can_not_null(item, (char *)month, 0, 20) < 0) 
	{
		sprintf(errinfo, "�����JSON������data�ڵ���month����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ�����·�Ϊ%s\n", month);
	

	item = cJSON_GetObjectItem(arrayItem, "sfxqqd");
	if (get_json_value_can_null(item, (char *)sfxqqd_s, 0, 10) < 0) {
		sfxqqd = 1;
	}
	else
	{
		sfxqqd = atoi((const char *)sfxqqd_s);
	}
	out("��ȡ�Ƿ������嵥Ϊ%d\n", sfxqqd);

	_lock_set(stream->inv_read_lock);
	//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_INV_DETAILS, plate_num, month, mqtt_pub_hook, message_line);
	result = fun_get_invoice_detail_mqtt(plate_num, month, mqtt_pub_hook, message_line, errinfo, 1, sfxqqd);
	_lock_un(stream->inv_read_lock);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		out("��Ʊ��ѯ����ʧ��\n");
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	cJSON_Delete(root);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���²�ѯ�ѿ���Ʊ��Ϣ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
//��������������Ʊ��������ѯ��Ʊ
static void analysis_request_afpdmhmcxfp(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int dzsyh;
	int result = -1;
	char plate_num[13] = {0};
	char fpdm[20] = {0};
	char fphm[20] = {0};
	char *inv_data=NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	out("Ҫ���ѯȫ��USB���ϵ�˰��ʱ��\n");
	
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

	out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON������data�нڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "fpdm");
		if (get_json_value_can_not_null(item, (char *)fpdm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���fpdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fpdm);

		item = cJSON_GetObjectItem(arrayItem, "fphm");
		if (get_json_value_can_not_null(item, (char *)fphm, 0, 20) < 0)
		{
			sprintf(errinfo, "�����JSON������data�ڵ���fphm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fphm);

		char tmp_dzsyh[10] = { 0 };
		item = cJSON_GetObjectItem(arrayItem, "dzsyh");
		if (get_json_value_can_not_null(item, (char *)tmp_dzsyh, 0, 20) < 0) {
			sprintf(errinfo, "��ƱJSON����data�е�ַ������ʧ��");
			dzsyh = 0;
		}
		else
		{
			dzsyh = atoi(tmp_dzsyh);
		}		
		out("��ȡ���ĵ�ַ������%d\n", dzsyh);

		result = fun_get_inv_from_fpdm_fphm(plate_num, fpdm, fphm, &inv_data, dzsyh, errinfo);
		if (result < 0)
		{
			//sprintf(errinfo, "ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ", fpdm, fphm);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			out("ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ\n", fpdm, fphm);
			if (inv_data)
			{
				out("ָ���ͷ�inv_data\n");
				free(inv_data);
				inv_data = NULL;
			}
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		if (inv_data)
		{
			out("�鷢Ʊ���ݰ�\n");
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "invoice_data", (const char *)inv_data);
			out("ָ���ͷ�inv_data\n");
			free(inv_data);
			inv_data = NULL;
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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


//��������������Ʊ��������ѯ��Ʊ--����
static void analysis_request_afpdmhmcxfpbs(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char plate_num[13] = { 0 };
	char fpdm[20] = { 0 };
	char fphm[20] = { 0 };
	char *inv_data = NULL;
	char *kp_bin = NULL;
	char *kp_str = NULL;
	int dzsyh;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON������data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);



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

	out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON������data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "fpdm");
		if (get_json_value_can_not_null(item, (char *)fpdm, 0, 20) < 0)
		{
			sprintf(errinfo, "�����JSON������data�ڵ���fpdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fpdm);

		item = cJSON_GetObjectItem(arrayItem, "fphm");
		if (get_json_value_can_not_null(item, (char *)fphm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���fphm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fphm);

		char tmp_dzsyh[10] = {0};
		item = cJSON_GetObjectItem(arrayItem, "dzsyh");
		if (get_json_value_can_not_null(item, (char *)tmp_dzsyh, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON������data�ڵ���dzsyh����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		dzsyh = atoi(tmp_dzsyh);
		out("��ȡ���ĵ�ַ������%d\n", dzsyh);

		result = fun_get_inv_from_fpdm_fphm_get_bin_str(plate_num, fpdm, fphm, dzsyh, &inv_data, &kp_bin, &kp_str, errinfo);
		if (result < 0)
		{
			//sprintf(errinfo, "ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ", fpdm, fphm);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			out("ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ\n", fpdm, fphm);
			if (inv_data)
			{
				out("ָ���ͷ�inv_data\n");
				free(inv_data);
				inv_data = NULL;
			}
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		if (inv_data)
		{
			out("�鷢Ʊ���ݰ�\n");
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			if (inv_data != NULL)
			{
				cJSON_AddStringToObject(data_layer, "invoice_data", (const char *)inv_data);
				free(inv_data);
				inv_data = NULL;
			}

			if (kp_bin != NULL)
			{
				cJSON_AddStringToObject(data_layer, "kp_bin", (const char *)kp_bin);
				free(kp_bin);
				kp_bin = NULL;
			}

			if (kp_str != NULL)
			{
				cJSON_AddStringToObject(data_layer, "kp_str", (const char *)kp_str);
				free(kp_str);
				kp_str = NULL;
			}

			out("ָ���ͷ�inv_data\n");
			
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ѯ��Ʊ--����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//����������ֹͣ�ѿ���Ʊ��ѯ
static void analysis_request_tzykfpcx(void *arg)
{
	struct _app     *stream;
	int size;
	//int i;
	//int result = -1;
	char plate_num[13] = { 0 };
	//char *inv_data = NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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

	out("��Ҫ���˰�̱�����ڲ�ѯ�ķ�Ʊ\n");

	//int time_now;
	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	fun_stop_invoice_query(plate_num, errinfo);


	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ֹͣ�ѿ���Ʊ��ѯ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
//���ݷ�Ʊ�����ȡ��ǰ��Ʊ�������
static void analysis_request_fpzl_fpdm_fphm(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	//int result = -1;
	unsigned char plate_num[13] = { 0 };
	//unsigned char cert_passwd[20];
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048]={0};

	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);



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
	//������ݽ���
	if (size == 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ��ǰ��Ʊ�������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

			item = cJSON_GetObjectItem(arrayItem, "fplxdm");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			int inv_type = 0;
			if (item->valuestring != NULL)
			{
				inv_type = atoi(item->valuestring);
			}

			char fpdm[20] = { 0 };
			char fphm[20] = { 0 };

			int result = fun_check_now_fpdm_fphm(plate_num, inv_type, fpdm, fphm, errinfo);
			if (result < 0)
			{
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
			char tmp_sc[20] = { 0 };
			sprintf(tmp_sc, "%03d", inv_type);
			cJSON_AddStringToObject(data_layer, "fplxdm", (const char *)tmp_sc);
			cJSON_AddStringToObject(data_layer, "fpdm", (const char *)fpdm);
			cJSON_AddStringToObject(data_layer, "fphm", (const char *)fphm);
			//memset(str_tmp, 0, sizeof(str_tmp));
			//sprintf(str_tmp, "%d", usb_sum);
			//cJSON_AddStringToObject(data_layer, "usb_port_num", str_tmp);
			//cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());



		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ�����ȡ��ǰ��Ʊ�������,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
	struct _app     *stream;
	struct _message_line *message_line;

	//int result;
	message_line = arg;
	stream = fd_stream;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն˻�����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	cJSON_ReplaceItemInObject(data, "usb_port_manage", cJSON_CreateString(""));

	

	//out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);

	str_replace(g_buf, "\\\\", "\\");

	//out("�ն˻�����Ϣ���ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);

	//out("�������ж˿��ٴ��ϱ�\n");
	fun_deal_all_port_report();


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
	struct _app     *stream;
	struct _message_line *message_line;
	//char tmp[50];
	char ter_id[20] = {0};
	char notice_type[20] = { 0 };
	//int result;
	message_line = arg;
	stream = fd_stream;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	cJSON *item;
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	
	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ter_id����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(ter_id, 0, sizeof(ter_id));
		memcpy(ter_id, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ���Ļ������Ϊ%s\n", ter_id);


	item = cJSON_GetObjectItem(arrayItem, "notice_type");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���notice_type����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(notice_type, 0, sizeof(notice_type));
		memcpy(notice_type, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����֪ͨ����Ϊ%s\n", notice_type);

	
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	
	if (strcmp(ter_info.ter_id, ter_id) != 0)
	{
		sprintf(errinfo, "������Ų�һ�£��ն˻����ܾ�");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
#ifdef RELEASE_SO
	if (strcmp(notice_type, "auth") == 0)
		;//write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ������Ȩ�ļ�", strlen("������Ҫ��ϵͳ������Ȩ�ļ�"));
	else if (strcmp(notice_type, "update") == 0)
		write_file("/etc/need_update_auth.log", "������Ҫ��ϵͳ���������ļ�", strlen("������Ҫ��ϵͳ���������ļ�"));
	else
	{
		sprintf(errinfo, "֪ͨ���ʹ���,�޷�֧��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˻�ȡ��������Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
#endif

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


	//out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);
	out("�ն�Ӧ�����ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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
	struct _app     *stream;
	struct _message_line *message_line;
	//char tmp[50];
	char ter_id[20] = { 0 };
	char auth_data[10*1024] = { 0 };
	char auth_md5[200] = { 0 };
	//int result;
	message_line = arg;
	stream = fd_stream;
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	cJSON *item;
	cJSON *arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ter_id����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(ter_id, 0, sizeof(ter_id));
		memcpy(ter_id, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ���Ļ������Ϊ%s\n", ter_id);


	item = cJSON_GetObjectItem(arrayItem, "auth_data");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���auth_data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(auth_data, 0, sizeof(auth_data));
		memcpy(auth_data, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ���ļ�����Ȩ�ļ���ϢΪ%s\n", auth_data);


	item = cJSON_GetObjectItem(arrayItem, "auth_md5");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���auth_md5����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(auth_md5, 0, sizeof(auth_md5));
		memcpy(auth_md5, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ���ļ�����Ȩ�ļ���Ϣauth_md5Ϊ:%s\n", auth_md5);


	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);


	if (strcmp(ter_info.ter_id, ter_id) != 0)
	{
		sprintf(errinfo, "������Ų�һ�£��ն˻����ܾ�");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char auth_file[10 * 1024] = {0};

	int auth_file_len = sizeof(auth_file);
	uncompress_asc_base64((const uint8 *)auth_data, strlen((const char *)auth_data), (uint8 *)auth_file, &auth_file_len);


	char auth_file_md5[200] = { 0 };
	md5_value(auth_file, strlen(auth_file), auth_file_md5);


	out("��Ȩ�����ļ�md5ֵΪ:%s\n", auth_file_md5);

	if (strcmp(auth_file_md5, auth_md5) != 0)
	{
		out("�ļ�MD5ֵ�봫���json�ֶ�MD5ֵ��һ��\n");
		sprintf(errinfo, "MD5ֵУ���쳣ֹͣ��Ȩ����");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ȩ�ն˵Ķ˿�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	if (access("/etc/auth_cert.json", 0) == 0)
	{
		system("rm /etc/auth_cert.json");
	}
	write_file("/etc/auth_cert.json", auth_file, strlen(auth_file));

	fun_upload_auth_cert_file();


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


	//out("ѹ��ǰ���ݳ���Ϊ%d,ѹ�������ݳ���Ϊ%d,ѹ����Ϊ%4.2f\n", data_len, compress_len, (float)compress_len / (float)data_len);

	char *g_buf;
	g_buf = cJSON_Print(json);
	out("�ն�Ӧ�����ݣ�%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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
//������������ȡ˰��ʱ��
static void analysis_request_plate_time(void *arg)
{
	struct _app     *stream;
	int size;
	int port_count;
	int i;
	int usb_sum = 0;
	int result = -1;
	unsigned char plate_num[13] = {0};
	struct _message_line *message_line;
	uint8 ter_id[13] = { 0 };
	char time[20] = { 0 };
	message_line = arg;
	stream = fd_stream;
	struct _usb_port_infor port_info;
	struct _switch_dev_id   id;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	out("Ҫ���ѯȫ��USB���ϵ�˰��ʱ��\n");
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);


	//out("����������\n");       
	memset(&id, 0, sizeof(id));
	memset(ter_id, 0, sizeof(ter_id));
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, ter_id, 6);
	if (result < 0)
	{
		out("get_sev_pro����������result = %d\n", result);
	}
	out("�ն˻������Ϊ:%s USB�˿�����Ϊ��%d\n", ter_id, usb_sum);
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
	//������ݽ���
	if (size == 0)
	{
		for (port_count = 0; port_count < usb_sum; port_count++)
		{
			memset(&port_info, 0, sizeof(struct _usb_port_infor));
			result = fun_get_plate_usb_info(port_count + 1, &port_info);
			if (result<0)
			{
				out("get_usb_dev_info ��ȡUSB�˿���Ϣʧ��result=%d\n", result);
				continue;
			}
			if (((port_info.vid == 0x101d) && (port_info.pid == 0x0003)) || \
				((port_info.vid == 0x1432) && (port_info.pid == 0x07dc)) || \
				((port_info.vid == 0x0680) && (port_info.pid == 0x1901)))
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, port_info.sn, 12);
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
				out("�����̺�:%s��ȡ˰��ʱ��\n", plate_num);
				memset(time, 0, sizeof(time));
				//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_PLATE_TIME, plate_num, time);
				result = fun_get_plate_now_time(plate_num, time);
				if (result < 0)
				{
					out("ͨ���̺Ų�ѯ˰�̵�ǰʱ��ʧ��\n");
					cJSON_AddStringToObject(data_layer, "time", "0000-00-00 00:00:00");
				}
				else
				{
					cJSON_AddStringToObject(data_layer, "time", time);
				}
			}
		}
	}
	else
	{
		out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
			out("�����̺Ż�ȡ˰��ʱ��\n");
			memset(time,0,sizeof(time));
			//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_PLATE_TIME, plate_num,time);
			result = fun_get_plate_now_time(plate_num, time);
			if (result < 0)
			{
				out("ͨ���̺Ų�ѯ˰�̵�ǰʱ��ʧ��\n");
				cJSON_AddStringToObject(data_layer, "time", "0000-00-00 00:00:00");
			}
			else
			{
				cJSON_AddStringToObject(data_layer, "time", time);
			}		
	
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��ʱ��,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
//������������ȡ˰��˰����Ϣ
static void analysis_request_plate_tax_info(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int port_num;
	int usb_sum=0;
	int result = -1;
	unsigned char plate_num[13] = {0};
	struct _message_line *message_line;
	uint8 ter_id[13] = { 0 };
	char tmp_s[10] = { 0 };
	message_line = arg;
	stream = fd_stream;
	struct _usb_port_infor port_info;
	struct _switch_dev_id   id;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array, *port_array;		 //���ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n",message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	out("Ҫ���ѯȫ��USB���ϵ�˰����Ϣ\n");
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	//out("����������\n");       
	memset(&id, 0, sizeof(id));
	memset(ter_id, 0, sizeof(ter_id));
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, ter_id, 6);
	if (result < 0)
	{
		out("get_sev_pro����������result = %d\n", result);
	}
	out("�ն˻������Ϊ:%s USB�˿�����Ϊ��%d\n", ter_id, usb_sum);
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
	//������ݽ���

	if (1)
	{
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

		cJSON_AddStringToObject(data_layer, "ter_id", (const char *)ter_id);
		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%d", usb_sum);
		cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
		cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
		cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
		cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());

		out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

			//out("ͨ��˰�̱�Ų�ѯ���ڶ˿ں�\n");
			//port_num = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_PLATE_USB_PORT, plate_num);
			port_num = fun_get_plate_usb_port(plate_num);
			if (port_num<0)
			{
				out("get_usb_dev_info ��ȡUSB�˿���Ϣʧ��result=%d\n", result);
				continue;
			}
			//out("��ѯ���Ķ˿ں�Ϊ��%d\n",port_num);
			memset(&port_info, 0, sizeof(struct _usb_port_infor));
			result = fun_get_plate_usb_info(port_num, &port_info);
			if (result<0)
			{
				out("get_usb_dev_info ��ȡUSB�˿���Ϣʧ��result=%d\n", result);
				continue;
			}
			cJSON *port_layer;
			cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());


			memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.port);
			cJSON_AddStringToObject(port_layer, "usb_port", tmp_s);

			memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.usb_app);
			cJSON_AddStringToObject(port_layer, "ca_type", tmp_s);

			cJSON_AddStringToObject(port_layer, "ca_name", (char *)port_info.ca_name);
			cJSON_AddStringToObject(port_layer, "ca_number", (char *)port_info.ca_serial);

			memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.ca_ok);
			cJSON_AddStringToObject(port_layer, "ca_support", tmp_s);

			fun_get_port_data_json(port_layer, port_info.port, 0);

		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	str_replace(g_buf, "\\\\", "\\");
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰��˰����Ϣ,���\r\n", message_line->parm.random);
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
//������������Ʊ���ݲ�ѯ����
static void analysis_request_plate_invoice_details(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	unsigned char month[20] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	item = cJSON_GetObjectItem(arrayItem, "month");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���month������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(month, 0, sizeof(month));
		memcpy(month, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����Ҫ��ѯ�·�Ϊ%s\n", month);


	_lock_set(stream->inv_read_lock);
	//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_INV_DETAILS, plate_num, month, mqtt_pub_hook, message_line);
	result = fun_get_invoice_detail_mqtt(plate_num, month, mqtt_pub_hook, message_line,errinfo,0,1);
	_lock_un(stream->inv_read_lock);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		out("��Ʊ��ѯ����ʧ��\n");
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	cJSON_Delete(root);
	

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���ݲ�ѯ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//����������ȫ���¼
// static void analysis_request_qddengl(void *arg)
// {
// 	struct _app     *stream;
// 	int size;
// 	int result = -1;
// 	struct _message_line *message_line;

// 	message_line = arg;
// 	stream = fd_stream;
// 	cJSON *arrayItem, *item; //����ʹ��
// 	char errinfo[1024] = {0};
// 	char outcode[512] = {0};
// 	char outgoto[64] = {0};
// 	char outtoken[512] = {0};

	
// 	int i;
// 	int usb_port;				//������
// 	int plate_train;			//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
// 	char related_type[32]={0};		//��¼Ա����
// 	char full_name[32]={0};			//��¼Ա����
// 	char sz_password[32]={0};		//��¼Ա����
// 	char mobile[32]={0};			//��¼Ա�ֻ���
// 	char busid[32]={0};
// 	char inbusid[32]={0};
// 	char idcard[32]={0};
// 	char sys_type[32]={0}; 			 //��������,Ŀǰ������Ҫ���ֽ���ҳ������

// 	char login_method[32] = {0x00};
// 	char area_code[32] = {0x00};

// 	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
// 	int soft_cert_port = 0;
// 	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

// 	char soft_cert_url[128] = {0x00};

// 	strcpy(svr_container,"02");

// 	//out("����ȫ���¼����\n\n");
// 	out("\n\nlbc -------------------------------message_line->message = %s\n\n", message_line->message);
// 	//out("cmd  = %s\n", message_line->parm.cmd);

// 	//�������
// 	cJSON *json = cJSON_CreateObject();
// 	cJSON *head_layer, *data_array;
// 	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
// 	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
// 	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
// 	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
// 	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
// 	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
// 	cJSON *root = cJSON_Parse(message_line->message);
// 	if (!root)
// 	{
// 		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
// 		free(message_line->message);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
// 		goto End;
// 	}
// 	free(message_line->message);
// 	cJSON *object_data = cJSON_GetObjectItem(root, "data");
// 	if (object_data == NULL)
// 	{
// 		sprintf(errinfo, "�����JSON����data����ʧ��");
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
// 		cJSON_Delete(root);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
// 		goto End;
// 	}
// 	size = cJSON_GetArraySize(object_data);
// 	if (size != 1)
// 	{
// 		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
// 		cJSON_Delete(root);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 		goto End;
// 	}
// 	else
// 	{
// 		for (i = 0; i < size; i++)
// 		{
// 			arrayItem = cJSON_GetArrayItem(object_data, i);
// 			if (arrayItem == NULL)
// 			{
// 				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
// 				cJSON_Delete(root);
// 				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 				goto End;
// 			}
// 			item = cJSON_GetObjectItem(arrayItem, "login_method");
// 			if (item == NULL)
// 			{
// 				strcpy(login_method,"02");
// 			}
// 			else
// 			{
// 				strcpy(login_method,item->valuestring);
// 			}
// 			out("��ȡ����login_methodΪ%s\n", login_method);
// 			if(0 == memcmp(login_method,"01",2)  || 0 == memcmp(login_method,"21",2))    //��֤���¼
// 			{
// 				item = cJSON_GetObjectItem(arrayItem, "svr_container");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(svr_container,item->valuestring);
// 				out("��ȡ����svr_containerΪ%s\n", svr_container);
// 				item = cJSON_GetObjectItem(arrayItem, "related_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(related_type, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
// 				item = cJSON_GetObjectItem(arrayItem, "mobile");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(mobile, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

// 				item = cJSON_GetObjectItem(arrayItem, "sz_password");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sz_password, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

// 				item = cJSON_GetObjectItem(arrayItem, "sys_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sys_type, item->valuestring);
// 				out("��ȡ����sys_typeΪ%s\n", sys_type);  //get_soft_cert_url
// 				item = cJSON_GetObjectItem(arrayItem, "area_code");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(area_code, item->valuestring);
// 				out("��ȡ����area_codeΪ%s\n", area_code);
// 				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(soft_cert_url, item->valuestring);
// 				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
// 				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
// 				{
// 					sprintf(errinfo, "soft_cert_url error!");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				tmp_ptr = soft_cert_url;
// 				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
// 				{
// 					ptr = tmp_ptr;
// 				}
				
// 				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
// 				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
// 				soft_cert_port = atoi(ptr + 1);
// 				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
// 				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
// 				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);
// 			}
// 			else
// 			{
// 				item = cJSON_GetObjectItem(arrayItem, "usb_port");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				usb_port = atoi(item->valuestring);
// 				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

// 				item = cJSON_GetObjectItem(arrayItem, "related_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(related_type, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����%s\n", related_type);

// 				item = cJSON_GetObjectItem(arrayItem, "plate_train");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				plate_train = atoi(item->valuestring);
// 				out("��ȡ����˰������Ϊ%d\n", plate_train);

// 				item = cJSON_GetObjectItem(arrayItem, "mobile");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(mobile, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


// 				item = cJSON_GetObjectItem(arrayItem, "idcard");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(idcard, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

// 				item = cJSON_GetObjectItem(arrayItem, "sz_password");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sz_password, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

// 				item = cJSON_GetObjectItem(arrayItem, "sys_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sys_type, item->valuestring);
// 				out("��ȡ����sys_typeΪ%s\n", sys_type);

// 				item = cJSON_GetObjectItem(arrayItem, "busid");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(busid, item->valuestring);
				
// 				strcpy(inbusid,busid+2);
// 				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
// 			}
// 		}
// 	}
// 	if(plate_train == 1){				//�����̲�����
// 		char code[10] = {};
// 		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
// 		cJSON_AddStringToObject(head_layer, "result", "f");
// 		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 		cJSON *data_layer;
// 		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 		memcpy(code,errinfo,3);
// 		printf("code = %s",code);
// 		cJSON_AddStringToObject(data_layer, "code", code);
// 		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼ʧ��\r\n", message_line->parm.random,usb_port);
// 	}
// 	else {
// 		result = mbi_etax_login_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
// 		if(result == 0){
// 			cJSON_AddStringToObject(head_layer, "result", "s");
// 			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 			cJSON *data_layer;
// 			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 			cJSON_AddStringToObject(data_layer, "outgoto", outgoto);
// 			cJSON_AddStringToObject(data_layer, "outcode", outcode);
// 			char mqtttoken[64]={0};
// 			char new_key16[64]={0};
// 			char naturepublicKey[256]={0};
// 			char natureuuid[64]={0};

// 			char *mqttnew_key16 = NULL;
// 			char *mqttnaturepublicKey = NULL;
// 			char *mqttnatureuuid = NULL;

// 			mqttnew_key16 = strstr(outtoken,";new_key16:");

// 			if(mqttnew_key16){

// 				strncpy(mqtttoken,outtoken,strlen(outtoken)-strlen(mqttnew_key16));
// 				cJSON_AddStringToObject(data_layer, "token",mqtttoken);

// 				memset(new_key16,0,strlen(new_key16));
// 				strncpy(new_key16,(mqttnew_key16+11),16);
// 				printf("new_key16=%s\r\n",new_key16);
// 				cJSON_AddStringToObject(data_layer, "new_key16",new_key16);
// 			}	

// 			mqttnaturepublicKey = strstr(outtoken,";naturepublicKey:");
// 			if(mqttnaturepublicKey){
// 				memset(naturepublicKey,0,strlen(naturepublicKey));
// 				strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
// 				printf("naturepublicKey=%s\r\n",naturepublicKey);
// 				cJSON_AddStringToObject(data_layer, "naturepublicKey",naturepublicKey);
// 			}	

// 			mqttnatureuuid = strstr(outtoken,";natureuuid:");
// 			if(mqttnatureuuid){
// 				memset(natureuuid,0,strlen(natureuuid));
// 				strncpy(natureuuid,(mqttnatureuuid+12),32);
// 				printf("natureuuid=%s\r\n",natureuuid);
// 				cJSON_AddStringToObject(data_layer, "natureuuid",natureuuid);
// 			}		
// 			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼�ɹ�\r\n", message_line->parm.random,usb_port);
// 		}
// 		else{
// 			char code[10] = {};
// 			cJSON_AddStringToObject(head_layer, "result", "f");
// 			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 			cJSON *data_layer;
// 			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 			memcpy(code,errinfo,3);
// 			//printf("code = %s",code);
// 			cJSON_AddStringToObject(data_layer, "code", code);
// 			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
// 			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼ʧ��\r\n", message_line->parm.random,usb_port);
// 		}
// 	}

// 	char *g_buf;
// 	g_buf = cJSON_Print(json);
// 	printf("g_buf=%s\r\n",g_buf);
// 	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
// 	free(g_buf);
// 	cJSON_Delete(root);

// End:
// 	_lock_set(stream->mqtt_lock);
// 	if (message_callback.task_num != 0)
// 		message_callback.task_num -= 1;
// 	message_line->state = 0;
// 	message_line->inuse = 0;
// 	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
// 	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
// 	message_line->message = NULL;
// 	_lock_un(stream->mqtt_lock);
// 	return;
// }

static void analysis_request_qddengl(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char outcode[512] = {0};
	char outgoto[64] = {0};
	char outtoken[512] = {0};

	
	int i;
	int usb_port;				//������
	int plate_train;			//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	char related_type[32]={0};		//��¼Ա����
	char full_name[32]={0};			//��¼Ա����
	char sz_password[32]={0};		//��¼Ա����
	char mobile[32]={0};			//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char sys_type[32]={0}; 			 //��������,Ŀǰ������Ҫ���ֽ���ҳ������

	char login_method[32] = {0x00};
	char area_code[32] = {0x00};

	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
	int soft_cert_port = 0;
	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

	char soft_cert_url[128] = {0x00};



	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����
	char *etax_cookie = NULL;
	char *tpass_cookie = NULL;
	char *dppt_cookie = NULL;	

	char temp_data[1024]={0}; 

	strcpy(svr_container,"02");

	//out("����ȫ���¼����\n\n");
	out("\n\nlbc -------------------------------message_line->message = %s\n\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "login_method");
			if (item == NULL)
			{
				strcpy(login_method,"02");
			}
			else
			{
				strcpy(login_method,item->valuestring);
			}
			out("��ȡ����login_methodΪ%s\n", login_method);
			if(0 == memcmp(login_method,"01",2)  || 0 == memcmp(login_method,"21",2))    //��֤���¼
			{
				item = cJSON_GetObjectItem(arrayItem, "svr_container");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(svr_container,item->valuestring);
				out("��ȡ����svr_containerΪ%s\n", svr_container);
				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type);  //get_soft_cert_url
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_codeΪ%s\n", area_code);

				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(soft_cert_url, item->valuestring);
				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
				{
					sprintf(errinfo, "soft_cert_url error!");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				tmp_ptr = soft_cert_url;
				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
				{
					ptr = tmp_ptr;
				}
				
				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
				soft_cert_port = atoi(ptr + 1);
				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);
			}
			else
			{
				item = cJSON_GetObjectItem(arrayItem, "usb_port");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				usb_port = atoi(item->valuestring);
				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);

				item = cJSON_GetObjectItem(arrayItem, "plate_train");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				plate_train = atoi(item->valuestring);
				out("��ȡ����˰������Ϊ%d\n", plate_train);

				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


				item = cJSON_GetObjectItem(arrayItem, "idcard");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(idcard, item->valuestring);
				out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type);

				item = cJSON_GetObjectItem(arrayItem, "busid");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(busid, item->valuestring);
				
				strcpy(inbusid,busid+2);
				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else {
		//result = mbi_etax_login_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		printf("	app		sys_type  = %s 	      \r\n",sys_type);  
		if (0 == strcmp(sys_type,"2"))
		{
			//result = mbi_etax_get_cookie_Interface_tmp(1,"1",related_type,mobile,"",sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);	
			//result = mbi_etax_get_cookie_Interface_tmp(1,"1",related_type,mobile,"",sz_password, 0,"","0",errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);	
			result = mbi_etax_get_cookie_Interface(usb_port,inbusid,related_type,mobile,idcard,sz_password, 0,"","0",errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);		
			if(result == 0){
				printf("dppt_cookie	= %s\r\n",dppt_cookie);
				char name_dppt[] = "dzfp-ssotoken";
				char domain_dppt[] = ".chinatax.gov.cn";
				char value_dppt[64] = {0};
				char *tmp_ptr = NULL;

				tmp_ptr = strstr(dppt_cookie,name_dppt);
				if (tmp_ptr)
				{
					tmp_ptr += 14;
					strncpy(value_dppt,tmp_ptr,32);
					printf("value_dppt = %s	\r\n",value_dppt);
				}
				
				cJSON_AddStringToObject(head_layer, "result", "s");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				if(etax_cookie != NULL)
					cJSON_AddStringToObject(data_layer, "etax_cookie", etax_cookie);
				else 
					cJSON_AddStringToObject(data_layer, "etax_cookie", "");
				if(tpass_cookie != NULL)
					cJSON_AddStringToObject(data_layer, "tpass_cookie", tpass_cookie);
				else 
				{
					cJSON *data_layer_tmp;
					cJSON_AddItemToObject(data_layer, "dppt_cookie", data_layer_tmp = cJSON_CreateObject());
					cJSON_AddStringToObject(data_layer_tmp, "name",name_dppt);
					cJSON_AddStringToObject(data_layer_tmp, "domain",domain_dppt);
					cJSON_AddStringToObject(data_layer_tmp, "value",value_dppt);
				}
					//cJSON_AddStringToObject(data_layer, "tpass_cookie", "");
				if(dppt_cookie != NULL)
				{
					printf("temp_data = %s	\r\n",temp_data);
					cJSON *data_layer_tmp;
					cJSON_AddItemToObject(data_layer, "tpass_cookie", data_layer_tmp = cJSON_CreateObject());
					cJSON *data_layer_tmp_local;
					cJSON_AddItemToObject(data_layer_tmp, "localStorage", data_layer_tmp_local = cJSON_CreateObject());
					char mqtttoken[64]={0};
					char new_key16[64]={0};
					char naturepublicKey[256]={0};
					char natureuuid[64]={0};

					char *mqttnew_key16 = NULL;
					char *mqttnaturepublicKey = NULL;
					char *mqttnatureuuid = NULL;
					
					mqttnew_key16 = strstr(temp_data,";new_key16:");

					if(mqttnew_key16){

						strncpy(mqtttoken,temp_data,strlen(temp_data)-strlen(mqttnew_key16));
						cJSON_AddStringToObject(data_layer_tmp, "token",mqtttoken);

						memset(new_key16,0,strlen(new_key16));
						strncpy(new_key16,(mqttnew_key16+11),16);
						printf("new_key16=%s\r\n",new_key16);
						cJSON_AddStringToObject(data_layer_tmp_local, "new_key16",new_key16);
					}	

					mqttnaturepublicKey = strstr(temp_data,";naturepublicKey:");
					if(mqttnaturepublicKey){
						memset(naturepublicKey,0,strlen(naturepublicKey));
						strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
						printf("naturepublicKey=%s\r\n",naturepublicKey);
						cJSON_AddStringToObject(data_layer_tmp_local, "naturepublicKey",naturepublicKey);
					}	

					mqttnatureuuid = strstr(temp_data,";natureuuid:");
					if(mqttnatureuuid){
						memset(natureuuid,0,strlen(natureuuid));
						strncpy(natureuuid,(mqttnatureuuid+12),32);
						printf("natureuuid=%s\r\n",natureuuid);
						cJSON_AddStringToObject(data_layer_tmp_local, "natureuuid",natureuuid);
					}
				}
					//cJSON_AddStringToObject(data_layer, "dppt_cookie", dppt_cookie);
				else 
					cJSON_AddStringToObject(data_layer, "dppt_cookie", "");
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookies�ɹ�\r\n", message_line->parm.random,usb_port);
				printf("ȫ���ȡcookies�ɹ�\r\n");
			}
			else{
				char code[10] = {};
				cJSON_AddStringToObject(head_layer, "result", "f");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				memcpy(code,errinfo,3);
				printf("code = %s",code);
				cJSON_AddStringToObject(data_layer, "code", code);
				cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookiesʧ��\r\n", message_line->parm.random,usb_port);
				printf("ȫ���ȡcookiesʧ��\r\n");
			}
		}
		else
		{
			//result = mbi_etax_login_Interface(1,"1",related_type,full_name,mobile,"",sz_password,errinfo,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
			result = mbi_etax_login_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
			if(result == 0){
				cJSON_AddStringToObject(head_layer, "result", "s");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(data_layer, "outgoto", outgoto);
				cJSON_AddStringToObject(data_layer, "outcode", outcode);
				char mqtttoken[64]={0};
				char new_key16[64]={0};
				char naturepublicKey[256]={0};
				char natureuuid[64]={0};

				char *mqttnew_key16 = NULL;
				char *mqttnaturepublicKey = NULL;
				char *mqttnatureuuid = NULL;

				mqttnew_key16 = strstr(outtoken,";new_key16:");

				if(mqttnew_key16){

					strncpy(mqtttoken,outtoken,strlen(outtoken)-strlen(mqttnew_key16));
					cJSON_AddStringToObject(data_layer, "token",mqtttoken);

					memset(new_key16,0,strlen(new_key16));
					strncpy(new_key16,(mqttnew_key16+11),16);
					printf("new_key16=%s\r\n",new_key16);
					cJSON_AddStringToObject(data_layer, "new_key16",new_key16);
				}	

				mqttnaturepublicKey = strstr(outtoken,";naturepublicKey:");
				if(mqttnaturepublicKey){
					memset(naturepublicKey,0,strlen(naturepublicKey));
					strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
					printf("naturepublicKey=%s\r\n",naturepublicKey);
					cJSON_AddStringToObject(data_layer, "naturepublicKey",naturepublicKey);
				}	

				mqttnatureuuid = strstr(outtoken,";natureuuid:");
				if(mqttnatureuuid){
					memset(natureuuid,0,strlen(natureuuid));
					strncpy(natureuuid,(mqttnatureuuid+12),32);
					printf("natureuuid=%s\r\n",natureuuid);
					cJSON_AddStringToObject(data_layer, "natureuuid",natureuuid);
				}		
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼�ɹ�\r\n", message_line->parm.random,usb_port);
				printf("ȫ���¼�ɹ�\r\n");
			}
			else{
				char code[10] = {};
				cJSON_AddStringToObject(head_layer, "result", "f");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				memcpy(code,errinfo,3);
				//printf("code = %s",code);
				cJSON_AddStringToObject(data_layer, "code", code);
				cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼ʧ��\r\n", message_line->parm.random,usb_port);
				printf("��¼ʧ�ܣ�����\r\n");
			}
		}	

	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);

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
	return;
}

//����������ȫ���û���Ϣ��ѯ ���ݡ���֤�顿
static void analysis_request_qdyongyxxhq(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *outinfo = NULL;

	int i;
	int usb_port;				//������
	int plate_train;
	char busid[32] = {0x00};
	char inbusid[32] = {0x00};

	char login_method[32] = {0x00};
	char area_code[32] = {0x00};

	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
	int soft_cert_port = 0;
	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

	char soft_cert_url[128] = {0x00};

	strcpy(svr_container,"02");


	// out("����ȫ�硾�û���Ϣ��ȡ������\n\n");
	// out("message_line->message = %s\n", message_line->message);
	// out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	// cJSON *root = cJSON_Parse(message_line->message);
	// if (!root)
	// {
	// 	sprintf(errinfo,"�����JSON���ݽ���ʧ��");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
	// 	free(message_line->message);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
	// 	goto End;
	// }
	// free(message_line->message);
	// cJSON *object_data = cJSON_GetObjectItem(root, "data");
	// if (object_data == NULL)
	// {
	// 	sprintf(errinfo, "�����JSON����data����ʧ��");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
	// 	cJSON_Delete(root);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
	// 	goto End;
	// }
	// size = cJSON_GetArraySize(object_data);
	// if (size != 1)
	// {
	// 	sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
	// 	cJSON_Delete(root);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 	goto End;
	// }
	// else
	// {
	// 	for (i = 0; i < size; i++)
	// 	{
	// 		arrayItem = cJSON_GetArrayItem(object_data, i);
	// 		if (arrayItem == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}

	// 		item = cJSON_GetObjectItem(arrayItem, "usb_port");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		usb_port = atoi(item->valuestring);
	// 		out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);


	// 		item = cJSON_GetObjectItem(arrayItem, "plate_train");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		plate_train = atoi(item->valuestring);
	// 		out("��ȡ����˰������Ϊ%d\n", plate_train);


	// 		item = cJSON_GetObjectItem(arrayItem, "busid");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		strcpy(busid, item->valuestring);
			
	// 		strcpy(inbusid,busid+2);
	// 		out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
	// 	}
	// }

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "login_method");
			if (item == NULL)
			{
				strcpy(login_method,"02");
			}
			else
			{
				strcpy(login_method,item->valuestring);
			}
			out("��ȡ����login_methodΪ%s\n", login_method);
			if(0 == memcmp(login_method,"01",2) || 0 == memcmp(login_method,"21",2))    //��֤���¼
			{
				printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\r\n");
				item = cJSON_GetObjectItem(arrayItem, "svr_container");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(svr_container,item->valuestring);
				out("��ȡ����svr_containerΪ%s\n", svr_container);
				/*item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);*/
				/*item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);*/

				/*item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type); */ //get_soft_cert_url
				
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_codeΪ%s\n", area_code);
				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(soft_cert_url, item->valuestring);
				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
				{
					sprintf(errinfo, "soft_cert_url error!");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				tmp_ptr = soft_cert_url;
				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
				{
					ptr = tmp_ptr;
				}
				
				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
				soft_cert_port = atoi(ptr + 1);
				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);
			}

			else
			{
				item = cJSON_GetObjectItem(arrayItem, "usb_port");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				usb_port = atoi(item->valuestring);
				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);


				item = cJSON_GetObjectItem(arrayItem, "plate_train");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				plate_train = atoi(item->valuestring);
				out("��ȡ����˰������Ϊ%d\n", plate_train);


				item = cJSON_GetObjectItem(arrayItem, "busid");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(busid, item->valuestring);
				
				strcpy(inbusid,busid+2);
				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
			
			}
		}
	}
	
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		//cJSON_AddStringToObject(data_layer, "msg", errinfo);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡ�û���Ϣʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		//result = mbi_etax_user_query_Interface(usb_port,inbusid,errinfo,&outinfo);
		result = mbi_etax_user_query_Interface(usb_port,inbusid,errinfo,&outinfo,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "outinfo", outinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡ�û���Ϣ�ɹ�\r\n", message_line->parm.random,usb_port);
			//printf("ȫ���ȡ�û���Ϣ�ɹ�\r\n");
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			//cJSON_AddStringToObject(data_layer, "msg", errinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡ�û���Ϣʧ��\r\n", message_line->parm.random,usb_port);
			//printf("ȫ���ȡ�û���Ϣʧ��\r\n");
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	if(outinfo)
		free(outinfo);
	outinfo = NULL;

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
	return;
}

//������������ҵ�б����� ���ݡ���֤�顿
static void analysis_request_qdqiyeqiehuanlbcx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *outenterpriselist = NULL;

	
	int i;
	int usb_port;				//������
	int plate_train;			//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	char related_type[32] = {0x00};		//��¼Ա����
	char full_name[32] = {0x00};			//��¼Ա����
	char sz_password[32] = {0x00};		//��¼Ա����
	char mobile[32] = {0x00};			//��¼Ա�ֻ���
	char busid[32] = {0x00};
	char inbusid[32] = {0x00};
	char idcard[32] = {0x00};


	char login_method[32] = {0x00};
	char area_code[32] = {0x00};

	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
	int soft_cert_port = 0;
	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

	char soft_cert_url[128] = {0x00};

	strcpy(svr_container,"02");


	out("����ȫ���¼����\n\n");
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	// cJSON *root = cJSON_Parse(message_line->message);
	// if (!root)
	// {
	// 	sprintf(errinfo,"�����JSON���ݽ���ʧ��");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�б���ȡ,%s\r\n", message_line->parm.random,errinfo);
	// 	free(message_line->message);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
	// 	goto End;
	// }
	// free(message_line->message);
	// cJSON *object_data = cJSON_GetObjectItem(root, "data");
	// if (object_data == NULL)
	// {
	// 	sprintf(errinfo, "�����JSON����data����ʧ��");
	// 	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�б���ȡ,%s\r\n", message_line->parm.random, errinfo);
	// 	cJSON_Delete(root);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
	// 	goto End;
	// }
	// size = cJSON_GetArraySize(object_data);
	// if (size != 1)
	// {
	// 	sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
	// 	cJSON_Delete(root);
	// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 	goto End;
	// }
	// else
	// {
	// 	for (i = 0; i < size; i++)
	// 	{
	// 		arrayItem = cJSON_GetArrayItem(object_data, i);
	// 		if (arrayItem == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}

	// 		item = cJSON_GetObjectItem(arrayItem, "usb_port");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		usb_port = atoi(item->valuestring);
	// 		out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

	// 		item = cJSON_GetObjectItem(arrayItem, "plate_train");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		plate_train = atoi(item->valuestring);
	// 		out("��ȡ����˰������Ϊ%d\n", plate_train);

	// 		item = cJSON_GetObjectItem(arrayItem, "mobile");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		strcpy(mobile, item->valuestring);
	// 		out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


	// 		item = cJSON_GetObjectItem(arrayItem, "idcard");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		strcpy(idcard, item->valuestring);
	// 		out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

	// 		item = cJSON_GetObjectItem(arrayItem, "sz_password");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		strcpy(sz_password, item->valuestring);
	// 		out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

	// 		item = cJSON_GetObjectItem(arrayItem, "busid");
	// 		if (item == NULL)
	// 		{
	// 			sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
	// 			cJSON_Delete(root);
	// 			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	// 			goto End;
	// 		}
	// 		strcpy(busid, item->valuestring);
			
	// 		strcpy(inbusid,busid+2);
	// 		out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
	// 	}
	// }
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�б���ȡ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�б���ȡ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "login_method");
			if (item == NULL)
			{
				strcpy(login_method,"02");
			}
			else
			{
				strcpy(login_method,item->valuestring);
			}
			out("��ȡ����login_methodΪ%s\n", login_method);
			if(0 == memcmp(login_method,"01",2) || 0 == memcmp(login_method,"21",2))    //��֤���¼
			{
				item = cJSON_GetObjectItem(arrayItem, "svr_container");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(svr_container,item->valuestring);
				out("��ȡ����svr_containerΪ%s\n", svr_container);
				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				/*item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type);*/  //get_soft_cert_url
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_codeΪ%s\n", area_code);
				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(soft_cert_url, item->valuestring);
				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
				{
					sprintf(errinfo, "soft_cert_url error!");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				tmp_ptr = soft_cert_url;
				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
				{
					ptr = tmp_ptr;
				}
				
				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
				soft_cert_port = atoi(ptr + 1);
				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);
			}

			else
			{
				item = cJSON_GetObjectItem(arrayItem, "usb_port");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				usb_port = atoi(item->valuestring);
				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

				item = cJSON_GetObjectItem(arrayItem, "plate_train");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				plate_train = atoi(item->valuestring);
				out("��ȡ����˰������Ϊ%d\n", plate_train);

				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Աrelated_typeΪ%s\n", related_type);		

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "busid");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(busid, item->valuestring);
				
				strcpy(inbusid,busid+2);
				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			}
		}
	}


	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		//cJSON_AddStringToObject(data_layer, "msg", errinfo);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л��б���ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		//result = mbi_etax_relationlist_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,&outenterpriselist);	
		result = mbi_etax_relationlist_Interface(usb_port,inbusid,related_type,full_name,mobile,"",sz_password,errinfo,&outenterpriselist,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "outenterpriselist", outenterpriselist);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л��б���ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
			//printf("ȫ����ҵ�л��б���ѯ�ɹ�\r\n");
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			//cJSON_AddStringToObject(data_layer, "msg", errinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л��б���ѯʧ��\r\n", message_line->parm.random,usb_port);
			//printf("ȫ����ҵ�л��б���ѯʧ��\r\n");
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);

	if(outenterpriselist)
		free(outenterpriselist);
	outenterpriselist = NULL;

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
	return;
}

//������������ҵ�л� ���ݡ���֤�顿
// static void analysis_request_qdqiyeqiehuan(void *arg)
// {
// 	struct _app     *stream;
// 	int size;
// 	int result = -1;
// 	struct _message_line *message_line;

// 	message_line = arg;
// 	stream = fd_stream;
// 	cJSON *arrayItem, *item; //����ʹ��
// 	char errinfo[1024] = {0};

// 	char outcode[512] = {0};
// 	char outgoto[64] = {0};
// 	char outtoken[512] = {0};

	
// 	int i;
// 	int usb_port;				//������
// 	int plate_train;
// 	char related_type[32] = {0x00};		//��¼Ա����
// 	char full_name[32] = {0x00};			//��¼Ա����
// 	char sz_password[32] = {0x00};		//��¼Ա����
// 	char mobile[32] = {0x00};			//��¼Ա�ֻ���
// 	char busid[32] = {0x00};
// 	char inbusid[32] = {0x00};
// 	char idcard[32] = {0x00};
// 	char uniqueIdentity[32] = {0x00};	//�л�����ҵ˰��
// 	char realationStatus[32] = {0x00};	//�л�����ҵ������
// 	char sys_type[32] = {0x00};


// 	char login_method[32] = {0x00};
// 	char area_code[32] = {0x00};

// 	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
// 	int soft_cert_port = 0;
// 	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

// 	char soft_cert_url[128] = {0x00};

// 	strcpy(svr_container,"02");

// 	out("����ȫ���¼����\n\n");
// 	out("message_line->message = %s\n", message_line->message);
// 	out("cmd  = %s\n", message_line->parm.cmd);

// 	//�������
// 	cJSON *json = cJSON_CreateObject();
// 	cJSON *head_layer, *data_array;
// 	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
// 	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
// 	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
// 	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
// 	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
// 	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

// 	cJSON *root = cJSON_Parse(message_line->message);
// 	if (!root)
// 	{
// 		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л���¼,%s\r\n", message_line->parm.random,errinfo);
// 		free(message_line->message);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
// 		goto End;
// 	}
// 	free(message_line->message);
// 	cJSON *object_data = cJSON_GetObjectItem(root, "data");
// 	if (object_data == NULL)
// 	{
// 		sprintf(errinfo, "�����JSON����data����ʧ��");
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л���¼,%s\r\n", message_line->parm.random, errinfo);
// 		cJSON_Delete(root);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
// 		goto End;
// 	}
// 	size = cJSON_GetArraySize(object_data);
// 	if (size != 1)
// 	{
// 		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
// 		cJSON_Delete(root);
// 		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 		goto End;
// 	}
// 	else
// 	{
// 		for (i = 0; i < size; i++)
// 		{
// 			arrayItem = cJSON_GetArrayItem(object_data, i);
// 			if (arrayItem == NULL)
// 			{
// 				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
// 				cJSON_Delete(root);
// 				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 				goto End;
// 			}
// 			item = cJSON_GetObjectItem(arrayItem, "login_method");
// 			if (item == NULL)
// 			{
// 				strcpy(login_method,"02");
// 			}
// 			else
// 			{
// 				strcpy(login_method,item->valuestring);
// 			}
// 			out("��ȡ����login_methodΪ%s\n", login_method);
// 			if(0 == memcmp(login_method,"01",2) || 0 == memcmp(login_method,"21",2))    //��֤���¼ 
// 			{
// 				item = cJSON_GetObjectItem(arrayItem, "svr_container");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(svr_container,item->valuestring);
// 				out("��ȡ����svr_containerΪ%s\n", svr_container);
// 				item = cJSON_GetObjectItem(arrayItem, "related_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(related_type, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
// 				item = cJSON_GetObjectItem(arrayItem, "mobile");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(mobile, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

// 				item = cJSON_GetObjectItem(arrayItem, "sz_password");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sz_password, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

// 				item = cJSON_GetObjectItem(arrayItem, "sys_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sys_type, item->valuestring);
// 				out("��ȡ����sys_typeΪ%s\n", sys_type);  //get_soft_cert_url
// 				item = cJSON_GetObjectItem(arrayItem, "area_code");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(area_code, item->valuestring);
// 				out("��ȡ����area_codeΪ%s\n", area_code);
// 				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(soft_cert_url, item->valuestring);
// 				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
// 				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
// 				{
// 					sprintf(errinfo, "soft_cert_url error!");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				tmp_ptr = soft_cert_url;
// 				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
// 				{
// 					ptr = tmp_ptr;
// 				}
				
// 				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
// 				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
// 				soft_cert_port = atoi(ptr + 1);
// 				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
// 				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
// 				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);



// 				item = cJSON_GetObjectItem(arrayItem, "realationStatus");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(realationStatus, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);


// 				item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(uniqueIdentity, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);


// 			}
			
// 			else
// 			{
// 				item = cJSON_GetObjectItem(arrayItem, "usb_port");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				usb_port = atoi(item->valuestring);
// 				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

// 				item = cJSON_GetObjectItem(arrayItem, "plate_train");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				plate_train = atoi(item->valuestring);
// 				out("��ȡ����˰������Ϊ%d\n", plate_train);

// 				item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(uniqueIdentity, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


// 				item = cJSON_GetObjectItem(arrayItem, "realationStatus");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(realationStatus, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

// 				item = cJSON_GetObjectItem(arrayItem, "mobile");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(mobile, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


// 				item = cJSON_GetObjectItem(arrayItem, "idcard");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(idcard, item->valuestring);
// 				out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

// 				item = cJSON_GetObjectItem(arrayItem, "sz_password");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(sz_password, item->valuestring);
// 				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

// 				item = cJSON_GetObjectItem(arrayItem, "sys_type");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				else
// 				strcpy(sys_type, item->valuestring);
// 				out("��ȡ����sys_typeΪ%s\n", sys_type);

// 				item = cJSON_GetObjectItem(arrayItem, "busid");
// 				if (item == NULL)
// 				{
// 					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
// 					cJSON_Delete(root);
// 					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
// 					goto End;
// 				}
// 				strcpy(busid, item->valuestring);
				
// 				strcpy(inbusid,busid+2);
// 				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
// 			}
// 		}
// 	}

// 	if(plate_train == 1){				//�����̲�����
// 		char code[10] = {};
// 		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
// 		cJSON_AddStringToObject(head_layer, "result", "f");
// 		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 		cJSON *data_layer;
// 		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 		memcpy(code,errinfo,3);
// 		printf("code = %s",code);
// 		cJSON_AddStringToObject(data_layer, "code", code);
// 		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
// 		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л�ʧ��\r\n", message_line->parm.random,usb_port);
// 	}
// 	else 
// 	{
// 		//result = mbi_etax_relationchange_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,outgoto,outcode,outtoken,sys_type);	
// 		result = mbi_etax_relationchange_Interface(usb_port,inbusid,related_type,full_name,mobile,"",sz_password,errinfo,uniqueIdentity,realationStatus,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
// 		if(result == 0){
// 			cJSON_AddStringToObject(head_layer, "result", "s");
// 			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 			cJSON *data_layer;
// 			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 			cJSON_AddStringToObject(data_layer, "outgoto", outgoto);
// 			cJSON_AddStringToObject(data_layer, "outcode", outcode);
// 			char mqtttoken[64]={0};
// 			char new_key16[64]={0};
// 			char naturepublicKey[256]={0};
// 			char natureuuid[64]={0};

// 			char *mqttnew_key16 = NULL;
// 			char *mqttnaturepublicKey = NULL;
// 			char *mqttnatureuuid = NULL;

// 			mqttnew_key16 = strstr(outtoken,";new_key16:");

// 			if(mqttnew_key16){

// 				strncpy(mqtttoken,outtoken,strlen(outtoken)-strlen(mqttnew_key16));
// 				cJSON_AddStringToObject(data_layer, "token",mqtttoken);

// 				memset(new_key16,0,strlen(new_key16));
// 				strncpy(new_key16,(mqttnew_key16+11),16);
// 				printf("new_key16=%s\r\n",new_key16);
// 				cJSON_AddStringToObject(data_layer, "new_key16",new_key16);
// 			}	

// 			mqttnaturepublicKey = strstr(outtoken,";naturepublicKey:");
// 			if(mqttnaturepublicKey){
// 				memset(naturepublicKey,0,strlen(naturepublicKey));
// 				strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
// 				printf("naturepublicKey=%s\r\n",naturepublicKey);
// 				cJSON_AddStringToObject(data_layer, "naturepublicKey",naturepublicKey);
// 			}	

// 			mqttnatureuuid = strstr(outtoken,";natureuuid:");
// 			if(mqttnatureuuid){
// 				memset(natureuuid,0,strlen(natureuuid));
// 				strncpy(natureuuid,(mqttnatureuuid+12),32);
// 				printf("natureuuid=%s\r\n",natureuuid);
// 				cJSON_AddStringToObject(data_layer, "natureuuid",natureuuid);
// 			}		
// 			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���¼�ɹ�\r\n", message_line->parm.random,usb_port);
// 		}
// 		else{
// 			char code[10] = {};
// 			cJSON_AddStringToObject(head_layer, "result", "f");
// 			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
// 			cJSON *data_layer;
// 			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
// 			memcpy(code,errinfo,3);
// 			//printf("code = %s",code);
// 			cJSON_AddStringToObject(data_layer, "code", code);
// 			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
// 			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л�ʧ��\r\n", message_line->parm.random,usb_port);
// 			//printf("ȫ����ҵ�л�ʧ��\r\n");
// 		}
// 	}
	
// 	char *g_buf;
// 	g_buf = cJSON_Print(json);
// 	//printf("g_buf=%s\r\n",g_buf);
// 	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
// 	free(g_buf);
// 	cJSON_Delete(root);

// End:
// 	_lock_set(stream->mqtt_lock);
// 	if (message_callback.task_num != 0)
// 		message_callback.task_num -= 1;
// 	message_line->state = 0;
// 	message_line->inuse = 0;
// 	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
// 	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
// 	message_line->message = NULL;
// 	_lock_un(stream->mqtt_lock);
// 	return;
// }

static void analysis_request_qdqiyeqiehuan(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};

	char outcode[512] = {0};
	char outgoto[64] = {0};
	char outtoken[512] = {0};

	
	int i;
	int usb_port;				//������
	int plate_train;
	char related_type[32] = {0x00};		//��¼Ա����
	char full_name[32] = {0x00};			//��¼Ա����
	char sz_password[32] = {0x00};		//��¼Ա����
	char mobile[32] = {0x00};			//��¼Ա�ֻ���
	char busid[32] = {0x00};
	char inbusid[32] = {0x00};
	char idcard[32] = {0x00};
	char uniqueIdentity[32] = {0x00};	//�л�����ҵ˰��
	char realationStatus[32] = {0x00};	//�л�����ҵ������
	char sys_type[32] = {0x00};


	char login_method[32] = {0x00};
	char area_code[32] = {0x00};

	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
	int soft_cert_port = 0;
	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

	char soft_cert_url[128] = {0x00};

	//get_cookie ����
	char *etax_cookie = NULL;
	char *tpass_cookie = NULL;
	char *dppt_cookie = NULL;	
	char temp_data[1024]={0}; 

	strcpy(svr_container,"02");
	out("����ȫ���¼����\n\n");
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ҵ�л���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "login_method");
			if (item == NULL)
			{
				strcpy(login_method,"02");
			}
			else
			{
				strcpy(login_method,item->valuestring);
			}
			out("��ȡ����login_methodΪ%s\n", login_method);
			if(0 == memcmp(login_method,"01",2) || 0 == memcmp(login_method,"21",2))    //��֤���¼ 
			{
				item = cJSON_GetObjectItem(arrayItem, "svr_container");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(svr_container,item->valuestring);
				out("��ȡ����svr_containerΪ%s\n", svr_container);
				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type);  //get_soft_cert_url
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_codeΪ%s\n", area_code);

				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(soft_cert_url, item->valuestring);
				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
				{
					sprintf(errinfo, "soft_cert_url error!");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				tmp_ptr = soft_cert_url;
				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
				{
					ptr = tmp_ptr;
				}
				
				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
				soft_cert_port = atoi(ptr + 1);
				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);



				item = cJSON_GetObjectItem(arrayItem, "realationStatus");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(realationStatus, item->valuestring);
				out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);


				item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(uniqueIdentity, item->valuestring);
				out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);


			}
			
			else
			{
				item = cJSON_GetObjectItem(arrayItem, "usb_port");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				usb_port = atoi(item->valuestring);
				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

				item = cJSON_GetObjectItem(arrayItem, "plate_train");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				plate_train = atoi(item->valuestring);
				out("��ȡ����˰������Ϊ%d\n", plate_train);

				item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(uniqueIdentity, item->valuestring);
				out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


				item = cJSON_GetObjectItem(arrayItem, "realationStatus");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(realationStatus, item->valuestring);
				out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


				item = cJSON_GetObjectItem(arrayItem, "idcard");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(idcard, item->valuestring);
				out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "sys_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				else
				strcpy(sys_type, item->valuestring);
				out("��ȡ����sys_typeΪ%s\n", sys_type);

				item = cJSON_GetObjectItem(arrayItem, "busid");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(busid, item->valuestring);
				
				strcpy(inbusid,busid+2);
				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
			}
		}
	}

	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л�ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		printf("	app		qiehuan		sys_type  = %s 	      \r\n",sys_type);  
		if (0 == strcmp(sys_type,"2"))
		{
			//result = mbi_etax_get_cookie_Interface_tmp(1,"1",related_type,mobile,"",sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);	
			//result = mbi_etax_get_cookie_Interface_tmp(1,"1",related_type,mobile,"",sz_password, 1,uniqueIdentity,realationStatus,errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);	
			result = mbi_etax_get_cookie_Interface(usb_port,inbusid,related_type,mobile,"",sz_password, 1,uniqueIdentity,realationStatus,errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);	
			if(result == 0){
				printf("dppt_cookie	= %s\r\n",dppt_cookie);
				char name_dppt[] = "dzfp-ssotoken";
				char domain_dppt[] = ".chinatax.gov.cn";
				char value_dppt[64] = {0};
				char *tmp_ptr = NULL;

				tmp_ptr = strstr(dppt_cookie,name_dppt);
				if (tmp_ptr)
				{
					tmp_ptr += 14;
					strncpy(value_dppt,tmp_ptr,32);
					printf("value_dppt = %s	\r\n",value_dppt);
				}
				
				cJSON_AddStringToObject(head_layer, "result", "s");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				if(etax_cookie != NULL)
					cJSON_AddStringToObject(data_layer, "etax_cookie", etax_cookie);
				else 
					cJSON_AddStringToObject(data_layer, "etax_cookie", "");
				if(tpass_cookie != NULL)
					cJSON_AddStringToObject(data_layer, "tpass_cookie", tpass_cookie);
				else 
				{
					cJSON *data_layer_tmp;
					cJSON_AddItemToObject(data_layer, "dppt_cookie", data_layer_tmp = cJSON_CreateObject());
					cJSON_AddStringToObject(data_layer_tmp, "name",name_dppt);
					cJSON_AddStringToObject(data_layer_tmp, "domain",domain_dppt);
					cJSON_AddStringToObject(data_layer_tmp, "value",value_dppt);
				}
					//cJSON_AddStringToObject(data_layer, "tpass_cookie", "");
				if(dppt_cookie != NULL)
				{
					printf("temp_data = %s	\r\n",temp_data);
					cJSON *data_layer_tmp;
					cJSON_AddItemToObject(data_layer, "tpass_cookie", data_layer_tmp = cJSON_CreateObject());
					cJSON *data_layer_tmp_local;
					cJSON_AddItemToObject(data_layer_tmp, "localStorage", data_layer_tmp_local = cJSON_CreateObject());
					char mqtttoken[64]={0};
					char new_key16[64]={0};
					char naturepublicKey[256]={0};
					char natureuuid[64]={0};

					char *mqttnew_key16 = NULL;
					char *mqttnaturepublicKey = NULL;
					char *mqttnatureuuid = NULL;
					
					mqttnew_key16 = strstr(temp_data,";new_key16:");

					if(mqttnew_key16){

						strncpy(mqtttoken,temp_data,strlen(temp_data)-strlen(mqttnew_key16));
						cJSON_AddStringToObject(data_layer_tmp, "token",mqtttoken);

						memset(new_key16,0,strlen(new_key16));
						strncpy(new_key16,(mqttnew_key16+11),16);
						printf("new_key16=%s\r\n",new_key16);
						cJSON_AddStringToObject(data_layer_tmp_local, "new_key16",new_key16);
					}	

					mqttnaturepublicKey = strstr(temp_data,";naturepublicKey:");
					if(mqttnaturepublicKey){
						memset(naturepublicKey,0,strlen(naturepublicKey));
						strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
						printf("naturepublicKey=%s\r\n",naturepublicKey);
						cJSON_AddStringToObject(data_layer_tmp_local, "naturepublicKey",naturepublicKey);
					}	

					mqttnatureuuid = strstr(temp_data,";natureuuid:");
					if(mqttnatureuuid){
						memset(natureuuid,0,strlen(natureuuid));
						strncpy(natureuuid,(mqttnatureuuid+12),32);
						printf("natureuuid=%s\r\n",natureuuid);
						cJSON_AddStringToObject(data_layer_tmp_local, "natureuuid",natureuuid);
					}
				}
					//cJSON_AddStringToObject(data_layer, "dppt_cookie", dppt_cookie);
				else 
					cJSON_AddStringToObject(data_layer, "dppt_cookie", "");
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookies�ɹ�\r\n", message_line->parm.random,usb_port);
				printf("ȫ���ȡcookies�ɹ�\r\n");
			}
			else{
				char code[10] = {};
				cJSON_AddStringToObject(head_layer, "result", "f");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				memcpy(code,errinfo,3);
				printf("code = %s",code);
				cJSON_AddStringToObject(data_layer, "code", code);
				cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookiesʧ��\r\n", message_line->parm.random,usb_port);
				printf("ȫ���ȡcookiesʧ��\r\n");
			}
		}
		else
		{
			//result = mbi_etax_relationchange_Interface(usb_port,inbusid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,outgoto,outcode,outtoken,sys_type);	
			result = mbi_etax_relationchange_Interface(usb_port,inbusid,related_type,full_name,mobile,"",sz_password,errinfo,uniqueIdentity,realationStatus,outgoto,outcode,outtoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);	
			if(result == 0){
				cJSON_AddStringToObject(head_layer, "result", "s");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(data_layer, "outgoto", outgoto);
				cJSON_AddStringToObject(data_layer, "outcode", outcode);
				char mqtttoken[64]={0};
				char new_key16[64]={0};
				char naturepublicKey[256]={0};
				char natureuuid[64]={0};

				char *mqttnew_key16 = NULL;
				char *mqttnaturepublicKey = NULL;
				char *mqttnatureuuid = NULL;

				mqttnew_key16 = strstr(outtoken,";new_key16:");

				if(mqttnew_key16){

					strncpy(mqtttoken,outtoken,strlen(outtoken)-strlen(mqttnew_key16));
					cJSON_AddStringToObject(data_layer, "token",mqtttoken);

					memset(new_key16,0,strlen(new_key16));
					strncpy(new_key16,(mqttnew_key16+11),16);
					printf("new_key16=%s\r\n",new_key16);
					cJSON_AddStringToObject(data_layer, "new_key16",new_key16);
				}	

				mqttnaturepublicKey = strstr(outtoken,";naturepublicKey:");
				if(mqttnaturepublicKey){
					memset(naturepublicKey,0,strlen(naturepublicKey));
					strncpy(naturepublicKey,(mqttnaturepublicKey+17),130);
					printf("naturepublicKey=%s\r\n",naturepublicKey);
					cJSON_AddStringToObject(data_layer, "naturepublicKey",naturepublicKey);
				}	

				mqttnatureuuid = strstr(outtoken,";natureuuid:");
				if(mqttnatureuuid){
					memset(natureuuid,0,strlen(natureuuid));
					strncpy(natureuuid,(mqttnatureuuid+12),32);
					printf("natureuuid=%s\r\n",natureuuid);
					cJSON_AddStringToObject(data_layer, "natureuuid",natureuuid);
				}		
				printf("ȫ����ҵ�л����\r\n");
			}
			else{
				char code[10] = {};
				cJSON_AddStringToObject(head_layer, "result", "f");
				cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
				cJSON *data_layer;
				cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
				memcpy(code,errinfo,3);
				//printf("code = %s",code);
				cJSON_AddStringToObject(data_layer, "code", code);
				cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����ҵ�л�ʧ��\r\n", message_line->parm.random,usb_port);
				printf("ȫ����ҵ�л�ʧ��\r\n");
			}
		}

	}
	
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);

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
	return;
}

//����������ȫ�緢Ʊ��ѯ
static void analysis_request_qdfpcx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	out("����ȫ���¼����\n\n");
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);



			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ��ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_invoice_query_Interface(usb_port,inbusid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
				strcpy(tmp_cookie,new_etax_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				//asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
				strcpy(tmp_cookie,new_tpass_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				//asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
				strcpy(tmp_cookie,new_dppt_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ��ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ��ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}

//������������ȡclienthello
static void analysis_request_get_clienthello(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1,len = 0;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	int i = 0;
	char plate_num[32] = {0x00},client_hello[1024*5] = {0x00};
	
	out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclienthello,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclienthello,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num,item->valuestring);
			out("plate_num = %s\n",plate_num);
		}	
	
		result = fun_client_hello_d(plate_num,client_hello,errinfo,1);  //Ĭ���ǵ���˰��
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "client_hello", client_hello);
			printf("client_hello = %s\n",client_hello);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclienthello�ɹ�\r\n", message_line->parm.random);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclienthelloʧ��\r\n", message_line->parm.random);
		}
	}
	
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

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
	return;
}

//������������ȡclientauth
static void analysis_request_get_clientauth(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1,len = 0;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	int i = 0;
	char plate_num[32] = {0x00},sever_hello[1024*5] = {0x00},client_auth[1024*5] = {0x00};
	
	out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclientauth,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclientauth,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num,item->valuestring);
			out("plate_num = %s\n",plate_num);

			item = cJSON_GetObjectItem(arrayItem, "sever_hello");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���sever_hello����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sever_hello,item->valuestring);
			out("sever_hello = %s\n",sever_hello);
		}	
	
		result = fun_client_auth_d(plate_num,sever_hello,client_auth,errinfo);
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "client_auth", client_auth);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclientauth�ɹ�\r\n", message_line->parm.random);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡclientauthʧ��\r\n", message_line->parm.random);
		}
	}
	
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

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
	return;
}

//����������read cert
static void analysis_request_read_cert(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1,len = 0;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	int i = 0;
	char plate_num[32] = {0x00},cert_data[1024*5] = {0x00};
	
	out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,read cert,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,read cert,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num,item->valuestring);
			out("plate_num = %s\n",plate_num);
		}	
	
		result = fun_get_cert_d(plate_num,cert_data,errinfo);
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "cert_data", cert_data);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,read cert�ɹ�\r\n", message_line->parm.random);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,read certʧ��\r\n", message_line->parm.random);
		}
	}
	
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

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
	return;
}


//����������ȫ���ȡcookie
static void analysis_request_qd_get_cookie(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1,len = 0;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	// char tmp_cookie[6*1024] = {0x00};
	// char etax_cookie[6*1024] = {0x00};
	// char tpass_cookie[6*1024] = {0x00};
	// char dppt_cookie[6*1024] = {0x00};
	char *etax_cookie = NULL;
	char *tpass_cookie = NULL;
	char *dppt_cookie = NULL;	

	char login_method[32] = {0x00};
	char area_code[32] = {0x00};

	char svr_container[32] = {0x00},soft_cert_ip[64] = {0x00},soft_cert_name[64] = {0x00};
	int soft_cert_port = 0;
	char *ptr_start = NULL,*ptr = NULL,*tmp_ptr = NULL;

	char soft_cert_url[128] = {0x00};

	char temp_data[1024]={0};
	
	strcpy(svr_container,"02");

	//out("����ȫ���¼����\n\n");
	out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "login_method");
			if (item == NULL)
			{
				strcpy(login_method,"02");
			}
			else
			{
				strcpy(login_method,item->valuestring);
			}
			out("��ȡ����login_methodΪ%s\n", login_method);
			if(0 == memcmp(login_method,"01",2))    //��֤���¼
			{
				item = cJSON_GetObjectItem(arrayItem, "svr_container");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����soft_cert_ip����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(svr_container,item->valuestring);
				out("��ȡ����svr_containerΪ%s\n", svr_container);
				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);
				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				// item = cJSON_GetObjectItem(arrayItem, "sys_type");
				// if (item == NULL)
				// {
				// 	sprintf(errinfo, "�����JSON����data�ڵ���sys_type���ݽ���ʧ��");
				// 	cJSON_Delete(root);
				// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				// 	goto End;
				// }
				// strcpy(sys_type, item->valuestring);
				// out("��ȡ����sys_typeΪ%s\n", sys_type);  //get_soft_cert_url
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_codeΪ%s\n", area_code);
				item = cJSON_GetObjectItem(arrayItem, "get_soft_cert_url");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں�����area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(soft_cert_url, item->valuestring);
				out("��ȡ����soft_cert_urlΪ%s\n", soft_cert_url);
				if(NULL == (strstr(soft_cert_url,":")) || NULL == (ptr_start = strstr(soft_cert_url,"://")))
				{
					sprintf(errinfo, "soft_cert_url error!");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				tmp_ptr = soft_cert_url;
				while(NULL != (tmp_ptr = strstr(tmp_ptr + 1,":")))
				{
					ptr = tmp_ptr;
				}
				
				memcpy(soft_cert_name,ptr_start + strlen("://"),ptr - ptr_start - strlen("://"));
				printf("lbc ============ soft_cert_name = %s\n",soft_cert_name);
				soft_cert_port = atoi(ptr + 1);
				printf("lbc ============ soft_cert_port = %d\n",soft_cert_port);
				result = get_DNS_ip(soft_cert_name,soft_cert_ip);
				printf("lbc ============ result = %d,soft_cert_ip = %s\n",result,soft_cert_ip);
			}
			else
			{
				item = cJSON_GetObjectItem(arrayItem, "usb_port");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				usb_port = atoi(item->valuestring);
				out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

				item = cJSON_GetObjectItem(arrayItem, "related_type");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(related_type, item->valuestring);
				out("��ȡ���ĵ�¼Ա����%s\n", related_type);

				item = cJSON_GetObjectItem(arrayItem, "plate_train");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				plate_train = atoi(item->valuestring);
				out("��ȡ����˰������Ϊ%d\n", plate_train);

				item = cJSON_GetObjectItem(arrayItem, "mobile");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(mobile, item->valuestring);
				out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


				item = cJSON_GetObjectItem(arrayItem, "idcard");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(idcard, item->valuestring);
				out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

				item = cJSON_GetObjectItem(arrayItem, "sz_password");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(sz_password, item->valuestring);
				out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

				item = cJSON_GetObjectItem(arrayItem, "realationtype");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				realationtype = atoi(item->valuestring);
				out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


				item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(uniqueIdentity, item->valuestring);
				out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


				item = cJSON_GetObjectItem(arrayItem, "realationStatus");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(realationStatus, item->valuestring);
				out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

				item = cJSON_GetObjectItem(arrayItem, "busid");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				strcpy(busid, item->valuestring);
				
				strcpy(inbusid,busid+2);
				out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookiesʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_get_cookie_Interface(usb_port,inbusid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&etax_cookie,&tpass_cookie,&dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			if(etax_cookie != NULL)
				cJSON_AddStringToObject(data_layer, "etax_cookie", etax_cookie);
			else 
				cJSON_AddStringToObject(data_layer, "etax_cookie", "");
			if(tpass_cookie != NULL)
				cJSON_AddStringToObject(data_layer, "tpass_cookie", tpass_cookie);
			else 
				cJSON_AddStringToObject(data_layer, "tpass_cookie", "");
			if(dppt_cookie != NULL)
				cJSON_AddStringToObject(data_layer, "dppt_cookie", dppt_cookie);
			else 
				cJSON_AddStringToObject(data_layer, "dppt_cookie", "");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookies�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���ȡcookiesʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(etax_cookie)
		free(etax_cookie);
	if(tpass_cookie)
		free(tpass_cookie);
	if(dppt_cookie)
		free(dppt_cookie);

	etax_cookie = NULL;
	tpass_cookie = NULL;
	dppt_cookie = NULL;

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
	return;
}


//����������ȫ�緢Ʊ�����ѯ
static void analysis_request_qdfpcxxq(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ�����ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_invoice_query_detail_Interface(usb_port,inbusid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				// asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ�����ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ�����ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}

//����������ȫ����˰�˻�����Ϣ
static void analysis_request_qd_get_nsrjcxx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_get_nsrjcxx_Interface(usb_port,inbusid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", "");
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", "");
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", "");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	printf("g_buf=%s\r\n",g_buf);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}



static void analysis_request_qd_get_nsrfxxx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;


	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_get_nsrfxxx_Interface(usb_port,inbusid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", "");
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", "");
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", "");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ����˰�˻�����Ϣ��ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	printf("g_buf=%s\r\n",g_buf);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;	
}

//����������ȫ�緢Ʊ����
static void analysis_request_qdfpcxxz(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;
	char *fpcxname = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ�緢Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ����ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_invoice_query_down_Interface(usb_port,inbusid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,&fpcxname,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			cJSON_AddStringToObject(data_layer, "fpcxname", fpcxname);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ���سɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ����ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

	if(fpcxname)
		free(fpcxname);
	fpcxname = NULL;
	
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
	return;
}


//����������ȫ�����Ŷ�Ȳ�ѯ
static void analysis_request_qdsxedcx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *sxeddata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char nsrsbh[32]={0};				//˰��

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ�����Ŷ�Ȳ�ѯ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ�����Ŷ�Ȳ�ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "nsrsbh");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���nsrsbh����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(nsrsbh, item->valuestring);
			out("��ȡ����˰��%s\n", nsrsbh);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�����Ŷ�Ȳ�ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_credit_limit_Interface(usb_port,inbusid,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&sxeddata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "sxeddata", sxeddata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�����Ŷ�Ȳ�ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�����Ŷ�Ȳ�ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(sxeddata)
		free(sxeddata);
	sxeddata = NULL;	
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
	return;
}

//����������ȫ��APPɨ����ɨ������
static void analysis_request_qdfpkjrpasl(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpkjdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char nsrsbh[32]={0};				//��˰��ʶ���
	char app_code[32]={0};
	char fpkjurl[128];				//�����ַ

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			
			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpkjurl���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpkjurl, item->valuestring);
			out("��ȡ���������ַΪ%s\n", fpkjurl);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);	


			item = cJSON_GetObjectItem(arrayItem, "nsrsbh");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���nsrsbh����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(nsrsbh, item->valuestring);
			out("��ȡ����nsrsbhΪ%s\n", nsrsbh);	

		
			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "app_code");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���app_code���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(app_code, item->valuestring);
			out("��ȡ����app_code����%s\n", app_code);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		

			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪��ɨ������ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_rpa_invoice_issuance_Interface(usb_port,inbusid,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "app_code", app_code);
			cJSON_AddStringToObject(data_layer, "fpkjdata", fpkjdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪��ɨ������ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪��ɨ������ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpkjdata)
		free(fpkjdata);
	fpkjdata = NULL;

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
	return;
}

//����������ȫ��������֤�ӿ�
static void analysis_request_qdfpkjrpasl_zt(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpkjdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char nsrsbh[32]={0};				//��˰��ʶ���
	char app_code[32]={0};
	char fpkjurl[128]={0};				//�����ַ

	char rzid[128]={0};

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//strcpy(svr_container,"02");

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	printf("##############################################\r\n");
	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			
			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpkjurl���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpkjurl, item->valuestring);
			out("��ȡ���������ַΪ%s\n", fpkjurl);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);	


			item = cJSON_GetObjectItem(arrayItem, "nsrsbh");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���nsrsbh����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(nsrsbh, item->valuestring);
			out("��ȡ����nsrsbhΪ%s\n", nsrsbh);	

		
			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "app_code");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���app_code���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(app_code, item->valuestring);
			out("��ȡ����app_code����%s\n", app_code);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		

			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);


			item = cJSON_GetObjectItem(arrayItem, "rzid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���rzid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(rzid, item->valuestring);
			out("��ȡ������֤ID=rzidΪ%s\n", rzid);	

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��������֤�ӿ�����ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_rpa_invoice_issuance_status_Interface(usb_port,inbusid,rzid,app_code,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "app_code", app_code);
			cJSON_AddStringToObject(data_layer, "fpkjdata", fpkjdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��������֤�ӿ�����ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��������֤�ӿ�����ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpkjdata)
		free(fpkjdata);
	fpkjdata = NULL;

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
	return;
}


//���ַ�Ʊ����
static void analysis_request_qdlzfpkj(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpkjout = NULL;
	char *hqnsrjcxx = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};

	char kjdata[1024 * 600 + 1] = {0};
	char *inv_json = NULL;

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);


			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);	


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		

			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}


			item = cJSON_GetObjectItem(arrayItem, "kjdata");
			if (get_json_value_can_not_null(item, (char *)kjdata, 0, 1024*600) < 0) 
			{
				sprintf(errinfo, "�����JSON����data�ڵ���kjdata����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
				goto End;
			}
			int inv_json_len = DF_MAX_INV_LEN;

			inv_json = (char *)malloc(DF_MAX_INV_LEN);
			if (inv_json == NULL)
			{
				sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
				goto End;
			}
			memset(inv_json, 0, DF_MAX_INV_LEN);
			uncompress_asc_base64((const uint8 *)kjdata, strlen((const char *)kjdata), (uint8 *)inv_json, &inv_json_len);
		}
	}

	char *inv_Encording = NULL;
	int inv_Encording_len = DF_MAX_INV_LEN;

	inv_Encording = (char *)malloc(DF_MAX_INV_LEN);
	if (inv_Encording == NULL)
	{
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	memset(inv_Encording, 0, DF_MAX_INV_LEN);
	utf82gbk((uint8 *)inv_json, (uint8 *)inv_Encording);

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ�����ַ�Ʊ����,��������,%s\r\n", message_line->parm.random,inv_Encording);

	if(inv_Encording)
		free(inv_Encording);
	inv_Encording = NULL;

	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪������ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_blue_ink_invoice_issuance_Interface(usb_port,inbusid,inv_json,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpkjout,&hqnsrjcxx,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpkjout", fpkjout);
			cJSON_AddStringToObject(data_layer, "hqnsrjcxx", hqnsrjcxx);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪������ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�翪������ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpkjout)
		free(fpkjout);
	fpkjout = NULL;

	if(hqnsrjcxx)
		free(hqnsrjcxx);
	hqnsrjcxx = NULL;

	if(inv_json)
		free(inv_json);
	inv_json = NULL;

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
	return;
}

//�����������ɿ���Ʊ����Ʊ��ѯ
static void analysis_request_qdhzqrxxszzhquery(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��ɿ���Ʊ����Ʊ��ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_hzqrxxSzzhQuery_Interface(usb_port,inbusid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��ɿ���Ʊ����Ʊ��ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ��ɿ���Ʊ����Ʊ��ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}

//����������ȫ�����ȷ�ϵ�����
static void analysis_request_hzqrxxsave(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char *inv_json = NULL;
	char kjdata[1024 * 600 + 1] = {0};

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			// item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			// if (item == NULL)
			// {
			// 	sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
			// 	cJSON_Delete(root);
			// 	mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			// 	goto End;
			// }
			// strcpy(fpcxsj, item->valuestring);
			// out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);

			

			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			item = cJSON_GetObjectItem(arrayItem, "kjdata");
			if (get_json_value_can_not_null(item, (char *)kjdata, 0, 1024*600) < 0) 
			{
				sprintf(errinfo, "�����JSON����data�ڵ���kjdata����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
				goto End;
			}
			int inv_json_len = DF_MAX_INV_LEN;

			inv_json = (char *)malloc(DF_MAX_INV_LEN);
			if (inv_json == NULL)
			{
				sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
				goto End;
			}
			memset(inv_json, 0, DF_MAX_INV_LEN);
			uncompress_asc_base64((const uint8 *)kjdata, strlen((const char *)kjdata), (uint8 *)inv_json, &inv_json_len);
			printf("inv_json = %s\n",inv_json);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}

	char *inv_Encording = NULL;
	int inv_Encording_len = DF_MAX_INV_LEN;

	inv_Encording = (char *)malloc(DF_MAX_INV_LEN);
	if (inv_Encording == NULL)
	{
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	memset(inv_Encording, 0, DF_MAX_INV_LEN);
	utf82gbk((uint8 *)inv_json, (uint8 *)inv_Encording);

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ����ַ�Ʊȷ�ϵ�����,��������,%s\r\n", message_line->parm.random,inv_Encording);
	
	if(inv_Encording)
		free(inv_Encording);
	inv_Encording = NULL;

	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ�����ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_hzqrxxSave_Interface(usb_port,inbusid,inv_json,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ����߳ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ�����ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

	

End:
	if(inv_json)
		free(inv_json);

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

//����������ȫ���Ʊȷ�ϵ��б���ѯ
static void analysis_request_hzqrxxquery(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	printf("plate_train = %d\n",plate_train);
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ��б���ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_hzqrxxQuery_Interface(usb_port,inbusid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			//printf("fpcxdata = %s\n",fpcxdata);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ��б���ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ��б���ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}

//����������ȫ���Ʊȷ�ϵ������ѯ
static void analysis_request_hzqrxxquerydetail(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fpcxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char full_name[32]={0};				//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[10*1024];				//��Ʊ��ѯ����
	char *inv_json = NULL;

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);
			int inv_json_len = DF_MAX_INV_LEN;

			inv_json = (char *)malloc(DF_MAX_INV_LEN);
			if (inv_json == NULL)
			{
				sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
				goto End;
			}
			memset(inv_json, 0, DF_MAX_INV_LEN);
			uncompress_asc_base64((const uint8 *)fpcxsj, strlen((const char *)fpcxsj), (uint8 *)inv_json, &inv_json_len);

			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ������ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_hzqrxxQueryDetail_Interface(usb_port,inbusid,inv_json,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fpcxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);		
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", fpcxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ������ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ���Ʊȷ�ϵ������ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fpcxdata)
		free(fpcxdata);
	fpcxdata = NULL;

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
	return;
}

//����������ȫ�緢Ʊ�������ѯ
static void analysis_request_qdhcstaquery(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *hcstaquerydata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(fpcxsj, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}
		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ������ѯʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_hcstaquery_Interface(usb_port,inbusid,fpcxsj,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&hcstaquerydata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);	
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fpcxdata", hcstaquerydata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
				//asc_compress_base64(new_etax_cookie,strlen(new_etax_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
				// asc_compress_base64(new_tpass_cookie,strlen(new_tpass_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
				// asc_compress_base64(new_dppt_cookie,strlen(new_dppt_cookie),tmp_cookie,&len);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ������ѯ�ɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�緢Ʊ������ѯʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	// printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);

	if(hcstaquerydata)
		free(hcstaquerydata);
	hcstaquerydata = NULL;

End:
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num > 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	return;
}

static void analysis_request_qdfjxxpeizhi(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *fjxxdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����
	char mode[32]={0};					//ģʽ


	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			int inv_json_len = 1024;
			uncompress_asc_base64((const uint8 *)item->valuestring, strlen((const char *)item->valuestring), (uint8 *)fpcxsj, &inv_json_len);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);

			item = cJSON_GetObjectItem(arrayItem, "mode");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���mode����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mode, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", mode);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}

		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�總����Ϣ����ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_fjxxpeizhi_Interface(usb_port,inbusid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&fjxxdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);	
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "fjxxdata", fjxxdata);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�總����Ϣ���óɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�總����Ϣ����ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(fjxxdata)
		free(fjxxdata);
	fjxxdata = NULL;

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
	return;	
}


static void analysis_request_qdcjmbpeizhi(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	struct _message_line *message_line;

	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[1024] = {0};
	char *cjmbdata = NULL;

	int i;
	int usb_port;					//������
	int plate_train;				//�Ƿ�Ϊ������(������û��֤�飬�ڻ�ȡ֤��ʱ��������ڴ�й©��������)
	int realationtype;				//�Ƿ���Ҫ�л���ҵ��ѯ 0����Ҫ 1��Ҫ
	char uniqueIdentity[32]={0};		//��Ҫ�л���ҵ��˰��
	char realationStatus[32]={0};		//��Ҫ�л���ҵ�ĵ�½����		
	char plate_num[32]={0};				//�̺�
	char related_type[32]={0};			//��¼Ա����
	char sz_password[32]={0};			//��¼Ա����
	char mobile[32]={0};				//��¼Ա�ֻ���
	char busid[32]={0};
	char inbusid[32]={0};
	char idcard[32]={0};
	char fpcxsj[1024]={0};				//��Ʊ��ѯ����
	char mode[32]={0};					//ģʽ

	char area_code[32] = {0x00};
	char tmp_cookie[6*1024] = {0x00};
	char etax_cookie[6*1024] = {0x00};
	char new_etax_cookie[6*1024] = {0x00};
	char tpass_cookie[6*1024] = {0x00};
	char new_tpass_cookie[6*1024] = {0x00};
	char dppt_cookie[6*1024] = {0x00};
	char new_dppt_cookie[6*1024] = {0x00};
	int use_cookie_flag = 0,len = 0;

	//out("����ȫ���¼����\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);

	//�������
	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_array;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);

	
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȫ���¼,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(item->valuestring);
			out("��ȡ���Ķ˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "plate_train");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���˰���������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			plate_train = atoi(item->valuestring);
			out("��ȡ����˰������Ϊ%d\n", plate_train);

			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(plate_num, item->valuestring);
			out("��ȡ�����̺�Ϊ%s\n", plate_num);			

			item = cJSON_GetObjectItem(arrayItem, "query_invoice_param");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���fpcxsj����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			int inv_json_len = 1024;
			uncompress_asc_base64((const uint8 *)item->valuestring, strlen((const char *)item->valuestring), (uint8 *)fpcxsj, &inv_json_len);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", fpcxsj);

			item = cJSON_GetObjectItem(arrayItem, "mode");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���mode����ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mode, item->valuestring);
			out("��ȡ���ķ�Ʊ��ѯ����%s\n", mode);


			item = cJSON_GetObjectItem(arrayItem, "related_type");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(related_type, item->valuestring);
			out("��ȡ���ĵ�¼Ա����%s\n", related_type);

			item = cJSON_GetObjectItem(arrayItem, "mobile");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�ֻ������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(mobile, item->valuestring);
			out("��ȡ���ĵ�¼ԱmobileΪ%s\n", mobile);		


			item = cJSON_GetObjectItem(arrayItem, "idcard");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա����֤���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(idcard, item->valuestring);
			out("��ȡ���ĵ�¼ԱidcardΪ%s\n", idcard);		

			item = cJSON_GetObjectItem(arrayItem, "sz_password");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��е�¼Ա�������ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(sz_password, item->valuestring);
			out("��ȡ���ĵ�¼Ա����Ϊ%s\n", sz_password);

			
			item = cJSON_GetObjectItem(arrayItem, "realationtype");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����л����ͽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			realationtype = atoi(item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationtypeΪ%d\n", realationtype);	


			item = cJSON_GetObjectItem(arrayItem, "uniqueIdentity");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ˰�Ž���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(uniqueIdentity, item->valuestring);
			out("��ȡ���ĵ�¼ԱuniqueIdentityΪ%s\n", uniqueIdentity);	


			item = cJSON_GetObjectItem(arrayItem, "realationStatus");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ�����ҵ�л����ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(realationStatus, item->valuestring);
			out("��ȡ���ĵ�¼ԱrealationStatusΪ%s\n", realationStatus);	

			item = cJSON_GetObjectItem(arrayItem, "busid");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿�busid���ݽ���ʧ��");
				cJSON_Delete(root);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			strcpy(busid, item->valuestring);
			
			strcpy(inbusid,busid + 2);
			out("��ȡ����busid:%s,inbusid:%s\n", busid,inbusid);

			//cookie �õ��Ĳ���
			item = cJSON_GetObjectItem(arrayItem, "use_cookie");
			if (item == NULL)
			{
				use_cookie_flag = 0;
			}
			else
			{
				use_cookie_flag = atoi(item->valuestring)>0?1:0;
			}
			if(use_cookie_flag)
			{
				item = cJSON_GetObjectItem(arrayItem, "etax_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(etax_cookie,0x00,sizeof(etax_cookie));
					strcpy(etax_cookie,tmp_cookie);
					//uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)etax_cookie, &len);
				}
				out("��ȡ����cookies = %s\n", etax_cookie);
				item = cJSON_GetObjectItem(arrayItem, "tpass_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					strcpy(tmp_cookie, item->valuestring);
					memset(tpass_cookie,0x00,sizeof(tpass_cookie));
					strcpy(tpass_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)tpass_cookie, &len);
				}
				out("��ȡ����tpass_cookie = %s\n", tpass_cookie);
				item = cJSON_GetObjectItem(arrayItem, "dppt_cookie");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���cookies����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				if(strlen(item->valuestring))
				{
					memset(tmp_cookie,0x00,sizeof(tmp_cookie));
					memset(dppt_cookie,0x00,sizeof(dppt_cookie));
					strcpy(tmp_cookie, item->valuestring);
					strcpy(dppt_cookie,tmp_cookie);
					// uncompress_asc_base64((const uint8 *)tmp_cookie, strlen((const char *)tmp_cookie), (uint8 *)dppt_cookie, &len);
				}
				out("��ȡ����dppt_cookie = %s\n", dppt_cookie);
				item = cJSON_GetObjectItem(arrayItem, "area_code");
				if (item == NULL)
				{
					sprintf(errinfo, "�����JSON����data�ڵ���area_code����ʧ��");
					cJSON_Delete(root);
					mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
					goto End;
				}
				memset(area_code,0x00,sizeof(area_code));
				strcpy(area_code, item->valuestring);
				out("��ȡ����area_code = %s\n", area_code);
			}

		}
	}
	if(plate_train == 1){				//�����̲�����
		char code[10] = {};
		sprintf(errinfo, "306��⵽����Ϊ������,��֧�ָýӿ�");
		cJSON_AddStringToObject(head_layer, "result", "f");
		cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		memcpy(code,errinfo,3);
		printf("code = %s",code);
		cJSON_AddStringToObject(data_layer, "code", code);
		cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�糡��ģ������ʧ��\r\n", message_line->parm.random,usb_port);
	}
	else 
	{
		result = mbi_etax_cjmbpeizhi_Interface(usb_port,inbusid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,&cjmbdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie_flag);	
		if(result == 0){
			cJSON_AddStringToObject(head_layer, "result", "s");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "cjmbdata", cjmbdata);

			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_etax_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_etax_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_etax_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_tpass_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_tpass_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_tpass_cookie", tmp_cookie);
			memset(tmp_cookie,0x00,sizeof(tmp_cookie));
			if(strlen(new_dppt_cookie))
			{
				len = sizeof(tmp_cookie);
				strcpy(tmp_cookie,new_dppt_cookie);
			}
			cJSON_AddStringToObject(data_layer, "new_dppt_cookie", tmp_cookie);

			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�糡��ģ�����óɹ�\r\n", message_line->parm.random,usb_port);
		}
		else{
			char code[10] = {};
			cJSON_AddStringToObject(head_layer, "result", "f");
			cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			memcpy(code,errinfo,3);
			printf("code = %s",code);
			cJSON_AddStringToObject(data_layer, "code", code);
			cJSON_AddStringToObject(data_layer, "errinfo", errinfo+3);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�˿ں�%d,ȫ�糡��ģ������ʧ��\r\n", message_line->parm.random,usb_port);
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf=%s\r\n",g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf); 
	cJSON_Delete(root);

	if(cjmbdata)
		free(cjmbdata);
	cjmbdata = NULL;

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
	return;	
}


//������������Ʊ���߶���
static void analysis_request_plate_fpkj(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char fpkpsj[1024 * 1024 + 1] = {0};
	char *inv_json;
	int inv_type = 0;
	char str_tmp[20] = { 0 };
#ifndef MTK_OPENWRT
	char kp_topic[200] = {0};
	char kp_plate_num[20] = {0};
#endif
	int sfsylsh;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};
	
	//out("���뷢Ʊ���߹���\n\n");
	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);
	

	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}
	//out("��ȡ���ķ�Ʊ���ʹ���Ϊ%d\n", inv_type);

	char use_compress[20] = { 0 };
	char test_only[20] = {0};

	item = cJSON_GetObjectItem(arrayItem, "use_compress");
	if (get_json_value_can_null(item, (char *)use_compress, 0, 10) < 0) {
		out("δ�����Ƿ�ʹ��ѹ������,Ĭ�ϲ�ѹ��\n");
	}
	if (atoi(use_compress) == 1)
	{
		//out("ʹ��ѹ���㷨\n");
		char compress_type[22] = {0};
		item = cJSON_GetObjectItem(arrayItem, "compress_type");
		if (get_json_value_can_null(item, (char *)compress_type, 0, 10) < 0) {
			out("δ����ѹ����ʽ,Ĭ��Ϊzlib\n");
			sprintf(compress_type,"zlib");
		}
		if (strcmp(compress_type, "zlib") != 0)
		{
			sprintf(errinfo, "�ݲ�֧��ѹ����ʽ��zlib��ʽ");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
			goto End;
		}
	}

	item = cJSON_GetObjectItem(arrayItem, "test_only");
	if (get_json_value_can_null(item, (char *)test_only, 0, 10) < 0) {
		out("δ��������Կ���,Ĭ����������\n");
	}



	item = cJSON_GetObjectItem(arrayItem, "fpkpsj");
	if (get_json_value_can_not_null(item, (char *)fpkpsj, 0, 1024*1024) < 0) 
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fpkpsj����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}

	
	//out("��ȡ���ķ�Ʊ����base64����Ϊ%s\n", fpkpsj);

	//out("base64����\n");
	inv_json = (char *)malloc(DF_MAX_INV_LEN);
	if (inv_json == NULL)
	{
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	memset(inv_json, 0, DF_MAX_INV_LEN);

	if (atoi(use_compress) == 0)
	{
		decode(fpkpsj, strlen((const char *)fpkpsj), inv_json);
	}
	else
	{
		int inv_json_len = DF_MAX_INV_LEN;
		uncompress_asc_base64((const uint8 *)fpkpsj, strlen((const char *)fpkpsj), (uint8 *)inv_json, &inv_json_len);
	}

	struct _make_invoice_result make_invoice_result;
	memset(&make_invoice_result, 0, sizeof(struct _make_invoice_result));

	if (atoi(test_only) == 1)
	{
		make_invoice_result.test_only = 1;
	}

	//out("�ж��Ƿ�ʹ�÷�Ʊ��ˮ��\n");
	analysis_invoice_sfsylsh(inv_json, &make_invoice_result);
	sfsylsh = make_invoice_result.sfsylsh;
	if ((sfsylsh == 1) && (atoi(use_compress) != 1))//����ߵư汾
	{
		//out("ʹ�÷�Ʊ������ˮ�ű�־\n");
#ifndef MTK_OPENWRT
		result = invoice_fpqqlsh_get_exsit(stream, kp_topic, kp_plate_num, &make_invoice_result);
		if (result >= 0)
		{
			sprintf(errinfo, "����ˮ�ţ�%s�ķ�Ʊ�Ѿ����߹�,�����ظ�����", make_invoice_result.fpqqlsh);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			mqtt_response_errno(DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_BEEN_USED, &message_line->parm, errinfo, DF_MQ_QOS2);
			cJSON_Delete(root);
			free(inv_json);
			goto End;
		}
		else if (result == -1)
		{
			sprintf(errinfo, "��Ʊ���ݿ����ʧ��,���������߷�Ʊ");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			mqtt_response_errno(DF_TAX_ERR_CODE_CONNECT_MYSQL_ERROR, &message_line->parm, errinfo, DF_MQ_QOS2);
			cJSON_Delete(root);
			free(inv_json);
			goto End;
		}
#endif
		result = invoice_fpqqlsh_get_using(stream, &make_invoice_result, message_line);
		if (result < 0)
		{
			sprintf(errinfo, "��ѯ����ǰ��ˮ��%s����ʹ����,���������߷�Ʊ", make_invoice_result.fpqqlsh);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS2);
			free(inv_json);
			goto End;
		}		
	}


	//printf("inv_json = %s\n", inv_json);
	//out("��ʼ��Ʊ�ײ㿪��\n");
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,���뷢Ʊ���߹���\r\n", message_line->parm.random, (char *)plate_num);
	result = fun_make_invoice_to_plate(plate_num, inv_type, (unsigned char *)inv_json, &make_invoice_result);
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,��Ʊ���߹��ܽ���\r\n", message_line->parm.random, (char *)plate_num);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, make_invoice_result.errinfo);
		free(inv_json);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, (char *)make_invoice_result.errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(inv_json);
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

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "fpqqlsh", (const char *)make_invoice_result.fpqqlsh);
	cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
	memset(str_tmp, 0, sizeof(str_tmp));	sprintf(str_tmp, "%03d", make_invoice_result.fplxdm);
	cJSON_AddStringToObject(data_layer, "fplxdm", str_tmp);
	cJSON_AddStringToObject(data_layer, "fpdm", (const char *)make_invoice_result.fpdm);
	cJSON_AddStringToObject(data_layer, "fphm", (const char *)make_invoice_result.fphm);
	cJSON_AddStringToObject(data_layer, "kpsj", (const char *)make_invoice_result.kpsj);
	cJSON_AddStringToObject(data_layer, "hsjbz", (const char *)"0");
	cJSON_AddStringToObject(data_layer, "hjje", (const char *)make_invoice_result.hjje);
	cJSON_AddStringToObject(data_layer, "hjse", (const char *)make_invoice_result.hjse);
	cJSON_AddStringToObject(data_layer, "jshj", (const char *)make_invoice_result.jshj);
	cJSON_AddStringToObject(data_layer, "jym", (const char *)make_invoice_result.jym);
	cJSON_AddStringToObject(data_layer, "mwq", (const char *)make_invoice_result.mwq);
	cJSON_AddStringToObject(data_layer, "bz", (const char *)make_invoice_result.bz);
	cJSON_AddStringToObject(data_layer, "ofdurl", (const char *)make_invoice_result.ofdurl);
#ifndef DF_OLD_MSERVER
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	if (make_invoice_result.scfpsj != NULL)
	{
		char *scfpsj_zlib_base;
		int scfpsj_zlib_base_len = strlen((const char *)make_invoice_result.scfpsj) + 1024;
		scfpsj_zlib_base = (char *)malloc(scfpsj_zlib_base_len);
		memset(scfpsj_zlib_base, 0, scfpsj_zlib_base_len);
		asc_compress_base64((const unsigned char *)make_invoice_result.scfpsj, strlen(make_invoice_result.scfpsj), (unsigned char *)scfpsj_zlib_base, &scfpsj_zlib_base_len);
		cJSON_AddStringToObject(data_layer, "inv_data", (const char *)scfpsj_zlib_base);
	}
#else
	if (make_invoice_result.scfpsj != NULL)
		cJSON_AddStringToObject(data_layer, "inv_data", (const char *)make_invoice_result.scfpsj);
#endif
	else
		cJSON_AddStringToObject(data_layer, "inv_data", "");
	char *g_buf;
	g_buf = cJSON_Print(json);
	//out("g_buf = %s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3, DF_MQ_QOS2);
	free(g_buf);

	if (1)
	{
		if (message_history.deal_num == MAX_HISTROY_DEAL_RESULT)//��ͷ����
			message_history.deal_num = 0;
		message_history.history_line[message_history.deal_num].state = 1;
		message_history.history_line[message_history.deal_num].result = 1;

#ifndef	DF_OLD_SERVER

#else
		cJSON_ReplaceItemInObject(data_layer, "inv_data", cJSON_CreateString(""));
		char *g_buf_g;
		g_buf_g = cJSON_Print(json);
		//out("�޸ĺ�ķ�Ʊ�������:%s\n", g_buf_g);
		char *auth_base;
		int auth_base_len = strlen(g_buf_g) + 1024;
		auth_base = (char *)malloc(auth_base_len);
		memset(auth_base, 0, auth_base_len);

		base64_enc((uint8 *)auth_base, (uint8 *)g_buf_g, strlen(g_buf_g));
		free(g_buf_g);
		if (strlen(auth_base) > sizeof(message_history.history_line[message_history.deal_num].inv_data))
		{
			out("�����������޷��洢\n");
		}
		else
		{
			strcpy(message_history.history_line[message_history.deal_num].inv_data, auth_base);
		}
		out("base64 ���������Ϊ��%d�ֽ�\n", strlen(auth_base));
		free(auth_base);
#endif


		strcpy(message_history.history_line[message_history.deal_num].random, message_line->parm.random);
		message_history.deal_num += 1;
	}



	if ((sfsylsh == 1) && (atoi(use_compress) != 1))
	{
		out("��Ʊ������ɣ�����Ʊ���ݴ������ݿ�\n");
#ifndef MTK_OPENWRT
		invoice_data_to_mysql(stream, &message_line->parm, (char *)plate_num, &make_invoice_result);
#endif
	}

	//�ͷ�ָ���ڴ�
	
	//out("��Ʊ������ɣ�Ӧ��Ʊ���\n");
	//update_invoice_to_m_server(stream, (char *)plate_num, &make_invoice_result);
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,��Ʊ��Ʊ�ɹ�,��Ʊ������룺%s_%s\r\n", message_line->parm.random, plate_num, make_invoice_result.fpdm, make_invoice_result.fphm);
	if (make_invoice_result.scfpsj != NULL)
		free(make_invoice_result.scfpsj);

	
	cJSON_Delete(root);
	cJSON_Delete(json);
	
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//out("������Ʊ�������\n");
	return;
}

//������������Ʊ���϶���
static void analysis_request_plate_fpzf(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	struct _message_line *message_line;
	unsigned char fpdm[20] = { 0 };
	unsigned char fphm[20] = { 0 };
	int zflx = 0;
	int zfzs = 0;
	unsigned char zfr[60] = { 0 };
	int inv_type = 0;
	char str_tmp[20] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);


	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}
	out("��ȡ���ķ�Ʊ���ʹ���Ϊ%d\n", inv_type);

	item = cJSON_GetObjectItem(arrayItem, "zflx");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���zflx����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		zflx = atoi(item->valuestring);
	}
	out("��ȡ������������Ϊ%d\n", zflx);
	if (zflx == 0)
	{
		item = cJSON_GetObjectItem(arrayItem, "zfzs");
		if (item == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ���zfzs����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
			goto End;
		}
		if (item->valuestring != NULL)
		{
			zfzs = atoi(item->valuestring);
		}
	}

	out("�ѿ���Ʊ�������ȡ��Ʊ����ͷ�Ʊ����\n");
	item = cJSON_GetObjectItem(arrayItem, "fpdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fpdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(fpdm, 0, sizeof(fpdm));
		memcpy(fpdm, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ���ķ�Ʊ����Ϊ%s\n", fpdm);


	item = cJSON_GetObjectItem(arrayItem, "fphm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fphm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(fphm, 0, sizeof(fphm));
		memcpy(fphm, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ���ķ�Ʊ����Ϊ%s\n", fphm);




	item = cJSON_GetObjectItem(arrayItem, "zfr");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���zfr����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS2);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(zfr, 0, sizeof(zfr));
		memcpy(zfr, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����������Ϊ%s\n", zfr);


	struct _cancel_invoice_result cancel_invoice_result;
	memset(&cancel_invoice_result, 0, sizeof(struct _cancel_invoice_result));

	cancel_invoice_result.fplxdm = inv_type;
	cancel_invoice_result.zfzs = zfzs;
	strcpy((char *)cancel_invoice_result.fpdm, (char *)fpdm);
	strcpy((char *)cancel_invoice_result.fphm, (char *)fphm);
	strcpy((char *)cancel_invoice_result.zfr, (char *)zfr);
	
	//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_CANCEL_INV_TO_PLATE, plate_num, inv_type, zflx, fpdm, fphm, zfr, &cancel_invoice_result);
	result = fun_cancel_invoice_to_plate(plate_num, inv_type, zflx, fpdm, fphm, zfr, &cancel_invoice_result);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, cancel_invoice_result.errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, (char *)cancel_invoice_result.errinfo, DF_MQ_QOS2);
		goto End;
	}

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

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
	memset(str_tmp, 0, sizeof(str_tmp));	sprintf(str_tmp, "%03d", cancel_invoice_result.fplxdm);
	if (zflx == 0)
	{
		char tmp[10];
		memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", zfzs);
		memset(cancel_invoice_result.fpdm, 0, sizeof(cancel_invoice_result.fpdm));
		memset(cancel_invoice_result.fphm, 0, sizeof(cancel_invoice_result.fphm));
		cJSON_AddStringToObject(data_layer, "zfzs", tmp);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "zfzs", "1");
	}
	cJSON_AddStringToObject(data_layer, "fplxdm", str_tmp);
	cJSON_AddStringToObject(data_layer, "fpdm", (const char *)cancel_invoice_result.fpdm);
	cJSON_AddStringToObject(data_layer, "fphm", (const char *)cancel_invoice_result.fphm);
	cJSON_AddStringToObject(data_layer, "zfrq", (const char *)cancel_invoice_result.zfsj);
	cJSON_AddStringToObject(data_layer, "jym", (const char *)cancel_invoice_result.jym);
	cJSON_AddStringToObject(data_layer, "mw", (const char *)cancel_invoice_result.mwq);
#ifndef DF_OLD_MSERVER
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");

	if (cancel_invoice_result.scfpsj != NULL)
	{
		char *scfpsj_zlib_base;
		int scfpsj_zlib_base_len = strlen((const char *)cancel_invoice_result.scfpsj) + 1024;
		scfpsj_zlib_base = (char *)malloc(scfpsj_zlib_base_len);
		memset(scfpsj_zlib_base, 0, scfpsj_zlib_base_len);
		asc_compress_base64((const unsigned char *)cancel_invoice_result.scfpsj, strlen(cancel_invoice_result.scfpsj), (unsigned char *)scfpsj_zlib_base, &scfpsj_zlib_base_len);
		free(cancel_invoice_result.scfpsj);
		cJSON_AddStringToObject(data_layer, "inv_data", (const char *)scfpsj_zlib_base);
	}
#else
	if (cancel_invoice_result.scfpsj != NULL)
	{
		cJSON_AddStringToObject(data_layer, "inv_data", (const char *)cancel_invoice_result.scfpsj);
		free(cancel_invoice_result.scfpsj);
	}
#endif
	else
		cJSON_AddStringToObject(data_layer, "inv_data", "");
	
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3, DF_MQ_QOS2);
	//update_cancel_invoice_to_m_server(stream, (char *)plate_num, &cancel_invoice_result);

	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺�:%s,��Ʊ���ϳɹ�,��Ʊ������룺%s_%s\r\n", message_line->parm.random, plate_num, cancel_invoice_result.fpdm, cancel_invoice_result.fphm);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ����,�̺ţ�%s,���\r\n", message_line->parm.random,(char *)plate_num);
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
//���������������忨
static void analysis_request_plate_cbqk(void *arg)
{

	struct _app     *stream;
	int size;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char tmp_s[20] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	struct _usb_port_infor port_info;
	uint8 ter_id[13] = { 0 };
	//char tmp_s[10];
	struct _switch_dev_id   id;
	int usb_sum;
	char errinfo[2048] = {0};
	//int unused_num;
	//cJSON *arrayItem, *item; //����ʹ��
	cJSON *port_array;		 //���ʹ��
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,%s\r\n", message_line->parm.random, errinfo);
		out("3Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);


	out("ִ���忨����ȡ�忨�������\n");
	result = fun_cb_qingka_get_usb_info(plate_num, &port_info);
	if (result<0)
	{
		sprintf(errinfo, "�ײ�ִ��ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	//out("Ҫ���ѯȫ��USB���ϵ�˰����Ϣ\n");
	usb_sum = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	//out("����������\n");       
	memset(&id, 0, sizeof(id));
	memset(ter_id, 0, sizeof(ter_id));
	result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	bcd2asc(id.id, ter_id, 6);
	if (result < 0)
	{
		out("get_sev_pro����������result = %d\n", result);
	}
	//out("�ն˻������Ϊ:%s USB�˿�����Ϊ��%d\n", ter_id, usb_sum);



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
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", (const char *)ter_id);
	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", usb_sum);
	cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
	cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());

	//out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");


		
	cJSON *port_layer;
	cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.port);
	cJSON_AddStringToObject(port_layer, "usb_port", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.usb_app);
	cJSON_AddStringToObject(port_layer, "ca_type", tmp_s);

	cJSON_AddStringToObject(port_layer, "ca_name", (char *)port_info.ca_name);
	cJSON_AddStringToObject(port_layer, "ca_number", (char *)port_info.ca_serial);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", port_info.ca_ok);
	cJSON_AddStringToObject(port_layer, "ca_support", tmp_s);

	fun_get_port_data_json(port_layer, port_info.port,1);


	

	char *g_buf;
	g_buf = cJSON_Print(json);
	//out("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����忨,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("���������忨���\n");
	return;
}
//�����������ϱ�Ƶ��
static void analysis_request_set_report_time(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	char report_time_s[10] = { 0 };
	int report_time = 0;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;

	cJSON *arrayItem, *item; //����ʹ��
	cJSON *dir1, *dir2;		 //���ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(dir1, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(dir1, "source_topic", stream->topic);
	cJSON_AddStringToObject(dir1, "random", message_line->parm.random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,%s\r\n", message_line->parm.random,errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "report_time");
		if (item == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ���report_time����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,%s\r\n", message_line->parm.random,errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		if (item->valuestring != NULL)
		{
			report_time = atoi(item->valuestring);
		}
		out("��ȡ�����ϱ�Ƶ��Ϊ%d��\n", report_time);

		

		//ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_SET_REPORT_TIME, report_time);		
		function_set_report_time(report_time);
		cJSON *dir3;
		cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
		memset(report_time_s, 0, sizeof(report_time_s));
		sprintf(report_time_s, "%d", report_time);
		cJSON_AddStringToObject(dir3, "report_time", (char *)report_time_s);


	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ϱ�Ƶ��,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("���������ϱ�Ƶ�����\n");
	return;
}
//����������˰��Ĭ��������֤
static void analysis_request_check_cert_passwd(void *arg)
{
	struct _app     *stream;
	int size;
	//int port_count;
	int i;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	unsigned char cert_passwd[20] = { 0 };
	struct _message_line *message_line;
	//uint8 ter_id[13];
	//char time[20];
	message_line = arg;
	stream = fd_stream;
	//struct _usb_port_infor port_info;
	//struct _switch_dev_id   id;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);


	//out("����������\n");       
	//memset(&id, 0, sizeof(id));
	//memset(ter_id, 0, sizeof(ter_id));
	//result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_ID, &id);
	//bcd2asc(id.id, ter_id, 6);
	//if (result < 0)
	//{
	//	out("get_sev_pro����������result = %d\n", result);
	//}
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
	//������ݽ���
	if (size == 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ����˰�̱��Ϊ%s\n", plate_num);



			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
			//memset(str_tmp, 0, sizeof(str_tmp));
			//sprintf(str_tmp, "%d", usb_sum);
			//cJSON_AddStringToObject(data_layer, "usb_port_num", str_tmp);
			//cJSON_AddItemToObject(data_layer, "usb_port_info", port_array = cJSON_CreateArray());

			out("��֤˰�̺�֤�������Ƿ���ȷ\n");
			result = fun_check_cert_passwd(plate_num, cert_passwd);
			if (result < 0)
			{
				out("ͨ���̺�Ĭ����֤֤������ʧ��\n");
				cJSON_AddStringToObject(data_layer, "cert_passwd", "????????");
			}
			else
			{
				cJSON_AddStringToObject(data_layer, "cert_passwd", (char *)cert_passwd);
			}

		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	

	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,˰��Ĭ��������֤,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("����Ĭ��������֤���\n");
	return;
}
//���ݷ�Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ
static void analysis_request_fpqqlsh_inv_data(void *arg)
{
	struct _app     *stream;
	int size;
	//int port_count;
	//int i;
#ifndef MTK_OPENWRT
	int result = -1;
#endif
	unsigned char fpqqlsh[100] = { 0 };
	//unsigned char cert_passwd[20];
	char kp_plate_num[20] = {0};
	char kp_topic[200] = {0};
	char str_tmp[20] = { 0 };
	struct _message_line *message_line;
	struct _make_invoice_result make_invoice_result;
	memset(&make_invoice_result, 0, sizeof(struct _make_invoice_result));
	message_line = arg;
	stream = fd_stream;
	//struct _usb_port_infor port_info;
	//struct _switch_dev_id   id;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);



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
	//������ݽ���
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		out("��Ҫ�����ˮ�Ż�ȡ�ѿ���Ʊ����\n");
		arrayItem = cJSON_GetArrayItem(object_data, 0);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "fpqqlsh");
		if (item == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ���fpqqlsh����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		if (item->valuestring != NULL)
		{
			memset(fpqqlsh, 0, sizeof(fpqqlsh));
			memcpy(fpqqlsh, item->valuestring, strlen(item->valuestring));
		}
		out("��ȡ���ķ�Ʊ������ˮ��Ϊ%s\n", fpqqlsh);
		memcpy(make_invoice_result.fpqqlsh, fpqqlsh, strlen((const char *)fpqqlsh));
#ifndef MTK_OPENWRT
		result = invoice_fpqqlsh_get_data(stream, kp_topic, kp_plate_num, &make_invoice_result);
		if (result < 0)
		{
			out("��ȡ���ķ�Ʊ������ˮ��Ϊ%sʧ�ܣ�result = %d\n", fpqqlsh,result);
			sprintf(errinfo, "���ݿ�δ��ѯ����ؼ�¼");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_INV_SERIAL_NUMBER_UNEXIST, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
#endif
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(data_layer, "fpqqlsh", (const char *)fpqqlsh);
		cJSON_AddStringToObject(data_layer, "kp_topic", (const char *)kp_topic);
		cJSON_AddStringToObject(data_layer, "plate_num", (const char *)kp_plate_num);
		memset(str_tmp, 0, sizeof(str_tmp));	sprintf(str_tmp, "%03d", make_invoice_result.fplxdm);
		cJSON_AddStringToObject(data_layer, "fplxdm", str_tmp);
		cJSON_AddStringToObject(data_layer, "fpdm", (const char *)make_invoice_result.fpdm);
		cJSON_AddStringToObject(data_layer, "fphm", (const char *)make_invoice_result.fphm);
		cJSON_AddStringToObject(data_layer, "kpsj", (const char *)make_invoice_result.kpsj);
		cJSON_AddStringToObject(data_layer, "hsjbz", (const char *)"0");
		cJSON_AddStringToObject(data_layer, "hjje", (const char *)make_invoice_result.hjje);
		cJSON_AddStringToObject(data_layer, "hjse", (const char *)make_invoice_result.hjse);
		cJSON_AddStringToObject(data_layer, "jshj", (const char *)make_invoice_result.jshj);
		cJSON_AddStringToObject(data_layer, "jym", (const char *)make_invoice_result.jym);
		cJSON_AddStringToObject(data_layer, "mwq", (const char *)make_invoice_result.mwq);	
		memset(str_tmp, 0, sizeof(str_tmp));	sprintf(str_tmp, "%d", make_invoice_result.fpdzsyh);
		cJSON_AddStringToObject(data_layer, "dzsyh", str_tmp);

	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);



End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ������ˮ�Ų�ѯ�ѿ���Ʊ��Ϣ,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("������ѯ˰��λ����Ϣ���\n");
	return;
}
//��������������Ʊ��������ϴ���Ʊ
static void analysis_request_afpdmhmscfp(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int dzsyh;
	int result = -1;
	char plate_num[13] = { 0 };
	char fpdm[20] = { 0 };
	char fphm[20] = { 0 };
	char *inv_data = NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	out("Ҫ���ѯȫ��USB���ϵ�˰��ʱ��\n");

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

	out("��Ҫ���˰�̱�Ų�ѯ˰����Ϣ\n");
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "fpdm");
		if (get_json_value_can_not_null(item, (char *)fpdm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���fpdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fpdm);

		item = cJSON_GetObjectItem(arrayItem, "fphm");
		if (get_json_value_can_not_null(item, (char *)fphm, 0, 20) < 0)
		{
			sprintf(errinfo, "�����JSON����data�ڵ���fphm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ����%s\n", fphm);

		char tmp_dzsyh[10] = { 0 };
		item = cJSON_GetObjectItem(arrayItem, "dzsyh");
		if (get_json_value_can_not_null(item, (char *)tmp_dzsyh, 0, 20) < 0) 
		{
			sprintf(errinfo, "��ƱJSON����data�е�ַ������ʧ��");
			dzsyh = 0;
		}
		else
		{
			dzsyh = atoi(tmp_dzsyh);
		}
		out("��ȡ���ĵ�ַ������%d\n", dzsyh);

		result = fun_update_inv_from_fpdm_fphm(plate_num, fpdm, fphm,dzsyh, &inv_data, errinfo);
		if (result < 0)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			//sprintf(errinfo, "ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ", fpdm, fphm);
			out("ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ\n", fpdm, fphm);
			if (inv_data)
			{
				out("ָ���ͷ�inv_data\n");
				free(inv_data);
				inv_data = NULL;
			}
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		if (inv_data)
		{
			out("�鷢Ʊ���ݰ�\n");
			cJSON *data_layer;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(data_layer, "invoice_data", (const char *)inv_data);
			out("ָ���ͷ�inv_data\n");
			free(inv_data);
			inv_data = NULL;
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����Ʊ��������ϴ���Ʊ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("������ѯ˰��˰��ʱ�����\n");
	return;
}



//����������У�����ַ�Ʊ���������Ϣ
static void analysis_request_jylzfpdmhm(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char plate_num[13] = { 0 };
	char fplxdm[20] = {0};
	char lzfpdm[20] = { 0 };
	char lzfphm[20] = { 0 };
	char dzsyh_s[20] = {0};
	int inv_type;
	//char *inv_data = NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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


	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (get_json_value_can_not_null(item, (char *)fplxdm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ���ʹ���%s\n", fplxdm);
		inv_type = atoi(fplxdm);


		item = cJSON_GetObjectItem(arrayItem, "lzfpdm");
		if (get_json_value_can_not_null(item, (char *)lzfpdm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���lzfpdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ�������ַ�Ʊ����%s\n", lzfpdm);

		item = cJSON_GetObjectItem(arrayItem, "lzfphm");
		if (get_json_value_can_not_null(item, (char *)lzfphm, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���lzfphm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ�������ַ�Ʊ����%s\n", lzfphm);


		item = cJSON_GetObjectItem(arrayItem, "dzsyh");
		if (get_json_value_can_not_null(item, (char *)dzsyh_s, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���dzsyh����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ĵ�ַ������%s\n", dzsyh_s);

		uint32 dzsyh = atoi(dzsyh_s);

		//fun_check_lzfpdm_lzfphm_allow(char *plate_num, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo)


		result = fun_check_lzfpdm_lzfphm_allow(plate_num, inv_type, lzfpdm, lzfphm, dzsyh, errinfo);
		if (result < 0)
		{
			//sprintf(errinfo, "ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ", fpdm, fphm);
			out("ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ\n", lzfpdm, lzfphm);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,У�����ַ�Ʊ���������Ϣ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
//����������ר�÷�Ʊ������Ϣ����Ż�ȡ
static void analysis_request_hqhzxxbbh(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char fpkpsj[1024 * 1024 + 1] = { 0 };
	char *inv_json = NULL;
	int inv_type = 0;
	char errinfo[2048] = {0};
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);


	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}
	//out("��ȡ���ķ�Ʊ���ʹ���Ϊ%d\n", inv_type);

	if ((inv_type != 4) && (inv_type != 28))
	{
		sprintf(errinfo, "��ר�÷�Ʊ�޷����������Ϣ�����");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char use_compress[20] = { 0 };

	item = cJSON_GetObjectItem(arrayItem, "use_compress");
	if (get_json_value_can_null(item, (char *)use_compress, 0, 10) < 0) {
		out("δ�����Ƿ�ʹ��ѹ������,Ĭ�ϲ�ѹ��\n");
	}
	if (atoi(use_compress) == 1)
	{
		//out("ʹ��ѹ���㷨\n");
		char compress_type[22] = { 0 };
		item = cJSON_GetObjectItem(arrayItem, "compress_type");
		if (get_json_value_can_null(item, (char *)compress_type, 0, 10) < 0) {
			out("δ����ѹ����ʽ,Ĭ��Ϊzlib\n");
			sprintf(compress_type, "zlib");
		}
		if (strcmp(compress_type, "zlib") != 0)
		{
			sprintf(errinfo, "�ݲ�֧��ѹ����ʽ��zlib��ʽ,��ǰѹ����ʽΪ%s", compress_type);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
	}


	item = cJSON_GetObjectItem(arrayItem, "fpkpsj");
	if (get_json_value_can_not_null(item, (char *)fpkpsj, 0, 1024 * 1024) < 0) 
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fpkpsj����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	//out("��ȡ���ķ�Ʊ����base64����Ϊ%s\n", fpkpsj);

	//out("base64����\n");
	inv_json = (char *)malloc(DF_MAX_INV_LEN);
	if (inv_json == NULL)
	{
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	memset(inv_json, 0, DF_MAX_INV_LEN);

	if (atoi(use_compress) == 0)
	{
		decode(fpkpsj, strlen((const char *)fpkpsj), inv_json);
	}
	else
	{
		int inv_json_len = DF_MAX_INV_LEN;
		uncompress_asc_base64((const uint8 *)fpkpsj, strlen((const char *)fpkpsj), (uint8 *)inv_json, &inv_json_len);
	}

	
	struct _askfor_tzdbh_result askfor_tzdbh_result;
	memset(&askfor_tzdbh_result, 0, sizeof(struct _askfor_tzdbh_result));

	//out("��ȡ���ķ�Ʊ����json����Ϊ%s\n", inv_json);
	result = fun_upload_hzxxb(plate_num,inv_type, inv_json, &askfor_tzdbh_result);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,�̺ţ�%s,�ײ������Ϣ:%s\r\n", message_line->parm.random, (char *)plate_num, askfor_tzdbh_result.errinfo);
		if (inv_json != NULL)
			free(inv_json);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, (char *)askfor_tzdbh_result.errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(inv_json);
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

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
	cJSON_AddStringToObject(data_layer, "lzfpdm", (const char *)askfor_tzdbh_result.lzfpdm);
	cJSON_AddStringToObject(data_layer, "lzfphm", (const char *)askfor_tzdbh_result.lzfphm);
	cJSON_AddStringToObject(data_layer, "tzdbh", (const char *)askfor_tzdbh_result.tzdbh);
	cJSON_AddStringToObject(data_layer, "sqlsh", (const char *)askfor_tzdbh_result.sqlsh);
	if (askfor_tzdbh_result.red_inv != NULL)
	{
		cJSON_AddStringToObject(data_layer, "red_inv", (const char *)askfor_tzdbh_result.red_inv);
		free(askfor_tzdbh_result.red_inv);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "red_inv", "");
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,˰�̱�ţ�%s,��Ʊ���룺%s,��Ʊ���룺%s,������Ϣ������ɹ�,��ţ�%s\r\n", message_line->parm.random, plate_num, askfor_tzdbh_result.lzfpdm, askfor_tzdbh_result.lzfphm, askfor_tzdbh_result.tzdbh);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���������Ϣ��,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//out("������Ʊ�������\n");
	return;
}

//����Ʊ�������������Ʊ������Ϣ��
static void analysis_request_afpdmhmhqhzxxb(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	int inv_type = 0;
	char fpdm[20] = {0};
	char fphm[20] = { 0 };
	char dzsyh[20] = { 0 };
	char errinfo[2048] = {0};
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);


	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}
	//out("��ȡ���ķ�Ʊ���ʹ���Ϊ%d\n", inv_type);

	if ((inv_type != 4) && (inv_type != 28))
	{
		sprintf(errinfo, "��ר�÷�Ʊ�޷����������Ϣ�����");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "fpdm");
	if (get_json_value_can_not_null(item, (char *)fpdm, 0, 20) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fpdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "fphm");
	if (get_json_value_can_not_null(item, (char *)fphm, 0, 20) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fphm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "dzsyh");
	if (get_json_value_can_not_null(item, (char *)dzsyh, 0, 20) < 0) 
	{
		sprintf(errinfo, "�����JSON����data�ڵ���dzsyh����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	struct _askfor_tzdbh_result askfor_tzdbh_result;
	memset(&askfor_tzdbh_result, 0, sizeof(struct _askfor_tzdbh_result));

	strcpy((char *)askfor_tzdbh_result.lzfpdm,fpdm);
	strcpy((char *)askfor_tzdbh_result.lzfphm, fphm);
	askfor_tzdbh_result.dzsyh = atoi(dzsyh);
	//out("��ȡ���ķ�Ʊ����json����Ϊ%s\n", inv_json);
	result = fun_upload_hzxxb(plate_num, inv_type, NULL, &askfor_tzdbh_result);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,�̺ţ�%s,�ײ������Ϣ:%s\r\n", message_line->parm.random, (char *)plate_num, askfor_tzdbh_result.errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, (char *)askfor_tzdbh_result.errinfo, DF_MQ_QOS0);
		goto End;
	}
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

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", (const char *)plate_num);
	cJSON_AddStringToObject(data_layer, "lzfpdm", (const char *)askfor_tzdbh_result.lzfpdm);
	cJSON_AddStringToObject(data_layer, "lzfphm", (const char *)askfor_tzdbh_result.lzfphm);
	cJSON_AddStringToObject(data_layer, "tzdbh", (const char *)askfor_tzdbh_result.tzdbh);
	cJSON_AddStringToObject(data_layer, "sqlsh", (const char *)askfor_tzdbh_result.sqlsh);
	if (askfor_tzdbh_result.red_inv != NULL)
	{
		cJSON_AddStringToObject(data_layer, "red_inv", (const char *)askfor_tzdbh_result.red_inv);
		free(askfor_tzdbh_result.red_inv);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "red_inv","");
	}

	char *g_buf;
	g_buf = cJSON_Print(json);

	//out("%s\n",g_buf);

	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,˰�̱�ţ�%s,��Ʊ���룺%s,��Ʊ���룺%s,������Ϣ������ɹ�,��ţ�%s\r\n", message_line->parm.random, plate_num, askfor_tzdbh_result.lzfpdm, askfor_tzdbh_result.lzfphm, askfor_tzdbh_result.tzdbh);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��Ʊ���������Ϣ��,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ
static void analysis_request_hqkeysqxx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	char key_info[4096] = {0};


	result = fun_auth_key_info(plate_num, key_info, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,�ײ������Ϣ��%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
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

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "key_enc_sign", (const char *)key_info);


	char *g_buf;
	g_buf = cJSON_Print(json);
	out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ�����̿�Ʊ����Ȩkey��Կ��Ϣ,���\r\n", message_line->parm.random);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//out("������Ʊ�������\n");
	return;
}

//������빫������ƽ̨
static void analysis_request_sqjrggfwpt(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	uint8 inv_type = 0;
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}



	result = fun_connect_pubservice(plate_num, inv_type, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
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




	char *g_buf;
	g_buf = cJSON_Print(json);
	out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨�ɹ�\r\n", message_line->parm.random);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������빫������ƽ̨,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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



//ȡ�����빫������ƽ̨
static void analysis_request_qxjrggfwpt(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	item = cJSON_GetObjectItem(arrayItem, "fplxdm");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	uint8 inv_type = 0;
	if (item->valuestring != NULL)
	{
		inv_type = atoi(item->valuestring);
	}



	result = fun_disconnect_pubservice(plate_num, inv_type, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
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

	


	char *g_buf;
	g_buf = cJSON_Print(json);
	out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨�ɹ�\r\n", message_line->parm.random);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//��ȡ������Ϣ����
static void analysis_request_hqhzxxsj(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	char month[20] = {0};
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	//out("��ȡ����˰�̱��Ϊ%s\n", plate_num);
	item = cJSON_GetObjectItem(arrayItem, "month");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���month����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(month, 0, sizeof(month));
		memcpy(month, item->valuestring, strlen(item->valuestring));
	}

	char *josn_data;

	result = fun_query_invoice_month_all_data(plate_num, month, &josn_data, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	
	

	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	if (josn_data != NULL)
	{
		cJSON_AddStringToObject(data_layer, "statistics_data", josn_data);
		free(josn_data);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "statistics_data", "");
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,ȡ�����빫������ƽ̨�ɹ�\r\n", message_line->parm.random);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ˰�̻�����Ϣ����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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


//��ȡ��ʷ��Ʊ��Ϣ
static void analysis_request_hqlsgpxx(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��

	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (item == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	if (item->valuestring != NULL)
	{
		memset(plate_num, 0, sizeof(plate_num));
		memcpy(plate_num, item->valuestring, strlen(item->valuestring));
	}
	

	char *josn_data;

	result = fun_query_history_invoice_coils(plate_num, &josn_data, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}




	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer, *data_layer;
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_RESPONSE);
	cJSON_AddStringToObject(head_layer, "cmd", message_line->parm.cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", stream->topic);
	cJSON_AddStringToObject(head_layer, "random", message_line->parm.random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	if (josn_data != NULL)
	{
		cJSON_AddStringToObject(data_layer, "history_data", josn_data);
		free(josn_data);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "history_data", "");
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	out("g_buf = %s\n", g_buf);



	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);
	//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ�ɹ�\r\n", message_line->parm.random);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ȡ��ʷ��Ʊ��Ϣ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
	struct _app     *stream;
	int size;
	int save_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
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
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}

			item = cJSON_GetObjectItem(arrayItem, "ter_save_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				//logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л�����ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			save_state = atoi(item->valuestring);
			out("��ȡ����������Դ״̬Ϊ%d\n", save_state);


		}
	}

	result = change_all_port_power_mode(save_state);
	if (result < 0)
	{
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_TER_SYSTEM_ERROR, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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
	struct _app     *stream;
	int size;
	int usb_port;
	int save_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
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
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}


			char usb_port_string[100] = { 0 };
			char save_state_string[100] = { 0 };
			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (get_json_value_can_not_null(item, usb_port_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(usb_port_string);
			out("��ȡ����USB�˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "save_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (get_json_value_can_not_null(item, save_state_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			save_state = atoi(save_state_string);
			out("��ȡ����USB�˿ںŵ�Դ״̬Ϊ%d\n", save_state);


		}
	}

	result = change_port_power_mode(usb_port, save_state);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�л��˿�ʡ��ģʽ,��%d��USB�˿�,�ײ������Ϣ��%s\r\n", message_line->parm.random, usb_port, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_PLATE_OPER_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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


//����������͸����ѡ��֤��������
static void analysis_request_tcgxrzfwsj(void *arg)
{
	struct _app     *stream;
	int size;
	int result;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	//int time_now;
	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	char plate_num[100] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (get_json_value_can_not_null(item, plate_num, 1, 12) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	//out("��ȡ����plate_numΪ%s\n", plate_num);

	char *http_data;
	int https_data_len = 1024 * 1024;
	http_data = calloc(1, https_data_len);

	item = cJSON_GetObjectItem(arrayItem, "http_data");
	if (get_json_value_can_not_null(item, http_data, 1, https_data_len) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���http_data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		free(http_data);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	//out("��ȡ����http_dataΪ%s\n", http_data);


	result = fun_zzsfpzhfwpt_auth_hello_data_tran(plate_num, http_data, https_data_len);
	if (result < 0)
	{
		out("result = %d\n", result);
		sprintf(errinfo, "http���ݴ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,%s\r\n", message_line->parm.random, errinfo);
		free(http_data);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", plate_num);
	cJSON_AddStringToObject(data_layer, "http_data", http_data);
	free(http_data);
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,͸����ѡ��֤��������,���\r\n", message_line->parm.random);
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

//��������������˰�����߷�Ʊ�ϴ�
static void analysis_request_kzlxfpsc(void *arg)
{
	struct _app     *stream;
	int size;
	//int result;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	//int time_now;
	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	char plate_num[100] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (get_json_value_can_not_null(item, plate_num, 1, 12) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	//out("��ȡ����plate_numΪ%s\n", plate_num);

	char offline_upload_s[10] = { 0 };
	int offline_upload;
	item = cJSON_GetObjectItem(arrayItem, "offline_upload");
	if (get_json_value_can_not_null(item, offline_upload_s, 1, sizeof(offline_upload_s)) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���offline_upload����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	offline_upload = atoi(offline_upload_s);
	//out("��ȡ����http_dataΪ%s\n", http_data);
	if ((offline_upload != 0) && (offline_upload != 1))
	{
		sprintf(errinfo, "�����JSON����data�ڵ���offline_upload����Ĳ�������");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	fun_stop_plate_upload_offline_invoice(plate_num, offline_upload, errinfo);



	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����˰�����߷�Ʊ�ϴ�,���\r\n", message_line->parm.random);
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
//����������֪ͨ�ն˶Ͽ�mqtt����
static void analysis_request_tzzddkmqtt(void *arg)
{
	struct _app     *stream;
	int size;
	//int result;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	//int time_now;
	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}


	char ter_id[100] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "ter_id");
	if (get_json_value_can_not_null(item, ter_id, 1, 12) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���ter_id����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	//out("��ȡ����plate_numΪ%s\n", plate_num);

	char mqtt_name[24] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "mqtt_name");
	if (get_json_value_can_not_null(item, mqtt_name, 1, sizeof(mqtt_name)) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���mqtt_name����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	
	if (strcmp(mqtt_name, "mqtt_fpkjxt") == 0)
	{
		_mqtt_client_disconnect();
	}
#ifndef DF_OLD_MSERVER
	else if(strcmp(mqtt_name, "mqtt_fptbxt") == 0)
	{
		_mqtt_invoice_disconnect();
	}
#endif
	else
	{
		sprintf(errinfo, "�����JSON����data�ڵ���mqtt_name��������");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,֪ͨ�ն˶Ͽ�mqtt����,���\r\n", message_line->parm.random);
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

//����������������Ϣ����ȡ��Ʊ
static void analysis_request_hzxxbhqfp(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char plate_num[13] = { 0 };
	char fplxdm[20] = { 0 };
	//char date_range[100] = { 0 };
	//char lzfphm[20] = { 0 };
	char rednum[20] = { 0 };
	//int inv_type;
	//char *inv_data = NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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


	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);



		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (get_json_value_can_not_null(item, (char *)fplxdm, 0, sizeof(fplxdm)) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ���ʹ���%s\n", fplxdm);


		item = cJSON_GetObjectItem(arrayItem, "tzdbh");
		if (get_json_value_can_not_null(item, (char *)rednum, 0, sizeof(rednum)) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���tzdbh����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ĺ�����Ϣ�����%s\n", rednum);

		//fun_check_lzfpdm_lzfphm_allow(char *plate_num, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh, char *errinfo)

		uint8 *inv_json = NULL;
		result = fun_get_redinv_online_from_rednum(plate_num, rednum,&inv_json, errinfo);
		if (result < 0)
		{
			//sprintf(errinfo, "ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ", fpdm, fphm);
			//out("ͨ����Ʊ����:%s,��Ʊ����:%sû�л�ȡ����Ʊ\n", lzfpdm, lzfphm);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

		cJSON_AddStringToObject(data_layer, "invoice_data", (const char *)inv_json);
		free(inv_json);

	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������֪ͨ����Ų�ѯ֪ͨ����Ϣ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
	_lock_set(stream->mqtt_lock);
	if (message_callback.task_num != 0)
		message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	out("������ѯ˰��˰��ʱ�����\n");
	return;
}




//֪ͨ�ն˲�ѯ���ϴ�ĳ�·�Ʊ��M����
static void analysis_request_tzzdscmyfp(void *arg)
{
	struct _app     *stream;
	int size;
	int result = -1;
	char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char year_month[20] = { 0 };
	char summary_month[20] = { 0 };
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	//int sfxqqd;
	//unsigned char sfxqqd_s[20] = { 0 };
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random, errinfo);

		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	item = cJSON_GetObjectItem(arrayItem, "plate_num");
	if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

	item = cJSON_GetObjectItem(arrayItem, "month");
	if (get_json_value_can_not_null(item, (char *)year_month, 0, 10) < 0)
	{
		sprintf(errinfo, "�����JSON����data�ڵ���month����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	out("��ȡ�����·�Ϊ%s\n", year_month);

	item = cJSON_GetObjectItem(arrayItem, "summary_month");
	if (item != NULL)
	{
		get_json_value_can_not_null(item, (char *)summary_month, 0, 10);
		out("��ȡ�����·�Ϊ%s\n", summary_month);
	}

	
	_lock_set(stream->inv_read_lock);
	//result = ty_ctl(stream->ty_usb_m_fd, DF_TY_USB_M_CM_GET_INV_DETAILS, plate_num, month, mqtt_pub_hook, message_line);
#ifndef DF_OLD_MSERVER
	result = fun_notice_plate_update_inv_to_mserver(plate_num, year_month, year_month, errinfo);
#else
	result = fun_notice_plate_update_inv_to_mserver(plate_num, year_month, summary_month, errinfo);
#endif
	_lock_un(stream->inv_read_lock);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		out("��Ʊ��ѯ����ʧ��\n");
		cJSON_Delete(root);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
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


	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(json);
	cJSON_Delete(root);
End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ϴ�ĳ�·�Ʊ��M����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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
#ifdef NEW_USB_SHARE
//�ر�USB�˿�
static void analysis_request_closeport(void *arg)
{

	struct _app     *stream;
	char sendbuf[1024] = { 0 };
	struct _message_line *message_line;
	char port_str[10] = { 0 };
	int result;
	message_line = arg;
	stream = fd_stream;
	char errinfo[2048] = {0};

	struct _new_usbshare_parm parm;
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
		out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
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
	int start, port;
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
	parm.port = port;
	port = port - start + 1;
	out("ǿ�ƹرն˿ڡ�%d��\n", port);
	result = close_port_dettach(port);
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

	out("ǿ�ƹرն˿ڳɹ�,�˿ںš�%d��\n", port);
	
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�ر�USB�˿�,�ɹ�,�˿ںţ�%d\r\n", message_line->parm.random, parm.port);

	prepare_mqtt_gw_close_packet(sendbuf, &parm);
	out("MQTT ���ر���Ϊ:%s\n", sendbuf);

	_mqtt_client_message_pub(parm.source_topic, sendbuf, 3, DF_MQ_QOS0);

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
//��USB�˿�
static void analysis_request_openport(void *arg)
{
	struct _app     *stream;
	struct _message_line *message_line;
	//struct mqtt_parm_newshare parm;
	char port_str[10] = { 0 };
	char trans_server[100] = { 0 };
	char client_id[200] = { 0 };
	int result;
	message_line = arg;
	stream = fd_stream;
	char errinfo[2048] = {0};
	cJSON *item;
	cJSON *root = cJSON_Parse(message_line->message);
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
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		out("���ݴ���,һ���ϱ����ܰ���һ̨������Ϣ\n");
		cJSON_Delete(root);
		goto End;
	}
	cJSON*arrayItem = cJSON_GetArrayItem(object_data, 0);
	if (arrayItem == NULL)
	{
		sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
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
	parm.port = atoi(port_str);

	memcpy(parm.source_topic, message_line->parm.source_topic, strlen(message_line->parm.source_topic));
	memcpy(parm.random, message_line->parm.random, strlen(message_line->parm.random));
	/*
	message_line->parm.data_lock = stream->data_lock;
	message_line->parm.port = atoi(port_str);
	memcpy(message_line->parm.trans_server, trans_server, strlen(trans_server));
	memcpy(message_line->parm.client_id, client_id, strlen(client_id));
	*/
	result = get_transport_line(&parm);
	if (result < 0)
	{
		sprintf(errinfo, "��������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,%s\r\n", message_line->parm.random, errinfo);
		mqtt_response_errno_gw(&parm, result);
		cJSON_Delete(root);
		goto End;
	}
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��USB�˿�,�ɹ�,�˿ںţ�%d,client ID��%s\r\n", message_line->parm.random, parm.port, parm.client_id);
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

//�����������޸�˰��֤�����
static void analysis_request_xgmrzsmmyz(void *arg)
{
	struct _app     *stream;
	int size;
	//int port_count;
	int i;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	char old_passwd[20] = { 0 };
	char new_passwd[20] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = { 0 };
	message_line = arg;
	stream = fd_stream;
	//struct _usb_port_infor port_info;
	//struct _switch_dev_id   id;
	cJSON *arrayItem, *item; //����ʹ��
	//cJSON *data_array;		 //���ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ����˰�̱��Ϊ%s\n", plate_num);

			item = cJSON_GetObjectItem(arrayItem, "old_passwd");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���old_passwd����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(old_passwd, 0, sizeof(old_passwd));
				memcpy(old_passwd, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ���ľɿ���Ϊ%s\n", old_passwd);

			item = cJSON_GetObjectItem(arrayItem, "new_passwd");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���new_passwd����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(new_passwd, 0, sizeof(new_passwd));
				memcpy(new_passwd, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ�����¿���Ϊ%s\n", new_passwd);


		}
	}
	
	result = fun_change_cert_passwd((char *)plate_num, old_passwd, new_passwd, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	printf("g_buf = %s\n", g_buf);


	out("MQTT��������\n");
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�޸�˰��֤�����,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//����������������Ϣ������
static void analysis_request_hzxxbhcx(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char plate_num[13] = { 0 };
	char fplxdm[20] = { 0 };
	char rednum[20] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	//cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());


	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);



		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (get_json_value_can_not_null(item, (char *)fplxdm, 0, sizeof(fplxdm)) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ���ʹ���%s\n", fplxdm);


		item = cJSON_GetObjectItem(arrayItem, "tzdbh");
		if (get_json_value_can_not_null(item, (char *)rednum, 0, sizeof(rednum)) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���tzdbh����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ĺ�����Ϣ�����%s\n", rednum);

		result = fun_cancel_hzxxb_online(plate_num, rednum, errinfo);
		if (result < 0)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	out("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,����������Ϣ��,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//����������������Ϣ����ʱ��β���
static void analysis_request_asjdcxhzxxb(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int rednum_count = 0;
	int result = -1;
	char plate_num[13] = { 0 };
	char fplxdm[20] = { 0 };
	//char date_range[100] = { 0 };
	//char lzfphm[20] = { 0 };
	char date_range[50] = { 0 };
	uint8 *s_data = NULL;
	int now_num;
	long index = 0;
	long invlen;
	char str_tmp[20] = { 0 };
	char gfsh[32] = {0};
	//int inv_type;
	//char *inv_data = NULL;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	//cJSON *data_array;		 //���ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����˰�̱��Ϊ%s\n", plate_num);



		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (get_json_value_can_not_null(item, (char *)fplxdm, 0, sizeof(fplxdm)) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�Ʊ���ʹ���%s\n", fplxdm);


		strcpy(gfsh,"");			//����˰��Ϊ��

		#ifndef DF_OLD_MSERVER
			item = cJSON_GetObjectItem(arrayItem, "gfsh");
			if (get_json_value_can_not_null(item, (char *)gfsh, 0, sizeof(gfsh)) < 0) {
				strcpy(gfsh,"");			//����˰��Ϊ��
				/*sprintf(errinfo, "�����JSON����data�ڵ���gfsh����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;*/
			}
			strcpy(gfsh, item->valuestring);
			out("��ȡ���Ĺ���˰��%s\n", gfsh);
		#endif		


		item = cJSON_GetObjectItem(arrayItem, "date_range");
		if (get_json_value_can_not_null(item, (char *)date_range, 0, sizeof(date_range)) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���date_range����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ĺ�����Ϣ����ѯʱ�䷶Χ%s\n", date_range);
		
		printf("gfsh=[%s]\r\n",gfsh);
		if (0 == strcmp(gfsh,""))
		{
			result = fun_query_rednum_from_date_range(plate_num, date_range, &s_data, &rednum_count,errinfo);
		}
		else
			result = fun_query_rednum_from_date_range_new(plate_num, date_range, gfsh,&s_data, &rednum_count,errinfo);
		

		// result = fun_query_rednum_from_date_range(plate_num, date_range, &s_data, &rednum_count,errinfo);
		if (result < 0)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			if (s_data != NULL)
				free(s_data);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		if (s_data == NULL)
		{
			sprintf(errinfo, "%sʱ�����δ��ѯ����Ч������Ϣ��", date_range);
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,�̺ţ�%s,%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			if (s_data != NULL)
				free(s_data);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}

		for (i = 0; i < rednum_count; i++)
		{
			cJSON *data_layer;
			char *inv_data = NULL;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			//cJSON_AddStringToObject(data_layer, "month", (char *)month);
			//cJSON_AddStringToObject(data_layer, "upload_type", "0");//��ѯ�ϴ�

			//cJSON_AddStringToObject(data_layer, "total_num", "0");//���ֶ�����Ч
			now_num = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("��ǰ��Ʊ���%d\n",now_num);
			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", now_num);
			cJSON_AddStringToObject(data_layer, "now_num", str_tmp);


			index += 4;
			invlen = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("��ǰ��Ʊ���ݳ���%d\n", invlen);
			index += 4;
			inv_data = (char *)malloc(invlen + 100);
			memset(inv_data, 0, invlen + 100);
			memcpy(inv_data, s_data + index, invlen);
			index += invlen;
			//out("��ǰ��Ʊ����%s\n", inv_data);
			cJSON_AddStringToObject(data_layer, "invoice_data", inv_data);

			free(inv_data);

			//out("�鷢ƱӦ������\n");
		}
		if (s_data != NULL)
			free(s_data);

		out("��ȡ��%d�����������ĺ�����Ϣ��\n", result);


	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("g_buf = %s\n", g_buf);
	//printf("MQTT��������%s\n", g_buf);
	//out("MQTT�������ݳ���%d\n", strlen(g_buf));
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ʱ��β�ѯ������Ϣ��,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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



//������������ѯ�ն���־�ļ�
static void analysis_request_cxzddqrzxx(void *arg)
{
	struct _app     *stream;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	char errinfo[2048] = {0};
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���־�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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

	out("ѹ��ǰ���ݳ���%d��ѹ�������ݳ���%d\n", log_len, strlen(log_zlib));
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
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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

//����������������άͨ��
static void analysis_request_kqzdywtd(void *arg)
{
	struct _app     *stream;
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
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "server_addr");
		if (get_json_value_can_not_null(item, (char *)server_addr, 0, 200) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_addr����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�������ַΪ%s\n", server_addr);

		item = cJSON_GetObjectItem(arrayItem, "server_port");
		if (get_json_value_can_not_null(item, (char *)server_port, 0, 6) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_port����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ������˿�Ϊ%s\n", server_port);

		item = cJSON_GetObjectItem(arrayItem, "server_user");
		if (get_json_value_can_not_null(item, (char *)server_user, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_user����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ������û���Ϊ%s\n", server_user);

		item = cJSON_GetObjectItem(arrayItem, "server_passwd");
		if (get_json_value_can_not_null(item, (char *)server_passwd, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���server_passwd����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ������û�����Ϊ%s\n", server_passwd);

		item = cJSON_GetObjectItem(arrayItem, "ssh_port");
		if (get_json_value_can_not_null(item, (char *)ssh_port, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���ssh_port����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ���ķ�������˿�Ϊ%s\n", ssh_port);

	
		out("�жϽ����Ƿ��Ѿ����������Ѿ��������Ƚ���\n");
		result = detect_process("\"ssh -CNfg -R\"", "tmpssh.txt");
		if (result > 0)
		{
			out("%s���̴���,�����\n", "\"ssh -CNfg -R\"");
			check_and_close_process("\"ssh -CNfg -R\"");
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
		_queue_add("ssh_tuunel", ssh_tunnel_start, NULL, 0);
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


		system("/etc/ssh_tunnel.sh");

		system("vsftpd /etc/vsftpd.conf-back &");
#endif



	}
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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
	out("����������������άͨ�����\n");
	return;
}




//��������������ά��ͨ��
static void analysis_request_jszdywtd(void *arg)
{
	struct _app     *stream;
	int result = -1;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	char errinfo[2048] = {0};

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,������άͨ��,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	out("�жϽ����Ƿ��Ѿ����������Ѿ��������Ƚ���\n");
	result = detect_process("ssh -CNfg -R", "tmpssh.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", "ssh -CNfg -R");
		check_and_close_process("ssh -CNfg -R");
	}

	system("killall vsftpd");
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
	printf("MQTT��������%s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);

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

//������������ѯ�ն���Ȩ�ļ�
static void analysis_request_cxzddqsqxx(void *arg)
{
	struct _app     *stream;
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	int result;
	char errinfo[1024];
	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *auth_data = NULL;


	result = fun_query_auth_file(&auth_data);
	if ((result < 0) || (auth_data == NULL))
	{
		sprintf(errinfo, "�ն���Ȩ��Ϣ�ļ���ȡʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��ѯ�ն���Ȩ�ļ�,�ײ������Ϣ��%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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

//������������Ʊ���ò�ѯ
static void analysis_request_asjdlyfpcx(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char date_range[200] = { 0 };
	char plate_num[20] = { 0 };
	char inv_coil_data[4096] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 200) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ�����̺�Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "date_range");
		if (get_json_value_can_not_null(item, (char *)date_range, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���date_range����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����ʱ�䷶ΧΪ%s\n", date_range);

		result = fun_query_net_inv_coils(plate_num, date_range, inv_coil_data, errinfo);
		if (result < 0)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
	}
	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(data_layer, "inv_coil_data", inv_coil_data);
	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ��ѯ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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


//������������Ʊ��������
static void analysis_request_asjdlyfpxz(void *arg)
{
	struct _app     *stream;
	int size;
	int i;
	int result = -1;
	char date_range[200] = { 0 };
	char plate_num[20] = { 0 };
	//char inv_coil_data[1024] = { 0 };
	uint8 fplxdm;
	char fpdm[20] = { 0 };
	char fpqshm[20] = { 0 };
	int fpzs;
	char tmp_s[20] = {0};

	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��
	char errinfo[2048] = { 0 };

	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random,errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	size = cJSON_GetArraySize(object_data);

	if (size != 1)
	{
		sprintf(errinfo, "�����JSON����data�ڵ�������ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		if (get_json_value_can_not_null(item, (char *)plate_num, 0, 200) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ�����̺�Ϊ%s\n", plate_num);

		item = cJSON_GetObjectItem(arrayItem, "date_range");
		if (get_json_value_can_not_null(item, (char *)date_range, 0, 20) < 0) 
		{
			sprintf(errinfo, "�����JSON����data�ڵ���date_range����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����ʱ�䷶ΧΪ%s\n", date_range);

		memset(tmp_s, 0, sizeof(tmp_s));
		item = cJSON_GetObjectItem(arrayItem, "fplxdm");
		if (get_json_value_can_not_null(item, (char *)tmp_s, 0, 10) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fplxdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		fplxdm = atoi(tmp_s);
		out("��ȡ����Ҫ���صķ�Ʊ���ʹ���Ϊ%03d\n", fplxdm);

		memset(tmp_s, 0, sizeof(tmp_s));
		item = cJSON_GetObjectItem(arrayItem, "fpzs");
		if (get_json_value_can_not_null(item, (char *)tmp_s, 0, 10) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fpzs����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		fpzs = atoi(tmp_s);
		out("��ȡ����Ҫ���صķ�Ʊ����Ϊ%d\n", fpzs);

		item = cJSON_GetObjectItem(arrayItem, "fpdm");
		if (get_json_value_can_not_null(item, (char *)fpdm, 0, 20) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fpdm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����fpdmΪ%s\n", fpdm);

		item = cJSON_GetObjectItem(arrayItem, "fpqshm");
		if (get_json_value_can_not_null(item, (char *)fpqshm, 0, 20) < 0) {
			sprintf(errinfo, "�����JSON����data�ڵ���fpqshm����ʧ��");
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,%s\r\n", message_line->parm.random, errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
		out("��ȡ����fpqshmΪ%s\n", fpqshm);

		result = fun_net_inv_coils_download(plate_num, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
		if (result < 0)
		{
			logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
			cJSON_Delete(root);
			cJSON_Delete(json);
			mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
			goto End;
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����÷�Ʊ���ذ�װ,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//������������������������
static void analysis_request_jcfwqlj(void *arg)
{
	struct _app     *stream;
	int size;
	char errinfo[2048] = { 0 };
	int i;
	int result = -1;
	unsigned char plate_num[13] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ����˰�̱��Ϊ%s\n", plate_num);
		}
	}
	char *splxxx = NULL;
	result = fun_test_plate_server_connect((char *)plate_num, &splxxx,errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,�̺ţ�%s,�ײ������Ϣ��%s\r\n", message_line->parm.random, (char *)plate_num,errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	cJSON *data_layer;
	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
	if (splxxx != NULL)
	{
		cJSON_AddStringToObject(data_layer, "splxxx", splxxx);
		free(splxxx);
	}
	else
		cJSON_AddStringToObject(data_layer, "splxxx", "");

	char *g_buf;
	g_buf = cJSON_Print(json);

	//out("gbug = %s\n", g_buf);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,��������������,�̺ţ�%s,���\r\n", message_line->parm.random, (char *)plate_num);
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

//���������������˿ڵ�Դ
static void analysis_request_adkhgldkdy(void *arg)
{
	struct _app     *stream;
	int size;
	int usb_port;
	int power_state;
	int i;
	int result = -1;
	char errinfo[1024] = { 0 };
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	else
	{
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}


			char usb_port_string[100] = {0};
			char power_state_string[100] = { 0 };
			item = cJSON_GetObjectItem(arrayItem, "usb_port");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (get_json_value_can_not_null(item, usb_port_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ��ж˿ں����ݽ���ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			usb_port = atoi(usb_port_string);
			out("��ȡ����USB�˿ں�Ϊ%d\n", usb_port);

			item = cJSON_GetObjectItem(arrayItem, "power_state");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (get_json_value_can_not_null(item, power_state_string, 1, 6) < 0)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���power_state����ʧ��2");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,%s\r\n", message_line->parm.random,errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			power_state = atoi(power_state_string);
			out("��ȡ����USB�˿ںŵ�Դ״̬Ϊ%d\n", power_state);


		}
	}
	
	result = fun_deal_usb_port_power(usb_port, power_state, errinfo);
	if (result < 0)
	{
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,��%d��USB�˿�,�ײ������Ϣ��%s\r\n", message_line->parm.random, usb_port,errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����˿ڵ�Դ,��%d��USB�˿�,���\r\n", message_line->parm.random, usb_port);
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



//���������������ն�
static void analysis_request_glzddy(void *arg)
{
	struct _app     *stream;
	char errinfo[2048] = {0};
	struct _message_line *message_line;
	message_line = arg;
	stream = fd_stream;
	//cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo, "�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,�����ն�,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
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



//�������������̺��������
static void analysis_request_asphqczshc(void *arg)
{
	struct _app     *stream;
	int size;
	//int port_count;
	int i;
	int result = -1;
	int mode = 0;
	unsigned char plate_num[13] = { 0 };
	struct _message_line *message_line;
	char errinfo[2048] = {0};
	message_line = arg;
	stream = fd_stream;
	cJSON *arrayItem, *item; //����ʹ��


	out("message_line->message = %s\n", message_line->message);
	out("cmd  = %s\n", message_line->parm.cmd);
	cJSON *root = cJSON_Parse(message_line->message);
	if (!root)
	{
		sprintf(errinfo,"�����JSON���ݽ���ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		free(message_line->message);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}
	free(message_line->message);
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		sprintf(errinfo, "�����JSON����data����ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,%s\r\n", message_line->parm.random, errinfo);
		cJSON_Delete(root);
		mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
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
	if (size == 0)
	{
		mode = 0;
	}
	else if (size == 1)
	{
		mode = 1;
		//out("��Ҫ���˰�̱�Ų�ѯ֤������\n");
		for (i = 0; i < size; i++)
		{
			//int time_now;
			arrayItem = cJSON_GetArrayItem(object_data, i);
			if (arrayItem == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			item = cJSON_GetObjectItem(arrayItem, "plate_num");
			if (item == NULL)
			{
				sprintf(errinfo, "�����JSON����data�ڵ���plate_num����ʧ��");
				logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,%s\r\n", message_line->parm.random, errinfo);
				cJSON_Delete(root);
				cJSON_Delete(json);
				mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
				goto End;
			}
			if (item->valuestring != NULL)
			{
				memset(plate_num, 0, sizeof(plate_num));
				memcpy(plate_num, item->valuestring, strlen(item->valuestring));
			}
			out("��ȡ����˰�̱��Ϊ%s\n", plate_num);
		}
	}

	result = fun_deal_clear_cert_err((char *)plate_num,mode);
	if (result < 0)
	{
		sprintf(errinfo,"���֤�������󻺴�ʧ��");
		logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,%s\r\n", message_line->parm.random,errinfo);
		cJSON_Delete(root);
		cJSON_Delete(json);
		mqtt_response_errno(result, &message_line->parm, errinfo, DF_MQ_QOS0);
		goto End;
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	_mqtt_client_message_pub(message_line->parm.source_topic, g_buf, 3,DF_MQ_QOS0);
	free(g_buf);
	cJSON_Delete(root);
	cJSON_Delete(json);

End:
	logout(INFO, "mqtt", message_line->parm.cmd, "��ˮ�ţ�%s,���̺����������Ϣ����,���\r\n", message_line->parm.random);
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

#endif
