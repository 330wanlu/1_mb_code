#include "_ty_usb_m.h"

int ty_usb_m_add(const char *usb_name,int dev_type)
{   struct _ty_usb_m  *stream;
	int result;
	stream=malloc(sizeof(struct _ty_usb_m));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
    //ty_usb_m_out("����:ty_usb_mģ��\n");
	memset(stream,0,sizeof(struct _ty_usb_m));
    memcpy(stream->ty_usb_name,usb_name,strlen(usb_name));
    //ty_usb_m_out("��ȡ����usbip_name[%s],usb_name[%s]\n",stream->usbip_name,stream->ty_usb_name);
	result = ty_file_add(DF_DEV_USB_M, stream, "ty_usb_m", (struct _file_fuc *)&ty_usb_m_fuc);
	if(result<0)
		free(stream);
	ty_usb_m_id = stream;
    //ty_usb_m_out("����:ty_usb_mģ��ɹ�\n");
	return result; 
}
int get_cert_json_file(uint8 *key_zlib_json, char **cert_json)
{
	int result = 0;
	char enc_cert[1024 * 1024] = { 0 };
	int enc_cert_len;
	//�жϼ�����Ȩ�ļ��Ƿ����
	result = access(AUTH_CERT_FILE_PATH, 0);
	if (result < 0)
	{
		ty_usb_m_out("��Ȩ�ļ�������,�޷�ʹ��˰����ع���\n");
		return -1;
	}
	//��ȡ��Ȩ�ļ�
	enc_cert_len = _t_file_c_read(AUTH_CERT_FILE_PATH, enc_cert, 1024 * 1024);
	if (enc_cert_len < 10)
	{
		ty_usb_m_out("��Ȩ�ļ���ȡʧ��,�ļ��쳣\n");
		return -2;
	}
	//����������Ȩ�ļ�
	int cert_len;
	result = mb_dec_file_function(key_zlib_json, enc_cert, enc_cert_len, cert_json, &cert_len);
	if (result != 0)
	{
		printf("lbc mb_dec_file_function failed result = %d\n",result);
		return result;
	}

	return result;
}
static int ty_usb_m_open(struct ty_file	*file)
{   struct _ty_usb_m  *stream;
    int j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    if(stream->state==0)
    {   //ty_usb_m_out("��:[%s]�ļ�\n",stream->ty_usb_name);
		//ty_usb_m_out("�״�������ȡ��˰�����ƽ���\n\n\n\n");

        stream->ty_usb_fd=ty_open(stream->ty_usb_name,0,0,NULL);
        if(stream->ty_usb_fd<0)
        {   ty_usb_m_out("���ļ�[%s]ʧ��:[%d]\n",stream->ty_usb_name,stream->ty_usb_fd);
            return stream->ty_usb_fd;
        }		
		stream->usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		//ty_usb_m_out("stream->usb_port_num = %d\n", stream->usb_port_numb);
		
		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);
		ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_MACHINE_ID, stream->ter_id);
		
		//��ȡ��Ƭ����-Կ��;
		
		
		int result;
		result = ty_ctl(stream->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, stream->key_s);
		if(result < 0)
		{
			ty_usb_m_out("����Ƭ����Կʧ��\n");
			stream->keyfile_load_err = 1;
		}
//#ifdef RELEASE_SO			
//		//ty_usb_m_out("��ȡ�������ݣ�%s", key_s+10);
//		//���ܶ�̬��
//		result = load_tax_lib_so(stream->key_s);
//		if (result < 0)
//		{
//			ty_usb_m_out("���ܶ�̬��ʧ��\n");
//			stream->lib_dec_err = 1;
//			stream->auth_dec_err = 1;
//			//return -100;
//		}
//		//��̬���ؿ�
//		if (stream->lib_dec_err != 1)
//		{
//			result = load_tax_lib_so_fun();
//			if (result < 0)
//			{
//				ty_usb_m_out("��̬���ؿ�ʧ��\n");
//				stream->lib_load_err= 1;
//				//return -101;
//			}
//		}
//		//��ʼ����̬��
//		if (stream->lib_dec_err != 1 && stream->lib_load_err != 1)
//		{
//			ty_usb_m_out("���ڳ�ʼ����̬��!\n");
//			result = so_common_init_tax_lib(stream->key_s, stream->tax_so_ver);
//			if (result <= 0)
//			{
//				ty_usb_m_out("��ʼ����̬��\n");
//				stream->auth_dec_err= 1;
//
//			}
//			//so_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
//		}
//		logout(INFO, "system", "init", "#################################��ȡ���ļ��ܿ�汾��Ϊ[%s]\r\n", stream->tax_so_ver);
//#else
//		result = function_common_init_tax_lib(stream->key_s, stream->tax_so_ver);
//
//		//function_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
//#endif
//		sprintf(stream->mbc_so_ver, "%s", MB_CLIB_VERSION);
//		ty_usb_m_out("��Ȩ�˿�������%d\n", result);
//        //ty_usb_m_out("��:[%s]�ļ�\n",stream->usbip_name);
//		//stream->usb_port_numb = 1;
//		
//
//		if (result <= 0)
//		{
//			lcd_print_info.tax_en = 1;
//		}
//		else
//		{
// #ifdef RELEASE_SO
// 			so_common_get_cert_json_file(stream->key_s,&stream->cert_json);
// #else
// 			function_common_get_cert_json_file(stream->key_s, &stream->cert_json);
// #endif
// 			//printf("cert json =%s\n", stream->cert_json);
// 			lcd_print_info.tax_en = 1;
// 		}
		get_cert_json_file(stream->key_s, &stream->cert_json);
		check_port_auth_info(stream);
		stream->lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
		ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
		
		stream->last_report_time_o = get_time_sec();//������
		stream->last_report_time_h = get_time_sec();//��̨����
		stream->last_report_time_s = get_time_sec();//������Ϣ
        //stream->usbip_fd=ty_open(stream->usbip_name,0,0,NULL);
        //if(stream->usbip_fd<0)
        //{   ty_usb_m_out("���ļ�[%s]ʧ��:[%d]\n",stream->usbip_name,stream->usbip_fd);
        //    ty_close(stream->ty_usb_fd);
        //    return stream->usbip_fd;
        //}
		stream->tran_fd=ty_open("/dev/tran_data",0,0,NULL);
		if(stream->tran_fd<0)
		{
			logout(INFO, "system", "init", "��ת��ģ��ʧ��\r\n");
			ty_usb_m_out("���ļ���/dev/tran_fdʧ��\n");
			return stream->tran_fd;
		}
		stream->deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		if (stream->deploy_fd<0)
		{
			logout(INFO, "system", "init", "������ģ��ʧ��\r\n");
			ty_usb_m_out("���ļ���/dev/deploy_fdʧ��\n");
			return stream->deploy_fd;
		}

		stream->event_file_fd = ty_open("/dev/event", 0, 0, NULL);
		if (stream->event_file_fd < 0)
		{
			logout(INFO, "system", "init", "���¼�ģ��ʧ��\r\n");
			ty_usb_m_out("���ļ���/dev/event_file_fdʧ��\n");
			return stream->event_file_fd;
		}
		stream->switch_fd = ty_open("/dev/switch", 0, 0, NULL);
		if (stream->switch_fd<0)
		{
			logout(INFO, "system", "init", "�򿪵�Ƭ������ģ��ʧ��\r\n");
			ty_usb_m_out("�򿪿����¼�ʧ��\n");
			return stream->switch_fd;
		}
		stream->get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
		
		ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_SUPPORT, &stream->dev_support);
		//check_stop_upload(&stream->stop_aisino_up, &stream->stop_nisec_up, &stream->stop_cntax_up, &stream->stop_mengbai_up);
		//ty_usb_m_out("��tran_data�ɹ�\n");
        stream->lock = _lock_open("_ty_usb_m",NULL);

		if (access("/tmp/passwd_err", 0) != 0)
		{
			system("mkdir /tmp/passwd_err");
		}

		for (j = 0; j < sizeof(stream->port) / sizeof(stream->port[0]); j++)
		{
			int left_num;
			stream->port[j].retry_forbidden = get_dev_passwd_err_port(j + 1, &left_num);
			stream->port[j].lock = _lock_open("s", NULL);
		}
        stream->timer_fd=timer_open();


		int read_power_mode = ty_ctl(stream->deploy_fd, DF_DEPLOY_CM_GET_TER_POW_MODE, &read_power_mode);

		if (read_power_mode == 1)//ǿ�Ʒǽڵ�汾
		{
			stream->dev_support.support_power = 0;
		}
		ty_usb_m_out("�����˿��ϵ�\n");
		all_port_power_open(stream);
		ty_usb_m_out("�����˿ڼ���߳�\n");
		if (stream->dev_support.support_power == 1)
		{
			for (j = 0; j < stream->usb_port_numb; j++)
			{		
				stream->port[j].power_mode = Saving_Mode;
			}
			_scheduled_task_open("ty_usb_m", task_check_port_ad_status, stream, 200, NULL);
			_scheduled_task_open("ty_usb_m", task_port_power_up_down, stream, 10, NULL);
			_scheduled_task_open("ty_usb_m", task_port_repair_ad_error, stream, 10, NULL);
			_scheduled_task_open("ty_usb_m", task_check_power_down_dev_remove, stream, 10, NULL);			
		}
		
		//ty_usb_m_out("�����߳�task_m\n");
		_scheduled_task_open("ty_usb_m",task_m,stream,400,NULL);	
		//ty_usb_m_out("�������˿��豸��ζ����߳�");

		_scheduled_task_open("ty_usb_m", task_check_port, stream, 30, NULL);      


		if (memcmp(stream->ter_id, "068180", 6) == 0) //��
		{
			//068180010960 - 068180021950
			//068180005925 - 068180005941
			int ter_id_num;
			char ter_id_num_str[200] = { 0 };
			memcpy(ter_id_num_str, stream->ter_id + 7, 4);
			ter_id_num = atoi(ter_id_num_str);
			ty_usb_m_out("����������Ϊ%d\n", ter_id_num);

			if (((ter_id_num >= 1096) && (ter_id_num <= 2195)) || ((ter_id_num >= 592) && (ter_id_num <= 594)) || ((ter_id_num >= 468) && (ter_id_num <= 470)))
			{
				_scheduled_task_open("ty_usb_m", task_check_port_repair_51err, stream, 1000, NULL);
			}

					
		}

		//ty_usb_m_out("�������ƹ����߳�\n");
		_scheduled_task_open("ty_usb_m", task_port_led_manage, stream, 500, NULL);
		
	#ifdef DF_SUPPORT_CA
		_scheduled_task_open("ty_usb_m", task_read_port_base_tax_info, stream, 10, NULL);				//��ȡCA֤��˰��
	#endif

		//�˿������ϱ�
		_scheduled_task_open("ty_usb_m", timely_tigger_report_status, stream, 100, NULL);
		_scheduled_task_open("ty_usb_m", timely_tigger_report_port_status, stream, 1, NULL);



		

		

    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
	return j+1;	   
}



static int ty_usb_m_ctl(void *data,int fd,int cm,va_list args)
{   struct _ty_usb_m        *stream;
    struct _ty_usb_m_fd     *id;
    int i;
    stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
	{   if(cm==ctl_fun[i].cm)
			return ctl_fun[i].ctl(id,args);
	}
	return DF_ERR_PORT_CORE_CM;       
}

static int ty_usb_m_close(void *data,int fd)
{   struct _ty_usb_m        *stream;
    struct _ty_usb_m_fd     *id;
    int j;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
	memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)
    {   ty_usb_m_out("Ҫ��ȫ���ر�\n");
        _lock_close(stream->lock);
        for(j=0;j<sizeof(stream->port)/sizeof(stream->port[0]);j++)
            _lock_close(stream->port[j].lock);
        timer_close(stream->timer_fd);
        ty_close(stream->ty_usb_fd);
        //ty_close(stream->usbip_fd);
    }
    return 0;   
}




static int all_port_power_open(struct _ty_usb_m  *stream)
{
	int i;
	int result;
	int port_number;
	struct _port_state   *state;
	int err_port = 0;
	port_number = stream->usb_port_numb;
	if (stream->dev_support.support_power == 1)//ʡ�����
	{
		//ǰ��ģ���Ѿ��ر������ظ��ص�
	}
	else
	{
		for (i = 1; i <= port_number; i++)
		{
			state = &stream->port[i - 1];
			//ty_usb_m_out("[ty_usb_m.c]���˿ںŴ򿪶˿� port = %d\n",i);
			//ǰ��ģ���Ѿ��ر������ظ��ص�
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, i);
			state->power = 1;
			if (result<0)
			{
				logout(INFO, "SYSTEM", "INIT", "֪ͨ��Ƭ���򿪶˿ڵ�Դʧ��,�˿ں�%d\r\n",i);
				err_port += 1;
				//ty_usb_m_out("task_open:�򿪶˿�ʧ��,�˿ں�%d\n", i);
				continue;
			}
		}
	}
	if (err_port == 0)
	{
		logout(INFO, "SYSTEM", "INIT", "����USB�˿ڳ�ʼ����Դ���\r\n");
	}
	else
	{
		logout(INFO, "SYSTEM", "INIT", "����USB�˿ڳ�ʼ�������д����쳣,�쳣����Ϊ%d��\r\n", err_port);
	}
	return 0;
}


static int check_port_auth_info(struct _ty_usb_m  *stream)
{
	//struct _port_state   *state;
	//int i;
	char auth[10] = { 0 };
	cJSON *arrayItem, *item;
	//int auth_count = 0;
	ty_usb_m_out("##############check_port_auth_info#################\n");
	if (stream->cert_json == NULL)
	{
		ty_usb_m_out("��Ȩ�ļ�Ϊ��\n");
		return -1;
	}

	char version[129] = { 0 };
	char serial_num[129] = { 0 };
	char name[129] = { 0 };

	ty_usb_m_out("��Ȩ�ļ�����:%s\n", stream->cert_json);

	cJSON *root = cJSON_Parse(stream->cert_json);
	if (!root)
	{
		ty_usb_m_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "version");
	if (get_json_value_can_not_null(item, version, 0, 128) < 0) {
		ty_usb_m_out("������Ȩ�ļ��汾��ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "serial_num");
	if (get_json_value_can_not_null(item, serial_num, 0, 128) < 0) {
		ty_usb_m_out("������Ȩ�ļ����к�ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "name");
	if (get_json_value_can_not_null(item, name, 0, 128) < 0) {
		ty_usb_m_out("������Ȩ�ļ�����ʧ��\n");
		goto End;
	}

	cJSON *object_share = cJSON_GetObjectItem(root, "usbshare");
	if (object_share == NULL)
	{
		ty_usb_m_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	int size = 0;
	size = cJSON_GetArraySize(object_share);
	if (size < 1)
	{
		ty_usb_m_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_share, 0);
	if (arrayItem == NULL)
	{
		ty_usb_m_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	memset(auth, 0, sizeof(auth));
	item = cJSON_GetObjectItem(arrayItem, "auth");
	if (get_json_value_can_not_null(item, auth, 0, 10) < 0) {
		ty_usb_m_out("����usbshare authʧ��\n");
		goto End;
	}
	ty_usb_m_out("��ȡ����usbshare auth Ϊ:%s\n", auth);
	if (memcmp(auth, "whole", 5) != 0)
	{
		cJSON_Delete(root);
		return -1;
	}
	char start_time[40] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "start_time");

	if (get_json_value_can_not_null(item, start_time, 0, 32) < 0) {
		ty_usb_m_out("����usbshare start_timeʧ��\n");
		goto End;
	}
	strcpy(stream->start_time, start_time);
	char end_time[40] = { 0 };

	item = cJSON_GetObjectItem(arrayItem, "end_time");

	if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
		ty_usb_m_out("����usbshare end_timeʧ��\n");
		goto End;
	}

	strcpy(stream->end_time, end_time);




	ty_usb_m_out("��ȡ����start_timeΪ:%s\n", stream->start_time);

	ty_usb_m_out("��ȡ����end_timeΪ:%s\n", stream->end_time);
End:
	cJSON_Delete(root);
	return 0;
}





static int free_h_plate(struct _port_state   *state)
{
	if (state->h_plate.need_free == 1)
	{
		//logout(INFO, "SYSTEM", "�ͷ�˰���ڴ�", "ԭ��%d�˿ڼ����µ�,�����رվ��\r\n", state->port);
		common_close_usb_device(state,&state->h_plate);
	}
	return 0;
}

/*
������:port_power_up
˵��:�����豸���ն˿ںź�˰���ͺ��ϵ�

*/

// static int port_power_up(void  *stream_t, int port, int sy_type)
static int port_power_up(struct _port_state   *state)
{
	int result;
	int port;
	char busid[32] = {0};
	int time = 0;
	int busnum = 0;
	int devnum = 0;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	
	port = state->port;

	if (port<1 || port>stream->usb_port_numb)
	{
		ty_usb_m_out("�˿ں�����\n");
		return POWER_ERR_NO_DEV;
	}

	if ((strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) != 0) && strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) != 0)
	{
		for (time = 0; time < 5000; time++)
		{
			if (state->port_init == 2)
				break;
			usleep(1000);
		}
		if (time == 5000)
		{
			ty_usb_m_out("%d�Ŷ˿�δ��ɳ�ʼ���ݲ�����ʹ��,state->port_init == %d,usedinfo = %s\n", port, state->port_init,state->port_info.used_info);
			return POWER_ERR_NO_DEV;
		}
	}

	get_usb_busid(port,busid);
	if ((state->power == 1) && (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port)==0))
	{
		busnum = get_busid_busnum(busid);
		devnum = get_busid_devnum(busid);
#ifndef DF_SUPPORT_CA
		state->h_plate.nBusID = get_busid_busnum(state->port_info.busid);
		state->h_plate.nDevID = get_busid_devnum(state->port_info.busid);
#endif
		printf("state->h_plate.nBusID=%x,busnum=%x  state->h_plate.nDevID=%x,devnum=%x\n",state->h_plate.nBusID,busnum,state->h_plate.nDevID,devnum);
		if ((state->h_plate.nBusID != busnum) || (state->h_plate.nDevID != devnum))
		{
			ty_usb_m_out("��⵽%d�Ŷ˿��Ѿ��ϵ�,��USBbus��Ϣ�������⣬�������µ��ϵ�\n",port);
			state->sys_reload = 1;//�ر��豸������Ϣ
			port_power_down(state);
		}
		else
		{
			//ty_usb_m_out("��⵽%d�Ŷ˿��Ѿ��ϵ�,ֱ�Ӽ���ʹ��\n",port);
			return 0;
		}				
	}
	if (stream->onlineport > DF_FULL_LOAD_NUM)
	{
		return POWER_ERR_FULL_LOAD;
	}
	//���жϽڵ����������
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
	if (result<0)
	{
		logout(INFO, "DEV", "�ϵ�", "�򿪶˿�ʧ��, �˿ں�%d\n", port);
		state->sys_reload = 0;//�ر��豸��������Ϣ
		port_power_down(state);//�豸�ѱ��γ�
		return POWER_ERR_NO_DEV;
	}
	state->power = 1;

	for (time = 0; time < 5000; time++)
	{
		busnum = get_busid_busnum(busid);
		devnum = get_busid_devnum(busid);
		if (((busnum == -1) || (devnum == -1)) || ((busnum == 0) || (devnum == 0)))
		{
			//logout(INFO, "port_lock", "�˿�����ʹ��", "USB�˿�%d,ԭ��¼��busnum = %d,devnum = %d,���������˿ڷ����������\r\n", state->port,state->h_plate.nBusID, state->h_plate.nDevID);
			usleep(1000);
			continue;
		}
		break;
	}
	if (time == 5000)
	{
		state->sys_reload = 0;//�ر��豸��������Ϣ
		ty_usb_m_out("��%d�˿ں��豸�ȴ�ö�ٳ�ʱ\n",port);
		port_power_down(state);
		return POWER_ERR_NO_DEV;
	}
	state->sys_reload = 0;//ϵͳ���ؽ���
	if ((state->port_info.usb_app == DEVICE_MENGBAI) || (state->port_info.usb_app == DEVICE_MENGBAI2))
		sleep(5);

	if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)//���ز��򿪾��
	{
		return 0;
	}
	if (strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) == 0)//��������Ϣ����Ҫ�򿪾��
	{
		return 0;
	}
	state->h_plate.nBusID = busnum;
	state->h_plate.nDevID = devnum;
	//ty_usb_m_out("��%d�˿ں�busnum = %04x devnum = %04x\n", port,state->h_plate.nBusID, state->h_plate.nDevID);
	state->h_plate.usb_type = state->port_info.usb_app;
	state->h_plate.need_check_passwd = 0;
	struct _plate_infos plate_infos;
	memset(&plate_infos, 0, sizeof(plate_infos));
	memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
	strcpy(state->h_plate.busid, state->port_info.busid);

// #ifdef RELEASE_SO
// 	result = so_common_get_basic_simple(&state->h_plate, &plate_infos);
// #else
// 	result = function_common_get_basic_simple(&state->h_plate, &plate_infos);
// #endif

// 	if (result < 0)
// 	{
// 		logout(INFO, "DEV", "�ϵ�", "�򿪶˿ھ��ʧ��, �˿ں�%d ,result:%d,use info:%s\n", port, result, state->port_info.used_info);
// 		state->sys_reload = 0;//�ر��豸��������Ϣ
// 		port_power_down(state);
// 		return POWER_ERR_NO_DEV;

// 	}
	return 0;


// 	struct _ty_usb_m  *stream;
// 	stream = ty_usb_m_id;
// 	int result;
// 	//long now_time = 0;
// 	struct _port_state   *state;

// 	state = &stream->port[port - 1];
// 	//logout(INFO, "DEV", "port_power_up", "�˿ں�%d-��ʼ�ϵ�:use info:%s\n", port,state->port_info.used_info);
// 	//ty_usb_m_out("�˿ں�%d-��ʼ�ϵ�:state->power_status = %d,use info:%s\n", port, state->power_status, state->port_info.used_info);
// 	state->power_used = 1;
// 	_lock_set(state->power_lock);
// 	if (state->power_status == 0)//���µ�
// 	{
// 		//if (powered_port_num >= MAX_POWERED_NUM)
// 		//{
// 		//	ty_usb_m_out("***********�Ѿ��ﵽ�������ͬʱ�ϵ�˿���!***********\n");
// 		//ty_usb_m_out("�˿ں�%d��Ҫ�ϵ�\n",port);
// 		_lock_un(state->power_lock);

// 		for (; state->last_oper == 3;)
// 		{
// 			//ty_usb_m_out("�˿ں�%d��Ҫ�ϵ�,�ϴ��ļ�ϵͳ��δ��ʧ,�ȴ�\n", port);//�˴�����������������
// 			usleep(200000);
// 			continue;
// 		}
// 		if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port) >= 0 && state->last_oper == 0)//�ϴζ���Ϊ�µ����ļ�ϵͳ����δ��ʧ
// 		{
// 			//ty_usb_m_out("�˿ں�%d��Ҫ�ϵ�,�ϴζ���Ϊ�µ����ļ�ϵͳ����δ��ʧ\n", port);
// 			int fail_num = 0;
// 			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
// 			usleep(2000);
// 			while (1)//�˴�������������
// 			{
				
// 				//ty_usb_m_out("***********���µ绹δ���ļ�ϵͳ����ʧ��***********\n");
// 				if (fail_num == 100)
// 				{
// 					result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
// 					if (result < 0)
// 					{
// 						logout(INFO, "DEV", "port_power_up", "%d�˿ڸ��µ绹δ���ļ�ϵͳ����ʧ,���ٴ��µ�ʧ��\n", port);
// 						state->power_used = 0;
// 						return -1;
// 					}
// 				}
// 				if (fail_num > 1200)
// 				{
// 					state->power_status = 0;
// 					state->status = 1;
// 					logout(INFO, "DEV", "port_power_up", "%d�˿ڸ��µ绹δ���ļ�ϵͳ����ʧ,���1200�κ�δ��ʧ-- using_info:%s  ad:%d \r\n", port, state->port_info.used_info, state->port_info.ad_status);

// 					//port_power_down_now(state);//??
// 					state->power_used = 0;
// 					return -1;
// 				}
// 				if ((ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port) < 0) && fail_num <= 1200)
// 				{
// 					logout(INFO, "DEV", "port_power_up", "%d�˿ڸ��µ绹δ���ļ�ϵͳ����ʧ,���%d�κ��Ѿ���ʧ-- using_info:%s  ad:%d \r\n", port, fail_num, state->port_info.used_info, state->port_info.ad_status);

// 					break;
// 				}
// 				fail_num++;
// 				usleep(20000);
// 			}

// 		}
// 		_lock_set(state->power_lock);
// 		state->power_status = 1;
// 		state->down_flag = 0;
// 		state->last_oper = 1;
// 		state->need_re_detect = 2;
// 		_lock_un(state->power_lock);
// 		//now_time = get_time_sec();
// 		/*if (now_time - state->last_down_power_time <= 1)
// 		{
// 			ty_usb_m_out("############�ϴ��µ綯��ʱ��̫�̣��ȴ����ϵ�:%ld - %ld!#############\n", now_time, state->last_down_power_time);
// 			usleep(2000 * 1);
// 		}*/
// 		usleep(2000);
// 		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
// 		if (result<0)
// 		{
			
// 			logout(INFO, "DEV", "port_power_up", "port_power_up_mb2001:�򿪶˿�ʧ��, �˿ں�%d\n", port);

// 			//port_power_down_now(state);

// 			state->need_re_detect = 0;
// 			state->power_used = 0;
// 			return -1;
// 		}
// 		if ((sy_type == DEVICE_MENGBAI) || (sy_type == DEVICE_MENGBAI2))
// 			sleep(8);
// 		int count = 0;
		
// 		ty_usb_m_out("���ϵ�[%d]�Ŷ˿ڣ���ʼ����Ƿ����ļ�ϵͳ�п��Բ鵽!\n", port);
// 		while (1)
// 		{

// 			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
// 			if (result < 0)
// 			{
// 				//ty_usb_m_out("port_power_up,�ļ�ϵͳ���ʧ��%d��--�˿ں�%d\n", count + 1, port);
// 				if (count == 2000)
// 				{

// 					logout(INFO, "DEV", "port_power_up", "���ϵ�[%d]�Ŷ˿ڣ��ļ�ϵͳ�м��ʧ��!\n", port);
// 					//port_power_down_now(state);

// 					state->need_re_detect = 0;
// 					state->power_used = 0;
// 					return POWER_ERR_NO_DEV;
// 				}
// 				usleep(20000);
// 				count++;
// 				continue;

// 			}
// 			state->need_re_detect = 0;
// 			break;

// 		}
// 		_lock_set(state->power_lock);
// 		state->power_status = 1;
// 		state->down_flag = 0;
// 		state->last_oper = 1;
// 		state->last_up_power_time = get_time_sec();
// 		_lock_un(state->power_lock);
// 		if ((sy_type != DEVICE_AISINO) && (sy_type != DEVICE_NISEC) && (sy_type != DEVICE_MENGBAI) && (sy_type != DEVICE_MENGBAI2) && 
// 			(sy_type != DEVICE_SKP_KEY) && (sy_type != DEVICE_CNTAX))
// 		{
// 			state->power_used = 0;
// 			return 0;
// 		}
// 		state = &stream->port[port - 1];
// 		//ty_usb_m_out("�򿪶˿���Ϣ:%s\n", state->port_info.used_info);
// 		if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)//���ز��򿪾��
// 		{
// 			state->h_plate.need_free = 0;
// 			state->power_used = 0;
// 			return 0;
// 		}
// 		if (strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) == 0)//��������Ϣ����Ҫ�򿪾��
// 		{
// 			state->h_plate.need_free = 1;
// 			state->power_used = 0;
// 			return 0;
// 		}
// 		int busnum = 0;
// 		int devnum = 0;
// 		int usb_type;
// 		busnum = get_busid_busnum(state->port_info.busid);
// 		devnum = get_busid_devnum(state->port_info.busid);
		
// 		if ((sy_type == DEVICE_MENGBAI) || (sy_type == DEVICE_MENGBAI2))
// 			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
// 		else
// 			usb_type = state->port_info.usb_app;
// 		state->h_plate.nBusID = busnum;
// 		state->h_plate.nDevID = devnum;
// 		state->h_plate.usb_type = usb_type;
// 		state->h_plate.need_check_passwd = 0;
// 		struct _plate_infos plate_infos;
// 		memset(&plate_infos, 0, sizeof(plate_infos));
// 		memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
// 		strcpy(state->h_plate.busid, state->port_info.busid);

// #ifdef RELEASE_SO
// 		result = so_common_get_basic_simple(&state->h_plate, &plate_infos);
// #else
// 		result = function_common_get_basic_simple(&state->h_plate, &plate_infos);
// #endif
// 		if (state->h_plate.nBusID == 0)
// 		{
// 			busnum = get_busid_busnum(state->port_info.busid);
// 			devnum = get_busid_devnum(state->port_info.busid);
// 			state->h_plate.nBusID = busnum;
// 			state->h_plate.nDevID = devnum;
// 			state->h_plate.usb_type = usb_type;
// 			state->h_plate.need_check_passwd = 0;
// 			strcpy(state->h_plate.busid, state->port_info.busid);
// 			logout(INFO, "SYSTEM", "port_power_up", "%d�˿��ϵ�1%s: busnum���� ������ :- busnum:%0d devnum:%0d busid:%s\r\n", state->port, state->port_info.used_info, state->h_plate.nBusID, state->h_plate.nDevID, state->h_plate.busid);

// 		}
// 		if (result < 0)
// 		{
// 			logout(INFO, "DEV", "port_power_up", "so_common_get_basic_simple:�򿪶˿�ʧ��, �˿ں�%d--result:%d--use info:%s\n", port, result, state->port_info.used_info);

// 			port_power_down_now(state,0);
// 			state->h_plate.need_free = 1;
// 			state->power_used = 0;
// 			return POWER_ERR_NO_DEV;
			
// 		}
// 		state->power_used = 0;
// 		state->h_plate.need_free = 1;
// 		return 0;
// 	}
// 	else//���ϵ�
// 	{
// 		_lock_un(state->power_lock);
// 		if ((sy_type != DEVICE_AISINO) && (sy_type != DEVICE_NISEC) && (sy_type != DEVICE_MENGBAI) && (sy_type != DEVICE_MENGBAI2) && 
// 			(sy_type != DEVICE_SKP_KEY) && (sy_type != DEVICE_CNTAX))//����˰�� 
// 		{

// 			if (state->need_re_detect != 1)//���������ط��ϵ�Ϊ����10S ���������¼���ļ�ϵͳ���Ƿ����
// 			{
// 				ty_usb_m_out("���������ط��ϵ�Ϊ����10S ���������¼���ļ�ϵͳ���Ƿ����\n");
// 				state->power_used = 0;
// 				return 0;
// 			}
// 			state->power_used = 0;
// 			return -1;
// 		}

// 		state = &stream->port[port - 1];
// 		if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)//���ز��򿪾��
// 		{
// 			ty_usb_m_out("���ϵ�˰�����豸���ز��򿪾��\n");
// 			state->h_plate.need_free = 0;
// 			_lock_set(state->power_lock);

// 			state->h_plate.nBusID = get_busid_busnum(state->port_info.busid);
// 			state->h_plate.nDevID = get_busid_devnum(state->port_info.busid);
// 			state->power_status = 1;
// 			state->down_flag = 0;
// 			state->last_up_power_time = get_time_sec();
// 			state->last_oper = 1;
// 			_lock_un(state->power_lock);

// 			state->h_plate.need_free = 0;
// 			state->power_used = 0;
// 			return 0;
// 		}

// 		if (strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) == 0)//��������Ϣ����Ҫ�򿪾��
// 		{
// 			ty_usb_m_out("���ϵ�˰�����豸��ȡ������Ϣ���򿪾��\n");
// 			state->h_plate.need_free = 0;
// 			_lock_set(state->power_lock);
// 			state->power_status = 1;
// 			state->down_flag = 0;
// 			state->last_up_power_time = get_time_sec();
// 			state->last_oper = 1;
// 			_lock_un(state->power_lock);
			
// 			state->h_plate.need_free = 1;
// 			state->power_used = 0;
// 			return 0;
// 		}
// 		_lock_set(state->power_lock);
// 		state->power_status = 1;
// 		state->down_flag = 0;
// 		state->last_up_power_time = get_time_sec();
// 		state->last_oper = 1;
// 		_lock_un(state->power_lock);


// 		state->h_plate.need_free = 1;
// 		state->power_used = 0;
// 		return 0;

// 	}
}

static void task_check_port_ad_status(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	int result;
	int r_len = 0;
	stream = arg;
	uint8 all_port_status[4096] = { 0 };
	struct _port_state   *state;
	int i;
	
	uint8 ad_status[4096] = {0};
	//sleep(3);
	//printf("task_check_down_power_port in\n");
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_AD_STATUS, ad_status);
	//ty_usb_m_out("adֵ\n");
	//printf_array(ad_status,600);
	if (result != 2 * stream->usb_port_numb)
	{
		ty_usb_m_out("��ȡ����ad���س��ȴ���:result =%d\n", result);
		return;
	}

	for (i = 0; i < stream->usb_port_numb; i++)
	{
		state = &stream->port[i];
		state->port_info.ad_status = ad_status[i*2]*256+ad_status[i*2+1];
	}
	//int down_port_num = 0;
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS, all_port_status);
	if (result < 0)
	{
		ty_usb_m_out("DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS err!\n");
		return;
	}
	r_len = result;


	_lock_set(stream->lock);
	memset(stream->ad_status, 0, sizeof(stream->ad_status));
	memcpy(stream->ad_status, ad_status, sizeof(ad_status));

	memset(stream->all_port_status, 0, sizeof(stream->all_port_status));
	memcpy(stream->all_port_status, all_port_status, sizeof(all_port_status));

	stream->all_port_status_len = r_len;
	_lock_un(stream->lock);

	//down_port_num = all_port_status[0] * 256 + all_port_status[1];
	//ty_usb_m_out("״ֵ̬\n");
	//printf_array(all_port_status,800);

	
	for (i = 0; i < stream->usb_port_numb; i++)
	{
		state = &stream->port[i];
		if (state->ad_repair_check != 1)
		{
			continue;
		}
		if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, i+1) == 0)
		{
			state->status = 1;
			state->power_mode = Normal_Mode;
			state->ad_repair_check = 0;
			state->ad_repair_down_time = get_time_sec();
			state->sys_reload = 0;
			logout(INFO, "SYSTEM", "AD_CHECK", "%d�Ŷ˿�ADֵΪ��%d,��Ƭ����Ϊû�в��豸,ʵ�ʼ�����豸,���˶˿ڸ�Ϊ�ǽڵ�ģʽ\r\n", i+1, state->port_info.ad_status);
		}
		else
		{
			long time_now = get_time_sec();
			if (time_now - state->ad_repair_up_time > 5)//�ϵ�ʱ�䳬��5�룬�ж�û���豸
			{
				state->ad_repair_check = 0;
				state->ad_repair_up_time = 0;
				state->ad_repair_down_time = get_time_sec();
				state->sys_reload = 0;
				//ty_usb_m_out("%d�Ŷ˿��ϵ糬ʱδö�ٹرն˿ڵ�Դ\n", i+1);
				port_power_down(state);
			}
		}
	}
}

static void port_power_down(struct _port_state   *state)//
{
	int result;
	struct _ty_usb_m *stream;	
	stream = ty_usb_m_id;
	if (state->power == 0)
	{
		return;
	}
	// if(state->need_up_power == 1)
	// {

	// 	return;
	// }
	state->need_down_power = 1;
	//ty_usb_m_out("port_power_down ִ�е�Ƭ���ر�%d�Ŷ˿ڵ�Դ\n",state->port);
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);//ִ���µ����
	if (result < 0)
	{
		logout(INFO, "DEV", "PORT_POWER", "port_power_down:��Ƭ��ִ���µ�ʧ��%d\n", state->port);
		usleep(20000);
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);
	}
	//wang 02 03 �ڵ�汾 ��ʱ̫�̻����ż���Զ˿��µ�δ������,֮ǰ�������ʱ�رսڵ��߼����������Ƿ��	
	//wang 02 02 ���Խ���ڵ�汾 ���е�������
	sleep(3);
	state->need_down_power = 0;
	if(state->need_up_power == 1)
	{
		state->need_up_power = 0;
	}
	state->power = 0;			
}

int get_powered_port_num(void)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int power_num=0;
	stream = ty_usb_m_id;
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port];
		if (state->power == 1)
			power_num += 1;
	}
	return power_num;
}

static void task_port_power_up_down(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	stream = arg;
	int r_len = 0;
	struct _port_state   *state;
	uint8 all_port_status[4096] = { 0 };
	uint8 ad_status[4096] = { 0 };
	int down_port = 0;
	int now_status = 0;
	int result;
	_lock_set(stream->lock);

	memset(ad_status, 0, sizeof(ad_status));
	memcpy(ad_status, stream->ad_status, sizeof(ad_status));

	memset(all_port_status, 0, sizeof(all_port_status));
	memcpy(all_port_status, stream->all_port_status, sizeof(all_port_status));

	r_len = stream->all_port_status_len;

	_lock_un(stream->lock);

	if (r_len == 0)
		return;
	uint8 *tmp = all_port_status;
	tmp += 2;

	//long time_now = get_time_sec();
	while (tmp - all_port_status < r_len)
	{
		down_port = *tmp * 256 + *(tmp + 1);
		tmp += 2;//��ǰ�˿�λƫ�ƹ�ȥ
		now_status = *tmp;
		tmp += 1;

		//ty_usb_m_out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//����ģʽ���������µ�˿ڼ��
		{
			continue;
		}
		if (now_status == 1)
		{
			state->status_counts = 0;
			if (state->status != 1)
			{
				state->status = 1;
			}
			if (state->sys_reload == 1)//�ѽ���ڵ�
			{
				continue;
			}
			//ty_usb_m_out("��Ƭ����֪%d�Ŷ˿����豸,��ǰ�豸����Ϊ%d,�ϵ�˿�����Ϊ%d\n", down_port, stream->onlineport, get_powered_port_num());
			if (state->ad_repair_check == 1) //���豸�˿����ϵ��������������豸�����⵽�ˣ�������ֹͣAD�޸����
			{
				state->ad_repair_check = 0;
				state->ad_repair_up_time = 0;
				state->ad_repair_down_time = get_time_sec();
				state->power_mode = Saving_Mode;
			}
			if (stream->onlineport > DF_FULL_LOAD_NUM)
			{
				ty_usb_m_out("��%d�Ŷ˿��ϵ磬����֧����������豸����\n", down_port);
				for (;;)
				{
					usleep(10000);
					if (stream->onlineport < DF_FULL_LOAD_NUM)
						break;
				}
			}
			if (get_powered_port_num() > DF_FULL_POWER_NUM)
			{
				ty_usb_m_out("��%d�Ŷ˿��ϵ磬����֧������ϵ�˿�����\n", down_port);
				for (;;)
				{
					usleep(10000);
					if (get_powered_port_num() < DF_FULL_POWER_NUM)
						break;
				}
			}
			if (state->power != 0)
			{
				//�˿ڵ�ǰΪ�ϵ�״̬���ȴ���
				//ty_usb_m_out("��%d�Ŷ˿��ϵ磬��ǰ�˿�Ϊ�ϵ�״̬\n", down_port);
				continue;
			}
			if ((state->power == 0) && (state->h_plate.need_free == 1))
			{
				//�˿�δ��ȫ�ͷ���ɣ��ȴ�
				ty_usb_m_out("��%d�Ŷ˿��ϵ磬��ǰ�˿�Ϊ�µ�״̬�Ҿ��δ���\n", down_port);
				continue;
			}
			ty_usb_m_out("��%d�Ŷ˿��ϵ磬port_init = %d, need_up_power = %d\n", down_port, state->port_init, state->need_up_power);
			if ((state->port_init == 0) && (state->need_up_power != 1) && (state->need_down_power!=1))
			{
				//ty_usb_m_out("��Ƭ����֪%d�Ŷ˿����豸,��ǰ�豸����Ϊ%d,�ϵ�˿�����Ϊ%d\n", down_port, stream->onlineport, get_powered_port_num());
				state->need_up_power = 1;//��ֹ���µ��̳߳�ͻ

				ty_usb_m_out("%d�Ŷ˿����豸��Ҫ��ȡ����˰��\n", down_port);
				//���жϽڵ����������
				result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, down_port);
				state->power = 1;
				if (result<0)
				{
					state->need_up_power = 0;//�����ϵ��У������ظ�����
					logout(INFO, "DEV", "�ϵ�", "�򿪶˿�ʧ��, �˿ں�%d\n", down_port);
					state->sys_reload = 0;//�ر��豸��������Ϣ
					port_power_down(state);//�豸�ѱ��γ�
					continue;
				}
				state->need_up_power = 1;
			}
		}
		else if (now_status == 2)
		{
			ty_usb_m_out("��Ƭ����֪%d�Ŷ˿ڶ�·\n", down_port);
			continue;
		}
	}
}

static void task_port_repair_ad_error(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	stream = arg;
	int r_len = 0;
	struct _port_state   *state;
	uint8 all_port_status[4096] = { 0 };
	uint8 ad_status[4096] = { 0 };
	int down_port = 0;
	int now_status = 0;
	int result;
	_lock_set(stream->lock);

	memset(ad_status, 0, sizeof(ad_status));
	memcpy(ad_status, stream->ad_status, sizeof(ad_status));

	memset(all_port_status, 0, sizeof(all_port_status));
	memcpy(all_port_status, stream->all_port_status, sizeof(all_port_status));

	r_len = stream->all_port_status_len;

	_lock_un(stream->lock);

	if (r_len == 0)
		return;
	uint8 *tmp = all_port_status;
	tmp += 2;

	//long time_now = get_time_sec();
	while (tmp - all_port_status < r_len)
	{
		down_port = *tmp * 256 + *(tmp + 1);
		tmp += 2;//��ǰ�˿�λƫ�ƹ�ȥ
		now_status = *tmp;
		tmp += 1;

		//ty_usb_m_out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//����ģʽ���������µ�˿ڼ��
		{
			continue;
		}
		
		if (now_status == 0)
		{
			long time_now = get_time_sec();
			if (time_now < 1656671934)//ʱ��δ����
			{
				continue;
			}
			if (state->ad_repair_down_time == 0)
			{
				state->ad_repair_down_time = time_now - 10;
				continue;
			}
			if (stream->onlineport > DF_FULL_LOAD_NUM)
			{
				for (;;)
				{
					usleep(10000);
					if (stream->onlineport < DF_FULL_LOAD_NUM)
						break;
				}
			}
			if (get_powered_port_num() >(DF_FULL_POWER_NUM / 2))
			{
				for (;;)
				{
					usleep(10000);
					if (get_powered_port_num() < DF_FULL_POWER_NUM / 2)
						break;
				}
			}
			if (state->power != 0)
			{
				//�˿ڵ�ǰΪ�ϵ�״̬���ȴ���
				continue;
			}
			if ((state->power == 0) && (state->h_plate.need_free == 1))
			{
				//�˿�δ��ȫ�ͷ���ɣ��ȴ�
				continue;
			}
			if (time_now - state->ad_repair_down_time > 15)
			{
				//logout(INFO, "SYSTEM", "AD_CHECK", "%d�Ŷ˿�ADֵΪ��%d,��Ƭ����Ϊû�в��豸,��ʼ�ϵ���\r\n", down_port, state->port_info.ad_status);
				//���жϽڵ����������
				result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, down_port);
				state->power = 1;
				if (result < 0)
				{
					logout(INFO, "DEV", "�ϵ�", "�򿪶˿�ʧ��, �˿ں�%d\n", down_port);
					state->sys_reload = 0;//�ر��豸��������Ϣ
					port_power_down(state);//�豸�ѱ��γ�
					continue;
				}
				state->ad_repair_check = 1;
				state->ad_repair_up_time = time_now;
			}
		}
	}
}

static void task_check_power_down_dev_remove(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	stream = arg;
	int r_len = 0;
	struct _port_state   *state;
	uint8 all_port_status[4096] = { 0 };
	uint8 ad_status[4096] = { 0 };
	int down_port = 0;
	int now_status = 0;
	//int result;
	_lock_set(stream->lock);

	memset(ad_status, 0, sizeof(ad_status));
	memcpy(ad_status, stream->ad_status, sizeof(ad_status));

	memset(all_port_status, 0, sizeof(all_port_status));
	memcpy(all_port_status, stream->all_port_status, sizeof(all_port_status));

	r_len = stream->all_port_status_len;

	_lock_un(stream->lock);

	if (r_len == 0)
		return;
	uint8 *tmp = all_port_status;
	tmp += 2;

	//long time_now = get_time_sec();
	while (tmp - all_port_status < r_len)
	{
		down_port = *tmp * 256 + *(tmp + 1);
		tmp += 2;//��ǰ�˿�λƫ�ƹ�ȥ
		now_status = *tmp;
		tmp += 1;

		//ty_usb_m_out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//����ģʽ���������µ�˿ڼ��
		{
			continue;
		}

		if (now_status == 0)
		{

			if ((state->status != 0))
			{
				if ((state->sys_reload == 1) || (state->port_init != 0))
				{
					if (state->status_counts < 2)//���ܴ����󱨣�����з�����
					{
						state->status_counts += 1;
						continue;
					}
					ty_usb_m_out("%d�Ŷ˿ڼ�⵽�ص�״̬�豸���γ�\n", state->port);
					state->status = 0;
					state->sys_reload = 0;
					port_power_down(state);
					state->port_init = 0;
					state->status_counts = 0;
				}
			}
			state->status = 0;			
		}
	}
}


//�˿ڼ�鴦���߳�
static void task_m(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	stream = arg;
	//stream->timer = timer;
	for (i = 0; i<sizeof(stream->port) / sizeof(stream->port[0]); i++)
	{
		state = &stream->port[i];
		check_port(state, stream);
		check_port_used_time(state);
	}
}

//�ȴ��¼���������������
//static void task_execute(void *arg,int timer)
//{	struct _ty_usb_m  *stream;
//	stream=arg;
//	jude_process_exsit_execute(stream);
//}

static void task_check_port_repair_51err(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int i;

	
	long now_time = get_time_sec();
	stream = arg;

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	if (net_status.nettime_ok != 1)
	{
		//ty_usb_m_out("��������δ����,�Ժ������ϱ��ն�״̬\n");
		return;
	}

	
	//ty_usb_m_out("���������������ɹ��ϵ�51��Ƭ�������账��\n");
	int port_num_array[] = { 25, 26, 59, 60 };

	for (i = 0; i < 4; i++)
	{
		port = port_num_array[i];
		state = &stream->port[port - 1];
		if (state->port_info.with_dev == 1)
		{
			state->last_without_time = 0;
			continue;
		}
		//ty_usb_m_out("��⵽��%d�Ŷ˿�û���豸,����Ϊ51�汾���쳣����,�ж϶ϵ�ʱ���Ƿ񳬹�3��\n", port);
		if (state->last_without_time == 0)
		{
			state->last_without_time = get_time_sec();
			continue;
		}
		//ty_usb_m_out("��ǰʱ�䣺%ld,�ϴ�����ʱ�䣺%ld\n", now_time , state->last_without_time);
		if (now_time - state->last_without_time > 3)
		{

			//ty_usb_m_out("��⵽��%d�Ŷ˿�û���豸,����Ϊ51�汾���쳣����,�ϵ�ʱ������3��,���¸��˿��ϵ�\n", port);
			//ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);

			port_power_down(state);
			usleep(20000);
			ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			state->power = 1;
			state->last_without_time = 0;
		}

	}
	
}

static int common_close_usb_device(struct _port_state   *state,struct HandlePlate *h_plate)
{

	int usb_type = h_plate->usb_type;

	if ((usb_type == DEVICE_MENGBAI)||(usb_type == DEVICE_NISEC) || (usb_type == DEVICE_AISINO) || (usb_type == DEVICE_CNTAX) || (usb_type == DEVICE_SKP_KEY))
	{
		ty_usb_m_out("�رվ���ͷ��ڴ�,˰�����豸,�˿ں�:%d\n", state->port);
		//usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, 0);
		if (h_plate->hUSB != NULL)
		{
			if (h_plate->hUSB->handle)
			{
				libusb_close(h_plate->hUSB->handle);
			}
			libusb_exit(h_plate->hUSB->context);
			_lock_set(state->lock);
			free(h_plate->hUSB);
			_lock_un(state->lock);
		}
		if (h_plate->hDev != NULL)
		{
			if (h_plate->hDev->pX509Cert != NULL) 
			{
				X509_free(h_plate->hDev->pX509Cert);
				h_plate->hDev->pX509Cert = NULL;
			}
			_lock_set(state->lock);
			free(h_plate->hDev);
			_lock_un(state->lock);
		}
	}
	else
	{
		return -1;
	}
	_lock_set(state->lock);
	h_plate->hUSB = NULL;
	h_plate->hDev = NULL;
	h_plate->nBusID = 0;
	h_plate->nDevID = 0;
	h_plate->usb_type = 0;
	h_plate->need_check_passwd = 0;
	h_plate->need_free = 0;
	memset(h_plate->busid, 0, sizeof(h_plate->busid));
	_lock_un(state->lock);
	//ty_usb_m_out("�رվ���ͷ��ڴ����,�˿ں�:%d\n", state->port);
	return 0;
}

//���˿ڲ���߳�
static void task_check_port(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	struct _usb_file_dev    dev;
	int port;
	int result;
	int port_number;
	//struct _net_status net_status;
	//memset(&net_status, 0, sizeof(struct _net_status));
	stream = arg;
	port_number = stream->usb_port_numb;
	//����̸߳�������
	ty_pd_update_task_status(DF_TASK_PORT_CHECK, get_time_sec());
	//���1-1�����Ƿ����
#ifndef MTK_OPENWRT
	if (access("/sys/bus/usb/devices/1-1", 0) != 0)
	{
		logout(INFO, "SYSTEM", "CHECK_PORT", "��⵽1-1���߶�ʧ,������������\r\n");
		sync();
		sleep(5);
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}
#endif
	int onlineport = 0;
	for (port = 1; port <= port_number; port++)
	{	
		state = &stream->port[port - 1];
		if (state->port != port)
			state->port = port;
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);		//���USB�ļ��Ƿ����
		if(result  == 0)
		{
			//wang 2022/11/24 Ŀǰ���ּ�����̨�ǽڵ�������ָú�������ֵ����0���������������ż����
			#ifdef DF_1908_0			//������̨���� �޷��ͷŶ˿�
			if((state->port_info.close_port_flag == 1))	
			{
				state->port_info.close_port_flag = 0;
				printf("port_info.with_dev = 1 && result = 0\r\n");
				result = -1;
			}
			#endif 
		}
		if (result<0)
		{
			//ty_usb_m_out("��⵽��%d�Ŷ˿�\n",port);
			if (state->sys_reload == 1)
			{ 
				//����ڵ�
				//ty_usb_m_out("state->sys_reload == 1\n");
				if (state->h_plate.need_free == 1)
				{
					//ty_usb_m_out("%d�Ŷ˿ڽ���ڵ�\n",port);
					if (state->port_info.port_used == 1)//�˿�ռ��
					{
						_lock_set(state->lock);
						if (state->h_plate.hDev != NULL)
						{
							if (state->h_plate.hDev->bBreakAllIO != 1)
								logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,��ǰʹ����ϢΪ��%s,Ҫ���ж�ʹ�ò��رն˿�\r\n", state->port_info.port_str, port, state->port_info.used_info);
							state->h_plate.hDev->bBreakAllIO = 1;
						}
						_lock_un(state->lock);
						// state->port_info.inv_query = 0;
						continue;
					}
					//ty_usb_m_out("%d�Ŷ˿ڽ���ڵ��ͷž��\n", port);
					free_h_plate(state);
				}
				continue;
			}
			if (state->port_info.with_dev == 1) //֮ǰ���豸����û����
			{
				//ty_usb_m_out("�µ������豸:%d---�Ƿ��Ѿ��ϵ�:%d\n", state->status, state->power);
				if (state->h_plate.hDev != NULL)
				{
					if (state->h_plate.hDev->bBreakAllIO != 1)
						logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,����ǰ�豸��Ϣ����,need_free = %d\r\n", state->port_info.port_str, port, state->h_plate.need_free);
				}
				//�˴����ͷ�˰���Ѵ򿪵ľ��
				if (state->h_plate.need_free == 1)
				{
					if (state->port_info.port_used == 1)//�˿�ռ��
					{
						_lock_set(state->lock);
						if (state->h_plate.hDev != NULL)
						{
							if (state->h_plate.hDev->bBreakAllIO != 1)
								logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,��ǰʹ����ϢΪ��%s\r\n", state->port_info.port_str, port, state->port_info.used_info);
							state->h_plate.hDev->bBreakAllIO = 1;
						}
						_lock_un(state->lock);
						// state->port_info.inv_query = 0;
						continue;
					}
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�����رվ��\n", state->port_info.port_str, port);
					free_h_plate(state);
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�رվ���ɹ�\n", state->port_info.port_str, port);
				}
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,��ǰʹ����ϢΪ��%s,����ڴ�\r\n", state->port_info.port_str, port, state->port_info.used_info);
				memset(&state->port_last_info, 0, sizeof(struct _port_last_info));				//_port_last_info:��һ��usb�豸���� ���������ϴ�
				state->port_last_info.usb_app = state->port_info.usb_app;
				memcpy(state->port_last_info.ca_name, state->port_info.ca_name, sizeof(state->port_info.ca_name));
				memcpy(state->port_last_info.ca_serial, state->port_info.ca_serial, sizeof(state->port_info.ca_serial));
				memcpy(state->port_last_info.sn, state->port_info.sn, sizeof(state->port_info.sn));
				state->port_last_info.extension = state->port_info.extension;
				state->port_last_info.vid = state->port_info.vid;
				state->port_last_info.pid = state->port_info.pid;
				state->off_report = 1;			//���������ϱ�
				state->last_act = 2;			//����
				state->sys_reload = 0;
				state->port_init = 0;
				state->ad_repair_check = 0;
				state->ad_repair_down_time = 0;	//���һ���µ�ʱ��
				state->ad_repair_up_time = 0;	//���һ���ϵ�ʱ��				
				state->need_up_power = 0;
				state->need_getinv = 0;			//���ϴ���Ʊͳ�Ʋ�ѯ
				state->status_counts = 0;

				if (stream->dev_support.support_power == 1)		//�ڵ�ģʽ
				{
					state->power_mode = Saving_Mode;			//0 ����ģʽ�˿�  1��ʡ��ģʽ�˿�
				}

				int ad_status = 0;

				if (state->port_info.ad_status != 0)			//ad״̬
				{
					_lock_set(state->lock);
					ad_status = state->port_info.ad_status;
					_lock_un(state->lock);
				}
				_lock_set(state->lock);			
				memset(&state->port_info, 0, sizeof(state->port_info));
				state->port_info.ad_status = ad_status;
				_lock_un(state->lock);
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���port_info\n", state->port_info.port_str, port);

				if ((state->power_mode == Saving_Mode) && (state->power == 1))//����ǽڵ�棬�ڶ˿�ͨ�����豸���γ�������˿��µ�
				{
					state->sys_reload = 0;
					ty_usb_m_out("%d�Ŷ˿ڿ��ŵ絫���豸����\n",state->port);
					port_power_down(state);
				}

			}
			continue;
		}
		//��⵽û���豸����
		onlineport += 1;//����ͨ���豸������1
		//����Ϊ�ļ�ϵͳ��⵽���豸
		state->port_info.with_dev = 1;			///��ʾ��⵽�豸��

		if (state->port_init == 0)//�ж��豸�Ƿ������仯,0�仯 1δ��,��ȡ���˿��豸��Ϣ�󽫴�״̬��Ϊ1,������ز��ٽ���˲���
		{
			ty_usb_m_out("%d�Ŷ˿��豸�����ϵ�,���ȡ������Ϣ\n", port);
			memset(state->port_info.busid, 0, sizeof(state->port_info.busid));
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, state->port_info.busid);
			if (result < 0)
			{
				ty_usb_m_out("��ȡ�豸busid��Ϣʧ��\n");
				//state->port_info.with_dev = 0;
				continue;
			}
			memset(&dev, 0, sizeof(dev));
			result = _usb_file_get_infor(state->port_info.busid, &dev);
			if (result < 0)
			{
				ty_usb_m_out("��ȡ�豸dev��Ϣʧ��\n");
				//state->port_info.with_dev = 0;
				continue;
			}
			state->port_info.subclass = dev.interface[0].bInterfaceSubClass;
			state->port_info.dev_type = dev.interface[0].bInterfaceClass;
			state->port_info.protocol_ver = 2;
			state->port_info.vid = dev.idVendor;
			state->port_info.pid = dev.idProduct;
			state->port_info.bnuminterface = dev.bNumInterfaces;

			state->insert_time = get_time_sec();

			
			if (vid_pid_jude_plate(state->port_info.vid, state->port_info.pid)>0)
			{
#ifdef DF_SUPPORT_CA
				state->port_init = 1;
				state->port_info.tax_read = 1;
				printf("state->port_info.tax_read = 1;\r\n");
#else
				if ((state->power_mode == Saving_Mode) && (state->power == 1) && (state->port_info.port_used == 0))//����ǽڵ�棬�ڶ˿�ͨ�����豸���γ�������˿��µ�
				{
					state->port_init = 0;
					state->sys_reload = 1;
					state->need_up_power = 0;//�����ϵ��У������ظ�����
					ty_usb_m_out("%d�Ŷ˿��ǽڵ��˿ڲ�ʶ���˰�����豸����ڵ繦��",state->port);
					port_power_down(state);
				}
#endif
			}
			else
			{
#ifdef DF_SUPPORT_CA
				state->port_init = 1;
				state->port_info.ca_read = 1;
#else
				if ((state->power_mode == Saving_Mode) && (state->power == 1) && (state->port_info.port_used == 0))//����ǽڵ�棬�ڶ˿�ͨ�����豸���γ�������˿��µ�
				{
					
					state->port_init = 0;
					state->sys_reload = 1;
					state->need_up_power = 0;//�����ϵ��У������ظ�����
					ty_usb_m_out("%d�Ŷ˿��ǽڵ��˿ڲ�ʶ���˰�����豸����ڵ繦��",state->port);
					port_power_down(state);
				}
#endif
			}

			ty_usb_m_out("��%d�ſڡ���⵽���豸����\n", port);
		}
	}

	stream->onlineport = onlineport;










// 	struct _ty_usb_m  *stream;
// 	struct _port_state   *state;
// 	struct _usb_file_dev    dev;
// 	int port;
// 	int result;
// 	int port_number;
// 	//struct _net_status net_status;
// 	//memset(&net_status, 0, sizeof(struct _net_status));
// 	stream = arg;
// 	port_number = stream->usb_port_numb;
// 	//����̸߳�������
// 	ty_pd_update_task_status(DF_TASK_PORT_CHECK, get_time_sec());
// 	//���1-1�����Ƿ����
// #ifndef MTK_OPENWRT
// 	if (access("/sys/bus/usb/devices/1-1", 0) != 0)
// 	{
// 		logout(INFO, "SYSTEM", "CHECK_PORT", "��⵽1-1���߶�ʧ,������������\r\n");
// 		sync();
// 		sleep(5);
// 		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
// 		sleep(10);
// 	}
// #endif

// 	for (port = 1; port <= port_number; port++)
// 	{	//ty_usb_m_out("����%d�Ŷ˿�\n",i);

// 		state = &stream->port[port - 1];

// 		state->port = port;
// 		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
// 		if (result<0 && state->sys_reload != 1)
// 		{
// 			if (state->power_mode == Saving_Mode)//ʡ��ģʽ���ļ�ϵͳ���豸
// 			{
// 				if (state->power_used == 1)
// 					continue;
// 				//�������ǰγ� 
// 				//|��·|���豸�����µ�
// 				if (state->status == 2	|| (state->status == 1 && state->power_status == 0))
// 				{
// 					if (state->status == 2)//�˿ڶ�·
// 					{

// 						if (state->h_plate.need_free == 1)
// 						{
// 							common_close_usb_device(state,&state->h_plate);
// 						}

// 					}
// 					state->sys_down_power = 0;
// 					continue;
// 				}
// 				else if (state->need_re_detect != 0 && state->power_status == 1)//�ϵ�������¼���ļ�ϵͳ
// 				{
// 					state->sys_down_power = 0;
// 					continue;
// 				}
// 				else if (state->status == 1 && state->open_circuit == 1)//���豸���Ƕ�·���ļ�ϵͳ��ⲻ��
// 				{
// 					if (state->h_plate.need_free == 1)
// 					{
// 						common_close_usb_device(state,&state->h_plate);
// 					}
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "ԭ��%d�˿����豸,�����µ�step2\n", port);
// 					port_power_down_now(state,0);//�����µ磬�´����¼��ö˿�
// 				}
// 			}//ʡ��ģʽ�������
// 			//ty_usb_m_out("����%d�Ŷ˿����豸\n", port);
// 			//ty_usb_m_out("=======��⵽%d�Ŷ˿�û���豸=======\n",port);
// 			if (state->port_info.with_dev == 1 && state->port_info.oper_type == 1 && state->port_info.check_num<20)//���֮ǰ�˿����豸,����Ϊ�رն˿�,�Ҽ�����С��20������Ϊ�豸����,
// 			{
// 				state->port_info.check_num += 1;
// 				ty_usb_m_out("=======��⵽%d�˿�Ϊ�ر��������µ綯��,�ǲ�ζ���,��%d�μ��=======\n", port, state->port_info.check_num);
// 				continue;
// 			}
// 			if (state->port_info.with_dev == 1)
// 			{
// 				//ty_usb_m_out("�µ������豸:%d---�Ƿ��Ѿ��ϵ�:%d\n", state->status, state->power);
// 				if (state->h_plate.hDev != NULL)
// 				{
// 					if (state->h_plate.hDev->bBreakAllIO != 1)
// 						logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,����ǰ�豸��Ϣ����,need_free = %d\r\n", state->port_info.port_str, port, state->h_plate.need_free);
// 				}
// 				//�˴����ͷ�˰���Ѵ򿪵ľ��
// 				if (state->h_plate.need_free == 1)
// 				{
					
// 					if(state->port_info.port_used == 1)//�˿�ռ��
// 					{
						
// 						_lock_set(state->lock);
// 						if (state->h_plate.hDev != NULL)
// 						{
// 							if (state->h_plate.hDev->bBreakAllIO != 1)
// 								logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,��ǰʹ����ϢΪ��%s\r\n", state->port_info.port_str, port, state->port_info.used_info);
// 							state->h_plate.hDev->bBreakAllIO = 1;
// 						}
// 						_lock_un(state->lock);
// 						state->port_info.inv_query = 0;
// 						if (state->power_mode == Saving_Mode)//ʡ��ģʽ���ļ�ϵͳ���豸
// 						{
// 							state->sys_down_power = 0;
// 						}
// 						continue;
// 					}
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�����رվ��\n", state->port_info.port_str,port);

				
// 					result = common_close_usb_device(state,&state->h_plate);					

// 					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�رվ���ɹ�\n",  state->port_info.port_str,port);
// 				}
// 				memset(&state->port_last_info, 0, sizeof(struct _port_last_info));
// 				state->port_last_info.usb_app = state->port_info.usb_app;
// 				memcpy(state->port_last_info.ca_name, state->port_info.ca_name, sizeof(state->port_info.ca_name));
// 				memcpy(state->port_last_info.ca_serial, state->port_info.ca_serial, sizeof(state->port_info.ca_serial));
// 				memcpy(state->port_last_info.sn, state->port_info.sn, sizeof(state->port_info.sn));
// 				state->port_last_info.extension = state->port_info.extension;
// 				state->port_last_info.vid = state->port_info.vid;
// 				state->port_last_info.pid = state->port_info.pid;
// 				state->off_report = 1;
// 				state->last_act = 2;
// 				if (state->status == 1 && state->power_status == 1)//����δ�µ�--δ�����µ��̼߳�ⳬʱʱ��
// 				{
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "ԭ��%d�˿����豸,�����µ�step1  --state->need_re_detect:%d \n", port, state->need_re_detect);
// 					port_power_down_now(state, 0);
// 				}

				
// 			}
			
// 			state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
// 			if (state->port_info.port_status != 0)//��ʾ��������
// 			{
// 				_lock_set(state->lock);
// 				int status;
// 				int usbip_n;
// 				int usbip_state;
// 				usbip_state = state->port_info.usbip_state;
// 				usbip_n = state->port_info.usbip_n;
// 				status = state->port_info.port_status;
// 				memset(&state->port_info, 0, sizeof(state->port_info));
// 				state->port_info.port_status = status;
// 				state->port_info.usbip_n = usbip_n;
// 				state->port_info.usbip_state = usbip_state;
// 				_lock_un(state->lock);
// 			}
// 			else
// 			{
// 				if (state->port_info.with_dev == 1)
// 				{

// 					int ad_status;
// 					int port_used = 0;
// 					if (state->power_mode == Saving_Mode)
// 					{
// 						ad_status = state->port_info.ad_status;
// 					}
// 					_lock_set(state->lock);
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���port_info\n", state->port_info.port_str, port);
// 					if (state->port_info.port_used == -1)
// 					{
// 						port_used = state->port_info.port_used;
// 					}
// 					memset(&state->port_info, 0, sizeof(state->port_info));
// 					state->port_info.port_used = port_used;

// 					if (state->power_mode == Saving_Mode)
// 					{
// 						state->port_info.ad_status = ad_status;
// 					}
// 					_lock_un(state->lock);
// 				}
// 			}
			
// 			if (state->power_mode == Saving_Mode)//ʡ��ģʽ�� �˿ڼ���쳣�γ�Ҫ�����µ���Ϊ���豸״̬
// 			{
// 				if (state->sys_down_power == 1)//�Ѿ��µ�
// 				{
// 					state->sys_down_power = 0;
// 				}			
// 				if (state->status == 1 && state->power_status == 1)//����δ�µ�
// 				{
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "ԭ��%d�˿����豸,�����µ�step3\n", port);
// 					port_power_down_now(state,0);
// 				}
// 			}
// 			continue;
// 		}
// 		if (state->power_mode == Saving_Mode)
// 		{
// 			if (state->power_status == 0)//�µ��δ���ļ�ϵͳ��ʧ
// 				continue;
// 			else
// 			{
// 				state->sys_down_power = 0;
// 				if (state->status == 0)
// 				{
// 					state->status = 1;//�ϵ��ļ�ϵͳ�п϶����豸
// 					logout(INFO, "SYSTEM", "CHECK_PORT", "%d�˿�,�ϵ��ļ�ϵͳ�п϶����豸\n", port);
// 				}
// 			}

// 		}
// 		//����Ϊ�ļ�ϵͳ��⵽
// 		state->port_info.with_dev = 1;
// 		if (state->port_info.closing == 0)//�ǹر�״̬�����״̬��(1������ڹص翪��),������ڹص��ǲ�����˲���
// 		{
// 			//�����ڹرն˿ڹ���
// 			if (state->port_info.oper_type == 1)
// 			{	//Ϊ����ؿ��˿�ģʽ
// 				if (vid_pid_jude_plate(state->port_info.vid, state->port_info.pid)>0)
// 				{
// 					ty_usb_m_out("task_check_port %d�˿ڹرպ������¶�ȡ˰��ϸ��Ϣ\n",state->port);
// 					state->port_info.tax_read = 1;
// 				}
// 			}
// 			state->port_info.check_num = 0;
// 			state->port_info.oper_type = 0;
// 		}
// 		if (state->port_info.change == 0)//�ж��豸�Ƿ������仯,0�仯 1δ��,��ȡ���˿��豸��Ϣ�󽫴�״̬��Ϊ1,������ز��ٽ���˲���
// 		{		
// 			memset(state->port_info.busid, 0, sizeof(state->port_info.busid));
// 			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, state->port_info.busid);
// 			if (result < 0)
// 			{
// 				ty_usb_m_out("��ȡ�豸busid��Ϣʧ��\n");
// 				state->port_info.with_dev = 0;
// 				continue;
// 			}
// 			memset(&dev, 0, sizeof(dev));
// 			result = _usb_file_get_infor(state->port_info.busid, &dev);
// 			if (result < 0)
// 			{
// 				ty_usb_m_out("��ȡ�豸dev��Ϣʧ��\n");
// 				state->port_info.with_dev = 0;
// 				continue;
// 			}
// 			state->port_info.subclass = dev.interface[0].bInterfaceSubClass;
// 			state->port_info.dev_type = dev.interface[0].bInterfaceClass;
// 			state->port_info.protocol_ver = 2;
// 			state->port_info.vid = dev.idVendor;
// 			state->port_info.pid = dev.idProduct;
// 			state->port_info.bnuminterface = dev.bNumInterfaces;
// 			state->port_info.change = 1;

// 			state->insert_time = get_time_sec();
			

// 			if (vid_pid_jude_plate(state->port_info.vid, state->port_info.pid)>0)
// 			{
// 				state->port_info.tax_read = 1;
// #ifdef ALL_SHARE
// 				continue;
// #endif
					
// 			}
// #ifdef ALL_SHARE
// 			else
// 			{
// 				state->port_info.ca_read = 1;
// 			}
// #endif
// 			//ty_usb_m_out("��%d�ſڡ���⵽���豸����\n", port);
// 		}
// #ifdef ALL_SHARE
// 		else
// 		{
// 			if (state->port_info.ca_read == 1)
// 			{
// 				long time_now = get_time_sec();
// 				if (time_now - state->insert_time > 60)
// 				{

// 					ty_usb_m_out("%d�Ŷ˿�����ʧ�ܴ���\n", port);
// 					state->fail_num = 0;

// 				}
// 				state->insert_time = get_time_sec();
// 				if (state->fail_num > 20)
// 				{
// 					ty_usb_m_out("%d�Ŷ˿ڶ�ȡ֤������˰��ʧ�ܶ��,�����ظ���ȡ\n", port);
// 					state->port_info.ca_read = 0;
// 					if (state->power_mode == Saving_Mode)
// 					{
// 						_lock_set(state->power_lock);
// 						state->down_flag = 1;
// 						state->power_status = 1;
// 						state->last_up_power_time = get_time_sec();
// 						_lock_un(state->power_lock);
// 					}
// 				}
// 				else
// 				{

// 					//	ty_usb_m_out("%d�Ŷ˿����¶�ȡ����˰��\n", port);
// 					state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
// 				}
// 			}
// 		}
// #endif
// 	}
}



#ifdef DF_SUPPORT_CA
//base64����3des����--����
static int base_3des_data(uint8 *in_data,uint8 *out_data,int src_len,unsigned char *key_s)
{	uint8 key[16];
uint8 buf[1024];
uint8 in_buf[1024];
int  base_len;
int  buf_len;
int i;
int j;
int m;
memset(key,0,sizeof(key));
sprintf((char *)key,"Liu+Zhou%s",key_s);
//printf("key = %s\n",key);
base_len =src_len;
memset(in_buf,0,sizeof(in_buf));
memcpy(in_buf,in_data,base_len);
//printf("base64 data len%d\n",base_len);
//decode_string(in_data,buf);
//base64_dec(buf,in_data,base_len);
//printf("get dest \ndata\n");
//for(i=0;i<base_len;i++)
//{
//	printf("%02x ",in_buf[i]);
//}
//printf("\n");
memset(buf,0,sizeof(buf));
for(i=0,j=0;i<=base_len;i=i+4,j=j+3)
{
	decode((char *)in_buf+i,4,(char *)buf+j);
	/*for(m=0;m<4;m++)
	{
	printf("%02x ",(unsigned char)in_buf[i+m]);
	}
	printf("\n");
	printf("3des:");
	for(n=0;n<3;n++)
	{
	printf("%02x ",(unsigned char)buf[j+n]);
	}
	printf("\n");*/
}
//printf("ȥ��base64��λ\n");

buf_len=(base_len/4)*3;
int yu=buf_len%8;
buf_len = buf_len -yu;	
//printf("base64 len %d\ndata\n",buf_len);
//for(i=0;i<buf_len;i++)
//{
//	printf("%02X ",buf[i]);
//}
//printf("\n");
//

des3_out(buf,buf_len,key);
//printf("�����Ĳ�1\n");
for(m=0;m<8;m++)
{
	if(buf[buf_len-1-m]==0x01)
		buf[buf_len-1-m]=0;
	else
		break;
}
//printf("end \ndata\n");
//for(i=0;i<buf_len;i++)
//{
//	printf("%02X ",buf[i]);
//}
//printf("\n");
//memset(out_data,0,sizeof(out_data));
memcpy(out_data,buf,buf_len);
return buf_len;	
}
//����������˰��Ӧ��JSON����
static int analysis_json_data(char *inbuf,char *key,int *valueInt,char *valueString)
{	
	cJSON *item;  
	cJSON *root=cJSON_Parse(inbuf);  
	if (!root)  
	{   ty_usb_m_out("Error before: [%s]\n",cJSON_GetErrorPtr());  
	return -1;  
	}  
	else  
	{	cJSON *object=cJSON_GetObjectItem(root,"data");  
	if(object==NULL)  
	{	printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
	cJSON_Delete(root);  
	return -1;  
	}    
	if(object!=NULL)  
	{	//ty_usb_m_out("��ȡЭ��汾\n");
		item=cJSON_GetObjectItem(object,key);  
		if(item!=NULL)  
		{	//ty_usb_m_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);
			if(item->type == 3)
			{
				*valueInt = item->valueint;
			}
			else if(item->type == 4)
			{
				memcpy(valueString,item->valuestring,strlen(item->valuestring));  
			}                
		}  
		else
		{
			cJSON_Delete(root);
			return -1;
		}
	}    
	cJSON_Delete(root); 
	return 0;
	}  
	return -1;
}
static int ty_socket_read(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{   //ty_usb_m_out("read data by read i = %d\n",i);
		result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{	//ty_usb_m_out("read data time ty_usb_m_out or err \n\n\n\n");
			return result;
		}
	}
	return i;
}
static int unpack_ca_data(int fd, uint8 *out_buf, int *out_len, uint16 cmd)
{
	uint8 edit[2];
	uint8 len_buf[2];
	uint8 order[2];
	int buf_len;
	uint16 order_c;
	//ty_usb_m_out("��ȡ�汾��Ϣ\n");
	memset(edit, 0, sizeof(edit));
	memset(len_buf, 0, sizeof(len_buf));
	memset(order, 0, sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
	if (ty_socket_read(fd, edit, sizeof(edit))<0)
	{
		ty_usb_m_out("��ȡ�汾��Ϣ����\n");
		return -1;
	}
	if ((edit[0] != 0x01) && (edit[1] != 0x10))
	{
		ty_usb_m_out("��ȡ���İ汾��Ϣ����%02x %02x\n", edit[0], edit[1]);
		return -2;
	}
	//ty_usb_m_out("�汾��Ϣ�ɹ� %02x %02x\n",edit[0],edit[1]);
	//ty_usb_m_out("��ȡ������Ϣ\n");
	if (ty_socket_read(fd, len_buf, sizeof(len_buf))<0)
	{
		ty_usb_m_out("������Ϣ��ȡʧ��\n");
		return -1;
	}
	buf_len = len_buf[0] * 256 + len_buf[1];
	//ty_usb_m_out("��֡����:%d\n",buf_len);
	if (buf_len<6)
	{
		ty_usb_m_out("����С��6����\n");
		return -3;
	}
	if (buf_len - 6>(*out_len))
	{
		ty_usb_m_out("���ȹ�����%d\n", buf_len - 6);
		return -4;
	}
	if (ty_socket_read(fd, order, sizeof(order))<0)
	{
		ty_usb_m_out("��ȡ������ʧ��\n");
		return -1;
	}
	order_c = order[0] * 256 + order[1];
	if (order_c != cmd)
	{	//ty_usb_m_out("�����ֲ�һ�»�Ϊ�������\n");
		return -1;
	}
	if (ty_socket_read(fd, out_buf, buf_len - 6)<0)
	{
		ty_usb_m_out("��ȡ����ʧ��\n");
		return -1;
	}
	(*out_len) = (buf_len - 6);
	return (order[0] * 256 + order[1]);
}


static int read_ukey_ca_name_1(struct _port_state   *state)
{
	//struct _port_state   *state;
	//struct _ty_usb_m  *stream;
	int socketfd;
	uint8 inbuf[50];
	uint8 outbuf[2000];
	int out_len;
	//int i;
	char name_tmp[500];
	char json_data[1000];
	char key[8];
	int valueInt;
	char valueString[500];
	int result;
	char so_name[200];
	memset(so_name, 0, sizeof(so_name));
	memset(name_tmp, 0, sizeof(name_tmp));
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));
	/*if (vid_pid_jude_plate(state->port_info.vid, state->port_info.pid)>0)
	return -1;*/
	sprintf(so_name, "/lib/lib%04x_%04x.so", state->port_info.vid, state->port_info.pid);
	if (access(so_name, 0) != 0)
	{
		ty_usb_m_out("�Ҳ���ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
		return CA_READ_ERR_NOT_FOUND_SO;
	}
	ty_usb_m_out("�ҵ�ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
	inbuf[0] = 0x01; inbuf[1] = 0x10;
	inbuf[2] = 0x00; inbuf[3] = 0x26;
	inbuf[4] = 0x05; inbuf[5] = 0x01;
	memcpy(inbuf + 6, state->port_info.busid, sizeof(state->port_info.busid));

	socketfd = socket_tcp_open_send_recv_timeout("127.0.0.1", 10003, inbuf, 38);
	if (socketfd < 0)
	{
		//ty_usb_m_out("��������������ʧ��\n");
		//ty_usb_m_out("read_ukey_ca_name_1 �ڴ˴�����CA�����Ƿ�����\n");
		//stream->execute_pro = 1;	 //�����Ҫ������CA����
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -1;
	}
	out_len = sizeof(outbuf);
	//ty_usb_m_out("������ɣ��ȴ�����\n");
	result = unpack_ca_data(socketfd, outbuf, &out_len, 0x1501);
	if (result < 0)
	{
		//ty_usb_m_out("��ȡ����ʧ�ܻ�ʱ\n");
		close(socketfd);
		//ty_usb_m_out("�ڴ˴�����CA�����Ƿ�����\n");
		return -2;
	}
	//ty_usb_m_out("������CA������Ϣ����Ϊ%d,����Ϊ%s\n",out_len,outbuf);
	close(socketfd);
	memcpy(name_tmp, outbuf, out_len);
	memset(key, 0, sizeof(key));
	sprintf(key, "%04x%04x", state->port_info.vid, state->port_info.pid);
	memset(json_data, 0, sizeof(json_data));
	base_3des_data((uint8 *)name_tmp, (uint8 *)json_data, strlen(name_tmp), (unsigned char *)key);
	//ty_usb_m_out("MAIN ���ܺ������:\n%s\n",json_data);

	_lock_set(state->lock);
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "result", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		ty_usb_m_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��1\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -3;
	}
	ty_usb_m_out("result = %d\n", valueInt);
	state->port_info.ca_ok = valueInt;
	result = analysis_json_data(json_data, "encording", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		ty_usb_m_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��2\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -4;
	}
	ty_usb_m_out("encording = %d\n", valueInt);
	state->port_info.encording = valueInt;
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "cert_num", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		ty_usb_m_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��3\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -5;
	}
	ty_usb_m_out("cert_num = %s\n", valueString);

	memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
	if (valueString[strlen(valueString) - 1] > 0x7A)//new change by whl 2021-02-25
	{
		valueString[strlen(valueString) - 1] = '\0';
	}
	memcpy(state->port_info.ca_serial, valueString, strlen(valueString));
	char name_tmp1[500];
	memset(name_tmp1, 0, sizeof(name_tmp1));
	if (state->port_info.encording == 1)
	{
		ty_usb_m_out("utf8����\n");
		memset(valueString, 0, sizeof(valueString));
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		//ty_usb_m_out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		utf82gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	else if (state->port_info.encording == 0)
	{
		ty_usb_m_out("unicode����\n");
		memset(valueString, 0, sizeof(valueString));
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		//ty_usb_m_out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		unicode2gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	memcpy(state->port_info.ca_name, name_tmp1, strlen(name_tmp1));
	//ty_usb_m_out("ca_name = %s\n",name_tmp1);
	ty_usb_m_out("ca_name = %s\n", state->port_info.ca_name);

	//result = analysis_json_data(json_data,"ukey_type",&valueInt,valueString);
	//if(result <0||valueInt==0)
	//{	ty_usb_m_out("��ȡukey����ʧ��\n");
	//	state->port_info.plate_type = 0;
	//}
	//else
	//{	ty_usb_m_out("ukey_type = %d\n",valueInt);
	//	state->port_info.plate_type = valueInt;
	//}

	result = analysis_json_data(json_data, "extension", &valueInt, valueString);
	if (result <0)
	{
		ty_usb_m_out("��ȡ�����̺�ʧ��\n");
		state->port_info.extension = 0;
	}
	else
	{
		ty_usb_m_out("extension = %d\n", valueInt);
		state->port_info.extension = valueInt;
	}

	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "SN", &valueInt, valueString);
	if (result <0)
	{
		ty_usb_m_out("��ȡ˰�̱��ʧ��\n");
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
	}
	else
	{
		ty_usb_m_out("SN = %s\n", valueString);
		memcpy(state->port_info.sn, valueString, strlen(valueString));
	}
	_lock_un(state->lock);
	return 0;
}

// static int judge_dev_down_power(struct _port_state   *state)
// {
// 	if (state->port_info.vid == 0x1a56 && state->port_info.pid == 0xdd01)
// 	{
// 		return 0;
// 	}
// 	return -1;

// }
static void update_dev_passwd_err_port(int port, int left_num)
{
	char file_name[200] = { 0 };
	char buf[1024] = { 0 };
	sprintf(file_name,"/tmp/passwd_err/port%d.txt",port);
	sprintf(buf, "%d", left_num);
	write_file(file_name, buf, strlen(buf));
}


static void task_read_port_base_tax_info(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int usb_type;
	stream = arg;
	//char time_now[20] = {0};

	if (stream->frist_start == 0)
	{
		//ty_usb_m_out("�豸������,��ʱ20����ٶ�ȡ֤������\n");
		stream->frist_start = 1;
		sleep(20);
		return;
	}

	//ty_usb_m_out("task_check_port ���� %d\n",stream->usb_port_numb);
	for (i = 1; i <= stream->usb_port_numb; i++)
	{	//ty_usb_m_out("����%d�Ŷ˿�\n",i);
		state = &stream->port[i - 1];



		if ((state->port_info.with_dev == 0) || (state->retry_forbidden))
		{
			continue;
		}
		if (state->in_read_base_tax != 0) //�����ظ������߳�
		{
			//ty_usb_m_out("˰���ϴ�ʹ����\n");
			continue;
		}
		if (state->port_info.tax_read == 1)
		{						
			if (state->power_mode == Saving_Mode)
			{
				if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port) < 0)
				{
					ty_usb_m_out("%d�˿ڶ������Ϣǰ���γ���!\n", state->port);
					logout(INFO, "SYSTEM", "CHECK_PORT", "��%d�ſڡ���ȡ�����Ϣǰ���γ���\r\n", state->port);
					state->port_info.tax_read = 0;
					//port_power_down_now(state,0);
					/*	_lock_set(state->power_lock);
						state->down_flag = 1;
						state->power_status = 1;
						state->status = 0;
						_lock_un(state->power_lock);*/
					continue;

				}
			}

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//ty_usb_m_out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if ((usb_type == DEVICE_MENGBAI) || (usb_type == DEVICE_MENGBAI2))
			{

				//�ɰ�ģ��������ʱ8�뷽�ɶ�ȡ��Ϣ
				long time_now;
				time_now = get_time_sec();
				if ((time_now - state->insert_time) < 8)
				{
					continue;
				}
			}
		

			//ty_usb_m_out("%d�Ŷ˿ڷ�Ʊ�ϴ��ж�\n", state->port);
			
			if (stream->base_read_queue_num > 4)
			{
				//ty_usb_m_out("�ϴ����д���5��,stream->up_queue_num = %d\n", stream->up_queue_num);
				continue;
			}
			if (state->port_info.used_level == 1)
			{
				continue;
			}

			stream->base_read_queue_num += 1;
			state->in_read_base_tax = 1;
			ty_usb_m_out("%d�Ŷ˿���Ҫִ�л�����Ϣ��ȡ����\n",state->port);
			//logout(INFO, "system", "������Ϣ��ȡ", "%d�Ŷ˿���Ҫ��ȡ������Ϣ,���������̶߳���\r\n", i);
			_delay_task_add("up_m_server", deal_read_plate_base_tax_info, state, 10);
			continue;
		}
		else if (state->port_info.ca_read == 1)
		{
			ty_usb_m_out("%d�˿���Ҫ��ȡCA��Ϣ!\n", state->port);
			state->h_plate.nBusID = get_busid_busnum(state->port_info.busid);
			state->h_plate.nDevID = get_busid_devnum(state->port_info.busid);
			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//ty_usb_m_out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if (usb_type !=0)
			{
				state->port_info.ca_read = 0;
				state->port_info.fail_num = 0;
				continue;
			}


			if (port_lock(state, 5, 2, DF_PORT_USED_DQMCSHXX) < 0)		//wang 02 02 ���Խ���ڵ�汾 ���е�������  if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)
			{
				ty_usb_m_out("�˿�%d,��ʹ����,�޷���ȡ֤������\n",i);
				continue;
			}
			// if (state->power_mode == Saving_Mode)
			// {
			// 	_lock_set(state->power_lock);
			// 	state->down_flag = 0;
			// 	_lock_un(state->power_lock);
			// }
			struct _careader_process careader_process;
			memset(&careader_process, 0, sizeof(struct _careader_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			//��ȡ��˰�̵���˰�����ƺ�˰��
			if (atoi(careader_process.process_run) != 1)//CA����δ����
			{
				sleep(8); //ty_usb_m_out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
				ty_usb_m_out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
			}
			else//�����Ѿ�����
			{
				if (state->port_info.vid == 0x096e && state->port_info.pid == 0x0702)
				{
					if (access("/etc/ca_read", 0) != 0)//����Ҫ��ȡ��ca����˰��
					{
						state->port_info.usb_app = 5;
						//state->fail_num = 0;
						state->port_info.fail_num = 0;
						state->port_info.ca_read = 0;
						sleep(1);
						ty_usb_m_out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", i);
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						goto END;
					}
				}
				int result = read_ukey_ca_name_1(state);
				if (result < 0)
				{
					state->port_info.fail_num++;
					if(3 < state->port_info.fail_num)
					{
						state->port_info.fail_num = 0;
						state->port_info.usb_app = 5;
						state->port_info.ca_read = 0;
						ty_usb_m_out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", i);
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						goto END;
					}
					else
					{
						goto END;
					}
				}
				else
				{
					ty_usb_m_out("֤���ȡ�ɹ����ϱ���%d���˿�ca�豸\n", i);
					state->port_info.usb_app = 4;	
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					state->port_info.ca_read = 0;
					state->port_info.fail_num = 0;
					goto END;
				}
			}
		END:

			port_unlock(state);
			continue;
		}

	}
}

static void deal_read_plate_base_tax_info(void *arg)
{
	struct _ty_usb_m *stream;
	stream = ty_usb_m_id;
	struct _port_state   *state;
	int result, i, usb_type;// upfile_flag = 0;
	int need_check_passwd=0;
	struct _plate_infos plate_infos;
	int busnum;
	int devnum;
	int left_num = 0;
	char sn[16] = { 0 };
	
	state = arg;

	if (state->usb_err != 0)
	{
		ty_usb_m_out("��ǰ�˿�%d����USBʱ���ֹ�����,��ʱ3���ٽ��ж�ȡ\n",state->port);
		if(state->power_mode == Normal_Mode)
		{
			//logout(INFO, "system", "������Ϣ��ȡ", "�˿ڣ�%d�ϵ�USB�豸������Ϣ��ȡʧ��,�����˿ڵ�Դ���ٴ�����\r\n", state->port);
			//restart_plate_power(state, state->port_info.sn, state->port, 0);	
			state->usb_err = 0;
		}
		else{
			//restart_plate_power(state, NULL, state->port);
		}
		sleep(3);
	}
	usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
	if ((usb_type == DEVICE_AISINO) || (usb_type == DEVICE_NISEC) || (usb_type == DEVICE_MENGBAI) || (usb_type == DEVICE_MENGBAI2) || (usb_type == DEVICE_SKP_KEY) || (usb_type == DEVICE_CNTAX))
	{
		busnum = get_busid_busnum(state->port_info.busid);
		devnum = get_busid_devnum(state->port_info.busid);
		state->h_plate.nBusID = busnum;
		state->h_plate.nDevID = devnum; 
		if (port_lock(state, 5, 2, DF_PORT_USED_DQMCSHXX) < 0)			//wang 02 02 ���Խ���ڵ�汾 ���е������� if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)	
		{
			//ty_usb_m_out("�˿�%d,��ʹ����,�޷���ȡ֤������\n",state->port);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		// if (state->power_mode == Saving_Mode)
		// {
		// 	_lock_set(state->power_lock);
		// 	state->down_flag = 0;
		// 	_lock_un(state->power_lock);
		// }
		memset(&plate_infos, 0, sizeof(struct _plate_infos));
		
		state->h_plate.usb_type = usb_type;
		state->h_plate.need_check_passwd = need_check_passwd;
		memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
		strcpy(state->h_plate.busid, state->port_info.busid);

		
		result = function_common_get_basic_tax_info_local(state->port_info.busid, usb_type, (char*)plate_infos.plate_basic_info.ca_name, (char*)plate_infos.plate_basic_info.ca_number, sn, (int*)(&plate_infos.plate_basic_info.cert_passwd_right), &left_num);

// #ifdef RELEASE_SO
// 		result = so_common_get_basic_tax_info(&state->h_plate, &plate_infos);
// #else
// 		result = function_common_get_basic_tax_info(&state->h_plate, &plate_infos);
// #endif
		if (result < 0)
		{
			ty_usb_m_out("��%d��˰�̶�ȡ����˰��ʧ��\n", state->port);
			state->usb_err = 1;//��¼USB���ֹ�����

			if(0 < left_num)
			{
				state->retry_forbidden = 1;
				update_dev_passwd_err_port(state->port, left_num);
			}

			// if (plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_DEV_PASSWD_ERR)
			// {
			// 	//ty_usb_m_out("��¼��˰���豸��������!\n");
			// 	logout(INFO, "system", "������Ϣ��ȡ", "��¼��%d�Ŷ˿�,˰���豸��������!,ʣ�ೢ�Դ���%d��\r\n", state->port,plate_infos.plate_basic_info.dev_left_num);
			// 	state->retry_forbidden = 1;
			// 	update_dev_passwd_err_port(state->port, plate_infos.plate_basic_info.dev_left_num);
			// 	state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
			// 	memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
			// 	sprintf((char *)state->port_info.ca_name, "˰���豸�������");
			// 	memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
			// 	sprintf((char *)state->port_info.ca_serial, "000000000000000");
			// 	memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
			// 	sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
			// 	memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "˰���豸�������");
			// 	memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
			// 	memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);				
			// 	state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
			// 	state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
			// 	state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
			// 	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			// 	state->last_act = 1;
			// 	state->port_info.tax_read = 0;
			// }
			// else if (plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_DEV_PASSWD_LOCK)
			// {
			// 	//ty_usb_m_out("��¼��˰���豸��������!\n");
			// 	logout(INFO, "system", "������Ϣ��ȡ", "��¼��%d�Ŷ˿�,˰���豸��������!,ʣ�ೢ�Դ���%d��\r\n", state->port, plate_infos.plate_basic_info.dev_left_num);
			// 	//state->retry_forbidden = 1;//�˿���������������
			// 	state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
			// 	memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
			// 	sprintf((char *)state->port_info.ca_name, "˰���豸�������");
			// 	memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
			// 	sprintf((char *)state->port_info.ca_serial, "000000000000000");
			// 	memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
			// 	sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
			// 	memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "˰���豸�������");
			// 	memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
			// 	memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
			// 	sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);
			// 	state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
			// 	state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
			// 	state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
			// 	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			// 	state->last_act = 1;
			// 	state->port_info.tax_read = 0;		
				
			// }
			//if(state->port_info.used_level == 2)
			port_unlock(state);
			state->port_init = 2;
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		_lock_set(state->lock);
		state->usb_err = 0;//�����ɹ����USB�����¼
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		memcpy(state->port_info.ca_name, plate_infos.plate_basic_info.ca_name, strlen((const char *)plate_infos.plate_basic_info.ca_name));
		memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
		memcpy(state->port_info.ca_serial, plate_infos.plate_basic_info.ca_number, strlen((const char*)plate_infos.plate_basic_info.ca_number));
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
		memcpy(state->port_info.sn, sn, strlen(sn));
		
		//state->port_info.usb_app = usb_type; 		//wang 2022��10��17�� Ĭ�϶���4����ca�ϱ�		
		state->nongye_report = result;				//wang 11/18
		state->port_info.usb_app = 4;
		//printf("deal_read_plate_base_tax_info usb_type = %d\r\n",usb_type);
		//state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
		state->port_info.ca_ok = plate_infos.plate_basic_info.result;
		state->port_info.extension = plate_infos.plate_basic_info.extension;
		// memset(&state->port_info.plate_infos.plate_basic_info, 0, sizeof(struct _plate_basic_info));
		// memcpy(&state->port_info.plate_infos.plate_basic_info, &plate_infos.plate_basic_info, sizeof(struct _plate_basic_info));
		// memset(&state->port_info.plate_infos.plate_tax_info, 0, sizeof(struct _plate_tax_info));
		// memcpy(&state->port_info.plate_infos.plate_tax_info, &plate_infos.plate_tax_info, sizeof(struct _plate_tax_info));
		state->port_info.offinv_num_exit = plate_infos.plate_tax_info.off_inv_num;
		memset(state->port_info.plate_time, 0, sizeof(state->port_info.plate_time));
		memcpy(state->port_info.plate_time, plate_infos.plate_tax_info.plate_time, strlen((char *)plate_infos.plate_tax_info.plate_time));
		sprintf(state->port_info.port_str,"���˿ںţ�%d,�̺ţ�%s,���ƣ�%s��",state->port,state->port_info.sn,state->port_info.ca_name);
		for (i = 0; i < DF_MAX_INV_TYPE; i++)
		{
			memset(&state->port_info.plate_infos.invoice_type_infos[i], 0, sizeof(struct _invoice_type_infos));
			memcpy(&state->port_info.plate_infos.invoice_type_infos[i], &plate_infos.invoice_type_infos[i], sizeof(struct _invoice_type_infos));
		}

		if (strlen((char *)state->port_info.plate_time) != 0)
		{
			long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
			long time_now = get_time_sec();
			//ty_usb_m_out("˰��ʱ���Ϊ%ld,��ǰϵͳʱ���Ϊ%ld\n", plate_time, time_now);
			state->port_info.plate_sys_time = plate_time - time_now;
			//ty_usb_m_out("˰���뵱ǰϵͳʱ���Ϊ%d\n", state->port_info.plate_sys_time);
		}
		state->port_init = 2;
		_lock_un(state->lock);


		


		// result = deal_read_plate_cert_passwd(state);
		// if (result < 0)
		// {
		// 	state->usb_err = 1;//��¼USB���ֹ�����
		// 	port_unlock(state);
		// 	stream->base_read_queue_num -= 1;
		// 	state->in_read_base_tax = 0;
		// 	return;
		// }
		port_unlock(state);

	
		
		_lock_set(state->lock);
		//ty_usb_m_out("%d�Ŷ˿ڵĽ�˰��ȫ��˰����Ϣ��ȡ���,���ѳ����忨�߳�\n", state->port);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		state->port_info.need_chaoshui = 1;
		state->port_info.need_huizong = 1;
		state->port_info.need_fanxiejiankong = 1;
		_lock_un(state->lock);
		sem_post(&stream->cb_qk_sem);

		
		state->need_getinv = 1;//���ϴ���Ʊͳ�Ʋ�ѯ
		//state->need_sync_tzdbh = 1; //ͬ��������Ϣ��
		//state->need_sync_netinv = 1;//ͬ��������ƱԴ
		state->port_info.tax_read = 0;
		logout(INFO, "system", "������Ϣ��ȡ", "%s,˰�ţ�%s,�ֻ��ţ�%d,˰�����ͣ�%d  tax_read:%d\r\n", state->port_info.port_str, state->port_info.ca_serial, state->port_info.extension, state->port_info.usb_app, state->port_info.tax_read);
		//upfile_flag = 1;

	}
	else
	{
	//	logout(INFO, "system", "������Ϣ��ȡ", "%d�˿�Ϊ�����豸\r\n", state->port);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		state->port_info.usb_app = 5;
		state->port_info.tax_read = 0;
	}
	


	//if (upfile_flag == 1)
	//{
	//	char cmd[100] = {0};
	//	memset(cmd, 0, sizeof(cmd));
	//	sprintf(cmd, "rm /home/share/exe/port_info.dat");
	//	system(cmd);
	//	write_port_info_file(stream);
	//	upfile_flag = 0;
	//}
	stream->base_read_queue_num -= 1;
	state->in_read_base_tax = 0;
	return;
}


#endif


static int get_dev_passwd_err_port(int port, int *left_num)
{
	char file_name[200] = { 0 };
	char s_buf[1024] = { 0 };
	sprintf(file_name, "/tmp/passwd_err/port%d.txt", port);
	if (access(file_name, 0) != 0)
	{
		return 0;
	}
	read_file(file_name, s_buf, sizeof(s_buf));
	*left_num = atoi(s_buf);
	return 1;
}

int vid_pid_jude_plate(uint16 vid, uint16 pid)
{
	//ty_usb_m_out("vid %04x,pid %04x\n", vid, pid);
	if ((vid == 0x101d) && (pid == 0x0003))
	{
		return DEVICE_AISINO;//���п�����˰��ukey DEVICE_CNTAX
	} 
	else if ((vid == 0x1432) && (pid == 0x07dc))
	{
		return DEVICE_NISEC;
	}
	else if ((vid == 0x0680) && (pid == 0x1901))
	{	
		return DEVICE_MENGBAI;
	}
	else if ((vid == 0x0680) && (pid == 0x2008))
	{
		return DEVICE_MENGBAI2;
	}
	else if ((vid == 0x14d6) && (pid == 0xa002))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x3254) && (pid == 0xff10))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x287f) && (pid == 0x00d4))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x1677) && (pid == 0x6101))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x1ea8) && (pid == 0xb00b))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0xd449) && (pid == 0x0001))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x2012) && (pid == 0x2010))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x163c) && (pid == 0x5480))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x1432) && (pid == 0x07d0))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x2ec9) && (pid == 0x1001))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x101d) && (pid == 0x0005))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x3a59) && (pid == 0x4458))
	{
		return DEVICE_CNTAX;
	}
	else if ((vid == 0x1432) && (pid == 0x07e2))
	{
		return DEVICE_SKP_KEY;
	}
	return 0;
}


//�������ƹ���
static void task_port_led_manage(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	stream = arg;
	//stream->timer = timer;
	for (i = 1; i <= stream->usb_port_numb; i++)
	{
		int now_port_led_status = 0;
		state = &stream->port[i - 1];
		//printf("state->port_info.with_dev = %d\r\n",state->port_info.with_dev);
		if (state->port_info.with_dev == 0)
		{
			if (state->retry_forbidden == 1)//�˿ڲ���豸���벻��ȷ��˰�̣��˶˿ڼ���������������ʹ��
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
			}
			else
			{
				if (jude_port_allow_attach(state) < 0)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
				}
				else
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_ON;
				}
				//now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;  //wang 11/16 ���ƻ������̵ƻ��߹ر�
			}
		}
		else
		{
			if (state->retry_forbidden == 1)//�˿ڲ���豸���벻��ȷ��˰�̣��˶˿ڼ���������������ʹ��
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				goto SET_LED;
			}
// #ifdef DF_SUPPORT_CA
// 			else if ((( (jude_port_led_auth(state) == -1) || (jude_port_led_auth(state) == -2) ) || (state->port_info.usb_app != 4 && jude_port_led_auth(state) < 0) ) && state->port_info.plate_infos.plate_basic_info.dev_err_reason >=0)//δ��Ȩ����Ȩ���� 
// 			{
// 				now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
// 				goto SET_LED;

// 			}
// 			else if (state->port_info.usb_app == 4)
// 			{
// 				now_port_led_status = DF_TY_USB_CM_PORT_RED_ON_GREN_OFF;
// 				goto SET_LED;
// 			}

// #endif
			else
			{
				if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_USB_PORT_NOT_AUTH)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
				}
				else if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_DEV_PASSWD_ERR)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				}
				else if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_DEV_PASSWD_LOCK)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				}
				else if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == DF_PLATE_CERT_PASSWD_ERR)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				}
				else
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_ON_GREN_OFF;
				}
			}
		}
SET_LED:
		if (state->port_led_status != now_port_led_status)
		{
			if (ty_ctl(stream->ty_usb_fd, now_port_led_status, i) < 0)
			{
				usleep(2000);
				ty_ctl(stream->ty_usb_fd, now_port_led_status, i);
			}

			state->port_led_status = now_port_led_status;
		}

	}
		

	//ty_usb_m_out("������޶˿ڱ�USBshare ����\n");
	for (i = 1; i <= stream->usb_port_numb; i++)
	{
		state = &stream->port[i - 1];
		if (state->port_info.usbip_state != 0)
		{
			break;
		}
	}
	if (i > stream->usb_port_numb)
	{
		if (stream->lcd_share_state != 10)
		{
			lcd_print_info.usbshare_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_share_state = 10;
		}

	}
	else
	{
		if (stream->lcd_share_state != 20)
		{
			lcd_print_info.usbshare_state = 1;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_share_state = 20;
		}

	}

	//ty_usb_m_out("���˿��������ڲ���˰����Ϣ\n");
	for (i = 1; i <= stream->usb_port_numb; i++)
	{
		state = &stream->port[i - 1];
		if (state->port_info.system_used != 0)
		{
			break;
		}
	}

	if (i > stream->usb_port_numb)
	{
		if (stream->lcd_tax_state != 10)
		{
			//ty_usb_m_out("lcd ָʾ���豸����˰����Ϣ\n");
			lcd_print_info.tax_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_tax_state = 10;
		}

	}
	else
	{
		//ty_usb_m_out("�޶˿ڲ���˰����Ϣ\n");
		if (stream->lcd_tax_state != 20)
		{
			//ty_usb_m_out("lcd ָʾ���豸����˰����Ϣ\n");
			lcd_print_info.tax_state = 1;			
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_tax_state = 20;
		}

	}
}




/*==========================================ctl����===============================================*/

int get_usb_busid(int port, char*busid)
{
	struct _ty_usb_m  *stream;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
	ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, busid);	
	return port;
}
//�л������ڵ�ģʽ
int change_all_port_power_mode(int mode)
{
	struct _ty_usb_m  *stream;
	int result;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	if ((mode == 1) || (mode == 0))
	{
		result = ty_ctl(stream->deploy_fd, DF_DEPLOY_CM_SET_TER_POW_MODE, mode);
	}
	else 
	{
		return -1;
	}
	return result;

}

//�ı�˿ڵ�Դģʽ port �˿ں�  mode:ģʽ 0Ϊ��ʡ��  1 Ϊʡ��
int change_port_power_mode(int port,int mode)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int result;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	if (stream->dev_support.support_power == 0)//�ǽڵ�汾��֧��
		return -1;
	if (mode == 0)
	{
		state->power_mode = Normal_Mode;
		//ty_usb_m_out("change_port_power_mode mode 0 �ر�%d�Ŷ˿�\n",state->port);
		//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);

		port_power_down(state);
		//if (result < 0)
		//{
		//	logout(INFO, "SYSTEM", "INIT", "֪ͨ��Ƭ���رն˿ڵ�Դʧ��,�˿ں�%d\r\n", state->port);
		//}
		//state->power = 0;
		usleep(10000);
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, state->port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "INIT", "֪ͨ��Ƭ���򿪱ն˿ڵ�Դʧ��,�˿ں�%d\r\n", state->port);
		}
		state->power = 1;
	}
	else if (mode == 1)
	{
		//ty_usb_m_out("change_port_power_mode mode 1 �ر�%d�Ŷ˿�\n", state->port);
		/*result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "INIT", "֪ͨ��Ƭ���رն˿ڵ�Դʧ��,�˿ں�%d\r\n", state->port);
		}
		state->power = 0;*/
		port_power_down(state);
		usleep(10000);
		state->power_mode = Saving_Mode;
	}
	return 0;
}

int change_port_gw_state(struct _new_usbshare_parm* parm,int port)
{
	struct _ty_usb_m  *stream;
	char serverip[100] = { 0 };

	struct _port_state   *state;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;

	

	state = &stream->port[port - 1];
	get_DNS_ip(parm->trans_server, serverip);
	_lock_set(stream->port[port - 1].lock);
	state->port_info.ip = ip_asc2ip(serverip, strlen(serverip));
	memcpy(state->port_info.client, parm->client_id, sizeof(parm->client_id));
	state->port_info.usb_share_mode = 1;

	_lock_un(stream->port[port - 1].lock);

	stream->port[port - 1].up_report = 1;//�˿�״̬������Ҫ�ϱ�
	stream->port[port - 1].last_act = 1;
	return 0;
}

static int jude_port_allow_attach(struct _port_state  *state)
{
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	char date_now[100] = { 0 };
	timer_read_y_m_d_h_m_s(date_now);
	//printf("jude_port_allow_attach date_now = %s stream->end_time = %s\n",date_now,stream->end_time);
	if (memcmp(date_now, stream->end_time, 10) > 0)
	{
		//ty_usb_m_out("�˿���Ȩ����\n");
		return -2;
	}
	return 0;
}

// static int jude_port_led_auth(struct _port_state  *state)
// {
// 	if (state->port_info.usb_app == 4)//ca 
// 	{
// 		if ((state->aisino_sup != 1) && (state->nisec_sup != 1) && (state->cntax_sup != 1))
// 			return -1;
// 		char date_now[100] = { 0 };
// 		timer_read_y_m_d_h_m_s(date_now);
// 		if (memcmp(date_now, state->end_time, 10) > 0)
// 		{
// 			return -2;
// 		}
// 	}
// 	else 
// 	{
// 		if (state->port_info.with_dev == 0)
// 		{

// 			if ((state->aisino_sup != 1) && (state->nisec_sup != 1) && (state->cntax_sup != 1))
// 				return -1;
// 			char date_now[100] = { 0 };
// 			timer_read_y_m_d_h_m_s(date_now);
// 			if (memcmp(date_now, state->end_time, 10) > 0)
// 			{
// 				return -2;
// 			}
// 		}
// 		else
// 		{

// 			// int result;
// 			// char errinfo[1024] = { 0 };
// // #ifdef RELEASE_SO
// // 			result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// // #else
// // 			result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// // #endif
// 			// if (result != 1)
// 			// {
// 				return -3;
// 			// }
// 		}
// 	}
// 	return 0;
// }

int open_port_attach(int port, uint32 ip, char *client, int *port_fd)
{
	struct _ty_usb_m  *stream;

	//int i;
	int result;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;

	struct _port_state  *state;
	state = &stream->port[port - 1];
	ty_usb_m_out("[open_port_ip]���˿ںŴ򿪶˿� port = %d\n", port);
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
#ifdef DF_SUPPORT_CA
	if ((result = jude_port_allow_attach(&stream->port[port - 1])) < 0)
	{
		printf("jude_port_allow_attach failed result = %d\n",result);
		return DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION;
	}
#endif
	if (state->power_mode == Normal_Mode)
	{
		if (state->port_info.usb_app == DEVICE_CNTAX && (state->port_info.vid == 0x101d && state->port_info.pid == 0x0003))//˰��Ukey
		{
			ty_usb_m_out("��ʼ���ض˿����µ�:%d\n");
			state->sys_reload = 1;
			/*ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			state->power = 0;*/
			port_power_down(state);
			usleep(20000);
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			state->power = 1;
			sleep(2);
			state->sys_reload = 0;
			ty_usb_m_out("�������ض˿����µ�\n");
		}
	}
	if (stream->port[port - 1].port_info.with_dev == 0)
	{
		ty_usb_m_out("[open_port_ip]û�з��ָ��豸����೬ʱ�ȴ�7��\n");

		int time = 0;
		for (time = 0; time < 7000; time++)
		{
			if (stream->port[port - 1].port_info.with_dev == 1)
			{
				sleep(1);
				break;
			}
			usleep(1000);
		}
		if (time == 7000)
		{
			ty_usb_m_out("[open_port_ip]�ö˿��Ѿ���,û�з��ָ��豸\n");
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
	}
	if ((stream->port[port - 1].port_info.port_status != 0) && (stream->port[port - 1].port_info.used_level == 1))
	{
		ty_usb_m_out("�˿��Ѿ�������\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}

	if (port_lock(&stream->port[port - 1], 10, 1, DF_PORT_USED_USB_SHARE) < 0)
	{
		ty_usb_m_out("���ض˿������˿�ʧ��\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}
	if ((result = open_port(stream, port, 0x7f000001, NULL))<0)
	{
		stream->port[port - 1].port_info.port_status = 0x00;
		ty_usb_m_out("[open_port_ip]�򿪶˿�ʧ��\n");
		port_unlock(&stream->port[port - 1]);
		return result;
	}
	_lock_set(stream->port[port - 1].lock);
	stream->port[port - 1].port_info.usbip_state = 1;
	stream->port[port - 1].port_info.app_fd = timer_read(stream->timer_fd);
	//stream->port[port - 1].port_info.usbip_n = result;
	stream->port[port - 1].port = port;
	stream->port[port - 1].port_info.ip = ip;
	char ip_str[50] = { 0 };
	ip2ip_asc(ip, ip_str);
	logout(INFO, "usbshare", "attach", "%s,10001��·����USB�˿�,�˿ںţ�%d,ʹ����IP��%s\r\n", state->port_info.port_str, port, ip_str);
	if (strlen(client) != 0)
		memcpy(stream->port[port - 1].port_info.client, client, strlen(client));
	*port_fd = stream->port[port - 1].port_info.app_fd;
	stream->port[port - 1].port_info.last_open_time = get_time_sec();
	stream->port[port - 1].up_report = 1;//�˿�״̬������Ҫ�ϱ�
	stream->port[port - 1].last_act = 1;
	_lock_un(stream->port[port - 1].lock);
	return port;
}
//ǿ�ƹرն˿�
int close_port_dettach(int port)
{
	struct _ty_usb_m  *stream;
	//int result;
	struct _port_state  *state;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	ty_usb_m_out("ʹ��ǿ�ƹرն˿�,�˿ںš�%d��\n", port);
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}

	state = &stream->port[port - 1];
	//if (state->power_mode == Saving_Mode)
	//{
	//	ty_usb_m_out("ʹ��ǿ�ƹرն˿�,�ͷ�USB����˿ںš�%d��\n", port);
	//	free_h_plate(state);
	//}
	close_port_socket(stream, port);
	logout(INFO, "10001", "dettach", "%s,ǿ���ͷ�USB�˿�,�˿ںţ�%d\r\n", stream->port[port - 1].port_info.port_str, port);
	if (state->power_mode == Saving_Mode)	
	{		
#ifdef DF_SUPPORT_CA
		ty_usb_m_out("ǿ���ͷ�USB�˿ں���CA��ȡ\n", port);
		state->port_info.ca_read = 1;
#endif		
	}
	return 0;
}




//��ȡUSb vid pid
int get_usb_vid_pid(int port,uint16 *vid,uint16 *pid)
{   
    struct _port_state  *state;
    //struct _usb_file_dev    dev;
	//const char *prod, *vend;
	//char buff[100];
    //int result;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	//ty_usb_m_out("[get_usb_port_information]��ȡ%d��USB��Ϣ\n",port);
	if (vid == NULL || pid == NULL)
	{	ty_usb_m_out("USB�˿ڲ�������\n");\

        return -1;
	}
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
    state=&stream->port[port-1];
    //_lock_set(state->lock);
	//printf("int step1--port:%d\n", port);
	if (port != 0 && state->port_info.with_dev != 0)
    {   //memset(&dev,0,sizeof(dev));

		//printf("int step2\n");
		*vid = state->port_info.vid;
		*pid = state->port_info.pid;

		return 0;

       
    }
	
    return -1;
}

int get_module_state(void)
{
	struct _ty_usb_m  *stream;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	struct _port_state   *state;
	state = &stream->port[0];
	if (strlen((char *)state->port_info.sn) > 0)
	{
		sleep(1);
		return 0;
	}
	return -1;
}

int get_reg_ble_name(char *ble_name)
{
	struct _ty_usb_m  *stream;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	struct _port_state  *state;
	state = &stream->port[0];

	//printf("get sn :%s\n", state->port_info.sn);
	memcpy(ble_name, state->port_info.sn, strlen((char *)state->port_info.sn));

	return 0;
}


int fun_deal_all_port_report(void)
{
	int port;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	stream = ty_usb_m_id;
	stream->report_ter_flag = 0;
	sleep(1);
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		if (state->port_info.with_dev != 0)
		{
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
	}
	return 0;
}





//=================================================



//STM32�򿪶˿�
static int open_port_ty_usb(struct _ty_usb_m  *stream,int port)
{   int result;
	struct _port_state   *state;
	state = &stream->port[port - 1];
    result=ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT,port);
	state->power = 1;
    if(result>=0)
        return result;
    if(result==DF_ERR_PORT_CORE_TY_USB_MAIN_HUB)
	{
		char name[DF_FILE_NAME_MAX_LEN] = { 0 };
        int fd;
        ty_usb_m_out("�����й�����,������hubû���ҵ�,ϵͳҪ����������...\n");
        memset(name,0,sizeof(name));
        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
        fd=ty_open(name,0,0,NULL);       
        ty_usb_m_out("д�¼�\n");
        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_MAIN_HUB);
        ty_close(fd);
        _delay_task_add("reset",reset_systerm,stream,10); 
    }
    return result;
}



static int close_port_ty_usb(struct _ty_usb_m  *stream,int port)//���ǽڵ���ʹ��
{	
	int result;
	struct _port_state   *state;
	state = &stream->port[port - 1];

	state->h_plate.nBusID = -1;
	state->h_plate.nDevID = -1;
	ty_usb_m_out("close_port_ty_usb �����رն˿ڵ�Դ\n");
	//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
	//state->power = 0;
	port_power_down(state);
    //usleep(80000);
	sleep(3);					//wang 11/25 ��ʱ̫�̻����ż���Զ˿��µ�δ��������Ŀǰ�ͻ����жܻ������������
	//if(result==DF_ERR_PORT_CORE_TY_USB_CLOSE)
	//{	//int fd;
	//	ty_usb_m_out("��Դ�رպ��ļ�ϵͳ��ʾUSB�豸��Ȼ����,�˿ں�%d\n",port);
	//}
	ty_usb_m_out("�رյ�Դ�󼴽����¿����˿ڵ�Դ\n");

	result = ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT_POWER,port);
	state->power = 1;
	if(result < 0)
	{
		ty_usb_m_out("�رն˿ڵ�Դ��,���´򿪶˿ڵ�Դʧ��,�˿ں�%d\n",port);
		logout(INFO, "system", "closeport", "�رն˿ڵ�Դ��,���´򿪶˿ڵ�Դʧ��,�ٴδ򿪶˿ڵ�Դ,�˿ں�%d\r\n", port);
		sleep(1);
		result = ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT_POWER,port);
		state->power = 1;
	}
	return result;	
	return 0; 
}


static void reset_systerm(void *arg)
{   //struct _ty_usb_m  *stream;
	char name[DF_FILE_NAME_MAX_LEN] = { 0 };
    int fd;
    //stream=arg;
    printf("reset_systerm\n");
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    fd=ty_open(name,0,0,NULL);
    ty_ctl(fd,DF_TY_PD_CM_RESET);
}


static int check_port_used_time(struct _port_state   *state)
{
	struct _ty_usb_m * stream;
	stream = ty_usb_m_id;
	long time_now;
	if (state->port_info.port_used != 1)
	{
		return 0;
	}
	time_now = get_time_sec();
	if (strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) == 0)
	{

		if (time_now - state->port_info.used_time > 60)
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������1����,ϵͳ�жϸ��̿��ܴ����쳣,������%d�˿ڽ��жϵ���������\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
			ty_usb_m_out("����%d�Ŷ˿ڵ�Դ\n", state->port);

			restart_plate_power(state, state->port_info.sn, state->port, 0);			
		}

	}
	else if ((strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USED_HZSC) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USED_FXJK) == 0))
	{
		
		if (time_now - state->port_info.used_time > 120)
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������2����,ϵͳ�жϸ��̿��ܴ����쳣,������%d�˿ڽ��жϵ���������\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
			ty_usb_m_out("����%d�Ŷ˿ڵ�Դ\n", state->port);
			restart_plate_power(state, state->port_info.sn, state->port, 0);
		}
		if ((time_now - state->port_info.used_time > 600) && (state->port_info.used_time != 0))
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������10����,ϵͳ�жϸ��̿��ܴ����쳣����,���������������\r\n", state->port_info.port_str, state->port_info.used_info);
			sleep(10);
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		}
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_FPCX_SCMB) == 0)
	{
		if (state->port_info.usb_app != DEVICE_MENGBAI)
		{
			if (time_now - state->port_info.used_time > 1200)
			{
				logout(INFO, "system", "check_port", "%s������%s��ʱ������20����,ϵͳ�жϸ��̿��ܴ����쳣,������%d�˿ڽ��жϵ���������\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
				ty_usb_m_out("����%d�Ŷ˿ڵ�Դ\n", state->port);
				restart_plate_power(state, state->port_info.sn, state->port, 0);
			}
			if ((time_now - state->port_info.used_time > 1500) && (state->port_info.used_time != 0))
			{
				logout(INFO, "system", "check_port", "%s������%s��ʱ������25����,ϵͳ�жϸ��̿��ܴ����쳣����,���������������\r\n", state->port_info.port_str, state->port_info.used_info);
				sleep(10);
				ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
			}
		}
	}
	else if ((strcmp(state->port_info.used_info, DF_PORT_USED_ASJDCXHZXXB) == 0) || 
		(strcmp(state->port_info.used_info, DF_PORT_USED_AXXBBHCXHZXXB) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_CXHZXXB) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_SQHZXXB) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_DLYFPCX) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_DLYFPXZAZ) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_SQJRGGFWPT) == 0) || 
		(strcmp(state->port_info.used_info, DF_PORT_USED_CXJRGGFWPT) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USED_FPKJ) == 0) ||
		(strcmp(state->port_info.used_info, DF_PORT_USED_FPZF) == 0))

	{
		if (time_now - state->port_info.used_time > 80)
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������1����,ϵͳ�жϸ��̿��ܴ����쳣,������%d�˿ڽ��жϵ���������\r\n", state->port_info.port_str, state->port_info.used_info,state->port);
			ty_usb_m_out("����%d�Ŷ˿ڵ�Դ\n", state->port);
			restart_plate_power(state, state->port_info.sn, state->port, 0);
		}
		if ((time_now - state->port_info.used_time > 300) && (state->port_info.used_time != 0))
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������5����,ϵͳ�жϸ��̿��ܴ����쳣����,���������������\r\n", state->port_info.port_str, state->port_info.used_info);
			sleep(10);
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		}
	}


	return 0;
}

//�˿ڼ�鴦���߼�
static int check_port(struct _port_state   *state,struct _ty_usb_m  *stream)
{
	int result;
	long time_now;
	if(_lock_set_no_dely(state->lock)<0)
        return 0;
    if(state->port==0)
    {   
		//printf("state->port=%d\r\n",state->port);
		_lock_un(state->lock);
        return 0;
    }
	if (state->port_info.usbip_state == 0)
    {  
		//printf("state->port_info.usbip_state=%d\r\n",state->port_info.usbip_state);
		_lock_un(state->lock);
        return 0;
    }
	if (state->port_info.port_status == 0)
	{
		//printf("state->port_info.port_status=%d\r\n",state->port_info.port_status);
		_lock_un(state->lock);
		return 0;
	}
	if (state->port_info.used_level!= 1)
	{
		//printf("state->port_info.used_level=%d\r\n",state->port_info.used_level);
		_lock_un(state->lock);
		return 0;
	}
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port);
	if (result<0 && state->sys_reload != 1)
	{
		logout(INFO, "system", "check", "%s,�˿ڴ��ż�⵽�豸���γ�\r\n", state->port_info.port_str);
		close_port_dettach(state->port);
		_lock_un(state->lock);
		return 0;
	}
	time_now = get_time_sec();
	//printf("time_now - state->port_info.last_open_time=%d\r\n",time_now - state->port_info.last_open_time);
	if ((time_now - state->port_info.last_open_time) > DF_TY_USB_DATA_MAX_TIMER_S)
	{
		result = ty_ctl(stream->tran_fd, DF_CHECK_PORT_LINE_STATUS, state->port);
		if (result < 0)
		{
			logout(INFO, "system", "check", "%s,ʹ��֮��,status�����err\r\n", state->port_info.port_str);
			close_port_dettach(state->port);
			_lock_un(state->lock);
			return 0;
		}
	}	

	_lock_un(state->lock);
    return 0;
}
//�˿ڴ򿪴����߼�
static int open_port(struct _ty_usb_m  *stream,int port,uint32 ip,uint8 *client_id)
{   
	char busid[DF_TY_USB_BUSID] = {0};
	int result;
	memset(busid,0,sizeof(busid));

	result=ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_PORT_BUSID,port,busid);
	if(result<0)
	{   ty_usb_m_out("[open_port]��ȡ�˿�[%d]busidʧ��\n",port);
		return result;
	}

	result=open_port_ty_usb(stream,port);
	if(result<0)
	{   
		ty_usb_m_out("[open_port]��ʧ��\n");
		return result;
	}

	return 1;
}
//�˿ڹرմ����߼�
static int close_port_socket(struct _ty_usb_m *stream,int port)
{
	struct _port_state   *state;
	state = &stream->port[port - 1];
	ty_usb_m_out("close_port_socket����ر�����,�˿ںš�%d��\n", port);
	_lock_set(stream->lock);
	state->port_info.with_dev = 1;//��������Ϊ�ر�
	_lock_un(stream->lock);
	ty_usb_m_out("***************���йر�socket����**************\n");
	if ((state->port_info.port_status == 0) && (state->port_info.used_level != 1) && (1 == state->port_info.tran_line_closed_flag))
	{
		ty_usb_m_out("�˿ڼ����Ѿ��رգ������ٴιر�\n");
		return 0;
	}

	state->port_info.port_used = 0;
	if (state->power_mode == Normal_Mode)
	{
		state->sys_reload = 0;
		close_port_ty_usb(stream, port);	//���ж�ֱ�ӹرյ�Դ

	}
	else
	{
		state->sys_reload = 0;
		port_power_down(state);
	}


	ty_usb_m_out("�ر�ת����·,�رն˿�=%d\n", port);
	state->port_info.tran_line_closed_flag = 0;
	ty_ctl(stream->tran_fd, DF_CLOSE_LINE_TRAN_TASK, port,&state->port_info.tran_line_closed_flag);

	_lock_set(stream->lock);

	state->port_info.close_port_flag = 1;		//wang 11/24

	state->port_info.usbip_state = 0;
	state->port_info.check_num = 0;
	state->port_info.port_status = 0x00;//�˿�״̬Ϊ�򿪹�,�����ѹر�   
	state->port_info.used_time = 0;
	memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	state->port_info.app_fd = 0;
	state->port_info.ip = 0;
	state->port_info.last_open_time = 0;
	state->port_info.system_used = 0;
	memset(state->port_info.client, 0, sizeof(stream->port[port - 1].port_info.client));
	state->port_info.usb_share_mode = 0;
	state->port_info.used_level = 0;
	_lock_un(stream->lock);
	//����豸��ʹ�ú͸��¶˿�״̬,port_unlock���������

	//ty_usb_m_out("֤��仯״̬= %d\n", stream->port[port - 1].port_info.change);
	ty_usb_m_out("[close_port_socket]�ر�%d�˿ڳɹ�\n", port);
	return 0;
}


//�����˿�ʹ��
static int port_lock(struct _port_state   *state,int second,int used_level,char *used_info)
{
		int result = 0;
	
	int i = 0;
	int count;
	_lock_set(state->lock);
	if (state->port_info.with_dev != 1)
	{
		_lock_un(state->lock);
		return -1;
	}
	_lock_un(state->lock);
	if (second != 0)
	{
		count = second * 1000;
		for (i = 0; i < count; i++, usleep(1000))
		{			
			//if (state->port_info.port_used == -1  )	  //�˿ڼ��غ��ͷ��������µ�δ��ȡ��������Ϣ
			//{
			//	ty_usb_m_out("\n\n\n�˿ڼ��غ��ͷ��������µ磬�ȴ���ȡ��������Ϣ��ִ�б���[%s]����\n\n\n", used_info);
			//	for (; i < count; i++, usleep(1000))
			//	{
			//		if (state->port_info.port_used == 0)//�Ѿ���ȡ��������Ϣ,used�ָ���0
			//			break;
			//	}
			//	continue;//���뵽�´��������
			//}

			if (state->port_info.port_used != 0)
			{
				//ty_usb_m_out("�˿��ѱ�����,�޷�����\n");
				//ty_usb_m_out("��ǰ�ȼ�%d,����ʹ��ʱ�ĵȼ�%d\n", used_level, state->port_info.used_level);
				if (used_level<state->port_info.used_level)
				{
					//ty_usb_m_out("��ǰ�ȼ�˵����%s��,����ʹ��ʱ�ĵȼ���%s��,DF_PORT_USED_FPKJ = %s,DF_PORT_USED_FPCX_MQTT= %s\n", used_info, state->port_info.used_info, DF_PORT_USED_FPKJ, DF_PORT_USED_FPCX_MQTT);
					if (strcmp(state->port_info.used_info, DF_PORT_USED_FPCX_SCMB) == 0)
					{

						if (
							(strcmp(used_info, DF_PORT_USED_USB_SHARE) == 0) ||
							(strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0) ||
							(strcmp(used_info, DF_PORT_USED_FPKJ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_FPZF) == 0) ||		
							(strcmp(used_info, DF_PORT_USED_FPHM_SCFP) == 0) ||
							(strcmp(used_info, DF_PORT_USED_FPYJCXBLJ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_DLYFPCX) == 0) ||
							(strcmp(used_info, DF_PORT_USED_DLYFPXZAZ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_MRKLRZ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_HQSPDQSJ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_CXDQFPHM) == 0) ||
							(strcmp(used_info, DF_PORT_USED_AYCXFPSLMQTT) == 0) ||
							(strcmp(used_info, DF_PORT_USED_AYCXFPSJMQTT) == 0) ||
							(strcmp(used_info, DF_PORT_USED_AFPHMCXFP) == 0) ||
							(strcmp(used_info, DF_PORT_USED_CXHZSJ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_HPKJLZHMJY) == 0) ||
							(strcmp(used_info, DF_PORT_USED_SQHZXXB) == 0) ||
							(strcmp(used_info, DF_PORT_USED_CXHZXXB) == 0) ||
							(strcmp(used_info, DF_PORT_USED_ASJDCXHZXXB) == 0) ||
							(strcmp(used_info, DF_PORT_USED_AXXBBHCXHZXXB) == 0)||
							(strcmp(used_info, DF_PORT_USED_CSLJ) == 0) ||
							(strcmp(used_info, DF_PORT_USED_SQJRGGFWPT) == 0) ||
							(strcmp(used_info, DF_PORT_USED_CXJRGGFWPT) == 0) //||
							//(strcmp(used_info, DF_PORT_USED_LXSC) == 0)
							)
						{
							if (state->port_info.inv_query != 0)
							{
								logout(INFO, "system", "�ж�", "%s,ԭ����Ϊ:%s,�ֲ���Ϊ:%s,�漴�жϷ�Ʊ��ѯ��ͬ��\r\n",state->port_info.port_str, state->port_info.used_info, used_info);
								state->port_info.inv_query = 0;
								sleep(1);
							}
						}
					}
					if (state->port_info.used_level == 4)
					{
						if ((strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0) ||
							(strcmp(state->port_info.used_info, DF_PORT_USED_HZSC) == 0) ||
							(strcmp(state->port_info.used_info, DF_PORT_USED_FXJK) == 0) ||
							(strcmp(state->port_info.used_info, DF_PORT_USED_CSLJ) == 0))
						{
							//ty_usb_m_out("��⵽����ռ�ã�ֱ��ǿ�ƽ�����\n");
							if (state->h_plate.hDev != NULL)
							{
								state->h_plate.hDev->bBreakAllIO = 1;
							}
						}
					}
				}
				continue;
			}
			if ((state->power == 0) &&(state->h_plate.need_free == 1))
			{
				//�˿�δ��ȫ�ͷ���ɣ��ȴ�
				continue;
			}
			_lock_set(state->lock);
			if (state->port_info.port_used == 1)
			{
				_lock_un(state->lock);//��ֹͬʱ����
				continue;
			}
			state->port_info.used_time = get_time_sec();
			state->port_info.port_used = 1;
			if (used_level == 1) //USB-SHAREʹ��
			{
				state->port_info.system_used = 0;
			}
			else
			{
				state->port_info.system_used = 1;
			}
			state->port_info.port_status = 2;
			state->port_info.used_level = used_level;
			memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
			memcpy(state->port_info.used_info, used_info, strlen(used_info));
			_lock_un(state->lock);
			if (state->power_mode == Saving_Mode)
			{
				//logout(INFO, "port_lock", "�˿�����ʹ��", "port_lock ��ʱģʽ %d�Ŷ˿��ٴ��ϵ��ȡ��USB�����%s��\n", state->port, used_info);
				result = port_power_up(state);
				if (result < 0)
				{
					_lock_set(state->lock);
					state->port_info.used_time = 0;
					state->port_info.port_used = 0;
					state->port_info.system_used = 0;
					state->port_info.port_status = 0;
					state->port_info.used_level = 0;
					memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
					_lock_un(state->lock);
					return result;
				}
				//logout(INFO, "port_lock", "�˿�����ʹ��", "port_lock ��ʱģʽ %d�Ŷ˿��ٴ��ϵ��ȡ��USB����ɹ���%s��\n", state->port, used_info);
				
			}
			return 0;
		}
	}
	else
	{
		_lock_set(state->lock);
		if (state->port_info.port_used != 0)
		{
			//ty_usb_m_out("�˿��ѱ�����,�޷�����\n");
			if (strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0)
			{
				ty_usb_m_out("%d�˿ںŶ�ȡ������Ϣ�����˿�\n",state->port);;
			}
			else
			{
				_lock_un(state->lock);
				return -1;
			}
		}
		state->port_info.used_time = get_time_sec();
		state->port_info.port_used = 1;
		if (used_level == 1) //USB-SHAREʹ��
		{
			state->port_info.system_used = 0;
		}
		else
		{
			state->port_info.system_used = 1;
		}
		state->port_info.port_status = 2;
		state->port_info.used_level = used_level;
		memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
		memcpy(state->port_info.used_info, used_info, strlen(used_info));
		_lock_un(state->lock);
		if (state->power_mode == Saving_Mode)
		{
			//logout(INFO, "port_lock", "�˿�����ʹ��", "port_lock �ǳ�ʱģʽ %d�Ŷ˿��ٴ��ϵ��ȡ��USB�����%s��\n", state->port, used_info);
			result = port_power_up(state);
			if (result < 0)
			{
				_lock_set(state->lock);
				state->port_info.used_time = 0;
				state->port_info.port_used = 0;
				state->port_info.system_used = 0;
				state->port_info.port_status = 0;
				state->port_info.used_level = 0;
				memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
				_lock_un(state->lock);
				return result;
			}
			//logout(INFO, "port_lock", "�˿�����ʹ��", "port_lock �ǳ�ʱģʽ %d�Ŷ˿��ٴ��ϵ��ȡ��USB����ɹ���%s��\n", state->port, used_info);
		}
		return 0;
	}
	return -1;


	// struct _ty_usb_m  *stream;
	// stream = ty_usb_m_id;
	// int result = 0;
	
	// int i = 0;
	// int count;
	// _lock_set(state->lock);
	// if (state->port_info.with_dev != 1)
	// {
	// 	_lock_un(state->lock);
	// 	return -1;
	// }
	// _lock_un(state->lock);
	// if (second != 0)
	// {
	// 	count = second * 1000;
	// 	for (i = 0; i < count; i++, usleep(1000))
	// 	{
	// 		_lock_set(state->lock);

			
	// 		if (state->port_info.port_used == -1  )	  //�˿ڼ��غ��ͷ��������µ�δ��ȡ��������Ϣ
	// 		{
	// 			ty_usb_m_out("\n\n\n�˿ڼ��غ��ͷ��������µ磬�ȴ���ȡ��������Ϣ��ִ�б���[%s]����\n\n\n", used_info);
	// 			_lock_un(state->lock);
	// 			for (; i < count; i++, usleep(1000))
	// 			{
	// 				if (state->port_info.port_used == 0)//�Ѿ���ȡ��������Ϣ,used�ָ���0
	// 					break;
	// 			}
	// 			continue;//���뵽�´��������

	// 		}

	// 		if (state->port_info.port_used != 0)
	// 		{
	// 			//ty_usb_m_out("�˿��ѱ�����,�޷�����\n");
	// 			//ty_usb_m_out("��ǰ�ȼ�%d,����ʹ��ʱ�ĵȼ�%d\n", used_level, state->port_info.used_level);
	// 			if (used_level<state->port_info.used_level)
	// 			{
	// 				//ty_usb_m_out("��ǰ�ȼ�˵����%s��,����ʹ��ʱ�ĵȼ���%s��,DF_PORT_USED_FPKJ = %s,DF_PORT_USED_FPCX_MQTT= %s\n", used_info, state->port_info.used_info, DF_PORT_USED_FPKJ, DF_PORT_USED_FPCX_MQTT);
	// 				if (strcmp(state->port_info.used_info, DF_PORT_USED_FPCX_SCMB) == 0)
	// 				{

	// 					if (
	// 						(strcmp(used_info, DF_PORT_USED_USB_SHARE) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_FPKJ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_FPZF) == 0) ||		
	// 						(strcmp(used_info, DF_PORT_USED_FPHM_SCFP) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_FPYJCXBLJ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_DLYFPCX) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_DLYFPXZAZ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_MRKLRZ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_HQSPDQSJ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_CXDQFPHM) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_AYCXFPSLMQTT) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_AYCXFPSJMQTT) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_AFPHMCXFP) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_CXHZSJ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_HPKJLZHMJY) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_SQHZXXB) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_CXHZXXB) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_ASJDCXHZXXB) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_AXXBBHCXHZXXB) == 0)||
	// 						(strcmp(used_info, DF_PORT_USED_CSLJ) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_SQJRGGFWPT) == 0) ||
	// 						(strcmp(used_info, DF_PORT_USED_CXJRGGFWPT) == 0) //||
	// 						//(strcmp(used_info, DF_PORT_USED_LXSC) == 0)
	// 						)
	// 					{
	// 						if (state->port_info.inv_query != 0)
	// 						{
	// 							logout(INFO, "system", "�ж�", "%s,ԭ����Ϊ:%s,�ֲ���Ϊ:%s,�漴�жϷ�Ʊ��ѯ��ͬ��\r\n",state->port_info.port_str, state->port_info.used_info, used_info);
	// 							state->port_info.inv_query = 0;
	// 							sleep(1);
	// 						}
	// 					}
	// 				}
	// 				if (state->port_info.used_level == 4)
	// 				{
	// 					if ((strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0) ||
	// 						(strcmp(state->port_info.used_info, DF_PORT_USED_HZSC) == 0) ||
	// 						(strcmp(state->port_info.used_info, DF_PORT_USED_FXJK) == 0) ||
	// 						(strcmp(state->port_info.used_info, DF_PORT_USED_CSLJ) == 0))
	// 					{
	// 						//ty_usb_m_out("��⵽����ռ�ã�ֱ��ǿ�ƽ�����\n");
	// 						if (state->h_plate.hDev != NULL)
	// 						{
	// 							state->h_plate.hDev->bBreakAllIO = 1;
	// 						}
	// 					}
	// 				}
	// 			}
	// 			_lock_un(state->lock);
	// 			continue;
	// 		}
	// 		state->port_info.used_time = get_time_sec();
	// 		state->port_info.port_used = 1;
	// 		if (used_level == 1) //USB-SHAREʹ��
	// 		{
	// 			state->port_info.system_used = 0;
	// 		}
	// 		else
	// 		{
	// 			state->port_info.system_used = 1;
	// 		}
	// 		state->port_info.port_status = 2;
	// 		state->port_info.used_level = used_level;
	// 		memcpy(state->port_info.used_info, used_info, strlen(used_info));
	// 		_lock_un(state->lock);
	// 		if (state->power_mode == Saving_Mode)
	// 		{
	// 			if (state->down_flag != 2)//Ϊ2 �����Ƕ�ȡ֤����Ϣ �Ѿ���task_check_port�Ϲ�����
	// 			{
	// 				ty_usb_m_out("�ٴ��ϵ��ȡ�¾��\n");
	// 				result = port_power_up(stream, state->port, state->port_info.usb_app);
	// 				if (result < 0)
	// 				{
	// 					if (result != -99)
	// 						port_unlock(state);
	// 					else
	// 					{
	// 						state->port_info.used_time = 0;
	// 						state->port_info.port_used = 0;
	// 						state->port_info.system_used = 0;
	// 						state->port_info.port_status = 0;
	// 						state->port_info.used_level = 0;
	// 						memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	// 					}
	// 					//sleep(1);//���ܴ��ڶ˿ڱ��γ����,�ȴ��µ��߳��µ缰��⵽�˿ڰγ�����!
	// 					//sleep(3);
	// 					return POWER_ERR_NO_DEV;
	// 				}
	// 			}
	// 		}
	// 		//if (((state->h_plate.nBusID == -1) || (state->h_plate.nDevID == -1)) || ((state->h_plate.nBusID == 0) || (state->h_plate.nDevID == 0)))
	// 		//{
	// 		//	state->port_info.port_used = 0;
	// 		//	state->port_info.used_level = 0;
	// 		//	//logout(INFO, "port_lock", "�˿�����ʹ��", "USB�˿�%d,ԭ��¼��busnum = %d,devnum = %d,���������˿ڷ����������\r\n", state->port,state->h_plate.nBusID, state->h_plate.nDevID);
	// 		//	continue;
	// 		//}
	// 		return 0;
	// 	}
	// }
	// else
	// {
	// 	_lock_set(state->lock);
	// 	if (state->port_info.port_used != 0)
	// 	{
	// 		//ty_usb_m_out("�˿��ѱ�����,�޷�����\n");
	// 		if (state->port_info.port_used == -1 && strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0)
	// 		{
	// 			ty_usb_m_out("�˿ڼ��غ��ͷ��������µ��ȡ������Ϣ\n");;
	// 		}
	// 		else
	// 		{
	// 			_lock_un(state->lock);
	// 			return -1;
	// 		}
	// 	}
	// 	state->port_info.used_time = get_time_sec();
	// 	state->port_info.port_used = 1;
	// 	if (used_level == 1) //USB-SHAREʹ��
	// 	{
	// 		state->port_info.system_used = 0;
	// 	}
	// 	else
	// 	{
	// 		state->port_info.system_used = 1;
	// 	}
	// 	state->port_info.port_status = 2;
	// 	state->port_info.used_level = used_level;
	// 	memcpy(state->port_info.used_info, used_info, strlen(used_info));
	// 	_lock_un(state->lock);
	// 	if (state->power_mode == Saving_Mode)
	// 	{
	// 		if (state->down_flag != 2)//Ϊ2 �����Ƕ�ȡ֤����Ϣ �Ѿ���task_check_port�Ϲ�����
	// 		{
	// 			result = port_power_up(stream, state->port, state->port_info.usb_app);
	// 			if (result < 0)
	// 			{
	// 				if (result != -99)
	// 					port_unlock(state);
	// 				else
	// 				{
	// 					state->port_info.used_time = 0;
	// 					state->port_info.port_used = 0;
	// 					state->port_info.system_used = 0;
	// 					state->port_info.port_status = 0;
	// 					state->port_info.used_level = 0;
	// 					memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	// 				}
	// 				//sleep(1);//���ܴ��ڶ˿ڱ��γ����,�ȴ��µ��߳��µ缰��⵽�˿ڰγ�����!
	// 				return POWER_ERR_NO_DEV;
	// 			}
	// 		}
	// 	}
	// 	return 0;
	// }
	// return -1;
}

static void close_port_power_saving_mode(void *arg)
{
	int time = 0;
	struct _port_state   *state;
	state = arg;

	for (time = 0; time < 5000; time++)
	{
		if (state->port_info.port_used != 0)
			break;
		if (state->power == 0)
			break;
		usleep(1000);
	}
	if (time < 5000)
	{
		ty_usb_m_out("%d�Ŷ˿��ͷ�ʱ�������ӿ�ʹ�ã��ݲ��رն˿ڵ�Դ\n",state->port);
		return;
	}
	state->sys_reload = 1; //�ر��豸������Ϣ
	//ty_usb_m_out("%d�Ŷ˿�ʹ�ý������رն˿ڵ�Դ\n",state->port);
	port_power_down(state);
	//ty_usb_m_out("%d�Ŷ˿�ʹ�ý������رն˿ڵ�Դ���\n", state->port);

}

//�����˿�ʹ��
static int port_unlock(struct _port_state   *state)
{
	//_lock_set(state->lock);
	state->port_info.used_time = 0;
	state->port_info.port_used = 0;
	state->port_info.system_used = 0;
	state->port_info.port_status = 0;
	state->port_info.used_level = 0;
	memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	//_lock_un(state->lock);
	if (state->power_mode == Saving_Mode)
	{
		_delay_task_add("up_m_server", close_port_power_saving_mode, state, 10);		
	}
	return 0;

// 	//_lock_set(state->lock);
// //	logout(INFO, "port_unlock", "�˿��ͷ�ʹ��", "�˿�:%d ˰�����ƣ�%s,����Ϊ:%s\r\n", state->port,state->port_info.ca_name, state->port_info.used_info);
// 	state->port_info.used_time = 0;
// 	state->port_info.port_used = 0;
// 	state->port_info.system_used = 0;
// 	state->port_info.port_status = 0;
// 	state->port_info.used_level = 0;
// 	memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
// 	if (state->power_mode == Saving_Mode)
// 	{
// 		//logout(INFO, "port_unlock", "�˿��ͷ�ʹ��", "˰�����ƣ�%s,����Ϊ:%s\r\n", state->port_info.ca_name, state->port_info.used_info);
// 		if (state->down_flag != 2)
// 		{
// 			//free_h_plate(state);
// 		}
// 	}
// 	//_lock_un(state->lock);
// 	if(state->power_mode == Saving_Mode)
// 	{
// 		port_down_power(state);
// 	}
// 	return 0;
// }
// static int port_down_power(struct _port_state   *state)
// {
// 	if (state->down_flag != 2 && state->status == 1)
// 	{
// 		_lock_set(state->power_lock);
// 		state->down_flag = 1;
// 		state->power_status = 1;
// 		state->last_up_power_time = get_time_sec();
// 		_lock_un(state->power_lock);
// 	}
// 	return 0;
}







//***================================ȫ�ֺ���===================================***//
//***===========================================================================***//
//���ö˿�USB�����ݴ���ָʾled��
int set_port_usb_data_tran(int port)
{
	struct _ty_usb_m  *stream;
	//ty_usb_m_out("������USB����,�˿ں�%d\n",port);
	stream = ty_usb_m_id;
	stream->port[port - 1].port_info.tran_data = 1;
	return 0;
}

int get_transport_line(struct _new_usbshare_parm *parm)
{
	int result;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	ty_usb_m_out("get_transport_line host:%s--port:%d-- clientid:%s\n", parm->trans_server, parm->port, parm->client_id);

	result = ty_ctl(stream->tran_fd, DF_OPEN_TRANSPORT_LINE, parm);//������ع���usbָ��
	if (result < 0)
	{
		return result;
		ty_usb_m_out("open_port_transport error DF_MQTT_REQUEST_SHARE_USB_PORT!\n");
	}
	return result;
}

int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	stream = ty_usb_m_id;
	state = &stream->port[port_num - 1];
	//update_plate_info_cbqkzt(state);
	//ty_usb_m_out("�˿ںţ�%d,��Դ״̬��%d,�Ƿ�����豸��%d\n", state->port_info.port, state->port_info.power, state->port_info.with_dev);
	memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
	port_info->port_power = state->power;
	port_info->port = state->port;//�˿ڽṹ����Ϣ����״̬�ṹ����ܲ�ͬ��
	_lock_set(state->lock);
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
	_lock_un(state->lock);
	port_info->ad_status = state->port_info.ad_status;

	return 0;
}

//��ʱ�ϴ��ն�״̬�����߳�
static void timely_tigger_report_status(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	int time_now;
	stream = arg;
	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	if (net_status.nettime_ok != 1)
	{
		//ty_usb_m_out("��������δ����,�Ժ������ϱ��ն�״̬\n");
		return ;
	}

	time_now = get_time_sec();
	
	//ty_usb_m_out("��ʼִ���������ϱ�\n");
	if (time_now - stream->last_report_time_o > DF_TY_USB_MQTT_REPORT_STATUS-1)//����15��ᴥ��һ���ϱ�
	{
		//ty_usb_m_out("����һ��״̬�ϱ�\n");
		if (stream->report_ter_flag != 1)
		{
			if (stream->report_o_used == 0)
			{
				//pack_report_json_data(stream);
				//ty_usb_m_out("����һ���ն���״̬��Ϣ�ϱ�\n");
				if (_delay_task_add("0", pack_report_json_data, stream, 0)<0)
				{
					ty_usb_m_out("��������ʧ��\n");
				}
			}
		}
		//����Ѿ��ϱ�M������Ϣ�ɹ����ڽ����ϰ汾���������ϱ�,�����˴���Ŀ��Ϊ���ݸߵ�ֱ��MQTT�Խ�
		
	}
	if (time_now - stream->last_report_time_h > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT-1)//����60��ᴥ��һ���ϱ�
	{
		//ty_usb_m_out("����һ��M��������\n");
		//ty_usb_m_out("����һ���ն�������Ϣ�ϱ�\n");
		if (stream->report_h_used == 0)
		{
			if (_delay_task_add("0", pack_report_m_server_heart_data, stream, 0)<0)
			{
				ty_usb_m_out("��������ʧ��\n");
			}
		}
	}
	if (time_now - stream->last_report_time_s > DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT - 1)
	{
		//ty_usb_m_out("���һ��M�����ն˻�����Ϣ�ϱ�\n");
		if ((stream->report_s_used == 0) && (stream->report_ter_flag == 0))
		{
			ty_usb_m_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
			stream->report_s_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			if (_delay_task_add("0", pack_report_m_server_status_data, stream, 0)<0)
			{
				ty_usb_m_out("��������ʧ��\n");
			}
			//pack_report_m_server_status_data(stream);
		}
	}
}

static void pack_report_json_data(void *arg)
{
	struct _ty_usb_m  *stream;
	char *g_buf;
	char tmp[20] = { 0 };
	int port_count;
	char source_topic[50] = { 0 };
	stream = arg;
	stream->report_o_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����

	cJSON *dir1, *dir2, *dir3, *dir4;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	memset(source_topic, 0, sizeof(source_topic));

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", DF_MQTT_SYNC_REPORT_TER_STATUS);
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", "123456789");
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	cJSON_AddStringToObject(dir3, "ter_id", stream->ter_id);
	cJSON_AddStringToObject(dir3, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(dir3, "ter_mac", net_status.mac);
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(dir3, "usb_port_num", tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", DF_TY_USB_MQTT_REPORT_STATUS);
	cJSON_AddStringToObject(dir3, "report_time", tmp);

	cJSON_AddItemToObject(dir3, "usb_port_info", dir4 = cJSON_CreateArray());
	for (port_count = 0; port_count < stream->usb_port_numb; port_count++)
	{
		int sp_type = stream->port[port_count].port_info.usb_app;
		cJSON *p_layer;
		cJSON_AddItemToObject(dir4, "dira", p_layer = cJSON_CreateObject());
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", port_count + 1);
		cJSON_AddStringToObject(p_layer, "usb_port", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].power);
		cJSON_AddStringToObject(p_layer, "port_power", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].port_info.with_dev);
		cJSON_AddStringToObject(p_layer, "dev_exists", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].port_info.dev_type);
		cJSON_AddStringToObject(p_layer, "usb_type", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", stream->port[port_count].port_info.vid);
		cJSON_AddStringToObject(p_layer, "usb_vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", stream->port[port_count].port_info.pid);
		cJSON_AddStringToObject(p_layer, "usb_pid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].port_info.ca_ok);
		cJSON_AddStringToObject(p_layer, "ca_support", tmp);
		if (stream->port[port_count].port_info.ca_ok != 0)
		{
			cJSON_AddStringToObject(p_layer, "ca_name", (char *)stream->port[port_count].port_info.ca_name);
			char tmp_s[50] = { 0 };
			memset(tmp_s, 0, sizeof(tmp_s));
			memcpy(tmp_s, stream->port[port_count].port_info.ca_serial, sizeof(stream->port[port_count].port_info.ca_serial));
			cJSON_AddStringToObject(p_layer, "ca_number", (char *)tmp_s);
			//if ((stream->port[port_count].port_info.vid == 0x101d) && (stream->port[port_count].port_info.pid == 0x0003))
			if (sp_type == DEVICE_AISINO)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "1");
			}
			//else if ((stream->port[port_count].port_info.vid == 0x1432) && (stream->port[port_count].port_info.pid == 0x07dc))
			else if (sp_type == DEVICE_NISEC)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "2");
			}
			//else if ((stream->port[port_count].port_info.vid == 0x0680) && (stream->port[port_count].port_info.pid == 0x1901))
			else if (sp_type == DEVICE_MENGBAI)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "3");
			}
			else if (sp_type == DEVICE_CNTAX)
			{
				char sp_type_s[20] = {0};
				sprintf(sp_type_s, "%d", sp_type);
				cJSON_AddStringToObject(p_layer, "ca_type", sp_type_s);
			}
			else
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "0");
			}
		}
	}
	g_buf = cJSON_Print(json);
	str_replace(g_buf, "\\\\", "\\");
	_m_pub_no_answer("mb_fpkjxt_kpfw", g_buf);			//mqtt pub ����:mb_fpkjxt_kpfw
	stream->last_report_time_o = get_time_sec();
	stream->report_o_used = 0;
	//ty_usb_m_out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);
	return ;
}

static void pack_report_m_server_heart_data(void *arg)
{
	struct _ty_usb_m  *stream;
	char *g_buf;
	stream = arg;
	stream->report_h_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "ter_id", stream->ter_id);
	g_buf = cJSON_PrintUnformatted(json);
	//ty_usb_m_out("g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	_m_pub_no_answer("mb_equ_ter_heart_up", g_buf);
	stream->last_report_time_h = get_time_sec();
	stream->report_h_used = 0;
	free(g_buf);
	cJSON_Delete(json);
	return ;
}

static void pack_report_m_server_status_data(void *arg)
{
	struct _ty_usb_m  *stream;
	char *g_buf;
	//int port_count;
	char source_topic[50] = { 0 };
	int result;
	char random[50] = {0};
	stream = arg;
	
	memset(source_topic, 0, sizeof(source_topic));
	
	get_radom_serial_number("zdsbztxx", random);

	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbztxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	fun_get_ter_base_data_json(dir3);


	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//ty_usb_m_out("g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = {0};
	ty_usb_m_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data);
	stream->last_report_time_s = get_time_sec();
	if (result < 0)
	{		
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,��������Ӧ��ʱ\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,������Ӧ����ʧ��\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End ;
	}
	else
	{
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,�ɹ�\r\n");
		stream->report_ter_flag = 1;
		ty_usb_m_out("�ϱ�M�����ն�״̬����,������Ӧ����ɹ�\n");
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_s_used = 0;
	return ;
}

#ifndef DF_OLD_MSERVER
static void pack_report_ca_info_new_mqtt(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_pub_buf;
	char *g_new_buf;
	char tmp[20];
	char source_topic[50];
	int result;
	char random[50] = { 0 };
	state = arg;
	stream = ty_usb_m_id;
	stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
	memset(source_topic, 0, sizeof(source_topic));
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		if (state->last_act == 1)//������������Ȳ��������ϴ�
			off_up_flag = 1;
		if (state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//���������ϴ�
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("zdsbcaxx", random);
	cJSON *json_data = cJSON_CreateObject();

	cJSON_AddStringToObject(json_data, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json_data, "port_num", tmp);


	if ((result = jude_port_allow_attach(state)) < 0)
	{
		cJSON_AddStringToObject(json_data, "tax_auth","0");
	}
	else
	{
		cJSON_AddStringToObject(json_data, "tax_auth","1");
	}

	cJSON_AddStringToObject(json_data, "tax_auth", tmp);
	cJSON_AddStringToObject(json_data, "start_time", stream->start_time);
	cJSON_AddStringToObject(json_data, "end_time", stream->end_time);


	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json_data, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json_data, "client_ip", "");
	}

	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port_last_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(json_data, "usb_app", tmp);
		//#endif

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(json_data, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(json_data, "pid", tmp);
		#ifndef DF_SUPPORT_CANAME				//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��
		if (strlen((char *)state->port_last_info.ca_name) == 0)
		{
			cJSON_AddStringToObject(json_data, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "name", (char *)state->port_last_info.ca_name);
		if (strlen((char *)state->port_last_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json_data, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "number", (char *)state->port_last_info.ca_serial);
		if (strlen((char *)state->port_last_info.sn) == 0)
		{
			cJSON_AddStringToObject(json_data, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "serial", (char *)state->port_last_info.sn);
		#endif
		//printf("step1\n");

		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json_data, "report_serial", tmp);

		cJSON_AddStringToObject(json_data, "usb_share", "0");
		cJSON_AddStringToObject(json_data, "usb_share_mode", "0");
		cJSON_AddStringToObject(json_data, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json_data, "offline", "0");
		//printf("step2\n");
	}
	else
	{
		//�����ϱ�

		//wang 11/10
		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json_data, "report_serial", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(json_data, "usb_app", tmp);
		//#endif
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json_data, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json_data, "pid", tmp);
		#ifndef DF_SUPPORT_CANAME				//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��
		if (strlen((char *)state->port_info.ca_name) == 0)
		{
			//ty_usb_m_out("#####################ca_name :%s###########\n",)
			cJSON_AddStringToObject(json_data, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "name", (char *)state->port_info.ca_name);
		if (strlen((char *)state->port_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json_data, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "number", (char *)state->port_info.ca_serial);
		if (strlen((char *)state->port_info.sn) == 0)
		{
			cJSON_AddStringToObject(json_data, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "serial", (char *)state->port_info.sn);
		#endif
		printf("lbc---------------------state->port_info.port_used=%d state->port_info.system_used=%d\n",state->port_info.port_used,state->port_info.system_used);
		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(json_data, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(json_data, "usb_share_mode", tmp);
		cJSON_AddStringToObject(json_data, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json_data, "offline", "1");
	}

	g_new_buf = cJSON_PrintUnformatted(json_data);
	cJSON_Delete(json_data);

	cJSON *port_array, *port_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "ca");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);			//014b
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);	
		free(g_new_buf);				
	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	//ty_usb_m_out("�ϱ�ca���ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json);

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", stream->ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_pub_buf = cJSON_PrintUnformatted(root);
	//ty_usb_m_out("�ϱ�ca�������ݣ�%s\n",g_pub_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_pub_buf);
	if (result < 0)
	{
		ty_usb_m_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��\n");
		free(g_pub_buf);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (off_up_flag == 1)
			state->off_report = 0;
		else
			state->up_report = 0;
		ty_usb_m_out("�ϴ���%d�ſڣ�%s��CA��Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	sleep(1);				//wang 11/24
	//ty_usb_m_out("�����������\n");
	free(g_pub_buf);
	cJSON_Delete(root);

End:
	ty_usb_m_out("����ca�ϱ�����1\n");
	stream->report_p_used = 0;
	return;
}
#endif

static void pack_report_ca_info(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf;
	char tmp[20];
	char source_topic[50];
	int result;
	char random[50] = { 0 };
	state = arg;
	stream = ty_usb_m_id;
	stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
	memset(source_topic, 0, sizeof(source_topic));
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		if (state->last_act == 1)//������������Ȳ��������ϴ�
			off_up_flag = 1;
		if (state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//���������ϴ�
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("zdsbcaxx", random);
	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbcaxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(dir3, "port_num", tmp);


	if ((result = jude_port_allow_attach(state)) < 0)
	{
		cJSON_AddStringToObject(dir3, "tax_auth","0");
	}
	else
	{
		cJSON_AddStringToObject(dir3, "tax_auth","1");
	}

	cJSON_AddStringToObject(dir3, "tax_auth", tmp);
	cJSON_AddStringToObject(dir3, "start_time", stream->start_time);
	cJSON_AddStringToObject(dir3, "end_time", stream->end_time);
	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(dir3, "usb_app", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(dir3, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(dir3, "pid", tmp);
		

		if (strlen((char *)state->port_last_info.ca_name) == 0)
		{
			cJSON_AddStringToObject(dir3, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "name", (char *)state->port_last_info.ca_name);
		if (strlen((char *)state->port_last_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(dir3, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "number", (char *)state->port_last_info.ca_serial);
		if (strlen((char *)state->port_last_info.sn) == 0)
		{
			cJSON_AddStringToObject(dir3, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "serial", (char *)state->port_last_info.sn);

		cJSON_AddStringToObject(dir3, "usb_share", "0");
		cJSON_AddStringToObject(dir3, "usb_share_mode", "0");
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
		cJSON_AddStringToObject(dir3, "offline", "0");
	}
	else
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(dir3, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(dir3, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(dir3, "pid", tmp);

		if (strlen((char *)state->port_info.ca_name) == 0)
		{
			cJSON_AddStringToObject(dir3, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "name", (char *)state->port_info.ca_name);
		if (strlen((char *)state->port_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(dir3, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "number", (char *)state->port_info.ca_serial);
		if (strlen((char *)state->port_info.sn) == 0)
		{
			cJSON_AddStringToObject(dir3, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(dir3, "serial", (char *)state->port_info.sn);
		
		printf("lbc---------------------state->port_info.port_used=%d state->port_info.system_used=%d\n",state->port_info.port_used,state->port_info.system_used);
		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(dir3, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(dir3, "usb_share_mode", tmp);
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
		cJSON_AddStringToObject(dir3, "offline", "1");
	}



	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//ty_usb_m_out("pack_report_ca_info g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_equ_ca_status_up", g_buf, g_data);
	if (result < 0)
	{
		ty_usb_m_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		ty_usb_m_out("������Ӧ����ʧ��,���Ӵ����������������ʱ��\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else
	{
		if (off_up_flag == 1)
			state->off_report = 0;
		else
			state->up_report = 0;
		logout(INFO, "MQTT", "JSON_UPDATE", "[%d]�Ŷ˿�״̬CA���»������ϱ��ɹ�\r\n", state->port);
		ty_usb_m_out("�ϴ���%d�ſڣ�%s��CA��Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	sleep(1);				//wang 11/24
	//ty_usb_m_out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	ty_usb_m_out("����ca�ϱ�����1\n");
	stream->report_p_used = 0;
	return;



// 	struct _port_state   *state;
// 	struct _ty_usb_m  *stream;
// 	char *g_buf;
// 	char tmp[20],tmp_s[30];
// 	char source_topic[50];
// 	int result;
// 	char random[50] = { 0 };
// 	state = arg;
// 	stream = ty_usb_m_id;
// 	stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
// 	memset(source_topic, 0, sizeof(source_topic));
// 	int off_up_flag = 0;
// 	if ((state->off_report == 1) && (state->up_report == 1))
// 	{
// 		if (state->last_act == 1)//������������Ȳ��������ϴ�
// 			off_up_flag = 1;
// 		if (state->last_act == 2)//������������Ȳ��������ϴ�
// 			off_up_flag = 2;
// 	}
// 	else if ((state->off_report == 1) && (state->up_report == 0))
// 		off_up_flag = 1;//���������ϴ�
// 	else if ((state->off_report == 0) && (state->up_report == 1))
// 		off_up_flag = 2;//���������ϴ�
// 	else
// 	{
// 		goto End;
// 	}
// 	struct _ter_info ter_info;
// 	memset(&ter_info, 0, sizeof(struct _ter_info));
// 	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

// 	get_radom_serial_number("zdsbcaxx", random);
// 	cJSON *dir1, *dir2, *dir3;		//���ʹ��
// 	cJSON *json = cJSON_CreateObject();
// 	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

// 	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
// 	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
// 	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
// 	cJSON_AddStringToObject(dir1, "cmd", "zdsbcaxx");
// 	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
// 	cJSON_AddStringToObject(dir1, "random", random);
// 	cJSON_AddStringToObject(dir1, "result", "s");
// 	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

// 	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


// 	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
// 	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
// 	cJSON_AddStringToObject(dir3, "port_num", tmp);


// 	char errinfo[1024]={0};
// #ifdef RELEASE_SO
// 	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// #else
// 	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// #endif
// 	if (result != 1)
// 	{
// 		cJSON_AddStringToObject(dir3, "tax_auth", "0");
// 	}
// 	else
// 	{
// 		cJSON_AddStringToObject(dir3, "tax_auth", "1");
// 	}

// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->aisino_sup);
// 	cJSON_AddStringToObject(dir3, "aisino_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->nisec_sup);
// 	cJSON_AddStringToObject(dir3, "nisec_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->cntax_sup);
// 	cJSON_AddStringToObject(dir3, "cntax_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->mengbai_sup);
// 	cJSON_AddStringToObject(dir3, "mengbai_sup", tmp_s);

// 	cJSON_AddStringToObject(dir3, "aisino_end_time", (char *)state->aisino_end_time);
// 	cJSON_AddStringToObject(dir3, "nisec_end_time", (char *)state->nisec_end_time);
// 	cJSON_AddStringToObject(dir3, "cntax_end_time", (char *)state->cntax_end_time);
// 	cJSON_AddStringToObject(dir3, "mengbai_end_time", (char *)state->mengbai_end_time);

// 	cJSON_AddStringToObject(dir3, "start_time", (char *)state->start_time);
// 	cJSON_AddStringToObject(dir3, "end_time", (char *)state->end_time);
// 	cJSON_AddStringToObject(dir3, "frozen_ago_time", (char *)state->frozen_ago_time);



// 	if (off_up_flag == 1)
// 	{
// 		//�����ϱ�
// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
// 		cJSON_AddStringToObject(dir3, "usb_app", tmp);


// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
// 		cJSON_AddStringToObject(dir3, "vid", tmp);
// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
// 		cJSON_AddStringToObject(dir3, "pid", tmp);
// 		if (strlen((char *)state->port_last_info.ca_name) == 0)
// 		{
// 			cJSON_AddStringToObject(dir3, "name", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "name", (char *)state->port_last_info.ca_name);
// 		if (strlen((char *)state->port_last_info.ca_serial) == 0)
// 		{
// 			cJSON_AddStringToObject(dir3, "number", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "number", (char *)state->port_last_info.ca_serial);
// 		if (strlen((char *)state->port_last_info.sn) == 0)
// 		{
// 			cJSON_AddStringToObject(dir3, "serial", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "serial", (char *)state->port_last_info.sn);
// 		//printf("step1\n");
// 		cJSON_AddStringToObject(dir3, "usb_share", "0");
// 		cJSON_AddStringToObject(dir3, "usb_share_mode", "0");
// 		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
// 		cJSON_AddStringToObject(dir3, "offline", "0");
// 		//printf("step2\n");
// 	}
// 	else
// 	{
// 		//�����ϱ�
// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
// 		cJSON_AddStringToObject(dir3, "usb_app", tmp);
// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
// 		cJSON_AddStringToObject(dir3, "vid", tmp);
// 		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
// 		cJSON_AddStringToObject(dir3, "pid", tmp);
// 		if (strlen((char *)state->port_info.ca_name) == 0)
// 		{
// 			//ty_usb_m_out("#####################ca_name :%s###########\n",)
// 			cJSON_AddStringToObject(dir3, "name", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "name", (char *)state->port_info.ca_name);
// 		if (strlen((char *)state->port_info.ca_serial) == 0)
// 		{
// 			cJSON_AddStringToObject(dir3, "number", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "number", (char *)state->port_info.ca_serial);
// 		if (strlen((char *)state->port_info.sn) == 0)
// 		{
// 			cJSON_AddStringToObject(dir3, "serial", "Unknown");
// 		}
// 		else
// 			cJSON_AddStringToObject(dir3, "serial", (char *)state->port_info.sn);

// 		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
// 		{
// 			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
// 		}
// 		else
// 		{
// 			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
// 		}
// 		cJSON_AddStringToObject(dir3, "usb_share", tmp);
// 		memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", state->port_info.usb_share_mode);
// 		cJSON_AddStringToObject(dir3, "usb_share_mode", tmp);
// 		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
// 		cJSON_AddStringToObject(dir3, "offline", "1");
// 	}



// 	g_buf = cJSON_PrintUnformatted(json);
// 	str_replace(g_buf, "\\\\", "\\");
// 	ty_usb_m_out("g_buf = %s\n", g_buf);
// 	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
// 	char g_data[5000] = { 0 };
// 	result = _m_pub_need_answer(random, "mb_equ_ca_status_up", g_buf, g_data);
// 	stream->last_report_time_s = get_time_sec();
// 	if (result < 0)
// 	{
// 		stream->report_time_p += 5;//����ʱ������5��
// 		ty_usb_m_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", stream->report_time_s);
// 		free(g_buf);
// 		cJSON_Delete(json);
// 		goto End;
// 	}
// 	else if (result == 0)
// 	{
// 		stream->report_time_p += 5;//����ʱ������5��
// 		ty_usb_m_out("������Ӧ����ʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", stream->report_time_s);
// 		free(g_buf);
// 		cJSON_Delete(json);
// 		goto End;
// 	}
// 	else
// 	{
// 		stream->report_time_p = 0;//����ʱ��
// 		if (off_up_flag == 1)
// 			state->off_report = 0;
// 		else
// 			state->up_report = 0;
// 		ty_usb_m_out("�ϴ���%d�ſڣ�%s��CA��Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
// 	}
// 	sleep(stream->report_time_p);
// 	//ty_usb_m_out("�����������\n");
// 	free(g_buf);
// 	cJSON_Delete(json);

// End:
// 	ty_usb_m_out("����ca�ϱ�����1\n");
// 	stream->report_p_used = 0;
// 	return;
}


//��ʱ�ϱ��˿���Ϣ�߳�
static void timely_tigger_report_port_status(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//ty_usb_m_out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�_1\n");
		return;
	}
	
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		if (stream->report_ter_flag == 0)
		{
			ty_usb_m_out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
			return;
		}
		
		state = &stream->port[port - 1];
		if ((state->off_report == 0) && (state->up_report == 0) && (state->need_getinv == 0))
		{
			continue;
		}
		usleep(5000);
						
		if (((state->up_report == 1) && (state->port_info.usb_app == DEVICE_AISINO || state->port_info.usb_app == DEVICE_NISEC || state->port_info.usb_app == DEVICE_MENGBAI || state->port_info.usb_app == DEVICE_MENGBAI2 || state->port_info.usb_app >= 101)) || \
			((state->off_report == 1) && (state->port_last_info.usb_app == DEVICE_AISINO || state->port_last_info.usb_app == DEVICE_NISEC || state->port_last_info.usb_app == DEVICE_MENGBAI || state->port_last_info.usb_app == DEVICE_MENGBAI2 || state->port_last_info.usb_app >= 101)))
		{
			if (stream->report_p_used != 0)
			{
				//ty_usb_m_out("�˿�״̬�ϴ���\n");
				continue;
			}
			stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			ty_usb_m_out("��%d�Ŷ˿ڴ���һ��˰�̸��»������ϱ�\n", state->port);
			logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�˰�̸��»������ϱ�\r\n", port);
			_delay_task_add("0", pack_report_plate_info, state, 0);
			continue;
		}
		else if ((state->off_report == 1) && (state->port_last_info.usb_app == 0 || state->port_last_info.usb_app == 5))
		{
			if (stream->report_p_used != 0)
			{
				//ty_usb_m_out("�˿�״̬�ϴ���\n");
				continue;
			}
			stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			ty_usb_m_out("����һ������USB���»������ϱ�\n");
			logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�����USB���»������ϱ�\r\n", port);
			#ifndef DF_OLD_MSERVER
			_delay_task_add("0", pack_report_other_info_new_mqtt, state, 0);
			#else
			_delay_task_add("0", pack_report_other_info, state, 0);
			#endif
			continue;
		}
		else if (state->up_report == 1 || state->off_report == 1)
		{
			if (stream->report_p_used != 0)
			{
				//ty_usb_m_out("�˿�״̬�ϴ��У�%d\n", port);
				continue;
			}
			if ((stream->port[port - 1].port_info.usb_app == 4) && (stream->port[port - 1].nongye_report == 0))		//wang 11/18
			{
				ty_usb_m_out("����һ��[%d]�˿�״̬CA���»������ϱ�--\n", port);
				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬CA���»������ϱ�\r\n", port);
				//state->up_report = 0;
				stream->report_p_used = 1;
				#ifndef DF_OLD_MSERVER			//wang 11/9 ��Ӷ�����̨�ϱ�json�ֶ�����
				_delay_task_add("0", pack_report_ca_info_new_mqtt, state, 0);
				#else
				_delay_task_add("0", pack_report_ca_info, state, 0);
				#endif
			}
			else
			{
				ty_usb_m_out("����һ��[%d]�˿�״̬�������»������ϱ�--\n", port);
				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬�������»������ϱ�\r\n", port);
				stream->report_p_used = 1;
				#ifndef DF_OLD_MSERVER			//wang 11/9 ��Ӷ�����̨�ϱ�json�ֶ�����
				_delay_task_add("0", pack_report_other_info_new_mqtt, state, 0);
				#else
				_delay_task_add("0", pack_report_other_info, state, 0);
				#endif
			}
			continue;
		}
	}
}

static void pack_report_plate_info(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf=NULL;

	char source_topic[50] = { 0 };
	int result;
	char random[50] = { 0 };	
	state = arg;
	stream = ty_usb_m_id;
	
	memset(source_topic, 0, sizeof(source_topic));
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		if (state->last_act == 1)//������������Ȳ��������ϴ�
			off_up_flag = 1;
		if (state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//���������ϴ�
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	get_radom_serial_number("zdsbspxx", random);

// wang 11/9 ��Ӷ�����̨�ϱ�json�ֶ�����
#ifndef DF_OLD_MSERVER
	char tmp[20] = {0};
	char *g_new_buf=NULL;
	char *g_pub_buf=NULL;
	pack_report_plate_info_json_new_mqtt(&g_new_buf, off_up_flag,random,state);

	cJSON *port_array, *port_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "plate");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);		
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);	
		free(g_new_buf);				
	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	//ty_usb_m_out("�ϱ�˰�̷��ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json);

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", stream->ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_pub_buf = cJSON_PrintUnformatted(root);
	//ty_usb_m_out("�ϱ�˰�̷������ݣ�%s\n",g_pub_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_pub_buf);
	free(g_pub_buf);
	cJSON_Delete(root);
#endif

	pack_report_plate_info_json(&g_buf, off_up_flag,random,state);

	//char g_data[5000] = { 0 };
	//result = _m_pub_need_answer(random, "mb_equ_plate_status_up", g_buf, g_data);
	result = _m_pub_no_answer("mb_equ_plate_status_up", g_buf);
	if (result < 0)
	{
		//logout(INFO, "system", "report_plate", "���ϱ�˰��״̬���˿ںţ�%d,�̺ţ�%s,���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\r\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name, stream->report_time_s);
		free(g_buf);
		sleep(5);
		goto End;
	}
	else
	{
		if (off_up_flag ==  1)
			state->off_report = 0;
		else
			state->up_report = 0;
		logout(INFO, "MQTT", "JSON_UPDATE", "[%d]�Ŷ˿�˰�̸��»������ϱ��ɹ�\r\n", state->port);
		//ty_usb_m_out("�ϴ���%d�ſڣ�%s��˰����Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	
	//ty_usb_m_out("�����������\n");
	free(g_buf);

End:
	stream->report_p_used = 0;
	return;
}

//wang 11/9 ��Ӷ�����̨�ϱ�json�ֶ�����
#ifndef DF_OLD_MSERVER
static int pack_report_plate_info_json_new_mqtt(char **g_buf, int off_up_flag, char *random,struct _port_state   *state)
{
	struct _ty_usb_m  *stream;
	struct _ter_info ter_info;
	char tmp[20] = { 0 };
	stream = ty_usb_m_id;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);


	cJSON *json = cJSON_CreateObject();


	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json, "client_ip", "");
	}

	if (off_up_flag == 1)
	{
		//�����ϱ�
		cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(json, "port_num", tmp);
		cJSON_AddStringToObject(json, "busid", state->port_info.busid);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		cJSON_AddStringToObject(json, "name", (char *)state->port_last_info.ca_name);
		cJSON_AddStringToObject(json, "number", (char *)state->port_last_info.ca_serial);
		cJSON_AddStringToObject(json, "plate_num", (char *)state->port_last_info.sn);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.extension);
		cJSON_AddStringToObject(json, "plate_extension", tmp);
		cJSON_AddStringToObject(json, "offline", "0");
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->power);
		cJSON_AddStringToObject(json, "port_power", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(json, "ad_status", tmp);


		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);


		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(json, "usb_share", "0");
		cJSON_AddStringToObject(json, "usb_share_mode", "0");
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		//printf("pack_report_plate_info_json online = 0 , state->port_info.client = %d\r\n",state->port_info.client);
	}
	else
	{
		//�����ϱ�
		//wang 11/10
		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(json, "offline", "1");
		fun_get_port_data_json(json, state->port,2);			//wang 11/3 �޸� is_commonֵ��ֹ�����ն˲�ѯ�Ͷ���ն˲�ѯ�ֶ��ظ�

	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);

	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	//ty_usb_m_out("����һ��˰����Ϣ�ϱ�,�˿ںţ�%d\n",state->port);
	//ty_usb_m_out("g_buf = %s\n", *g_buf);
	return 0;
}
#endif

static int pack_report_plate_info_json(char **g_buf, int off_up_flag, char *random,struct _port_state   *state)
{
	struct _ty_usb_m  *stream;
	struct _ter_info ter_info;
	char source_topic[50] = { 0 };
	//int result;
	char tmp[20] = { 0 };
	stream = ty_usb_m_id;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);


	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	

	if (off_up_flag == 1)
	{
		//�����ϱ�
		cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(dir3, "port_num", tmp);
		cJSON_AddStringToObject(dir3, "busid", state->port_info.busid);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(dir3, "usb_app", tmp);
		cJSON_AddStringToObject(dir3, "name", (char *)state->port_last_info.ca_name);
		cJSON_AddStringToObject(dir3, "number", (char *)state->port_last_info.ca_serial);
		cJSON_AddStringToObject(dir3, "plate_num", (char *)state->port_last_info.sn);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.extension);
		cJSON_AddStringToObject(dir3, "plate_extension", tmp);
		cJSON_AddStringToObject(dir3, "offline", "0");
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->power);
		cJSON_AddStringToObject(dir3, "port_power", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(dir3, "ad_status", tmp);


		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(dir3, "usb_share", "0");
		cJSON_AddStringToObject(dir3, "usb_share_mode", "0");
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
		//printf("pack_report_plate_info_json online = 0 , state->port_info.client = %d\r\n",state->port_info.client);
	}
	else
	{
		//�����ϱ�
		cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(dir3, "offline", "1");
		fun_get_port_data_json(dir3, state->port,2);			//wang 11/3 �޸� is_commonֵ��ֹ�����ն˲�ѯ�Ͷ���ն˲�ѯ�ֶ��ظ�

	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);

	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	//ty_usb_m_out("����һ��˰����Ϣ�ϱ�,�˿ںţ�%d\n",state->port);
	//ty_usb_m_out("g_buf = %s\n", *g_buf);
	return 0;
}



static void pack_report_other_info(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf;
	char tmp[20] = { 0 };
	char source_topic[50] = { 0 };
	int result;
	char random[50] = { 0 };
	state = arg;
	stream = ty_usb_m_id;
	
	memset(source_topic, 0, sizeof(source_topic));
	//ty_usb_m_out("����һ������USB�ϱ�,�˿ںţ�%d\n",state->port);
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		//ty_usb_m_out("state->last_act = %d\n", state->last_act);
		if (state->last_act == 1)//������������Ȳ��������ϴ�
			off_up_flag = 1;
		if (state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if ((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//���������ϴ�
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("zdsbqtusb", random);

	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbqtusb");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(dir3, "port_num", tmp);

	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(dir3, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(dir3, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(dir3, "pid", tmp);
		cJSON_AddStringToObject(dir3, "offline", "0");


		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(dir3, "usb_share", "0");
		cJSON_AddStringToObject(dir3, "usb_share_mode", "0");
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
	}
	else
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(dir3, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(dir3, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(dir3, "pid", tmp);
		cJSON_AddStringToObject(dir3, "offline", "1");

		//wang 2022��10��17�� ���json����ȡ

		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}

		cJSON_AddStringToObject(dir3, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(dir3, "usb_share_mode", tmp);
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
	}



	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//ty_usb_m_out("pack_report_other_info g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	//char g_data[5000] = { 0 };
	//result = _m_pub_need_answer(random, "mb_equ_other_status_up", g_buf, g_data);
	result = _m_pub_no_answer("mb_equ_other_status_up", g_buf);
	if (result < 0)
	{
		ty_usb_m_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", 5);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	//else if (result == 0)
	//{
	//	
	//	ty_usb_m_out("������Ӧ����ʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", stream->report_time_s);
	//	free(g_buf);
	//	cJSON_Delete(json);
	//	goto End;
	//}
	else
	{
		if (off_up_flag == 1)
			state->off_report = 0;
		else
			state->up_report = 0;
		logout(INFO, "MQTT", "JSON_UPDATE", "[%d]�Ŷ˿�����USB���»������ϱ��ɹ�\r\n", state->port);
		//ty_usb_m_out("�ϴ���%d�ſڡ�����USB��Ϣ������Ӧ����ɹ�\n\n\n", state->port);
	}
	sleep(1);				//wang 11/24
	//ty_usb_m_out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_p_used = 0;
	return;
}

#ifndef DF_OLD_MSERVER
static void pack_report_other_info_new_mqtt(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf;
	char *g_new_buf;
	char tmp[20] = { 0 };
	char source_topic[50] = { 0 };
	int result;
	char random[50] = { 0 };
	state = arg;
	stream = ty_usb_m_id;
	
	memset(source_topic, 0, sizeof(source_topic));
	//ty_usb_m_out("����һ������USB�ϱ�,�˿ںţ�%d\n",state->port);
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		//ty_usb_m_out("state->last_act = %d\n", state->last_act);
		if (state->last_act == 1)//������������Ȳ��������ϴ�
			off_up_flag = 1;
		if (state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if ((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//���������ϴ�
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json, "port_num", tmp);

	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json, "client_ip", "");
	}

	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		cJSON_AddStringToObject(json, "offline", "0");

		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(json, "usb_share", "0");
		cJSON_AddStringToObject(json, "usb_share_mode", "0");
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
	}
	else
	{
		//�����ϱ�

		//wang 11/10
		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		cJSON_AddStringToObject(json, "offline", "1");

		//wang 2022��10��17�� ���json����ȡ
		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(json, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(json, "usb_share_mode", tmp);
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
	}


	g_new_buf = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);
	
	cJSON *port_array, *port_layer;				//���ʹ��
	cJSON *json_port_data = cJSON_CreateObject();
	cJSON_AddItemToObject(json_port_data, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "other");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);			//014b
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);	
		free(g_new_buf);				
	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json_port_data);
	//ty_usb_m_out("�ϱ�����usb���ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json_port_data);

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", stream->ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_buf = cJSON_PrintUnformatted(root);
	//ty_usb_m_out("�ϱ�����usb�������ݣ�%s\n",g_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_buf);
	if (result < 0)
	{
		ty_usb_m_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", 5);
		free(g_buf);
		cJSON_Delete(root);
		goto End;
	}
	//else if (result == 0)
	//{
	//	
	//	ty_usb_m_out("������Ӧ����ʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", stream->report_time_s);
	//	free(g_buf);
	//	cJSON_Delete(json);
	//	goto End;
	//}
	else
	{
		if (off_up_flag == 1)
			state->off_report = 0;
		else
			state->up_report = 0;
		//ty_usb_m_out("�ϴ���%d�ſڡ�����USB��Ϣ������Ӧ����ɹ�\n\n\n", state->port);
	}
	sleep(5);
	//ty_usb_m_out("�����������\n");
	free(g_buf);
	cJSON_Delete(root);

End:
	stream->report_p_used = 0;
	return;
}
#endif


static int waiting_for_plate_uninstall(uint8 *plate_num, struct _port_state   *state,int outtimes)
{
	int i=0;
	long outtimem = outtimes * 1000;
	if (strlen((char *)plate_num) == 0)
	{
		sleep(1);
		return 0;
	}
	for (i = 0; i < outtimem; i++)
	{
		if (strcmp((char *)plate_num, (char *)state->port_info.sn) != 0)
		{
			return 0;
		}
		usleep(1000);
	}
	return -1;
}

static int waiting_for_plate_install(uint8 *plate_num, struct _port_state   *state, int outtimes)
{
	int i = 0;
	long outtimem = outtimes * 1000;
	for (i = 0; i < outtimem; i++)
	{
		if ((strcmp((char *)plate_num, (char *)state->port_info.sn) == 0) && (state->port_info.tax_read == 0))//�˴��ж϶�ȡ�Ѿ����
		{
			if (state->up_report == 0)//�˴��ж��ϱ��Ѿ����
				return 0;
		}
		usleep(1000);
	}
	return -1;
}
static int restart_plate_power(struct _port_state   *state, uint8 *state_plate_num, int port,int wait_install)
{
	struct _ty_usb_m  *stream;
	int result;
	char errinfo[1024] = {0};
	char busid[32] = { 0 };
	char plate_num[20] = { 0 };
	strcpy(busid, state->port_info.busid);
	strcpy(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num);
	stream = ty_usb_m_id;

	state->h_plate.nBusID = -1;
	state->h_plate.nDevID = -1;

	if (state->power_mode == Saving_Mode)
	{
		state->sys_reload = 0;
		port_power_down(state);
		return 0;
	}
	logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s,��������%d�Ŷ˿ڵ�Դ,�̺ţ�%s\r\n", state->port_info.port_str, port, plate_num);
	//	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
//	if (result < 0)
//	{
//		sprintf(errinfo, "Ҫ��Ƭ���ر�%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
//		logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
//#ifdef RELEASE_SO
//		so_common_report_event((char *)plate_num, "��Ƭ���˿ڹص�ʧ��", errinfo, -1);
//#else
//		function_common_report_event((char *)plate_num, "��Ƭ���˿ڹص�ʧ��", errinfo, -1);
//#endif	
//		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
//		if (result < 0)
//		{
//			memset(errinfo,0,sizeof(errinfo));
//			sprintf(errinfo, "Ҫ��Ƭ�����Թر�%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
//			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
//#ifdef RELEASE_SO
//			so_common_report_event((char *)plate_num, "��Ƭ���˿����Թص�ʧ��", errinfo, -1);
//#else
//			function_common_report_event((char *)plate_num, "��Ƭ���˿����Թص�ʧ��", errinfo, -1);
//#endif	
//			return 0;
//		}
//	}
//
//	state->power = 0;

	port_power_down(state);
	result = waiting_for_plate_uninstall((uint8 *)plate_num, state, 1);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "����%d�Ŷ˿ڵ�Դ��,˰�̻���δ���,�̺ţ�%s,busid : %s\r\n", port, plate_num,busid);

		ty_usb_m_out("�򵥼��busid�Ƿ����\n");
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "����%d�Ŷ˿ڵ�Դ��,USB��busid:%s�����ļ�ϵͳ��ʧ,�̺ţ�%s,����Ϊ�̼߳�����\r\n", port, busid, plate_num);
		}
		else
		{
			sprintf(errinfo,"����%d�Ŷ˿ڵ�Դ��,USB��busid:%sδ���ļ�ϵͳ��ʧ,�̺ţ�%s,����ΪUSB�ļ�ϵͳ���ϻ�δ�����ص�", port, busid, plate_num);
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
// #ifdef RELEASE_SO
// 			so_common_report_event((char *)plate_num, "Ӳ���˿ڹص�ʧ��", errinfo, -1);
// #else
// 			function_common_report_event((char *)plate_num, "Ӳ���˿ڹص�ʧ��", errinfo, -1);
// #endif	
			state->port_info.tax_read = 1;
			printf("state->port_info.tax_read = 1;1\r\n");
			waiting_for_plate_install((uint8 *)plate_num, state, 10);
			return 0;

		}
	}

	//ty_usb_m_out("˰���ѱ�ж��\n");
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
	if (result < 0)
	{
		sprintf(errinfo, "Ҫ��Ƭ����%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
		logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
// #ifdef RELEASE_SO
// 		so_common_report_event((char *)plate_num, "��Ƭ���˿ڿ���ʧ��", errinfo, -1);
// #else
// 		function_common_report_event((char *)plate_num, "��Ƭ���˿ڿ���ʧ��", errinfo, -1);
// #endif	
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
		if (result < 0)
		{
			sprintf(errinfo, "Ҫ��Ƭ�����Դ�%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
// #ifdef RELEASE_SO
// 			so_common_report_event((char *)plate_num, "��Ƭ���˿����Կ���ʧ��", errinfo, -1);
// #else
// 			function_common_report_event((char *)plate_num, "��Ƭ���˿����Կ���ʧ��", errinfo, -1);
// #endif	
		}

	}
	state->power = 1;

	if (wait_install == 1)
	{
		result = waiting_for_plate_install((uint8 *)plate_num, state, 10);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "����%d�Ŷ˿ڵ�Դ��,USB��δ�������¼���,�̺ţ�%s\r\n", port, plate_num);
		}
	}
	//ty_usb_m_out("˰���Ѽ��سɹ�\n");
	return 0;
}

int restart_plate_power_extern(int port)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	stream = ty_usb_m_id;
	state = &stream->port[port - 1];
	restart_plate_power(state,state->port_info.sn,port,0);
	return 0;
}



//�����ϱ�Ƶ��
int function_set_report_time(int report_time)
{

	ty_usb_m_out("�����ϱ�Ƶ��Ϊ%d\n", report_time);
	return 0;

}



int fun_query_auth_file(char **auth_file)
{
	struct _ty_usb_m  *stream;

	stream = ty_usb_m_id;
	if (stream->cert_json == NULL)
	{
		ty_usb_m_out("��Ȩ�ļ�Ϊ��\n");
		return -1;
	}
	*auth_file = stream->cert_json;
	return 0;
}


int fun_deal_usb_port_power(int usb_port, int power_state,char *errinfo)
{
	struct _ty_usb_m  *stream;
	int port, result;
	struct _port_state   *state;
	char busid[32] = {0};
	char plate_num[20] = {0};
	stream = ty_usb_m_id;
	port = usb_port;
	state = &stream->port[port - 1];
	strcpy(busid, state->port_info.busid);
	strcpy(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num);
	if (port<0 || port>stream->usb_port_numb)
	{
		sprintf(errinfo, "Ҫ������USB�˿ںŲ��Ϸ�");
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
	if (power_state == 0)
	{
		ty_usb_m_out("�ر�%d�Ŷ˿ڵ�Դ\n",port);
		if (state->power_mode == Saving_Mode)
		{
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "�ڵ�治֧�ֹرն˿ڵ�Դ\r\n");
		}
		else
		{
			//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			//if (result <0)
			//{
			//	logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "Ҫ��Ƭ���ر�%d�Ŷ˿ڵ�Դʧ��,result = %d\r\n", port, result);
			//}
			//state->power = 0;
			port_power_down(state);
		}
		
	}
	else if (power_state == 1)
	{
		ty_usb_m_out("��%d�Ŷ˿ڵ�Դ\n", port);
		if (state->power_mode == Saving_Mode)
		{
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "�ڵ�治֧�ִ򿪶˿ڵ�Դ\r\n");
		}
		else
		{
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			if (result <0)
			{
				logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "Ҫ��Ƭ����%d�Ŷ˿ڵ�Դʧ��,result = %d\r\n", port, result);
			}
			state->power = 1;
		}
	}
	else if(power_state == 2)
	{
		ty_usb_m_out("����%d�Ŷ˿ڵ�Դ\n", port);
		restart_plate_power(state, state->port_info.sn, state->port, 0);
	}
	else 
	{
		sprintf(errinfo, "�����˿ڵ�Դָ�����");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	return 0;
}
//��ȡ��汾��
int fun_deal_get_so_version(char *tax_ver, char *mbc_ver,char *tax_load,char *auth_load)
{
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	strcpy(tax_ver, stream->tax_so_ver);
	strcpy(mbc_ver, stream->mbc_so_ver);

	if (stream->lib_dec_err != 1)
	{
		strcpy(tax_load,"1");
	}
	else
	{
		strcpy(tax_load,"0");
	}
	if (stream->auth_dec_err != 1)
	{
		strcpy(auth_load, "1");
	}
	else
	{
		strcpy(auth_load,  "0");
	}
	return 0;
}







int fun_get_ter_all_data_json_jude_need_port(cJSON *data_layer,int need_port_info,int need_tax_info)
{
	struct _ty_usb_m  *stream;

	char tmp_s[100] = { 0 };
	stream = ty_usb_m_id;
	
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	struct _mode_support mode_support;
	memset(&mode_support, 0, sizeof(struct _mode_support));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, &mode_support);


	cJSON_AddStringToObject(data_layer, "product_code", ter_info.product_code);
	cJSON_AddStringToObject(data_layer, "product_name", ter_info.product_name);
	cJSON_AddStringToObject(data_layer, "product_rename", ter_info.product_rename);
	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);	
	cJSON_AddStringToObject(data_layer, "ter_version", ter_info.ter_version);
	cJSON_AddStringToObject(data_layer, "ter_version_detail", ter_info.ter_version_detail);
	cJSON_AddStringToObject(data_layer, "ter_type", ter_info.ter_type);
	if (need_tax_info == 1)
	{
		cJSON_AddStringToObject(data_layer, "cpu_id", ter_info.cpu_id);
		cJSON_AddStringToObject(data_layer, "product_date", ter_info.product_date);
		cJSON_AddStringToObject(data_layer, "total_space", ter_info.total_space);
		cJSON_AddStringToObject(data_layer, "left_space", ter_info.left_space);
		cJSON_AddStringToObject(data_layer, "ram", ter_info.ram);
		cJSON_AddStringToObject(data_layer, "mem_per", ter_info.mem_per);
		cJSON_AddStringToObject(data_layer, "cpu_per", ter_info.cpu_per);
		cJSON_AddStringToObject(data_layer, "uptime", ter_info.uptime);
		cJSON_AddStringToObject(data_layer, "kernel_ver", ter_info.kernel_ver);
		if (stream->keyfile_load_err == 1)
			cJSON_AddStringToObject(data_layer, "key_file_load", "0");
		else
			cJSON_AddStringToObject(data_layer, "key_file_load", "1");
		cJSON_AddStringToObject(data_layer, "tax_so_load", ter_info.tax_so_load);
		cJSON_AddStringToObject(data_layer, "auth_file_load", ter_info.auth_file_load);
		cJSON_AddStringToObject(data_layer, "tax_so_ver", ter_info.tax_ver);
		cJSON_AddStringToObject(data_layer, "mbc_so_ver", ter_info.mbc_ver);
		cJSON_AddStringToObject(data_layer, "need_auth_aisino", "1");
		cJSON_AddStringToObject(data_layer, "need_auth_nisec", "1");
#ifdef UKEY_AUTH
		cJSON_AddStringToObject(data_layer, "need_auth_cntax", "0");
#else
		cJSON_AddStringToObject(data_layer, "need_auth_cntax", "1");
#endif
		cJSON_AddStringToObject(data_layer, "need_auth_mengbai", "0");

		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%ld", ter_info.boot_size);
		cJSON_AddStringToObject(data_layer, "boot_size", tmp_s);

		//ty_usb_m_out("�ն˻�����Ϣ������\n");
	}
	cJSON *manage_layer;
	cJSON_AddItemToObject(data_layer, "module_manage", manage_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(manage_layer, "eth_support", mode_support.eth_support);
	cJSON_AddStringToObject(manage_layer, "wifi_support", mode_support.wifi_support);
	cJSON_AddStringToObject(manage_layer, "4g_support", mode_support.g4_support);
	cJSON_AddStringToObject(manage_layer, "mysql_support", mode_support.mysql_support);
	cJSON_AddStringToObject(manage_layer, "sqlite_support", mode_support.sqlite_support);
	cJSON_AddStringToObject(manage_layer, "ftp_support", mode_support.ftp_support);
	cJSON_AddStringToObject(manage_layer, "ssh_support", mode_support.ssh_support);
	cJSON_AddStringToObject(manage_layer, "ntpdate_support", mode_support.ntpdate_support);
	cJSON_AddStringToObject(manage_layer, "tf_card_support", mode_support.ftp_support);
	cJSON_AddStringToObject(manage_layer, "event_log_support ", mode_support.event_log_support);
	cJSON_AddStringToObject(manage_layer, "usbshare_support", mode_support.usbshare_support);
	cJSON_AddStringToObject(manage_layer, "device_manage_support", mode_support.device_manage_support);
	cJSON_AddStringToObject(manage_layer, "mqtt_support", mode_support.mqtt_support);
	cJSON_AddStringToObject(manage_layer, "tax_invoice_support", mode_support.tax_invoice_support);
	cJSON_AddStringToObject(manage_layer, "extend_space_support ", mode_support.extend_space_support);
	cJSON_AddStringToObject(manage_layer, "usb_port_support", mode_support.usb_port_support);
	cJSON_AddStringToObject(manage_layer, "careader_support", mode_support.careader_support);
	cJSON_AddStringToObject(manage_layer, "userload_support", mode_support.userload_support);

	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->dev_support.support_ble);
	cJSON_AddStringToObject(manage_layer, "ble_support", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->dev_support.support_power);
	cJSON_AddStringToObject(manage_layer, "power_support", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->dev_support.support_lcd);
	cJSON_AddStringToObject(manage_layer, "lcd_support", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->dev_support.support_fan);
	cJSON_AddStringToObject(manage_layer, "fan_support", tmp_s);
		//ty_usb_m_out("�ն�ģ��֧��������\n");
	
	struct _mode_enable mode_enable;
	memset(&mode_enable, 0, sizeof(struct _mode_enable));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_ENABLE, &mode_enable);

	if (atoi(mode_support.eth_support) != 0)
	{
		struct _net_status net_status;
		memset(&net_status, 0, sizeof(struct _net_status));
		ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
		cJSON *eth_layer;
		cJSON_AddItemToObject(data_layer, "eth_manage", eth_layer = cJSON_CreateObject());
		if (strcmp(net_status.eth_wlan, "eth0") == 0)
		{					
			cJSON_AddStringToObject(eth_layer, "internet", net_status.internet);
			cJSON_AddStringToObject(eth_layer, "ip_type", net_status.ip_type);			
			cJSON_AddStringToObject(eth_layer, "ter_mac", net_status.mac);
			cJSON_AddStringToObject(eth_layer, "ip_address", net_status.ip_address);
			cJSON_AddStringToObject(eth_layer, "netmask", net_status.netmask);
			cJSON_AddStringToObject(eth_layer, "gateway", net_status.gateway);
			cJSON_AddStringToObject(eth_layer, "dns", net_status.dns);			
			if (need_tax_info == 1)
			{				
				cJSON_AddStringToObject(eth_layer, "eth_enable", mode_enable.eth_enable);
				cJSON_AddStringToObject(eth_layer, "first_net", net_status.first_net);
				cJSON_AddStringToObject(eth_layer, "eth_type", net_status.mac);
				cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "0");
			}
		}
		else
		{						
			cJSON_AddStringToObject(eth_layer, "internet", "");
			cJSON_AddStringToObject(eth_layer, "ip_type","");			
			cJSON_AddStringToObject(eth_layer, "ter_mac", "");
			cJSON_AddStringToObject(eth_layer, "ip_address", "");
			cJSON_AddStringToObject(eth_layer, "netmask", "");
			cJSON_AddStringToObject(eth_layer, "gateway", "");
			cJSON_AddStringToObject(eth_layer, "dns", "");
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "eth_enable", "0");
				cJSON_AddStringToObject(eth_layer, "first_net", "0");
				cJSON_AddStringToObject(eth_layer, "eth_type", "");
				cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "");
			}
		}
	}
	//ty_usb_m_out("�ն����ڹ���������\n");
	if (atoi(mode_support.wifi_support) != 0)
	{
		struct _net_status net_status;
		memset(&net_status, 0, sizeof(struct _net_status));
		ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
		cJSON *eth_layer;
		cJSON_AddItemToObject(data_layer, "wifi_manage", eth_layer = cJSON_CreateObject());
		if (strcmp(net_status.eth_wlan, "wlan0") == 0)
		{
					
			cJSON_AddStringToObject(eth_layer, "internet", net_status.internet);
			cJSON_AddStringToObject(eth_layer, "ip_type", net_status.ip_type);
			cJSON_AddStringToObject(eth_layer, "ter_mac", net_status.mac);
			cJSON_AddStringToObject(eth_layer, "ip_address", net_status.ip_address);
			cJSON_AddStringToObject(eth_layer, "netmask", net_status.netmask);
			cJSON_AddStringToObject(eth_layer, "gateway", net_status.gateway);
			cJSON_AddStringToObject(eth_layer, "dns", net_status.dns);
			cJSON_AddStringToObject(eth_layer, "connected_ssid", net_status.wifi_ssid);
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "wifi_enable", mode_enable.eth_enable);
				cJSON_AddStringToObject(eth_layer, "first_net", net_status.first_net);
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "0");
			}
		}
		else
		{
				
			cJSON_AddStringToObject(eth_layer, "internet", "");
			cJSON_AddStringToObject(eth_layer, "ip_type", "");
			cJSON_AddStringToObject(eth_layer, "ter_mac", "");
			cJSON_AddStringToObject(eth_layer, "ip_address", "");
			cJSON_AddStringToObject(eth_layer, "netmask", "");
			cJSON_AddStringToObject(eth_layer, "gateway", "");
			cJSON_AddStringToObject(eth_layer, "dns", "");
			cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "wifi_enable", "0");
				cJSON_AddStringToObject(eth_layer, "first_net", "0");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "");
			}
		}
	}
	if (atoi(mode_support.g4_support) != 0)
	{
		;
	}
	if (need_port_info == 0)
	{
		return 0;
	}
	if (need_tax_info == 1)
	{

#ifndef MTK_OPENWRT
		if (atoi(mode_support.mysql_support) != 0)
		{
			struct _mysql_process mysql_process;
			memset(&mysql_process, 0, sizeof(struct _mysql_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS, &mysql_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "mysql_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "mysql_enable", mode_enable.mysql_enable);
			cJSON_AddStringToObject(layer, "process_run", mysql_process.process_run);
			cJSON_AddStringToObject(layer, "port", mysql_process.port);
			cJSON_AddStringToObject(layer, "load_name", mysql_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", mysql_process.load_passwd);
			cJSON_AddStringToObject(layer, "data_path", mysql_process.data_path);
		}
#endif




		//ty_usb_m_out("�ն�mysql����������\n");
		if (atoi(mode_support.extend_space_support) != 0)
		{
			struct _extend_space_process extend_space_process;
			memset(&extend_space_process, 0, sizeof(struct _extend_space_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS, &extend_space_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "extend_space_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "extend_space_enable", mode_enable.extend_space_enable);
			cJSON_AddStringToObject(layer, "space_size", extend_space_process.space_size);
			cJSON_AddStringToObject(layer, "mount", extend_space_process.mount);
			cJSON_AddStringToObject(layer, "mount_dir", extend_space_process.mount_dir);
		}
		//ty_usb_m_out("�ն���չ�ڴ����������\n");
		if (atoi(mode_support.ftp_support) != 0)
		{
			struct _ftp_process ftp_process;
			memset(&ftp_process, 0, sizeof(struct _ftp_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_FTP_PROCESS, &ftp_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ftp_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ftp_enable", mode_enable.ftp_enable);
			cJSON_AddStringToObject(layer, "process_run", ftp_process.process_run);
			cJSON_AddStringToObject(layer, "port", ftp_process.port);
			cJSON_AddStringToObject(layer, "load_name", ftp_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", ftp_process.load_passwd);
			cJSON_AddStringToObject(layer, "default_path", ftp_process.default_path);
		}
		//ty_usb_m_out("�ն�ftp����������\n");
		if (atoi(mode_support.ssh_support) != 0)
		{
			struct _ssh_process ssh_process;
			memset(&ssh_process, 0, sizeof(struct _ssh_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_SSH_PROCESS, &ssh_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ssh_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ssh_enable", mode_enable.ssh_enable);
			cJSON_AddStringToObject(layer, "process_run", ssh_process.process_run);
			cJSON_AddStringToObject(layer, "port", ssh_process.port);
			cJSON_AddStringToObject(layer, "load_name", ssh_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", ssh_process.load_passwd);
		}
		//ty_usb_m_out("�ն�ssh����������\n");
		if (atoi(mode_support.ntpdate_support) != 0)
		{
			struct _ntpdate_process ntpdate_process;
			memset(&ntpdate_process, 0, sizeof(struct _ntpdate_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS, &ntpdate_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ntpdate_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ntpdate_enable", mode_enable.ntpdate_enable);
			cJSON_AddStringToObject(layer, "process_run", ntpdate_process.process_run);
			cJSON_AddStringToObject(layer, "server", ntpdate_process.server);
			cJSON_AddStringToObject(layer, "time_now", ntpdate_process.time_now);
		}
		//ty_usb_m_out("�ն�ntpdate����������\n");
		if (atoi(mode_support.tf_card_support) != 0)
		{
			struct _tfcard_process tfcard_process;
			memset(&tfcard_process, 0, sizeof(struct _tfcard_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS, &tfcard_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tf_card_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tf_card_enable", mode_enable.tf_card_enable);
			cJSON_AddStringToObject(layer, "insert", tfcard_process.insert);
			cJSON_AddStringToObject(layer, "mount", tfcard_process.mount);
			cJSON_AddStringToObject(layer, "status", tfcard_process.status);
		}
		//ty_usb_m_out("�ն�tfcard����������\n");
		if (atoi(mode_support.event_log_support) != 0)
		{

			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "event_log_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "event_log_enable", mode_enable.event_log_enable);
			cJSON_AddStringToObject(layer, "event_log_num", "0");
			cJSON_AddStringToObject(layer, "event_log", "");
		}
		//ty_usb_m_out("�ն��¼���־����������\n");
		if (atoi(mode_support.usbshare_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "usbshare_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "usbshare_enable", mode_enable.usbshare_enable);
			cJSON_AddStringToObject(layer, "port", "3240");
			cJSON_AddStringToObject(layer, "stop_use", "0");
		}
		//ty_usb_m_out("�ն�usbshare����������\n");
		if (atoi(mode_support.device_manage_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "device_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "device_manage_enable", mode_enable.device_manage_enable);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
		}
		//ty_usb_m_out("�ն��豸�������������\n");
		if (atoi(mode_support.mqtt_support) != 0)
		{
			char mqtt_state[10] = { 0 };
			char mqtt_server[50] = { 0 };
			int mqtt_port = 0;
			char user_name[50] = { 0 };
			char user_passwd[50] = { 0 };
			cJSON *layer;
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM, mqtt_server, &mqtt_port, user_name, user_passwd);

			cJSON_AddItemToObject(data_layer, "mqtt_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "mqtt_enable", mode_enable.mqtt_enable);
			cJSON_AddStringToObject(layer, "server_addr", mqtt_server);
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", mqtt_port);
			cJSON_AddStringToObject(layer, "server_port", tmp_s);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
			cJSON_AddStringToObject(layer, "cert_path", "/etc");
			sprintf(mqtt_state, "%d", lcd_print_info.mqtt_state);
			cJSON_AddStringToObject(layer, "connect_status", mqtt_state);
		}
		//ty_usb_m_out("�ն�mqtt����������\n");
		if (atoi(mode_support.tax_invoice_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tax_invoice_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tax_invoice_enable", mode_enable.tax_invoice_enable);
		}
		//ty_usb_m_out("�ն�˰��Ʊ����������\n");
		if (atoi(mode_support.careader_support) != 0)
		{
			struct _careader_process careader_process;
			memset(&careader_process, 0, sizeof(struct _careader_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "careader_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "careader_enable", mode_enable.careader_enable);
			cJSON_AddStringToObject(layer, "process_run", careader_process.process_run);
			cJSON_AddStringToObject(layer, "err_describe", careader_process.err_describe);
		}
		//ty_usb_m_out("�ն�CA���̹���������\n");
		if (atoi(mode_support.userload_support) != 0)
		{
			struct _userload_process userload_process;
			memset(&userload_process, 0, sizeof(struct _userload_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS, &userload_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "userload_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "userload_enable", mode_enable.userload_enable);
			cJSON_AddStringToObject(layer, "process_run", userload_process.process_run);
			cJSON_AddStringToObject(layer, "err_describe", userload_process.err_describe);
		}
	}
	//ty_usb_m_out("�ն��û���¼���̹���������\n");
	cJSON *usblayer;
	

	int port;
	int err_num = 0;
	char usb_port_num[10] = { 0 };
	char without_dev_port_num[10] = { 0 };
	char without_dev_ports[500] = { 0 };
	cJSON_AddItemToObject(data_layer, "usb_port_manage", usblayer = cJSON_CreateObject());
	int port_num = stream->usb_port_numb;
	//ty_usb_m_out("���ն�һ����%d��USB�˿�\n");
	for (port = 1; port<= port_num; port++)
	{
		if (stream->port[port - 1].port_info.with_dev != 1)
		{
			//ty_usb_m_out("%d�Ŷ˿�û���豸\n", port_count + 1);
			sprintf(without_dev_ports + strlen(without_dev_ports), "%d;", port);
			err_num = err_num + 1;
		}
	}
	sprintf(usb_port_num, "%d", port_num);
	sprintf(without_dev_port_num, "%d", err_num);
	cJSON_AddStringToObject(usblayer, "usb_port_num", usb_port_num);
	cJSON_AddStringToObject(usblayer, "without_dev_port_num", without_dev_port_num);
	cJSON_AddStringToObject(usblayer, "without_dev_ports", without_dev_ports);


	cJSON_AddStringToObject(usblayer, "short_circuit_ports", "");
	cJSON_AddStringToObject(usblayer, "open_circuit_ports", "");



	cJSON *port_array;		 //���ʹ��
	
	cJSON_AddItemToObject(usblayer, "usb_port_info", port_array = cJSON_CreateArray());
	for (port = 1; port <= port_num; port++)
	{
		struct _port_state   *state;
		
		state = &stream->port[port - 1];

		if ((need_tax_info == 0) && (state->port_info.with_dev == 0))
		{
			continue;
		}


		cJSON *port_layer;
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port);
		cJSON_AddStringToObject(port_layer, "usb_port", tmp_s);
		cJSON_AddStringToObject(port_layer, "busid", state->port_info.busid);		
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.with_dev);
		cJSON_AddStringToObject(port_layer, "dev_exists", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->power);
		cJSON_AddStringToObject(port_layer, "port_power", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(port_layer, "ad_status", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", 0);
		cJSON_AddStringToObject(port_layer, "dev_abnormal", tmp_s);
		if (need_tax_info == 1)
		{			
			if (state->port_info.dev_type == 3)
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Hid");
			}
			else if (state->port_info.dev_type == 8)
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Masstore");
			}
			else
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Unkown");
			}
			cJSON_AddStringToObject(port_layer, "usb_type", tmp_s);
		}
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(port_layer, "usb_vid", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(port_layer, "usb_pid", tmp_s);


		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(port_layer, "usb_app", tmp_s);
		//#endif
		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "1");
		}
		else
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "0");
		}
		cJSON_AddStringToObject(port_layer, "usb_share", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(port_layer, "usb_share_mode", tmp_s);
		cJSON_AddStringToObject(port_layer, "client_id", state->port_info.client);
		if (state->port_info.ip != 0x000000)
		{
			char ip_str[50] = { 0 };
			ip2ip_asc(state->port_info.ip, ip_str);
			cJSON_AddStringToObject(port_layer, "client_ip", ip_str);
		}
		else
		{
			cJSON_AddStringToObject(port_layer, "client_ip", "");
		}
		if (state->port_info.with_dev != 0)
		{
			if (state->port_info.usb_app == 4)
			{
				#ifndef DF_SUPPORT_CANAME							//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��				
				cJSON *ca_layer;
				cJSON_AddItemToObject(port_layer, "ca_infos", ca_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(ca_layer, "name", (char *)state->port_info.ca_name);
				cJSON_AddStringToObject(ca_layer, "number", (char *)state->port_info.ca_serial);
				cJSON_AddStringToObject(ca_layer, "cert_file_name", "");
				cJSON_AddStringToObject(ca_layer, "cert_file_data", "");		
				#endif		
			}
			else if ((state->port_info.usb_app == 1) || (state->port_info.usb_app == 2) || (state->port_info.usb_app == 3) || state->port_info.usb_app >= 101)
			{
				cJSON *plate_layer;
				cJSON_AddItemToObject(port_layer, "plate_infos", plate_layer = cJSON_CreateObject());
				if (need_tax_info == 0)
				{
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port);
					cJSON_AddStringToObject(plate_layer, "port_num", tmp_s);
					cJSON_AddStringToObject(plate_layer, "busid", state->port_info.busid);
					memset(tmp_s, 0, sizeof(tmp_s));				//wang usb_app��ʾ101
					sprintf(tmp_s, "%d", state->port_info.usb_app);
					cJSON_AddStringToObject(plate_layer, "usb_app", tmp_s);

					#ifndef DF_SUPPORT_CANAME						//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��	
					cJSON_AddStringToObject(plate_layer, "name", (char *)state->port_info.ca_name);
					cJSON_AddStringToObject(plate_layer, "number", (char *)state->port_info.ca_serial);
					cJSON_AddStringToObject(plate_layer, "plate_num", (char *)state->port_info.sn);
					#endif	
				}
				else
				{				
					fun_get_port_data_json(plate_layer, port, 1);
				}
			}
		}

	}


	
	return 0;

}

int fun_get_ter_all_data_json(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 1,1);
	return 0;
}

int fun_get_ter_all_data_json_without_port(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 0,0);
	return 0;
}

int fun_get_ter_all_data_json_without_tax(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 1,0);
	return 0;
}

int fun_get_ter_base_data_json(cJSON *data_layer)
{
	char tmp_s[100] = { 0 };
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(stream->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	cJSON_AddStringToObject(data_layer, "product_code", ter_info.product_code);
	cJSON_AddStringToObject(data_layer, "product_name", ter_info.product_name);
	cJSON_AddStringToObject(data_layer, "product_rename", ter_info.product_rename);
	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	cJSON_AddStringToObject(data_layer, "cpu_id", ter_info.cpu_id);
	cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
	cJSON_AddStringToObject(data_layer, "ter_version", ter_info.ter_version);
	cJSON_AddStringToObject(data_layer, "ter_version_detail", ter_info.ter_version_detail);
	if (stream->keyfile_load_err == 1)
		cJSON_AddStringToObject(data_layer, "key_file_load", "0");
	else
		cJSON_AddStringToObject(data_layer, "key_file_load", "1");
	cJSON_AddStringToObject(data_layer, "tax_so_load", ter_info.tax_so_load);
	cJSON_AddStringToObject(data_layer, "auth_file_load", ter_info.auth_file_load);
	cJSON_AddStringToObject(data_layer, "tax_so_ver", ter_info.tax_ver);
	cJSON_AddStringToObject(data_layer, "mbc_so_ver", ter_info.mbc_ver);
	cJSON_AddStringToObject(data_layer, "ter_type", ter_info.ter_type);
	cJSON_AddStringToObject(data_layer, "internet_ip", net_status.internet_ip);
	cJSON_AddStringToObject(data_layer, "longitude", net_status.longitude);
	cJSON_AddStringToObject(data_layer, "latitude", net_status.latitude);
	cJSON_AddStringToObject(data_layer, "area_info", net_status.area_info);
	cJSON_AddStringToObject(data_layer, "total_space", ter_info.total_space);
	cJSON_AddStringToObject(data_layer, "left_space", ter_info.left_space);
	cJSON_AddStringToObject(data_layer, "kernel_ver", ter_info.kernel_ver);


	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%ld", ter_info.boot_size);
	cJSON_AddStringToObject(data_layer, "boot_size", tmp_s);

	cJSON_AddStringToObject(data_layer, "short_circuit_ports", "");
	cJSON_AddStringToObject(data_layer, "open_circuit_ports", "");
	cJSON_AddStringToObject(data_layer, "ip_conflict", "0");


	cJSON_AddStringToObject(data_layer, "need_auth_aisino", "1");
	cJSON_AddStringToObject(data_layer, "need_auth_nisec", "1");
#ifdef UKEY_AUTH
	cJSON_AddStringToObject(data_layer, "need_auth_cntax", "0");
#else
	cJSON_AddStringToObject(data_layer, "need_auth_cntax", "1");
#endif
	cJSON_AddStringToObject(data_layer, "need_auth_mengbai", "0");

	cJSON *dir4;
	cJSON_AddItemToObject(data_layer, "plate_function", dir4 = cJSON_CreateArray());


//wang ����24�ڳ���ע�������ֶ�
/*
	int i;
	for (i = 0; i < sizeof(plate_function) / sizeof(struct _plate_function); i++)
	{
		cJSON_AddItemToObject(dir4, "dira", dir5 = cJSON_CreateObject());
		cJSON_AddStringToObject(dir5, "usb_app", plate_function[i].usb_app);
		cJSON_AddStringToObject(dir5, "plate_type", plate_function[i].plate_type);
		cJSON_AddStringToObject(dir5, "jcxxdq", plate_function[i].jcxxdq);
		cJSON_AddStringToObject(dir5, "jkxxdq", plate_function[i].jkxxdq);
		cJSON_AddStringToObject(dir5, "kcxxdq", plate_function[i].kcxxdq);
		cJSON_AddStringToObject(dir5, "cxspdqsj", plate_function[i].cxspdqsj);
		cJSON_AddStringToObject(dir5, "cxdqfpdmhm", plate_function[i].cxdqfpdmhm);
		cJSON_AddStringToObject(dir5, "aycxykfpsj", plate_function[i].aycxykfpsj);
		cJSON_AddStringToObject(dir5, "aycxykfpzs", plate_function[i].aycxykfpzs);
		cJSON_AddStringToObject(dir5, "afpdmhmcxykfp", plate_function[i].afpdmhmcxykfp);
		cJSON_AddStringToObject(dir5, "yzzskl", plate_function[i].yzzskl);
		cJSON_AddStringToObject(dir5, "xgzskl", plate_function[i].xgzskl);
		cJSON_AddStringToObject(dir5, "zsfpkj", plate_function[i].zsfpkj);
		cJSON_AddStringToObject(dir5, "fsfpkj", plate_function[i].fsfpkj);
		cJSON_AddStringToObject(dir5, "ykzsfpzf", plate_function[i].ykzsfpzf);
		cJSON_AddStringToObject(dir5, "ykfsfpzf", plate_function[i].ykfsfpzf);
		cJSON_AddStringToObject(dir5, "wkfpzf", plate_function[i].wkfpzf);
		cJSON_AddStringToObject(dir5, "yqsbfpzf", plate_function[i].yqsbfpzf);
		cJSON_AddStringToObject(dir5, "asjdcxhzxxb", plate_function[i].asjdcxhzxxb);
		cJSON_AddStringToObject(dir5, "axxbbhcxhzxxb", plate_function[i].axxbbhcxhzxxb);
		cJSON_AddStringToObject(dir5, "xfsqhzxxb", plate_function[i].xfsqhzxxb);
		cJSON_AddStringToObject(dir5, "gfsqhzxxb", plate_function[i].gfsqhzxxb);
		cJSON_AddStringToObject(dir5, "cxhzxxb", plate_function[i].cxhzxxb);
		cJSON_AddStringToObject(dir5, "zdcshz", plate_function[i].zdcshz);
		cJSON_AddStringToObject(dir5, "zdqk", plate_function[i].zdqk);
		cJSON_AddStringToObject(dir5, "lxfpzdsc", plate_function[i].lxfpzdsc);
		cJSON_AddStringToObject(dir5, "afpdmhmscfp", plate_function[i].afpdmhmscfp);
		cJSON_AddStringToObject(dir5, "csspyfwqlj", plate_function[i].csspyfwqlj);
		cJSON_AddStringToObject(dir5, "dlyfpcx", plate_function[i].dlyfpcx);
		cJSON_AddStringToObject(dir5, "dlyfpxzaz", plate_function[i].dlyfpxzaz);
		cJSON_AddStringToObject(dir5, "jrggfwpt", plate_function[i].jrggfwpt);
		cJSON_AddStringToObject(dir5, "qxggfwpt", plate_function[i].qxggfwpt);
	}
*/
	return 0;
}

int fun_get_port_data_json(cJSON *data_layer,int port,int is_common)
{
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	char tmp_s[1024] = {0};
	//struct _ter_info ter_info;
	//memset(&ter_info, 0, sizeof(struct _ter_info));
	//ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	struct _port_state   *state;

	state = &stream->port[port - 1];

	
	memset(tmp_s, 0, sizeof(tmp_s));	
	sprintf(tmp_s, "%d", state->port);
	
	
	cJSON_AddStringToObject(data_layer, "port_num", tmp_s);


	if(is_common == 2)
	{
		cJSON_AddStringToObject(data_layer, "busid", state->port_info.busid);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.with_dev);
		cJSON_AddStringToObject(data_layer, "dev_exists", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(data_layer, "usb_app", tmp_s);
	}

	

	cJSON_AddStringToObject(data_layer, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(data_layer, "number", (char *)state->port_info.ca_serial);


	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.dev_type);
		cJSON_AddStringToObject(data_layer, "usb_type", tmp_s);
	}

	cJSON_AddStringToObject(data_layer, "plate_num", (char *)state->port_info.sn);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.extension);
	cJSON_AddStringToObject(data_layer, "plate_extension", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_basic_info.plate_test);
	cJSON_AddStringToObject(data_layer, "plate_train", tmp_s);


	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(data_layer, "usb_vid", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(data_layer, "usb_pid", tmp_s);
	}

	//int busnum = 0;
	//int devnum = 0;
	//busnum = get_busid_busnum(state->port_info.busid);
	//devnum = get_busid_devnum(state->port_info.busid);
	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", busnum);
	//cJSON_AddStringToObject(data_layer, "usb_busnum", tmp_s);
	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", devnum);
	//cJSON_AddStringToObject(data_layer, "usb_devnum", tmp_s);
	

	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->power);
		cJSON_AddStringToObject(data_layer, "port_power", tmp_s);
		
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(data_layer, "ad_status", tmp_s);

		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "1");
		}
		else
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "0");
		}

		cJSON_AddStringToObject(data_layer, "usb_share", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); 
		sprintf(tmp_s, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(data_layer, "usb_share_mode", tmp_s);
		cJSON_AddStringToObject(data_layer, "client_id", state->port_info.client);

		if (state->port_info.ip != 0x000000)
		{
			char ip_str[50] = { 0 };
			ip2ip_asc(state->port_info.ip, ip_str);
			cJSON_AddStringToObject(data_layer, "client_ip", ip_str);
		}
		else
		{
			cJSON_AddStringToObject(data_layer, "client_ip", "");
		}
	}

// 	if ((state->port_info.upload_inv_num.need_fpdmhm_flag == 1) || (state->port_info.upload_inv_num.auto_up_end == 0))
// 	{
// 		cJSON_AddStringToObject(data_layer, "up_mserver_complete", "0");
// 	}
// 	else
// 	{
// 		cJSON_AddStringToObject(data_layer, "up_mserver_complete", "1");
// 	}

// 	if (state->port_info.plate_infos.plate_tax_info.off_inv_num == 0)
// 	{
// 		cJSON_AddStringToObject(data_layer, "offline_inv_exsit", "0");
// 		cJSON_AddStringToObject(data_layer, "offline_inv_info", "");
// 	}
// 	else
// 	{
// 		cJSON_AddStringToObject(data_layer, "offline_inv_exsit", "1");
// 		char offline_inv_info[1024] = { 0 };
// 		sprintf(offline_inv_info, "%s||%s||%s||%d", state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, state->port_info.plate_infos.plate_tax_info.off_inv_fphm,
// 			state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh);
// 		cJSON_AddStringToObject(data_layer, "offline_inv_info", offline_inv_info);
// 	}
// 	if (state->port_info.offinv_stop_upload == 0)
// 		cJSON_AddStringToObject(data_layer, "offline_inv_stop_upload", "0");
// 	else
// 		cJSON_AddStringToObject(data_layer, "offline_inv_stop_upload", "1");
// 	cJSON_AddStringToObject(data_layer, "offline_inv_status", (char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status);
// 	cJSON_AddStringToObject(data_layer, "plate_using_info", (char *)state->port_info.used_info);

// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_inv_state);
// 	cJSON_AddStringToObject(data_layer, "m_inv_state", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_tzdbh_state);
// 	cJSON_AddStringToObject(data_layer, "m_tzdbh_state", tmp_s);
// 	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_netinv_state);
// 	//cJSON_AddStringToObject(data_layer, "netinv_m_state", tmp_s);




// 	cJSON_AddStringToObject(data_layer, "tax_nature", (char *)state->port_info.plate_infos.plate_tax_info.tax_nature);
// 	cJSON_AddStringToObject(data_layer, "tax_office_name", (char *)state->port_info.plate_infos.plate_tax_info.tax_office_name);
// 	cJSON_AddStringToObject(data_layer, "tax_office_code", (char *)state->port_info.plate_infos.plate_tax_info.tax_office_code);
// 	cJSON_AddStringToObject(data_layer, "tax_server_url", (char *)state->port_info.plate_infos.plate_tax_info.tax_server_url);
// 	cJSON_AddStringToObject(data_layer, "integrated_server_url", (char *)state->port_info.plate_infos.plate_tax_info.integrated_server_url);

// 	cJSON *connect_layer;
// 	cJSON_AddItemToObject(data_layer, "connect_tax_server", connect_layer = cJSON_CreateObject());
// 	_lock_set(state->lock);
// 	if (state->h_plate.hDev != NULL)
// 	{
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_success_count);
// 		cJSON_AddStringToObject(connect_layer, "success_count", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_failed_count);
// 		cJSON_AddStringToObject(connect_layer, "failed_count", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_success_time);
// 		cJSON_AddStringToObject(connect_layer, "success_msec", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_failed_time);
// 		cJSON_AddStringToObject(connect_layer, "failed_msec", tmp_s);
// 		if(strlen(state->h_plate.hDev->DevHttp.start_time_s)!= 0)
// 			cJSON_AddStringToObject(connect_layer, "last_start_time", state->h_plate.hDev->DevHttp.start_time_s);
// 		else
// 			cJSON_AddStringToObject(connect_layer, "last_start_time", "");
// 		if (strlen(state->h_plate.hDev->DevHttp.end_time_s) != 0)
// 			cJSON_AddStringToObject(connect_layer, "last_end_time", state->h_plate.hDev->DevHttp.end_time_s);
// 		else
// 			cJSON_AddStringToObject(connect_layer, "last_end_time", "");
// 		if ((state->h_plate.hDev->hUSB != NULL) && (state->h_plate.hDev->hUSB->errcode == ErrorNET) && (strlen(state->h_plate.hDev->hUSB->errinfo) != 0))
// 			cJSON_AddStringToObject(connect_layer, "last_errinfo", state->h_plate.hDev->hUSB->errinfo);
// 		else
// 			cJSON_AddStringToObject(connect_layer, "last_errinfo", "");
// 		cJSON_AddStringToObject(connect_layer, "last_tax_business", state->port_info.tax_business);
// 	}
// 	else
// 	{
// 		cJSON_AddStringToObject(connect_layer, "success_count", "0");
// 		cJSON_AddStringToObject(connect_layer, "failed_count", "0");
// 		cJSON_AddStringToObject(connect_layer, "success_msec", "0");
// 		cJSON_AddStringToObject(connect_layer, "failed_msec", "0");
// 		cJSON_AddStringToObject(connect_layer, "last_start_time", "");
// 		cJSON_AddStringToObject(connect_layer, "last_end_time", "");
// 		cJSON_AddStringToObject(connect_layer, "last_errinfo", "");
// 		cJSON_AddStringToObject(connect_layer, "last_tax_business", state->port_info.tax_business);
// 	}
// 	_lock_un(state->lock);

// 	cJSON_AddStringToObject(data_layer, "plate_start_date", (char *)state->port_info.plate_infos.plate_tax_info.startup_date);

// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_sys_time);
// 	cJSON_AddStringToObject(data_layer, "plate_sys_time_diff", tmp_s);
// 	cJSON_AddStringToObject(data_layer, "driver_ver", (char *)state->port_info.plate_infos.plate_tax_info.driver_ver);
// 	cJSON_AddStringToObject(data_layer, "driver_ver_err", (char *)state->port_info.plate_infos.plate_tax_info.driver_ver_err);

// 	if (strlen((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status) == 0)
// 	{
// 		if (strlen((char *)state->port_info.plate_infos.plate_tax_info.current_report_time) == 0)
// 		{
// 			cJSON_AddStringToObject(data_layer, "report_tax_status", "˰�̸�����,��δִ�е������忨����");
// 		}
// 		else
// 		{
// 			cJSON_AddStringToObject(data_layer, "report_tax_status", "˰����ִ�г����忨����,δ�յ���������");
// 		}
// 	}
// 	else
// 	{
// 		if (strcmp((const char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�") == 0)
// 		{
// 			int types_count;
// 			for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
// 			{
// 				if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
// 				{
// 					continue;
// 				}
// 				if (atoi((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt) != 2)
// 				{
// 					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "Ʊ��%03d��д��ز��ɹ�", state->port_info.plate_infos.invoice_type_infos[types_count].fplxdm);
// 				}
// 			}
// 		}
// 		cJSON_AddStringToObject(data_layer, "report_tax_status", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
// 	}	




// 	cJSON_AddStringToObject(data_layer, "current_report_time", (char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
// 	cJSON_AddStringToObject(data_layer, "next_report_time", (char *)state->port_info.plate_infos.plate_tax_info.next_report_time);

// 	// int result;
// 	// char errinfo[1024]={0};
// // #ifdef RELEASE_SO
// // 	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// // #else
// // 	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
// // #endif
// 	// if (result != 1)
// 	// {
// 		cJSON_AddStringToObject(data_layer, "tax_auth", "0");
// 	// }
// 	// else
// 	// {
// 	// 	cJSON_AddStringToObject(data_layer, "tax_auth", "1");
// 	// }

// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->aisino_sup);
// 	cJSON_AddStringToObject(data_layer, "aisino_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->nisec_sup);
// 	cJSON_AddStringToObject(data_layer, "nisec_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->cntax_sup);
// 	cJSON_AddStringToObject(data_layer, "cntax_sup", tmp_s);
// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->mengbai_sup);
// 	cJSON_AddStringToObject(data_layer, "mengbai_sup", tmp_s);

// 	cJSON_AddStringToObject(data_layer, "aisino_end_time", (char *)state->aisino_end_time);
// 	cJSON_AddStringToObject(data_layer, "nisec_end_time", (char *)state->nisec_end_time);
// 	cJSON_AddStringToObject(data_layer, "cntax_end_time", (char *)state->cntax_end_time);
// 	cJSON_AddStringToObject(data_layer, "mengbai_end_time", (char *)state->mengbai_end_time);

	//wang 11/3 ��Ӷ���ʼʱ��ͽ���ʱ���ֶ��ϴ�
 	cJSON_AddStringToObject(data_layer, "start_time", stream->start_time);
 	cJSON_AddStringToObject(data_layer, "end_time", stream->end_time);
// 	cJSON_AddStringToObject(data_layer, "frozen_ago_time", (char *)state->frozen_ago_time);

// 	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_basic_info.cert_passwd_right);
// 	cJSON_AddStringToObject(data_layer, "cert_passwd_rignht", tmp_s);
// 	cJSON_AddStringToObject(data_layer, "area_code", (char *)state->port_info.plate_infos.plate_tax_info.area_code);

// 	cJSON_AddStringToObject(data_layer, "used_ofd", (char *)state->port_info.plate_infos.plate_basic_info.used_ofd);
// 	cJSON_AddStringToObject(data_layer, "server_type", (char *)state->port_info.plate_infos.plate_basic_info.server_type);
// 	cJSON_AddStringToObject(data_layer, "server_number", (char *)state->port_info.plate_infos.plate_basic_info.server_number);

// 	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_tax_info.inv_type_num);
// 	cJSON_AddStringToObject(data_layer, "tax_types_num", tmp_s);
// 	cJSON *inv_type_array;
// 	cJSON_AddItemToObject(data_layer, "tax_infos", inv_type_array = cJSON_CreateArray());
// 	//update_plate_info_cbqkzt(state);
// 	int types_count = 0;
// 	for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
// 	{
// 		if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
// 		{
// 			continue;
// 		}
// 		cJSON *type_layer;
// 		cJSON_AddItemToObject(inv_type_array, "dira", type_layer = cJSON_CreateObject());
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%03d", state->port_info.plate_infos.invoice_type_infos[types_count].fplxdm);
// 		cJSON_AddStringToObject(type_layer, "fplxdm", tmp_s);
// 		cJSON_AddStringToObject(type_layer, "sqslv", "");
// 		cJSON_AddStringToObject(type_layer, "cbqkzt", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt);

// 		if (strcmp((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj,"00010101") == 0)
// 		{
// 			logout(ERROR, "system", "˰��ͬ��", "%s,˰����Ϣ��������з��ֿ�Ʊ��ֹʱ���쳣\r\n", state->port_info.port_str);
// 		}

// 		cJSON_AddStringToObject(type_layer, "kpjzsj", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj);
// 		cJSON_AddStringToObject(type_layer, "bsqsrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.bsqsrq);
// 		cJSON_AddStringToObject(type_layer, "bszzrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.bszzrq);
// 		cJSON_AddStringToObject(type_layer, "zxbsrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.zxbsrq);
// 		cJSON_AddStringToObject(type_layer, "dzkpxe", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.dzkpxe);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxkpsc);
// 		cJSON_AddStringToObject(type_layer, "lxsc", tmp_s);
// 		cJSON_AddStringToObject(type_layer, "lxzsljje", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxzsljje);
// 		cJSON_AddStringToObject(type_layer, "lxzssyje", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxzssyje);


// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.sjxsje);
// 		cJSON_AddStringToObject(type_layer, "sjxsje", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.sjxsse);
// 		cJSON_AddStringToObject(type_layer, "sjxsse", tmp_s);


// 		cJSON_AddStringToObject(type_layer, "dqfpdm", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.dqfpdm);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.dqfphm);
// 		cJSON_AddStringToObject(type_layer, "dqfphm", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.zsyfpfs);
// 		cJSON_AddStringToObject(type_layer, "zsyfpfs", tmp_s);
// 		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl);
// 		cJSON_AddStringToObject(type_layer, "wsyfpjsl", tmp_s);

// 		if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl != 0)
// 		{
// 			cJSON *coil_array;
// 			int unused_num = 0;
// 			if (is_common == 1)
// 			{
				
// 				cJSON_AddItemToObject(type_layer, "wsyfpjxx", coil_array = cJSON_CreateArray());
				
// 				for (unused_num = 0; unused_num < state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl; unused_num++)
// 				{
// 					cJSON *unused_layer;
// 					if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].state == 0)
// 						continue;
// 					cJSON_AddItemToObject(coil_array, "dira", unused_layer = cJSON_CreateObject());
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", unused_num + 1);
// 					cJSON_AddStringToObject(unused_layer, "fpjh", tmp_s);
// 					cJSON_AddStringToObject(unused_layer, "fplbdm", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fplbdm);
// 					cJSON_AddStringToObject(unused_layer, "fpgmsj", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsj);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsl);
// 					cJSON_AddStringToObject(unused_layer, "fpgmsl", tmp_s);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpqshm);
// 					cJSON_AddStringToObject(unused_layer, "fpqshm", tmp_s);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpzzhm);
// 					cJSON_AddStringToObject(unused_layer, "fpzzhm", tmp_s);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpsyfs);
// 					cJSON_AddStringToObject(unused_layer, "fpsyfs", tmp_s);
// 				}
// 			}
// 			else//Ŀǰ�ߵƷ�ʽ
// 			{
// 				cJSON_AddItemToObject(type_layer, "unused_invoices", coil_array = cJSON_CreateArray());
// 				//ty_usb_m_out("δʹ�÷�Ʊ������:%d,port = %d\n", stream->port[port_num].port_info.tax_infos[types_num].wsyfpjsl, port_num+1);
// 				for (unused_num = 0; unused_num < state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl; unused_num++)
// 				{
// 					cJSON *unused_layer;
// 					if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].state == 0)
// 						continue;
// 					cJSON_AddItemToObject(coil_array, "dira", unused_layer = cJSON_CreateObject());
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", unused_num + 1);
// 					cJSON_AddStringToObject(unused_layer, "coil_num", tmp_s);
// 					cJSON_AddStringToObject(unused_layer, "coil_code", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fplbdm);
// 					cJSON_AddStringToObject(unused_layer, "buy_time", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsj);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsl);
// 					cJSON_AddStringToObject(unused_layer, "buy_num", tmp_s);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpqshm);
// 					cJSON_AddStringToObject(unused_layer, "start_num", tmp_s);
// 					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpsyfs);
// 					cJSON_AddStringToObject(unused_layer, "remain_num", tmp_s);
// 				}
// 			}

// 		}
// 	}
	return 0;
}


static int save_cert_file(char *base_file, char *path)
{
	char cmd[1024] = {0};
	sprintf(cmd, "rm %s", path);
	system(cmd);
	write_file(path, base_file, strlen(base_file));
	sync();
	return 0;
}


static int analysis_mqtt_connect_data_md5_data(char *data, char *server_addr, int *port, char *mqtt_uid, char *mqtt_pwd, char *crt_md5)
{
	char tmp[50] = { 0 };
	cJSON *root = NULL, *item;
	root = cJSON_Parse((const char *)data);
	if (root == NULL)
	{
		return -1;
	}
	item = cJSON_GetObjectItem(root, "server_addr");
	if (get_json_value_can_not_null(item, server_addr, 1, 100) < 0)
	{
		cJSON_Delete(root);
		return -2;
	}

	item = cJSON_GetObjectItem(root, "server_port");
	if (get_json_value_can_not_null(item, tmp, 1, 6) < 0)
	{
		cJSON_Delete(root);
		return -3;
	}
	*port = atoi(tmp);

	item = cJSON_GetObjectItem(root, "uid");
	if (get_json_value_can_not_null(item, mqtt_uid, 1, 20) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}

	item = cJSON_GetObjectItem(root, "pwd");
	if (get_json_value_can_not_null(item, mqtt_pwd, 1, 50) < 0)
	{
		cJSON_Delete(root);
		return -5;
	}

	item = cJSON_GetObjectItem(root, "crt_md5");
	if (get_json_value_can_not_null(item, crt_md5, 1, 50) < 0)
	{
		cJSON_Delete(root);
		return -6;
	}


	ty_usb_m_out("�����������ĵ�ǰmqtt֤���ļ�md5ֵΪ��%s\n", crt_md5);
	cJSON_Delete(root);
	return 0;
}

static int analysis_mqtt_info_data(char *data, char *server_addr, int *port, char *uid, char *pwd)
{

	char protocol[50];
	char code_type[10];
	char cmd[50];
	char result_s[2];
	char mqtt_server[200];
	char mqtt_uid[20];
	char mqtt_pwd[50];
	char mqtt_port[6];
	char ca_cert_base[4000];
	char client_cert_base[4000];
	char client_key_base[4000];

	memset(protocol, 0, sizeof(protocol));
	memset(code_type, 0, sizeof(code_type));
	memset(cmd, 0, sizeof(cmd));
	memset(result_s, 0, sizeof(result_s));
	memset(mqtt_server, 0, sizeof(mqtt_server));
	memset(mqtt_port, 0, sizeof(mqtt_port));
	cJSON *item;
	cJSON *root = cJSON_Parse(data);
	if (!root)
	{
		ty_usb_m_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}

	item = cJSON_GetObjectItem(root, "server_addr");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_server, 0, sizeof(mqtt_server));
		memcpy(mqtt_server, item->valuestring, strlen(item->valuestring));
		memcpy(server_addr, mqtt_server, strlen(item->valuestring));
	}
	ty_usb_m_out("��ȡ����MQTT��������ַΪ%s\n", mqtt_server);
	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_port, 0, sizeof(mqtt_port));
		memcpy(mqtt_port, item->valuestring, strlen(item->valuestring));
		*port = atoi(mqtt_port);
	}
	ty_usb_m_out("��ȡ����MQTT�������˿ں�Ϊ%s\n", mqtt_port);



	item = cJSON_GetObjectItem(root, "uid");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_uid, 0, sizeof(mqtt_pwd));
		memcpy(mqtt_uid, item->valuestring, strlen(item->valuestring));
		memcpy(uid, mqtt_uid, strlen(item->valuestring));
	}
	ty_usb_m_out("��ȡ����MQTT uid%s\n", uid);
	item = cJSON_GetObjectItem(root, "pwd");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(mqtt_pwd, 0, sizeof(mqtt_pwd));
		memcpy(mqtt_pwd, item->valuestring, strlen(item->valuestring));
		memcpy(pwd, mqtt_pwd, strlen(item->valuestring));
	}
	ty_usb_m_out("��ȡ����MQTT pwd%s\n", pwd);



	item = cJSON_GetObjectItem(root, "ca_crt");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(ca_cert_base, 0, sizeof(ca_cert_base));
		memcpy(ca_cert_base, item->valuestring, strlen(item->valuestring));
	}
	//ty_usb_m_out("��ȡ����ca.crt֤���ļ�Ϊ%s\n", ca_cert_base);

	item = cJSON_GetObjectItem(root, "client_crt");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(client_cert_base, 0, sizeof(client_cert_base));
		memcpy(client_cert_base, item->valuestring, strlen(item->valuestring));
	}
	//ty_usb_m_out("��ȡ����client.crt֤���ļ�Ϊ%s\n", client_cert_base);

	item = cJSON_GetObjectItem(root, "client_key");
	if (item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	if (item->valuestring != NULL)
	{
		memset(client_key_base, 0, sizeof(client_key_base));
		memcpy(client_key_base, item->valuestring, strlen(item->valuestring));
	}
	//ty_usb_m_out("��ȡ����client.key֤���ļ�Ϊ%s\n", client_key_base);

	char tmp[4000];
	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/ca.crt", tmp, sizeof(tmp))<0)
	{
		ty_usb_m_out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/ca.crt");
		save_cert_file(ca_cert_base, "/etc/ca.crt");
	}
	else
	{
		if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
		{
			ty_usb_m_out("ca.cert�ļ���һ����Ҫ����\n");
			save_cert_file(ca_cert_base, "/etc/ca.crt");
		}
	}
	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/client.crt", tmp, sizeof(tmp))<0)
	{
		ty_usb_m_out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/client.crt");
		save_cert_file(client_cert_base, "/etc/client.crt");
	}
	else
	{
		if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
		{
			ty_usb_m_out("client.cert�ļ���һ����Ҫ����\n");
			save_cert_file(client_cert_base, "/etc/client.crt");
		}
	}


	memset(tmp, 0, sizeof(tmp));
	if (read_file("/etc/client.key", tmp, sizeof(tmp))<0)
	{
		ty_usb_m_out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/client.key");
		save_cert_file(client_key_base, "/etc/client.key");
	}
	else
	{
		if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
		{
			ty_usb_m_out("client.key�ļ���һ����Ҫ����\n");
			save_cert_file(client_key_base, "/etc/client.key");
		}
	}
	cJSON_Delete(root);
	return 0;
}


int function_get_mqtt_connect_data_md5_jude_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd)
{

	//timer_read_asc(ter_time);
	int ret;
	char *r_buf;
	char *outdata;
	int outdatalen;
	int r_len;
	char *json_data;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;

	mb_enc_get_mqtt_connect_info_json(stream->key_s, stream->ter_id, &json_data);

	if (json_data == NULL)
	{
		ty_usb_m_out("�����������\n");
		return -1;
	}

	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));


	/*sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;*/

#ifndef DF_OLD_MSERVER										//wang ��Ӷ�������̨�궨��  ifndefδ��������̨�궨��
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");			//����̨��ַ
	parm.s_port = 20020;									
	//sprintf(parm.s_ip, "%s", "192.168.0.129");
	//parm.s_port = 8000;
#else
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;
#endif

	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);

	ret = http_token_get_mqtt_connect_md5(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		ty_usb_m_out("function_get_mqtt_connect_data_md5_jude_update �����������ʧ��,ret = %d\n",ret);
		free(json_data);
		return ret;
	}
	free(json_data);
	mb_dec_file_function(stream->key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	free(r_buf);


	char crt_md5[200] = { 0 };
	ret = analysis_mqtt_connect_data_md5_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd, crt_md5);
	if (ret < 0)
	{
		ty_usb_m_out("function_get_mqtt_connect_data_md5_jude_update ����json���ݽ���ʧ��,ret = %d,data %s\n", ret, outdata);
		free(outdata);
		return -1;
	}
	free(outdata);
	char local_file[81920] = { 0 };
	read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));

	//ty_usb_m_out("local_file %s", local_file);

	char md5_data[200] = { 0 };
	md5_value(local_file, strlen(local_file), md5_data);


	ty_usb_m_out("��Ʊ����������mqtt֤���ļ�md5ֵΪ��%s\n", md5_data);

	if (strcmp(md5_data, crt_md5) == 0)
	{
		ty_usb_m_out("����֤���ļ��������֤���ļ�md5ֵһ��,����Ҫ����\n");
		return 0;
	}

	ty_usb_m_out("����֤���ļ��������֤���ļ�md5ֵ��һ��,��Ҫ����\n");
	return 1;
}


int function_get_mqtt_connect_data_info_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd)
{

	//timer_read_asc(ter_time);
	int ret;
	char *r_buf;
	char *outdata;
	int outdatalen;
	int r_len;
	char *json_data;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	mb_enc_get_mqtt_connect_info_json(stream->key_s, stream->ter_id, &json_data);
	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
	
	
	/*sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;*/

#ifndef DF_OLD_MSERVER										//wang ��Ӷ�������̨�궨��  ifndefδ��������̨�궨��
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");			//����̨��ַ
	parm.s_port = 20020;									
	//sprintf(parm.s_ip, "%s", "192.168.0.129");
	//parm.s_port = 8000;
#else
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;
#endif

	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);

	ret = http_token_get_mqtt_connect_info(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		ty_usb_m_out("function_get_mqtt_connect_data_info_update �����������ʧ��,ret = %d\n", ret);
		free(json_data);
		return ret;
	}
	free(json_data);
	mb_dec_file_function(stream->key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	free(r_buf);


	analysis_mqtt_info_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd);
	free(outdata);
	if (strcmp(mqtt_uid, stream->ter_id) != 0)
	{
		logout(INFO, "system", "mqtt��Ϣ��ȡ", "���������ص��ն�ID�뱾������\r\n");
		return -1;
	}

	
	//=======================================================================================//
	char tmp[1024] = { 0 };
	char cmd[1024] = { 0 };
	sprintf(tmp, "#=====MQTT SERVER SET=====#\r\n  #MQTT SERVER ADDRESS\r\nserver_address = %s\r\n  #MQTT SERVER PORT\r\nserver_port = %d\r\n", server_addr, *server_port);
	sprintf(cmd, "rm %s", DF_CONFIG_FILE_PATH);
	system(cmd);
	write_file(DF_CONFIG_FILE_PATH, tmp, strlen(tmp));
	sync();

	return 0;
}

int fun_upload_auth_cert_file(void)
{
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;

// #ifdef RELEASE_SO
// 	so_common_reload_tax_auth(stream->key_s);
// 	so_common_get_cert_json_file(stream->key_s, &stream->cert_json);
// #else
// 	function_common_reload_tax_auth(stream->key_s);
// 	function_common_get_cert_json_file(stream->key_s, &stream->cert_json);
// #endif	

	check_port_auth_info(stream);

	int port;
	struct _port_state   *state;
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		if (state->port_info.with_dev != 0)
		{
			//if (state->port_info.usb_app == DEVICE_AISINO ||
			//	state->port_info.usb_app == DEVICE_NISEC ||
			//	state->port_info.usb_app == DEVICE_CNTAX ||
			//	state->port_info.usb_app == DEVICE_MENGBAI)
			//{
			//	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			//	state->last_act = 1;
			//}
			//else
			//{
			//	state->port_info.tax_read = 1;
			//}

			state->port_info.tax_read = 1;
			printf("state->port_info.tax_read = 1;11\r\n");
		}
	}
	return 0;
	

}
