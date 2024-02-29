#define _ty_usb_m_c
#include "_ty_usb_m.h"

int ty_usb_m_add(const char *usb_name,int dev_type)
{   struct _ty_usb_m  *stream;
	int result;
	stream=malloc(sizeof(struct _ty_usb_m));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
    //out("����:ty_usb_mģ��\n");
	memset(stream,0,sizeof(struct _ty_usb_m));
    memcpy(stream->ty_usb_name,usb_name,strlen(usb_name));
    //out("��ȡ����usbip_name[%s],usb_name[%s]\n",stream->usbip_name,stream->ty_usb_name);
	result = ty_file_add(DF_DEV_USB_M, stream, "ty_usb_m", (struct _file_fuc *)&ty_usb_m_fuc);
	if(result<0)
		free(stream);
	ty_usb_m_id = stream;
    //out("����:ty_usb_mģ��ɹ�\n");
	return result; 
}

static int ty_usb_m_open(struct ty_file	*file)
{   struct _ty_usb_m  *stream;
    int j;
	int i;
	int readkeylen = 0;
	//��ȡ��Ƭ����-Կ��;
	int result;
	uint16 crc_ret = 0;
	
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    if(stream->state==0)
    {   //out("��:[%s]�ļ�\n",stream->ty_usb_name);
		//out("�״�������ȡ��˰�����ƽ���\n\n\n\n");

#ifndef DF_1201_1
        stream->ty_usb_fd=ty_open(stream->ty_usb_name,0,0,NULL);
        if(stream->ty_usb_fd<0)
        {   
			logout(INFO, "system", "init", "���ļ�ʧ�� ty_usb_m_open\r\n");//wang
			out("���ļ�[%s]ʧ��:[%d]\n",stream->ty_usb_name,stream->ty_usb_fd);
            return stream->ty_usb_fd;
        }		
		stream->usb_port_numb = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
		//out("stream->usb_port_num = %d\n", stream->usb_port_numb);
		
		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);
		ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_MACHINE_ID, stream->ter_id);

		if (access("/etc/init.d/Mbipc", 0) != 0)
		{
			system("chmod +x /etc/npc");
			sync();
			system("/etc/npc install");
			sync();
			system("/etc/init.d/Mbipc start &");			
		}
		else system("/etc/init.d/Mbipc start &");

		system("rm -rf /home/share/exe/tar/tmpapp");
		sync();		

#endif
		result = ty_ctl(stream->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, stream->key_s);
		if(result < 0)
		{
			logout(INFO, "system", "init", "����Ƭ����Կʧ��\r\n");//wang
			out("����Ƭ����Կʧ��\n");
			stream->keyfile_load_err = 1;
		}
		if(stream->keyfile_load_err == 1)		//����Կʧ����
		{
			memset(stream->key_s,0,4096);
			readkeylen = read_file("/etc/key_s_back", (char *)stream->key_s, 4096);
			printf("readkeylen=%d\r\n",readkeylen);
		}
		else 
		{
			write_file("/etc/key_s_back", (char *)stream->key_s, 4096);
		}
		crc_ret = crc16_MAXIM(stream->key_s,4096);
		logout(INFO, "system", "init", "��Ƭ����ԿУ��ֵΪ%x\r\n",crc_ret);//wang
#ifdef RELEASE_SO			
		//���ܶ�̬��
		/*result = load_tax_lib_so(stream->key_s);
		if (result < 0)
		{
			logout(INFO, "system", "init", "���ܶ�̬��ʧ��\r\n");//wang
			out("���ܶ�̬��ʧ��\n");
			stream->lib_dec_err = 1;
			stream->auth_dec_err = 1;
			//return -100;
		}*/

		int k;
		for ( k = 0; k < 3; k++)   
		{
			result = load_tax_lib_so(stream->key_s);  //��������0 
						
			if(result == 0)
			{
				out("���ܶ�̬��--��%d�γɹ�\r\n",k+1);
				break;
			}
				
			if (result < 0 && k == 2)
			{
				logout(INFO, "system", "init", "���ܶ�̬��ʧ��\r\n");//wang
				out("���ܶ�̬��ʧ��\n");
				stream->lib_dec_err = 1;
				stream->auth_dec_err = 1;
				//return -100;
			}

		}

		//��̬���ؿ�
		//wang 02 13
		if (stream->lib_dec_err != 1)
		{
			result = load_tax_lib_so_fun();
			if (result < 0)
			{
				logout(INFO, "system", "init", "��̬���ؿ�ʧ��\r\n");//wang
				out("��̬���ؿ�ʧ��\n");
				stream->lib_load_err= 1;
				//return -101;
			}
		}
		else 
		{
			result = load_tax_lib_so_fun_wang();
			if (result < 0)
			{
				logout(INFO, "system", "init", "��̬���ؿ����ʧ��\r\n");//wang
				out("��̬���ؿ�ʧ��\n");
				stream->lib_load_err= 1;
				//return -101;
			}	
			else{
				stream->lib_dec_err = 0;	//�����ٴμ��سɹ����ͷŸñ�־λ
				logout(INFO, "system", "init", "��̬���ؿ���γɹ�,�����ܵ��¡���Ȩ�˿���Ϊ0��,����������\r\n");//wang
			}		
		}
		//��ʼ����̬��
		//logout(INFO, "system", "init", "��̬���־λ״̬lib_dec_err=%d,lib_load_err=%d\r\n", stream->lib_dec_err,stream->lib_load_err);//wang
		logout(INFO, "system", "init", "��̬���־λ״̬lib_dec_err=%d\r\n", stream->lib_dec_err);// zwl 0802
		if (stream->lib_dec_err != 1 && stream->lib_load_err != 1)
		{
			out("���ڳ�ʼ����̬��!\n");
			result = so_common_init_tax_lib(stream->key_s, stream->tax_so_ver);
			if (result <= 0)
			{
				out("��ʼ����̬��\n");
				stream->auth_dec_err= 1;

			}
			//so_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
		}
		//logout(INFO, "system", "init", "******��ȡ���ļ��ܿ�汾��Ϊ[%s]******\r\n", stream->tax_so_ver);			//wang
		out("******��ȡ���ļ��ܿ�汾��Ϊ[%s]******\r\n", stream->tax_so_ver); //zwl out 

#else
		result = function_common_init_tax_lib(stream->key_s, stream->tax_so_ver);

		//function_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
#endif
		sprintf(stream->mbc_so_ver, "%s", MB_CLIB_VERSION);
		out("��Ȩ�˿�������%d\n", result);
		//logout(INFO, "system", "init", "��Ȩ�˿�����:%d\r\n", result);
		out( "��Ȩ�˿�����:%d\r\n", result); //zwl 0802
        //out("��:[%s]�ļ�\n",stream->usbip_name);
		//stream->usb_port_numb = 1;
		if (result <= 0)
		{
			lcd_print_info.tax_en = 1;
		}
		else
		{
#ifdef RELEASE_SO
			so_common_get_cert_json_file(stream->key_s,&stream->cert_json);
#else
			function_common_get_cert_json_file(stream->key_s, &stream->cert_json);
#endif
			//printf("cert json =%s\n", stream->cert_json);
			lcd_print_info.tax_en = 1;
		}
		check_port_auth_info(stream);
#ifndef DF_1201_1
		stream->lcd_fd = ty_open("/dev/lcd_state", 0, 0, NULL);
		ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
#endif
		
		stream->last_report_time_o = get_time_sec();//������
		stream->last_report_time_h = get_time_sec();//��̨����
		stream->last_report_time_s = get_time_sec();//������Ϣ
        //stream->usbip_fd=ty_open(stream->usbip_name,0,0,NULL);
        //if(stream->usbip_fd<0)
        //{   out("���ļ�[%s]ʧ��:[%d]\n",stream->usbip_name,stream->usbip_fd);
        //    ty_close(stream->ty_usb_fd);
        //    return stream->usbip_fd;
        //}
#ifndef DF_1201_1
		stream->tran_fd=ty_open("/dev/tran_data",0,0,NULL);
		if(stream->tran_fd<0)
		{
			logout(INFO, "system", "init", "��ת��ģ��ʧ��\r\n");
			out("���ļ���/dev/tran_fdʧ��\n");
			return stream->tran_fd;
		}
		stream->deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		if (stream->deploy_fd<0)
		{
			logout(INFO, "system", "init", "������ģ��ʧ��\r\n");
			out("���ļ���/dev/deploy_fdʧ��\n");
			return stream->deploy_fd;
		}

		stream->event_file_fd = ty_open("/dev/event", 0, 0, NULL);
		if (stream->event_file_fd < 0)
		{
			logout(INFO, "system", "init", "���¼�ģ��ʧ��\r\n");
			out("���ļ���/dev/event_file_fdʧ��\n");
			return stream->event_file_fd;
		}
		stream->switch_fd = ty_open("/dev/switch", 0, 0, NULL);
		if (stream->switch_fd<0)
		{
			logout(INFO, "system", "init", "�򿪵�Ƭ������ģ��ʧ��\r\n");
			out("�򿪿����¼�ʧ��\n");
			return stream->switch_fd;
		}
		stream->get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
		
		ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_SUPPORT, &stream->dev_support);
		check_stop_upload(&stream->stop_aisino_up, &stream->stop_nisec_up, &stream->stop_cntax_up, &stream->stop_mengbai_up);
		//out("��tran_data�ɹ�\n");
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

		#if (defined DF_2212_1) || (defined DF_2212_8) || (defined DF_2216_1) || (defined DF_2216_8) //zwl 0504  
			if(stream->dev_support.support_power == 0)  //wang 0428   //��Ҫ���120 240�� ������ϵͳ376
			{
				stream->dev_support.support_power = 1;
			}
			read_power_mode = 0;
			out("---zwl--�ڵ�ģʽ��--1--\n");  //zwl 0504
		#endif

		out("�����˿��ϵ�\n");
		all_port_power_open(stream);
		out("�����˿ڼ���߳�\n");
		if (stream->dev_support.support_power == 1)
		{
			for (j = 0; j < stream->usb_port_numb; j++)
			{		
				stream->port[j].power_mode = Saving_Mode;//�ڵ�������ж˿�ȫ��Ĭ��Ϊ�ڵ�
			}
			_task_open("ty_usb_m", task_check_port_ad_status, stream, 200, NULL);	
			_task_open("ty_usb_m", task_port_power_up_down, stream, 200, NULL);
			_task_open("ty_usb_m", task_port_repair_ad_error, stream, 200, NULL);
			_task_open("ty_usb_m", task_check_power_down_dev_remove, stream, 200, NULL);			
		}
		
		//out("�����߳�task_m\n");
		_task_open("ty_usb_m",task_m,stream,400,NULL);	

		//out("�������˿��豸��ζ����߳�");
		_task_open("ty_usb_m", task_check_port, stream, 30, NULL);      


		if (memcmp(stream->ter_id, "068180", 6) == 0) //��
		{
			//068180010960 - 068180021950
			//068180005925 - 068180005941
			int ter_id_num;
			char ter_id_num_str[200] = { 0 };
			memcpy(ter_id_num_str, stream->ter_id + 7, 4);
			ter_id_num = atoi(ter_id_num_str);
			out("����������Ϊ%d\n", ter_id_num);

			if (((ter_id_num >= 1096) && (ter_id_num <= 2195)) || ((ter_id_num >= 592) && (ter_id_num <= 594)) || ((ter_id_num >= 468) && (ter_id_num <= 470)))
			{
				_task_open("ty_usb_m", task_check_port_repair_51err, stream, 1000, NULL);
			}
		}
		//out("������CA�̼߳�������˰����Ϣ�߳�\n");
#ifdef RELEASE_SO
		if (stream->lib_dec_err != 1 && stream->lib_load_err != 1)
		{
			_task_open("ty_usb_m", task_read_port_base_tax_info, stream, 10, NULL);
		}
		else
		{
#ifdef PRODUCE_PROGRAM
			_task_open("ty_usb_m", task_read_port_base_tax_info_simple, stream, 10, NULL);
#endif
		}
#else
		_task_open("ty_usb_m", task_read_port_base_tax_info, stream, 10, NULL);
#endif		
		//out("�������ƹ����߳�\n");
		_task_open("ty_usb_m", task_port_led_manage, stream, 500, NULL);

#ifndef PRODUCE_PROGRAM		
		sem_init(&stream->cb_qk_sem, 0, 0);//10�µ�ͳһ�޸�,ȡ���ź���

		_task_open("ty_usb_m", timely_tigger_report_status, stream, 100, NULL);

		_task_open("ty_usb_m", timely_tigger_report_port_status, stream, 1, NULL);

#ifndef DF_OLD_MSERVER
		_task_open("ty_usb_m", timely_tigger_get_plate_invoice_db_info, stream, 1, NULL);
#endif
		_task_open("ty_usb_m", timely_tigger_deal_plate, stream, 100, NULL);


		_task_open("ty_usb_m", report_offline_inv_to_server, stream, 100, NULL);

		_task_open("ty_usb_m", auto_cb_jkxxhc_thread, stream, 100, NULL);
		_task_open("ty_usb_m", timely_tigger_auto_cb_jkxxhc, stream, 15 * 1000 * 60, NULL);

		//_task_open("ty_usb_m", timely_tigger_sync_plate_cpy_info, stream,4 * 60 * 60 * 1000, NULL);
#endif
#else
	_task_open("ty_usb_m", timely_tigger_report_status, stream, 100, NULL);
#endif	
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
    {   out("Ҫ��ȫ���ر�\n");
        _lock_close(stream->lock);
        for(j=0;j<sizeof(stream->port)/sizeof(stream->port[0]);j++)
            _lock_close(stream->port[j].lock);
        timer_close(stream->timer_fd);
        ty_close(stream->ty_usb_fd);
        //ty_close(stream->usbip_fd);
    }
    return 0;   
}

//wang crc
static uint16 crc16_MAXIM(uint8 *addr, int num)  
{  
    uint16 crc = 0x0000;//��ʼֵ
    int i;  
    for (; num > 0; num--)             
    {  
        crc = crc ^ (*addr++) ;     //��8λ���
        for (i = 0; i < 8; i++)             
        {  
            if (crc & 0x0001)       //����ǰ��ͺ���ʡȥ�˷�ת���������������ƣ�������ֵΪ����ʽ�ķ�תֵ
                crc = (crc >> 1) ^ 0xA001;//���ƺ������ʽ��ת�����
            else                   //����ֱ������
                crc >>= 1;                    
        }                               
    }
    return(crc^0xffff);            //����У��ֵ 
} 


#ifdef RELEASE_SO
static int load_tax_lib_so(unsigned char *key_file)
{
	char tax_so_enc[2 * 1024 * 1024] = { 0 };
	int so_enc;
	char *so_data;
	int so_len;
	int result;
	so_enc = read_file("/etc/tax_so.json", tax_so_enc, 2 * 1024 * 1024);
	result = mb_dec_file_function(key_file, tax_so_enc, so_enc, &so_data, &so_len);
	if (result < 0)
	{
		out("���ܿ����ʧ��\n");
		return -1;
	}

#ifndef CESHI
	#ifdef MTK_OPENWRT
		write_file("/tmp/libmb_taxlib.so", so_data, so_len);
	#else
		write_file("/lib/libmb_taxlib.so", so_data, so_len);
	#endif
#endif
	sync();
	if (so_data != NULL)
		free(so_data);
	return 0;
}
static int load_tax_lib_so_fun_wang(void)
{
	void *handle;
#ifdef MTK_OPENWRT
	handle = dlopen("/tmp/libmb_taxlib_back.so", RTLD_LAZY);
#else
	handle = dlopen("/lib/libmb_taxlib_back.so", RTLD_LAZY);
#endif
	if (NULL == handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		printf("Can not open the libmb_taxlib.so\n");
		return -1;
	}

	//==ͨ�ýӿ�==//
	so_common_init_tax_lib = dlsym(handle, "function_common_init_tax_lib");
	if (NULL == so_common_init_tax_lib)
	{
		printf("load function so_common_init_tax_lib error\n");
		dlclose(handle);
		return -2;
	}

	so_common_reload_tax_auth = dlsym(handle, "function_common_reload_tax_auth");
	if (NULL == so_common_reload_tax_auth)
	{
		printf("load function so_common_reload_tax_auth error\n");
		dlclose(handle);
		return -2;
	}

	so_common_jude_plate_auth = dlsym(handle, "function_common_jude_plate_auth");
	if (NULL == so_common_jude_plate_auth)
	{
		printf("load function so_common_jude_plate_auth error\n");
		dlclose(handle);
		return -2;
	}

	so_common_find_inv_type_mem = dlsym(handle, "function_common_find_inv_type_mem");
	if (NULL == so_common_find_inv_type_mem)
	{
		printf("load function so_common_find_inv_type_mem error\n");
		dlclose(handle);
		return -2;
	}

	so_common_get_cert_json_file = dlsym(handle, "function_common_get_cert_json_file");
	if (NULL == so_common_get_cert_json_file)
	{
		printf("load function so_common_get_cert_json_file error\n");
		dlclose(handle);
		return -2;
	}


	so_common_report_event =  dlsym(handle, "function_common_report_event");
	if (NULL == so_common_report_event)
	{
		printf("load function so_common_report_event error\n");
		dlclose(handle);
		return -2;
	}

	so_common_analyze_json_buff =  dlsym(handle, "function_common_analyze_json_buff");
	if (NULL == so_common_analyze_json_buff)
	{
		printf("load function so_common_analyze_json_buff error\n");
		dlclose(handle);
		return -2;
	}

	so_common_get_basic_tax_info = dlsym(handle, "function_common_get_basic_tax_info");
	if (NULL == so_common_get_basic_tax_info)
	{
		printf("load function so_common_get_basic_tax_info error\n");
		dlclose(handle);
		return -2;
	}
	so_common_get_basic_simple = dlsym(handle, "function_common_get_basic_simple");
	if (NULL == so_common_get_basic_simple)
	{
		printf("load function so_common_get_basic_simple error\n");
		dlclose(handle);
		return -2;
	}

	so_common_close_usb_device = dlsym(handle, "function_common_close_usb_device");
	if (NULL == so_common_close_usb_device)
	{
		printf("load function so_common_close_usb_device error\n");
		dlclose(handle);
		return -2;
	}

	//==����Ҫ��Ȩ�Ҳ���Ҫ������֤ͨ����ɵ��õĽӿ�==//
	so_aisino_get_tax_time = dlsym(handle, "function_aisino_get_tax_time");
	if (NULL == so_aisino_get_tax_time)
	{
		printf("load function so_aisino_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_tax_time = dlsym(handle, "function_nisec_get_tax_time");
	if (NULL == so_nisec_get_tax_time)
	{
		printf("load function so_nisec_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_tax_time = dlsym(handle, "function_cntax_get_tax_time");
	if (NULL == so_cntax_get_tax_time)
	{
		printf("load function so_cntax_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_plate_time = dlsym(handle, "function_mengbai_get_plate_time");
	if (NULL == so_mb_get_plate_time)
	{
		printf("load function so_mb_get_plate_time error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_get_inv_details_mqtt = dlsym(handle, "function_aisino_get_inv_details_mqtt");
	if (NULL == so_aisino_get_inv_details_mqtt)
	{
		printf("load function so_aisino_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_inv_details_mqtt = dlsym(handle, "function_nisec_get_inv_details_mqtt");
	if (NULL == so_nisec_get_inv_details_mqtt)
	{
		printf("load function so_nisec_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_inv_details_mqtt = dlsym(handle, "function_cntax_get_inv_details_mqtt");
	if (NULL == so_cntax_get_inv_details_mqtt)
	{
		printf("load function so_cntax_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_inv_details_mqtt = dlsym(handle, "function_mengbai_get_inv_details_mqtt");
	if (NULL == so_mb_get_inv_details_mqtt)
	{
		printf("load function so_mb_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_upload_m_server = dlsym(handle, "function_aisino_upload_m_server");
	if (NULL == so_aisino_upload_m_server)
	{
		printf("load function so_aisino_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_upload_m_server = dlsym(handle, "function_nisec_upload_m_server");
	if (NULL == so_nisec_upload_m_server)
	{
		printf("load function so_nisec_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_upload_m_server = dlsym(handle, "function_cntax_upload_m_server");
	if (NULL == so_cntax_upload_m_server)
	{
		printf("load function so_cntax_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_upload_m_server = dlsym(handle, "function_mengbai_upload_m_server");
	if (NULL == so_mb_upload_m_server)
	{
		printf("load function so_mb_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_get_month_invoice_num = dlsym(handle, "function_aisino_get_month_invoice_num");
	if (NULL == so_aisino_get_month_invoice_num)
	{
		printf("load function so_aisino_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_month_invoice_num = dlsym(handle, "function_nisec_get_month_invoice_num");
	if (NULL == so_nisec_get_month_invoice_num)
	{
		printf("load function so_nisec_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_month_invoice_num = dlsym(handle, "function_cntax_get_month_invoice_num");
	if (NULL == so_cntax_get_month_invoice_num)
	{
		printf("load function so_cntax_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_month_invoice_num = dlsym(handle, "function_mengbai_get_month_invoice_num");
	if (NULL == so_mb_get_month_invoice_num)
	{
		printf("load function so_mb_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_fpdm_fphm_get_invs = dlsym(handle, "function_aisino_fpdm_fphm_get_invs");
	if (NULL == so_aisino_fpdm_fphm_get_invs)
	{
		printf("load function so_aisino_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_fpdm_fphm_get_invs = dlsym(handle, "function_nisec_fpdm_fphm_get_invs");
	if (NULL == so_nisec_fpdm_fphm_get_invs)
	{
		printf("load function so_nisec_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_fpdm_fphm_get_invs = dlsym(handle, "function_cntax_fpdm_fphm_get_invs");
	if (NULL == so_cntax_fpdm_fphm_get_invs)
	{
		printf("load function so_cntax_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_aisino_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_aisino_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_aisino_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_nisec_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_nisec_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_nisec_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_cntax_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_cntax_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_cntax_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}

	//==��Ҫ��Ȩδ��֤�����������ɵ��õĽӿ�==//
	so_aisino_get_auth_passwd = dlsym(handle, "function_aisino_get_auth_passwd");
	if (NULL == so_aisino_get_auth_passwd)
	{
		printf("load function so_aisino_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_get_auth_passwd = dlsym(handle, "function_nisec_get_auth_passwd");
	if (NULL == so_nisec_get_auth_passwd)
	{
		printf("load function so_nisec_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_get_auth_passwd = dlsym(handle, "function_cntax_get_auth_passwd");
	if (NULL == so_cntax_get_auth_passwd)
	{
		printf("load function so_cntax_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_get_auth_passwd = dlsym(handle, "function_mengbai_get_auth_passwd");
	if (NULL == so_mb_get_auth_passwd)
	{
		printf("load function so_mb_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_aisino_verify_cert_passwd = dlsym(handle, "function_aisino_verify_cert_passwd");
	if (NULL == so_aisino_verify_cert_passwd)
	{
		printf("load function so_aisino_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_verify_cert_passwd = dlsym(handle, "function_nisec_verify_cert_passwd");
	if (NULL == so_nisec_verify_cert_passwd)
	{
		printf("load function so_nisec_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_verify_cert_passwd = dlsym(handle, "function_cntax_verify_cert_passwd");
	if (NULL == so_cntax_verify_cert_passwd)
	{
		printf("load function so_cntax_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_verify_cert_passwd = dlsym(handle, "function_mengbai_verify_cert_passwd");
	if (NULL == so_mb_verify_cert_passwd)
	{
		printf("load function so_mb_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_aisino_change_certpassword = dlsym(handle, "function_aisino_change_certpassword");
	if (NULL == so_aisino_change_certpassword)
	{
		printf("load function so_aisino_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_change_certpassword = dlsym(handle, "function_nisec_change_certpassword");
	if (NULL == so_nisec_change_certpassword)
	{
		printf("load function so_nisec_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_change_certpassword = dlsym(handle, "function_cntax_change_certpassword");
	if (NULL == so_cntax_change_certpassword)
	{
		printf("load function so_cntax_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_change_certpassword = dlsym(handle, "function_mengbai_change_certpassword");
	if (NULL == so_mb_change_certpassword)
	{
		printf("load function so_mb_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}

	so_aisino_get_current_invcode = dlsym(handle, "function_aisino_get_current_invcode");
	if (NULL == so_aisino_get_current_invcode)
	{
		printf("load function so_aisino_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_get_current_invcode = dlsym(handle, "function_nisec_get_current_invcode");
	if (NULL == so_nisec_get_current_invcode)
	{
		printf("load function so_nisec_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_get_current_invcode = dlsym(handle, "function_cntax_get_current_invcode");
	if (NULL == so_cntax_get_current_invcode)
	{
		printf("load function so_cntax_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_get_current_invcode = dlsym(handle, "function_mengbai_get_current_invcode");
	if (NULL == so_mb_get_current_invcode)
	{
		printf("load function so_mb_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}

	//======��Ҫ��Ȩ����֤������ȷ�󷽿ɲ����Ľӿ�======//
	so_aisino_copy_report_data = dlsym(handle, "function_aisino_copy_report_data");
	if (NULL == so_aisino_copy_report_data)
	{
		printf("load function so_aisino_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_copy_report_data = dlsym(handle, "function_nisec_copy_report_data");
	if (NULL == so_nisec_copy_report_data)
	{
		printf("load function so_nisec_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_copy_report_data = dlsym(handle, "function_cntax_copy_report_data");
	if (NULL == so_cntax_copy_report_data)
	{
		printf("load function so_cntax_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_copy_report_data = dlsym(handle, "function_mengbai_copy_report_data");
	if (NULL == so_mb_copy_report_data)
	{
		printf("load function so_mb_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_report_summary_write_back = dlsym(handle, "function_aisino_report_summary_write_back");
	if (NULL == so_aisino_report_summary_write_back)
	{
		printf("load function so_aisino_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_report_summary_write_back = dlsym(handle, "function_nisec_report_summary_write_back");
	if (NULL == so_nisec_report_summary_write_back)
	{
		printf("load function so_nisec_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_report_summary_write_back = dlsym(handle, "function_cntax_report_summary_write_back");
	if (NULL == so_cntax_report_summary_write_back)
	{
		printf("load function so_cntax_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_report_summary_write_back = dlsym(handle, "function_mengbai_report_summary_write_back");
	if (NULL == so_mb_report_summary_write_back)
	{
		printf("load function so_mb_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_inv_upload_server = dlsym(handle, "function_aisino_inv_upload_server");
	if (NULL == so_aisino_inv_upload_server)
	{
		printf("load function so_aisino_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_inv_upload_server = dlsym(handle, "function_nisec_inv_upload_server");
	if (NULL == so_nisec_inv_upload_server)
	{
		printf("load function so_nisec_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_inv_upload_server = dlsym(handle, "function_cntax_inv_upload_server");
	if (NULL == so_cntax_inv_upload_server)
	{
		printf("load function so_cntax_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_inv_upload_server = dlsym(handle, "function_mengbai_inv_upload_server");
	if (NULL == so_mb_inv_upload_server)
	{
		printf("load function so_mb_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_fpdm_fphm_update_invs = dlsym(handle, "function_aisino_fpdm_fphm_update_invs");
	if (NULL == so_aisino_fpdm_fphm_update_invs)
	{
		printf("load function so_aisino_fpdm_fphm_update_invs error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_fpdm_fphm_update_invs = dlsym(handle, "function_nisec_fpdm_fphm_update_invs");
	if (NULL == so_nisec_fpdm_fphm_update_invs)
	{
		printf("load function so_nisec_fpdm_fphm_update_invs error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_test_server_connect = dlsym(handle, "function_aisino_test_server_connect");
	if (NULL == so_aisino_test_server_connect)
	{
		printf("load function so_aisino_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_test_server_connect = dlsym(handle, "function_nisec_test_server_connect");
	if (NULL == so_nisec_test_server_connect)
	{
		printf("load function so_nisec_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_test_server_connect = dlsym(handle, "function_cntax_test_server_connect");
	if (NULL == so_cntax_test_server_connect)
	{
		printf("load function so_cntax_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_test_server_connect = dlsym(handle, "function_mengbai_test_server_connect");
	if (NULL == so_mb_test_server_connect)
	{
		printf("load function so_mb_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_make_invoice = dlsym(handle, "function_aisino_make_invoice");
	if (NULL == so_aisino_make_invoice)
	{
		printf("load function so_aisino_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_make_invoice = dlsym(handle, "function_nisec_make_invoice");
	if (NULL == so_nisec_make_invoice)
	{
		printf("load function so_nisec_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_make_invoice = dlsym(handle, "function_cntax_make_invoice");
	if (NULL == so_cntax_make_invoice)
	{
		printf("load function so_cntax_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_make_invoice = dlsym(handle, "function_mengbai_make_invoice");
	if (NULL == so_mb_make_invoice)
	{
		printf("load function so_mb_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_waste_invoice = dlsym(handle, "function_aisino_waste_invoice");
	if (NULL == so_aisino_waste_invoice)
	{
		printf("load function so_aisino_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_waste_invoice = dlsym(handle, "function_nisec_waste_invoice");
	if (NULL == so_nisec_waste_invoice)
	{
		printf("load function so_nisec_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_waste_invoice = dlsym(handle, "function_cntax_waste_invoice");
	if (NULL == so_cntax_waste_invoice)
	{
		printf("load function so_cntax_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_waste_invoice = dlsym(handle, "function_mengbai_waste_invoice");
	if (NULL == so_mb_waste_invoice)
	{
		printf("load function so_mb_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_waste_null_invoice = dlsym(handle, "function_aisino_waste_null_invoice");
	if (NULL == so_aisino_waste_null_invoice)
	{
		printf("load function so_aisino_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_waste_null_invoice = dlsym(handle, "function_nisec_waste_null_invoice");
	if (NULL == so_nisec_waste_null_invoice)
	{
		printf("load function so_nisec_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_waste_null_invoice = dlsym(handle, "function_cntax_waste_null_invoice");
	if (NULL == so_cntax_waste_null_invoice)
	{
		printf("load function so_cntax_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_waste_null_invoice = dlsym(handle, "function_mengbai_waste_null_invoice");
	if (NULL == so_mb_waste_null_invoice)
	{
		printf("load function so_mb_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_check_lzfpdm_lzfphm_allow = dlsym(handle, "function_aisino_check_lzfpdm_lzfphm_allow");
	if (NULL == so_aisino_check_lzfpdm_lzfphm_allow)
	{
		printf("load function so_aisino_check_lzfpdm_lzfphm_allow error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_upload_hzxxb = dlsym(handle, "function_aisino_upload_hzxxb");
	if (NULL == so_aisino_upload_hzxxb)
	{
		printf("load function so_aisino_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_upload_hzxxb = dlsym(handle, "function_nisec_upload_hzxxb");
	if (NULL == so_nisec_upload_hzxxb)
	{
		printf("load function so_nisec_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_upload_hzxxb = dlsym(handle, "function_cntax_upload_hzxxb");
	if (NULL == so_cntax_upload_hzxxb)
	{
		printf("load function so_cntax_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_rednum= dlsym(handle, "function_aisino_download_hzxxb_from_rednum");
	if (NULL == so_aisino_download_hzxxb_from_rednum)
	{
		printf("load function so_aisino_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_download_hzxxb_from_rednum= dlsym(handle, "function_nisec_download_hzxxb_from_rednum");
	if (NULL == so_nisec_download_hzxxb_from_rednum)
	{
		printf("load function so_nisec_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_download_hzxxb_from_rednum= dlsym(handle, "function_cntax_download_hzxxb_from_rednum");
	if (NULL == so_cntax_download_hzxxb_from_rednum)
	{
		printf("load function so_cntax_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_date_range = dlsym(handle, "function_aisino_download_hzxxb_from_date_range");
	if (NULL == so_aisino_download_hzxxb_from_date_range)
	{
		printf("load function so_aisino_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_date_range_new = dlsym(handle, "function_aisino_download_hzxxb_from_date_range_new");
	if (NULL == so_aisino_download_hzxxb_from_date_range_new)
	{
		printf("load function so_aisino_download_hzxxb_from_date_range_new error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_download_hzxxb_from_date_range = dlsym(handle, "function_nisec_download_hzxxb_from_date_range");
	if (NULL == so_nisec_download_hzxxb_from_date_range)
	{
		printf("load function so_nisec_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_download_hzxxb_from_date_range = dlsym(handle, "function_cntax_download_hzxxb_from_date_range");
	if (NULL == so_cntax_download_hzxxb_from_date_range)
	{
		printf("load function so_cntax_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_cancel_hzxxb = dlsym(handle, "function_aisino_cancel_hzxxb");
	if (NULL == so_aisino_cancel_hzxxb)
	{
		printf("load function so_aisino_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_cancel_hzxxb = dlsym(handle, "function_nisec_cancel_hzxxb");
	if (NULL == so_nisec_cancel_hzxxb)
	{
		printf("load function so_nisec_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_cancel_hzxxb = dlsym(handle, "function_cntax_cancel_hzxxb");
	if (NULL == so_cntax_cancel_hzxxb)
	{
		printf("load function so_cntax_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	//so_aisino_get_download_net_invoice_coil = dlsym(handle, "function_aisino_get_download_net_invoice_coil");//�ӿ�ͣ��
	//if (NULL == so_aisino_get_download_net_invoice_coil)
	//{
	//	printf("load function so_aisino_get_download_net_invoice_coil error\n");
	//	dlclose(handle);
	//	return -5;
	//}
	so_aisino_query_net_invoice_coil = dlsym(handle, "function_aisino_query_net_invoice_coil");
	if (NULL == so_aisino_query_net_invoice_coil)
	{
		printf("load function so_aisino_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_query_net_invoice_coil = dlsym(handle, "function_nisec_query_net_invoice_coil");
	if (NULL == so_nisec_query_net_invoice_coil)
	{
		printf("load function so_nisec_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_query_net_invoice_coil = dlsym(handle, "function_cntax_query_net_invoice_coil");
	if (NULL == so_cntax_query_net_invoice_coil)
	{
		printf("load function so_cntax_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_net_invoice_coil_download_unlock = dlsym(handle, "function_aisino_net_invoice_coil_download_unlock");
	if (NULL == so_aisino_net_invoice_coil_download_unlock)
	{
		printf("load function so_aisino_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_net_invoice_coil_download_unlock = dlsym(handle, "function_nisec_net_invoice_coil_download_unlock");
	if (NULL == so_nisec_net_invoice_coil_download_unlock)
	{
		printf("load function so_nisec_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_net_invoice_coil_download_unlock = dlsym(handle, "function_cntax_net_invoice_coil_download_unlock");
	if (NULL == so_cntax_net_invoice_coil_download_unlock)
	{
		printf("load function so_cntax_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_connect_pubservice = dlsym(handle, "function_aisino_connect_pubservice");
	if (NULL == so_aisino_connect_pubservice)
	{
		printf("load function so_aisino_connect_pubservice error\n");
		dlclose(handle);
		return -5;
	}
	
	so_nisec_connect_pubservice = dlsym(handle, "function_nisec_connect_pubservice");
	if (NULL == so_nisec_connect_pubservice)
	{
		printf("load function so_nisec_connect_pubservice error\n");
		dlclose(handle);
		return -5;
	}


	so_aisino_disconnect_pubservice = dlsym(handle, "function_aisino_disconnect_pubservice");
	if (NULL == so_aisino_disconnect_pubservice)
	{
		printf("load function so_aisino_disconnect_pubservice error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_disconnect_pubservice = dlsym(handle, "function_nisec_disconnect_pubservice");
	if (NULL == so_nisec_disconnect_pubservice)
	{
		printf("load function so_nisec_disconnect_pubservice error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_client_hello = dlsym(handle, "function_aisino_client_hello");
	if (NULL == so_aisino_client_hello)
	{
		printf("load function so_aisino_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_client_hello = dlsym(handle, "function_nisec_client_hello");
	if (NULL == so_nisec_client_hello)
	{
		printf("load function so_nisec_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_client_hello = dlsym(handle, "function_cntax_client_hello");
	if (NULL == so_cntax_client_hello)
	{
		printf("load function so_cntax_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_client_auth = dlsym(handle, "function_aisino_client_auth");
	if (NULL == so_aisino_client_auth)
	{
		printf("load function so_aisino_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_client_auth = dlsym(handle, "function_nisec_client_auth");
	if (NULL == so_nisec_client_auth)
	{
		printf("load function so_nisec_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_client_auth = dlsym(handle, "function_cntax_client_auth");
	if (NULL == so_cntax_client_auth)
	{
		printf("load function so_cntax_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_get_cert = dlsym(handle, "function_aisino_get_cert");
	if (NULL == so_aisino_get_cert)
	{
		printf("load function so_aisino_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_get_cert = dlsym(handle, "function_nisec_get_cert");
	if (NULL == so_nisec_get_cert)
	{
		printf("load function so_nisec_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_get_cert = dlsym(handle, "function_cntax_get_cert");
	if (NULL == so_cntax_get_cert)
	{
		printf("load function so_cntax_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_query_invoice_month_all_data = dlsym(handle, "function_aisino_query_invoice_month_all_data");
	if (NULL == so_aisino_query_invoice_month_all_data)
	{
		printf("load function so_aisino_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_query_invoice_month_all_data = dlsym(handle, "function_nisec_query_invoice_month_all_data");
	if (NULL == so_nisec_query_invoice_month_all_data)
	{
		printf("load function so_nisec_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_query_invoice_month_all_data = dlsym(handle, "function_cntax_query_invoice_month_all_data");
	if (NULL == so_cntax_query_invoice_month_all_data)
	{
		printf("load function so_cntax_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_read_inv_coil_number_history = dlsym(handle, "function_nisec_read_inv_coil_number_history");
	if (NULL == so_nisec_read_inv_coil_number_history)
	{
		printf("load function so_nisec_read_inv_coil_number_history error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_read_inv_coil_number_history = dlsym(handle, "function_cntax_read_inv_coil_number_history");
	if (NULL == so_cntax_read_inv_coil_number_history)
	{
		printf("load function so_cntax_read_inv_coil_number_history error\n");
		dlclose(handle);
		return -5;
	}

	so_mb_server_key_info = dlsym(handle, "function_mengbai_server_key_info");
	if (NULL == so_mb_server_key_info)
	{
		printf("load function so_mb_server_key_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_login_info = dlsym(handle, "function_common_etax_login_Interface");
	if (NULL == so_etax_login_info)
	{
		printf("load function so_etax_login_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_user_query_info = dlsym(handle, "function_common_etax_user_query_Interface");
	if (NULL == so_etax_user_query_info)
	{
		printf("load function so_etax_user_query_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_relationlist_info = dlsym(handle, "function_common_etax_relationlist_Interface");
	if (NULL == so_etax_relationlist_info)
	{
		printf("load function so_etax_relationlist_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_relationchange_info = dlsym(handle, "function_common_etax_relationchange_Interface");
	if (NULL == so_etax_relationchange_info)
	{
		printf("load function so_etax_relationchange_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_invoice_query_info = dlsym(handle, "function_common_etax_invoice_query_Interface");
	if (NULL == so_etax_invoice_query_info)
	{
		printf("load function so_etax_invoice_query_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_get_cookie_info = dlsym(handle, "function_common_etax_get_cookie_Interface");
	if (NULL == so_etax_get_cookie_info)
	{
		printf("load function so_etax_get_cookie_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_invoice_query_detail_info = dlsym(handle, "function_common_etax_invoice_query_detail_Interface");
	if (NULL == so_etax_invoice_query_detail_info)
	{
		printf("load function so_etax_invoice_query_detail_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_invoice_query_down_info = dlsym(handle, "function_common_etax_invoice_query_down_Interface");
	if (NULL == so_etax_invoice_query_down_info)
	{
		printf("load function so_etax_invoice_query_down_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_rpa_invoice_issuance_info = dlsym(handle, "function_common_etax_rpa_invoice_issuance_Interface");
	if (NULL == so_etax_rpa_invoice_issuance_info)
	{
		printf("load function so_etax_rpa_invoice_issuance_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_rpa_invoice_issuance_status_info = dlsym(handle, "function_common_etax_rpa_invoice_issuance_status_Interface");
	if (NULL == so_etax_rpa_invoice_issuance_status_info)
	{
		printf("load function so_etax_rpa_invoice_issuance_status_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_credit_limit_info = dlsym(handle, "function_common_credit_limit_Interface");
	if (NULL == so_etax_credit_limit_info)
	{
		printf("load function so_etax_credit_limit_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_blue_ink_invoice_issuance_info = dlsym(handle, "function_common_etax_blue_ink_invoice_issuance_Interface");
	if (NULL == so_etax_blue_ink_invoice_issuance_info)
	{
		printf("load function so_etax_blue_ink_invoice_issuance_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxSzzhQuery_info = dlsym(handle, "function_common_etax_hzqrxxSzzhQuery_Interface");
	if (NULL == so_etax_hzqrxxSzzhQuery_info)
	{
		printf("load function so_etax_hzqrxxSzzhQuery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxSave_info = dlsym(handle, "function_common_etax_hzqrxxSave_Interface");
	if (NULL == so_etax_hzqrxxSave_info)
	{
		printf("load function so_etax_hzqrxxSave_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxQuery_info = dlsym(handle, "function_common_etax_hzqrxxQuery_Interface");
	if (NULL == so_etax_hzqrxxQuery_info)
	{
		printf("load function so_etax_hzqrxxQuery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxQueryDetail_info = dlsym(handle, "function_common_etax_hzqrxxQueryDetail_Interface");
	if (NULL == so_etax_hzqrxxQueryDetail_info)
	{
		printf("load function so_etax_hzqrxxQueryDetail_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hcstaquery_info = dlsym(handle, "function_common_hcstaquery_invoice_issuance_Interface");
	if (NULL == so_etax_hcstaquery_info)
	{
		printf("load function so_etax_hcstaquery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_fjxxpeizhi_info = dlsym(handle, "function_common_fjxxpeizhi_Interface");
	if (NULL == so_etax_fjxxpeizhi_info)
	{
		printf("load function so_etax_fjxxpeizhi_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_cjmbpeizhi_info = dlsym(handle, "function_common_cjmbpeizhi_Interface");
	if (NULL == so_etax_cjmbpeizhi_info)
	{
		printf("load function so_etax_cjmbpeizhi_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_get_nsrjcxx_info = dlsym(handle, "function_common_etax_get_nsrjcxx_Interface");
	if (NULL == so_etax_get_nsrjcxx_info)
	{
		printf("load function so_etax_get_nsrjcxx_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_get_nsrfxxx_info = dlsym(handle, "function_common_etax_get_nsrfxxx_Interface");
	if (NULL == so_etax_get_nsrfxxx_info)
	{
		printf("load function so_etax_get_nsrfxxx_info error\n");
		dlclose(handle);
		return -5;
	}

	return 0;
}

static int load_tax_lib_so_fun(void)
{
	void *handle;
#ifdef MTK_OPENWRT
	handle = dlopen("/tmp/libmb_taxlib.so", RTLD_LAZY);
#else
	handle = dlopen("/lib/libmb_taxlib.so", RTLD_LAZY);
#endif
	if (NULL == handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		printf("Can not open the libmb_taxlib.so\n");
		return -1;
	}

	//==ͨ�ýӿ�==//
	so_common_init_tax_lib = dlsym(handle, "function_common_init_tax_lib");
	if (NULL == so_common_init_tax_lib)
	{
		printf("load function so_common_init_tax_lib error\n");
		dlclose(handle);
		return -2;
	}

	so_common_reload_tax_auth = dlsym(handle, "function_common_reload_tax_auth");
	if (NULL == so_common_reload_tax_auth)
	{
		printf("load function so_common_reload_tax_auth error\n");
		dlclose(handle);
		return -2;
	}

	so_common_jude_plate_auth = dlsym(handle, "function_common_jude_plate_auth");
	if (NULL == so_common_jude_plate_auth)
	{
		printf("load function so_common_jude_plate_auth error\n");
		dlclose(handle);
		return -2;
	}

	so_common_find_inv_type_mem = dlsym(handle, "function_common_find_inv_type_mem");
	if (NULL == so_common_find_inv_type_mem)
	{
		printf("load function so_common_find_inv_type_mem error\n");
		dlclose(handle);
		return -2;
	}

	so_common_get_cert_json_file = dlsym(handle, "function_common_get_cert_json_file");
	if (NULL == so_common_get_cert_json_file)
	{
		printf("load function so_common_get_cert_json_file error\n");
		dlclose(handle);
		return -2;
	}


	so_common_report_event =  dlsym(handle, "function_common_report_event");
	if (NULL == so_common_report_event)
	{
		printf("load function so_common_report_event error\n");
		dlclose(handle);
		return -2;
	}

	so_common_analyze_json_buff =  dlsym(handle, "function_common_analyze_json_buff");
	if (NULL == so_common_analyze_json_buff)
	{
		printf("load function so_common_analyze_json_buff error\n");
		dlclose(handle);
		return -2;
	}

	so_common_get_basic_tax_info = dlsym(handle, "function_common_get_basic_tax_info");
	if (NULL == so_common_get_basic_tax_info)
	{
		printf("load function so_common_get_basic_tax_info error\n");
		dlclose(handle);
		return -2;
	}
	so_common_get_basic_simple = dlsym(handle, "function_common_get_basic_simple");
	if (NULL == so_common_get_basic_simple)
	{
		printf("load function so_common_get_basic_simple error\n");
		dlclose(handle);
		return -2;
	}

	so_common_close_usb_device = dlsym(handle, "function_common_close_usb_device");
	if (NULL == so_common_close_usb_device)
	{
		printf("load function so_common_close_usb_device error\n");
		dlclose(handle);
		return -2;
	}

	//==����Ҫ��Ȩ�Ҳ���Ҫ������֤ͨ����ɵ��õĽӿ�==//
	so_aisino_get_tax_time = dlsym(handle, "function_aisino_get_tax_time");
	if (NULL == so_aisino_get_tax_time)
	{
		printf("load function so_aisino_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_tax_time = dlsym(handle, "function_nisec_get_tax_time");
	if (NULL == so_nisec_get_tax_time)
	{
		printf("load function so_nisec_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_tax_time = dlsym(handle, "function_cntax_get_tax_time");
	if (NULL == so_cntax_get_tax_time)
	{
		printf("load function so_cntax_get_tax_time error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_plate_time = dlsym(handle, "function_mengbai_get_plate_time");
	if (NULL == so_mb_get_plate_time)
	{
		printf("load function so_mb_get_plate_time error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_get_inv_details_mqtt = dlsym(handle, "function_aisino_get_inv_details_mqtt");
	if (NULL == so_aisino_get_inv_details_mqtt)
	{
		printf("load function so_aisino_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_inv_details_mqtt = dlsym(handle, "function_nisec_get_inv_details_mqtt");
	if (NULL == so_nisec_get_inv_details_mqtt)
	{
		printf("load function so_nisec_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_inv_details_mqtt = dlsym(handle, "function_cntax_get_inv_details_mqtt");
	if (NULL == so_cntax_get_inv_details_mqtt)
	{
		printf("load function so_cntax_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_inv_details_mqtt = dlsym(handle, "function_mengbai_get_inv_details_mqtt");
	if (NULL == so_mb_get_inv_details_mqtt)
	{
		printf("load function so_mb_get_inv_details_mqtt error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_upload_m_server = dlsym(handle, "function_aisino_upload_m_server");
	if (NULL == so_aisino_upload_m_server)
	{
		printf("load function so_aisino_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_upload_m_server = dlsym(handle, "function_nisec_upload_m_server");
	if (NULL == so_nisec_upload_m_server)
	{
		printf("load function so_nisec_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_upload_m_server = dlsym(handle, "function_cntax_upload_m_server");
	if (NULL == so_cntax_upload_m_server)
	{
		printf("load function so_cntax_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_upload_m_server = dlsym(handle, "function_mengbai_upload_m_server");
	if (NULL == so_mb_upload_m_server)
	{
		printf("load function so_mb_upload_m_server error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_get_month_invoice_num = dlsym(handle, "function_aisino_get_month_invoice_num");
	if (NULL == so_aisino_get_month_invoice_num)
	{
		printf("load function so_aisino_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_get_month_invoice_num = dlsym(handle, "function_nisec_get_month_invoice_num");
	if (NULL == so_nisec_get_month_invoice_num)
	{
		printf("load function so_nisec_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_get_month_invoice_num = dlsym(handle, "function_cntax_get_month_invoice_num");
	if (NULL == so_cntax_get_month_invoice_num)
	{
		printf("load function so_cntax_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_mb_get_month_invoice_num = dlsym(handle, "function_mengbai_get_month_invoice_num");
	if (NULL == so_mb_get_month_invoice_num)
	{
		printf("load function so_mb_get_month_invoice_num error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_fpdm_fphm_get_invs = dlsym(handle, "function_aisino_fpdm_fphm_get_invs");
	if (NULL == so_aisino_fpdm_fphm_get_invs)
	{
		printf("load function so_aisino_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_fpdm_fphm_get_invs = dlsym(handle, "function_nisec_fpdm_fphm_get_invs");
	if (NULL == so_nisec_fpdm_fphm_get_invs)
	{
		printf("load function so_nisec_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_fpdm_fphm_get_invs = dlsym(handle, "function_cntax_fpdm_fphm_get_invs");
	if (NULL == so_cntax_fpdm_fphm_get_invs)
	{
		printf("load function so_cntax_fpdm_fphm_get_invs error\n");
		dlclose(handle);
		return -3;
	}
	so_aisino_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_aisino_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_aisino_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_aisino_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}
	so_nisec_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_nisec_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_nisec_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_nisec_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}
	so_cntax_fpdm_fphm_get_invs_kpstr = dlsym(handle, "function_cntax_fpdm_fphm_get_invs_kpstr");
	if (NULL == so_cntax_fpdm_fphm_get_invs_kpstr)
	{
		printf("load function so_cntax_fpdm_fphm_get_invs_kpstr error\n");
		dlclose(handle);
		return -3;
	}

	//==��Ҫ��Ȩδ��֤�����������ɵ��õĽӿ�==//
	so_aisino_get_auth_passwd = dlsym(handle, "function_aisino_get_auth_passwd");
	if (NULL == so_aisino_get_auth_passwd)
	{
		printf("load function so_aisino_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_get_auth_passwd = dlsym(handle, "function_nisec_get_auth_passwd");
	if (NULL == so_nisec_get_auth_passwd)
	{
		printf("load function so_nisec_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_get_auth_passwd = dlsym(handle, "function_cntax_get_auth_passwd");
	if (NULL == so_cntax_get_auth_passwd)
	{
		printf("load function so_cntax_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_get_auth_passwd = dlsym(handle, "function_mengbai_get_auth_passwd");
	if (NULL == so_mb_get_auth_passwd)
	{
		printf("load function so_mb_get_auth_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_aisino_verify_cert_passwd = dlsym(handle, "function_aisino_verify_cert_passwd");
	if (NULL == so_aisino_verify_cert_passwd)
	{
		printf("load function so_aisino_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_verify_cert_passwd = dlsym(handle, "function_nisec_verify_cert_passwd");
	if (NULL == so_nisec_verify_cert_passwd)
	{
		printf("load function so_nisec_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_verify_cert_passwd = dlsym(handle, "function_cntax_verify_cert_passwd");
	if (NULL == so_cntax_verify_cert_passwd)
	{
		printf("load function so_cntax_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_verify_cert_passwd = dlsym(handle, "function_mengbai_verify_cert_passwd");
	if (NULL == so_mb_verify_cert_passwd)
	{
		printf("load function so_mb_verify_cert_passwd error\n");
		dlclose(handle);
		return -4;
	}
	so_aisino_change_certpassword = dlsym(handle, "function_aisino_change_certpassword");
	if (NULL == so_aisino_change_certpassword)
	{
		printf("load function so_aisino_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_change_certpassword = dlsym(handle, "function_nisec_change_certpassword");
	if (NULL == so_nisec_change_certpassword)
	{
		printf("load function so_nisec_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_change_certpassword = dlsym(handle, "function_cntax_change_certpassword");
	if (NULL == so_cntax_change_certpassword)
	{
		printf("load function so_cntax_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_change_certpassword = dlsym(handle, "function_mengbai_change_certpassword");
	if (NULL == so_mb_change_certpassword)
	{
		printf("load function so_mb_change_certpassword error\n");
		dlclose(handle);
		return -4;
	}

	so_aisino_get_current_invcode = dlsym(handle, "function_aisino_get_current_invcode");
	if (NULL == so_aisino_get_current_invcode)
	{
		printf("load function so_aisino_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_nisec_get_current_invcode = dlsym(handle, "function_nisec_get_current_invcode");
	if (NULL == so_nisec_get_current_invcode)
	{
		printf("load function so_nisec_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_cntax_get_current_invcode = dlsym(handle, "function_cntax_get_current_invcode");
	if (NULL == so_cntax_get_current_invcode)
	{
		printf("load function so_cntax_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}
	so_mb_get_current_invcode = dlsym(handle, "function_mengbai_get_current_invcode");
	if (NULL == so_mb_get_current_invcode)
	{
		printf("load function so_mb_get_current_invcode error\n");
		dlclose(handle);
		return -4;
	}

	//======��Ҫ��Ȩ����֤������ȷ�󷽿ɲ����Ľӿ�======//
	so_aisino_copy_report_data = dlsym(handle, "function_aisino_copy_report_data");
	if (NULL == so_aisino_copy_report_data)
	{
		printf("load function so_aisino_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_copy_report_data = dlsym(handle, "function_nisec_copy_report_data");
	if (NULL == so_nisec_copy_report_data)
	{
		printf("load function so_nisec_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_copy_report_data = dlsym(handle, "function_cntax_copy_report_data");
	if (NULL == so_cntax_copy_report_data)
	{
		printf("load function so_cntax_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_copy_report_data = dlsym(handle, "function_mengbai_copy_report_data");
	if (NULL == so_mb_copy_report_data)
	{
		printf("load function so_mb_copy_report_data error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_report_summary_write_back = dlsym(handle, "function_aisino_report_summary_write_back");
	if (NULL == so_aisino_report_summary_write_back)
	{
		printf("load function so_aisino_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_report_summary_write_back = dlsym(handle, "function_nisec_report_summary_write_back");
	if (NULL == so_nisec_report_summary_write_back)
	{
		printf("load function so_nisec_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_report_summary_write_back = dlsym(handle, "function_cntax_report_summary_write_back");
	if (NULL == so_cntax_report_summary_write_back)
	{
		printf("load function so_cntax_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_report_summary_write_back = dlsym(handle, "function_mengbai_report_summary_write_back");
	if (NULL == so_mb_report_summary_write_back)
	{
		printf("load function so_mb_report_summary_write_back error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_inv_upload_server = dlsym(handle, "function_aisino_inv_upload_server");
	if (NULL == so_aisino_inv_upload_server)
	{
		printf("load function so_aisino_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_inv_upload_server = dlsym(handle, "function_nisec_inv_upload_server");
	if (NULL == so_nisec_inv_upload_server)
	{
		printf("load function so_nisec_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_inv_upload_server = dlsym(handle, "function_cntax_inv_upload_server");
	if (NULL == so_cntax_inv_upload_server)
	{
		printf("load function so_cntax_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_inv_upload_server = dlsym(handle, "function_mengbai_inv_upload_server");
	if (NULL == so_mb_inv_upload_server)
	{
		printf("load function so_mb_inv_upload_server error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_fpdm_fphm_update_invs = dlsym(handle, "function_aisino_fpdm_fphm_update_invs");
	if (NULL == so_aisino_fpdm_fphm_update_invs)
	{
		printf("load function so_aisino_fpdm_fphm_update_invs error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_fpdm_fphm_update_invs = dlsym(handle, "function_nisec_fpdm_fphm_update_invs");
	if (NULL == so_nisec_fpdm_fphm_update_invs)
	{
		printf("load function so_nisec_fpdm_fphm_update_invs error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_test_server_connect = dlsym(handle, "function_aisino_test_server_connect");
	if (NULL == so_aisino_test_server_connect)
	{
		printf("load function so_aisino_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_test_server_connect = dlsym(handle, "function_nisec_test_server_connect");
	if (NULL == so_nisec_test_server_connect)
	{
		printf("load function so_nisec_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_test_server_connect = dlsym(handle, "function_cntax_test_server_connect");
	if (NULL == so_cntax_test_server_connect)
	{
		printf("load function so_cntax_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_test_server_connect = dlsym(handle, "function_mengbai_test_server_connect");
	if (NULL == so_mb_test_server_connect)
	{
		printf("load function so_mb_test_server_connect error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_make_invoice = dlsym(handle, "function_aisino_make_invoice");
	if (NULL == so_aisino_make_invoice)
	{
		printf("load function so_aisino_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_make_invoice = dlsym(handle, "function_nisec_make_invoice");
	if (NULL == so_nisec_make_invoice)
	{
		printf("load function so_nisec_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_make_invoice = dlsym(handle, "function_cntax_make_invoice");
	if (NULL == so_cntax_make_invoice)
	{
		printf("load function so_cntax_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_make_invoice = dlsym(handle, "function_mengbai_make_invoice");
	if (NULL == so_mb_make_invoice)
	{
		printf("load function so_mb_make_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_waste_invoice = dlsym(handle, "function_aisino_waste_invoice");
	if (NULL == so_aisino_waste_invoice)
	{
		printf("load function so_aisino_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_waste_invoice = dlsym(handle, "function_nisec_waste_invoice");
	if (NULL == so_nisec_waste_invoice)
	{
		printf("load function so_nisec_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_waste_invoice = dlsym(handle, "function_cntax_waste_invoice");
	if (NULL == so_cntax_waste_invoice)
	{
		printf("load function so_cntax_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_waste_invoice = dlsym(handle, "function_mengbai_waste_invoice");
	if (NULL == so_mb_waste_invoice)
	{
		printf("load function so_mb_waste_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_waste_null_invoice = dlsym(handle, "function_aisino_waste_null_invoice");
	if (NULL == so_aisino_waste_null_invoice)
	{
		printf("load function so_aisino_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_waste_null_invoice = dlsym(handle, "function_nisec_waste_null_invoice");
	if (NULL == so_nisec_waste_null_invoice)
	{
		printf("load function so_nisec_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_waste_null_invoice = dlsym(handle, "function_cntax_waste_null_invoice");
	if (NULL == so_cntax_waste_null_invoice)
	{
		printf("load function so_cntax_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_mb_waste_null_invoice = dlsym(handle, "function_mengbai_waste_null_invoice");
	if (NULL == so_mb_waste_null_invoice)
	{
		printf("load function so_mb_waste_null_invoice error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_check_lzfpdm_lzfphm_allow = dlsym(handle, "function_aisino_check_lzfpdm_lzfphm_allow");
	if (NULL == so_aisino_check_lzfpdm_lzfphm_allow)
	{
		printf("load function so_aisino_check_lzfpdm_lzfphm_allow error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_upload_hzxxb = dlsym(handle, "function_aisino_upload_hzxxb");
	if (NULL == so_aisino_upload_hzxxb)
	{
		printf("load function so_aisino_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_upload_hzxxb = dlsym(handle, "function_nisec_upload_hzxxb");
	if (NULL == so_nisec_upload_hzxxb)
	{
		printf("load function so_nisec_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_upload_hzxxb = dlsym(handle, "function_cntax_upload_hzxxb");
	if (NULL == so_cntax_upload_hzxxb)
	{
		printf("load function so_cntax_upload_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_rednum= dlsym(handle, "function_aisino_download_hzxxb_from_rednum");
	if (NULL == so_aisino_download_hzxxb_from_rednum)
	{
		printf("load function so_aisino_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_download_hzxxb_from_rednum= dlsym(handle, "function_nisec_download_hzxxb_from_rednum");
	if (NULL == so_nisec_download_hzxxb_from_rednum)
	{
		printf("load function so_nisec_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_download_hzxxb_from_rednum= dlsym(handle, "function_cntax_download_hzxxb_from_rednum");
	if (NULL == so_cntax_download_hzxxb_from_rednum)
	{
		printf("load function so_cntax_download_hzxxb_from_rednum error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_date_range = dlsym(handle, "function_aisino_download_hzxxb_from_date_range");
	if (NULL == so_aisino_download_hzxxb_from_date_range)
	{
		printf("load function so_aisino_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_download_hzxxb_from_date_range_new = dlsym(handle, "function_aisino_download_hzxxb_from_date_range_new");
	if (NULL == so_aisino_download_hzxxb_from_date_range_new)
	{
		printf("load function so_aisino_download_hzxxb_from_date_range_new error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_download_hzxxb_from_date_range = dlsym(handle, "function_nisec_download_hzxxb_from_date_range");
	if (NULL == so_nisec_download_hzxxb_from_date_range)
	{
		printf("load function so_nisec_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_download_hzxxb_from_date_range = dlsym(handle, "function_cntax_download_hzxxb_from_date_range");
	if (NULL == so_cntax_download_hzxxb_from_date_range)
	{
		printf("load function so_cntax_download_hzxxb_from_date_range error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_cancel_hzxxb = dlsym(handle, "function_aisino_cancel_hzxxb");
	if (NULL == so_aisino_cancel_hzxxb)
	{
		printf("load function so_aisino_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_cancel_hzxxb = dlsym(handle, "function_nisec_cancel_hzxxb");
	if (NULL == so_nisec_cancel_hzxxb)
	{
		printf("load function so_nisec_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_cancel_hzxxb = dlsym(handle, "function_cntax_cancel_hzxxb");
	if (NULL == so_cntax_cancel_hzxxb)
	{
		printf("load function so_cntax_cancel_hzxxb error\n");
		dlclose(handle);
		return -5;
	}
	//so_aisino_get_download_net_invoice_coil = dlsym(handle, "function_aisino_get_download_net_invoice_coil");//�ӿ�ͣ��
	//if (NULL == so_aisino_get_download_net_invoice_coil)
	//{
	//	printf("load function so_aisino_get_download_net_invoice_coil error\n");
	//	dlclose(handle);
	//	return -5;
	//}
	so_aisino_query_net_invoice_coil = dlsym(handle, "function_aisino_query_net_invoice_coil");
	if (NULL == so_aisino_query_net_invoice_coil)
	{
		printf("load function so_aisino_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_query_net_invoice_coil = dlsym(handle, "function_nisec_query_net_invoice_coil");
	if (NULL == so_nisec_query_net_invoice_coil)
	{
		printf("load function so_nisec_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_query_net_invoice_coil = dlsym(handle, "function_cntax_query_net_invoice_coil");
	if (NULL == so_cntax_query_net_invoice_coil)
	{
		printf("load function so_cntax_query_net_invoice_coil error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_net_invoice_coil_download_unlock = dlsym(handle, "function_aisino_net_invoice_coil_download_unlock");
	if (NULL == so_aisino_net_invoice_coil_download_unlock)
	{
		printf("load function so_aisino_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_nisec_net_invoice_coil_download_unlock = dlsym(handle, "function_nisec_net_invoice_coil_download_unlock");
	if (NULL == so_nisec_net_invoice_coil_download_unlock)
	{
		printf("load function so_nisec_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_cntax_net_invoice_coil_download_unlock = dlsym(handle, "function_cntax_net_invoice_coil_download_unlock");
	if (NULL == so_cntax_net_invoice_coil_download_unlock)
	{
		printf("load function so_cntax_net_invoice_coil_download_unlock error\n");
		dlclose(handle);
		return -5;
	}
	so_aisino_connect_pubservice = dlsym(handle, "function_aisino_connect_pubservice");
	if (NULL == so_aisino_connect_pubservice)
	{
		printf("load function so_aisino_connect_pubservice error\n");
		dlclose(handle);
		return -5;
	}
	
	so_nisec_connect_pubservice = dlsym(handle, "function_nisec_connect_pubservice");
	if (NULL == so_nisec_connect_pubservice)
	{
		printf("load function so_nisec_connect_pubservice error\n");
		dlclose(handle);
		return -5;
	}


	so_aisino_disconnect_pubservice = dlsym(handle, "function_aisino_disconnect_pubservice");
	if (NULL == so_aisino_disconnect_pubservice)
	{
		printf("load function so_aisino_disconnect_pubservice error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_disconnect_pubservice = dlsym(handle, "function_nisec_disconnect_pubservice");
	if (NULL == so_nisec_disconnect_pubservice)
	{
		printf("load function so_nisec_disconnect_pubservice error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_client_hello = dlsym(handle, "function_aisino_client_hello");
	if (NULL == so_aisino_client_hello)
	{
		printf("load function so_aisino_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_client_hello = dlsym(handle, "function_nisec_client_hello");
	if (NULL == so_nisec_client_hello)
	{
		printf("load function so_nisec_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_client_hello = dlsym(handle, "function_cntax_client_hello");
	if (NULL == so_cntax_client_hello)
	{
		printf("load function so_cntax_client_hello error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_client_auth = dlsym(handle, "function_aisino_client_auth");
	if (NULL == so_aisino_client_auth)
	{
		printf("load function so_aisino_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_client_auth = dlsym(handle, "function_nisec_client_auth");
	if (NULL == so_nisec_client_auth)
	{
		printf("load function so_nisec_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_client_auth = dlsym(handle, "function_cntax_client_auth");
	if (NULL == so_cntax_client_auth)
	{
		printf("load function so_cntax_client_auth error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_get_cert = dlsym(handle, "function_aisino_get_cert");
	if (NULL == so_aisino_get_cert)
	{
		printf("load function so_aisino_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_get_cert = dlsym(handle, "function_nisec_get_cert");
	if (NULL == so_nisec_get_cert)
	{
		printf("load function so_nisec_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_get_cert = dlsym(handle, "function_cntax_get_cert");
	if (NULL == so_cntax_get_cert)
	{
		printf("load function so_cntax_get_cert error\n");
		dlclose(handle);
		return -5;
	}

	so_aisino_query_invoice_month_all_data = dlsym(handle, "function_aisino_query_invoice_month_all_data");
	if (NULL == so_aisino_query_invoice_month_all_data)
	{
		printf("load function so_aisino_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_query_invoice_month_all_data = dlsym(handle, "function_nisec_query_invoice_month_all_data");
	if (NULL == so_nisec_query_invoice_month_all_data)
	{
		printf("load function so_nisec_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_query_invoice_month_all_data = dlsym(handle, "function_cntax_query_invoice_month_all_data");
	if (NULL == so_cntax_query_invoice_month_all_data)
	{
		printf("load function so_cntax_query_invoice_month_all_data error\n");
		dlclose(handle);
		return -5;
	}

	so_nisec_read_inv_coil_number_history = dlsym(handle, "function_nisec_read_inv_coil_number_history");
	if (NULL == so_nisec_read_inv_coil_number_history)
	{
		printf("load function so_nisec_read_inv_coil_number_history error\n");
		dlclose(handle);
		return -5;
	}

	so_cntax_read_inv_coil_number_history = dlsym(handle, "function_cntax_read_inv_coil_number_history");
	if (NULL == so_cntax_read_inv_coil_number_history)
	{
		printf("load function so_cntax_read_inv_coil_number_history error\n");
		dlclose(handle);
		return -5;
	}

	so_mb_server_key_info = dlsym(handle, "function_mengbai_server_key_info");
	if (NULL == so_mb_server_key_info)
	{
		printf("load function so_mb_server_key_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_login_info = dlsym(handle, "function_common_etax_login_Interface");
	if (NULL == so_etax_login_info)
	{
		printf("load function so_etax_login_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_user_query_info = dlsym(handle, "function_common_etax_user_query_Interface");
	if (NULL == so_etax_user_query_info)
	{
		printf("load function so_etax_user_query_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_relationlist_info = dlsym(handle, "function_common_etax_relationlist_Interface");
	if (NULL == so_etax_relationlist_info)
	{
		printf("load function so_etax_relationlist_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_relationchange_info = dlsym(handle, "function_common_etax_relationchange_Interface");
	if (NULL == so_etax_relationchange_info)
	{
		printf("load function so_etax_relationchange_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_invoice_query_info = dlsym(handle, "function_common_etax_invoice_query_Interface");
	if (NULL == so_etax_invoice_query_info)
	{
		printf("load function so_etax_invoice_query_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_get_cookie_info = dlsym(handle, "function_common_etax_get_cookie_Interface");
	if (NULL == so_etax_get_cookie_info)
	{
		printf("load function so_etax_get_cookie_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_invoice_query_down_info = dlsym(handle, "function_common_etax_invoice_query_down_Interface");
	if (NULL == so_etax_invoice_query_down_info)
	{
		printf("load function so_etax_invoice_query_down_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_invoice_query_detail_info = dlsym(handle, "function_common_etax_invoice_query_detail_Interface");
	if (NULL == so_etax_invoice_query_detail_info)
	{
		printf("load function so_etax_invoice_query_detail_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_rpa_invoice_issuance_info = dlsym(handle, "function_common_etax_rpa_invoice_issuance_Interface");
	if (NULL == so_etax_rpa_invoice_issuance_info)
	{
		printf("load function so_etax_rpa_invoice_issuance_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_rpa_invoice_issuance_status_info = dlsym(handle, "function_common_etax_rpa_invoice_issuance_status_Interface");
	if (NULL == so_etax_rpa_invoice_issuance_status_info)
	{
		printf("load function so_etax_rpa_invoice_issuance_status_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_credit_limit_info = dlsym(handle, "function_common_credit_limit_Interface");
	if (NULL == so_etax_credit_limit_info)
	{
		printf("load function so_etax_credit_limit_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_blue_ink_invoice_issuance_info = dlsym(handle, "function_common_etax_blue_ink_invoice_issuance_Interface");
	if (NULL == so_etax_blue_ink_invoice_issuance_info)
	{
		printf("load function so_etax_blue_ink_invoice_issuance_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxSzzhQuery_info = dlsym(handle, "function_common_etax_hzqrxxSzzhQuery_Interface");
	if (NULL == so_etax_hzqrxxSzzhQuery_info)
	{
		printf("load function so_etax_hzqrxxSzzhQuery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxSave_info = dlsym(handle, "function_common_etax_hzqrxxSave_Interface");
	if (NULL == so_etax_hzqrxxSave_info)
	{
		printf("load function so_etax_hzqrxxSave_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxQuery_info = dlsym(handle, "function_common_etax_hzqrxxQuery_Interface");
	if (NULL == so_etax_hzqrxxQuery_info)
	{
		printf("load function so_etax_hzqrxxQuery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hzqrxxQueryDetail_info = dlsym(handle, "function_common_etax_hzqrxxQueryDetail_Interface");
	if (NULL == so_etax_hzqrxxQueryDetail_info)
	{
		printf("load function so_etax_hzqrxxQueryDetail_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_hcstaquery_info = dlsym(handle, "function_common_hcstaquery_invoice_issuance_Interface");
	if (NULL == so_etax_hcstaquery_info)
	{
		printf("load function so_etax_hcstaquery_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_fjxxpeizhi_info = dlsym(handle, "function_common_fjxxpeizhi_Interface");
	if (NULL == so_etax_fjxxpeizhi_info)
	{
		printf("load function so_etax_fjxxpeizhi_info error\n");
		dlclose(handle);
		return -5;
	}

	so_etax_cjmbpeizhi_info = dlsym(handle, "function_common_cjmbpeizhi_Interface");
	if (NULL == so_etax_cjmbpeizhi_info)
	{
		printf("load function so_etax_cjmbpeizhi_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_get_nsrjcxx_info = dlsym(handle, "function_common_etax_get_nsrjcxx_Interface");
	if (NULL == so_etax_get_nsrjcxx_info)
	{
		printf("load function so_etax_get_nsrjcxx_info error\n");
		dlclose(handle);
		return -5;
	}


	so_etax_get_nsrfxxx_info = dlsym(handle, "function_common_etax_get_nsrfxxx_Interface");
	if (NULL == so_etax_get_nsrfxxx_info)
	{
		printf("load function so_etax_get_nsrfxxx_info error\n");
		dlclose(handle);
		return -5;
	}

	#ifdef MTK_OPENWRT
		system("cp /tmp/libmb_taxlib.so /tmp/libmb_taxlib_back.so");
	#else
		system("cp /lib/libmb_taxlib.so /lib/libmb_taxlib_back.so");
	#endif

#ifndef CESHI
	#ifdef MTK_OPENWRT
		system("rm /tmp/libmb_taxlib.so");
	#else
		system("rm /lib/libmb_taxlib.so");
	#endif	
#endif
	return 0;
}

#endif


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
			//out("[ty_usb_m.c]���˿ںŴ򿪶˿� port = %d\n",i);
			//ǰ��ģ���Ѿ��ر������ظ��ص�
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, i);
			state->power = 1;
			if (result<0)
			{
				logout(INFO, "SYSTEM", "INIT", "֪ͨ��Ƭ���򿪶˿ڵ�Դʧ��,�˿ں�%d\r\n",i);
				err_port += 1;
				//out("task_open:�򿪶˿�ʧ��,�˿ں�%d\n", i);
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


int check_port_auth_info(struct _ty_usb_m  *stream)
{
	struct _port_state   *state;
	char busid[32] = {0};
	int aisino_sup;
	int nisec_sup;
	char start_time[20] = {0};//��ʼʱ��
	char end_time[20] = {0};//����ʱ��
	int size;
	int port;
	int i;
	char num[10] = {0};
	cJSON *arrayItem, *item;
	if (stream->cert_json == NULL)
	{
		out("��Ȩ�ļ�Ϊ��\n");
		return -1;
	}

	//out("��Ȩ�ļ�����:%s\n", stream->cert_json);

	cJSON *root = cJSON_Parse(stream->cert_json);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "version");
	if (get_json_value_can_not_null(item, tax_auth.version, 0, 128) < 0) {
		out("������Ȩ�ļ��汾��ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "serial_num");
	if (get_json_value_can_not_null(item, tax_auth.serial_num, 0, 128) < 0) {
		out("������Ȩ�ļ����к�ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "name");
	if (get_json_value_can_not_null(item, tax_auth.name, 0, 128) < 0) {
		out("������Ȩ�ļ�����ʧ��\n");
		goto End;
	}

	cJSON *object_tax = cJSON_GetObjectItem(root, "tax");
	if (object_tax == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_tax);
	if (size < 1)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	for (i = 0; i < size; i++)
	{
		//int num_i;
		arrayItem = cJSON_GetArrayItem(object_tax, i);
		if (arrayItem == NULL)
		{
			out("Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		memset(num, 0, sizeof(num));
		item = cJSON_GetObjectItem(arrayItem, "port_num");
		if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
			out("����USB�˿ں�ʧ��,i=%d\n", i);
			goto End;
		}
		port = atoi(num);
		state = &stream->port[port - 1];

		item = cJSON_GetObjectItem(arrayItem, "busid");
		if (get_json_value_can_not_null(item, busid, 0, 32) < 0) {
			out("����busidʧ��,i=%d\n", i);
			goto End;
		}

		if (strcmp(tax_auth.version,"V1.1.01") <= 0)
		{
			memset(num, 0, sizeof(num));
			item = cJSON_GetObjectItem(arrayItem, "aisino_sup");
			if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
				out("����������֧��ʧ��,i=%d\n", i);
				goto End;
			}
			aisino_sup = atoi(num);
			state->aisino_sup = aisino_sup;
#ifdef UKEY_AUTH
			state->cntax_sup = 1;
#else
			state->cntax_sup = aisino_sup;
#endif				
			state->mengbai_sup = 1;

			memset(num, 0, sizeof(num));
			item = cJSON_GetObjectItem(arrayItem, "nesic_sup");
			if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
				out("����������֧��ʧ��,i=%d\n", i);
				goto End;
			}
			nisec_sup = atoi(num);
			state->nisec_sup = nisec_sup;

			item = cJSON_GetObjectItem(arrayItem, "start_time");
			if (get_json_value_can_not_null(item, start_time, 0, 32) < 0) {
				out("������ʼʱ��ʧ��,i=%d\n", i);
				goto End;
			}
			memset(state->start_time, 0, sizeof(state->start_time));
			strcpy(state->start_time, start_time);

			item = cJSON_GetObjectItem(arrayItem, "end_time");
			if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
				out("������ʼʱ��ʧ��,i=%d\n", i);
				goto End;
			}
			memset(state->end_time, 0, sizeof(state->end_time));
			strcpy(state->end_time, end_time);

			strcpy(state->aisino_end_time, state->end_time);
			strcpy(state->nisec_end_time, state->end_time);
			strcpy(state->cntax_end_time, state->end_time);
			strcpy(state->mengbai_end_time, state->end_time);

			item = cJSON_GetObjectItem(arrayItem, "frozen_ago_time");
			memset(state->frozen_ago_time,0,20);
			get_json_value_can_not_null(item, state->frozen_ago_time, 0, 32);
		}
		else
		{
			char *p1,*p2;
			char auth_str[10] = {0};
			char tmp_str[100] = {0};


			item = cJSON_GetObjectItem(arrayItem, "start_time");
			if (get_json_value_can_not_null(item, start_time, 0, 32) < 0) {
				out("������ʼʱ��ʧ��,i=%d\n", i);
				continue;
			}
			memset(state->start_time, 0, sizeof(state->start_time));
			strcpy(state->start_time, start_time);

			item = cJSON_GetObjectItem(arrayItem, "end_time");
			if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
				out("������ֹʱ��ʧ��,i=%d\n", i);
				continue;
			}
			memset(state->end_time, 0, sizeof(state->end_time));
			strcpy(state->end_time, end_time);

			item = cJSON_GetObjectItem(arrayItem, "frozen_ago_time");
			memset(state->frozen_ago_time,0,20);
			if (get_json_value_can_not_null(item, state->frozen_ago_time, 0, 32) < 0) {
				out("��������ʱ��ʧ��,i=%d\n", i);
				continue;
			}
			//��˰��
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "aisino_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("������˰����Ȩ��Ϣʧ��,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			state->aisino_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(state->aisino_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//˰����
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "nesic_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("������˰����Ȩ��Ϣʧ��,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			state->nisec_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(state->nisec_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//˰��ukey
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "cntax_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("����˰��ukey��Ȩ��Ϣʧ��,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			state->cntax_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(state->cntax_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

			//�ɰ�
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "mengbai_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("����ģ������Ȩ��Ϣʧ��,i=%d\n", i);
				continue;
			}
			p1 = strstr(tmp_str, ",");
			if (p1 == NULL)
				continue;
			p2 += strlen(",");
			memset(auth_str, 0, sizeof(auth_str));
			memcpy(auth_str, tmp_str, p1 - tmp_str);
			state->mengbai_sup = atoi(auth_str);

			p1 = strstr(p2, ",");
			if (p1 == NULL)
				continue;
			memcpy(state->mengbai_end_time, p2, p1 - p2);
			p1 = NULL;
			p2 = NULL;

		}
	}
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
	�쳣�����˿��µ�
	�µ���Ͻ��˿���Ϊ���µ����豸״̬
*/
static void port_power_down(struct _port_state   *state)//
{
	int result;
	struct _ty_usb_m *stream;	
	stream = ty_usb_m_id;
	if (state->power == 0)
	{
		return;
	}
	state->need_down_power = 1;
	//out("port_power_down ִ�е�Ƭ���ر�%d�Ŷ˿ڵ�Դ\n",state->port);
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);//ִ���µ����
	if (result < 0)
	{
		logout(INFO, "DEV", "PORT_POWER", "port_power_down:��Ƭ��ִ���µ�ʧ��%d\n", state->port);
		usleep(20000);
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);
	}
	state->need_down_power = 0;
	if (state->need_up_power == 1)
	{
		//logout(INFO, "DEV", "PORT_POWER", "%d�Ŷ˿�ִ���µ����ʱ���֣�ϵͳ����߳�����ִ���ϵ磬��ֹ��ͻ��������ϵ�״̬\n", state->port);
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

int port_power_up(struct _port_state   *state)
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
		out("�˿ں�����\n");
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
			out("%d�Ŷ˿�δ��ɳ�ʼ���ݲ�����ʹ��,state->port_init == %d,usedinfo = %s\n", port, state->port_init,state->port_info.used_info);
			return POWER_ERR_NO_DEV;
		}
	}

	get_usb_busid(port,busid);
	if ((state->power == 1) && (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port)==0))
	{
		busnum = get_busid_busnum(busid);
		devnum = get_busid_devnum(busid);
		if ((state->h_plate.nBusID != busnum) || (state->h_plate.nDevID != devnum))
		{
			out("��⵽%d�Ŷ˿��Ѿ��ϵ�,��USBbus��Ϣ�������⣬�������µ��ϵ�\n",port);
			state->sys_reload = 1;//�ر��豸������Ϣ
			port_power_down(state);
		}
		else
		{
			//out("��⵽%d�Ŷ˿��Ѿ��ϵ�,ֱ�Ӽ���ʹ��\n",port);
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
		out("��%d�˿ں��豸�ȴ�ö�ٳ�ʱ\n",port);
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
	//out("��%d�˿ں�busnum = %04x devnum = %04x\n", port,state->h_plate.nBusID, state->h_plate.nDevID);
	state->h_plate.usb_type = state->port_info.usb_app;
	state->h_plate.need_check_passwd = 0;
	struct _plate_infos plate_infos;
	memset(&plate_infos, 0, sizeof(plate_infos));
	memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
	strcpy(state->h_plate.busid, state->port_info.busid);

#ifdef RELEASE_SO
	result = so_common_get_basic_simple(&state->h_plate, &plate_infos);
#else
	result = function_common_get_basic_simple(&state->h_plate, &plate_infos);
#endif

	if (result < 0)
	{
		logout(INFO, "DEV", "�ϵ�", "�򿪶˿ھ��ʧ��, �˿ں�%d ,result:%d,use info:%s\n", port, result, state->port_info.used_info);
		state->sys_reload = 0;//�ر��豸��������Ϣ
		port_power_down(state);
		return POWER_ERR_NO_DEV;

	}
	return 0;
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

	//wang �޸Ķ�ȡAD�߼�
	if((stream->all_port_updata & 0x01) == 0)
		return;
	else 
		stream->all_port_updata = stream->all_port_updata & 0x06;

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

		//out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
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
						if(state->port_info.ad_status > 2000)			//wang 02 23 ������
							state->status_counts += 1;
						else{
							printf("status_counts port=%d\r\n",state->port);
						}
						continue;
					}
					out("%d�Ŷ˿ڼ�⵽�ص�״̬�豸���γ�\n", state->port);
					if(state->port_info.port_used != 1)
					{
						state->status = 0;
						state->sys_reload = 0;
						port_power_down(state);
						state->port_init = 0;
					}
					state->status_counts = 0;
				}
			}
			state->status = 0;			
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

	//wang �޸Ķ�ȡAD�߼�
	if((stream->all_port_updata & 0x02) == 0)
		return;
	else 
		stream->all_port_updata = stream->all_port_updata & 0x05;

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

		//out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//����ģʽ���������µ�˿ڼ��
		{
			continue;
		}
		
		if ((now_status == 0)&&(state->port_info.ad_status > 2000)) //wang 02 24 ������
		{
			long time_now = get_time_sec();
			if (time_now < 1656671934)//ʱ��δ����
			{
				continue;
			}
			if (state->ad_repair_down_time == 0)
			{
				state->ad_repair_down_time = time_now - 10;//��ֹϵͳ���µ粿���豸��Ƭ����ⲻ�����ٴ��ϵ���������
				continue;
			}
			if ((state->status != 0)) //�ȴ������̴߳�����
			{
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
			if (state->power == 1)
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

	//wang �޸Ķ�ȡAD�߼� ��֤����ADֵ����ִ���ж�
	if((stream->all_port_updata & 0x04) == 0)
		return;
	else 
		stream->all_port_updata = stream->all_port_updata & 0x03;

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

		//out("�˿ںţ�%d,״̬��%d\n", down_port, now_status);
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
			//out("��Ƭ����֪%d�Ŷ˿����豸,��ǰ�豸����Ϊ%d,�ϵ�˿�����Ϊ%d\n", down_port, stream->onlineport, get_powered_port_num());
			if (state->ad_repair_check == 1) //���豸�˿����ϵ��������������豸�����⵽�ˣ�������ֹͣAD�޸����
			{
				state->ad_repair_check = 0;
				state->ad_repair_up_time = 0;
				state->ad_repair_down_time = get_time_sec();
				state->power_mode = Saving_Mode;
			}
			if (stream->onlineport > DF_FULL_LOAD_NUM)
			{
				out("��%d�Ŷ˿��ϵ磬����֧����������豸����\n", down_port);
				for (;;)
				{
					usleep(10000);
					if (stream->onlineport < DF_FULL_LOAD_NUM)
						break;
				}
			}
			if (get_powered_port_num() > DF_FULL_POWER_NUM)
			{
				out("��%d�Ŷ˿��ϵ磬����֧������ϵ�˿�����\n", down_port);
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
				//out("��%d�Ŷ˿��ϵ磬��ǰ�˿�Ϊ�ϵ�״̬\n", down_port);
				continue;
			}
			if ((state->power == 0) && (state->h_plate.need_free == 1))
			{
				//�˿�δ��ȫ�ͷ���ɣ��ȴ�
				out("��%d�Ŷ˿��ϵ磬��ǰ�˿�Ϊ�µ�״̬�Ҿ��δ���\n", down_port);
				continue;
			}
			//out("��%d�Ŷ˿��ϵ磬port_init = %d, need_up_power = %d\n", down_port, state->port_init, state->need_up_power);
			if ((state->port_init == 0) && (state->need_up_power != 1) && (state->need_down_power!=1))
			{
				//out("��Ƭ����֪%d�Ŷ˿����豸,��ǰ�豸����Ϊ%d,�ϵ�˿�����Ϊ%d\n", down_port, stream->onlineport, get_powered_port_num());
				state->need_up_power = 1;//��ֹ���µ��̳߳�ͻ
				//out("%d�Ŷ˿����豸��Ҫ��ȡ����˰��\n", down_port);
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
			out("��Ƭ����֪%d�Ŷ˿ڶ�·\n", down_port);
			continue;
		}
	}
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
	//out("adֵ\n");
	//printf_array(ad_status,600);
	if (result != 2 * stream->usb_port_numb)
	{
		out("��ȡ����ad���س��ȴ���:result =%d\n", result);
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
		out("DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS err!\n");
		return;
	}
	r_len = result;


	_lock_set(stream->lock);
	memset(stream->ad_status, 0, sizeof(stream->ad_status));
	memcpy(stream->ad_status, ad_status, sizeof(ad_status));

	memset(stream->all_port_status, 0, sizeof(stream->all_port_status));
	memcpy(stream->all_port_status, all_port_status, sizeof(all_port_status));

	stream->all_port_status_len = r_len;
	stream->all_port_updata = 0x07;						//wang 02 23
	_lock_un(stream->lock);

	//down_port_num = all_port_status[0] * 256 + all_port_status[1];
	//out("״ֵ̬\n");
	//printf_array(all_port_status,302);//100ֻ��302��Ч

	
	for (i = 0; i < stream->usb_port_numb; i++)
	{
		state = &stream->port[i];
		if (state->ad_repair_check == 0)
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
				//out("%d�Ŷ˿��ϵ糬ʱδö�ٹرն˿ڵ�Դ\n", i+1);
				port_power_down(state);
			}
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
		//out("��������δ����,�Ժ������ϱ��ն�״̬\n");
		return;
	}

	//out("���������������ɹ��ϵ�51��Ƭ�������账��\n");
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
		//out("��⵽��%d�Ŷ˿�û���豸,����Ϊ51�汾���쳣����,�ж϶ϵ�ʱ���Ƿ񳬹�3��\n", port);
		if (state->last_without_time == 0)
		{
			state->last_without_time = get_time_sec();
			continue;
		}
		//out("��ǰʱ�䣺%ld,�ϴ�����ʱ�䣺%ld\n", now_time , state->last_without_time);
		if (now_time - state->last_without_time > 3)
		{

			//out("��⵽��%d�Ŷ˿�û���豸,����Ϊ51�汾���쳣����,�ϵ�ʱ������3��,���¸��˿��ϵ�\n", port);
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
		out("�رվ���ͷ��ڴ�,˰�����豸,�˿ں�:%d\n", state->port);
		//usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, 0);
		if (h_plate->hUSB != NULL)
		{
			if (h_plate->hUSB->handle)
			{
				//printf("if (h_plate->hUSB->handle)\r\n");
				libusb_close(h_plate->hUSB->handle);
				h_plate->hUSB->handle = NULL;		//wang 02 07 ���Խ��������������
			}
			//printf("common_close_usb_device h_plate->hUSB->context = %x\r\n",h_plate->hUSB->context);		


			libusb_device **device_list = NULL;
			int device_num = libusb_get_device_list(h_plate->hUSB->context, &device_list);
			/*if (device_num <= 0) {
				libusb_free_device_list(device_list, 1);
				fprintf(stderr, "[Device] No device found on the bus.\n");
				return -1;
			}*/
			printf("device_num=%d\r\n",device_num);

			libusb_exit(h_plate->hUSB->context);	
			h_plate->hUSB->context = NULL;			//wang 02 07 ���Խ��������������

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
	else if(usb_type != DEVICE_MENGBAI2)
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
	//out("�رվ���ͷ��ڴ����,�˿ں�:%d\n", state->port);
	return 0;
}

/*
static int common_close_usb_device(struct _port_state   *state,struct HandlePlate *h_plate)
{

	int usb_type = h_plate->usb_type;

	if ((usb_type == DEVICE_MENGBAI)||(usb_type == DEVICE_NISEC) || (usb_type == DEVICE_AISINO) || (usb_type == DEVICE_CNTAX) || (usb_type == DEVICE_SKP_KEY))
	{
		//out("�رվ���ͷ��ڴ�,˰�����豸,�˿ں�:%d\n", state->port);
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
	else if(usb_type != DEVICE_MENGBAI2)
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
	//out("�رվ���ͷ��ڴ����,�˿ں�:%d\n", state->port);
	return 0;
}*/
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
	{	//out("����%d�Ŷ˿�\n",i);

		state = &stream->port[port - 1];
		if (state->port != port)
			state->port = port;
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
		if (result<0)
		{
			if (state->sys_reload == 1)
			{ //����ڵ�
				if (state->h_plate.need_free == 1)
				{
					//out("%d�Ŷ˿ڽ���ڵ�\n",port);
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
						state->port_info.inv_query = 0;
						continue;
					}
					//out("%d�Ŷ˿ڽ���ڵ��ͷž��\n", port);
					free_h_plate(state);
				}
				continue;
			}
			if (state->port_info.with_dev == 1) //֮ǰ���豸����û����
			{
				//out("�µ������豸:%d---�Ƿ��Ѿ��ϵ�:%d\n", state->status, state->power);
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
						state->port_info.inv_query = 0;
						continue;
					}
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�����رվ��\n", state->port_info.port_str, port);
					free_h_plate(state);
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,�رվ���ɹ�\n", state->port_info.port_str, port);
				}
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���ڼ�⵽�豸���γ���,��ǰʹ����ϢΪ��%s,����ڴ�\r\n", state->port_info.port_str, port, state->port_info.used_info);
				memset(&state->port_last_info, 0, sizeof(struct _port_last_info));
				state->port_last_info.usb_app = state->port_info.usb_app;
				memcpy(state->port_last_info.ca_name, state->port_info.ca_name, sizeof(state->port_info.ca_name));
				memcpy(state->port_last_info.ca_serial, state->port_info.ca_serial, sizeof(state->port_info.ca_serial));
				memcpy(state->port_last_info.sn, state->port_info.sn, sizeof(state->port_info.sn));
				state->port_last_info.extension = state->port_info.extension;
				state->port_last_info.vid = state->port_info.vid;
				state->port_last_info.pid = state->port_info.pid;
				state->off_report = 1;
				state->last_act = 2;
				state->sys_reload = 0;
				state->port_init = 0;
				state->ad_repair_check = 0;
				state->ad_repair_down_time = 0;
				state->ad_repair_up_time = 0;				
				state->need_up_power = 0;
				state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
				state->status_counts = 0;

				if (stream->dev_support.support_power == 1)
				{
					state->power_mode = Saving_Mode;
				}

				int ad_status = 0;

				if (state->port_info.ad_status != 0)
				{
					_lock_set(state->lock);
					ad_status = state->port_info.ad_status;
					_lock_un(state->lock);
				}
				_lock_set(state->lock);
#ifndef DF_OLD_MSERVER
				if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
				{
					free(state->port_info.upload_inv_num.dmhmzfsbstr);
					state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
				}
#endif				
				memset(&state->port_info, 0, sizeof(state->port_info));
				state->port_info.ad_status = ad_status;
				_lock_un(state->lock);
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,ԭ��%d�˿����豸,���port_info\n", state->port_info.port_str, port);

				if ((state->power_mode == Saving_Mode) && (state->power == 1))//����ǽڵ�棬�ڶ˿�ͨ�����豸���γ�������˿��µ�
				{
					state->sys_reload = 0;
					//out("%d�Ŷ˿ڿ��ŵ絫���豸����\n",state->port);
					port_power_down(state);
				}
			}
			continue;
		}//��⵽û���豸����
		onlineport += 1;//����ͨ���豸������1
		//����Ϊ�ļ�ϵͳ��⵽���豸
		state->port_info.with_dev = 1;

		if (state->port_init == 0 && state->h_plate.need_free != 1)//�ж��豸�Ƿ������仯,0�仯 1δ��,��ȡ���˿��豸��Ϣ�󽫴�״̬��Ϊ1,������ز��ٽ���˲���  
		{
			//out("%d�Ŷ˿��豸�����ϵ�,���ȡ������Ϣ\n", port);
			memset(state->port_info.busid, 0, sizeof(state->port_info.busid));
			//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, state->port_info.busid);
			result = get_busid_info(port, state->port_info.busid);
			if (result < 0)
			{
				out("%d�Ŷ˿ڣ���ȡ�豸busid��Ϣʧ��\n", state->port);
				//state->port_info.with_dev = 0;
				continue;
			}
			memset(&dev, 0, sizeof(dev));
			result = _usb_file_get_infor(state->port_info.busid, &dev);
			if (result < 0)
			{
				out("%d�Ŷ˿ڣ���ȡ�豸dev��Ϣʧ��\n",state->port);
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
				state->port_init = 1;
				state->port_info.tax_read = 1;
			}
			else
			{	
#ifdef ALL_SHARE
				state->port_init = 1;
				state->port_info.ca_read = 1;
#else
				if ((state->power_mode == Saving_Mode) && (state->power == 1) && (state->port_info.port_used == 0))//����ǽڵ�棬�ڶ˿�ͨ�����豸���γ�������˿��µ�
				{
					state->port_init = 0;
					state->sys_reload = 1;
					state->need_up_power = 0;//�����ϵ��У������ظ�����
					//out("%d�Ŷ˿��ǽڵ��˿ڲ�ʶ���˰�����豸����ڵ繦��",state->port);
					port_power_down(state);
				}
#endif
			}
			//out("��%d�ſڡ���⵽���豸����\n", port);
		}
	}

	stream->onlineport = onlineport;
}



#ifdef ALL_SHARE

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
	{   out("Error before: [%s]\n",cJSON_GetErrorPtr());  
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
	{	//out("��ȡЭ��汾\n");
		item=cJSON_GetObjectItem(object,key);  
		if(item!=NULL)  
		{	//out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);
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
	{   //out("read data by read i = %d\n",i);
		result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{	//out("read data time out or err \n\n\n\n");
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
	//out("��ȡ�汾��Ϣ\n");
	memset(edit, 0, sizeof(edit));
	memset(len_buf, 0, sizeof(len_buf));
	memset(order, 0, sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
	if (ty_socket_read(fd, edit, sizeof(edit))<0)
	{
		out("��ȡ�汾��Ϣ����\n");
		return -1;
	}
	if ((edit[0] != 0x01) && (edit[1] != 0x10))
	{
		out("��ȡ���İ汾��Ϣ����%02x %02x\n", edit[0], edit[1]);
		return -2;
	}
	//out("�汾��Ϣ�ɹ� %02x %02x\n",edit[0],edit[1]);
	//out("��ȡ������Ϣ\n");
	if (ty_socket_read(fd, len_buf, sizeof(len_buf))<0)
	{
		out("������Ϣ��ȡʧ��\n");
		return -1;
	}
	buf_len = len_buf[0] * 256 + len_buf[1];
	//out("��֡����:%d\n",buf_len);
	if (buf_len<6)
	{
		out("����С��6����\n");
		return -3;
	}
	if (buf_len - 6>(*out_len))
	{
		out("���ȹ�����%d\n", buf_len - 6);
		return -4;
	}
	if (ty_socket_read(fd, order, sizeof(order))<0)
	{
		out("��ȡ������ʧ��\n");
		return -1;
	}
	order_c = order[0] * 256 + order[1];
	if (order_c != cmd)
	{	//out("�����ֲ�һ�»�Ϊ�������\n");
		return -1;
	}
	if (ty_socket_read(fd, out_buf, buf_len - 6)<0)
	{
		out("��ȡ����ʧ��\n");
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
		out("�Ҳ���ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
		return CA_READ_ERR_NOT_FOUND_SO;
	}
	out("�ҵ�ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
	inbuf[0] = 0x01; inbuf[1] = 0x10;
	inbuf[2] = 0x00; inbuf[3] = 0x26;
	inbuf[4] = 0x05; inbuf[5] = 0x01;
	memcpy(inbuf + 6, state->port_info.busid, sizeof(state->port_info.busid));

	socketfd = socket_tcp_open_send_recv_timeout("127.0.0.1", 10003, inbuf, 38);
	if (socketfd < 0)
	{
		//out("��������������ʧ��\n");
		//out("read_ukey_ca_name_1 �ڴ˴�����CA�����Ƿ�����\n");
		//stream->execute_pro = 1;	 //�����Ҫ������CA����
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -1;
	}
	out_len = sizeof(outbuf);
	//out("������ɣ��ȴ�����\n");
	result = unpack_ca_data(socketfd, outbuf, &out_len, 0x1501);
	if (result < 0)
	{
		//out("��ȡ����ʧ�ܻ�ʱ\n");
		close(socketfd);
		//out("�ڴ˴�����CA�����Ƿ�����\n");
		return -2;
	}
	//out("������CA������Ϣ����Ϊ%d,����Ϊ%s\n",out_len,outbuf);
	close(socketfd);
	memcpy(name_tmp, outbuf, out_len);
	memset(key, 0, sizeof(key));
	sprintf(key, "%04x%04x", state->port_info.vid, state->port_info.pid);
	memset(json_data, 0, sizeof(json_data));
	base_3des_data((uint8 *)name_tmp, (uint8 *)json_data, strlen(name_tmp), (unsigned char *)key);
	//out("MAIN ���ܺ������:\n%s\n",json_data);

	_lock_set(state->lock);
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "result", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��1\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -3;
	}
	out("result = %d\n", valueInt);
	state->port_info.ca_ok = valueInt;
	result = analysis_json_data(json_data, "encording", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��2\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -4;
	}
	out("encording = %d\n", valueInt);
	state->port_info.encording = valueInt;
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "cert_num", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��3\n");
		state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -5;
	}
	out("cert_num = %s\n", valueString);

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
		out("utf8����\n");
		memset(valueString, 0, sizeof(valueString));
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		//out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		utf82gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	else if (state->port_info.encording == 0)
	{
		out("unicode����\n");
		memset(valueString, 0, sizeof(valueString));
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		//out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		unicode2gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	memcpy(state->port_info.ca_name, name_tmp1, strlen(name_tmp1));
	//out("ca_name = %s\n",name_tmp1);
	out("ca_name = %s\n", state->port_info.ca_name);

	//result = analysis_json_data(json_data,"ukey_type",&valueInt,valueString);
	//if(result <0||valueInt==0)
	//{	out("��ȡukey����ʧ��\n");
	//	state->port_info.plate_type = 0;
	//}
	//else
	//{	out("ukey_type = %d\n",valueInt);
	//	state->port_info.plate_type = valueInt;
	//}

	result = analysis_json_data(json_data, "extension", &valueInt, valueString);
	if (result <0)
	{
		out("��ȡ�����̺�ʧ��\n");
		state->port_info.extension = 0;
	}
	else
	{
		out("extension = %d\n", valueInt);
		state->port_info.extension = valueInt;
	}

	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "SN", &valueInt, valueString);
	if (result <0)
	{
		out("��ȡ˰�̱��ʧ��\n");
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
	}
	else
	{
		out("SN = %s\n", valueString);
		memcpy(state->port_info.sn, valueString, strlen(valueString));
	}
	_lock_un(state->lock);
	return 0;
}

static int judge_dev_down_power(struct _port_state   *state)
{
	if (state->port_info.vid == 0x1a56 && state->port_info.pid == 0xdd01)
	{
		return 0;
	}
	return -1;

}
#endif

static void task_read_port_base_tax_info(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int usb_type;
	stream = arg;
	char time_now[20] = {0};

	if (stream->frist_start == 0)
	{
		//out("�豸������,��ʱ5����ٶ�ȡ֤������\n");
		stream->frist_start = 1;
		sleep(5);
		return;
	}
	if (stream->frist_start == 1)
	{
		memset(time_now, 0, sizeof(time_now));
		timer_read_y_m_d_h_m_s(time_now);
		if (strcmp(time_now, SYSTEM_LEAST_TIME) < 0)
		{
			sleep(1);
			//out("�ȴ���ʱ���\n");
			return;
		}
		stream->frist_start = 2;
		return;
	}
	//out("task_check_port ���� %d\n",stream->usb_port_numb);
	for (i = 1; i <= stream->usb_port_numb; i++)
	{	//out("����%d�Ŷ˿�\n",i);
		state = &stream->port[i - 1];



		if ((state->port_info.with_dev == 0) || (state->retry_forbidden))
		{
			continue;
		}
		if (state->in_read_base_tax != 0)
		{
			//out("˰���ϴ�ʹ����\n");
			continue;
		}
		if (state->port_info.tax_read == 1)
		{
						
			if (state->power_mode == Saving_Mode)
			{
				if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port) < 0)
				{
					//out("%d�˿ڶ������Ϣǰ���γ���!\n", state->port);
					logout(INFO, "SYSTEM", "CHECK_PORT", "��%d�ſڡ���ȡ�����Ϣǰ���γ���2\r\n", state->port);
					state->port_info.tax_read = 0;
					state->sys_reload = 0;
					port_power_down(state);
					continue;

				}
			}

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
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
		


			//out("%d�Ŷ˿ڷ�Ʊ�ϴ��ж�\n", state->port);
			
			if (stream->base_read_queue_num > 4)
			{
				continue;
			}
			if (state->port_info.used_level == 1)
			{
				continue;
			}

			stream->base_read_queue_num += 1;
			state->in_read_base_tax = 1;
			//out("%d�Ŷ˿���Ҫִ�л�����Ϣ��ȡ����\n",state->port);
			//logout(INFO, "system", "������Ϣ��ȡ", "%d�Ŷ˿���Ҫ��ȡ������Ϣ,���������̶߳���\r\n", i);
			_queue_add("up_m_server", deal_read_plate_base_tax_info, state, 10);
			continue;
		}
#ifdef ALL_SHARE
		else if (state->port_info.ca_read == 1)
		{
			out("%d�˿���Ҫ��ȡCA��Ϣ!\n", state->port);

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if (usb_type !=0)
			{
				state->port_info.ca_read = 0;
				continue;
			}


			if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)		
			{
				out("�˿�%d,��ʹ����,�޷���ȡ֤������\n",i);
				continue;
			}
			struct _careader_process careader_process;
			memset(&careader_process, 0, sizeof(struct _careader_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			//��ȡ��˰�̵���˰�����ƺ�˰��
			if (atoi(careader_process.process_run) != 1)//CA����δ����
			{
				sleep(8); //out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
				out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
			}
			else//�����Ѿ�����
			{
				if (state->port_info.vid == 0x096e && state->port_info.pid == 0x0702)
				{
					if (access("/etc/ca_read", 0) != 0)//����Ҫ��ȡ��ca����˰��
					{
						state->port_info.usb_app = 5;
						state->fail_num = 30;
						state->port_info.ca_read = 0;
						sleep(1);
						out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", i);
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						goto END;
					}
				}
				int result = read_ukey_ca_name_1(state);
				if (result < 0)
				{
					//out("CA��ȡ֤��ʧ��--�˿ں�:[%d] fail_num:%d\n", i,state->fail_num);
					state->port_info.usb_app = 5;
					state->fail_num += 1;
					if (result == CA_READ_ERR_NOT_FOUND_SO)
					{

						state->fail_num = 30;
						state->port_info.ca_read = 0;
					}
					sleep(1);
					out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", i);
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					goto END;
				}
				else
				{
					if (judge_dev_down_power(state) == 0)
					{
						out("���ӿڰ��豸���⴦��!\n");

					}
					out("֤���ȡ�ɹ����ϱ���%d���˿�ca�豸\n", i);
					state->port_info.usb_app = 4;
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					state->port_info.ca_read = 0;
					goto END;

				}
			}
		END:
			port_unlock(state);
			continue;
		}
#endif
	}
}

#ifdef PRODUCE_PROGRAM

//ģ����������
#define DF_PC_USB_ORDER_CONNECT			0X01				//��������
#define DF_PC_USB_ORDER_GET_CARD_IFNFO	0X11				//��ȡ����Ϣ

#define LIBUSB_CHECK(action)                                                                   \
do {                                                                                           \
    int ret = (action);                                                                        \
    if (ret != LIBUSB_SUCCESS) {                                                               \
        return -1;                                                                             \
    }                                                                                          \
    break;                                                                                     \
} while (0)




int usb_device_open_hid(HUSB hUSB, int nBusNum, int nDevNum)
{
	if ((nBusNum<1) || (nBusNum>127))
	{
		printf("nBusNum range error\n");
		return -1;
	}
	if ((nDevNum<1) || (nDevNum>127))
	{
		printf("nDevNum range error\n");
		return -1;
	}

	libusb_device **device_list = NULL;
	int device_num = libusb_get_device_list(hUSB->context, &device_list);
	if (device_num <= 0) {
		libusb_free_device_list(device_list, 1);
		fprintf(stderr, "[Device] No device found on the bus.\n");
		return -1;
	}
	int i = 0;
	for (i = 0; i < device_num; i++) {
		struct libusb_device_descriptor desc;
		LIBUSB_CHECK(libusb_get_device_descriptor(device_list[i], &desc));
		int nNowBus = libusb_get_bus_number(device_list[i]);
		int nNowDev = libusb_get_device_address(device_list[i]);
		if (nBusNum != 0 || nDevNum != 0) {
			if (nNowBus != nBusNum || nNowDev != nDevNum) {
				continue;
			}
		}
		struct libusb_config_descriptor *conf_desc;
		LIBUSB_CHECK(libusb_get_active_config_descriptor(device_list[i], &conf_desc));
		unsigned int j = 0;
		for (j = 0; j < conf_desc->bNumInterfaces; j++) {
			const struct libusb_interface *interface = &conf_desc->interface[j];
			if (interface->num_altsetting >= 1) {
				const struct libusb_interface_descriptor *if_desc = &interface->altsetting[0];
				/*if (if_desc->bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE &&
				if_desc->bInterfaceSubClass == 0x06 && if_desc->bInterfaceProtocol == 0x50) {*/
				if (1){
					unsigned int k = 0;
					for (k = 0; k < if_desc->bNumEndpoints; k++) {
						const struct libusb_endpoint_descriptor *ep_desc = &if_desc->endpoint[k];

						if ((ep_desc->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
							if (ep_desc->bEndpointAddress & 0x80) {
								hUSB->endpoint_d2h = ep_desc->bEndpointAddress;
								//payload_d2h = ep_desc->wMaxPacketSize;
							}
							else {
								hUSB->endpoint_h2d = ep_desc->bEndpointAddress;
								//payload_h2d = ep_desc->wMaxPacketSize;
							}
						}
					}


					LIBUSB_CHECK(libusb_open(device_list[i], &(hUSB->handle)));
					LIBUSB_CHECK(libusb_reset_device(hUSB->handle));
					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					LIBUSB_CHECK(libusb_claim_interface(hUSB->handle, 0));

					libusb_set_auto_detach_kernel_driver(hUSB->handle, 1);
					if (libusb_reset_device(hUSB->handle) < 0) {
						fprintf(stderr, "[Device] libusb_reset_device reset failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -3;
					}
					uint8 szManufacturer[256] = "";
					if (libusb_get_string_descriptor_ascii(hUSB->handle, desc.iManufacturer,
						szManufacturer, 256) < 0)
					{
						fprintf(stderr, "[Device] libusb_get_string_descriptor_ascii failed\n");
						libusb_free_config_descriptor(conf_desc);
						libusb_free_device_list(device_list, 1);
						libusb_close(hUSB->handle);
						return -1;
					}

					int nDeviceType = -1;
					if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x1901))
					{
						nDeviceType = DEVICE_MENGBAI;
					}
					else if ((desc.idVendor == 0x0680) && (desc.idProduct == 0x2008))
					{
						nDeviceType = DEVICE_MENGBAI2;
					}
					else
						nDeviceType = DEVICE_UNKNOW;

					libusb_free_config_descriptor(conf_desc);
					libusb_free_device_list(device_list, 1);
					return nDeviceType;
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);
	}
	libusb_free_device_list(device_list, 1);
	fprintf(stderr, "[Device] No target device found.\n");
	return -4;
}


void usb_hid_close(HUSB husb)
{
	if (husb != NULL)
		libusb_close(husb->handle);
}


int base_err_to_errnum(unsigned char err, char *errinfo)
{
	int errnum;
	switch (err)
	{
	case 0xFF:
		out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFE:
		out("ʱ���ʽ����\n");
		sprintf(errinfo, "ʱ���ʽ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFD:
		out("����Ų����Ϲ淶\n");
		sprintf(errinfo, "����Ų����Ϲ淶");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFC:
		out("δע��\n");
		sprintf(errinfo, "δע��");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFB:
		out("δ����\n");
		sprintf(errinfo, "δ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFA:
		out("�˿��Ѹ�ʽ�����������ϵ�\n");
		sprintf(errinfo, "�˿��Ѹ�ʽ�����������ϵ�");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF9:
		out("��������,�����Ѿ�����\n");
		sprintf(errinfo, "��������,�����Ѿ�����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF8:
		out("�޴˷�Ʊ����\n");
		sprintf(errinfo, "�޴˷�Ʊ����");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF7:
		out("�޿��÷�Ʊ\n");
		sprintf(errinfo, "�޿��÷�Ʊ");
		errnum = DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		break;
	case 0xF6:
		out("�������\n");
		sprintf(errinfo, "�������");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF5:
		out("����֤�����Ϊ8λ\n");
		sprintf(errinfo, "����֤�����Ϊ8λ");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF4:
		out("�÷�Ʊ���ʹ����Ѵ���\n");
		sprintf(errinfo, "�÷�Ʊ���ʹ����Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF3:
		out("����Ȩ˰���Ѵ���\n");
		sprintf(errinfo, "����Ȩ˰���Ѵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF2:
		out("�÷�Ʊ�δ����ظ�\n");
		sprintf(errinfo, "�÷�Ʊ�δ����ظ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF1:
		out("��Ʊ�������\n");
		sprintf(errinfo, "��Ʊ�������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF0:
		out("�޴˷�Ʊ��\n");
		sprintf(errinfo, "�޴˷�Ʊ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xEF:
		out("������λ�Ǳ�����\n");
		sprintf(errinfo, "������λ�Ǳ�����");
		errnum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		break;
	case 0xEE:
		out("������Ʊ��������\n");
		sprintf(errinfo, "������Ʊ��������");
		errnum = DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		break;
	case 0xED:
		out("�������ŷ�Ʊ�޶�\n");
		sprintf(errinfo, "�������ŷ�Ʊ�޶�");
		errnum = DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT;
		break;
	case 0xEC:
		out("��������ʱ��\n");
		sprintf(errinfo, "��������ʱ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME;
		break;
	case 0xEB:
		out("���������ۼƽ��\n");
		sprintf(errinfo, "���������ۼƽ��");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
		break;
	case 0xEA:
		out("�޴���Ȩ˰��\n");
		sprintf(errinfo, "�޴���Ȩ˰��");
		errnum = DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
		break;
	case 0xE9:
		out("���ݿ��޴˷�Ʊ\n");
		sprintf(errinfo, "���ݿ��޴˷�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE8:
		out("δ������ʱ��\n");
		sprintf(errinfo, "δ������ʱ��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE7:
		out("��������Ϣ�ش�\n");
		sprintf(errinfo, "��������Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE6:
		out("��¼��������Ʊ��Χ\n");
		sprintf(errinfo, "��¼��������Ʊ��Χ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE5:
		out("��Ʊ������\n");
		sprintf(errinfo, "��Ʊ������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE4:
		out("�������ϸ���Ʊ\n");
		sprintf(errinfo, "�������ϸ���Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE3:
		out("ϵͳʱ�Ӵ���\n");
		sprintf(errinfo, "ϵͳʱ�Ӵ���");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE2:
		out("δ���������ܽ��м����Ϣ�ش�\n");
		sprintf(errinfo, "δ���������ܽ��м����Ϣ�ش�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE1:
		out("�����Ϣ�ش�����Ʊ��������Ӧ����˳��\n");
		sprintf(errinfo, "�����Ϣ�ش�����Ʊ��������Ӧ����˳��");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE0:
		out("�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ\n");
		sprintf(errinfo, "�����Ϸ�Ʊδ�ϴ������ϴ����Ϸ�Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDF:
		out("������Ʊδ�ϴ��������ϴ�����Ʊ\n");
		sprintf(errinfo, "������Ʊδ�ϴ��������ϴ�����Ʊ");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDE:
		out("ռδ֧�ָ�����\n");
		sprintf(errinfo, "ռδ֧�ָ�����");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDD:
		out("�к�Ʊδ�ϴ�\n");
		sprintf(errinfo, "�к�Ʊδ�ϴ�");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDC:
		out("�հ׷ϲ�֧������\n");
		sprintf(errinfo, "�հ׷ϲ�֧������");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	default:
		out("�������δ����\n");
		sprintf(errinfo, "ģ���̴������δ����");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	}
	return errnum;
}

int usb_data_write_read(HUSB husb, unsigned char *indata, int in_len, unsigned char *outdata, int *out_len, char *errinfo)
{
	int err_num;
	int result;
	int i;
	int num;
	int size;
	int total_len;
	int left_len;
	int index = 0;
	unsigned char s_data[50];
	unsigned char *r_data;
	uint16 crc_v;
	uint16 crc;

	r_data = (uint8 *)malloc(1000000);
	memset(r_data, 0, sizeof(1000000));
	num = in_len / 64 + 1;

	for (i = 0; i < num; i++)
	{
		result = libusb_interrupt_transfer(husb->handle, 0x01, indata + i * 64, 64, &size, 4000);
		if (result < 0)
		{
			out("���ݷ���ʧ��,result = %d\n", result);
			sprintf(errinfo, "ģ���̵ײ�����д�볬ʱʧ��");
			free(r_data);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}

	}
	//out("׼���������ݽ���\n");	
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 2000);
	if (result < 0)
	{
		out("���ݽ���ʧ��,result = %d\n", result);
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��ʱʧ��");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("�汾�Ŵ���\n");
		sprintf(errinfo, "ģ���̵ײ�����Ӧ��汾�Ŵ���");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	if (total_len > *out_len)
	{
		out("���ջ���ȥ�ڴ�̫С\n");
		sprintf(errinfo, "ģ���̵ײ����ݽ��ջ�����̫С");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if (left_len > 0)
	{
		num = (left_len / 22) + 1;
		//out("num = %d\n", num);
		for (i = 0; i < num; i++)
		{
			memset(s_data, 0, sizeof(s_data));
			libusb_interrupt_transfer(husb->handle, 0x82, s_data, 22, &size, 1000);
			memcpy(r_data + (22 * (i + 1)), s_data, size);
			index += size;
		}
	}
	else
	{
		num = 0;
	}
	crc = crc_8005(r_data, total_len - 2, 0);
	crc_v = r_data[total_len - 2] * 256 + r_data[total_len - 1];
	if (crc != crc_v)
	{
		out("crcУ�����\n");
		sprintf(errinfo, "ģ���̵ײ�Ӧ��CRC����У�����");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	if (r_data[4] != 0)
	{
		out("USB����Ӧ��Ϊ�������,%02x\n", r_data[4]);

		err_num = base_err_to_errnum(r_data[4], errinfo);
		free(r_data);
		r_data = NULL;
		return err_num;
	}

	*out_len = total_len;
	memcpy(outdata, r_data + 6, total_len - 8);
	free(r_data);
	r_data = NULL;
	return *out_len;
}

static int pack_s_buf(uint8 order, int s_num, uint8 *in_buf, uint8 *out_buf, int in_len)
{
	uint16 len;
	uint16 crc;
	len = in_len + 8;
	out_buf[0] = 'M'; out_buf[1] = 'B';
	//out("len = %d\n",len);
	out_buf[2] = ((len >> 8) & 0xff); out_buf[3] = (len & 0xff);
	out_buf[4] = order;
	out_buf[5] = s_num;
	memcpy(out_buf + 6, in_buf, in_len);
	crc = crc_8005(out_buf, len - 2, 0);
	out_buf[len - 2] = ((crc >> 8) & 0xff);
	out_buf[len - 1] = (crc & 0xff);
	return (int)len;
}

int LoadMengBaiInfo(HUSB device, HDEV HDev)
{
	int in_len, out_len;
	unsigned char s_data[4096];
	unsigned char r_data[4096];
	int result;
	char errinfo[2048] = { 0 };
	//��������
	//////////////////////////////////////////////////////////////////////////
	//out("��������\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//out("���õ�ǰʱ��%s\n", time_now);

	strcpy(HDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);

	result = usb_data_write_read(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("����ʧ��\n");
		return result;
	}

	//��ȡ����Ϣ
	////////////////////////////////////////////////////////////////////////
	//out("��ȡ����Ϣ\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("��ȡ����Ϣʧ��\n");
		return -1;
	}
	//out("��ȡ����Ϣ�ɹ�\n");

	HDev->uICCardNo = r_data[122] * 0x1000000 + r_data[123] * 0x10000 + r_data[124] * 0x100 + r_data[125] * 0x1;
	memcpy(HDev->szDeviceID, r_data, 12);
	memcpy(HDev->szCompanyName, r_data + 32, 80);
	//out("��ȡ������˰�����ƣ�%s\n", device->szCompanyName);
	memcpy(HDev->szCommonTaxID, r_data + 12, 20);


	return 0;
}



HDEV MallocDevInfo()
{
	HDEV hDev = calloc(1, sizeof(struct StaticDeviceInfo));
	if (!hDev)
		return NULL;
	return hDev;
}

HUSB MallocUSBSession()
{
	HUSB husb = calloc(1, sizeof(struct USBSession));
	if (!husb)
		return NULL;
	libusb_init(&husb->context);
	// libusb_set_debug(husb->context, LIBUSB_LOG_LEVEL_INFO);  // LIBUSB_LOG_LEVEL_DEBUG

	return husb;
}


void FreeDevInfo(HDEV hDev)
{
	if (!hDev)
		return;
	if (hDev->pX509Cert != NULL) {
		X509_free(hDev->pX509Cert);
		hDev->pX509Cert = NULL;
	}
	free(hDev);
	hDev = NULL;
}

static int usb_hid_close_serssion(HUSB h_USB, HDEV h_Dev)
{
	if (!h_USB)
		return 0;
	if (h_USB->handle)
		usb_hid_close(h_USB);
	libusb_exit(h_USB->context);
	free(h_USB);
	FreeDevInfo(h_Dev);
	return 0;
}
static int usb_hid_open_serssion(HUSB *h_USB, HDEV *h_Dev, int nBusID, int nDevID)
{
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nBusNum range error\n");
		return -1;
	}
	if ((nBusID<1) || (nDevID>127))
	{
		//printf("nDevNum range error\n");
		return -2;
	}
	HUSB hUSB = MallocUSBSession();
	HDEV hDev = MallocDevInfo();
	int nhUsbType = usb_device_open_hid(hUSB, nBusID, nDevID);
	if (nhUsbType == DEVICE_MENGBAI)
	{
		if (LoadMengBaiInfo(hUSB, hDev) < 0)
		{
			out("�豸�򿪳ɹ�,����ʧ��\n");
			usb_hid_close_serssion(hUSB, hDev);
			return -3;
		}
	}
	else
	{
		out("�豸��ʧ��,result = %d\n", nhUsbType);
		//usb_hid_close_serssion(hUSB, hDev);
		libusb_exit(hUSB->context);
		free(hUSB);
		FreeDevInfo(hDev);
		return -5;
	}

	*h_USB = hUSB;
	*h_Dev = hDev;
	hDev->hUSB = hUSB;
	return 0;
}

int function_common_get_basic_tax_info_simple(char *busid, int usb_type, char *ca_name, char *ca_serial, char *sn, int *passwd_right, int *left_num)
{
	int result;
	HUSB h_usb;
	HDEV h_dev;
	int nBusID;
	int nDevID;
	nBusID = get_busid_busnum(busid);
	nDevID = get_busid_devnum(busid);

	if (usb_type == DEVICE_MENGBAI)
	{
		*passwd_right= 1;
		result = usb_hid_open_serssion(&h_usb, &h_dev, nBusID, nDevID);
		if (result < 0)
		{
			out("�豸��ʧ��,result = %d\n", result);
			return DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR;
		}
		strcpy(ca_name,h_dev->szCompanyName);
		strcpy(ca_serial, h_dev->szCommonTaxID);
		strcpy(sn, h_dev->szDeviceID);
		usb_hid_close_serssion(h_usb, h_dev);
		return 0;
	}
	return -1;
}

static void deal_read_plate_base_tax_info_simple(void *arg)
{
	struct _ty_usb_m * stream;
	stream = ty_usb_m_id;
	struct _port_state   *state;
	int result, usb_type;// upfile_flag = 0;


	state = arg;

	if (state->usb_err != 0)
	{
		out("��ǰ�˿�%d����USBʱ���ֹ�����,��ʱ3���ٽ��ж�ȡ\n", state->port);
		if (state->power_mode == Normal_Mode)
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
	if (usb_type == DEVICE_MENGBAI)
	{
		if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)
		{
			sleep(3);
			stream->base_read_queue_num -= 1;

			state->in_read_base_tax = 0;
			return;
		}
		char ca_name[200] = { 0 };
		char ca_serial[20] = { 0 };
		char sn[16] = { 0 };
		int passwd_right;
		int left_num;

		result = function_common_get_basic_tax_info_simple(state->port_info.busid, usb_type, ca_name, ca_serial, sn, &passwd_right, &left_num);
		if (result < 0)
		{
			out("��%d��˰�̶�ȡ����˰��ʧ��\n", state->port);

			state->usb_err = 1;//��¼USB���ֹ�����
			if (passwd_right == 0)
			{
				out("��¼��˰���豸��������!\n");
				state->retry_forbidden = 1;
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "˰���豸�������");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				state->port_info.usb_app = 4;
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
			}
			port_unlock(state);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		_lock_set(state->lock);
		state->usb_err = 0;//�����ɹ����USB�����¼
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
		memcpy(state->port_info.ca_name, ca_name, strlen(ca_name));
		memcpy(state->port_info.ca_serial, ca_serial, strlen(ca_serial));
		memcpy(state->port_info.sn, sn, strlen(sn));
		state->port_info.usb_app = 4;
		state->port_info.ca_ok = 3;
		state->port_info.extension = 0;
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		out("���˿ںţ�%d,�̺ţ�%s,���ƣ�%s��\n", state->port, state->port_info.sn, state->port_info.ca_name);
		_lock_un(state->lock);
		state->port_info.tax_read = 0;
		port_unlock(state);
		stream->base_read_queue_num -= 1;
		state->in_read_base_tax = 0;
		return;
	}	
	return;
}


static void task_read_port_base_tax_info_simple(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int usb_type;
	stream = arg;
	char time_now[20] = { 0 };

	if (stream->frist_start == 0)
	{
		//out("�豸������,��ʱ5����ٶ�ȡ֤������\n");
		stream->frist_start = 1;
		sleep(5);
		return;
	}
	if (stream->frist_start == 1)
	{
		memset(time_now, 0, sizeof(time_now));
		timer_read_y_m_d_h_m_s(time_now);
		if (strcmp(time_now, SYSTEM_LEAST_TIME) < 0)
		{
			sleep(1);
			//out("�ȴ���ʱ���\n");
			return;
		}
		stream->frist_start = 2;
		return;
	}
	//out("task_check_port ���� %d\n",stream->usb_port_numb);
	for (i = 1; i <= stream->usb_port_numb; i++)
	{	//out("����%d�Ŷ˿�\n",i);
		state = &stream->port[i - 1];



		if ((state->port_info.with_dev == 0) || (state->retry_forbidden))
		{
			continue;
		}
		if (state->in_read_base_tax != 0)
		{
			//out("˰���ϴ�ʹ����\n");
			continue;
		}
		if (state->port_info.tax_read == 1)
		{

			if (state->power_mode == Saving_Mode)
			{
				if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port) < 0)
				{
					out("%d�˿ڶ������Ϣǰ���γ���!\n", state->port);
					logout(INFO, "SYSTEM", "CHECK_PORT", "��%d�ſڡ���ȡ�����Ϣǰ���γ���1\r\n", state->port);
					state->port_info.tax_read = 0;
					port_power_down(state);
					continue;

				}
			}

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if (usb_type == DEVICE_MENGBAI)
			{

				//�ɰ�ģ��������ʱ8�뷽�ɶ�ȡ��Ϣ
				long time_now;
				time_now = get_time_sec();
				if ((time_now - state->insert_time) < 8)
				{
					continue;
				}
			}
			else
			{
				continue;
			}


			//out("%d�Ŷ˿ڷ�Ʊ�ϴ��ж�\n", state->port);

			if (stream->base_read_queue_num > 4)
			{
				//out("�ϴ����д���5��,stream->up_queue_num = %d\n", stream->up_queue_num);
				continue;
			}
			if (state->port_info.used_level == 1)
			{
				continue;
			}

			stream->base_read_queue_num += 1;
			state->in_read_base_tax = 1;
			//out("%d�Ŷ˿���Ҫִ�л�����Ϣ��ȡ����\n",state->port);
			//logout(INFO, "system", "������Ϣ��ȡ", "%d�Ŷ˿���Ҫ��ȡ������Ϣ,���������̶߳���\r\n", i);
			_queue_add("up_m_server", deal_read_plate_base_tax_info_simple, state, 10);
			continue;
		}
	}
}

#endif

static int deal_read_plate_cert_passwd(struct _port_state   *state)
{
	int result;
	struct _plate_infos plate_infos;
	int left_num;
	char errinfo[1024] = { 0 };
#ifdef RELEASE_SO
	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#else
	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#endif
#ifndef ALL_SHARE
	if (result != 1)
	{
		state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
		state->port_info.plate_infos.plate_basic_info.cert_left_num = 99;
		state->port_info.plate_infos.plate_basic_info.cert_err_reason = DF_PLATE_USB_PORT_NOT_AUTH;
		return 0;
	}
#endif
	result = get_cert_passwd_err_port((char *)state->port_info.sn, &left_num);
	if (result == 0)
	{
		memset(&plate_infos, 0, sizeof(struct _plate_infos));
		if (state->port_info.usb_app == DEVICE_AISINO)//��˰��
		{
#ifdef RELEASE_SO
			result = so_aisino_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_aisino_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_NISEC)//˰����
		{
#ifdef RELEASE_SO
			result = so_nisec_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_nisec_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_CNTAX)//˰��UKEY
		{
#ifdef RELEASE_SO
			result = so_cntax_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_cntax_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_MENGBAI)//ģ����
		{
#ifdef RELEASE_SO
			result = so_mb_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_mengbai_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else
		{
			plate_infos.plate_basic_info.cert_passwd_right = 1;
			plate_infos.plate_basic_info.cert_left_num = 10;
			plate_infos.plate_basic_info.cert_err_reason = 0;
		}

		state->port_info.plate_infos.plate_basic_info.cert_passwd_right = plate_infos.plate_basic_info.cert_passwd_right;
		state->port_info.plate_infos.plate_basic_info.cert_left_num = plate_infos.plate_basic_info.cert_left_num;
		state->port_info.plate_infos.plate_basic_info.cert_err_reason = plate_infos.plate_basic_info.cert_err_reason;

		if ((state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0))//ERR_CERT_PASSWORD_ERR
		{

			if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
			{
				logout(INFO, "system", "passwd_auth", "%s,�ö˿�˰��֤����������֤ʧ��,����ԭ��Ϊ˰�̿����Ĭ�Ͽ���,ʣ�������֤����%d\r\n", state->port_info.port_str, state->port_info.plate_infos.plate_basic_info.cert_left_num);
				//state->cert_err = -1;
				update_cert_passwd_err_port((char *)state->port_info.sn, state->port_info.plate_infos.plate_basic_info.cert_left_num);
			}
			else
			{
				logout(INFO, "system", "passwd_auth", "%s,�ö˿�˰��֤����������֤ʧ��,����ԭ��ΪUSB����ʧ��\r\n", state->port_info.port_str);
				return -1;
			}
		}
	}
	else
	{
		state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
		state->port_info.plate_infos.plate_basic_info.cert_left_num = left_num;
		state->port_info.plate_infos.plate_basic_info.cert_err_reason = ERR_CERT_PASSWORD_ERR;
	}
	

	
	//out("֤���������\n");
	return 0;
}

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

static int get_cert_passwd_err_port(char *plate_num, int *left_num)
{
	char file_name[200] = { 0 };
	char s_buf[1024] = {0};
	sprintf(file_name, "/tmp/passwd_err/%s.txt", plate_num);
	if (access(file_name, 0) != 0)
	{
		return 0;
	}
	read_file(file_name, s_buf, sizeof(s_buf));
	*left_num = atoi(s_buf);
	return 1;
}

static void update_dev_passwd_err_port(int port, int left_num)
{
	char file_name[200] = { 0 };
	char buf[1024] = { 0 };
	sprintf(file_name,"/tmp/passwd_err/port%d.txt",port);
	sprintf(buf, "%d", left_num);
	write_file(file_name, buf, strlen(buf));
}

static void update_cert_passwd_err_port(char *plate_num,int left_num)
{
	char file_name[200] = { 0 };
	char buf[1024] = { 0 };
	sprintf(file_name, "/tmp/passwd_err/%s.txt", plate_num);
	sprintf(buf, "%d", left_num);
	write_file(file_name, buf,strlen(buf));	
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

	
	state = arg;

	if (state->usb_err != 0)
	{
		out("��ǰ�˿�%d����USBʱ���ֹ�����,��ʱ3���ٽ��ж�ȡ\n",state->port);
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
		if ((busnum == 0) || (busnum == -1) || (devnum == 0) || (devnum == -1))
		{
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}

		state->h_plate.nBusID = busnum;
		state->h_plate.nDevID = devnum;
		if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)
		{
			//out("�˿�%d,��ʹ����,�޷���ȡ֤������\n",i);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		memset(&plate_infos, 0, sizeof(struct _plate_infos));
		
		state->h_plate.usb_type = usb_type;
		state->h_plate.need_check_passwd = need_check_passwd;
		memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
		strcpy(state->h_plate.busid, state->port_info.busid);
#ifdef RELEASE_SO
		result = so_common_get_basic_tax_info(&state->h_plate, &plate_infos);
#else
		result = function_common_get_basic_tax_info(&state->h_plate, &plate_infos);
#endif
		if (result < 0)
		{
			out("��%d��˰�̶�ȡ����˰��ʧ��\n", state->port);
			state->usb_err = 1;//��¼USB���ֹ�����
			if (plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_ERROR)
			{
				//out("��¼��˰���豸��������!\n");
				logout(INFO, "system", "������Ϣ��ȡ", "��¼��%d�Ŷ˿�,˰���豸��������!,ʣ�ೢ�Դ���%d��\r\n", state->port,plate_infos.plate_basic_info.dev_left_num);
				state->retry_forbidden = 1;
				update_dev_passwd_err_port(state->port, plate_infos.plate_basic_info.dev_left_num);
				state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "˰���豸�������");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "˰���豸�������");
				memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
				memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);				
				state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
				state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
				state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				state->port_info.tax_read = 0;
			}
			else if (plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_LOCK)
			{
				//out("��¼��˰���豸��������!\n");
				logout(INFO, "system", "������Ϣ��ȡ", "��¼��%d�Ŷ˿�,˰���豸��������!,ʣ�ೢ�Դ���%d��\r\n", state->port, plate_infos.plate_basic_info.dev_left_num);
				//state->retry_forbidden = 1;//�˿���������������
				state->need_getinv = 0;//���ϴ���Ʊͳ�Ʋ�ѯ
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "˰���豸�������");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "˰���豸�������");
				memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
				memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);
				state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
				state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
				state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				state->port_info.tax_read = 0;		
				
			}
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
		memcpy(state->port_info.sn, plate_infos.plate_basic_info.plate_num, strlen((const char*)plate_infos.plate_basic_info.plate_num));
		state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
		state->port_info.ca_ok = plate_infos.plate_basic_info.result;
		state->port_info.extension = plate_infos.plate_basic_info.extension;
		memset(&state->port_info.plate_infos.plate_basic_info, 0, sizeof(struct _plate_basic_info));
		memcpy(&state->port_info.plate_infos.plate_basic_info, &plate_infos.plate_basic_info, sizeof(struct _plate_basic_info));
		memset(&state->port_info.plate_infos.plate_tax_info, 0, sizeof(struct _plate_tax_info));
		memcpy(&state->port_info.plate_infos.plate_tax_info, &plate_infos.plate_tax_info, sizeof(struct _plate_tax_info));
		state->port_info.offinv_num_exit = plate_infos.plate_tax_info.off_inv_num;
		memset(state->port_info.plate_time, 0, sizeof(state->port_info.plate_time));
		memcpy(state->port_info.plate_time, plate_infos.plate_tax_info.plate_time, strlen((char *)plate_infos.plate_tax_info.plate_time));
		sprintf(state->port_info.port_str,"���˿ںţ�%d,�̺ţ�%s,���ƣ�%s��",state->port,state->port_info.sn,state->port_info.ca_name);
		for (i = 0; i < DF_MAX_INV_TYPE; i++)
		{
			memset(&state->port_info.plate_infos.invoice_type_infos[i], 0, sizeof(struct _invoice_type_infos));
			memcpy(&state->port_info.plate_infos.invoice_type_infos[i], &plate_infos.invoice_type_infos[i], sizeof(struct _invoice_type_infos));

		}


		//wang 02 03
		//Ŀǰ�����ʱ�������� ���Ǵ�����滹���������
		//��֪ͨ��ά���ֱ������������ ��ֹstate->port_info.plate_sys_time��ȡ����ֵ���е����湦�ܵ�����������
		if (strlen((char *)state->port_info.plate_time) != 0)
		{
			// wang 02 03 ��ֹ����˰��ʱ����󣬼������ֶδ���
			// eg 20a0-03-01 02:12:26
			unsigned int iYear, iMon, iDay, iHour, iMin, iSec;
			sscanf((char *)state->port_info.plate_time, "%04d-%02d-%02d %02d:%02d:%02d", &iYear, &iMon, &iDay, &iHour, &iMin, &iSec);
			if((iYear < 1970)||(iMon < 1)||(iMon > 12)||(iDay < 1)||(iDay > 31)){
				out("%d�Ŷ˿�˰��ʱ���ȡ����,��ȡ��ʱ��Ϊ%s,��γ����̲���ϵ�ͻ��ڵ�����ȷ��˰��ʱ��!\r\n", state->port,(char *)state->port_info.plate_time);
				logout(INFO, "system", "������Ϣ��ȡ", "%d�Ŷ˿�˰��ʱ���ȡ����,��ȡ��ʱ��Ϊ%s,��γ����̲���ϵ�ͻ��ڵ�����ȷ��˰��ʱ��!\r\n", state->port,(char *)state->port_info.plate_time);		
			}
			else 
			{
				long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
				long time_now = get_time_sec();
				//out("˰��ʱ���Ϊ%ld,��ǰϵͳʱ���Ϊ%ld\n", plate_time, time_now);
				state->port_info.plate_sys_time = plate_time - time_now;
				//out("˰���뵱ǰϵͳʱ���Ϊ%d\n", state->port_info.plate_sys_time);
			}
		}

		/*if (strlen((char *)state->port_info.plate_time) != 0)
		{
			long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
			long time_now = get_time_sec();
			//out("˰��ʱ���Ϊ%ld,��ǰϵͳʱ���Ϊ%ld\n", plate_time, time_now);
			state->port_info.plate_sys_time = plate_time - time_now;
			//out("˰���뵱ǰϵͳʱ���Ϊ%d\n", state->port_info.plate_sys_time);
		}*/

		state->port_init = 2;
		_lock_un(state->lock);


		


		result = deal_read_plate_cert_passwd(state);
		if (result < 0)
		{
			state->usb_err = 1;//��¼USB���ֹ�����
			port_unlock(state);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		port_unlock(state);

	
		
		_lock_set(state->lock);
		//out("%d�Ŷ˿ڵĽ�˰��ȫ��˰����Ϣ��ȡ���,���ѳ����忨�߳�\n", state->port);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		state->port_info.need_chaoshui = 1;
		state->port_info.need_huizong = 1;
		state->port_info.need_fanxiejiankong = 1;
		_lock_un(state->lock);
		sem_post(&stream->cb_qk_sem);

#ifndef DF_OLD_MSERVER			
		state->last_report_time_t = 0;
#endif
		state->need_getinv = 1;//���ϴ���Ʊͳ�Ʋ�ѯ
		
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





//static int check_all_port_last_cert_passwd(struct _ty_usb_m  *stream, unsigned char *plate_num)
//{
//	int port;
//	int port_num = stream->usb_port_numb;
//	struct _port_state   *state;
//	for (port = 1; port <= port_num; port++)
//	{
//		state = &stream->port[port - 1];
//		if (state->port_last_info.usb_app == 0)
//		{
//			continue;
//		}
//		if (strcmp((const char *)plate_num, (const char *)state->port_last_info.sn) == 0)
//		{
//			if (state->port_last_info.cert_err == -1)
//				return -1;
//		}
//	}
//	return 0;
//}


static int vid_pid_jude_plate(uint16 vid, uint16 pid)
{
	//out("vid %04x,pid %04x\n", vid, pid);
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
		if (state->port_info.with_dev == 0)
		{
			if (state->retry_forbidden == 1)//�˿ڲ���豸���벻��ȷ��˰�̣��˶˿ڼ���������������ʹ��
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
			}
			else
			{
				if (jude_port_led_auth(state) < 0)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;

				}
				else
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_ON;
				}
			}
		}
		else
		{
			if (state->retry_forbidden == 1)//�˿ڲ���豸���벻��ȷ��˰�̣��˶˿ڼ���������������ʹ��
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				goto SET_LED;
			}
#ifdef ALL_SHARE
			else if ((( (jude_port_led_auth(state) == -1) || (jude_port_led_auth(state) == -2) ) || (state->port_info.usb_app != 4 && jude_port_led_auth(state) < 0) ) && state->port_info.plate_infos.plate_basic_info.dev_err_reason >=0)//δ��Ȩ����Ȩ���� 
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
				goto SET_LED;

			}
			else if (state->port_info.usb_app == 4)
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_ON_GREN_OFF;
				goto SET_LED;
			}

#endif
			else
			{
				if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == DF_PLATE_USB_PORT_NOT_AUTH)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
				}
				else if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_ERROR)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				}
				else if (state->port_info.plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_LOCK)
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				}
				else if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
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
		

	//out("������޶˿ڱ�USBshare ����\n");
	for (i = 1; i <= stream->usb_port_numb; i++)
	{
		state = &stream->port[i - 1];
		if ((state->port_info.port_status != 0) && (state->port_info.used_level==1))
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

	//out("���˿��������ڲ���˰����Ϣ\n");
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
			//out("lcd ָʾ���豸����˰����Ϣ\n");
			lcd_print_info.tax_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_tax_state = 10;
		}

	}
	else
	{
		//out("�޶˿ڲ���˰����Ϣ\n");
		if (stream->lcd_tax_state != 20)
		{
			//out("lcd ָʾ���豸����˰����Ϣ\n");
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
		//out("change_port_power_mode mode 0 �ر�%d�Ŷ˿�\n",state->port);
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
		//out("change_port_power_mode mode 1 �ر�%d�Ŷ˿�\n", state->port);
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

//����ֵ�������������ֽ���
int ReadBinFileIn(char *szPath, void **pBuff, int nBuffLen)
{
    uint8 *pDst = NULL;
    struct stat statbuf;
    int nFileSize = 0;
    int nRet = stat(szPath, &statbuf);
    if (nRet < 0)
        return -1;
    nFileSize = statbuf.st_size;
    int nReadMax = 0;
    uint8 bDynamicbuff = 0;
    nRet = -3;
    if (nBuffLen == 0) {
        bDynamicbuff = 1;
        pDst = malloc(nFileSize);
        if (!pDst)
            return -1;
        nReadMax = nFileSize;
    } else {
        if (nFileSize > nBuffLen)
            return -2;

        nReadMax = nFileSize;
        pDst = (uint8 *)pBuff;  //��̬�����һ���׵�ַ���������ַ
    }
    FILE *fp = fopen(szPath, "r");
    if (fp == NULL)
        goto ReadFileFromPath_Finish;
    int nByteIO = fread(pDst, 1, nReadMax, fp);
    fclose(fp);
    // 4096ʱ�п���ʱ�����С��ʵ���ļ�����4096��С�����ж�
    if (nReadMax != 4096) {
        //������ļ����жϴ�С�������豸��Ķ���4096��С
        if (nByteIO != nReadMax)
            goto ReadFileFromPath_Finish;
    }
    nRet = 0;
ReadFileFromPath_Finish:
    if (nRet) {
        if (bDynamicbuff)
            free(pDst);
    } else {
        if (bDynamicbuff)
            *pBuff = pDst;
        nRet = nByteIO;
    }
    return nRet;
}

int GetBusDevId(int *nBusID, int *nDevID,char *busid)
{
    bool ret = false;
    do {
        char szStr[4096] = "";  // ReadBinFile ��ȡ��С4k
        char szMountPath[256] = "";
        strcpy(szStr, busid);
        char *p = NULL;
        if ((p = strchr(szStr, ':'))) {
            // busid:devid
            *p = '\0';
            *nBusID = atoi(szStr);
            *nDevID = atoi(p + 1);
            ret = true;
            break;
        }
        if (!(p = strchr(szStr, '.'))) {
            printf("Error mount path1");
            break;
        }
        // mount path
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/busnum", busid);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFileIn(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            //printf("Read mount path failed :%s", szMountPath);
            break;
        }
        *nBusID = atoi(szStr);  //ĩβ�и�\n��Ӱ��
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/devnum", busid);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFileIn(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            //printf("Read mount path failed :%s", szMountPath);
            break;
        }
        *nDevID = atoi(szStr);  //ĩβ�и�\n��Ӱ��
        if (!*nBusID || !*nDevID) {
            //printf("Error mount path2");
            break;
        }
        ret = true;
    } while (false);
    return ret;
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

#ifndef ALL_SHARE
static int jude_port_allow_attach(struct _port_state  *state)
{

	if (state->port_info.with_dev == 0)
	{

		if((state->aisino_sup != 1)&&(state->nisec_sup != 1)&&(state->cntax_sup != 1))
			return -1;
		char date_now[100] = { 0 };
		timer_read_y_m_d_h_m_s(date_now);
		if (memcmp(date_now, state->end_time, 10) > 0)
		{
			return -2;
		}
	}
	else
	{

		int result;
		char errinfo[1024]={0};
#ifdef RELEASE_SO
		result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid,errinfo);
#else
		result = function_common_jude_plate_auth(state->port_info.usb_app,state->port_info.busid,errinfo);
#endif
		if (result != 1)
		{
			return -3;
		}
	}
	return 0;
}
#endif

static int jude_port_led_auth(struct _port_state  *state)
{
	if (state->port_info.usb_app == 4)//ca 
	{
		if ((state->aisino_sup != 1) && (state->nisec_sup != 1) && (state->cntax_sup != 1))
			return -1;
		char date_now[100] = { 0 };
		timer_read_y_m_d_h_m_s(date_now);
		if (memcmp(date_now, state->end_time, 10) > 0)
		{
			return -2;
		}
	}
	else 
	{
		if (state->port_info.with_dev == 0)
		{

			if ((state->aisino_sup != 1) && (state->nisec_sup != 1) && (state->cntax_sup != 1))
				return -1;
			char date_now[100] = { 0 };
			timer_read_y_m_d_h_m_s(date_now);
			if (memcmp(date_now, state->end_time, 10) > 0)
			{
				return -2;
			}
		}
		else
		{

			int result;
			char errinfo[1024] = { 0 };
#ifdef RELEASE_SO
			result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#else
			result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#endif
			if (result != 1)
			{
				return -3;
			}
		}
	}
	return 0;
}

int open_port_attach(int port, uint32 ip, char *client, int *port_fd)
{
	struct _ty_usb_m  *stream;

	//int i;
	int result;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;

	struct _port_state  *state;
	state = &stream->port[port - 1];
	out("[open_port_ip]���˿ںŴ򿪶˿� port = %d\n", port);
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
#ifndef ALL_SHARE
	if ((result = jude_port_allow_attach(&stream->port[port - 1])) < 0)
	{
		return DF_ERR_PORT_CORE_TY_USB_NO_PERMISSION;
	}
#endif
	if (state->power_mode == Normal_Mode)
	{
		if (state->port_info.usb_app == DEVICE_CNTAX && (state->port_info.vid == 0x101d && state->port_info.pid == 0x0003))//˰��Ukey
		{
			out("��ʼ���ض˿����µ�:%d\n");
			state->sys_reload = 1;
			/*ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			state->power = 0;*/
			port_power_down(state);
			usleep(20000);
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			state->power = 1;
			sleep(2);
			state->sys_reload = 0;
			out("�������ض˿����µ�\n");
		}
	}
	if (stream->port[port - 1].port_info.with_dev == 0)
	{
		out("[open_port_ip]û�з��ָ��豸����೬ʱ�ȴ�5��\n");

		int time = 0;
		for (time = 0; time < 5000; time++)
		{
			if (stream->port[port - 1].port_info.with_dev == 1)
			{
				sleep(1);
				break;
			}
			usleep(1000);
		}
		if (time == 5000)
		{
			out("[open_port_ip]�ö˿��Ѿ���,û�з��ָ��豸\n");
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
	}
	//if(stream->port[port-1].port_info.port_status!=0)
	//{   _lock_un(stream->port[port-1].lock);
	//    out("[open_port_ip]�ö˿��Ѿ�����ʹ��\n");
	//    return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;    
	//}
	//_lock_un(stream->port[port - 1].lock);
	//for (i = 0; i < 100; i++, usleep(50000))
	//{
	//	if (stream->port[port - 1].port_info.oper_type == 0)
	//		break;
	//}
	//if (i == 100)
	//{
	//	out("ϵͳ����ռ�ô�USB�豸,�˿ںţ�%d\n", port);
	//	return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	//}

	if ((stream->port[port - 1].port_info.port_status != 0) && (stream->port[port - 1].port_info.used_level == 1))
	{
		out("�˿��Ѿ�������\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}

	if (port_lock(&stream->port[port - 1], 10, 1, DF_PORT_USED_USB_SHARE) < 0)
	{
		out("���ض˿������˿�ʧ��\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}
	if ((result = open_port(stream, port, 0x7f000001, NULL))<0)
	{
		stream->port[port - 1].port_info.port_status = 0x00;
		out("[open_port_ip]�򿪶˿�ʧ��\n");
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
	out("ʹ��ǿ�ƹرն˿�,�˿ںš�%d��\n", port);
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}

	state = &stream->port[port - 1];
	//if (state->power_mode == Saving_Mode)
	//{
	//	out("ʹ��ǿ�ƹرն˿�,�ͷ�USB����˿ںš�%d��\n", port);
	//	free_h_plate(state);
	//}
	close_port_socket(stream, port);
	logout(INFO, "10001", "dettach", "%s,ǿ���ͷ�USB�˿�,�˿ںţ�%d\r\n", stream->port[port - 1].port_info.port_str, port);
	if (state->power_mode == Saving_Mode)
	{		
#ifdef ALL_SHARE
		state->port_info.ca_read = 1;
#endif		
	}
	return 0;
}

//�˿ڹرմ����߼�
static int close_port_socket(struct _ty_usb_m *stream, int port)
{
	struct _port_state   *state;
	state = &stream->port[port - 1];
	out("close_port_socket����ر�����,�˿ںš�%d��\n", port);
	_lock_set(stream->lock);
	state->port_info.with_dev = 1;//��������Ϊ�ر�
	_lock_un(stream->lock);
	out("***************���йر�socket����**************\n");

	if ((state->port_info.port_status == 0) && (state->port_info.used_level != 1))
	{
		out("�˿ڼ����Ѿ��رգ������ٴιر�\n");
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

	out("�ر�ת����·,�رն˿�=%d\n", port);
	ty_ctl(stream->tran_fd, DF_CLOSE_LINE_TRAN_TASK, port);

	_lock_set(stream->lock);
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


	//out("֤��仯״̬= %d\n", stream->port[port - 1].port_info.change);
	out("[close_port_socket]�ر�%d�˿ڳɹ�\n", port);
	return 0;
}

int get_usb_vid_pid(int port,uint16 *vid,uint16 *pid)
{   
    struct _port_state  *state;
    //struct _usb_file_dev    dev;
	//const char *prod, *vend;
	//char buff[100];
    //int result;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	//out("[get_usb_port_information]��ȡ%d��USB��Ϣ\n",port);
	if (vid == NULL || pid == NULL)
	{	out("USB�˿ڲ�������\n");\

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




static int find_usb_port_by_plate_num(struct _ty_usb_m  *stream, unsigned char *palte_num)
{
	int i=0;
	int time = 0;
	for (time = 0; time < 5000; time++)
	{
		for (i = 0; i < stream->usb_port_numb; i++)
		{
			//out("��%d���ڴ��¼���̺�Ϊ��%s,Ҫ��ѯ���̺�Ϊ:%s\n", i, stream->port[i].port_info.sn, palte_num);
			if ((memcmp(stream->port[i].port_info.sn, palte_num, 12) == 0) && (stream->port[i].in_read_base_tax == 0))//��ֹ�ն��������Ϣ��δ��ɿ�����֤�ͽ������������������쳣����
			{
				return i + 1;
			}
		}
		if (time == 5000)
		{
			return -1;
		}
		usleep(1000);
		continue;
	}
	return -1;
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
        out("�����й�����,������hubû���ҵ�,ϵͳҪ����������...\n");
        memset(name,0,sizeof(name));
        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
        fd=ty_open(name,0,0,NULL);       
        out("д�¼�\n");
        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_MAIN_HUB);
        ty_close(fd);
        _queue_add("reset",reset_systerm,stream,10); 
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
	out("close_port_ty_usb �����رն˿ڵ�Դ\n");
	//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
	//state->power = 0;
	port_power_down(state);
    usleep(80000);
	//if(result==DF_ERR_PORT_CORE_TY_USB_CLOSE)
	//{	//int fd;
	//	out("��Դ�رպ��ļ�ϵͳ��ʾUSB�豸��Ȼ����,�˿ں�%d\n",port);
	//}
	out("�رյ�Դ�󼴽����¿����˿ڵ�Դ\n");
	result = ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT_POWER,port);
	state->power = 1;
	if(result < 0)
	{
		out("�رն˿ڵ�Դ��,���´򿪶˿ڵ�Դʧ��,�˿ں�%d\n",port);
		logout(INFO, "system", "closeport", "�رն˿ڵ�Դ��,���´򿪶˿ڵ�Դʧ��,�ٴδ򿪶˿ڵ�Դ,�˿ں�%d\r\n", port);
		sleep(1);
		result = ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT_POWER,port);
		state->power = 1;
	}
	return result; 
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
			out("����%d�Ŷ˿ڵ�Դ\n", state->port);

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
			out("����%d�Ŷ˿ڵ�Դ\n", state->port);
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
				out("����%d�Ŷ˿ڵ�Դ\n", state->port);
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
		(strcmp(state->port_info.used_info, DF_PORT_USED_FPZF) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USBD_QD_LOCK) == 0))

	{
		if (time_now - state->port_info.used_time > 80)
		{
			logout(INFO, "system", "check_port", "%s������%s��ʱ������1����,ϵͳ�жϸ��̿��ܴ����쳣,������%d�˿ڽ��жϵ���������\r\n", state->port_info.port_str, state->port_info.used_info,state->port);
			out("����%d�Ŷ˿ڵ�Դ\n", state->port);
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
    {   _lock_un(state->lock);
        return 0;
    }
	if (state->port_info.usbip_state == 0)
    {   _lock_un(state->lock);
        return 0;
    }
	if (state->port_info.port_status == 0)
	{
		_lock_un(state->lock);
		return 0;
	}
	if (state->port_info.used_level!= 1)
	{
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
	{   out("[open_port]��ȡ�˿�[%d]busidʧ��\n",port);
		return result;
	}

	result=open_port_ty_usb(stream,port);
	if(result<0)
	{   
		out("[open_port]��ʧ��\n");
		return result;
	}

	return 1;
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
			if (state->port_info.port_used != 0)
			{
				//out("�˿��ѱ�����,�޷�����\n");
				//out("��ǰ�ȼ�%d,����ʹ��ʱ�ĵȼ�%d\n", used_level, state->port_info.used_level);
				if (used_level < state->port_info.used_level)
				{
					//out("��ǰ�ȼ�˵����%s��,����ʹ��ʱ�ĵȼ���%s��,DF_PORT_USED_FPKJ = %s,DF_PORT_USED_FPCX_MQTT= %s\n", used_info, state->port_info.used_info);
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
							(strcmp(used_info, DF_PORT_USED_CXJRGGFWPT) == 0) ||
							(strcmp(used_info, DF_PORT_USBD_QD_LOCK) == 0)
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
						if (strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)
						{
							int types_count = 0;
							int lxsc = 1;
							int lxsyje = 1;
							for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
							{
								if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
								{
									continue;
								}
								if (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxkpsc == 0)
								{
									lxsc = 0;
									break;
								}
								if (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxzssyje <= 0)
								{
									lxsyje = 0;
									break;
								}
							}
							if ((lxsc == 1) && (lxsyje == 1))
							{
								//out("��⵽����ռ�ã�ֱ��ǿ�ƽ�����\n");
								if (state->h_plate.hDev != NULL)
								{
									state->h_plate.hDev->bBreakAllIO = 1;
								}
							}
						}
						if ((strcmp(state->port_info.used_info, DF_PORT_USED_HZSC) == 0) ||
							(strcmp(state->port_info.used_info, DF_PORT_USED_FXJK) == 0) ||
							(strcmp(state->port_info.used_info, DF_PORT_USED_CSLJ) == 0))
						{
							//out("��⵽����ռ�ã�ֱ��ǿ�ƽ�����\n");
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
			//out("�˿��ѱ�����,�޷�����\n");
			if (strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0)
			{
				//out("%d�˿ںŶ�ȡ������Ϣ�����˿�\n",state->port);;
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
		out("%d�Ŷ˿��ͷ�ʱ�������ӿ�ʹ�ã��ݲ��رն˿ڵ�Դ\n",state->port);
		return;
	}
	state->sys_reload = 1; //�ر��豸������Ϣ
	state->ad_repair_down_time = get_time_sec();
	//out("%d�Ŷ˿�ʹ�ý������رն˿ڵ�Դ\n",state->port);
	port_power_down(state);
	//out("%d�Ŷ˿�ʹ�ý������رն˿ڵ�Դ���\n", state->port);
	
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
		//out("%d�Ŷ˿ڣ��ڽڵ�ģʽ��������������\n",state->port);
		_queue_add("up_m_server", close_port_power_saving_mode, state, 10);		
	}
	return 0;
}

//***================================ȫ�ֺ���===================================***//
//***===========================================================================***//
//���ö˿�USB�����ݴ���ָʾled��
int set_port_usb_data_tran(int port)
{
	struct _ty_usb_m  *stream;
	//out("������USB����,�˿ں�%d\n",port);
	stream = ty_usb_m_id;
	stream->port[port - 1].port_info.tran_data = 1;
	return 0;
}

int get_transport_line(struct _new_usbshare_parm *parm)
{
	int result;
	struct _ty_usb_m  *stream;
	stream = ty_usb_m_id;
	out("get_transport_line host:%s--port:%d-- clientid:%s\n", parm->trans_server, parm->port, parm->client_id);

	result = ty_ctl(stream->tran_fd, DF_OPEN_TRANSPORT_LINE, parm);//������ع���usbָ��
	if (result < 0)
	{
		return result;
		out("open_port_transport error DF_MQTT_REQUEST_SHARE_USB_PORT!\n");
	}
	return result;
}

//���»�ȡ��Ʊ����JSON����
int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	stream = ty_usb_m_id;
	state = &stream->port[port_num - 1];
	update_plate_info_cbqkzt(state);
	//out("�˿ںţ�%d,��Դ״̬��%d,�Ƿ�����豸��%d\n", state->port_info.port, state->port_info.power, state->port_info.with_dev);
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
int fun_get_palte_month_inv_num(char *plate_num, char *month, unsigned int *fp_num, unsigned long *fp_len, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int len=0;
	stream = ty_usb_m_id;
	
	//out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 5, DF_PORT_USED_AYCXFPSLMQTT) != 0)
	{
		sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ�·�Ʊ����\n");		
#ifdef RELEASE_SO
		len = so_aisino_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_aisino_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ�·�Ʊ����\n");
#ifdef RELEASE_SO
		len = so_nisec_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_nisec_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰���̻�ȡ�·�Ʊ����\n");
#ifdef RELEASE_SO
		len = so_cntax_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_cntax_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("�ɰ��̻�ȡ�·�Ʊ����\n");
#ifdef RELEASE_SO
		len = so_mb_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_mengbai_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	else
	{
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		return -1;
	}
	port_unlock(state);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (len < 0)
	{
		out("��ȡ��Ʊ����ʧ��\n");
	}	
	return 0;
}

//��Ʊ��ϸ��ѯ
int fun_get_invoice_detail_mqtt(char *plate_num, unsigned char *month, void * function, void * arg, char *errinfo, int old_new, int sfxqqd)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;

	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 5, DF_PORT_USED_AYCXFPSJMQTT) != 0)
	{
		sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̷�Ʊ��ϸ��ѯ\n");
#ifdef RELEASE_SO
		result = so_aisino_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_aisino_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̷�Ʊ��ϸ��ѯ\n");
#ifdef RELEASE_SO
		result = so_nisec_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_nisec_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY��Ʊ��ϸ��ѯ\n");
#ifdef RELEASE_SO
		result = so_cntax_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_cntax_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("�ɰ��̷�Ʊ��ϸ��ѯ\n");
#ifdef RELEASE_SO
		result = so_mb_get_inv_details_mqtt(&state->h_plate, month, function, arg,  errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_mengbai_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
		
	}
	else
	{
		result = -1;
	}
	port_unlock(state);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("��Ʊ��ѯʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "���»�ȡ��Ʊ����MQTT", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n",state->port_info.port_str);
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return result;
	}
	out("��Ʊ��ѯ�ɹ�\n");
	return 0;
}

int fun_get_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, char **inv_data, int dzsyh, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream,(unsigned char *) plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
#ifdef RELEASE_SO
		result = so_aisino_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm,dzsyh, inv_data);
#else
		result = function_aisino_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
		port_unlock(state);

		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#else
		result = function_nisec_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#else
		result = function_cntax_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
		
	}
	//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
	else if (sp_type == DEVICE_MENGBAI)
	{
		return -1;
	}
	return -1;
}

int fun_get_inv_from_fpdm_fphm_get_bin_str(char *plate_num, char *fpdm, char *fphm, int dzsyh,char **inv_data,char **kp_bin,char **kp_str, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_aisino_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#else
		result = function_aisino_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#else
		result = function_nisec_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��Ʊ��������ȡ��Ʊ��Ϣ\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#else
		result = function_cntax_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		if (result < 0)
		{
			out("��Ʊ��ѯʧ��\n");
			sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		return -1;
	}
	return -1;
}

int fun_stop_invoice_query(char *plate_num, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];

	if (state->port_info.inv_query != 0)
	{
		state->port_info.inv_query = 0;
		sleep(2);



		out("��ʱ3��,��%d�Ŷ˿��������µ�\n",port);
		logout(INFO, "MQTT", "ֹͣ��Ʊ��ѯ", "%s,ֹͣ��Ʊ��ѯ����,���������˿ڵ�Դ\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);

		
	}
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
#ifndef DF_1201_1
	if (net_status.nettime_ok != 1)
	{
		//out("��������δ����,�Ժ������ϱ��ն�״̬\n");
		return ;
	}
#endif

	time_now = get_time_sec();
	
	//out("��ʼִ���������ϱ�\n");
	if (time_now - stream->last_report_time_o > DF_TY_USB_MQTT_REPORT_STATUS-1)//����15��ᴥ��һ���ϱ�,�ߵ����ô���
	{
		//out("����һ��״̬�ϱ�\n");
		if (stream->report_ter_flag != 1)
		{
			if (stream->report_o_used == 0)
			{
				stream->report_o_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
				//pack_report_json_data(stream);
				//out("����һ���ն���״̬��Ϣ�ϱ�\n");
				if (_queue_add("0", pack_report_json_data, stream, 0)<0)
				{
					out("��������ʧ��\n");
				}
			}
		}
		//����Ѿ��ϱ�M������Ϣ�ɹ����ڽ����ϰ汾���������ϱ�,�����˴���Ŀ��Ϊ���ݸߵ�ֱ��MQTT�Խ�
		
	}
	if (time_now - stream->last_report_time_h > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT - 1)//����60��ᴥ��һ���ϱ�
	{
		//out("����һ��M��������\n");
		//out("����һ���ն�������Ϣ�ϱ�\n");
		if (stream->report_h_used == 0)
		{
			stream->report_h_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			if (_queue_add("0", pack_report_m_server_heart_data, stream, 0)<0)
			{
				out("��������ʧ��\n");
			}
		}
	}
	if (time_now - stream->last_report_time_s > DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT - 1)
	{
	
		//out("���һ��M�����ն˻�����Ϣ�ϱ�\n");
		if ((stream->report_s_used == 0) && (stream->report_ter_flag == 0))
		{
			//out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
			stream->report_s_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
#ifndef DF_1201_1
			if (_queue_add("0", pack_report_m_server_status_data, stream, 0)<0)
			{
				out("��������ʧ��\n");
			}
#else
			if (_queue_add("0", pack_report_m_server_status_data_linux64, stream, 0)<0)
			{
				out("��������ʧ��\n");
			}
#endif
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
		/*if (((stream->port[port_count].port_info.vid == 0x101d) && (stream->port[port_count].port_info.pid == 0x0003)) || \
			((stream->port[port_count].port_info.vid == 0x1432) && (stream->port[port_count].port_info.pid == 0x07dc)) || \
			((stream->port[port_count].port_info.vid == 0x0680) && (stream->port[port_count].port_info.pid == 0x1901)))*/
		if (sp_type>0)
		{
			cJSON_AddStringToObject(p_layer, "plate_num", (char *)stream->port[port_count].port_info.plate_infos.plate_basic_info.plate_num);
			memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].port_info.extension);
			cJSON_AddStringToObject(p_layer, "plate_extension", tmp);


			int result;
			char errinfo[1024] = { 0 };
#ifdef RELEASE_SO
			result = so_common_jude_plate_auth(stream->port[port_count].port_info.usb_app, stream->port[port_count].port_info.busid, errinfo);
#else
			result = function_common_jude_plate_auth(stream->port[port_count].port_info.usb_app, stream->port[port_count].port_info.busid, errinfo);
#endif
			if (result != 1)
			{
				cJSON_AddStringToObject(p_layer, "tax_auth", "0");
			}
			else
			{
				cJSON_AddStringToObject(p_layer, "tax_auth", "1");
			}

			memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", stream->port[port_count].port_info.plate_infos.plate_basic_info.cert_passwd_right);
			cJSON_AddStringToObject(p_layer, "cert_passwd_rignht", tmp);

			cJSON_AddStringToObject(p_layer, "area_code", (char *)stream->port[port_count].port_info.plate_infos.plate_tax_info.area_code);
		}
	}
	g_buf = cJSON_Print(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	_m_pub_no_answer("mb_fpkjxt_kpfw", g_buf);
	stream->last_report_time_o = get_time_sec();//�����������ϱ�ʱ��

	stream->report_o_used = 0;
	//out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);
	return ;
}

static void pack_report_m_server_status_data_linux64(void *arg)
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

	cJSON *dir1, *dir2, *dir3 ,*dir4;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	//sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);
	sprintf(source_topic, "mb_fpkjxt_%s", "000000001234");

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbztxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());


	//cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	//fun_get_ter_base_data_json(dir3);
	//cJSON_ReplaceItemInObject(dir3, "usb_port_manage", cJSON_CreateString(""));

	cJSON_AddItemToObject(dir2, "server_linux64", dir4 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir4, "ter_id", "000000001234");
	cJSON_AddStringToObject(dir4, "system_version", "ubunru-20.04");
	cJSON_AddStringToObject(dir4, "Linux_version", "5.15.0-67-generic");//Linux version 5.15.0-67-generic
	cJSON_AddStringToObject(dir4, "ter_version", "0-0-7-1");			//0-0Ԥ����7-1 ��ʾ7�µ�1��
	cJSON_AddStringToObject(dir4, "software_version_time", "23-7-19");
	cJSON_AddStringToObject(dir4, "server_ip", "mb_192.168.0.21");
	cJSON_AddStringToObject(dir4, "server_name", "mbi");


	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	
	out("		1		g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = {0};
	
	out("		2		����һ��M�����ն˻�����Ϣ�ϱ�\n");
	
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
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
		//out("�ϱ�M�����ն�״̬����,������Ӧ����ɹ�\n");
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	//stream->report_s_used = 0;
	stream->report_s_used = 1;
	return ;
}



void fp_sync_report_json_data(void *arg,char *platebuff,char *monthbuff)
{
	struct _ty_usb_m  *stream;
	char *g_buf;
	char source_topic[50] = { 0 };
	stream = arg;
	
	cJSON *dir1, *dir2,*dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	memset(source_topic, 0, sizeof(source_topic));

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

    cJSON_AddStringToObject(dir3, "plate_num", platebuff);
	cJSON_AddStringToObject(dir3, "month", monthbuff);

	g_buf = cJSON_Print(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("fp_sync_report_json_data g_buf = \n%s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	_m_pub_no_answer("mb_fp_month_sync_report", g_buf);

	free(g_buf);
	cJSON_Delete(json);
	return ;
}

static void pack_report_m_server_heart_data(void *arg)
{
	struct _ty_usb_m  *stream;
	char *g_buf;
	stream = arg;
	
	cJSON *json = cJSON_CreateObject();

#ifndef DF_1201_1
	cJSON_AddStringToObject(json, "ter_id", stream->ter_id);
#else
	cJSON_AddStringToObject(json, "ter_id", "000000001234");
#endif

	g_buf = cJSON_PrintUnformatted(json);
	//out("g_buf = %s\n",g_buf);
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


	cJSON_ReplaceItemInObject(dir3, "usb_port_manage", cJSON_CreateString(""));




	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = {0};
	//out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
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
		//out("�ϱ�M�����ն�״̬����,������Ӧ����ɹ�\n");
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_s_used = 0;
	return ;
}













#ifndef DF_OLD_MSERVER

//��ʱ�ϱ��˿���Ϣ�߳�
static void timely_tigger_report_port_status(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	char tmp[20] = {0};
	int port_counts = 0;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
		return;
	}
	char *port_data;

	cJSON *port_array, *port_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "port_data", port_array = cJSON_CreateArray());

	for (port = 1; port <= stream->usb_port_numb; port++)
	{	
		
		state = &stream->port[port - 1];
		if ((state->off_report == 0) && (state->up_report == 0))
		{
			continue;
		}	
		else if ((state->off_report == 1) && (state->up_report == 0))
			state->off_up_flag = 1;//���������ϴ�
		else if ((state->off_report == 0) && (state->up_report == 1))
			state->off_up_flag = 2;//���������ϴ�
		else if ((state->off_report == 1) && (state->up_report == 1))
		{
			if (state->last_act == 1)//������������Ȳ��������ϴ�
				state->off_up_flag = 1;
			if (state->last_act == 2)//������������Ȳ��������ϴ�
				state->off_up_flag = 2;
		}
		else
		{
			state->off_up_flag = 2;
		}

		if (state->off_up_flag == 2)
		{
			//out("�����%d�Ŷ˿��ϱ�\n", port);
			state->up_report = 0;
			
			if (state->port_info.usb_app == DEVICE_AISINO ||
				state->port_info.usb_app == DEVICE_NISEC ||
				state->port_info.usb_app == DEVICE_MENGBAI ||
				state->port_info.usb_app == DEVICE_MENGBAI2 ||
				state->port_info.usb_app == DEVICE_CNTAX)
			{
				//out("��%d�Ŷ˿ڴ���һ��˰�̸����ϱ�\n", state->port);
				pack_report_plate_info_json(state, state->off_up_flag, &port_data);
				//logout(INFO, "system", "report_plate", "���˿ںţ�%d���ϱ�˰��״̬,report_serial = %04x\r\n", state->port, state->now_report_counts);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "plate");
					cJSON_AddStringToObject(port_layer, "port_off_on", "on");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);					
					free(port_data);
				}				
				continue;
			}
#ifdef ALL_SHARE
			else if (state->port_info.usb_app == 0 || state->port_info.usb_app ==5)
			{
				pack_report_other_info_json(state, state->off_up_flag, &port_data);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "other");
					cJSON_AddStringToObject(port_layer, "port_off_on", "on");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);					
					free(port_data);
				}
				continue;
			}
			else if(state->port_info.usb_app == 0 || state->port_info.usb_app == 4)
			{
				pack_report_ca_info_json(state, state->off_up_flag, &port_data);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "ca");
					cJSON_AddStringToObject(port_layer, "port_off_on", "on");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);					
					free(port_data);
				}
				continue;
			}
#endif
			else
			{
				continue;
			}
		}
		else if (state->off_up_flag == 1)
		{
			state->off_report = 0;

			if (state->port_info.usb_app == DEVICE_AISINO ||
				state->port_info.usb_app == DEVICE_NISEC ||
				state->port_info.usb_app == DEVICE_MENGBAI ||
				state->port_info.usb_app == DEVICE_MENGBAI2 ||
				state->port_info.usb_app == DEVICE_CNTAX)
			{
				out("��%d�Ŷ˿ڴ���һ��˰�������ϱ�\n", state->port);
				pack_report_plate_info_json(state, state->off_up_flag, &port_data);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "plate");
					cJSON_AddStringToObject(port_layer, "port_off_on", "off");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);					
					free(port_data);
				}
				continue;
			}

			else if (state->port_info.usb_app == 0 || state->port_info.usb_app == 5)
			{
				pack_report_other_info_json(state, state->off_up_flag, &port_data);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "other");
					cJSON_AddStringToObject(port_layer, "port_off_on", "off");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);					
					free(port_data);
				}
				continue;
			}
#ifdef ALL_SHARE
			else if (state->port_info.usb_app == 0 || state->port_info.usb_app == 4)
			{
				pack_report_ca_info_json(state, state->off_up_flag, &port_data);
				if (port_data != NULL)
				{
					state->in_report = 1;
					port_counts += 1;
					cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
					cJSON_AddStringToObject(port_layer, "port_num", tmp);
					cJSON_AddStringToObject(port_layer, "port_type", "ca");
					cJSON_AddStringToObject(port_layer, "port_off_on", "off");
					memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
					cJSON_AddStringToObject(port_layer, "report_serial", tmp);
					cJSON_AddStringToObject(port_layer, "port_data", port_data);
					
					free(port_data);
				}
				continue;
			}
#endif
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	if (port_counts == 0)
	{
		cJSON_Delete(json);
		return ;
	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);



	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);

	//out("���������ѹ��ǰ���ݴ�С��%d,ѹ�������ݴ�С��%d\n", strlen(tmp_json), json_zlib_len);

	free(tmp_json);


	char source_topic[50] = { 0 };
	char random[50] = { 0 };
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

	char *g_buf = cJSON_PrintUnformatted(root);
	//out("�ϱ�˰�̷������ݣ�%s\n",g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_equ_ports_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	free(g_buf);
	cJSON_Delete(root);
	if (result <= 0)
	{
		if (result < 0)
		{
			logout(INFO, "system", "report_plate", "���ϱ�˰��״̬�����������ճ�ʱʧ��,��������\r\n");
		}
		else
		{
			logout(INFO, "SYSTEM", "report_plate", "���ϱ�˰��״̬��������Ӧ����ʧ��,��������\r\n");			
		}

		fun_deal_all_port_report();//������Ӧ��ʧ�ܣ����´�������˰����Ϣ
		return;
	}
	//out("��ȡ���ķ�������Ϊ%s\n", g_data);
	analysis_ports_report_response(state, g_data);

	//logout(INFO, "system", "report_plate", "���ϱ�˰��״̬��������Ӧ��ɹ�,�����������\r\n");
	
	return;
}

static int analysis_ports_report_response(struct _port_state   *state, char *json_data)
{
	struct _ty_usb_m  *stream;
	int result = -1;
	int size;
	int i;
	char ports_report_serial[2000] = { 0 };
	stream = ty_usb_m_id;
	int true_return_len = stream->usb_port_numb *5;
	cJSON *root = cJSON_Parse(json_data);
	if (!root)
	{
		out("1Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		out("2Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size != 1)
	{
		goto End;
	}
	cJSON *arrayItem, *item;
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			out("3Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "ports_report_serial");
		memset(ports_report_serial, 0, sizeof(ports_report_serial));
		if (get_json_value_can_not_null(item, ports_report_serial, 0, 2000) < 0) {
			goto End;
		}
		//out("��ȡ�������ж˿�����Ϊ%s\n", ports_report_serial);
		if (strlen(ports_report_serial) != true_return_len)
		{
			out("���صĶ˿���Ϣ���ݳ�������\n");
			goto End;
		}

		int port=0;
		int index=0;
		for (port = 1;port <= stream->usb_port_numb; port++)
		{
			uint8 hex_tmp[10] = {0};
			char tmp[10] = {0};
			uint16 port_report_serial;

			memset(tmp,0,sizeof(tmp));
			memset(hex_tmp, 0, sizeof(hex_tmp));
			state = &stream->port[port - 1];

			memcpy(tmp, ports_report_serial + index,4);
			index += 5;
			asc2hexbuf(tmp, hex_tmp, strlen(tmp));
			port_report_serial = hex_tmp[0] * 0x100 + hex_tmp[1];
			//out("%d�Ŷ˿ڻ�ȡ�����ϱ����к�Ϊ��%02x,�ڴ��е�ǰ���к�Ϊ:%02x\n",port, port_report_serial,state->now_report_counts);

			if (port_report_serial != state->now_report_counts)
			{
				logout(INFO, "system", "report_plate", "���˿ںţ�%d����ȡ�����ϱ����к�Ϊ��%04x,�ڴ��е�ǰ���к�Ϊ:%04x,�ڴ����ݲ�һ��,��Ҫ�����ϴ�,str = %s\r\n", port, port_report_serial, state->now_report_counts, tmp);
				//out("%d�Ŷ˿ڻ�ȡ���ϱ����кŲ�һ��,��Ҫ�����ϴ�\n",port);
				//out("%d�Ŷ˿ڻ�ȡ�����ϱ����к�Ϊ��%02x,�ڴ��е�ǰ���к�Ϊ:%02x\n",port, port_report_serial,state->now_report_counts);
				if (state->now_report_counts == 0)
				{
					state->off_report = 1;
					state->last_act = 2;					
				}
				else
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
				}
			}
			else
			{
				//out("%d�Ŷ˿ڻ�ȡ���ϱ����к�һ��,����Ҫ�����ϴ�\n", port);
				if (state->in_report != 0)
				{
					//out("�ϴ�˰����Ϣ��%d�ſڣ�%s��˰����Ϣ������Ӧ����ɹ�\n", state->port, state->port_info.ca_name);
					state->in_report = 0;
				}
				state->off_up_flag = 0;
			}

		}


	}


	result = 0;
End:
	cJSON_Delete(root);
	return result;
}


//��ʱ��ȡ˰�̵�ǰ�����Ϣ
static void timely_tigger_get_plate_invoice_db_info(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//	out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		if (stream->report_ter_flag == 0)
		{
			//	out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
			return;
		}
		state = &stream->port[port - 1];
		if (state->now_report_counts == 0)
		{
			//	out("�˿�״̬δ�ϴ��ɹ������в�ѯ\n");
			continue;
		}
		if (stream->sync_queue_num > 4)
		{
			continue;
		}
		if (state->need_getinv == 1 &&
			(state->port_info.usb_app == DEVICE_AISINO ||
			state->port_info.usb_app == DEVICE_NISEC ||
			state->port_info.usb_app == DEVICE_MENGBAI ||
			state->port_info.usb_app == DEVICE_CNTAX))
		{
			//if (stream->report_i_used == 1)
			//{
			//	//out("�˿�״̬�ϴ���\n");
			//	continue;
			//}
			long time_now = get_time_sec();

			if (time_now - state->last_report_time_t > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT - 1)//����60��ᴥ��һ��
			{				
				if (stream->report_t_used == 0)
				{
					out("��ѯ���ʱ�䣺%d\n", time_now - state->last_report_time_t);
					stream->report_t_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
					stream->sync_queue_num += 1;
					out("����һ�λ�ȡ˰�����ϴ���Ʊ������ѯ\n");
					_queue_add("0", pack_request_upload_inv_num, state, 0);
				}
			}
		}
	}
}

static int pack_report_plate_info_json(struct _port_state   *state, int off_up_flag, char **g_buf)
{
	struct _ty_usb_m  *stream;
	struct _ter_info ter_info;
	char tmp[20] = { 0 };
	stream = ty_usb_m_id;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	cJSON *json = cJSON_CreateObject();	
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

		state->now_report_counts = 0;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
	}
	else
	{
		//�����ϱ�
		cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(json, "offline", "1");

		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
		fun_get_port_data_json(json, state->port, 1);
	}




	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	return 0;

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	out("˰���豸���������ѹ��ǰ���ݴ�С��%d,ѹ�������ݴ�С��%d\n", strlen(tmp_json), json_zlib_len);
	free(tmp_json);
	*g_buf = json_zlib;
	//out("����һ��˰����Ϣ�ϱ�,�˿ںţ�%d\n",state->port);
	//out("g_buf = %s\n", *g_buf);
	return 0;
}

static int pack_report_other_info_json(struct _port_state   *state, int off_up_flag, char **g_buf)
{
	struct _ty_usb_m  *stream;
	char *tmp_json;
	char tmp[20] = { 0 };
	stream = ty_usb_m_id;	
	//out("����һ������USB�ϱ�,�˿ںţ�%d\n",state->port);
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json, "port_num", tmp);

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

		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
	}
	else
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		cJSON_AddStringToObject(json, "offline", "1");

		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
	}
	tmp_json = cJSON_PrintUnformatted(json);
	str_replace(tmp_json, "\\\\", "\\");
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	//char g_data[5000] = { 0 };
	//result = _m_pub_need_answer(random, "mb_equ_other_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	
	cJSON_Delete(json);

	*g_buf = tmp_json;
	return 0;


	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	out("δ֪�豸���������ѹ��ǰ���ݴ�С��%d,ѹ�������ݴ�С��%d\n", strlen(tmp_json), json_zlib_len);
	free(tmp_json);
	*g_buf = json_zlib;
	return 0;
}

#ifdef ALL_SHARE
static int pack_report_ca_info_json(struct _port_state   *state, int off_up_flag, char **g_buf)
{

	struct _ty_usb_m  *stream;
	char *tmp_json;
	char tmp[20];
	int result;
	stream = ty_usb_m_id;
	
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	cJSON *json = cJSON_CreateObject();

	


	cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json, "port_num", tmp);


	char errinfo[1024] = { 0 };
#ifdef RELEASE_SO
	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#else
	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#endif
	if (result != 1)
	{
		cJSON_AddStringToObject(json, "tax_auth", "0");
	}
	else
	{
		cJSON_AddStringToObject(json, "tax_auth", "1");
	}

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->aisino_sup);
	cJSON_AddStringToObject(json, "aisino_sup", tmp);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->nisec_sup);
	cJSON_AddStringToObject(json, "nisec_sup", tmp);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->cntax_sup);
	cJSON_AddStringToObject(json, "cntax_sup", tmp);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->mengbai_sup);
	cJSON_AddStringToObject(json, "mengbai_sup", tmp);

	cJSON_AddStringToObject(json, "aisino_end_time", (char *)state->aisino_end_time);
	cJSON_AddStringToObject(json, "nisec_end_time", (char *)state->nisec_end_time);
	cJSON_AddStringToObject(json, "cntax_end_time", (char *)state->cntax_end_time);
	cJSON_AddStringToObject(json, "mengbai_end_time", (char *)state->mengbai_end_time);

	cJSON_AddStringToObject(json, "start_time", (char *)state->start_time);
	cJSON_AddStringToObject(json, "end_time", (char *)state->end_time);
	cJSON_AddStringToObject(json, "frozen_ago_time", (char *)state->frozen_ago_time);


	/*memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->dev_stream->auth_ok);
	cJSON_AddStringToObject(dir3, "tax_auth", "1");
	cJSON_AddStringToObject(dir3, "start_time", state->dev_stream->start_time);
	cJSON_AddStringToObject(dir3, "end_time", state->dev_stream->end_time);*/
	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);


		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		if (strlen((char *)state->port_last_info.ca_name) == 0)
		{
			cJSON_AddStringToObject(json, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "name", (char *)state->port_last_info.ca_name);
		if (strlen((char *)state->port_last_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "number", (char *)state->port_last_info.ca_serial);
		if (strlen((char *)state->port_last_info.sn) == 0)
		{
			cJSON_AddStringToObject(json, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "serial", (char *)state->port_last_info.sn);
		//printf("step1\n");
		cJSON_AddStringToObject(json, "usb_share", "0");
		cJSON_AddStringToObject(json, "usb_share_mode", "0");
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json, "offline", "0");
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
		//printf("step2\n");
	}
	else
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		if (strlen((char *)state->port_info.ca_name) == 0)
		{
			//out("#####################ca_name :%s###########\n",)
			cJSON_AddStringToObject(json, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "name", (char *)state->port_info.ca_name);
		if (strlen((char *)state->port_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "number", (char *)state->port_info.ca_serial);
		if (strlen((char *)state->port_info.sn) == 0)
		{
			cJSON_AddStringToObject(json, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json, "serial", (char *)state->port_info.sn);

		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(json, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(json, "usb_share_mode", tmp);
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json, "offline", "1");
		stream->report_counts += 1;
		state->now_report_counts = stream->report_counts;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);
	}



	tmp_json = cJSON_PrintUnformatted(json);
	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	return 0;



	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	out("CA�豸���������ѹ��ǰ���ݴ�С��%d,ѹ�������ݴ�С��%d\n", strlen(tmp_json), json_zlib_len);
	free(tmp_json);
	*g_buf = json_zlib;
	return 0;
}
#endif
static void pack_request_upload_inv_num(void *arg)
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

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("xscfpcx", random);

	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "xscfpcx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
	cJSON_AddStringToObject(dir3, "usb_app", tmp);
	cJSON_AddStringToObject(dir3, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(dir3, "number", (char *)state->port_info.ca_serial);
	cJSON_AddStringToObject(dir3, "plate_num", (char *)state->port_info.sn);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.extension);
	cJSON_AddStringToObject(dir3, "plate_extension", tmp);
	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("g_buf = %s\n", g_buf);
	state->last_report_time_t = get_time_sec();
	
	if ((state->port == 0) || (strlen((char *)state->port_info.sn) == 0))
	{
		logout(INFO, "system", "M�����ѯͬ��", "%s,�˿ں���ֵ�쳣���̺��쳣\r\n", state->port_info.port_str);
		state->need_getinv = 0; //����˰���Ѿ��γ�������˴����ѯ����
		cJSON_Delete(json);
		stream->report_t_used = 0;
		stream->sync_queue_num -= 1;
		return;
	}


	int g_data_len = 2*1024 * 1024;
	char *g_data;
	g_data = calloc(1, g_data_len);
	result = _m_pub_need_answer(random, "mb_inv_up_get", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		logout(INFO, "SYSTEM", "M�����ѯͬ��", "%s,��ѯ˰�̷�Ʊͬ����Ϣ,��������Ӧ��ʱ\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M�����ѯͬ��", "%s,��ѯ˰�̷�Ʊͬ����Ϣ,������Ӧ����ʧ��\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else
	{		
		//out("������Ӧ�����ѯ�·�Ʊ�����ɹ�,Ӧ��������Ϊ%s\n\n\n", g_data);
		result = analysis_get_inv_num_response(state,g_data);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "M�����ѯͬ��", "%s,��ѯ˰�̷�Ʊͬ����Ϣ,Ӧ�����ݽ���ʧ��\r\n", state->port_info.port_str);
			free(g_buf);
			cJSON_Delete(json);
			goto End;
		}
		state->need_getinv = 0;

	}
	//sleep(5);
	//out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	free(g_data);
	stream->report_t_used = 0;
	stream->sync_queue_num -= 1;
	return;
}

static int analysis_get_inv_num_response(struct _port_state   *state, char *json_data)
{
	int result = -1;
	int size;
	int i;
	char invoice_month[20] = { 0 };
	char summary_month[20] = { 0 };
	char plate_num[20] = { 0 };
	char *fphm_crc = NULL;
	cJSON *root = cJSON_Parse(json_data);
	if (!root)
	{
		out("1Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		out("2Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size > 1)
	{
		goto End;
	}
	cJSON *arrayItem, *item;
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			out("3Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		memset(plate_num, 0, sizeof(plate_num));
		if (get_json_value_can_not_null(item, plate_num, 0, 20) < 0) {
			goto End;
		}
		if (strlen(plate_num) == 0)
		{
			out("��ȡ�����̺�Ϊ��,����������\n");
			goto End;
		}
		//out("��ȡ�����·�Ϊ%s\n", state->port_info.upload_inv_num[i].month);
		if (strcmp(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num) != 0)
		{
			out("��ȡ�����̺�%s,�뵱ǰ�̺ţ�%s��һ��\n", plate_num, state->port_info.plate_infos.plate_basic_info.plate_num);
			goto End;
		}
		
		item = cJSON_GetObjectItem(arrayItem, "invoice_month");
		memset(invoice_month, 0, sizeof(invoice_month));
		if (get_json_value_can_not_null(item, invoice_month, 0, 10) < 0) {
			goto End;
		}

		item = cJSON_GetObjectItem(arrayItem, "summary_month");
		memset(summary_month, 0, sizeof(summary_month));
		if (get_json_value_can_not_null(item, summary_month, 0, 10) < 0) {
			goto End;
		}
				

		item = cJSON_GetObjectItem(arrayItem, "fphm_fpxxcrc");
		int dmhmzfsbstr_len = 1024 * 1024;
		char *dmhmzfsbstr;
		dmhmzfsbstr = malloc(dmhmzfsbstr_len);
		if (dmhmzfsbstr == NULL)
			goto End;
		memset(dmhmzfsbstr, 0, dmhmzfsbstr_len);
		result = get_json_value_can_not_null(item, dmhmzfsbstr, 0, dmhmzfsbstr_len);
		if (result < 0)
		{
			out("��ȡdmhmzfsbstr����ʧ��\n");
			free(dmhmzfsbstr);
			goto End;
		}
		//out("dmhmzfsbstr:%s\n", dmhmzfsbstr);
		if (strlen(dmhmzfsbstr) == 0)
		{
			free(dmhmzfsbstr);
			dmhmzfsbstr = NULL;
			if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
			{
				free(state->port_info.upload_inv_num.dmhmzfsbstr);				
			}
			state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
		}
		else
		{

			
			char *dmhmzfsbstr_tmp = malloc(dmhmzfsbstr_len);
			if (dmhmzfsbstr_tmp == NULL)
			{
				free(dmhmzfsbstr);
				goto End;
			}
			memset(dmhmzfsbstr_tmp, 0, dmhmzfsbstr_len);
			uncompress_asc_base64((const uint8 *)dmhmzfsbstr, strlen((const char *)dmhmzfsbstr), (uint8 *)dmhmzfsbstr_tmp, &dmhmzfsbstr_len);
			free(dmhmzfsbstr);
			//out("dmhmzfsbstr:%s\n", dmhmzfsbstr_tmp);

			int fphm_crc_len = strlen(dmhmzfsbstr_tmp) + 10;
			fphm_crc = malloc(fphm_crc_len);
			if (fphm_crc == NULL)
			{
				free(dmhmzfsbstr_tmp);
				goto End;
			}

			memset(fphm_crc, 0, fphm_crc_len);
			strcpy(fphm_crc, dmhmzfsbstr_tmp);
			free(dmhmzfsbstr_tmp);


		}
		


		_lock_set(state->lock);
		memset(state->port_info.upload_inv_num.invoice_month, 0, sizeof(state->port_info.upload_inv_num.invoice_month));
		strcpy(state->port_info.upload_inv_num.invoice_month, invoice_month);
		memset(state->port_info.upload_inv_num.summary_month, 0, sizeof(state->port_info.upload_inv_num.summary_month));
		strcpy(state->port_info.upload_inv_num.summary_month, summary_month);


		if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
		{
			free(state->port_info.upload_inv_num.dmhmzfsbstr);
			state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
		}
		state->port_info.upload_inv_num.dmhmzfsbstr = fphm_crc;

		state->port_info.upload_inv_num.state = 1;
		_lock_un(state->lock);

	}


	result = 0;
End:
	cJSON_Delete(root);
	return result;
}
//��ʱ����˰����Ϣ�߳�
static void timely_tigger_deal_plate(void *arg, int timer)
{
	//int result;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int need_tax_read = 0;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//out("�ն�״̬δ�ϴ��ɹ������з�Ʊͬ��\n");
		return;
	}
	if (stream->frist_up_inv == 0)
	{
		//out("�豸������,��ʱ5����ٶ�ȡ֤������\n");
		stream->frist_up_inv = 1;
		sleep(30);
		return;
	}
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;
		if ((sp_type == DEVICE_AISINO) || (sp_type == DEVICE_NISEC) || (sp_type == DEVICE_MENGBAI))
		{

			if (stream->port[port - 1].port_info.tax_read == 1)
			{
				need_tax_read += 1;
			}
		}
	}
	if (need_tax_read > 3)
	{
		//out("���ڶ���˿����ڶ�ȡ֤�鼰�����Ϣ,ϵͳ�д���˰�����豸��ȡ֤�����ƻ�����Ϣ������,δ��ֹ��ͻ,�ݲ��ϴ�����Ʊ");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		//int i;
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;

		if (state->port_info.with_dev == 0)
		{
			//out("�˿�û���豸�����з�Ʊͬ��\n");
			continue;
		}
		//out("%d�Ŷ˿ڷ�Ʊ�ϴ��ж�\n", state->port);
		if (state->port_info.upload_inv_num.in_used != 0)
		{
			//out("˰���ϴ�ʹ����\n");
			continue;
		}		
		if ((sp_type != DEVICE_AISINO) && (sp_type != DEVICE_NISEC) && (sp_type != DEVICE_MENGBAI) && (sp_type != DEVICE_CNTAX))
		{
			//out("˰�����ʹ���,sp_type = %d\n", sp_type);
			continue;
		}
		if (state->port_info.upload_inv_num.state != 1)
		{
			//out("%d�Ŷ˿��޷�Ʊ��Ҫ�ϴ�\n", sp_type);
			continue;
		}
		if (stream->up_queue_num > 9)
		{
			//out("�ϴ����д���5��,�������ڶ�ȡ������Ϣ�Ķ��д���3��,stream->up_queue_num = %d\n", stream->up_queue_num);
			continue;
		}
		state->port_info.upload_inv_num.in_used = 1;
		//out("%d�Ŷ˿���Ҫִ�з�Ʊ�ϴ�\n",state->port);
		if(_queue_add("up_m_server", deal_plate_upload_m_server_queue, state, 10) > 0)
		{
			stream->up_queue_num += 1;
		}
		continue;
	}
	return ;
}
static int trun_month_sum_to_plate_info(struct _inv_sum_data *now_month_sum, struct _plate_infos *plate_infos)
{
	int sum_cout;
	uint8 mem_i;
	for (sum_cout = 0; sum_cout < sizeof(now_month_sum->type_sum) / sizeof(now_month_sum->type_sum[0]); sum_cout++)
	{
		if (sum_cout == 0)
			mem_i = 0;
		else if (sum_cout == 1)
			mem_i = 3;
		else if (sum_cout == 2)
			mem_i = 6;
		else if (sum_cout == 3)
			mem_i = 7;
		else
			return -1;
		if (plate_infos->invoice_type_infos[mem_i].state == 1)
		{
			//out("%03dƱ�֣�ʵ�����۽��%4.2Lf,ʵ������˰��%4.2Lf,�ڳ�������Ϊ%d\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum->type_sum[sum_cout].sjxsje, now_month_sum->type_sum[sum_cout].sjxsse, now_month_sum->type_sum[sum_cout].qckcfs);
			memset(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, 0, sizeof(struct _inv_type_sum_data));
			memcpy(&plate_infos->invoice_type_infos[mem_i].monitor_info.type_sum, &now_month_sum->type_sum[sum_cout], sizeof(struct _inv_type_sum_data));
		}
	}
	return 0;
}
static void deal_plate_upload_m_server_queue(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m * stream;
	stream = ty_usb_m_id;
	state = arg;
	int result;
	char start_time[50] = { 0 };
	char end_time[50] = { 0 };
	int sp_type = state->port_info.usb_app;

	state->port_info.queue_state.use_flag = 1;
	memset(state->port_info.queue_state.start_time,0x00,sizeof(state->port_info.queue_state.start_time));
	timer_read_asc(state->port_info.queue_state.start_time);
	memset(state->port_info.queue_state.summary_month,0x00,sizeof(state->port_info.queue_state.summary_month));
	strcpy(state->port_info.queue_state.summary_month,state->port_info.upload_inv_num.invoice_month);
	//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,Ҫ��ͬ��M����Ʊ����,��ǰ�̶߳���������%d\r\n", state->port_info.port_str, stream->up_queue_num);
	if (state->port_info.upload_inv_num.state == 1)
	{
		
		//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,���ѯ%s�·�Ʊͳ��\n", state->port_info.port_str, state->port_info.upload_inv_num.invoice_month);
		if (port_lock(state, 0, 6, DF_PORT_USED_FPCX_SCMB) != 0)
		{
			//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,ͬ����Ʊ����˰��ʧ��,˰������ִ�У�%s\n", state->port_info.port_str, state->port_info.used_info);
			
			stream->up_queue_num = stream->up_queue_num - 1;
			sleep(5);
			state->port_info.upload_inv_num.in_used = 0;
			state->port_info.queue_state.use_flag = 2;
			memset(state->port_info.queue_state.end_time,0x00,sizeof(state->port_info.queue_state.end_time));
			timer_read_asc(state->port_info.queue_state.end_time);
			//out("deal_plate_upload_m_server_queue �ϴ���������%d\n", stream->up_queue_num);
			return ;
		}
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%d�Ŷ˿�,%s,��ʼͬ��M����Ʊ����\r\n", state->port,state->port_info.port_str);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		state->port_info.m_inv_state = 1;
		timer_read_asc(start_time); 
		struct _inv_sum_data now_month_sum;
		memset(&now_month_sum, 0, sizeof(struct _inv_sum_data));

		if (sp_type == DEVICE_AISINO)
		{
#ifdef RELEASE_SO
			result = so_aisino_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_aisino_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif
		}
		else if (sp_type == DEVICE_NISEC)
		{
#ifdef RELEASE_SO
			result = so_nisec_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_nisec_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif					
		}
		else if (sp_type == DEVICE_CNTAX)
		{
#ifdef RELEASE_SO
			result = so_cntax_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_cntax_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif					
		}
		else if (sp_type == DEVICE_MENGBAI)
		{
#ifdef RELEASE_SO
			result = so_mb_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_mengbai_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif
		}
		else
		{
			result = 0;
		}
		//strcpy(state->port_info.queue_state.now_month,now_month_sum.type_sum.);
		//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��ѯ���ݽ���,�����ͷŶ˿�ռ��\r\n", state->port_info.port_str);
		port_unlock(state);
		timer_read_asc(end_time);
		

		
		_lock_set(state->lock);
		state->need_getinv = 0;//1�������·�Ʊ����   2��������ʷ�·�Ʊ���ݣ���һ�ι̶�Ϊ������,�����½��������޲�ζ���,��ִ����ʷ�·�Ʊ����

		state->port_info.upload_inv_num.state = 0;
		if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
		{
			free(state->port_info.upload_inv_num.dmhmzfsbstr);
			state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
		}

		if (result < 0)//���жϷ�ʽ�����ϴ�
		{
			state->port_info.m_inv_state = -1;
			logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�����ϴ�M��������г����쳣,�����������ͬ�����ش�\r\n", state->port_info.port_str);
			state->need_getinv = 1;
		}
		else
		{
			//out("�����ݱ������ڴ���\n");
			trun_month_sum_to_plate_info(&now_month_sum, &state->port_info.plate_infos);

			state->port_info.m_inv_state = 2;
			logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,ͬ����ʼʱ�䣺%s,ͬ������ʱ�䣺%s\r\n", state->port_info.port_str, start_time, end_time);
		}
		_lock_un(state->lock);
	}
	state->port_info.upload_inv_num.in_used = 0;

	stream->up_queue_num = stream->up_queue_num -1;
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
	{
		logout(INFO, "SYSTEM", "�ϴ�M����", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
		restart_plate_power(state, (uint8 *)state->port_info.sn, state->port, 1);
	}

	//fp_sync_report_json_data((void*)stream,(char*)(&state->port_info.sn),state->port_info.upload_inv_num.summary_month);

	//out("�ϴ�����,�ϴ���������%d\n", stream->up_queue_num);
	state->port_info.queue_state.use_flag = 0;
	memset(state->port_info.queue_state.end_time,0x00,sizeof(state->port_info.queue_state.end_time));
	timer_read_asc(state->port_info.queue_state.end_time);
	return;
}
static int mqtt_pub_hook(void *arg, char *month, char *s_data,int inv_sum)
{
	struct _ty_usb_m  *stream;
	int result;
	struct _port_state   *state;
	state = arg;
	stream = ty_usb_m_id;

	char cmd[100] = {0};
	char source_topic[50] = { 0 };
	char dest_topic[50] = { 0 };
	char random[50] = { 0 };
	char use_compress[3] = { 0 };
	char compress_type[10] = { 0 };
	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fptbxt_%s", stream->ter_id);
	if (inv_sum == DF_UPLOAD_INV)
	{
		sprintf(cmd, "zdsbfpxx");
		sprintf(use_compress, "1");
		sprintf(dest_topic, "mb_invoice_data_upload");
		sprintf(compress_type,"zip");
	}
	else
	{
		sprintf(cmd, "zdsbhzxx");
		sprintf(use_compress, "0");
		sprintf(dest_topic, "mb_summary_data_upload");
		sprintf(compress_type, "base64");
	}
		
	get_radom_serial_number(cmd, random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", cmd);
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", (char *)state->port_info.plate_infos.plate_basic_info.plate_num);
	cJSON_AddStringToObject(data_layer, "number", (char *)state->port_info.plate_infos.plate_basic_info.ca_number);
	cJSON_AddStringToObject(data_layer, "month", (char *)month);
	cJSON_AddStringToObject(data_layer, "use_compress", use_compress);
	cJSON_AddStringToObject(data_layer, "compress_type", compress_type);
	if (s_data != NULL)
		cJSON_AddStringToObject(data_layer, "compress_data", (char *)s_data);
	else
		cJSON_AddStringToObject(data_layer, "compress_data","");

	char *g_buf = cJSON_PrintUnformatted(root);
	if (inv_sum == DF_UPLOAD_INV)
	{
		out("�ϱ���Ʊ���������ɣ����ȣ�%d�ֽ�\n", strlen(g_buf));
		//printf("pub data : %s\n", g_buf);
	}
	else
	{
		out("�ϱ��������������ɣ����ȣ�%d�ֽ�\n", strlen(g_buf));
		//printf("pub data : %s\n", g_buf);
	}
	char g_data[5000] = { 0 };
	
	result = _m_pub_need_answer(random, dest_topic, g_buf, g_data, DF_MQTT_FPTBXT);

	free(g_buf);
	cJSON_Delete(root);
	if (result < 0)
	{
		if (inv_sum == DF_UPLOAD_INV)
			logout(INFO, "system", "report_invoice", "���ϱ���Ʊ�����������ճ�ʱʧ��,��������\r\n");
		else
			logout(INFO, "system", "report_invoice", "���ϱ����ܡ����������ճ�ʱʧ��,��������\r\n");
		state->need_getinv = 1;//���ϴ���Ʊͳ�Ʋ�ѯ
		return -1;
	}



	return 0;
}
#else

#ifdef ALL_SHARE
static void pack_report_ca_info(void *arg)
{

	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf;
	char tmp[20],tmp_s[30];
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


	char errinfo[1024]={0};
#ifdef RELEASE_SO
	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#else
	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#endif
	if (result != 1)
	{
		cJSON_AddStringToObject(dir3, "tax_auth", "0");
	}
	else
	{
		cJSON_AddStringToObject(dir3, "tax_auth", "1");
	}

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->aisino_sup);
	cJSON_AddStringToObject(dir3, "aisino_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->nisec_sup);
	cJSON_AddStringToObject(dir3, "nisec_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->cntax_sup);
	cJSON_AddStringToObject(dir3, "cntax_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->mengbai_sup);
	cJSON_AddStringToObject(dir3, "mengbai_sup", tmp_s);

	cJSON_AddStringToObject(dir3, "aisino_end_time", (char *)state->aisino_end_time);
	cJSON_AddStringToObject(dir3, "nisec_end_time", (char *)state->nisec_end_time);
	cJSON_AddStringToObject(dir3, "cntax_end_time", (char *)state->cntax_end_time);
	cJSON_AddStringToObject(dir3, "mengbai_end_time", (char *)state->mengbai_end_time);

	cJSON_AddStringToObject(dir3, "start_time", (char *)state->start_time);
	cJSON_AddStringToObject(dir3, "end_time", (char *)state->end_time);
	cJSON_AddStringToObject(dir3, "frozen_ago_time", (char *)state->frozen_ago_time);



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
		//printf("step1\n");
		cJSON_AddStringToObject(dir3, "usb_share", "0");
		cJSON_AddStringToObject(dir3, "usb_share_mode", "0");
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
		cJSON_AddStringToObject(dir3, "offline", "0");
		//printf("step2\n");
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
			//out("#####################ca_name :%s###########\n",)
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

		if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(dir3, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "%d", state->port_info.usb_share_mode);
		cJSON_AddStringToObject(dir3, "usb_share_mode", tmp);
		cJSON_AddStringToObject(dir3, "client_id", state->port_info.client);
		cJSON_AddStringToObject(dir3, "offline", "1");
	}



	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	out("g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_equ_ca_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		out("������Ӧ����ʧ��,���Ӵ����������������ʱ��\n");
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
		out("�ϴ���%d�ſڣ�%s��CA��Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	sleep(5);
	//out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	out("����ca�ϱ�����1\n");
	stream->report_p_used = 0;
	return;
}
#endif

//��ʱ�ϱ��˿���Ϣ�߳�
static void timely_tigger_report_port_status(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
	//	out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
		return;
	}
	
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		if (stream->report_ter_flag == 0)
		{
			//	out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");
			return;
		}
		
		state = &stream->port[port - 1];
		if ((state->off_report == 0) && (state->up_report == 0) && (state->need_getinv == 0))
		{
			continue;
		}

		usleep(5000);
		//out("�˿ں�%d,up_report = %d,off_report = %d,need_getinv = %d,now_usb_app = %d,last_usb_app = %d,stream->report_p_used = %d\n", port, state->up_report, state->off_report, state->need_getinv, state->port_info.usb_app, state->port_last_info.usb_app, stream->report_p_used);

				
		if (((state->up_report == 1) && (state->port_info.usb_app == DEVICE_AISINO || state->port_info.usb_app == DEVICE_NISEC || state->port_info.usb_app == DEVICE_MENGBAI || state->port_info.usb_app == DEVICE_MENGBAI2 || state->port_info.usb_app >= 101)) || \
			((state->off_report == 1) && (state->port_last_info.usb_app == DEVICE_AISINO || state->port_last_info.usb_app == DEVICE_NISEC || state->port_last_info.usb_app == DEVICE_MENGBAI || state->port_last_info.usb_app == DEVICE_MENGBAI2 || state->port_last_info.usb_app >= 101)))
		{
			if (stream->report_p_used != 0)
			{
				//out("�˿�״̬�ϴ���\n");
				continue;
			}
			stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			//out("��%d�Ŷ˿ڴ���һ��˰�̸��»������ϱ�\n", state->port);
			_queue_add("0", pack_report_plate_info, state, 0);
			continue;

		}
		else if ((state->off_report == 1) && (state->port_last_info.usb_app == 0 || state->port_last_info.usb_app == 5))
		{
			if (stream->report_p_used != 0)
			{
				//out("�˿�״̬�ϴ���\n");
				continue;
			}
			stream->report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			//out("����һ������USB���»������ϱ�\n");
			_queue_add("0", pack_report_other_info, state, 0);
			continue;
		}
#ifdef ALL_SHARE
		else if (state->up_report == 1 || state->off_report == 1)
		{
			if (stream->report_p_used != 0)
			{
				//out("�˿�״̬�ϴ��У�%d\n", port);
				continue;
			}
			out("����һ��[%d]�˿�״̬���»������ϱ�--\n", port);
			if (stream->port[port - 1].port_info.usb_app == 4)
			{
				//state->up_report = 0;
				stream->report_p_used = 1;
				_queue_add("0", pack_report_ca_info, state, 0);
			}
			else
			{
				stream->report_p_used = 1;
				_queue_add("0", pack_report_other_info, state, 0);//����usb�˿���Ϣ�ϱ�
			}
			continue;

		}
#endif
		else if (state->need_getinv == 1 &&
			(state->port_info.usb_app == DEVICE_AISINO || 
			state->port_info.usb_app == DEVICE_NISEC || 
			state->port_info.usb_app == DEVICE_MENGBAI || 
			state->port_info.usb_app == DEVICE_CNTAX))
		{
			if (stream->report_i_used == 1)
			{
				//out("�˿�״̬�ϴ���\n");
				continue;
			}
			stream->report_i_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			//out("����һ�λ�ȡ˰�����ϴ���Ʊ������ѯ,״̬��%d,1Ϊ��ǰ��,2Ϊ��ʷ��\n", state->need_getinv);
			_queue_add("0", pack_request_upload_inv_num, state, 0);
			continue;
		}
		
	}

}

static void pack_report_plate_info(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m  *stream;
	char *g_buf=NULL;
	//char tmp[20];
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
		//out("�ϴ���%d�ſڣ�%s��˰����Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	
	//out("�����������\n");
	free(g_buf);

End:
	stream->report_p_used = 0;
	return;
}

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

	}
	else
	{
		//�����ϱ�
		cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(dir3, "offline", "1");
		fun_get_port_data_json(dir3, state->port,1);

	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	//out("����һ��˰����Ϣ�ϱ�,�˿ںţ�%d\n",state->port);
	//out("g_buf = %s\n", *g_buf);
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
	//out("����һ������USB�ϱ�,�˿ںţ�%d\n",state->port);
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		//out("state->last_act = %d\n", state->last_act);
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
	}



	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	out("g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	//char g_data[5000] = { 0 };
	//result = _m_pub_need_answer(random, "mb_equ_other_status_up", g_buf, g_data);
	result = _m_pub_no_answer("mb_equ_other_status_up", g_buf);
	if (result < 0)
	{
		out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", 5);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	//else if (result == 0)
	//{
	//	
	//	out("������Ӧ����ʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", stream->report_time_s);
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
		//out("�ϴ���%d�ſڡ�����USB��Ϣ������Ӧ����ɹ�\n\n\n", state->port);
	}
	sleep(5);
	//out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_p_used = 0;
	return;
}


static void pack_request_upload_inv_num(void *arg)
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

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("xscfpcx", random);

	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "xscfpcx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);

	if (state->port == 0)
	{
		logout(INFO, "system", "M�����ѯͬ��", "%s,�˿ں���ֵ�쳣\r\n", state->port_info.port_str);
		cJSON_Delete(json);
		goto End;
	}

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(dir3, "port_num", tmp);
	//�����ϱ�
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
	cJSON_AddStringToObject(dir3, "usb_app", tmp);
	cJSON_AddStringToObject(dir3, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(dir3, "number", (char *)state->port_info.ca_serial);
	cJSON_AddStringToObject(dir3, "plate_num", (char *)state->port_info.sn);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.extension);
	cJSON_AddStringToObject(dir3, "plate_extension", tmp);
	cJSON_AddStringToObject(dir3, "current_month", "0");//�汾�޸�,�������ֶν�������

	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[10000] = { 0 };
	result = _m_pub_need_answer(random, "mb_inv_up_get", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		//out("g_buf = %s\n", g_buf);
		logout(INFO, "SYSTEM", "M�����ѯͬ��", "%s,��ѯ˰�̷�Ʊͬ����Ϣ,��������Ӧ��ʱ\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		state->need_getinv = 0;
		logout(INFO, "SYSTEM", "M�����ѯͬ��", "%s,��ѯ˰�̷�Ʊͬ����Ϣ,������Ӧ����ʧ��\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else
	{
		state->need_getinv = 0;
		//out("������Ӧ�����ѯ�·�Ʊ�����ɹ�,Ӧ��������Ϊ%s\n\n\n", g_data);
		analysis_get_inv_num_response(state,g_data);
	}
	//sleep(5);
	//out("�����������\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_i_used = 0;
	return;
}

static int analysis_get_inv_num_response(struct _port_state   *state, char *json_data)
{
	int result = -1;
	int size;
	int fplx_size;
	int fplx_cout;
	int i;
	char now_num[50] = {0};
	char month[20] = { 0 };
	char summary_month[20] = {0};
	char month_data[7500] = {0};
	char plate_num[20] = { 0 };
	char fpzls[10] = {0};
	int fpzl;
	int mem_i;
	cJSON *root = cJSON_Parse(json_data);
	if (!root)
	{
		out("1Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	cJSON *object_data = cJSON_GetObjectItem(root, "data");
	if (object_data == NULL)
	{
		out("2Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	size = cJSON_GetArraySize(object_data);
	if (size > 1)
	{
		goto End;
	}
	cJSON *arrayItem, *item;
	for (i = 0; i < size; i++)
	{
		//int time_now;
		arrayItem = cJSON_GetArrayItem(object_data, i);
		if (arrayItem == NULL)
		{
			out("3Error before: [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		item = cJSON_GetObjectItem(arrayItem, "plate_num");
		memset(plate_num, 0, sizeof(plate_num));
		if (get_json_value_can_not_null(item, plate_num, 0, 20) < 0) {
			goto End;
		}
		//out("��ȡ�����·�Ϊ%s\n", state->port_info.upload_inv_num[i].month);
		if (strcmp(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num) != 0)
		{
			out("��ȡ�����̺�%s,�뵱ǰ�̺ţ�%s��һ��\n", plate_num, state->port_info.plate_infos.plate_basic_info.plate_num);
			goto End;
		}
		
		item = cJSON_GetObjectItem(arrayItem, "year_month");
		memset(month,0,sizeof(month));
		if (get_json_value_can_not_null(item, month, 0, 10) < 0) {
			goto End;
		}

		item = cJSON_GetObjectItem(arrayItem, "now_num");
		memset(now_num, 0, sizeof(now_num));
		if (get_json_value_can_not_null(item, now_num, 0, 10) < 0) {
			goto End;
		}
		//out("��ȡ�����·�Ϊ%s\n", month);

		item = cJSON_GetObjectItem(arrayItem, "dmhmzfsbstr");
		memset(month_data, 0, sizeof(month_data));
		get_json_value_can_not_null(item, month_data, 0, 7500);
		//out("dmhmzfsbstr:%s\n", month_data);

		item = cJSON_GetObjectItem(arrayItem, "summary_month");
		memset(summary_month, 0, sizeof(summary_month));
		get_json_value_can_not_null(item, summary_month, 0, 20);


		cJSON *object_fplx = cJSON_GetObjectItem(arrayItem, "inv_type");
		if (object_fplx == NULL)
		{
			out("object_inv_normal : [%s]\n", cJSON_GetErrorPtr());
			goto End;
		}
		fplx_size = cJSON_GetArraySize(object_fplx);
		if (fplx_size > 4)
		{
			goto End;
		}
		cJSON *array_fplx, *item_fplx;
		for (fplx_cout = 0; fplx_cout < fplx_size; fplx_cout++)
		{
			array_fplx = cJSON_GetArrayItem(object_fplx, fplx_cout);
			if (array_fplx == NULL)
			{
				out("array_fplx before: [%s]\n", cJSON_GetErrorPtr());
				goto End;
			}
			item_fplx = cJSON_GetObjectItem(array_fplx, "fpzls");
			memset(fpzls, 0, sizeof(fpzls));
			if (get_json_value_can_not_null(item_fplx, fpzls, 0, 3) < 0) {
				goto End;
			}

			fpzl = atoi(fpzls);
			switch (fpzl)
			{
			case FPLX_COMMON_ZYFP:
				mem_i = 0;
				break;
			case FPLX_COMMON_PTFP:
				mem_i = 1;
				break;
			case FPLX_COMMON_DZFP:
				mem_i = 2;
				break;
			case FPLX_COMMON_DZZP:
				mem_i = 3;
				break;
			default:
				//out("analysis_get_inv_num_response ��֧�ֵķ�Ʊ���ࣺ%03d\n", fpzl);
				goto End;
			}

			state->port_info.upload_inv_num.upload_inv_type[mem_i].fplx = fpzl;
			
			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpsj, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpsj));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhkpsj");			
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpsj, 0, 20);

			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfsj, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfsj));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhzfsj");
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfsj, 0, 20);

			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpdm, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpdm));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhkpfpdm");
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkpdm, 0, 20);

			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkphm, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkphm));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhkpfphm");
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhkphm, 0, 20);

			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfdm, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfdm));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhzffpdm");
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfdm, 0, 20);

			memset(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfhm, 0, sizeof(state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfhm));
			item_fplx = cJSON_GetObjectItem(array_fplx, "zhzffphm");
			get_json_value_can_null(item_fplx, state->port_info.upload_inv_num.upload_inv_type[mem_i].zhzfhm, 0, 20);

			state->port_info.upload_inv_num.upload_inv_type[mem_i].state = 1;
		}
		
		_lock_set(state->lock);
		memset(state->port_info.upload_inv_num.year_month, 0, sizeof(state->port_info.upload_inv_num.year_month));
		memset(state->port_info.upload_inv_num.summary_month, 0, sizeof(state->port_info.upload_inv_num.summary_month));
		memset(state->port_info.upload_inv_num.plate_num, 0, sizeof(state->port_info.upload_inv_num.plate_num));
		memset(state->port_info.upload_inv_num.year_month_data, 0, sizeof(state->port_info.upload_inv_num.year_month_data));
		strcpy(state->port_info.upload_inv_num.year_month,month);
		strcpy(state->port_info.upload_inv_num.summary_month, summary_month);
		strcpy(state->port_info.upload_inv_num.plate_num, plate_num);
		strcpy(state->port_info.upload_inv_num.year_month_data,month_data);
		//out("��ȡ�����Ϸ�Ʊ����Ϊ%d\n", state->port_info.upload_inv_num[i].cancel_num);
		state->port_info.upload_inv_num.now_num = atoi(now_num);
		state->port_info.upload_inv_num.state = 1;
		state->port_info.upload_inv_num.auto_up_end = 0;
		_lock_un(state->lock);

	}


	result = 0;
End:
	cJSON_Delete(root);
	return result;
}
//��ʱ����˰����Ϣ�߳�
static void timely_tigger_deal_plate(void *arg, int timer)
{
	//int result;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int need_tax_read = 0;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//out("�ն�״̬δ�ϴ��ɹ������з�Ʊͬ��\n");
		return;
	}
	if (stream->frist_up_inv == 0)
	{
		//out("�豸������,��ʱ5����ٶ�ȡ֤������\n");
		stream->frist_up_inv = 1;
		sleep(30);
		return;
	}
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;
		if ((sp_type == DEVICE_AISINO) || (sp_type == DEVICE_NISEC) || (sp_type == DEVICE_MENGBAI))
		{

			if (stream->port[port - 1].port_info.tax_read == 1)
			{
				need_tax_read += 1;
			}
		}
	}
	if (need_tax_read > 3)
	{
		//out("���ڶ���˿����ڶ�ȡ֤�鼰�����Ϣ,ϵͳ�д���˰�����豸��ȡ֤�����ƻ�����Ϣ������,δ��ֹ��ͻ,�ݲ��ϴ�����Ʊ");
		return;
	}
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		//int i;
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;

		if (state->port_info.with_dev == 0)
		{
			//out("�˿�û���豸�����з�Ʊͬ��\n");
			continue;
		}
		//out("%d�Ŷ˿ڷ�Ʊ�ϴ��ж�\n", state->port);
		if (state->port_info.upload_inv_num.in_used != 0)
		{
			//out("˰���ϴ�ʹ����\n");
			continue;
		}		
		if ((sp_type != DEVICE_AISINO) && (sp_type != DEVICE_NISEC) && (sp_type != DEVICE_MENGBAI) && (sp_type != DEVICE_CNTAX))
		{
			//out("˰�����ʹ���,sp_type = %d\n", sp_type);
			continue;
		}
		if ((state->port_info.upload_inv_num.state != 1) && (state->port_info.upload_inv_num.need_fpdmhm_flag != 1))
		{
			//out("%d�Ŷ˿��޷�Ʊ��Ҫ�ϴ�\n", sp_type);
			continue;
		}
		if (stream->up_queue_num > 4)
		{
			//out("�ϴ����д���5��,�������ڶ�ȡ������Ϣ�Ķ��д���3��,stream->up_queue_num = %d\n", stream->up_queue_num);
			continue;
		}
		stream->up_queue_num += 1;
		state->port_info.upload_inv_num.in_used = 1;
		//out("%d�Ŷ˿���Ҫִ�з�Ʊ�ϴ�\n",port);
		_queue_add("up_m_server", deal_plate_upload_m_server_queue, state, 10);
		continue;
	}
	return ;
}
static int trun_month_sum_to_plate_info(struct _inv_sum_data *now_month_sum, struct _plate_infos *plate_infos)
{
	int sum_cout;
	uint8 mem_i;
	for (sum_cout = 0; sum_cout < sizeof(now_month_sum->type_sum) / sizeof(now_month_sum->type_sum[0]); sum_cout++)
	{
		if (sum_cout == 0)
			mem_i = 0;
		else if (sum_cout == 1)
			mem_i = 3;
		else if (sum_cout == 2)
			mem_i = 6;
		else if (sum_cout == 3)
			mem_i = 7;
		else
			return -1;
		if (plate_infos->invoice_type_infos[mem_i].state == 1)
		{
			//out("%03dƱ�֣�ʵ�����۽��%4.2Lf,ʵ������˰��%4.2Lf\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum->type_sum[sum_cout].sjxsje, now_month_sum->type_sum[sum_cout].sjxsse);
			plate_infos->invoice_type_infos[mem_i].monitor_info.sjxsje = now_month_sum->type_sum[sum_cout].sjxsje;
			plate_infos->invoice_type_infos[mem_i].monitor_info.sjxsse = now_month_sum->type_sum[sum_cout].sjxsse;
		}
	}
	return 0;
}
static void deal_plate_upload_m_server_queue(void *arg)
{
	struct _port_state   *state;
	struct _ty_usb_m * stream;
	stream = ty_usb_m_id;
	state = arg;
	int result;
	int i;
	char start_time[50] = { 0 };
	char end_time[50] = { 0 };
	int sp_type = state->port_info.usb_app;


	//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,Ҫ��ͬ��M����Ʊ����,��ǰ�̶߳���������%d\r\n", state->port_info.port_str, stream->up_queue_num);
	if ((state->port_info.upload_inv_num.state == 1) || (state->port_info.upload_inv_num.need_fpdmhm_flag == 1))
	{
		
		//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,���ѯ%s�·�Ʊͳ��\n", state->port_info.port_str, state->port_info.upload_inv_num.year_month);
		if (port_lock(state, 20, 6, DF_PORT_USED_FPCX_SCMB) != 0)
		{
			//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,ͬ����Ʊ����˰��ʧ��,˰������ִ�У�%s\n", state->port_info.port_str, state->port_info.used_info);
			state->port_info.upload_inv_num.in_used = 0;
			stream->up_queue_num = stream->up_queue_num - 1;
			sleep(5);
			//out("deal_plate_upload_m_server_queue �ϴ���������%d\n", stream->up_queue_num);
			return ;
		}
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��ʼͬ��M����Ʊ����\r\n", state->port_info.port_str);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		state->port_info.m_inv_state = 1;
		timer_read_asc(start_time); 
		struct _inv_sum_data now_month_sum;
		memset(&now_month_sum, 0, sizeof(struct _inv_sum_data));

		if (sp_type == DEVICE_AISINO)
		{
#ifdef RELEASE_SO
			result = so_aisino_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_aisino_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif
		}
		else if (sp_type == DEVICE_NISEC)
		{
#ifdef RELEASE_SO
			result = so_nisec_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_nisec_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif					
		}
		else if (sp_type == DEVICE_CNTAX)
		{
#ifdef RELEASE_SO
			result = so_cntax_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_cntax_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif					
		}
		else if (sp_type == DEVICE_MENGBAI)
		{
#ifdef RELEASE_SO
			result = so_mb_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#else
			result = function_mengbai_upload_m_server(&state->h_plate, &state->port_info.inv_query, &state->port_info.upload_inv_num, mqtt_pub_hook, state, &now_month_sum);
#endif
		}
		else
		{
			result = 0;
		}
		//logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��ѯ���ݽ���,�����ͷŶ˿�ռ��\r\n", state->port_info.port_str);
		port_unlock(state);
		timer_read_asc(end_time);
		

		
		_lock_set(state->lock);
		state->need_getinv = 0;//1�������·�Ʊ����   2��������ʷ�·�Ʊ���ݣ���һ�ι̶�Ϊ������,�����½��������޲�ζ���,��ִ����ʷ�·�Ʊ����

		state->port_info.upload_inv_num.state = 0;
		state->port_info.upload_inv_num.need_fpdmhm_flag = 0;
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.upload_inv_type) / sizeof(state->port_info.upload_inv_num.upload_inv_type[0]); i++)
			memset(&state->port_info.upload_inv_num.upload_inv_type[i], 0, sizeof(struct _upload_inv_type));
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]); i++)
			memset(&state->port_info.upload_inv_num.need_upload_dmhm[i], 0, sizeof(struct _need_upload_dmhm));

		if (result < 0)//���жϷ�ʽ�����ϴ�
		{
			state->port_info.m_inv_state = -1;
			logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�����ϴ�M��������г����쳣,�����������ͬ�����ش�\r\n", state->port_info.port_str);
			state->need_getinv = 1;
		}
		else
		{

			//out("�����ݱ������ڴ���\n");
			trun_month_sum_to_plate_info(&now_month_sum, &state->port_info.plate_infos);

			state->port_info.m_inv_state = 2;
			logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,ͬ����ʼʱ�䣺%s,ͬ������ʱ�䣺%s\r\n", state->port_info.port_str, start_time, end_time);
			state->port_info.upload_inv_num.auto_up_end = 1;
		}
		_lock_un(state->lock);
	}
	state->port_info.upload_inv_num.in_used = 0;

	stream->up_queue_num = stream->up_queue_num -1;
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
	{
		logout(INFO, "SYSTEM", "�ϴ�M����", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
		restart_plate_power(state, (uint8 *)state->port_info.sn, state->port, 1);
	}
	//out("�ϴ�����,�ϴ���������%d\n", stream->up_queue_num);
	return;
}
static int mqtt_pub_hook(unsigned char *s_data, int complete, int now_count, void *arg, char *plate_num, unsigned char *month, char *statistics,int complete_err)
{
	struct _ty_usb_m * stream;
	stream = ty_usb_m_id;
	//struct _message_line *message_line;
	struct _port_state   *state;
	//stream = fd_stream;
	state = arg;
	int i;
	//out("��ƱJSON complete = %d,nowcount = %d\n", complete, now_count);
	char str_tmp[10] = { 0 };
	int now_num;
	int invlen;
	long index = 0;
	char source_topic[50] = { 0 };
	int result;
	char random[50] = { 0 };
	char cbqkzt[3] = { 0 };
	char zxbsrq[20] = { 0 };
	int types_num = 0;
	//out("����״̬��Ϣ\n");
	update_plate_info_cbqkzt(state);
	//out("����״̬��Ϣ����\n");
	for (types_num = 0; types_num < DF_MAX_INV_TYPE; types_num++)
	{
		if (state->port_info.plate_infos.invoice_type_infos[types_num].monitor_info.state == 0)
			continue;
		if (strlen(zxbsrq) == 0)
		{
			memset(zxbsrq, 0, sizeof(zxbsrq));
			memcpy(zxbsrq, (char *)state->port_info.plate_infos.invoice_type_infos[types_num].monitor_info.zxbsrq,6);
		}
		else
		{
			if (strcmp(zxbsrq, (char *)state->port_info.plate_infos.invoice_type_infos[types_num].monitor_info.zxbsrq)>0)
			{
				memset(zxbsrq, 0, sizeof(zxbsrq));
				memcpy(zxbsrq, (char *)state->port_info.plate_infos.invoice_type_infos[types_num].monitor_info.zxbsrq,6);
			}
		}
	}

	if (strlen(zxbsrq) == 0)
	{
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ����ͬ��M����,���±�˰����Ϊ��,˰�̶˿ڿ��ܷ����仯,ֹͣ��Ʊͬ��\r\n", state->port_info.port_str);
	}

	if (strcmp((char *)month, zxbsrq) == 0)
	{
		sprintf(cbqkzt,"1");
	}
	else if (strcmp((char *)month, zxbsrq) > 0)
	{
		sprintf(cbqkzt, "0");
	}
	else
	{
		sprintf(cbqkzt, "2");
	}
	get_radom_serial_number("zdsbfpxx", random);
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);

	if (strlen((char *)month) == 0)
	{
		logout(ERROR, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�ϴ�M�����������,month�ֶ��쳣1\r\n", state->port_info.port_str);
	}

	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON *data_array;		 //���ʹ��
	cJSON_AddItemToObject(json, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REQUEST);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbfpxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(json, "data", data_array = cJSON_CreateArray());

	if (complete == 1)
	{
		cJSON *data_layer;
		cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());
		cJSON_AddStringToObject(data_layer, "month", (char *)month);
		//cJSON_AddStringToObject(data_layer, "total_num", "0");
		cJSON_AddStringToObject(data_layer, "upload_type", "0");//��ѯ�ϴ�
		memset(str_tmp, 0, sizeof(str_tmp));
		sprintf(str_tmp, "%d", now_count);
		cJSON_AddStringToObject(data_layer, "now_num", str_tmp);
		cJSON_AddStringToObject(data_layer, "complete_flag", "1");
		cJSON_AddStringToObject(data_layer, "plate_num", plate_num);
		cJSON_AddStringToObject(data_layer, "plate_start_date", (char *)state->port_info.plate_infos.plate_tax_info.startup_date);
		memset(str_tmp, 0, sizeof(str_tmp));
		sprintf(str_tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(data_layer, "usb_app", str_tmp);		
		cJSON_AddStringToObject(data_layer, "cbqkzt", cbqkzt);
		cJSON_AddStringToObject(data_layer, "zxbsrq", zxbsrq);
		if (s_data != NULL)
			cJSON_AddStringToObject(data_layer, "invoice_data", (char *)s_data);
		else
			cJSON_AddStringToObject(data_layer, "invoice_data", "");
		if (statistics != NULL)
			cJSON_AddStringToObject(data_layer, "statistics_data", statistics);
		else
			cJSON_AddStringToObject(data_layer, "statistics_data", "");
		memset(str_tmp, 0, sizeof(str_tmp));
		sprintf(str_tmp, "%d", complete_err);
		cJSON_AddStringToObject(data_layer, "complete_err", str_tmp);	
	}
	else
	{
		for (i = 0; i < now_count; i++)
		{
			cJSON *data_layer;
			char *inv_data = NULL;
			cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

			cJSON_AddStringToObject(data_layer, "month", (char *)month);
			if (strlen((char *)month) == 0)
			{
				logout(ERROR, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�ϴ�M�����������,month�ֶ��쳣2\r\n", state->port_info.port_str);
			}
			cJSON_AddStringToObject(data_layer, "upload_type", "0");//��ѯ�ϴ�
		
			//cJSON_AddStringToObject(data_layer, "total_num", "0");//���ֶ�����Ч

			now_num = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("��ǰ��Ʊ���%d\n",now_num);
			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", now_num);
			cJSON_AddStringToObject(data_layer, "now_num", str_tmp);
			if (strlen((char *)month) == 0)
			{
				logout(ERROR, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�ϴ�M�����������,month�ֶ��쳣3\r\n", state->port_info.port_str);
			}

			if ((complete == 1) && (i == now_count-1))
				cJSON_AddStringToObject(data_layer, "complete_flag", "1");
			else
				cJSON_AddStringToObject(data_layer, "complete_flag", "0");

			cJSON_AddStringToObject(data_layer, "plate_num", plate_num);
			cJSON_AddStringToObject(data_layer, "plate_start_date", (char *)state->port_info.plate_infos.plate_tax_info.startup_date);
			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", state->port_info.usb_app);
			cJSON_AddStringToObject(data_layer, "usb_app", str_tmp);
			cJSON_AddStringToObject(data_layer, "cbqkzt", cbqkzt);
			cJSON_AddStringToObject(data_layer, "zxbsrq", zxbsrq);
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
			cJSON_AddStringToObject(data_layer, "statistics_data", "");
			cJSON_AddStringToObject(data_layer, "complete_err", "0");
			free(inv_data);

			//out("�鷢ƱӦ������\n");
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("mqtt  = %s\n", g_buf);
	cJSON_Delete(json);

	if ((strcmp(plate_num, (char *)state->port_info.sn) != 0) || (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port)<0))
	{
		free(g_buf);
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ����ͬ��M����,����˰�̶˿ڷ����仯,ֹͣ��Ʊͬ��\r\n", state->port_info.port_str);
		return -1;
	}

	_lock_set(state->lock);//���ж��߼��ڴ˴����岻��,������mqtt���ͺܿ죬����ռ��̫��ʱ�䣬���ж���������С������������100kʱִ���ж�
	if (state->h_plate.hDev != NULL)
	{
		if ((state->h_plate.hDev->bBreakAllIO == 1) && (strlen(g_buf)>100 * 1024))
		{
			_lock_un(state->lock);
			free(g_buf);
			logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ����ͬ��M����,�ⲿҪ���ж�,����Ϊ�ӿڵ��û�˰�̵���,ֹͣ��Ʊͬ��\r\n", state->port_info.port_str);
			return -1;
		}
	}
	_lock_un(state->lock);
	//out("MQTT�ص���������\n");
	//char g_data[40960] = {0};
	char g_data[5000] = { 0 };
	//out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
	result = _m_pub_need_answer(random, "mb_inv_month_upload", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ����ͬ��M����,������Ӧ��ʱ\r\n", state->port_info.port_str);
		out("��Ʊ���ݻص�����ʧ��\n");
	}
	//out("MQTT�ص������������,response len = %d\n", strlen(g_data));
	free(g_buf);

	return result;
}
#endif
//��ʱ�ϱ�����Ʊ
//static void timely_tigger_report_offline(void *arg, int timer)
//{
//	struct _ty_usb_m  *stream;
//	stream = arg;
//	sem_post(&stream->offline_sem);
//}

static int check_stop_upload(int *stop_aisino_up, int *stop_nisec_up,int *stop_cntax_up, int *stop_mengbai_up)
{
	char buf[1024] = {0};
	if (access("/etc/stop_upload", 0) != 0)
	{
		out("�Ƿ�����ϴ������ļ�������\n");
		return 0;
	}
	read_file("/etc/stop_upload",buf,sizeof(buf));


	cJSON *root = cJSON_Parse(buf);
	cJSON *item;
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return 0;
	}
	
	//out("��ȡЭ��汾\n");
	char tmp[10] = {0};
	memset(tmp,0,sizeof(tmp));
	item = cJSON_GetObjectItem(root, "stop_aisino_up");
	if (get_json_value_can_not_null(item, tmp, 1, 3) < 0)
	{
		;
	}
	else
	{
		*stop_aisino_up = atoi(tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	item = cJSON_GetObjectItem(root, "stop_nisec_up");
	if (get_json_value_can_not_null(item, tmp, 1, 3) < 0)
	{
		;
	}
	else
	{
		*stop_nisec_up = atoi(tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	item = cJSON_GetObjectItem(root, "stop_cntax_up");
	if (get_json_value_can_not_null(item, tmp, 1, 3) < 0)
	{
		;
	}
	else
	{
		*stop_cntax_up = atoi(tmp);
	}

	memset(tmp, 0, sizeof(tmp));
	item = cJSON_GetObjectItem(root, "stop_mengbai_up");
	if (get_json_value_can_not_null(item, tmp, 1, 3) < 0)
	{
		;
	}
	else
	{
		*stop_mengbai_up = atoi(tmp);
	}
	out("��˰�̣�%d,˰���̣�%d,ģ���̣�%d\n", *stop_aisino_up, *stop_nisec_up, *stop_mengbai_up);
	cJSON_Delete(root);
	return 0;
}


//�ȴ��ź��ϱ�����Ʊ
static void report_offline_inv_to_server(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int need_tax_read = 0;
	stream = arg;

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;
		if ((sp_type == DEVICE_AISINO) || (sp_type == DEVICE_NISEC) || (sp_type == DEVICE_MENGBAI))
		{

			if (stream->port[port - 1].port_info.tax_read == 1)
			{
				need_tax_read += 1;
			}
		}
	}
	if (need_tax_read > 3)
	{
		//out("���ڶ���˿����ڶ�ȡ֤�鼰�����Ϣ,ϵͳ�д���˰�����豸��ȡ֤�����ƻ�����Ϣ������,δ��ֹ��ͻ,�ݲ��ϴ�����Ʊ");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		//out("��%d�Ŷ˿�������Ʊ��Ҫ�ϴ���%d��\n", port,state->port_info.off_inv_num);
		if (state->port_info.offinv_num_exit <= 0)
			continue;
		if (state->port_info.offinv_stop_upload == 1)
			continue;

		if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 
				"�������߷�Ʊ,��ǰ˰���ڸ߰汾�����ʹ�ù�,���ڽ��汾ʹ�÷���,�������߰汾���ʹ��");
			continue;
		}

		//out("��%d�Ŷ˿�������Ʊ��Ҫ�ϴ� state->port_info.offinv_num_exit = %d\n\n\n", port, state->port_info.offinv_num_exit);
		int sp_type = state->port_info.usb_app;

		if (sp_type == DEVICE_AISINO)
		{
			//out("��ʼ�ϴ���˰�����߷�Ʊ\n");
			if (stream->stop_aisino_up == 1)
			{
				out("����ͣ��˰�̷�Ʊ�ϴ�����\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("��ǰ˰��ʱ��%lld,���˰�̿�Ʊʱ��%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("������˰�����߷�Ʊ�ϴ�����\n");
				_queue_add("0", get_offline_inv_and_upload_aisino, state, 0);
			}
		}
		else if (sp_type == DEVICE_NISEC)
		{
			//out("��ʼ�ϴ�˰�������߷�Ʊ\n");
			if (stream->stop_nisec_up == 1)
			{
				out("����ͣ˰���̷�Ʊ�ϴ�����\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("��ǰ˰��ʱ��%lld,���˰�̿�Ʊʱ��%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("����˰�������߷�Ʊ�ϴ�����\n");
				_queue_add("0", get_offline_inv_and_upload_nisec, state, 0);
			}
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			//out("��ʼ�ϴ�˰�������߷�Ʊ\n");
			if (stream->stop_cntax_up == 1)
			{
				out("����ͣ˰���̷�Ʊ�ϴ�����\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("��ǰ˰��ʱ��%lld,���˰�̿�Ʊʱ��%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue;
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("����˰�������߷�Ʊ�ϴ�����\n");
				_queue_add("0", get_offline_inv_and_upload_cntax, state, 0);
			}
		}
		else if (sp_type == DEVICE_MENGBAI)
		{
			if (stream->stop_mengbai_up == 1)
			{
				out("����ͣģ���̷�Ʊ�ϴ�����\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("��ǰ˰��ʱ��%lld,���˰�̿�Ʊʱ��%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				out("����ģ�������߷�Ʊ�ϴ�����\n");
				_queue_add("0", get_offline_inv_and_upload_mengbai, state, 0);
			}
		}
		else
		{
			state->port_info.offinv_num_exit = 0;
			out("��USB�豸��֧�����߷�Ʊ��Ϣ�ϴ�\n");
			continue;
		}

	}

}

static int update_plate_info_cbqkzt(struct _port_state   *state)
{

	int types_count;
	long time_now;
	long plate_time;
	char time_s[20] = { 0 };
	time_now = get_time_sec();
	//out("time_now =  %ld\n", time_now);
	plate_time = time_now + state->port_info.plate_sys_time;
	//out("plate_time =  %ld\n", plate_time);
	memset(time_s, 0, sizeof(time_s));
	get_date_time_from_second_turn(plate_time, time_s);
	cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);


	for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
	{
		if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
		{
			continue;
		}
		if ((memcmp(time_s, state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(time_s, state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			sprintf((char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt, "0");
		}
		else
		{
			sprintf((char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt, "1");
			if (memcmp(time_s, (char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj, 6) >= 0)
			{
				sprintf((char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt, "1");
			}
			else
			{
				sprintf((char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt, "2");
			}
		}
	}
	return 0;
}


//��ʱ����˰��ͬ����ҵ��Ϣ
//static void timely_tigger_sync_plate_cpy_info(void *arg, int timer)
//{
//	struct _ty_usb_m  *stream;
//	stream = arg;
//	struct _port_state   *state;
//	int port_count;
//	char errinfo[2048];
//	int result;
//	//out("�ж�ʱ�䣬���ʱ����ҹ��Ļ�������ͬ��\n");
//	long time_now = get_time_sec();
//	long plate_now;
//	char plate_time_s[20];
//	char hour_min[10];
//
//	out("2Сʱ����˰��ͬ����ҵ��Ϣ�߳�\n");
//	for (port_count = 1; port_count <= stream->usb_port_numb; port_count++)
//	{
//		state = &stream->port[port_count - 1];
//		if (state->port_info.with_dev == 0)
//		{
//			//out("�˿�û���豸��������ҵ��Ϣͬ��\n");
//			continue;
//		}
//		plate_now = time_now + state->port_info.plate_sys_time;
//		memset(plate_time_s, 0, sizeof(plate_time_s));
//		memset(hour_min, 0, sizeof(hour_min));
//		get_date_time_from_second_turn(plate_now, plate_time_s);
//		out("��ǰ˰�̣�%s,ʱ��Ϊ%s\n", state->port_info.port_str, plate_time_s);
//		memcpy(hour_min, plate_time_s+11,5);
//		//out("��ǰʱ�̣�%s\n", hour_min);
//		if (strcmp(hour_min, "07:00") < 0)
//		{
//			continue;
//		}
//		if (strcmp(hour_min, "20:00") > 0)
//		{
//			continue;
//		}
//		
//		if (state->port_info.usb_app == DEVICE_AISINO)//Ŀǰ��֧�ֽ�˰��
//		{
//			if (port_lock(state, 0, 4, DF_PORT_USED_CSLJ) != 0)
//			{
//				continue;
//			}
//			out("��ʼͬ��˰�̣�%s��ҵ��Ϣ,����˰�̳ɹ�\n", state->port_info.plate_infos.plate_basic_info.plate_num);
//			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
//			state->last_act = 1;
//			memset(errinfo, 0, sizeof(errinfo));
//#ifdef RELEASE_SO
//			result = so_aisino_test_server_connect(&state->h_plate, NULL, errinfo);
//#else
//			result = function_aisino_test_server_connect(&state->h_plate, NULL, errinfo);
//#endif
//			if (result < 0)
//			{
//				out("ͬ��˰�̣�%s��ҵ��Ϣʧ��,errinfo %s\n", state->port_info.plate_infos.plate_basic_info.plate_num);
//			}
//			port_unlock(state);
//			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
//			state->last_act = 1;
//		}
//	}
//}
//��ʱ���������������Ϣ�ش�
static void timely_tigger_auto_cb_jkxxhc(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	stream = arg;
	struct _port_state   *state;
	int port_count;
	out("15���Ӵ��������忨�߳�\n");
	sync();
	system("echo 3 > /proc/sys/vm/drop_caches");
	for (port_count = 1; port_count <= stream->usb_port_numb; port_count++)
	{
		state = &stream->port[port_count - 1];
		int sp_type = state->port_info.usb_app;

		update_plate_info_cbqkzt(state);

		if ((sp_type == DEVICE_AISINO) || (sp_type == DEVICE_NISEC) || (sp_type == DEVICE_MENGBAI) || (sp_type == DEVICE_CNTAX))
		{
			state->port_info.need_chaoshui = 1;
			state->port_info.need_huizong = 1;
			state->port_info.need_fanxiejiankong = 1;
		}

		//�����������Ʊδ�ϴ��򴥷�����Ʊ�ϴ�
		if ((state->port_info.plate_infos.plate_tax_info.off_inv_num != 0) && (state->port_info.offinv_num_exit == 0))//��ֹ��Ʊ����ϴ�ʧ�ܣ������޿�Ʊ��������ζ��������ش�
		{
			out("2Сʱ���������忨�߳�,˳���������߷�Ʊ�ϴ�\n");
			state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
		}

	}

	sem_post(&stream->cb_qk_sem);
}

//�ȴ���Ϣ���г����������Ϣ�ش�
static void auto_cb_jkxxhc_thread(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port_count;
	long time_now;
	long plate_time;
	char time_s[20] = { 0 };
	stream = arg;
	sem_wait(&stream->cb_qk_sem);

	if (access("/etc/stop_cb_hcjk", 0) == 0)
	{
		//out("����ͣ��Ʊ�ϴ�����\n");
		//remove("/etc/stop_cb_hcjk");//41�汾��ʱ����
		return;
	}

	sleep(30);
	//out("��˰�����ܼ���ػش���ؽӿ��ѱ�����\n");
	for (port_count = 1; port_count <= stream->usb_port_numb; port_count++)
	{
		state = &stream->port[port_count - 1];
		if ((state->port_info.need_chaoshui == 0) && (state->port_info.need_huizong == 0) && (state->port_info.need_fanxiejiankong == 0))
		{
			continue;
		}
		int sp_type = state->port_info.usb_app;

		//out("%d�ſ��ж��Ƿ���Ҫ�����忨\n", port_count);
		//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))

		time_now = get_time_sec();
		out("time_now =  %ld,state->port_info.plate_sys_time=%ld\n", time_now,state->port_info.plate_sys_time);
		plate_time = time_now + state->port_info.plate_sys_time;
		//out("plate_time =  %ld\n", plate_time);
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		if (sp_type == DEVICE_AISINO)
		{
			out("%d�ſ��ǽ�˰��,���г����忨����\n", port_count);			
			aisino_get_cb_data_and_upload(state, time_s, plate_time);
		}
		//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
		else if (sp_type == DEVICE_NISEC)
		{
			out("%d�ſ���˰����,���г����忨����\n", port_count);			
			nisec_get_cb_data_and_upload(state, time_s, plate_time);
			//out("��USB�豸Ϊ˰����,�ݲ�֧�ֳ����忨\n");
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("%d�ſ���˰��ukey,���г����忨����\n", port_count);			
			cntax_get_cb_data_and_upload(state, time_s, plate_time);
		}

		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("%d�ſ���ģ����,���г����忨����\n", port_count);			
			mengbai_get_cb_data_and_upload(state, time_s, plate_time);			
		}
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(1, state->port_info.plate_infos.plate_tax_info.next_report_time);

	}
	return;


}


//��Ʊ���߳ɹ�,���¼����Ϣ����
#ifndef DF_OLD_MSERVER
static int make_invoice_ok_update_monitor_status_and_notice_server(struct _port_state   *state, struct _plate_infos *plate_infos)
{
	
	int i;
	//int result;
	_lock_set(state->lock);
	state->port_info.plate_infos.plate_tax_info.off_inv_num = 1;
	state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
	_lock_un(state->lock);
	for (i = 0; i < DF_MAX_INV_TYPE; i++)
	{
		struct _inv_type_sum_data  type_sum;
		memset(&type_sum, 0, sizeof(struct _inv_type_sum_data));
		memcpy(&type_sum, &state->port_info.plate_infos.invoice_type_infos[i].monitor_info.type_sum, sizeof(struct _inv_type_sum_data));

		_lock_set(state->lock);
		memset(&state->port_info.plate_infos.invoice_type_infos[i], 0, sizeof(struct _invoice_type_infos));
		memcpy(&state->port_info.plate_infos.invoice_type_infos[i], &plate_infos->invoice_type_infos[i], sizeof(struct _invoice_type_infos));
		memcpy(&state->port_info.plate_infos.invoice_type_infos[i].monitor_info.type_sum, &type_sum, sizeof(struct _inv_type_sum_data));
		_lock_un(state->lock);
	}
	out("˰�̿�Ʊ��ɣ�����˰��״̬��Ϣ\n");
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	int time = 0;
	for (time = 0; time < 3 * 1000; time++)
	{
		usleep(1000);
		if (state->up_report == 0)
		{
			out("˰��״̬�ϱ��ɹ�\n");
			break;
		}
	}
	if (time == 3 * 1000)
	{
		out("˰��״̬�ϱ���ʱ\n");
	}
	return 0;
}
#else
static int make_invoice_ok_update_monitor_status_and_notice_server(struct _port_state   *state, struct _plate_infos *plate_infos)
{
	
	int i;
	char *g_buf = NULL;
	//int result;
	_lock_set(state->lock);
	state->port_info.plate_infos.plate_tax_info.off_inv_num = 1;
	state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
	_lock_un(state->lock);
	for (i = 0; i < DF_MAX_INV_TYPE; i++)
	{
		long double sjxsje = state->port_info.plate_infos.invoice_type_infos[i].monitor_info.sjxsje;
		long double sjxsse = state->port_info.plate_infos.invoice_type_infos[i].monitor_info.sjxsse;
		_lock_set(state->lock);
		memset(&state->port_info.plate_infos.invoice_type_infos[i], 0, sizeof(struct _invoice_type_infos));
		memcpy(&state->port_info.plate_infos.invoice_type_infos[i], &plate_infos->invoice_type_infos[i], sizeof(struct _invoice_type_infos));
		state->port_info.plate_infos.invoice_type_infos[i].monitor_info.sjxsje = sjxsje;
		state->port_info.plate_infos.invoice_type_infos[i].monitor_info.sjxsse = sjxsse;
		_lock_un(state->lock);
	}
	char random[1024] = {0};
	get_radom_serial_number("zdsbspxx", random);
	pack_report_plate_info_json(&g_buf, 0, random,state);
	_m_pub_no_answer("mb_equ_plate_status_up", g_buf);
	free(g_buf);
	return 0;


}
#endif


//��Ʊ���߳ɹ�,���¼����Ϣ����
static int make_invoice_ok_update_monitor_status(struct _port_state   *state, int read_flag)
{
	//sleep(1);
	//out("Ҫ�����¶�ȡ�����Ϣ\n");
	//state->port_info.tax_read = 1;
	//return 0;

	//int i;
	//int result;
	//unsigned short busnum;
	//unsigned short devnum;

	if (read_flag == 0)
	{
		//out("���ٶ����⿪Ʊ��ɲ�ֱ�Ӷ�ȡ�����Ϣ,�����̴߳���\n");
		state->port_info.tax_read = 1;
		return 0;
	}
	return 0;
}

static int aisino_inv_upload_complete_update_lxzssyje(struct _port_state   *state, struct _offline_upload_result *offline_upload_result)
{
	int mem_i;
#ifdef RELEASE_SO
	so_common_find_inv_type_mem(offline_upload_result->fplxdm, &mem_i);
#else
	function_common_find_inv_type_mem(offline_upload_result->fplxdm, &mem_i);
#endif

	long double scje = strtold((char *)offline_upload_result->hjje, NULL);
	if ((scje <= 0) || (offline_upload_result->zfbz ==1))
	{
		out("��Ʊ�ϴ������������ϲ�������������ʣ�������\n");
		return 0;
	}

	long double lxzssyje = strtold((char *)state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, NULL) + strtold((char *)offline_upload_result->hjje, NULL);

	memset(state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, 0, sizeof(state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje));
	sprintf((char *)state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, "%4.2Lf", lxzssyje);
	return 0;
}

static int aisino_inv_cancel_complete_update_lxzssyje(struct _port_state   *state, struct _cancel_invoice_result *cancel_invoice_result)
{
	int mem_i;
#ifdef RELEASE_SO
	so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
#else
	function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
#endif

	long double zfje = strtold((char *)cancel_invoice_result->hjje, NULL);
	if (zfje <= 0)
	{
		out("��Ʊ���ϣ��������ϲ�������������ʣ�������\n");
		return 0;
	}

	long double lxzssyje = strtold((char *)state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, NULL) + strtold((char *)cancel_invoice_result->hjje, NULL);

	memset(state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, 0, sizeof(state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje));
	sprintf((char *)state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.lxzssyje, "%4.2Lf", lxzssyje);
	return 0;
}


static void get_offline_inv_and_upload_aisino(void *arg)
{
	//int sql_id;
	struct _port_state   *state;
	//struct _plate_infos *plate_infos;
	int result;
	state = arg;
	state->port_info.off_up_state = 1;
	//plate_infos = &state->port_info.plate_infos;
	if (state->port_info.port_used != 0)
	{
		//Ԥ�ж϶˿��Ƿ�ռ�ã�ռ��ֱ���˳�
		state->port_info.off_up_state = 0;
		return;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,���뷢Ʊ�ϴ�����\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = {0};
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("��˰��Ĭ�Ͽ����ȷ,��ִ�����߷�Ʊ�ϴ�����\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}


	out("��ʼ�ϴ�˰�̣�%s������Ʊ,��������˰��\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//ͨ���̺�ȡ����Ʊ����
	for (;;)
	{
		//��ʼ��Ʊ�ϴ��߼�
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("˰������ʹ�����޷�ʹ��,˰������ִ�У�%s\n", state->port_info.used_info);
			goto go_end;//�߳�ǰ���Ѽ��жϣ��˴��˳�����ѭ����������
		}
		out("��ʼ�ϴ�˰�̣�%s������Ʊ,����˰�̳ɹ�\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		struct _offline_upload_result offline_upload_result;
		memset(&offline_upload_result, 0, sizeof(struct _offline_upload_result));


#ifdef RELEASE_SO
		result = so_aisino_inv_upload_server(&state->h_plate, &offline_upload_result);
#else
		result = function_aisino_inv_upload_server(&state->h_plate, &offline_upload_result);
#endif
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_LXSC, REFER_TO_TAX_ADDRESS, result, (char *)offline_upload_result.errinfo);
		if (result < 0)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "���߷�Ʊ�ϴ�", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n",state->port_info.port_str);				
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "֤���ѹ�ʧ�������ϻ�״̬δ֪") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			
			sleep(3);
			out("��%d�Ŷ˿�����Ʊ�ϴ�ʧ��\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "��Ʊ�ϴ������б������ӿ��ж�") == NULL)
			{
				state->port_info.upfailed_count += 1;//ʧ�ܴ�����1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//������
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//û������Ʊ
		{
			state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//����Ʊ����Ϊ0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("��%d�Ŷ˿�˰���в��������߷�Ʊ,�����¼����Ϣֱ���ϴ�״̬\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
		out("%s,�˿ڷ�Ʊ�ϴ��ɹ�����д�ɹ�\n", state->port_info.port_str);

		//out("���˱��ͳɹ������ϴ���M����\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);
		inv_change_notice_upload(state, DF_INV_ACTION_SC, (char *)offline_upload_result.fpdm, (char *)offline_upload_result.fphm, (char *)offline_upload_result.scfpsj, (char *)offline_upload_result.kpsj);
		aisino_inv_upload_complete_update_lxzssyje(state, &offline_upload_result);
		if (offline_upload_result.scfpsj != NULL)
			free(offline_upload_result.scfpsj);


		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("��%d�Ŷ˿ڻ�ȡ�˿�����Ʊ���ϴ�ȫ�����\n", state->port);
	state->port_info.off_up_state = 0;
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	return;
}


static void get_offline_inv_and_upload_nisec(void *arg)
{
	//int sql_id;
	struct _port_state   *state;
	//struct _plate_infos *plate_infos;
	int result;
	state = arg;
	state->port_info.off_up_state = 1;
	//plate_infos = &state->port_info.plate_infos;
	if (state->port_info.port_used != 0)
	{
		//Ԥ�ж϶˿��Ƿ�ռ�ã�ռ��ֱ���˳�
		state->port_info.off_up_state = 0;
		return;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,���뷢Ʊ�ϴ�����\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("˰����Ĭ�Ͽ����ȷ,��ִ�����߷�Ʊ�ϴ�����\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,��������˰��\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//ͨ���̺�ȡ����Ʊ����
	for (;;)
	{
		//��ʼ��Ʊ�ϴ��߼�
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("˰������ʹ�����޷�ʹ��,˰������ִ�У�%s\n", state->port_info.used_info);
			goto go_end;//�߳�ǰ���Ѽ��жϣ��˴��˳�����ѭ����������
		}
		out("��ʼ�ϴ�˰�̣�%s������Ʊ,����˰�̳ɹ�\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		struct _offline_upload_result offline_upload_result;
		memset(&offline_upload_result, 0, sizeof(struct _offline_upload_result));


#ifdef RELEASE_SO
		result = so_nisec_inv_upload_server(&state->h_plate, &offline_upload_result);
#else
		result = function_nisec_inv_upload_server(&state->h_plate, &offline_upload_result);
#endif
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_LXSC, REFER_TO_TAX_ADDRESS, result, (char *)offline_upload_result.errinfo);
		if (result < 0)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "���߷�Ʊ�ϴ�", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n",state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "֤���ѹ�ʧ�������ϻ�״̬δ֪") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			sleep(3);
			out("��%d�Ŷ˿�����Ʊ�ϴ�ʧ��\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "��Ʊ�ϴ������б������ӿ��ж�") == NULL)
			{
				state->port_info.upfailed_count += 1;//ʧ�ܴ�����1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//������
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//û������Ʊ
		{
			state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//����Ʊ����Ϊ0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("��%d�Ŷ˿�˰���в��������߷�Ʊ,�����¼����Ϣֱ���ϴ�״̬\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
		out("��%d�Ŷ˿ڷ�Ʊ�ϴ��ɹ�����д�ɹ�\n", state->port);

		out("���˱��ͳɹ������ϴ���M����\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);
		inv_change_notice_upload(state, DF_INV_ACTION_SC, (char *)offline_upload_result.fpdm, (char *)offline_upload_result.fphm, (char *)offline_upload_result.scfpsj, (char *)offline_upload_result.kpsj);

		if (offline_upload_result.scfpsj != NULL)
			free(offline_upload_result.scfpsj);



		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("��%d�Ŷ˿ڻ�ȡ�˿�����Ʊ���ϴ�ȫ�����\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	return;
}

static void get_offline_inv_and_upload_cntax(void *arg)
{
	//int sql_id;
	struct _port_state   *state;
	//struct _plate_infos *plate_infos;
	int result;
	state = arg;
	state->port_info.off_up_state = 1;
	//plate_infos = &state->port_info.plate_infos;
	if (state->port_info.port_used != 0)
	{
		//Ԥ�ж϶˿��Ƿ�ռ�ã�ռ��ֱ���˳�
		state->port_info.off_up_state = 0;
		return;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,���뷢Ʊ�ϴ�����\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("˰��ukeyĬ�Ͽ����ȷ,��ִ�����߷�Ʊ�ϴ�����\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,��������˰��\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//ͨ���̺�ȡ����Ʊ����
	for (;;)
	{
		//��ʼ��Ʊ�ϴ��߼�
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("˰������ʹ�����޷�ʹ��,˰������ִ�У�%s\n", state->port_info.used_info);
			goto go_end;//�߳�ǰ���Ѽ��жϣ��˴��˳�����ѭ����������
		}
		out("��ʼ�ϴ�˰�̣�%s������Ʊ,����˰�̳ɹ�\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		struct _offline_upload_result offline_upload_result;
		memset(&offline_upload_result, 0, sizeof(struct _offline_upload_result));


#ifdef RELEASE_SO
		result = so_cntax_inv_upload_server(&state->h_plate, &offline_upload_result);
#else
		result = function_cntax_inv_upload_server(&state->h_plate, &offline_upload_result);
#endif
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_LXSC, REFER_TO_TAX_ADDRESS, result, (char *)offline_upload_result.errinfo);
		if (result < 0)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "���߷�Ʊ�ϴ�", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "֤���ѹ�ʧ�������ϻ�״̬δ֪") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			sleep(3);
			out("��%d�Ŷ˿�����Ʊ�ϴ�ʧ��\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "��Ʊ�ϴ������б������ӿ��ж�") == NULL)
			{
				state->port_info.upfailed_count += 1;//ʧ�ܴ�����1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//������
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//û������Ʊ
		{
			state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//����Ʊ����Ϊ0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("��%d�Ŷ˿�˰���в��������߷�Ʊ,�����¼����Ϣֱ���ϴ�״̬\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
		out("��%d�Ŷ˿ڷ�Ʊ�ϴ��ɹ�����д�ɹ�\n", state->port);

		out("���˱��ͳɹ������ϴ���M����\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);
		inv_change_notice_upload(state, DF_INV_ACTION_SC, (char *)offline_upload_result.fpdm, (char *)offline_upload_result.fphm, (char *)offline_upload_result.scfpsj, (char *)offline_upload_result.kpsj);

		if (offline_upload_result.scfpsj != NULL)
			free(offline_upload_result.scfpsj);



		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("��%d�Ŷ˿ڻ�ȡ�˿�����Ʊ���ϴ�ȫ�����\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	return;
}

static void get_offline_inv_and_upload_mengbai(void *arg)
{
	//int sql_id;
	struct _port_state   *state;
	//struct _plate_infos *plate_infos;
	int result;
	state = arg;
	state->port_info.off_up_state = 1;
	//plate_infos = &state->port_info.plate_infos;
	if (state->port_info.port_used != 0)
	{
		//Ԥ�ж϶˿��Ƿ�ռ�ã�ռ��ֱ���˳�
		state->port_info.off_up_state = 0;
		return;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,���뷢Ʊ�ϴ�����\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("ģ����Ĭ�Ͽ����ȷ,��ִ�����߷�Ʊ�ϴ�����\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("��ʼ�ϴ�˰�̣�%s������Ʊ,��������˰��\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//ͨ���̺�ȡ����Ʊ����
	for (;;)
	{
		//��ʼ��Ʊ�ϴ��߼�
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("˰������ʹ�����޷�ʹ��,˰������ִ�У�%s\n", state->port_info.used_info);
			goto go_end;//�߳�ǰ���Ѽ��жϣ��˴��˳�����ѭ����������
		}
		out("��ʼ�ϴ�˰�̣�%s������Ʊ,����˰�̳ɹ�\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		struct _offline_upload_result offline_upload_result;
		memset(&offline_upload_result, 0, sizeof(struct _offline_upload_result));


#ifdef RELEASE_SO
		result = so_mb_inv_upload_server(&state->h_plate, &offline_upload_result);
#else
		result = function_mengbai_inv_upload_server(&state->h_plate, &offline_upload_result);
#endif
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_LXSC, REFER_TO_TAX_ADDRESS, result, (char *)offline_upload_result.errinfo);
		if (result < 0)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "���߷�Ʊ�ϴ�", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n",state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			sleep(3);
			out("��%d�Ŷ˿�����Ʊ�ϴ�ʧ��\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "��Ʊ�ϴ������б������ӿ��ж�") == NULL)
			{
				state->port_info.upfailed_count += 1;//ʧ�ܴ�����1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//������
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "�������߷�Ʊδ�ϴ��ɹ�,��Ʊ���룺%s,��Ʊ���룺%s,��Ʊʱ�䣺%s,����ԭ�򣺡�%s��", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//û������Ʊ
		{
			state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//����Ʊ����Ϊ0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("��%d�Ŷ˿�˰���в��������߷�Ʊ,�����¼����Ϣֱ���ϴ�״̬\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//�������Ʊ�ϴ��������
		out("��%d�Ŷ˿ڷ�Ʊ�ϴ��ɹ�����д�ɹ�\n", state->port);

		out("���˱��ͳɹ������ϴ���M����\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);




		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("��%d�Ŷ˿ڻ�ȡ�˿�����Ʊ���ϴ�ȫ�����\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	return;
}


static int timer_read_now_time_add_hour_asc(int hour, uint8 *timer_asc)
{
	long time_int = get_time_sec();
	time_int += 15*60;
	get_date_time_from_second_turn(time_int, (char *)timer_asc);
	return 0;
}

//��˰�̳��������ϴ��������Ϣ��д
static int aisino_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l)
{
	int result;
	int qk_flag = 0;
	int type_count;
	char errinfo[2048] = { 0 };
	char month_start[20] = {0};
	struct _invoice_type_infos *t_info;
	int zp_pp_ok = false;

	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰�����ڶ˿�δ��Ȩ,�ݲ�ִ�г����忨");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��֤������ȷ,�ݲ�ִ�г����忨");
		}
		return 0;
	}
	out("��%d�ſ�:%s�����Խ��г����忨\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����");

			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			out("�忨״̬��%s��\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;

			return 0;
		}
		if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
			if (zp_pp_ok)  //ר��Ʊ��Ϊһ��,ֻ����һ��
				continue;
			zp_pp_ok = true;
		}

		out("��%d�ſڡ���Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���г�˰,���±�˰���ڣ�%s,��Ʊ��ֹ���ڣ�%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��г�˰����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫ����˰\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				logout(INFO, "MQTT", "�����忨", "%d�˿�,��˰�̽��г���˰\r\n", state->port);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰������ʹ����,�޷�ʹ��r\n", state->port);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��ռ����,��ʱδ���г����忨");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_aisino_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_aisino_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_HZSC, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰�̳�˰ʧ��,����ֵΪ%d\r\n", state->port,result);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,��ȡ��������ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("��˰�̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,��˰�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ����˰ʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("��%s����Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���ܼ���д�����Ϣ,˰�̵�ǰʱ�䣺%s,��Ʊ��ֹʱ�䣺%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��л��ܼ���д����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫִ�л��ܼ���д��ض���\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				//logout(INFO, "MQTT", "�����忨", "%d�˿�,ִ�л��ܼ���д��ض���\r\n", state->port);
				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰������ʹ����,�޷�ʹ��\r\n", state->port);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_aisino_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_aisino_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_FXJK, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					//logout(INFO, "MQTT", "�����忨", "%d�˿�,˰���忨ʧ��,����ֵΪ%d\r\n", state->port,result);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,�忨ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("���ܳɹ�,���忨ʧ��\n");						
						out("��˰�̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
						state->port_info.hz_state = 1;
						state->port_info.qk_state = -1;
					}
					else
					{
						state->port_info.hz_state = -1;
						state->port_info.qk_state = -1;
					}
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);


				}
				else
				{
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,�ϱ����ܼ��忨�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ�������Ϣ��дʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("��%d�ſڡ�������ڴ�����,ִ�жϵ����\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,�������忨����������˿ڵ�Դ\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}


	//logout(INFO, "system", "cb_qk","�忨״̬��%s��\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);

	//wang 04 03 
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================��ǰ�˿�%d�����忨����\n",state->port+1);
	return 0;

}
//˰���̳��������ϴ��������Ϣ��д
static int nisec_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l)
{
	int result;
	int qk_flag = 0;
	int type_count;
	char errinfo[2048] = { 0 };
	char month_start[20] = {0};
	struct _invoice_type_infos *t_info;
	//int zp_pp_ok = false;

	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰�����ڶ˿�δ��Ȩ,�ݲ�ִ�г����忨");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��֤������ȷ,�ݲ�ִ�г����忨");
		}

		return 0;
	}
	out("��%d�ſ�:%s�����Խ��г����忨\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����");

			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			out("�忨״̬��%s��\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		//if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
		//	if (zp_pp_ok)  //ר��Ʊ��Ϊһ��,ֻ����һ��
		//		continue;
		//	zp_pp_ok = true;
		//}
		out("��%d�ſڡ���Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���г�˰,���±�˰���ڣ�%s,��Ʊ��ֹ���ڣ�%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��г�˰����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				logout(INFO, "MQTT", "�����忨", "%d�˿�,˰������Ҫ����˰\r\n", state->port);
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫ����˰\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰������ʹ����,�޷�ʹ��\r\n", state->port);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��ռ����,��ʱδ���г����忨");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_nisec_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_nisec_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_HZSC, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰�̳���˰ʧ��,����ֵΪ%d\r\n", state->port,result);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,��ȡ��������ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("˰���̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,��˰�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ����˰ʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("��%s����Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���ܼ���д�����Ϣ,˰�̵�ǰʱ�䣺%s,��Ʊ��ֹʱ�䣺%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��л��ܼ���д����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				//logout(INFO, "MQTT", "�����忨", "%d�˿�,��Ҫִ�л��ܼ���д��ض���\r\n", state->port);
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫִ�л��ܼ���д��ض���\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);

				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					logout(INFO, "MQTT", "�����忨", "%d�˿�,˰������ʹ����,�޷�ʹ��\r\n", state->port);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_nisec_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_nisec_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_FXJK, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					//logout(INFO, "MQTT", "�����忨", "%d�˿�,˰���忨ʧ��,����ֵΪ%d\r\n", state->port,result);
					out("��%d�ſڡ���Ʊ���ͣ�%03d,�忨ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("���ܳɹ�,���忨ʧ��\n");						
						out("˰���̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
						state->port_info.hz_state = 1;
						state->port_info.qk_state = -1;
					}
					else
					{
						state->port_info.hz_state = -1;
						state->port_info.qk_state = -1;
					}
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);

				}
				else
				{
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,�ϱ����ܼ��忨�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ�������Ϣ��дʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("��%d�ſڡ�������ڴ�����,ִ�жϵ����\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,�������忨����������˿ڵ�Դ\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}

	//logout(INFO, "system", "cb_qk","�忨״̬��%s��\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	//wang 04 03 
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;

	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================��ǰ�˿�%d�����忨����\n",state->port+1);
	return 0;

}

//˰��ukey���������ϴ��������Ϣ��д
static int cntax_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l)
{
	int result;
	int qk_flag = 0,need_cb_flag = 0,need_qk_flag = 0;
	int type_count;
	char errinfo[2048] = { 0 };
	char month_start[20] = { 0 };
	char plate_time_s[20] = { 0 };
	struct _invoice_type_infos *t_info;
	//int zp_pp_ok = false;


	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰�����ڶ˿�δ��Ȩ,�ݲ�ִ�г����忨");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��֤������ȷ,�ݲ�ִ�г����忨");
		}
		return 0;
	}
	out("��%d�ſ�:%s�����Խ��г����忨\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];
		if (t_info->state == 0)
			continue;
		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26 && t_info->fplxdm != 28)
		{
			out("���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����");

			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			out("�忨״̬��%s��\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		out("˰��ʱ��Ϊ time = %s\r\n",plate_time);
		out("��%d�ſڡ���Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���г�˰,���±�˰���ڣ�%s,��Ʊ��ֹ���ڣ�%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��г�˰����\n");
			memset(plate_time_s,0x00,sizeof(plate_time_s));
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫ����˰\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				need_cb_flag++;   //��Ҫ������־����
			}
		}
		else
		{
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ����˰ʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("��%s����Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���ܼ���д�����Ϣ,˰�̵�ǰʱ�䣺%s,��Ʊ��ֹʱ�䣺%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��л��ܼ���д����\n");
			memset(plate_time_s,0x00,sizeof(plate_time_s));
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫ�忨\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				need_qk_flag++;    //��Ҫ�忨��־����
			}
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ�������Ϣ��дʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
	}
	if(need_cb_flag > 0)  //���г���
	{
		//��������
		logout(INFO, "MQTT", "�����忨", "%d�˿�,˰��ukey���г���\r\n", state->port);
		if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
		{
			out("��%d�ſڡ�׼������˰,˰������ʹ����,�޷�ʹ��\n", state->port);
			logout(INFO, "MQTT", "�����忨", "%d�˿�,׼������,˰������ʹ����,�޷�ʹ��\r\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��ռ����,��ʱδ���г����忨");
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
#else				                                              //��Ʊ���Ͳ�����Ҫ����㴫����0��Ԫ��
		result = function_cntax_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
#endif				
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_HZSC, REFER_TO_TAX_ADDRESS, result, errinfo);
		if (result < 0 && result != -3)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return 0;
			}
			logout(INFO, "MQTT", "�����忨", "%d�˿�,�������󣬴���ֵΪ%d\r\n", state->port,result);
			out("��%d�ſڡ���ȡ��������ʧ��\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			if (strlen(errinfo)<200)
				sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
			out("˰��ukey���ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
			state->port_info.cb_state = -1;
		}
		else
		{
			logout(INFO, "system", "cb_qk", "%s,��˰�ɹ�\r\n", state->port_info.port_str);
			result = 0;
			state->port_info.cb_state = 1;
			qk_flag += 1;
		}
	}
	if(need_qk_flag > 0) //�����忨
	{
		logout(INFO, "MQTT", "�����忨", "%d�˿�,˰��ukey�����忨\r\n", state->port);
		if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
		{
			logout(INFO, "MQTT", "�����忨", "%d�˿�,׼���忨,˰������ʹ����,�޷�ʹ��\r\n", state->port);
			out("��%d�ſڡ�׼���忨,˰������ʹ����,�޷�ʹ��\n", state->port);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
	#ifdef RELEASE_SO												 //��Ʊ���Ͳ�����Ҫ����㴫����0��Ԫ��
		result = so_cntax_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
	#else				
		result = function_cntax_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
	#endif				
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_FXJK, REFER_TO_TAX_ADDRESS, result, errinfo);
		if (result < 0 && result != -3)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return 0;
			}
			logout(INFO, "MQTT", "�����忨", "%d�˿�,�忨ʧ�ܣ�����ֵΪ%d\r\n", state->port,result);
			out("��%d�ſڡ��忨ʧ��\n", state->port);
			if (result == -4)
			{
				//out("���ܳɹ�,���忨ʧ��\n");						
				out("˰��ukey���ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
				state->port_info.hz_state = 1;
				state->port_info.qk_state = -1;
			}
			else
			{
				state->port_info.hz_state = -1;
				state->port_info.qk_state = -1;
			}
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			if (strlen(errinfo)<200)
				sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
			logout(INFO, "system", "cb_qk", "%s,�ϱ����ܼ��忨�ɹ�\r\n", state->port_info.port_str);
			result = 0;
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			qk_flag += 1;
		}
	}
	if (qk_flag != 0)
	{
		logout(INFO, "system", "cb_qk", "%s,�������忨����������˿ڵ�Դ\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}

	//logout(INFO, "system", "cb_qk","�忨״̬��%s��\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	//wang 04 03 
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	
	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	return 0;
}

//ģ���̳��������ϴ��������Ϣ��д
static int mengbai_get_cb_data_and_upload(struct _port_state   *state, char *plate_time, long plate_time_l)
{
	int result;
	int qk_flag = 0;
	int type_count;
	char errinfo[2048] = { 0 };
	char month_start[20] = {0};
	struct _invoice_type_infos *t_info;
	//int zp_pp_ok = false;

	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰�����ڶ˿�δ��Ȩ,�ݲ�ִ�г����忨");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��֤������ȷ,�ݲ�ִ�г����忨");
		}
		return 0;
	}
	out("��%d�ſ�:%s�����Խ��г����忨\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "���ڳ�ר�յ�Ʊ֮�������Ʊ�֣��ݲ�֧���Զ�����");

			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			out("�忨״̬��%s��\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		//if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
		//	if (zp_pp_ok)  //ר��Ʊ��Ϊһ��,ֻ����һ��
		//		continue;
		//	zp_pp_ok = true;
		//}
		
		out("��%d�ſڡ���Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���г�˰,���±�˰���ڣ�%s,��Ʊ��ֹ���ڣ�%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//��ǰ�´������±�˰������,��ҪС�ڿ�Ʊ��ֹ����,��Ҫ��˰����
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��г�˰����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫ����˰\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "˰��ռ����,��ʱδ���г����忨");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_mb_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_mengbai_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_HZSC, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					out("��%d�ſڡ���Ʊ���ͣ�%03d,��ȡ��������ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("ģ���̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,��˰�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ����˰ʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("��%s����Ʊ���ͣ�%03d,�ж��Ƿ���Ҫ���ܼ���д�����Ϣ,˰�̵�ǰʱ�䣺%s,��Ʊ��ֹʱ�䣺%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("�ж������³���һ����賿1��֮ǰ,�����򲻽��л��ܼ���д����\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("��%d�ſڡ���Ʊ���ͣ�%03d,��Ҫִ�л��ܼ���д��ض���\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);

				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					out("��%d�ſڡ���Ʊ���ͣ�%03d,˰������ʹ����,�޷�ʹ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
#ifdef RELEASE_SO
				result = so_mb_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#else				
				result = function_mengbai_report_summary_write_back(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, errinfo);
#endif				
				port_unlock(state);
				update_tax_business_info(state, DF_PORT_USED_FXJK, REFER_TO_TAX_ADDRESS, result, errinfo);
				if (result < 0)
				{
					if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
					{
						logout(INFO, "MQTT", "�����忨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					out("��%d�ſڡ���Ʊ���ͣ�%03d,�忨ʧ��\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("���ܳɹ�,���忨ʧ��\n");						
						out("˰���̻��ܡ���ط�дʧ�ܡ�%s��\n", errinfo);
						state->port_info.hz_state = 1;
						state->port_info.qk_state = -1;
					}
					else
					{
						state->port_info.hz_state = -1;
						state->port_info.qk_state = -1;
					}
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);


				}
				else
				{
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
					logout(INFO, "system", "cb_qk", "%s,��Ʊ���ͣ�%03d,�ϱ����ܼ��忨�ɹ�\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("��%d�ſڡ���Ʊ���ͣ�%03d,δ�������Ϣ��дʱ��,˰��ʱ��Ϊ��%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("��%d�ſڡ�������ڴ�����,ִ�жϵ����\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,�������忨����������˿ڵ�Դ\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}
	out("�忨״̬��%s��\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================��ǰ�˿�%d�����忨����\n",state->port+1);
	return 0;

}

int InvCheckInvoiceStr(uint8 *str)
{
	int i_len = strlen((const char *)str), i = 0;
	for (i = 0; i < i_len; i++) {
		//if (str[i] == '&' || str[i] == '\\')
		if (str[i] == '\\')
			return -1;
	}
	return 0;
}

static int InvDataIsGBK(const char *str)
{
	unsigned int nBytes = 0;  // GBK����1-2���ֽڱ���,��������
	// ,Ӣ��һ��
	uint8 chr = *str;
	int bAllAscii = 1;  //���ȫ������ASCII,
	int i = 0;
	for (i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
		if ((chr & 0x80) != 0 && nBytes == 0) {  // �ж��Ƿ�ASCII����,�������,˵���п�����GBK
			bAllAscii = 0;
		}
		if (nBytes == 0) {
			if (chr >= 0x80) {
				if (chr >= 0x81 && chr <= 0xFE) {
					nBytes = +2;
				}
				else {
					return 0;
				}
				nBytes--;
			}
		}
		else {
			if (chr < 0x40 || chr > 0xFE) {
				return 0;
			}
			nBytes--;
		}  // else end
	}
	if (nBytes != 0) {  //Υ������
		return 0;
	}
	if (bAllAscii) {  //���ȫ������ASCII,
		//Ҳ��GBK
		return 1;
	}
	return 1;
}

//�ͷŷ�Ʊ��Ϣ(������Ʒ��ϸ)�ṹ��
int InvFreeFpxx(HFPXX stp_Root)
{
	if (stp_Root == NULL)
		return -1;
	//ѭ���ͷ�ÿ�ŷ�Ʊ����ϸ
	HFPXX stp_FpxxNode = stp_Root;
	if (!stp_FpxxNode)
		return -2;
	struct Spxx *stp_MxxxNode = stp_FpxxNode->stp_MxxxHead, *stp_TmpSpxxNode = NULL;
	//�ͷ���Ʒ��ϸ
	while (stp_MxxxNode) {
		stp_TmpSpxxNode = stp_MxxxNode->stp_next;
		free(stp_MxxxNode);
		stp_MxxxNode = stp_TmpSpxxNode;
	}
	//���������ԭʼ��Ʊ��Ϣ�����ͷ�
	if (stp_FpxxNode->pRawFPBin)
		free(stp_FpxxNode->pRawFPBin);
	//�ͷ���Ʒ��ϸͷ
	free(stp_FpxxNode);
	return 0;
}


static int judge_plate_allow_kp(struct _port_state   *state, int inv_type, char *inv_data,char *errinfo)
{
	char kpjzsj[20] = {0};
	char tmp[20] = {0};
	char time_now[20] = {0};
	int i, j;
	int result;

	if (abs(state->port_info.plate_sys_time) > 12 * 3600)
	{
		out("˰��ʱ��������12Сʱ\n");
		out("˰��ʱ��� %d\n", state->port_info.plate_sys_time);
		if (state->port_info.plate_infos.plate_basic_info.plate_test != 1)
		{
			out("˰��ʱ�����̫������Ʊ\n");
			sprintf(errinfo, "˰��ʱ�����̫������Ʊ");
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
	}	

	if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
	{		
		sprintf(errinfo, "��ǰ˰���ڸ߰汾�����ʹ�ù�,���ڽ��汾ʹ�÷���,�������߰汾���ʹ��");
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	}
	
	for (i = 0; i < DF_MAX_INV_TYPE; i++)
	{
		//out("�ڴ�%d��Ʊ���ʹ��룺%d,inv_type = %d\n", i, state->port_info.tax_infos[i].fplxdm, inv_type);
		if (state->port_info.plate_infos.invoice_type_infos[i].fplxdm != inv_type)
		{
			continue;
		}
		//out("�ҵ���Ӧ�ķ�Ʊ���� %03d\n", inv_type);
		//out("�ж��Ƿ񳬹���Ʊ��ֹ����%s\n", state->port_info.plate_infos.invoice_type_infos[i].monitor_info.kpjzsj);
		memset(kpjzsj, 0, sizeof(kpjzsj));
		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, state->port_info.plate_infos.invoice_type_infos[i].monitor_info.kpjzsj, 4);
		sprintf(kpjzsj, "%s", tmp);

		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, state->port_info.plate_infos.invoice_type_infos[i].monitor_info.kpjzsj + 4, 2);
		sprintf(kpjzsj + strlen(kpjzsj), "-%s", tmp);

		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, state->port_info.plate_infos.invoice_type_infos[i].monitor_info.kpjzsj + 6, 2);
		sprintf(kpjzsj + strlen(kpjzsj), "-%s", tmp);

		sprintf(kpjzsj + strlen(kpjzsj), " 23:50:00");//ԭʱ��Ϊ00��00��00�ָ�Ϊ23��50��00���ſ�23Сʱ
		//out("��Ʊ��ֹ����Ϊ��%s\n",kpjzsj);

		//out("��ȡ��ǰʱ��\n");
		memset(time_now, 0, sizeof(time_now));
		timer_read_y_m_d_h_m_s(time_now);
		//out("��ǰʱ�䣺%s\n", time_now);
		//out("�жϵ�ǰʱ���Ƿ�Ϸ�\n");
		if (memcmp(time_now, SYSTEM_LEAST_TIME, strlen(time_now)) <= 0)
		{
			out("��ǰϵͳʱ�����,�޷���Ʊ\n");
			sprintf(errinfo,"��ǰϵͳʱ�����,�޷���Ʊ");
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
		if (memcmp(time_now, kpjzsj, strlen(time_now)) >= 0)
		{
			out("������Ʊ��ֹ����,�޷���Ʊ,time_now = %s,kpjzsj = %s\n", time_now, kpjzsj);
			sprintf(errinfo, "������Ʊ��ֹ����,�޷���Ʊ,time_now = %s,kpjzsj = %s", time_now, kpjzsj);
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
		if ((state->port_info.usb_app == DEVICE_NISEC) || (state->port_info.usb_app == DEVICE_CNTAX))
		{
			if (state->port_info.offinv_num_exit != 0)
			{
				if (state->port_info.plate_infos.invoice_type_infos[i].monitor_info.lxkpsc == 0)
				{
					sprintf(errinfo, "˰������ʱ��Ϊ0ʱ�������߶������߷�Ʊ");
					out("%s\n",errinfo);
					return DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
				}
			}
		}

		if (state->port_info.plate_infos.invoice_type_infos[i].invoice_coils_info.wsyfpjsl == 0)
		{
			out("�޿��÷�Ʊ��,wsyfpjsl = 0,i = %d\n", i);
			sprintf(errinfo, "�޿��÷�Ʊ��");
			return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		}
		for (j = 0; j < state->port_info.plate_infos.invoice_type_infos[i].invoice_coils_info.wsyfpjsl; j++)
		{
			if (state->port_info.plate_infos.invoice_type_infos[i].invoice_coils_info.invoice_coil[j].fpsyfs == 0)
			{
				continue;
			}
			break;
		}
		if (j == state->port_info.plate_infos.invoice_type_infos[i].invoice_coils_info.wsyfpjsl)
		{
			out("�޿��÷�Ʊ��,ʣ�෢Ʊ����Ϊ0\n");
			sprintf(errinfo, "�޿��÷�Ʊ��,ʣ�෢Ʊ����Ϊ0");
			return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		}
		//out("�ҵ��п��÷�Ʊ\n");
		break;
	}
	if (i == DF_MAX_INV_TYPE)
	{
		out("��Ʊ���Ͳ�֧��\n");
		sprintf(errinfo, "��Ʊ���Ͳ�֧��");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}

	str_replace(inv_data, "\\", "[@*br/*@]");//�Ų�json�е�\ת���ַ�

	result = InvCheckInvoiceStr((unsigned char *)inv_data);
	if (result < 0)
	{
		sprintf(errinfo, "��ƱJSON�����д��������ַ��޷�����,����&��\\");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
	}

	result = InvDataIsGBK(inv_data);
	if (result != 1)
	{
		sprintf(errinfo, "��ƱJSON���ݺ��ֱ����ʽ����,��GBK����");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//�����ʽ����
	}

	//out("\n��ʼУ�鷢Ʊ����\n");

	struct Fpxx *stp_fpxx = NULL;
	stp_fpxx = malloc(sizeof(struct Fpxx));
	if (stp_fpxx == NULL)
	{
		printf("stp_fpxx malloc Err\n");
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��,������ܴ��ڹ���,����������");
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	memset(stp_fpxx, 0, sizeof(struct Fpxx));
	stp_fpxx->hDev = state->h_plate.hDev;
	stp_fpxx->stp_MxxxHead = malloc(sizeof(struct Spxx));
	if (stp_fpxx->stp_MxxxHead == NULL)
	{
		printf("stp_fpxx->stp_MxxxHead malloc Err\n");
		sprintf(errinfo, "ϵͳ�ڴ�����ʧ��,������ܴ��ڹ���,����������");
		free(stp_fpxx);
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	memset(stp_fpxx->stp_MxxxHead, 0, sizeof(struct Spxx));

	//stp_fpxx->hDev = state->h_plate.hDev;
	strcpy(stp_fpxx->tax_nature, (char *)state->port_info.plate_infos.plate_tax_info.tax_nature);

	uint8 bDeviceType;
	if (state->port_info.usb_app == DEVICE_MENGBAI)
		bDeviceType = DEVICE_AISINO;
	else
		bDeviceType = state->port_info.usb_app;
#ifdef RELEASE_SO
	result = so_common_analyze_json_buff(bDeviceType, inv_data, stp_fpxx, errinfo);
#else
	result = function_common_analyze_json_buff(bDeviceType, inv_data, stp_fpxx, errinfo);
#endif
	if (result != 0)
	{
		logout(INFO, "MQTT", "��Ʊ����", "��Ʊ����ʧ��,�������%d\r\n", result);
		InvFreeFpxx(stp_fpxx);
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	InvFreeFpxx(stp_fpxx);
	//out("\nУ�鷢Ʊ���ݽ���\n");
	return 0;
}





//��ȡ˰�����ڶ˿ں�
int fun_get_plate_usb_port(unsigned char *plate_num)
{
	
	struct _ty_usb_m  *stream;
	int port;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("���̺�δ�ڵ�ǰ�ڴ��¼���ҵ�\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	return port;
}

//��ȡ˰�̵�ǰʱ��
int fun_get_plate_now_time(unsigned char *plate_num, char *time)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	
	out("ͨ���̺Ų������ڵĻ���USB�˿ں�\n");
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("���̺�δ�ڵ�ǰ�ڴ��¼���ҵ�\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 3, DF_PORT_USED_HQSPDQSJ) != 0)
	{
		out("˰������ʹ�����޷�ʹ��\n");
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ˰�̵�ǰʱ��\n");
#ifdef RELEASE_SO
		result = so_aisino_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_aisino_get_tax_time(&state->h_plate, (char *)time);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ˰�̵�ǰʱ��\n");
#ifdef RELEASE_SO
		result = so_nisec_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_nisec_get_tax_time(&state->h_plate, (char *)time);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY��ȡ˰�̵�ǰʱ��\n");
#ifdef RELEASE_SO
		result = so_cntax_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_cntax_get_tax_time(&state->h_plate, (char *)time);
#endif		
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("�ɰ��̻�ȡ˰�̵�ǰʱ��\n");
#ifdef RELEASE_SO
		result = so_mb_get_plate_time(&state->h_plate, (char *)time);
#else
		result = function_mengbai_get_plate_time(&state->h_plate, (char *)time);
#endif
		
	}
	else
	{
		result = -1;
	}
	port_unlock(state);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ��ǰ˰��ʱ��", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		return result;
	}
	return 0;
}

#ifndef DF_OLD_MSERVER
static int single_inv_upload(struct _port_state   *state, char *scfpsj, char *kpsj)
{
	struct _ty_usb_m  *stream;
	int result;
	char month[20] = { 0 };
	stream = ty_usb_m_id;
	memcpy(month, kpsj, 4);
	memcpy(month + strlen(month), kpsj + 5, 2);


	cJSON *inv_array, *inv_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "inv_data", inv_array = cJSON_CreateArray());

	cJSON_AddItemToObject(inv_array, "dira", inv_layer = cJSON_CreateObject());

	uint16 crc = crc_8005((uint8 *)scfpsj, strlen(scfpsj), 0);
	char invoice_crc[10] = {0};
	sprintf(invoice_crc,"%04x",crc);
	//out("Ҫ�ϴ��ķ�Ʊ����,crc Ϊ��%s\n", invoice_crc);

	cJSON_AddStringToObject(inv_layer, "invoice_crc", invoice_crc);
	cJSON_AddStringToObject(inv_layer, "invoice_data", scfpsj);

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);



	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);

	out("���������ѹ��ǰ���ݴ�С��%d,ѹ�������ݴ�С��%d\n", strlen(tmp_json), json_zlib_len);

	free(tmp_json);


	char source_topic[50] = { 0 };
	char random[50] = { 0 };
	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", stream->ter_id);
	get_radom_serial_number("zdsbfpxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbfpxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "plate_num", (char *)state->port_info.plate_infos.plate_basic_info.plate_num);
	cJSON_AddStringToObject(data_layer, "number", (char *)state->port_info.plate_infos.plate_basic_info.ca_number);
	cJSON_AddStringToObject(data_layer, "month", month);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	char *g_buf = cJSON_PrintUnformatted(root);
	//out("�ϱ���Ʊ�������ݣ�%s\n", g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_invoice_data_upload", g_buf, g_data, DF_MQTT_FPKJXT);
	free(g_buf);
	cJSON_Delete(root);
	if (result < 0)
	{
		logout(INFO, "system", "report_invoice", "���ϱ���Ʊ�����������ճ�ʱʧ��,��������\r\n");
		state->need_getinv = 1;//���ϴ���Ʊͳ�Ʋ�ѯ
		return 0;
	}
 	return 0;
}

static int inv_change_notice_upload(struct _port_state   *state,int act_type, char *fpdm, char *fphm,char *scfpsj,char *kpsj)
{
	
	switch (act_type)
	{
	case DF_INV_ACTION_KP:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ���߶���ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);//��Ʊ����Դ�
		break;
	case DF_INV_ACTION_ZF:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ���϶���ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);//���Ͻ���Դ�
		break;
	case DF_INV_ACTION_SC:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�ϴ�����ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	default:
		//out("��Ʊ�������ʹ���\n");
		return -1;
	}
	



	if ( (scfpsj != NULL) && (kpsj != NULL))
	{
		single_inv_upload(state, scfpsj, kpsj);
		return 0;
	}

	return 0;
}


#else
static int mengbai_plate_inv_upload(struct _port_state   *state, char *scfpsj,char *kpsj)
{
	int inv_len = 0;
	char month[20] = { 0 };
	char *send_data;
	int max_len = 1024 * 1024;
	out("��Ʊ���ߡ����Ϻ������ϴ���ֱ��ͬ����Ʊ����,��Ʊʱ�䣺%s\n", kpsj);
	send_data = malloc(max_len); //��Ʊ���ݻ�����1M
	memset(send_data, 0, max_len);
	send_data[0] = (((1) >> 24) & 0xff); //��Ʊ���
	send_data[1] = (((1) >> 16) & 0xff);
	send_data[2] = (((1) >> 8) & 0xff);
	send_data[3] = (((1) >> 0) & 0xff);
	if (scfpsj != NULL)
		inv_len = strlen((const char *)scfpsj);
	send_data[4] = ((inv_len >> 24) & 0xff); send_data[5] = ((inv_len >> 16) & 0xff);//��Ʊ����
	send_data[6] = ((inv_len >> 8) & 0xff); send_data[7] = ((inv_len >> 0) & 0xff);
	memcpy(send_data + 8, scfpsj, inv_len);

	memcpy(month, kpsj, 4);
	memcpy(month + strlen(month), kpsj + 5, 2);

	mqtt_pub_hook((uint8 *)send_data, 0, 1, state, (char *)state->port_info.plate_infos.plate_basic_info.plate_num, (uint8 *)month, NULL,1);
	free(send_data);
	return 0;
}
static int inv_change_notice_upload(struct _port_state   *state,int act_type, char *fpdm, char *fphm,char *scfpsj,char *kpsj)
{
	int i=0,j=0;
	switch (act_type)
	{
	case DF_INV_ACTION_KP:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ���߶���ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	case DF_INV_ACTION_ZF:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ���϶���ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	case DF_INV_ACTION_SC:
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ʊ�ϴ�����ͬ�����ŷ�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	default:
		//out("��Ʊ�������ʹ���\n");
		return -1;
	}
	
	if (state->port_info.upload_inv_num.auto_up_end != 1)
	{
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,ֱ����ϵͳ�Զ��ϴ�����,���赥���ϴ��÷�Ʊ,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		return 0;
	}
	if (state->port_info.usb_app == 3)//ģ���̽�֧�����²�ѯ
	{
		state->need_getinv = 1;
		return 0;
	}
	if (strlen(fpdm) == 0 || (strlen(fphm) == 0))
	{
		logout(INFO, "system", "��Ʊ�ϴ�M����", "%s,��Ҫ�ϴ����ŷ�Ʊ,������ķ�Ʊ������볤������,ֱ�Ӵ��������ϴ�,��Ʊ���룺%s ��Ʊ���룺%s\r\n", state->port_info.port_str, fpdm, fphm);
		state->need_getinv = 1;
		return 0;
	}
	if ( (scfpsj != NULL) && (kpsj != NULL))
	{
		mengbai_plate_inv_upload(state, scfpsj, kpsj);
		return 0;
	}
	else
	{
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]); i++)
		{
			if (state->port_info.upload_inv_num.need_upload_dmhm[i].state == 0)
			{
				continue;
			}
			if ((strcmp(state->port_info.upload_inv_num.need_upload_dmhm[i].fpdm, fpdm) == 0) && ((strcmp(state->port_info.upload_inv_num.need_upload_dmhm[i].fphm, fphm) == 0)))
			{
				out("�˷�Ʊ����:%s����:%s���ڻ������\n", fpdm, fphm);
				return 0;
			}
		}
		out("�˷�Ʊ����:%s����:%s���ڻ������\n", fpdm, fphm);
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]); i++)
		{
			if (state->port_info.upload_inv_num.need_upload_dmhm[i].state != 0)
			{
				continue;
			}
			break;
		}
		if (i == sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]))
		{
			out("�����������,�޷�����,ֱ�Ӵ��������ϴ�\n");
			state->need_getinv = 1;
			state->port_info.upload_inv_num.need_fpdmhm_flag = 0;
			return 0;
		}

		for (j = i + 1; j < sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]); j++)
		{
			if (state->port_info.upload_inv_num.need_upload_dmhm[j].state != 0)
			{
				break;
			}
		}
		if (j != sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]))
		{
			out("������п����Ѿ�����,���ܴ��������,ֱ�Ӵ��������ϴ�\n");
			state->need_getinv = 1;
			state->port_info.upload_inv_num.need_fpdmhm_flag = 0;
			return 0;
		}

		state->port_info.upload_inv_num.need_upload_dmhm[i].state = 1;
		memset(state->port_info.upload_inv_num.need_upload_dmhm[i].fpdm, 0, sizeof(state->port_info.upload_inv_num.need_upload_dmhm[i].fpdm));
		memcpy(state->port_info.upload_inv_num.need_upload_dmhm[i].fpdm, fpdm, strlen(fpdm));

		memset(state->port_info.upload_inv_num.need_upload_dmhm[i].fphm, 0, sizeof(state->port_info.upload_inv_num.need_upload_dmhm[i].fphm));
		memcpy(state->port_info.upload_inv_num.need_upload_dmhm[i].fphm, fphm, strlen(fphm));


		state->port_info.upload_inv_num.need_fpdmhm_flag = 1;
		out("��Ʊ���ߡ��ϴ����������ϴ���M����\n");
	}
	return 0;
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
	int nBusID = state->h_plate.nBusID;
	int nDevID = state->h_plate.nDevID;

	state->h_plate.nBusID = -1;
	state->h_plate.nDevID = -1;

	if (state->power_mode == Saving_Mode)
	{
		state->sys_reload = 0;
		port_power_down(state);
		return 0;
	}
	state->sys_reload = 0;
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
		logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "����%d�Ŷ˿ڵ�Դ��,˰�̻���δ���,�̺ţ�%s,busid : %s,used_info:%s\r\n", port, plate_num,busid,state->port_info.used_info);

		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "����%d�Ŷ˿ڵ�Դ��,USB��busid:%s�����ļ�ϵͳ��ʧ,�̺ţ�%s,����Ϊ�̼߳�����\r\n", port, busid, plate_num);
		}
		else
		{
			sprintf(errinfo,"����%d�Ŷ˿ڵ�Դ��,USB��busid:%sδ���ļ�ϵͳ��ʧ,�̺ţ�%s,����ΪUSB�ļ�ϵͳ���ϻ�δ�����ص�", port, busid, plate_num);
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
#ifdef RELEASE_SO
			so_common_report_event((char *)plate_num, "Ӳ���˿ڹص�ʧ��", errinfo, -1);
#else
			function_common_report_event((char *)plate_num, "Ӳ���˿ڹص�ʧ��", errinfo, -1);
#endif	
			state->h_plate.nBusID = nBusID;
			state->h_plate.nDevID = nDevID;
			state->power = 1; // �ص�δ�ɹ����ѵ�Դ��־��Ϊ�е�
			//state->port_info.tax_read = 1;
			waiting_for_plate_install((uint8 *)plate_num, state, 10);
			return 0;

		}
	}

	//out("˰���ѱ�ж��\n");
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
	if (result < 0)
	{
		sprintf(errinfo, "Ҫ��Ƭ����%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
		logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
#ifdef RELEASE_SO
		so_common_report_event((char *)plate_num, "��Ƭ���˿ڿ���ʧ��", errinfo, -1);
#else
		function_common_report_event((char *)plate_num, "��Ƭ���˿ڿ���ʧ��", errinfo, -1);
#endif	
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
		if (result < 0)
		{
			sprintf(errinfo, "Ҫ��Ƭ�����Դ�%d�Ŷ˿ڵ�Դʧ��,result = %d", port, result);
			logout(INFO, "SYSTEM", "�����˿ڵ�Դ", "%s\r\n", errinfo);
#ifdef RELEASE_SO
			so_common_report_event((char *)plate_num, "��Ƭ���˿����Կ���ʧ��", errinfo, -1);
#else
			function_common_report_event((char *)plate_num, "��Ƭ���˿����Կ���ʧ��", errinfo, -1);
#endif	
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
	//out("˰���Ѽ��سɹ�\n");
	return 0;
}

static int plate_useinfo_errinfo(struct _port_state   *state, char *errinfo)
{
	char used_now[1024] = { 0 };
	if (strlen((char *)state->port_info.sn) == 0)
	{
		sprintf(errinfo, "����������˰������������");
		return 0;
	}
	if (strcmp(state->port_info.used_info, DF_PORT_USED_FPKJ) == 0)
	{
		sprintf(used_now, "���ڿ���������Ʊ");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)
	{
		sprintf(used_now, "����ʹ��usb-share���ع���");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USBD_QD_LOCK) == 0)
	{
		sprintf(used_now, "����ʹ��ȫ�繦��");
	}	
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)
	{
		sprintf(used_now, "�����ϴ����߷�Ʊ");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_FPCX_SCMB) == 0)
	{
		sprintf(used_now, "����ͬ��M����Ʊ����");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_AYCXFPSJMQTT) == 0)
	{
		sprintf(used_now, "���ڰ��²�ѯ��Ʊ����");
	}
	else
	{
		logout(INFO, "SYSTEM", "SYS", "����ִ������ռ��˰�̲���,%s\r\n", (char *)state->port_info.used_info);
		sprintf(used_now, "����ִ������ռ��˰�̲���");
	}
	sprintf(errinfo, "˰������ʹ����,��ǰ%s", used_now);
	return 0;
}

int kj_zf_update_sjxsje_se(struct _port_state   *state,char *base_inv_json)
{
	char fplx_s[5] = {0};
	char zfbs_s[3] = {0};
	char hjje_s[25] = {0};
	char hjse_s[25] = {0};
	int zfbs;
	uint8 fplxdm;
	long double hjje;
	long double hjse;
	long inv_len;
	if (base_inv_json == NULL)
		return -1;
	inv_len = strlen(base_inv_json) + 1024;
	char *inv_json = malloc(inv_len);
	memset(inv_json, 0,inv_len);

	//Base64_Decode(base_inv_json, strlen(base_inv_json), inv_json);
#ifndef DF_OLD_MSERVER
	strcpy(inv_json, base_inv_json);
#else
	base64_dec((uint8 *)inv_json, (uint8 *)base_inv_json, inv_len);
#endif
	//out("��Ʊ����[%s]", inv_json);
	cJSON *item;
	cJSON *root = cJSON_Parse(inv_json);
	if (!root)
	{
		//out("��Ʊ���ݽ���ʧ��[%s]", inv_json);
		free(inv_json);
		return -3;
	}
	free(inv_json);

	item = cJSON_GetObjectItem(root, "fpzls");
	memset(fplx_s, 0, sizeof(fplx_s));
	if (get_json_value_can_not_null(item, fplx_s, 1, 5) < 0)
	{
		cJSON_Delete(root);
		return -4;
	}
	fplxdm = atoi(fplx_s);
	
	item = cJSON_GetObjectItem(root, "zfbz");
	memset(zfbs_s, 0, sizeof(zfbs_s));
	if (get_json_value_can_not_null(item, zfbs_s, 1, 3) < 0)
	{
		cJSON_Delete(root);
		return -5;
	}
	zfbs = atoi(zfbs_s);

	item = cJSON_GetObjectItem(root, "hjje");
	memset(hjje_s, 0, sizeof(hjje_s));
	if (get_json_value_can_not_null(item, hjje_s, 1, 25) < 0)
	{
		cJSON_Delete(root);
		return -6;
	}
	hjje = strtold(hjje_s, NULL);

	item = cJSON_GetObjectItem(root, "hjse");
	memset(hjse_s, 0, sizeof(hjse_s));
	if (get_json_value_can_not_null(item, hjse_s, 1, 25) < 0)
	{
		cJSON_Delete(root);
		return -7;
	}
	hjse = strtold(hjse_s, NULL);
	cJSON_Delete(root);
	int mem_i;
#ifdef RELEASE_SO
	so_common_find_inv_type_mem(fplxdm, &mem_i);
#else
	function_common_find_inv_type_mem(fplxdm, &mem_i);
#endif
#ifndef DF_OLD_MSERVER
	if (zfbs == 1)
	{
		if (hjje > 0)
		{
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zffpfs += 1;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zffpljje += hjje;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zffpljse += hjse;
		}
		else if (hjje < 0)
		{
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fffpfs += 1;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fffpljje += hjje;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fffpljse += hjse;
		}
		else
		{
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.kffpfs += 1;
		}
		
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.sjxsje -= hjje;
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.sjxsse -= hjse;
	}
	else
	{
		if (hjje > 0)
		{
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zsfpfs += 1;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zsfpljje += hjje;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.zsfpljse += hjse;
		}
		else if (hjje < 0)
		{
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fsfpfs += 1;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fsfpljje += hjje;
			state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.fsfpljse += hjse;
		}
		//out("��ĩ��������1\n");
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.qmkcfs -= 1;
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.sjxsje += hjje;
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.type_sum.sjxsse += hjse;
	}
#else
	if (zfbs == 1)
	{
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.sjxsje -= hjje;
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.sjxsse -= hjse;
	}
	else
	{
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.sjxsje += hjje;
		state->port_info.plate_infos.invoice_type_infos[mem_i].monitor_info.sjxsse += hjse;
	}
#endif
	return 0;
}

static int jude_port_auth_passwd(struct _port_state   *state,char *errinfo)
{
	int result;
#ifdef RELEASE_SO
	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid,errinfo);
#else
	result = function_common_jude_plate_auth(state->port_info.usb_app,state->port_info.busid,errinfo);
#endif
	if (result != 1)
	{
		return DF_TAX_ERR_CODE_USB_PORT_NOT_AUTH;
	}
	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
		else
			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
	return 0;
}

//��Ʊ����
int fun_make_invoice_to_plate(unsigned char *plate_num, int inv_type, unsigned char *inv_data, struct _make_invoice_result *make_invoice_result)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;	
	int port;
	int result;
	struct _plate_infos plate_infos;
	memset(&plate_infos, 0, sizeof(struct _plate_infos));
	stream = ty_usb_m_id;

	
	//out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		logout(INFO, "MQTT", "FPKJ", "%s\r\n", (char *)make_invoice_result->errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	memcpy(&plate_infos, &state->port_info.plate_infos, sizeof(struct _plate_infos));

	result = jude_port_auth_passwd(state, (char *)make_invoice_result->errinfo);
	if (result < 0)
	{
		return result;
	}
	//out("�ж���˰���Ƿ�������Ʊ\n");
	result = judge_plate_allow_kp(state, inv_type, (char *)inv_data,(char *) make_invoice_result->errinfo);
	if (result < 0)
	{
		return result;
	}

	//logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ����˰��״̬����Ʊ����ԤУ��ͨ��\r\n", state->port_info.port_str);
	int sp_type = state->port_info.usb_app;
	//out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (sp_type == DEVICE_AISINO)
	{
		if ((result = port_lock(state, 40, 3, DF_PORT_USED_FPKJ)) != 0)
		{
			if (state->power_mode == Saving_Mode)
			{
				if (result == POWER_ERR_NO_DEV)
				{
					out("˰������ʹ�����޷���Ʊ\n");
					sprintf((char *)make_invoice_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
				}
				else if (result == POWER_ERR_FULL_LOAD)
				{
					out("������������\n");
					sprintf((char *)make_invoice_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
				}
				else
				{
					out("˰������ʹ�����޷���Ʊ\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
				}
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			out("˰������ʹ�����޷���Ʊ\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_aisino_make_invoice(&state->h_plate,(unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
		result = function_aisino_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif
		state->port_info.last_kp_time = get_time_msec();
		port_unlock(state);
		if (result < 0)
		{
			if ((make_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
			{
				logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ߵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port,1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰������ʹ�����޷�ʹ��\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
#ifdef RELEASE_SO
				result = so_aisino_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
				result = function_aisino_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif		
				state->port_info.last_kp_time = get_time_msec();
				port_unlock(state);
				if (result < 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("��˰�̷�Ʊ����ʧ��\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				out("��˰�̷�Ʊ����ʧ��\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//���ڿ��߽���д���Ʊͬ��,�˴���ȡ��
	}
	else if (sp_type == DEVICE_NISEC)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("˰������ʹ�����޷���Ʊ\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_nisec_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
		result = function_nisec_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif
		state->port_info.last_kp_time = get_time_msec();
		port_unlock(state);
		if (result < 0)
		{
			if ((make_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
			{
				logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ߵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰������ʹ�����޷�ʹ��\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
#ifdef RELEASE_SO
				result = so_nisec_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
				result = function_nisec_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif		
				state->port_info.last_kp_time = get_time_msec();
				port_unlock(state);
				if (result < 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰���̷�Ʊ����ʧ��\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				out("˰���̷�Ʊ����ʧ��\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//���ڿ��߽���д���Ʊͬ��,�˴���ȡ��
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("˰������ʹ�����޷���Ʊ\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
		result = function_cntax_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif
		state->port_info.last_kp_time = get_time_msec();
		port_unlock(state);
		if (result < 0)
		{
			if ((make_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
			{
				logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ߵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰������ʹ�����޷�ʹ��\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
#ifdef RELEASE_SO
				result = so_cntax_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
				result = function_cntax_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif		
				state->port_info.last_kp_time = get_time_msec();
				port_unlock(state);

				if (result < 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰��ukey��Ʊ����ʧ��\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				out("˰��ukey��Ʊ����ʧ��\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//���ڿ��߽���д���Ʊͬ��,�˴���ȡ��
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("˰������ʹ�����޷���Ʊ\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_mb_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
		result = function_mengbai_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif
		state->port_info.last_kp_time = get_time_msec();
		port_unlock(state);
		if (result < 0)
		{
			if ((make_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
			{
				logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ߵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("˰������ʹ�����޷�ʹ��\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
#ifdef RELEASE_SO
				result = so_mb_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#else
				result = function_mengbai_make_invoice(&state->h_plate, (unsigned char)inv_type, (char *)inv_data, &plate_infos, make_invoice_result);
#endif		
				state->port_info.last_kp_time = get_time_msec();
				port_unlock(state);
				if (result < 0)
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					out("ģ���̷�Ʊ����ʧ��\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				out("ģ���̷�Ʊ����ʧ��\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//mengbai_plate_inv_upload(state, make_invoice_result->scfpsj, (char *)make_invoice_result->kpsj);
	}	
	return 0;

}



int fun_cancel_invoice_to_plate(unsigned char *plate_num, int inv_type, int zflx, unsigned char *fpdm, unsigned char *fphm, unsigned char *zfr, struct _cancel_invoice_result *cancel_invoice_result)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	int z,y;
	int mem_i;
	int fpzf_need_restart = 0;
	int fpzf_fphm = 0;;
	struct _plate_infos plate_infos;
	memset(&plate_infos, 0, sizeof(struct _plate_infos));

	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)cancel_invoice_result->errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	memcpy(&plate_infos, &state->port_info.plate_infos, sizeof(struct _plate_infos));
	result = jude_port_auth_passwd(state, (char *)cancel_invoice_result->errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;

	if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "��ǰ˰���ڸ߰汾�����ʹ�ù�,���ڽ��汾ʹ�÷���,�������߰汾���ʹ��");
		out("%s\n", (char *)cancel_invoice_result->errinfo);
		return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	}
	
	if (zflx == 1)
	{
		out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);

		//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
		if (sp_type == DEVICE_AISINO)
		{
			out("��˰�̷�Ʊ����\n");
			if ((result = port_lock(state, 20, 3, DF_PORT_USED_FPZF)) != 0)
			{
				if (state->power_mode == Saving_Mode)
				{
					if (result == POWER_ERR_NO_DEV)
					{
						out("˰������ʹ�����޷���Ʊ\n");
						sprintf((char *)cancel_invoice_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
					else if (result == POWER_ERR_FULL_LOAD)
					{
						out("������������\n");
						sprintf((char *)cancel_invoice_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
					}
				}
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
#ifdef RELEASE_SO
			result = so_aisino_waste_invoice(&state->h_plate,(unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
			result = function_aisino_waste_invoice(&state->h_plate,(unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
			port_unlock(state);
			if (result < 0)
			{

				if ((cancel_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
				{
					logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ϵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
					out("����ʧ����ʾ�ײ������ϵ������\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
						out("%s\n", cancel_invoice_result->errinfo);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
#ifdef RELEASE_SO
					result = so_aisino_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
					result = function_aisino_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
					port_unlock(state);
					if (result < 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("��Ʊ����ʧ��\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("��Ʊ����ʧ��\n");
					return result;
				}

			}
			out("��Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			aisino_inv_cancel_complete_update_lxzssyje(state, cancel_invoice_result);
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			return 0;
		}
		else if (sp_type == DEVICE_NISEC)
		{
			out("�����̷�Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
#ifdef RELEASE_SO
			result = so_nisec_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
			result = function_nisec_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
			port_unlock(state);
			if (result < 0)
			{

				if ((cancel_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
				{
					logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ϵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
					out("����ʧ����ʾ�ײ������ϵ������\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
						out("%s\n", cancel_invoice_result->errinfo);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
#ifdef RELEASE_SO
					result = so_nisec_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
					result = function_nisec_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
					port_unlock(state);
					if (result < 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("��Ʊ����ʧ��\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("��Ʊ����ʧ��\n");
					return result;
				}

			}
			out("��Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			return 0;
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("˰��UKEY��Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
#ifdef RELEASE_SO
			result = so_cntax_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
			result = function_cntax_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
			port_unlock(state);
			if (result < 0)
			{

				if ((cancel_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
				{
					logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ϵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
					out("����ʧ����ʾ�ײ������ϵ������\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
						out("%s\n", cancel_invoice_result->errinfo);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
#ifdef RELEASE_SO
					result = so_cntax_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
					result = function_cntax_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
					port_unlock(state);
					if (result < 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("��Ʊ����ʧ��\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("��Ʊ����ʧ��\n");
					return result;
				}

			}
			out("��Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			return 0;
		}
		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("�ɰ��̷�Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
#ifdef RELEASE_SO
			result = so_mb_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
			result = function_mengbai_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
			port_unlock(state);
			if (result < 0)
			{

				if ((cancel_invoice_result->need_restart == 1) || (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR))
				{
					logout(INFO, "MQTT", "FPKJ", "%s,��Ʊ���ϵײ�ʧ��,�����˿ڵ�Դ��������һ��\r\n", state->port_info.port_str);
					out("����ʧ����ʾ�ײ������ϵ������\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
						out("%s\n", cancel_invoice_result->errinfo);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
#ifdef RELEASE_SO
					result = so_mb_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#else
					result = function_mengbai_waste_invoice(&state->h_plate, (unsigned char)inv_type, (char *)fpdm, (char *)fphm, (char *)zfr, cancel_invoice_result);
#endif			
					port_unlock(state);
					if (result < 0)
					{
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("��Ʊ����ʧ��\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("��Ʊ����ʧ��\n");
					return result;
				}

			}
			out("��Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, NULL, NULL);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			return 0;
		}
	}
	else if (zflx == 0)
	{
		out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
		//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
		if (sp_type == DEVICE_AISINO)
		{
			out("��˰�̷�Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 �հ׷�Ʊ�������һ��Ӧ�ò��޸� 
			//wang  03 31 ��Ѱ��������Ʊ�ֵķ�Ʊ��ʼ�����Ƿ�������Ϻ���
			//��ͬ��Ƚ�����������ʣ���������ͬ�����ϳɹ��������˿�
			logout(INFO, "MQTT", "FPZF", "��Ʊ����=%d,��Ʊ���Ϻ���=%s,��Ʊ���Ϻ���=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 ��ѯ���Ҷ�Ӧ���ϵķ�Ʊ��������ķ�Ʊ��ʣ������
				//�жϷ�Ʊ��ʣ�������Ƿ������������
				//��Ⱦ������˿�
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "AISINO�հ׷�Ʊ���Ϸ�Ʊ����%d,��ǰ��Ʊʣ������=%d,��������=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "AISINO�հ׷�Ʊ����ʣ�෢Ʊ��%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
						fpzf_need_restart = 1;
					}
				}
			}
#ifdef RELEASE_SO
			result = so_aisino_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#else 
			result = function_aisino_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#endif
			port_unlock(state);
			if (result < 0)
			{
				out("��Ʊ����ʧ��\n");
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;

				//����ʧ��ҲҪ�ж��Ƿ���Ҫ����
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}
				return result;
			}
			logout(INFO,"MQTT", "FPZF", "�հ׷�Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//����ϴβ�ѯʱ�䣬������ѯ��ͬ����Ʊ
#endif
			state->need_getinv = 1;
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}

		//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
		else if (sp_type == DEVICE_NISEC)
		{
			out("˰���̷�Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 �հ׷�Ʊ�������һ��Ӧ�ò��޸� 
			//wang  03 31 ��Ѱ��������Ʊ�ֵķ�Ʊ��ʼ�����Ƿ�������Ϻ���
			//��ͬ��Ƚ�����������ʣ���������ͬ�����ϳɹ��������˿�
			logout(INFO, "MQTT", "FPZF", "��Ʊ����=%d,��Ʊ���Ϻ���=%s,��Ʊ���Ϻ���=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 ��ѯ���Ҷ�Ӧ���ϵķ�Ʊ��������ķ�Ʊ��ʣ������
				//�жϷ�Ʊ��ʣ�������Ƿ������������
				//��Ⱦ������˿�
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "NISEC�հ׷�Ʊ���Ϸ�Ʊ����%d,��ǰ��Ʊʣ������=%d,��������=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "NISEC�հ׷�Ʊ����ʣ�෢Ʊ��%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
						fpzf_need_restart = 1;
					}
				}
			}
#ifdef RELEASE_SO
			result = so_nisec_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#else 
			result = function_nisec_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#endif
			port_unlock(state);
			if (result < 0)
			{
				out("��Ʊ����ʧ��\n");
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;

				//����ʧ��ҲҪ�ж��Ƿ���Ҫ����
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "�հ׷�Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//����ϴβ�ѯʱ�䣬������ѯ��ͬ����Ʊ
#endif
			state->need_getinv = 1;
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("˰��ukey��Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 �հ׷�Ʊ�������һ��Ӧ�ò��޸� 
			//wang  03 31 ��Ѱ��������Ʊ�ֵķ�Ʊ��ʼ�����Ƿ�������Ϻ���
			//��ͬ��Ƚ�����������ʣ���������ͬ�����ϳɹ��������˿�
			logout(INFO, "MQTT", "FPZF", "��Ʊ����=%d,��Ʊ���Ϻ���=%s,��Ʊ���Ϻ���=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 ��ѯ���Ҷ�Ӧ���ϵķ�Ʊ��������ķ�Ʊ��ʣ������
				//�жϷ�Ʊ��ʣ�������Ƿ������������
				//��Ⱦ������˿�
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "CNTAX�հ׷�Ʊ���Ϸ�Ʊ����%d,��ǰ��Ʊʣ������=%d,��������=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "CNTAX�հ׷�Ʊ����ʣ�෢Ʊ��%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
						fpzf_need_restart = 1;
					}
				}
			}
#ifdef RELEASE_SO
			result = so_cntax_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#else 
			result = function_cntax_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#endif
			port_unlock(state);
			if (result < 0)
			{
				out("��Ʊ����ʧ��\n");
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;

				//����ʧ��ҲҪ�ж��Ƿ���Ҫ����
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "�հ׷�Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);	
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//����ϴβ�ѯʱ�䣬������ѯ��ͬ����Ʊ
#endif
			state->need_getinv = 1;
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("�ɰ��̷�Ʊ����\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 �հ׷�Ʊ�������һ��Ӧ�ò��޸� 
			//wang  03 31 ��Ѱ��������Ʊ�ֵķ�Ʊ��ʼ�����Ƿ�������Ϻ���
			//��ͬ��Ƚ�����������ʣ���������ͬ�����ϳɹ��������˿�
			logout(INFO, "MQTT", "FPZF", "��Ʊ����=%d,��Ʊ���Ϻ���=%s,��Ʊ���Ϻ���=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 ��ѯ���Ҷ�Ӧ���ϵķ�Ʊ��������ķ�Ʊ��ʣ������
				//�жϷ�Ʊ��ʣ�������Ƿ������������
				//��Ⱦ������˿�
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "MENGBAI�հ׷�Ʊ���Ϸ�Ʊ����%d,��ǰ��Ʊʣ������=%d,��������=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "MENGBAI�հ׷�Ʊ����ʣ�෢Ʊ��%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
						fpzf_need_restart = 1;
					}
				}
			}
#ifdef RELEASE_SO
			result = so_mb_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#else 
			result = function_mengbai_waste_null_invoice(&state->h_plate, &plate_infos, cancel_invoice_result);
#endif
			port_unlock(state);
			if (result < 0)
			{
				out("��Ʊ����ʧ��\n");
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;

				//����ʧ��ҲҪ�ж��Ƿ���Ҫ����
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "�հ׷�Ʊ���ϳɹ�,��Ʊ���룺%s,��Ʊ���룺%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//����ϴβ�ѯʱ�䣬������ѯ��ͬ����Ʊ
#endif
			state->need_getinv = 1;
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,��Ʊ�����һ�ŷ�Ʊ������������˿ڵ�Դ\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
	}
	return -1;
}
//�����ϱ�Ƶ��
int function_set_report_time(int report_time)
{
	//struct _ty_usb_m  *stream;
	//stream = ty_usb_m_id;
	out("�����ϱ�Ƶ��Ϊ%d\n", report_time);
	//stream->report_time_o = report_time;
	return 0;

}
//��֤֤����Կ�Ƿ���ȷ
int fun_check_cert_passwd(unsigned char *plate_num, unsigned char *cert_passwd)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream,plate_num);
	if (port <= 0)
	{
		out("���̺�δ�ڵ�ǰ�ڴ��¼���ҵ�\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;

	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		out("��¼��Ĭ�Ͽ������\n");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
	
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 3, DF_PORT_USED_MRKLRZ) != 0)
	{
		out("˰������ʹ�����޷���Ʊ\n");
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰����֤��������֤\n");		
#ifdef RELEASE_SO
		result = so_aisino_verify_cert_passwd(&state->h_plate,(char *)cert_passwd);
#else
		result = function_aisino_verify_cert_passwd(&state->h_plate,(char *)cert_passwd);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰����֤��������֤\n");
#ifdef RELEASE_SO
		result = so_nisec_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#else
		result = function_nisec_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY֤��������֤\n");
#ifdef RELEASE_SO
		result = so_cntax_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#else
		result = function_cntax_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#endif
	}
	//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("�ɰ���֤��������֤\n");
#ifdef RELEASE_SO
		result = so_mb_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#else
		result = function_mengbai_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#endif		
	}
	else
	{
		result = -1;		
	}
	port_unlock(state);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result <= 0)
	{
		out("֤��������֤ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "֤��������֤", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		return result;
	}
	return 0;
}




int fun_check_now_fpdm_fphm(unsigned char *plate_num, int inv_type, char *fpdm, char *fphm, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];


	int mem_i;
#ifdef RELEASE_SO
	so_common_find_inv_type_mem(inv_type, &mem_i);
#else
	function_common_find_inv_type_mem(inv_type, &mem_i);
#endif

	if (state->port_info.plate_infos.invoice_type_infos[mem_i].state == 0)
	{
		sprintf(errinfo, "��֧�ִ�Ʊ��");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}
	if (state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.state == 0)
	{
		sprintf(errinfo, "δ�ҵ���Ӧ��Ʊ����ĵ�ǰ��Ʊ���롢����,��ȷ�ϸ�Ʊ���Ƿ���ڷ�Ʊ!");
		return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
	}
	if (state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl == 0)
	{
		sprintf(errinfo, "δ�ҵ���Ӧ��Ʊ����ĵ�ǰ��Ʊ���롢����,��ȷ�ϸ�Ʊ���Ƿ���ڷ�Ʊ!");
		return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
	}

	strcpy(fpdm, (char *)state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.dqfpdm);
	sprintf(fphm, "%08d", state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.dqfphm);
	return 0;

}


int fun_cb_qingka_get_usb_info(unsigned char *plate_num, struct _usb_port_infor *port_info)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	//unsigned short busnum;
	//unsigned short devnum;
	long time_now;
	long plate_time;
	char time_s[20]={0};
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("���̺�δ�ڵ�ǰ�ڴ��¼���ҵ�\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("%d�ſ��ǽ�˰��,���г����忨����\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "�����忨1", "%d�ſ��ǽ�˰��,���г����忨����\n", port);	
		aisino_get_cb_data_and_upload(state, time_s, plate_time);
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(4, state->port_info.plate_infos.plate_tax_info.next_report_time);		
		sleep(5);
		update_plate_info_cbqkzt(state);
		memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
		port_info->port = state->port;
		port_info->port_power = state->power;
		return 0;
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("%d�ſ���˰����,���г����忨����\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "�����忨1", "%d�ſ���˰����,���г����忨����\n", port);	
		nisec_get_cb_data_and_upload(state, time_s, plate_time);
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(4, state->port_info.plate_infos.plate_tax_info.next_report_time);
		sleep(5);
		update_plate_info_cbqkzt(state);
		memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
		port_info->port = state->port;
		port_info->port_power = state->power;
		return 0;
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("%d�ſ���˰��UKEY,���г����忨����\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "�����忨1", "%d�ſ���˰��UKEY,���г����忨����\n", port);
		cntax_get_cb_data_and_upload(state, time_s, plate_time);
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(4, state->port_info.plate_infos.plate_tax_info.next_report_time);
		sleep(5);
		update_plate_info_cbqkzt(state);
		memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
		port_info->port = state->port;
		port_info->port_power = state->power;
		return 0;
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("%d�ſ���ģ����,���г����忨����\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "�����忨1", "%d�ſ���ģ����,���г����忨����\n", port);	
		mengbai_get_cb_data_and_upload(state, time_s, plate_time);
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(4, state->port_info.plate_infos.plate_tax_info.next_report_time);
		sleep(5);
		update_plate_info_cbqkzt(state);
		memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
		port_info->port = state->port;
		port_info->port_power = state->power;
		return 0;
	}
	return -1;
}

int fun_update_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, int dzsyh,char **inv_data, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}

	int sp_type = state->port_info.usb_app;
	out("ͨ���ж��ҵ��Ķ˿ں�Ϊ��%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if ((result = port_lock(state, 20, 4, DF_PORT_USED_FPHM_SCFP)) != 0)
	{
		if (result == POWER_ERR_NO_DEV)
		{
			sprintf(errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("������������\n");
			sprintf((char *)errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
		}
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
#ifdef RELEASE_SO
		result = so_aisino_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#else
		result = function_aisino_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̷�Ʊ��������ȡ��Ʊ��Ϣ\n");
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#else
		result = function_nisec_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#endif
	}
	else 
	{
		sprintf(errinfo, "˰�����Ͳ�֧�ַ�Ʊ��������ϴ���Ʊ");
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_FPHM_SCFP, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("��Ʊ��ѯʧ��\n");
		sprintf(errinfo, "���̺ţ�%s�豸,��Ʊ���룺%s��Ʊ���룺%s��ѯ��Ʊʧ��", plate_num, fpdm, fphm);
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "����Ʊ�����ϴ���Ʊ", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return result;
	}
	make_invoice_ok_update_monitor_status(state, 0);//����ָ����Ʊ�ϴ�����¼����Ϣ
	return 0;
}

//���������Ϣ��
int fun_upload_hzxxb(char *plate_num, uint8 inv_type,char* inv_data, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)askfor_tzdbh_result->errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)askfor_tzdbh_result->errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, (char *)askfor_tzdbh_result->errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if ((result = port_lock(state, 20, 4, DF_PORT_USED_SQHZXXB)) != 0)
	{
		if (result == POWER_ERR_NO_DEV)
		{
			sprintf((char *)askfor_tzdbh_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("������������\n");
			sprintf((char *)askfor_tzdbh_result->errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
		}
		sprintf((char *)askfor_tzdbh_result->errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", askfor_tzdbh_result->errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����������Ϣ����Ϣ\n");		
#ifdef RELEASE_SO
		result = so_aisino_upload_hzxxb(&state->h_plate,inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_aisino_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������������Ϣ����Ϣ\n");
#ifdef RELEASE_SO
		result = so_nisec_upload_hzxxb(&state->h_plate, inv_type,inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_nisec_upload_hzxxb(&state->h_plate, inv_type,inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY���������Ϣ����Ϣ\n");
#ifdef RELEASE_SO
		result = so_cntax_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_cntax_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif
	}
	else
	{
		sprintf((char *)askfor_tzdbh_result->errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֺ�����Ϣ������", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_SQHZXXB, REFER_TO_TAX_ADDRESS, result, (char *)askfor_tzdbh_result->errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("������Ϣ������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "������Ϣ������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf((char *)askfor_tzdbh_result->errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return result;
	}
	return 0;
}

int fun_check_lzfpdm_lzfphm_allow(char *plate_num, unsigned char inv_type, char *lzfpdm, char *lzfphm, uint32 dzsyh,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if ((result = port_lock(state, 20, 4, DF_PORT_USED_HPKJLZHMJY)) != 0)
	{
		if (result == POWER_ERR_NO_DEV)
		{
			sprintf(errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("������������\n");
			sprintf((char *)errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
		}
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̺�Ʊ����ǰ����ƱУ��\n");		
#ifdef RELEASE_SO
		result = so_aisino_check_lzfpdm_lzfphm_allow(&state->h_plate, inv_type, lzfpdm, lzfphm, dzsyh,errinfo);
#else
		result = function_aisino_check_lzfpdm_lzfphm_allow(&state->h_plate, inv_type, lzfpdm, lzfphm, dzsyh,errinfo);
#endif
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֺ�Ʊ����ǰ����ƱУ��", plate_num);
		result =  -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_HPKJLZHMJY, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("��Ʊ����ǰ����ƱУ��ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��Ʊ����ǰ��ƱУ��", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

int fun_get_redinv_online_from_rednum(char *plate_num, char *rednum, uint8 **inv_json,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if ((result = port_lock(state, 20, 4, DF_PORT_USED_AXXBBHCXHZXXB)) != 0)
	{
		if (result == POWER_ERR_NO_DEV)
		{
			sprintf(errinfo, "���̺ţ�%s�ϵ�ʧ��,�����ԣ�", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("������������\n");
			sprintf((char *)errinfo, "���̺ţ�%s�ϵ�ʧ��,������������", plate_num);
		}
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰������ͨ��������Ϣ���ȡ����ķ�Ʊ\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_rednum(&state->h_plate, rednum,inv_json, errinfo);
#else
		result = function_aisino_download_hzxxb_from_rednum(&state->h_plate, rednum,inv_json, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰��������ͨ��������Ϣ���ȡ����ķ�Ʊ\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#else
		result = function_nisec_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey����ͨ��������Ϣ���ȡ����ķ�Ʊ\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#else
		result = function_cntax_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߷�Ʊ����", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_AXXBBHCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("˰�������߷�Ʊ����ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "������Ϣ���ѯ", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

#ifndef DF_OLD_MSERVER
int fun_notice_plate_update_inv_to_mserver(char *plate_num, char *invoice_month, char *summary_month, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	if (strcmp(invoice_month, summary_month) != 0)
	{
		sprintf((char *)errinfo, "֪ͨ˰���ϴ���Ʊ�ͻ�����Ϣʱ�·ݲ�һ��,�ϴ���Ʊ�·ݣ�%s,�ϴ������·ݣ�%s", invoice_month,summary_month);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_DATA_PARSING_FAILED;
	}

	state = &stream->port[port - 1];
	if (state->port_info.upload_inv_num.state == 0)
	{
		_lock_set(state->lock);

		if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
		{
			free(state->port_info.upload_inv_num.dmhmzfsbstr);
			state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
		}

		strcpy(state->port_info.upload_inv_num.invoice_month, invoice_month);
		strcpy(state->port_info.upload_inv_num.summary_month, summary_month);
		state->port_info.upload_inv_num.just_single_month = 1;		
		state->port_info.upload_inv_num.state = 1;
		_lock_un(state->lock);
		return 0;
	}
	sprintf(errinfo, "���̺ţ�%s�����ϴ������·ݷ�Ʊ,[year_month = %s],���Ժ�����", plate_num, state->port_info.upload_inv_num.invoice_month);
	return DF_TAX_ERR_CODE_PLATE_IN_USED;
}
#else
int fun_notice_plate_update_inv_to_mserver(char *plate_num, char *month, char *summary_month, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	if (state->port_info.upload_inv_num.state == 0)
	{
		_lock_set(state->lock);
		strcpy(state->port_info.upload_inv_num.year_month, month);
		strcpy(state->port_info.upload_inv_num.summary_month, summary_month);
		state->port_info.upload_inv_num.just_single_month = 1;		
		state->port_info.upload_inv_num.now_num = 0;
		state->port_info.upload_inv_num.state = 1;
		_lock_un(state->lock);
		return 0;
	}
	sprintf(errinfo, "���̺ţ�%s�����ϴ������·ݷ�Ʊ,[year_month = %s,summary_month = %s],���Ժ�����", plate_num, state->port_info.upload_inv_num.year_month, state->port_info.upload_inv_num.summary_month);
	return DF_TAX_ERR_CODE_PLATE_IN_USED;
}
#endif
int fun_change_cert_passwd(char *plate_num, char *old_passwd,char *new_passwd, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_MRKLRZ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰���޸�֤�����\n");
#ifdef RELEASE_SO
		result = so_aisino_change_certpassword(&state->h_plate, old_passwd,new_passwd, errinfo);
#else
		result = function_aisino_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�����޸�֤�����\n");
#ifdef RELEASE_SO
		result = so_nisec_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#else
		result = function_nisec_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif
	}
//	else if (sp_type == DEVICE_CNTAX) //֤��������豸�����Ѻϲ����˽ӿ���������
//	{
//		out("˰��UKEY�޸�֤�����\n");
//#ifdef RELEASE_SO
//		result = so_cntax_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
//#else
//		result = function_cntax_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
//#endif
//	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("ģ�����޸�֤�����\n");
#ifdef RELEASE_SO
		result = so_mb_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#else
		result = function_mengbai_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧���޸�֤�����", plate_num);
		result =  -1;
	}
	port_unlock(state);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("��˰���޸�֤�����ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "�޸�֤�����", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	

	char cmd[100] = { 0 };
	sprintf(cmd, "rm /tmp/passwd_err/%s.txt", (char *)plate_num);
	system(cmd);


	make_invoice_ok_update_monitor_status(state, 0);//�޸�֤���������֤����Ϣ
	return 0;
}

int fun_cancel_hzxxb_online(char *plate_num, char *rednum, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXHZXXB) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����߳���������Ϣ������\n");
#ifdef RELEASE_SO
		result = so_aisino_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_aisino_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������߳���������Ϣ������\n");
#ifdef RELEASE_SO
		result = so_nisec_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_nisec_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey���߳���������Ϣ������\n");
#ifdef RELEASE_SO
		result = so_cntax_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_cntax_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߳���������Ϣ��", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("˰�������߳���������Ϣ��ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "����������Ϣ��", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

int fun_query_rednum_from_date_range(char *plate_num, char *date_range,uint8 **rednum_data, int *rednum_count,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_ASJDCXHZXXB) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey���߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߲�ѯ������Ϣ��", plate_num);
		result = -1;
	}
	out("���߰�ʱ��β�ѯ������Ϣ�����\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_ASJDCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("˰�����߰�ʱ��β�ѯ������Ϣ��ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ʱ���ѯ������Ϣ��", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	*rednum_count = result;
	return result;//�˴�����ֵΪ������Ϣ�����������������ж�
}


int fun_query_rednum_from_date_range_new(char *plate_num, char *date_range,char *gfsh,uint8 **rednum_data, int *rednum_count,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_ASJDCXHZXXB) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_date_range_new(&state->h_plate, date_range,gfsh,rednum_data, errinfo);
#else
		result = function_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range,gfsh, rednum_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey���߰�ʱ��β�ѯ������Ϣ��\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߲�ѯ������Ϣ��", plate_num);
		result = -1;
	}
	out("���߰�ʱ��β�ѯ������Ϣ�����\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_ASJDCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("˰�����߰�ʱ��β�ѯ������Ϣ��ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ʱ���ѯ������Ϣ��", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	*rednum_count = result;
	return result;//�˴�����ֵΪ������Ϣ�����������������ж�
}

int fun_query_auth_file(char **auth_file)
{
	struct _ty_usb_m  *stream;

	stream = ty_usb_m_id;
	if (stream->cert_json == NULL)
	{
		out("��Ȩ�ļ�Ϊ��\n");
		return -1;
	}
	*auth_file = stream->cert_json;
	return 0;
}

int fun_query_net_inv_coils(char *plate_num, char *date_range,char *inv_data, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_DLYFPCX) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����߷�Ʊ���ò�ѯ\n");		
#ifdef RELEASE_SO
		result = so_aisino_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_aisino_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������߷�Ʊ���ò�ѯ\n");
#ifdef RELEASE_SO
		result = so_nisec_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_nisec_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey���߷�Ʊ���ò�ѯ\n");
#ifdef RELEASE_SO
		result = so_cntax_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_cntax_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߷�Ʊ���ò�ѯ", plate_num);
		result = -1;
	}
	out("���߷�Ʊ���ò�ѯ����\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_DLYFPCX, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("���߷�Ʊ���ò�ѯʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��Ʊ���ò�ѯ", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}
//�жϴ����ط�ƱƱԴ�Ƿ��ڿ���д���
static int jude_coils_download_is_allow(struct _port_state   *state, uint8 fplxdm, char *fpdm, char *fpqshm, int fpzs)
{
	int mem_i = 0;
	int wsyfpj_count=0;
	int fpqshm_i = atoi(fpqshm);
#ifdef RELEASE_SO
	so_common_find_inv_type_mem(fplxdm, &mem_i);
#else
	function_common_find_inv_type_mem(fplxdm, &mem_i);
#endif
	if (state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl != 0)
	{
		for (wsyfpj_count = 0; wsyfpj_count < state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl; wsyfpj_count++)
		{
			if (strcmp(fpdm, (char *)state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fplbdm) == 0)
			{
				
				if ((fpqshm_i >= state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fpqshm) &&
					(fpqshm_i <= state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fpzzhm))
				{
					out("�ҵ���ͬ�ķ�Ʊ����,�жϷ�Ʊ����������\n");
					logout(INFO, "MQTT", "��Ʊ��������", "%s,Ҫ���ص�ƱԴ�ڿ���д���,��������Ϣ����Ʊ���ͣ�%03d,��Ʊ���룺%s,��Ʊ��ʼ���룺%s,��Ʊ������%d��,"
						"�������Ϣ����Ʊ���ͣ�%03d,��Ʊ���룺%s,��Ʊ��ʼ���룺%08d,��Ʊ��ֹ���룺%08d,��Ʊ������%d��\r\n", state->port_info.port_str,fplxdm, fpdm, fpqshm, fpzs, state->port_info.plate_infos.invoice_type_infos[mem_i].fplxdm, \
						state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fplbdm,\
						state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fpqshm,\
						state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fpzzhm, \
						state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[wsyfpj_count].fpsyfs);
					return 1;
				}
			}
		}
	}
	return 0;
}

int fun_net_inv_coils_download(char *plate_num, char *date_range, uint8 fplxdm, char *fpdm, char *fpqshm, int fpzs, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s��ȡ�˿ں�\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	result = jude_coils_download_is_allow(state, fplxdm, fpdm,fpqshm, fpzs);
	if (result == 1)
	{
		//logout(INFO, "MQTT", "��Ʊ��������", "Ҫ���ص�ƱԴ�ڿ���д���\r\n");
		return 0;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_DLYFPXZAZ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�����߷�Ʊ��������\n");
#ifdef RELEASE_SO
		result = so_aisino_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_aisino_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰�������߷�Ʊ��������\n");
#ifdef RELEASE_SO
		result = so_nisec_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_nisec_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY���߷�Ʊ��������\n");
#ifdef RELEASE_SO
		result = so_cntax_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_cntax_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�����߷�Ʊ��������", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_DLYFPXZAZ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("���߷�Ʊ��������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��Ʊ��������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "��Ʊ��������ʧ��,������");
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1);
	//make_invoice_ok_update_monitor_status(state, 0);//����ָ����Ʊ�����¼����Ϣ
	//sleep(5);
	return 0;
}

int fun_test_plate_server_connect(char *plate_num,char **splxxx,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("ͨ��˰�ţ�%s���������������\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CSLJ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̼�����������\n");
#ifdef RELEASE_SO
		result = so_aisino_test_server_connect(&state->h_plate,splxxx,errinfo);
#else
		result = function_aisino_test_server_connect(&state->h_plate, splxxx,errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̼�����������\n");
#ifdef RELEASE_SO
		result = so_nisec_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_nisec_test_server_connect(&state->h_plate, splxxx,errinfo);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��UKEY������������\n");
#ifdef RELEASE_SO
		result = so_cntax_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_cntax_test_server_connect(&state->h_plate, splxxx, errinfo);
#endif
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("ģ���̼�����������\n");
#ifdef RELEASE_SO
		result = so_mb_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_mengbai_test_server_connect(&state->h_plate, splxxx, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ּ�����������", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CSLJ,REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("������������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "������������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

static int update_tax_business_info(struct _port_state   *state,char *use_info ,int if_defer_tax,int result ,char *errinfo)
{
	char time_s[30] = { 0 };
	char time_now[30] = { 0 };
	timer_read_asc_ms(time_s);
	memcpy(time_now, time_s + 1, 23);
	char connect_errinfo[1024] = {0};
	_lock_set(state->lock);
	if (state->h_plate.hDev != NULL)
	{
		if ((state->h_plate.hDev->hUSB != NULL) && (state->h_plate.hDev->hUSB->errcode == ErrorNET))
			strcpy(connect_errinfo, state->h_plate.hDev->hUSB->errinfo);
	}
	_lock_un(state->lock);

	if(result == -3) 
	{
		out("�����Ϊ����洢����,����zxbsrq�����-3���⡣");
		result = 0; //�����Ϊ����洢���⣬���µ�-3���⡣��������
	}

	memset(state->port_info.tax_business, 0, sizeof(state->port_info.tax_business));
	sprintf(state->port_info.tax_business, "%s;%s;%d;%d;%d;%s;%s;", time_now, use_info, if_defer_tax, \
		strlen(connect_errinfo) == 0 ? 1 : 0, result < 0 ? 0 : 1, connect_errinfo, errinfo);
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
		out("�ر�%d�Ŷ˿ڵ�Դ\n",port);
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
		out("��%d�Ŷ˿ڵ�Դ\n", port);
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
		out("����%d�Ŷ˿ڵ�Դ\n", port);
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



int fun_deal_clear_cert_err(char *plate_num, int mode)
{
	struct _ty_usb_m  *stream;	
	struct _port_state   *state;
	stream = ty_usb_m_id;
	int port = 0;
	if (mode == 0)
	{
		out("Ҫ�����ȫ��������Ϣ����\n");
		system("rm /tmp/passwd_err/*");
		for (port = 1; port <= stream->usb_port_numb; port++)
		{
			out("���%d�ſ���Ϣ\n",port);
			state = &stream->port[port - 1];
			make_invoice_ok_update_monitor_status(state, 0);//���֤�������󻺴�����֤����Ϣ
		}
	}
	else
	{
		out("Ҫ�����%s˰�̻�����Ϣ\n", plate_num);
		char cmd[100] = {0};
		sprintf(cmd, "rm /tmp/passwd_err/%s.txt", plate_num);
		system(cmd);
		port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
		if (port <= 0)
		{
			out("���̺�δ�ڵ�ǰ�ڴ��¼���ҵ�\n");
			return 0;
		}
		state = &stream->port[port - 1];
		make_invoice_ok_update_monitor_status(state, 0);//���֤�������󻺴�����֤����Ϣ
	}
	return 0;
}

int fun_auth_key_info(char *plate_num, char *key_info,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	//out("ͨ��˰�ţ�%s���������������\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	int sp_type = state->port_info.usb_app;
	if (sp_type == DEVICE_MENGBAI2)
	{
		if (port_lock(state, 20, 3, DF_PORT_USED_HQFWSJMXX) != 0)
		{
			sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}

#ifdef RELEASE_SO
		result = so_mb_server_key_info(&state->h_plate, key_info);
#else
		result = function_mengbai_server_key_info(&state->h_plate, key_info);
#endif
		port_unlock(state);
		if (result < 0)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "��Ȩkey��Ϣ", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
				sprintf(errinfo, "USB����ʧ��,������");
				restart_plate_power(state, (uint8 *)plate_num, port, 1);
			}
			sprintf(errinfo, "��ȡ��Ȩ��Կ��Ϣʧ��");
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		return result;
	}
	sprintf(errinfo, "�ÿ��ţ�%s��֧��", plate_num);
	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;

}

//���빫������ƽ̨
int fun_connect_pubservice(char *plate_num, uint8 inv_type,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_SQJRGGFWPT) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̽��빫������ƽ̨\n");
#ifdef RELEASE_SO
		result = so_aisino_connect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_aisino_connect_pubservice(&state->h_plate, inv_type, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̽��빫������ƽ̨\n");
#ifdef RELEASE_SO
		result = so_nisec_connect_pubservice(&state->h_plate, inv_type,errinfo);
#else
		result = function_nisec_connect_pubservice(&state->h_plate, inv_type, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֽ��빫������ƽ̨�ӿ�", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_SQJRGGFWPT, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("���빫������ƽ̨ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "���빫������ƽ̨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1);//������Դ����˰��״̬
	return 0;
}
//ȡ�����빫������ƽ̨
int fun_disconnect_pubservice(char *plate_num, uint8 inv_type,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXJRGGFWPT) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰��ȡ�����빫������ƽ̨\n");
#ifdef RELEASE_SO
		result = so_aisino_disconnect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_aisino_disconnect_pubservice(&state->h_plate, inv_type, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰����ȡ�����빫������ƽ̨\n");
#ifdef RELEASE_SO
		result = so_nisec_disconnect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_nisec_disconnect_pubservice(&state->h_plate, inv_type, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧��ȡ�����빫������ƽ̨�ӿ�", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXJRGGFWPT, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("ȡ�����빫������ƽ̨ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "ȡ�����빫������ƽ̨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo,"USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1); //������Դ����˰��״̬
	return 0;
}

//��������������--hello
int fun_client_hello(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
#ifndef ALL_SHARE
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
#else
	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
		else
			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->h_plate.hDev->postMessage_flag = postMessage_flag;
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	/*if (NULL == state->h_plate)								//wang 01-17 �鿴 �̺ţ�917005374751,���ƣ�����������ά�������޹�˾ Busnum��Devnum��Ϣ�����������
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "h_plate ��������仯\r\n");
	}*/
	if (NULL == state->h_plate.hDev)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hDev ��������仯\r\n");
	}
	if (NULL == state->h_plate.hDev->hUSB)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hUSB ��������仯\r\n");
	}	
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ�������������������\n");
#ifdef RELEASE_SO
		result = so_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ�������������������\n");
#ifdef RELEASE_SO
		result = so_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ�������������������\n");		
#ifdef RELEASE_SO
		result = so_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ�������������������", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ�������������������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ�������������������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}

//������������֤--auth
int fun_client_auth(char *plate_num,char* server_hello, char *client_auth, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
#ifndef ALL_SHARE
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
#else
	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
		else
			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJRZ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_nisec_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_nisec_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_cntax_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_cntax_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ�������������֤����", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ�������������֤����ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ�������������֤����", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}

//��ȡ֤��
int fun_get_cert(char *plate_num, char *cert_data, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
#ifndef ALL_SHARE
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		return result;
	}
#else
	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
		else
			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ֤������", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ֤������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ֤������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}



/////
//��������������--hello
int fun_client_hello_d(char *plate_num, char *client_hello, char *errinfo,int postMessage_flag)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
// #ifndef ALL_SHARE
// 	result = jude_port_auth_passwd(state, errinfo);
// 	if (result < 0)
// 	{
// 		return result;
// 	}
// #else
// 	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
// 	{
// 		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
// 			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
// 		else
// 			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->h_plate.hDev->postMessage_flag = postMessage_flag;
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	/*if (NULL == state->h_plate)								//wang 01-17 �鿴 �̺ţ�917005374751,���ƣ�����������ά�������޹�˾ Busnum��Devnum��Ϣ�����������
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "h_plate ��������仯\r\n");
	}*/
	if (NULL == state->h_plate.hDev)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hDev ��������仯\r\n");
	}
	if (NULL == state->h_plate.hDev->hUSB)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hUSB ��������仯\r\n");
	}	
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ�������������������\n");
#ifdef RELEASE_SO
		result = so_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ�������������������\n");
#ifdef RELEASE_SO
		result = so_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ�������������������\n");		
#ifdef RELEASE_SO
		result = so_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ�������������������", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ�������������������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ�������������������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}

//������������֤--auth
int fun_client_auth_d(char *plate_num,char* server_hello, char *client_auth, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
// #ifndef ALL_SHARE
// 	result = jude_port_auth_passwd(state, errinfo);
// 	if (result < 0)
// 	{
// 		return result;
// 	}
// #else
// 	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
// 	{
// 		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
// 			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
// 		else
// 			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJRZ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_nisec_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_nisec_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ�������������֤����\n");
#ifdef RELEASE_SO
		result = so_cntax_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_cntax_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ�������������֤����", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ�������������֤����ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ�������������֤����", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}

//��ȡ֤��
int fun_get_cert_d(char *plate_num, char *cert_data, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
// #ifndef ALL_SHARE
// 	result = jude_port_auth_passwd(state, errinfo);
// 	if (result < 0)
// 	{
// 		return result;
// 	}
// #else
// 	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
// 	{
// 		if (state->port_info.plate_infos.plate_basic_info.cert_err_reason == ERR_CERT_PASSWORD_ERR)
// 			sprintf(errinfo, "˰��Ĭ��֤������ȷ,ȷ��֤������Ƿ�Ϊ12345678,�����޸�����������");
// 		else
// 			sprintf(errinfo, "˰��֤������쳣,��ȷ��˰�����쳣,����˰�����ºͽӿ�");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̻�ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̻�ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ȡ֤������\n");
#ifdef RELEASE_SO
		result = so_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ֻ�ȡ֤������", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("��ȡ֤������ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ȡ֤������", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	return 0;
}
/////


static int get_http_first_line(char *http_data, char *buf, int size)
{
	char *p;
	p = strstr(http_data,"\r\n");
	if (p == NULL)
	{
		//out("http�������󣬲����ڻس����з�,����Ƿ���ڻ��з�\r\n");
		p = strstr(http_data, "\n");
		if (p == NULL)
		{
			out("http�������󣬲����ڻ��з�\r\n");
			return -1;
		}
		
	}
	int buf_len = p - http_data;
	if (buf_len > size)
	{
		out("http�������󣬵������ݳ��������С\r\n");
		return -2;
	}
	memcpy(buf, http_data, buf_len);
	return buf_len;
}

static int get_http_body_line(char *http_data, char *buf, int size)
{
	char *p;
	p = strstr(http_data, "\r\n\r\n");
	if (p == NULL)
	{
		//out("http�������󣬲����������س����з�,����Ƿ�����������з�\r\n");
		p = strstr(http_data, "\n\n");
		if (p == NULL)
		{
			out("http�������󣬲������������з�\r\n");
			return -1;
		}
		else
		{
			p += strlen("\n\n");
		}
	}
	else
	{
		p += strlen("\r\n\r\n");
	}

	int buf_len = strlen(http_data) - (p - http_data);
	if (buf_len > size)
	{
		out("http�������󣬵������ݳ��������С\r\n");
		return -2;
	}
	memcpy(buf, p, buf_len);
	return buf_len;
}

static int response_order_tran(char *buf, int buf_len, char *http_base)
{   //uint8 s_buf[409600];
	char *s_buf = NULL;
	//int len,result;
	//out("�ظ����ݣ�\n");
	s_buf = calloc(1, buf_len + 1024);
	if (s_buf == NULL)
	{
		out("��������ڴ�ʧ��\n");
		return -1;
	}
	memset(s_buf, 0, buf_len + 1024);
	/*��Ӧ�ͻ�����ȷ�� HTTP ���� */
	//sprintf(s_buf, "HTTP/1.0 400 BAD REQUEST\r\n");  
	sprintf(s_buf, "HTTP/1.1 200 OK\r\n");
	sprintf(s_buf + strlen(s_buf), "Server: CHINATAX_CRYPT/1.4\r\n");
	sprintf(s_buf + strlen(s_buf), "Access-Control-Methods: POST\r\n");
	sprintf(s_buf + strlen(s_buf), "Access-Control-Allow-Origin: *\r\n");
	sprintf(s_buf + strlen(s_buf), "Connection: keep-alive\r\n");
	sprintf(s_buf + strlen(s_buf), "Content-Type: application/json;charset=UTF-8\r\n");
	sprintf(s_buf + strlen(s_buf), "Content-Length: %d\r\n", buf_len);
	sprintf(s_buf + strlen(s_buf), "\r\n");
	sprintf(s_buf + strlen(s_buf), "%s", buf);
	//printf("return :%s\n", s_buf);
	
	base64_enc((uint8 *)http_base, (uint8 *)s_buf, strlen(s_buf));

	free(s_buf);
	//send(fd, s_buf, sizeof(s_buf), 0);  
	return 0;
}

static int response_order_tran_postMessage(char *buf, int buf_len, char *http_base)
{   //uint8 s_buf[409600];
	char *s_buf = NULL;
	//int len,result;
	//out("�ظ����ݣ�\n");
	s_buf = calloc(1, buf_len + 1024);
	if (s_buf == NULL)
	{
		out("��������ڴ�ʧ��\n");
		return -1;
	}
	memset(s_buf, 0, buf_len + 1024);
	/*��Ӧ�ͻ�����ȷ�� HTTP ���� */
	//sprintf(s_buf, "HTTP/1.0 400 BAD REQUEST\r\n");  
	sprintf(s_buf, "HTTP/1.1 200 OK\r\n");
	sprintf(s_buf + strlen(s_buf), "Server: CHINATAX_CRYPT/1.4\r\n");
	sprintf(s_buf + strlen(s_buf), "Access-Control-Methods: POST\r\n");
	sprintf(s_buf + strlen(s_buf), "Access-Control-Allow-Origin: *\r\n");
	sprintf(s_buf + strlen(s_buf), "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n");
	sprintf(s_buf + strlen(s_buf), "Connection: keep-alive\r\n");
	sprintf(s_buf + strlen(s_buf), "Content-Type: text/html;charset=UTF-8\r\n");
	sprintf(s_buf + strlen(s_buf), "Content-Length: %d\r\n", buf_len);
	sprintf(s_buf + strlen(s_buf), "\r\n");
	sprintf(s_buf + strlen(s_buf), "%s", buf);
	printf("return :%s\n", s_buf);

	base64_enc((uint8 *)http_base, (uint8 *)s_buf, strlen(s_buf));

	free(s_buf);
	return 0;
}

static int deal_get_version_order(char *plate_num, char *body_data, char *http_base)
{

	char *s_buf_gbk;
	char s_buf_utf8[1024] = {0};
	int postMessage_flag = 0;
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);

	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("����˰��ַ�ʽ��¼\n");
		postMessage_flag = 1;
	}

	cJSON *json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "��ȡ�汾�ųɹ�");
	cJSON_AddStringToObject(json, "version", "1.4");
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	if (postMessage_flag == 0)
		response_order_tran(s_buf_utf8, strlen(s_buf_utf8), http_base);
	else
	{
		char postMessage[5120] = { 0 };
		sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);
		response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	}
	return 0;
}


static int deal_read_cert_info(char *plate_num, char *body_data, char *http_base)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	char *s_buf_gbk;
	char s_buf_utf8[1024];
	int postMessage_flag = 0;
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("����˰��ַ�ʽ��¼\n");
		postMessage_flag = 1;
	}
	cJSON *json = cJSON_CreateObject();
	stream = ty_usb_m_id;
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "���豸ʧ��-δ��USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		cJSON_AddNumberToObject(json, "code", 0);
		cJSON_AddStringToObject(json, "msg", "��ȡ֤����Ϣ�ɹ�");
		if (strstr(body_data, "certInfoNo=71") != NULL)
		{
			cJSON_AddStringToObject(json, "certInfo", (char *)state->port_info.ca_serial);
		}
		else if (strstr(body_data, "certInfoNo=27") != NULL)
		{
			cJSON_AddStringToObject(json, "certInfo", (char *)state->port_info.ca_name);
		}
		else if (strstr(body_data, "certInfoNo=70") != NULL)
		{
			char device_sn[100] = {0};
			if (state->port_info.usb_app == DEVICE_AISINO)
				sprintf(device_sn, "44%s", state->port_info.sn);
			else if (state->port_info.usb_app == DEVICE_NISEC)
				sprintf(device_sn, "33%s", state->port_info.sn);
			else if (state->port_info.usb_app == DEVICE_CNTAX)
				sprintf(device_sn, "03-66%s", state->port_info.sn);
			cJSON_AddStringToObject(json, "certInfo", device_sn);
		}
	}	
	s_buf_gbk = cJSON_PrintUnformatted(json);
	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	if (postMessage_flag == 0)
		response_order_tran(s_buf_utf8, strlen(s_buf_utf8), http_base);
	else
	{
		char postMessage[5120] = { 0 };
		sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);
		response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	}
	return 0;
}

static int deal_order_verify_pin(char *plate_num, char *body_data, char *http_base)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	char *s_buf_gbk;
	char s_buf_utf8[1024];
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);
	stream = ty_usb_m_id;
	cJSON *json = cJSON_CreateObject();

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "���豸ʧ��-δ��USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		if ((state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0) || 
			(strstr(body_data,"password=12345678&dwProvType=2050&strContainer=%2F%2FSM2%2FSM2CONTAINER0002") == NULL))
		{
			out("��¼��Ĭ�Ͽ������\n");
			cJSON_AddNumberToObject(json, "code", 805306370);
			cJSON_AddStringToObject(json, "msg", "��֤����ʧ��--δ֪����(0x30000002)");
		}
		else
		{
			cJSON_AddNumberToObject(json, "code", 0);
			cJSON_AddStringToObject(json, "msg", "��֤����ɹ�");
		}
	}	
	s_buf_gbk = cJSON_PrintUnformatted(json);
	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);
	response_order_tran(s_buf_utf8, strlen(s_buf_utf8), http_base);
	return 0;
}


static int deal_order_client_hello(char *plate_num, char *body_data, char *http_base)
{
	int result;
	char *s_buf_gbk;
	char s_buf_utf8[1024];
	char client_hello[5120] = { 0 };
	char errinfo[1024] = { 0 };
	int postMessage_flag=0,auth_flag = 0;
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("����˰��ַ�ʽ��¼\n"); authType=0
		postMessage_flag = 1;
	}
	p_start = strstr(body_data, "authType=1");
	if(p_start != NULL)
	{
		auth_flag = 1;
	}
	//printf("enter fun_client_hello\n");
	result = fun_client_hello(plate_num, client_hello, errinfo,auth_flag);
	if (result < 0)
	{
		out("��ȡclient_helloʧ�ܣ�errinfo��%s\n", errinfo);
	}
	//printf("exit fun_client_hello\n");
	cJSON *json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "code", 0);
	if (auth_flag == 0)
		cJSON_AddStringToObject(json, "msg", "���ɿͻ�����֤����--������֤��ʽ�ɹ�");
	else
		cJSON_AddStringToObject(json, "msg", "���ɿͻ�����֤����--˫����֤��ʽ�ɹ�");
	cJSON_AddStringToObject(json, "clientHello", client_hello);
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	if (postMessage_flag == 0)
		response_order_tran(s_buf_utf8, strlen(s_buf_utf8), http_base);
	else
	{
		char postMessage[5120] = { 0 };
		sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);
		response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	}
	return 0;
}

static int deal_order_client_auth(char *plate_num,char *body_data,char *http_base)
{
	int result;
	char *s_buf_gbk;
	char s_buf_utf8[4096] = { 0 };
	char server_hello[5120] = { 0 };
	char client_auth[5120] = { 0 };
	char errinfo[1024] = { 0 };
	int postMessage_flag = 0;
	out("�����������,����Ϊ��%d\n", strlen(body_data));
	out("�������ݣ�%s\n", body_data);
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		postMessage_flag = 1;
	}

	cJSON *json = cJSON_CreateObject();
	p_start = strstr(body_data, "serverHello=");
	if (p_start == NULL)
	{
		out("��������ݽ���ʧ��,ȱ��serverHello=\n");
		goto End;
	}
	p_start += strlen("serverHello=");
	char *p_end;
	p_end = strstr(p_start,"&");
	if (p_end == NULL)
	{
		out("��������ݽ���ʧ��,ȱ��&\n");
		goto End;
	}
	// if (postMessage_flag == 0)
	// {
		
	// 	p_end = strstr(p_start, "&dwProvType");
	// 	if (p_end == NULL)
	// 	{
	// 		out("��������ݽ���ʧ��,ȱ��&dwProvType\n");
	// 		goto End;
	// 	}
	// }
	// else
	// {
	// 	p_end = strstr(p_start, "&password");
	// 	if (p_end == NULL)
	// 	{
	// 		out("��������ݽ���ʧ��,ȱ��&password\n");
	// 		goto End;
	// 	}
	// }

	memcpy(server_hello, p_start, p_end - p_start);

	//out("server_hello = %s\n", server_hello);


	result = fun_client_auth(plate_num, server_hello, client_auth, errinfo);
	if (result < 0)
	{
		out("��ȡclient_helloʧ�ܣ�errinfo��%s\n", errinfo);
		goto End;
	}

End:
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "���ɿͻ�����֤ƾ�ݳɹ�");
	cJSON_AddStringToObject(json, "clientAuth", client_auth);
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	if (postMessage_flag == 0)
		response_order_tran(s_buf_utf8, strlen(s_buf_utf8), http_base);
	else
	{
		char postMessage[5120] = { 0 };
		sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);
		response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	}
	return 0;
}

static int deal_order_check_key(char *plate_num, char *body_data, char *http_base)
{
	//int result;
	char *s_buf_gbk;
	char s_buf_utf8[4096] = { 0 };
	char postMessage[5120] = { 0 };
	//char errinfo[1024] = { 0 };

	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("��������ݽ���ʧ��,ȱ��crosFlag=\n");
		return -1;
	}
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "�豸�Ѳ�������");
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);

	response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	return 0;
}


static int deal_order_get_device_num(char *plate_num, char *body_data, char *http_base)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	char *s_buf_gbk;
	char s_buf_utf8[4096] = { 0 };
	char postMessage[5120] = { 0 };
	//char errinfo[1024] = { 0 };
	stream = ty_usb_m_id;
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("��������ݽ���ʧ��,ȱ��crosFlag=\n");
		return -1;
	}

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "���豸ʧ��-δ��USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		cJSON_AddNumberToObject(json, "code", 0);
		cJSON_AddStringToObject(json, "msg", "��ȡ�豸�ųɹ�");


		char device_sn[100] = { 0 };
		if (state->port_info.usb_app == DEVICE_AISINO)
			sprintf(device_sn, "44%s", state->port_info.sn);
		else if (state->port_info.usb_app == DEVICE_NISEC)
			sprintf(device_sn, "33%s", state->port_info.sn);
		else if (state->port_info.usb_app == DEVICE_CNTAX)
			sprintf(device_sn, "66%s", state->port_info.sn);

		cJSON_AddStringToObject(json, "deviceNum", device_sn);
	}
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);

	response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	return 0;
}


static int deal_order_read_cert(char *plate_num, char *body_data, char *http_base)
{
	//struct _ty_usb_m  *stream;
	//struct _port_state   *state;
	//int port;
	//int result;
	char *s_buf_gbk;
	char s_buf_utf8[4096] = { 0 };
	char postMessage[5120] = { 0 };
	char cert_data[4096] = { 0 };
	char errinfo[1024] = { 0 };
	//stream = ty_usb_m_id;
	//out("�����������,����Ϊ��%d\n", strlen(body_data));
	//out("�������ݣ�%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("��������ݽ���ʧ��,ȱ��crosFlag=\n");
		return -1;
	}
	fun_get_cert(plate_num, cert_data, errinfo);
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "��ȡ����֤��ɹ�");
	cJSON_AddStringToObject(json, "cert", cert_data);//��cert������Դ�����
	
	s_buf_gbk = cJSON_PrintUnformatted(json);

	gbk2utf8((uint8 *)s_buf_gbk, (uint8 *)s_buf_utf8);
	free(s_buf_gbk);
	cJSON_Delete(json);

	sprintf(postMessage, "<script>parent.postMessage('%s','*');</script>", s_buf_utf8);

	response_order_tran_postMessage(postMessage, strlen(postMessage), http_base);
	return 0;
}


int fun_zzsfpzhfwpt_auth_hello_data_tran(char *plate_num, char *http_data_base, int https_size)
{
	char buf[1024] = { 0 };
	char method[255];
	char url[255];
	char body_data[1024 * 20] = {0};
	size_t i, j;
	int http_len;
	char *http_data;
	http_len = strlen(http_data_base) + 1024;
	http_data = calloc(1, http_len);
	base64_dec((uint8 *)http_data, (uint8 *)http_data_base, http_len);
	memset(http_data_base, 0, https_size);
	
	out("fun_zzsfpzhfwpt_auth_hello_data_tran http�������ݣ�%s\n", http_data);

	memset(method, 0, sizeof(method));
	memset(url, 0, sizeof(url));
	memset(buf, 0, sizeof(buf));

	get_http_first_line(http_data, buf, sizeof(buf));

	get_http_body_line(http_data, body_data, sizeof(body_data));

	i = 0; j = 0;
	/*�ѿͻ��˵����󷽷��浽 method ����*/
	while (!ISspace(buf[j]) && (i < sizeof(method)-1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';
	//out("method = %s\n", method);

	/*������� POST ���޷����� */
	if (strcasecmp(method, "POST") != 0)
	{
		//unimplemented(client);  
		return -1;
	}

	/*��ȡ url ��ַ*/
	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url)-1) && (j < sizeof(buf)))
	{
		/*���� url */
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';
	//out("���յ�������URLΪ��%s\n", url);
	out("��������Ϊ%s\n", url);


	if (strstr(url, DF_ORDER_GET_VERSION)!= NULL)
	{
		deal_get_version_order(plate_num,body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_READ_CERTINFO) != NULL)
	{
		deal_read_cert_info(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_VERIFRY_PIN) != NULL)
	{
		deal_order_verify_pin(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_CLIENT_HELLO) != NULL)
	{
		deal_order_client_hello(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_CLIENT_AUTH) != NULL)
	{
		deal_order_client_auth(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_CHECK_KEY) != NULL)
	{
		deal_order_check_key(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_GET_DEVICE_NUM) != NULL)
	{
		deal_order_get_device_num(plate_num, body_data, http_data_base);
	}
	else if (strstr(url, DF_ORDER_READ_CERT) != NULL)
	{
		deal_order_read_cert(plate_num, body_data, http_data_base);
	}
	else
	{
		out("���֧�֣�url = %s\n", url);
		return -1;
	}
	return 0;	
}




//���»�ȡ˰�̻�����Ϣ�ӿ�
int fun_query_invoice_month_all_data(char *plate_num, char *month, char **data_json, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXHZSJ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("��˰�̰��»�ȡ˰�̻�����Ϣ�ӿ�\n");
#ifdef RELEASE_SO
		result = so_aisino_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_aisino_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰���̰��»�ȡ˰�̻�����Ϣ�ӿ�\n");
#ifdef RELEASE_SO
		result = so_nisec_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_nisec_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey���»�ȡ˰�̻�����Ϣ�ӿ�\n");
#ifdef RELEASE_SO
		result = so_cntax_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_cntax_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧�ְ��»�ȡ˰�̻�����Ϣ�ӿ�", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZSJ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("ȡ�����빫������ƽ̨ʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "ȡ�����빫������ƽ̨", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

//��ȡ��ʷ��Ʊ��Ϣ
int fun_query_history_invoice_coils(char *plate_num, char **data_json, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXHZSJ) != 0)
	{
		sprintf((char *)errinfo, "˰������ʹ����,���Ժ�����,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		sprintf((char *)errinfo, "��˰���ݲ�֧����ʷ��Ʊ��Ϣ��ѯ");
		return DF_TAX_ERR_CODE_TER_SYSTEM_ERROR;
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("˰������ʷ��Ʊ��Ϣ��ѯ�ӿ�\n");
#ifdef RELEASE_SO
		result = so_nisec_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#else
		result = function_nisec_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("˰��ukey��ʷ��Ʊ��Ϣ��ѯ�ӿ�\n");
#ifdef RELEASE_SO
		result = so_cntax_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#else
		result = function_cntax_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "���̺ţ�%s�豸,�ݲ�֧����ʷ��Ʊ��Ϣ��ѯ", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZSJ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	state->last_act = 1;
	if (result < 0)
	{
		out("��ʷ��Ʊ��Ϣ��ѯʧ��\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "��ʷ��Ʊ��Ϣ��ѯ", "%s,Busnum��Devnum��Ϣ�������,�������˿�\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB����ʧ��,������");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
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
	memset(tmp_s,0x00,sizeof(tmp_s));
	sprintf(tmp_s,"%d",stream->up_queue_num);
	cJSON_AddStringToObject(data_layer, "queue_num", tmp_s);
	memset(tmp_s,0x00,sizeof(tmp_s));
	sprintf(tmp_s,"%d",stream->base_read_queue_num);
	cJSON_AddStringToObject(data_layer, "base_read_queue_num", tmp_s);
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

		//out("�ն˻�����Ϣ������\n");
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
		//out("�ն�ģ��֧��������\n");
	
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
	//out("�ն����ڹ���������\n");
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




		//out("�ն�mysql����������\n");
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
		//out("�ն���չ�ڴ����������\n");
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
		//out("�ն�ftp����������\n");
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
		//out("�ն�ssh����������\n");
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
		//out("�ն�ntpdate����������\n");
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
		//out("�ն�tfcard����������\n");
		if (atoi(mode_support.event_log_support) != 0)
		{

			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "event_log_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "event_log_enable", mode_enable.event_log_enable);
			cJSON_AddStringToObject(layer, "event_log_num", "0");
			cJSON_AddStringToObject(layer, "event_log", "");
		}
		//out("�ն��¼���־����������\n");
		if (atoi(mode_support.usbshare_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "usbshare_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "usbshare_enable", mode_enable.usbshare_enable);
			cJSON_AddStringToObject(layer, "port", "3240");
			cJSON_AddStringToObject(layer, "stop_use", "0");
		}
		//out("�ն�usbshare����������\n");
		if (atoi(mode_support.device_manage_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "device_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "device_manage_enable", mode_enable.device_manage_enable);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
		}
		//out("�ն��豸�������������\n");
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
		//out("�ն�mqtt����������\n");
		if (atoi(mode_support.tax_invoice_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tax_invoice_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tax_invoice_enable", mode_enable.tax_invoice_enable);
		}
		//out("�ն�˰��Ʊ����������\n");
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
		//out("�ն�CA���̹���������\n");
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
	//out("�ն��û���¼���̹���������\n");
	cJSON *usblayer;
	

	int port;
	int err_num = 0;
	char usb_port_num[10] = { 0 };
	char without_dev_port_num[10] = { 0 };
	char without_dev_ports[500] = { 0 };
	cJSON_AddItemToObject(data_layer, "usb_port_manage", usblayer = cJSON_CreateObject());
	int port_num = stream->usb_port_numb;
	//out("���ն�һ����%d��USB�˿�\n");
	for (port = 1; port<= port_num; port++)
	{
		if (stream->port[port - 1].port_info.with_dev != 1)
		{
			//out("%d�Ŷ˿�û���豸\n", port_count + 1);
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
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.queue_state.use_flag);
		cJSON_AddStringToObject(port_layer, "queue_usedflag", tmp_s);
		cJSON_AddStringToObject(port_layer, "queue_start_time", state->port_info.queue_state.start_time);
		cJSON_AddStringToObject(port_layer, "queue_end_time", state->port_info.queue_state.end_time);
		cJSON_AddStringToObject(port_layer, "queue_summary_month", state->port_info.queue_state.summary_month);
		cJSON_AddStringToObject(port_layer, "busid", state->port_info.busid);		
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.with_dev);
		cJSON_AddStringToObject(port_layer, "dev_exists", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->power);
		cJSON_AddStringToObject(port_layer, "port_power", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.ad_status);
		//printf("fun_get_ter_all_data_json_jude_need_port port=%d, ad_status=%s\n",state->port,tmp_s);
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
		cJSON_AddStringToObject(port_layer, "usb_app", tmp_s);
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
				cJSON *ca_layer;
				cJSON_AddItemToObject(port_layer, "ca_infos", ca_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(ca_layer, "name", (char *)state->port_info.ca_name);
				cJSON_AddStringToObject(ca_layer, "number", (char *)state->port_info.ca_serial);
				cJSON_AddStringToObject(ca_layer, "cert_file_name", "");
				cJSON_AddStringToObject(ca_layer, "cert_file_data", "");
			}
			else if ((state->port_info.usb_app == 1) || (state->port_info.usb_app == 2) || (state->port_info.usb_app == 3) || state->port_info.usb_app >= 101)
			{
				cJSON *plate_layer;
				cJSON_AddItemToObject(port_layer, "plate_infos", plate_layer = cJSON_CreateObject());
				if (need_tax_info == 0)
				{
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port);
					cJSON_AddStringToObject(plate_layer, "port_num", tmp_s);
					//cJSON_AddStringToObject(plate_layer, "busid", state->port_info.busid);
					//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.usb_app);
					cJSON_AddStringToObject(plate_layer, "usb_app", tmp_s);
					cJSON_AddStringToObject(plate_layer, "name", (char *)state->port_info.ca_name);
					cJSON_AddStringToObject(plate_layer, "number", (char *)state->port_info.ca_serial);
					cJSON_AddStringToObject(plate_layer, "plate_num", (char *)state->port_info.sn);
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

	cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%d", stream->usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
	cJSON_AddStringToObject(data_layer, "internet_ip", net_status.internet_ip);
	cJSON_AddStringToObject(data_layer, "longitude", net_status.longitude);
	cJSON_AddStringToObject(data_layer, "latitude", net_status.latitude);
	cJSON_AddStringToObject(data_layer, "area_info", net_status.area_info);
#ifndef DF_OLD_MSERVER 
	fun_get_ter_all_data_json_jude_need_port(data_layer, 1, 1);
#else
	cJSON_AddStringToObject(data_layer, "product_code", ter_info.product_code);
	cJSON_AddStringToObject(data_layer, "product_name", ter_info.product_name);
	cJSON_AddStringToObject(data_layer, "product_rename", ter_info.product_rename);
	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	cJSON_AddStringToObject(data_layer, "cpu_id", ter_info.cpu_id);
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
#endif

	cJSON *dir4, *dir5;
	cJSON_AddItemToObject(data_layer, "plate_function", dir4 = cJSON_CreateArray());


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

	
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port);
	
	cJSON_AddStringToObject(data_layer, "port_num", tmp_s);


	cJSON_AddStringToObject(data_layer, "busid", state->port_info.busid);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.with_dev);
	cJSON_AddStringToObject(data_layer, "dev_exists", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.usb_app);
	cJSON_AddStringToObject(data_layer, "usb_app", tmp_s);
	
	cJSON_AddStringToObject(data_layer, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(data_layer, "number", (char *)state->port_info.ca_serial);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.dev_type);
	cJSON_AddStringToObject(data_layer, "usb_type", tmp_s);
	cJSON_AddStringToObject(data_layer, "plate_num", (char *)state->port_info.sn);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.extension);
	cJSON_AddStringToObject(data_layer, "plate_extension", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_basic_info.plate_test);
	cJSON_AddStringToObject(data_layer, "plate_train", tmp_s);


	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.vid);
	cJSON_AddStringToObject(data_layer, "usb_vid", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.pid);
	cJSON_AddStringToObject(data_layer, "usb_pid", tmp_s);


	//int busnum = 0;
	//int devnum = 0;
	//busnum = get_busid_busnum(state->port_info.busid);
	//devnum = get_busid_devnum(state->port_info.busid);
	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", busnum);
	//cJSON_AddStringToObject(data_layer, "usb_busnum", tmp_s);
	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", devnum);
	//cJSON_AddStringToObject(data_layer, "usb_devnum", tmp_s);
	
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->power);
	cJSON_AddStringToObject(data_layer, "port_power", tmp_s);
	
#ifndef DF_OLD_MSERVER
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->now_report_counts);
	cJSON_AddStringToObject(data_layer, "report_serial", tmp_s);
#endif
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
	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.usb_share_mode);
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
#ifndef DF_OLD_MSERVER
	if (state->port_info.upload_inv_num.state == 1)
#else
	if ((state->port_info.upload_inv_num.need_fpdmhm_flag == 1) || (state->port_info.upload_inv_num.auto_up_end == 0))
#endif
	{
		cJSON_AddStringToObject(data_layer, "up_mserver_complete", "0");
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "up_mserver_complete", "1");
	}

	if (state->port_info.plate_infos.plate_tax_info.off_inv_num == 0)
	{
		cJSON_AddStringToObject(data_layer, "offline_inv_exsit", "0");
		cJSON_AddStringToObject(data_layer, "offline_inv_info", "");
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "offline_inv_exsit", "1");
		char offline_inv_info[1024] = { 0 };
		sprintf(offline_inv_info, "%s||%s||%s||%d", state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, state->port_info.plate_infos.plate_tax_info.off_inv_fphm, \
			state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh);
		cJSON_AddStringToObject(data_layer, "offline_inv_info", offline_inv_info);
	}
	if (state->port_info.offinv_stop_upload == 0)
		cJSON_AddStringToObject(data_layer, "offline_inv_stop_upload", "0");
	else
		cJSON_AddStringToObject(data_layer, "offline_inv_stop_upload", "1");
	cJSON_AddStringToObject(data_layer, "offline_inv_status", (char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status);
	cJSON_AddStringToObject(data_layer, "plate_using_info", (char *)state->port_info.used_info);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_inv_state);
	cJSON_AddStringToObject(data_layer, "m_inv_state", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_tzdbh_state);
	cJSON_AddStringToObject(data_layer, "m_tzdbh_state", tmp_s);
	//memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.m_netinv_state);
	//cJSON_AddStringToObject(data_layer, "netinv_m_state", tmp_s);




	cJSON_AddStringToObject(data_layer, "tax_nature", (char *)state->port_info.plate_infos.plate_tax_info.tax_nature);
	cJSON_AddStringToObject(data_layer, "tax_office_name", (char *)state->port_info.plate_infos.plate_tax_info.tax_office_name);
	cJSON_AddStringToObject(data_layer, "tax_office_code", (char *)state->port_info.plate_infos.plate_tax_info.tax_office_code);
	cJSON_AddStringToObject(data_layer, "tax_server_url", (char *)state->port_info.plate_infos.plate_tax_info.tax_server_url);
	cJSON_AddStringToObject(data_layer, "integrated_server_url", (char *)state->port_info.plate_infos.plate_tax_info.integrated_server_url);

	cJSON *connect_layer;
	cJSON_AddItemToObject(data_layer, "connect_tax_server", connect_layer = cJSON_CreateObject());
	_lock_set(state->lock);
	if (state->h_plate.hDev != NULL)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_success_count);
		cJSON_AddStringToObject(connect_layer, "success_count", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_failed_count);
		cJSON_AddStringToObject(connect_layer, "failed_count", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_success_time);
		cJSON_AddStringToObject(connect_layer, "success_msec", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->h_plate.hDev->connect_failed_time);
		cJSON_AddStringToObject(connect_layer, "failed_msec", tmp_s);
		if(strlen(state->h_plate.hDev->DevHttp.start_time_s)!= 0)
			cJSON_AddStringToObject(connect_layer, "last_start_time", state->h_plate.hDev->DevHttp.start_time_s);
		else
			cJSON_AddStringToObject(connect_layer, "last_start_time", "");
		if (strlen(state->h_plate.hDev->DevHttp.end_time_s) != 0)
			cJSON_AddStringToObject(connect_layer, "last_end_time", state->h_plate.hDev->DevHttp.end_time_s);
		else
			cJSON_AddStringToObject(connect_layer, "last_end_time", "");
		if ((state->h_plate.hDev->hUSB != NULL) && (state->h_plate.hDev->hUSB->errcode == ErrorNET) && (strlen(state->h_plate.hDev->hUSB->errinfo) != 0))
			cJSON_AddStringToObject(connect_layer, "last_errinfo", state->h_plate.hDev->hUSB->errinfo);
		else
			cJSON_AddStringToObject(connect_layer, "last_errinfo", "");
		cJSON_AddStringToObject(connect_layer, "last_tax_business", state->port_info.tax_business);
	}
	else
	{
		cJSON_AddStringToObject(connect_layer, "success_count", "0");
		cJSON_AddStringToObject(connect_layer, "failed_count", "0");
		cJSON_AddStringToObject(connect_layer, "success_msec", "0");
		cJSON_AddStringToObject(connect_layer, "failed_msec", "0");
		cJSON_AddStringToObject(connect_layer, "last_start_time", "");
		cJSON_AddStringToObject(connect_layer, "last_end_time", "");
		cJSON_AddStringToObject(connect_layer, "last_errinfo", "");
		cJSON_AddStringToObject(connect_layer, "last_tax_business", state->port_info.tax_business);
	}
	_lock_un(state->lock);

	cJSON_AddStringToObject(data_layer, "plate_start_date", (char *)state->port_info.plate_infos.plate_tax_info.startup_date);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_sys_time);
	cJSON_AddStringToObject(data_layer, "plate_sys_time_diff", tmp_s);

	if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
	{
		cJSON_AddStringToObject(data_layer, "use_downgrade_version", "1");
		cJSON_AddStringToObject(data_layer, "minimum_version", (char *)state->port_info.plate_infos.plate_basic_info.minimum_version);
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "use_downgrade_version", "0");
		cJSON_AddStringToObject(data_layer, "minimum_version", (char *)state->port_info.plate_infos.plate_basic_info.minimum_version);
	}

	cJSON_AddStringToObject(data_layer, "driver_ver", (char *)state->port_info.plate_infos.plate_tax_info.driver_ver);
	cJSON_AddStringToObject(data_layer, "driver_ver_err", (char *)state->port_info.plate_infos.plate_tax_info.driver_ver_err);

	if (strlen((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status) == 0)
	{
		if (strlen((char *)state->port_info.plate_infos.plate_tax_info.current_report_time) == 0)
		{
			int cbqkzt = 2;
			int types_count;
			for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
			{
				if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || 
					(state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
				{
					continue;
				}
				if (atoi((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt) != 2)
				{
					cbqkzt = atoi((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt);
					break;
				}
			}
			if (cbqkzt == 2)
				cJSON_AddStringToObject(data_layer, "report_tax_status", "�����忨���ɹ�");
			else
				cJSON_AddStringToObject(data_layer, "report_tax_status", "˰�̸�����,��δִ�е������忨����");
		}
		//wang 04 03 
		else
		{
			//cJSON_AddStringToObject(data_layer, "report_tax_status", "˰����ִ�г����忨����,δ�յ���������");
		}
	}
	else
	{
		if (strcmp((const char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "�����忨���ɹ�") == 0)
		{
			int types_count;
			for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
			{
				if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
				{
					continue;
				}
				if (atoi((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt) != 2)
				{
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "Ʊ��%03d��д��ز��ɹ�", state->port_info.plate_infos.invoice_type_infos[types_count].fplxdm);
				}
			}
		}
		cJSON_AddStringToObject(data_layer, "report_tax_status", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	}	

	cJSON_AddStringToObject(data_layer, "current_report_time", (char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
	cJSON_AddStringToObject(data_layer, "next_report_time", (char *)state->port_info.plate_infos.plate_tax_info.next_report_time);

	int result;
	char errinfo[1024]={0};
#ifdef RELEASE_SO
	result = so_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#else
	result = function_common_jude_plate_auth(state->port_info.usb_app, state->port_info.busid, errinfo);
#endif
	if (result != 1)
	{
		cJSON_AddStringToObject(data_layer, "tax_auth", "0");
	}
	else
	{
		cJSON_AddStringToObject(data_layer, "tax_auth", "1");
	}

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->aisino_sup);
	cJSON_AddStringToObject(data_layer, "aisino_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->nisec_sup);
	cJSON_AddStringToObject(data_layer, "nisec_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->cntax_sup);
	cJSON_AddStringToObject(data_layer, "cntax_sup", tmp_s);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->mengbai_sup);
	cJSON_AddStringToObject(data_layer, "mengbai_sup", tmp_s);

	cJSON_AddStringToObject(data_layer, "aisino_end_time", (char *)state->aisino_end_time);
	cJSON_AddStringToObject(data_layer, "nisec_end_time", (char *)state->nisec_end_time);
	cJSON_AddStringToObject(data_layer, "cntax_end_time", (char *)state->cntax_end_time);
	cJSON_AddStringToObject(data_layer, "mengbai_end_time", (char *)state->mengbai_end_time);

	cJSON_AddStringToObject(data_layer, "start_time", (char *)state->start_time);
	cJSON_AddStringToObject(data_layer, "end_time", (char *)state->end_time);
	cJSON_AddStringToObject(data_layer, "frozen_ago_time", (char *)state->frozen_ago_time);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_basic_info.cert_passwd_right);
	cJSON_AddStringToObject(data_layer, "cert_passwd_rignht", tmp_s);
	cJSON_AddStringToObject(data_layer, "area_code", (char *)state->port_info.plate_infos.plate_tax_info.area_code);

	cJSON_AddStringToObject(data_layer, "used_ofd", (char *)state->port_info.plate_infos.plate_basic_info.used_ofd);
	cJSON_AddStringToObject(data_layer, "server_type", (char *)state->port_info.plate_infos.plate_basic_info.server_type);
	cJSON_AddStringToObject(data_layer, "server_number", (char *)state->port_info.plate_infos.plate_basic_info.server_number);

	memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_tax_info.inv_type_num);
	cJSON_AddStringToObject(data_layer, "tax_types_num", tmp_s);
	cJSON *inv_type_array;
	cJSON_AddItemToObject(data_layer, "tax_infos", inv_type_array = cJSON_CreateArray());
	update_plate_info_cbqkzt(state);
	int types_count = 0;
	for (types_count = 0; types_count < DF_MAX_INV_TYPE; types_count++)
	{
		if ((state->port_info.plate_infos.invoice_type_infos[types_count].state == 0) || (state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.state == 0))
		{
			continue;
		}
		cJSON *type_layer;
		cJSON_AddItemToObject(inv_type_array, "dira", type_layer = cJSON_CreateObject());
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%03d", state->port_info.plate_infos.invoice_type_infos[types_count].fplxdm);
		cJSON_AddStringToObject(type_layer, "fplxdm", tmp_s);
		cJSON_AddStringToObject(type_layer, "sqslv", "");
		cJSON_AddStringToObject(type_layer, "cbqkzt", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.cbqkzt);

		if (strcmp((const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj,"00010101") == 0)
		{
			logout(ERROR, "system", "˰��ͬ��", "%s,˰����Ϣ��������з��ֿ�Ʊ��ֹʱ���쳣\r\n", state->port_info.port_str);
		}

		cJSON_AddStringToObject(type_layer, "kpjzsj", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj);
		cJSON_AddStringToObject(type_layer, "bsqsrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.bsqsrq);
		cJSON_AddStringToObject(type_layer, "bszzrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.bszzrq);
		cJSON_AddStringToObject(type_layer, "zxbsrq", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.zxbsrq);
		cJSON_AddStringToObject(type_layer, "dzkpxe", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.dzkpxe);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxkpsc);
		cJSON_AddStringToObject(type_layer, "lxsc", tmp_s);
		cJSON_AddStringToObject(type_layer, "lxzsljje", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxzsljje);
		cJSON_AddStringToObject(type_layer, "lxzssyje", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.lxzssyje);

#ifndef DF_OLD_MSERVER
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.qckcfs);
		cJSON_AddStringToObject(type_layer, "qckcfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.lgfpfs);
		cJSON_AddStringToObject(type_layer, "lgfpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.thfpfs);
		cJSON_AddStringToObject(type_layer, "thfpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.qmkcfs);
		cJSON_AddStringToObject(type_layer, "qmkcfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zsfpfs);
		cJSON_AddStringToObject(type_layer, "zsfpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zffpfs);
		cJSON_AddStringToObject(type_layer, "zffpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fsfpfs);
		cJSON_AddStringToObject(type_layer, "fsfpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fffpfs);
		cJSON_AddStringToObject(type_layer, "fffpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.kffpfs);
		cJSON_AddStringToObject(type_layer, "kffpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zsfpljje);
		cJSON_AddStringToObject(type_layer, "zsfpljje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zsfpljse);
		cJSON_AddStringToObject(type_layer, "zsfpljse", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zffpljje);
		cJSON_AddStringToObject(type_layer, "zffpljje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.zffpljse);
		cJSON_AddStringToObject(type_layer, "zffpljse", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fsfpljje);
		cJSON_AddStringToObject(type_layer, "fsfpljje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fsfpljse);
		cJSON_AddStringToObject(type_layer, "fsfpljse", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fffpljje);
		cJSON_AddStringToObject(type_layer, "fffpljje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.fffpljse);
		cJSON_AddStringToObject(type_layer, "fffpljse", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.sjxsje);
		cJSON_AddStringToObject(type_layer, "sjxsje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.type_sum.sjxsse);
		cJSON_AddStringToObject(type_layer, "sjxsse", tmp_s);
#else
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.sjxsje);
		cJSON_AddStringToObject(type_layer, "sjxsje", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%4.2Lf", state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.sjxsse);
		cJSON_AddStringToObject(type_layer, "sjxsse", tmp_s);
#endif
		cJSON_AddStringToObject(type_layer, "dqfpdm", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.dqfpdm);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.dqfphm);
		cJSON_AddStringToObject(type_layer, "dqfphm", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.zsyfpfs);
		cJSON_AddStringToObject(type_layer, "zsyfpfs", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl);
		cJSON_AddStringToObject(type_layer, "wsyfpjsl", tmp_s);

		if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl != 0)
		{
			cJSON *coil_array;
			int unused_num = 0;
			if (is_common == 1)
			{
				
				cJSON_AddItemToObject(type_layer, "wsyfpjxx", coil_array = cJSON_CreateArray());
				
				for (unused_num = 0; unused_num < state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl; unused_num++)
				{
					cJSON *unused_layer;
					if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].state == 0)
						continue;
					cJSON_AddItemToObject(coil_array, "dira", unused_layer = cJSON_CreateObject());
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", unused_num + 1);
					cJSON_AddStringToObject(unused_layer, "fpjh", tmp_s);
					cJSON_AddStringToObject(unused_layer, "fplbdm", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fplbdm);
					cJSON_AddStringToObject(unused_layer, "fpgmsj", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsj);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsl);
					cJSON_AddStringToObject(unused_layer, "fpgmsl", tmp_s);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpqshm);
					cJSON_AddStringToObject(unused_layer, "fpqshm", tmp_s);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpzzhm);
					cJSON_AddStringToObject(unused_layer, "fpzzhm", tmp_s);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpsyfs);
					cJSON_AddStringToObject(unused_layer, "fpsyfs", tmp_s);
				}
			}
			else//Ŀǰ�ߵƷ�ʽ
			{
				cJSON_AddItemToObject(type_layer, "unused_invoices", coil_array = cJSON_CreateArray());
				//out("δʹ�÷�Ʊ������:%d,port = %d\n", stream->port[port_num].port_info.tax_infos[types_num].wsyfpjsl, port_num+1);
				for (unused_num = 0; unused_num < state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.wsyfpjsl; unused_num++)
				{
					cJSON *unused_layer;
					if (state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].state == 0)
						continue;
					cJSON_AddItemToObject(coil_array, "dira", unused_layer = cJSON_CreateObject());
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", unused_num + 1);
					cJSON_AddStringToObject(unused_layer, "coil_num", tmp_s);
					cJSON_AddStringToObject(unused_layer, "coil_code", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fplbdm);
					cJSON_AddStringToObject(unused_layer, "buy_time", (const char *)state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsj);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpgmsl);
					cJSON_AddStringToObject(unused_layer, "buy_num", tmp_s);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%08d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpqshm);
					cJSON_AddStringToObject(unused_layer, "start_num", tmp_s);
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.invoice_type_infos[types_count].invoice_coils_info.invoice_coil[unused_num].fpsyfs);
					cJSON_AddStringToObject(unused_layer, "remain_num", tmp_s);
				}
			}

		}
	}
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


	out("�����������ĵ�ǰmqtt֤���ļ�md5ֵΪ��%s\n", crt_md5);
	cJSON_Delete(root);
	return 0;
}

static int analysis_mqtt_info_data(char *data, char *server_addr, int *port, char *uid, char *pwd,int mqtt_type)
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
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
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
	out("��ȡ����MQTT��������ַΪ%s\n", mqtt_server);
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
	out("��ȡ����MQTT�������˿ں�Ϊ%s\n", mqtt_port);



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
	out("��ȡ����MQTT uid%s\n", uid);
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
	out("��ȡ����MQTT pwd%s\n", pwd);



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
	//out("��ȡ����ca.crt֤���ļ�Ϊ%s\n", ca_cert_base);

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
	//out("��ȡ����client.crt֤���ļ�Ϊ%s\n", client_cert_base);

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
	//out("��ȡ����client.key֤���ļ�Ϊ%s\n", client_key_base);

	char tmp[4000];
	if (mqtt_type == DF_MQTT_FPKJXT)
	{
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/ca.crt", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/ca.crt");
			save_cert_file(ca_cert_base, "/etc/ca.crt");
		}
		else
		{
			if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("ca.cert�ļ���һ����Ҫ����\n");
				save_cert_file(ca_cert_base, "/etc/ca.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/client.crt", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/client.crt");
			save_cert_file(client_cert_base, "/etc/client.crt");
		}
		else
		{
			if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("client.cert�ļ���һ����Ҫ����\n");
				save_cert_file(client_cert_base, "/etc/client.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/client.key", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/client.key");
			save_cert_file(client_key_base, "/etc/client.key");
		}
		else
		{
			if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
			{
				out("client.key�ļ���һ����Ҫ����\n");
				save_cert_file(client_key_base, "/etc/client.key");
			}
		}
	}
	else
	{
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_ca.crt", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/fptbxt_ca.crt");
			save_cert_file(ca_cert_base, "/etc/fptbxt_ca.crt");
		}
		else
		{
			if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_ca.cert�ļ���һ����Ҫ����\n");
				save_cert_file(ca_cert_base, "/etc/fptbxt_ca.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_client.crt", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/fptbxt_client.crt");
			save_cert_file(client_cert_base, "/etc/fptbxt_client.crt");
		}
		else
		{
			if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_client.cert�ļ���һ����Ҫ����\n");
				save_cert_file(client_cert_base, "/etc/fptbxt_client.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_client.key", tmp, sizeof(tmp)) < 0)
		{
			out("��ȡ�ļ�ʧ��:[%s]\n", "/etc/fptbxt_client.key");
			save_cert_file(client_key_base, "/etc/fptbxt_client.key");
		}
		else
		{
			if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_client.key�ļ���һ����Ҫ����\n");
				save_cert_file(client_key_base, "/etc/fptbxt_client.key");
			}
		}
	}
	cJSON_Delete(root);
	return 0;
}


int function_get_mqtt_connect_data_md5_jude_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type)
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
	char local_file[81920] = { 0 };
	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));

#ifndef DF_1201_1
	mb_enc_get_mqtt_connect_info_json(stream->key_s, stream->ter_id, mqtt_type,&json_data);

	if (json_data == NULL)
	{
		out("�����������\n");
		return -1;
	}

#ifndef DF_OLD_MSERVER
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");
	parm.s_port = 20020;
	//wang ���ص���
#ifdef DF_BENDI_MSERVER
	sprintf(parm.s_ip, "%s", "192.168.0.139");
	parm.s_port = 8000;
#endif
#else
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;
#endif
#else
	sprintf(parm.s_ip, "%s", "192.168.0.139");
	parm.s_port = 1883;
#endif
	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);
#ifndef DF_1201_1
	ret = http_token_get_mqtt_connect_md5(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		out("function_get_mqtt_connect_data_md5_jude_update �����������ʧ��,ret = %d\n",ret);
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
		out("function_get_mqtt_connect_data_md5_jude_update ����json���ݽ���ʧ��,ret = %d,data %s\n", ret, outdata);
		free(outdata);
		return -1;
	}
	free(outdata);
	

	if (mqtt_type == DF_MQTT_FPKJXT)
	{
		read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	}
	else
	{
		read_file("/etc/fptbxt_ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/fptbxt_client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/fptbxt_client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	}
	
	out("mqtt_type = %d, server_addr = %s, server_port = %d, mqtt_uid = %s, mqtt_pwd = %s\n", mqtt_type, server_addr, *server_port, mqtt_uid, mqtt_pwd);
	//out("local_file %s", local_file);

	char md5_data[200] = { 0 };
	md5_value(local_file, strlen(local_file), md5_data);


	out("��Ʊ����������mqtt֤���ļ�md5ֵΪ��%s\n", md5_data);

	if (strcmp(md5_data, crt_md5) == 0)
	{
		out("����֤���ļ��������֤���ļ�md5ֵһ��,����Ҫ���� mqtt_type= %d\n", mqtt_type);
		return 0;
	}

	out("����֤���ļ��������֤���ļ�md5ֵ��һ��,��Ҫ���� mqtt_type= %d\n", mqtt_type);
#else
	read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
#endif
	return 1;
}


int function_get_mqtt_connect_data_md5_jude_update_1201(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd,int mqtt_type)
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


	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));

#ifndef CA_1201
	sprintf(parm.s_ip, "%s", "112.124.26.97");
	parm.s_port = 19983;
#else

	#ifndef CA_1201_37
		sprintf(parm.s_ip, "%s", "192.168.0.139"); //���ز���
		parm.s_port = 1883;
	#else
		sprintf(parm.s_ip, "%s", "60.204.142.202");
		parm.s_port = 19983;
	#endif

#endif

	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());


	char local_file[81920] = { 0 };
#ifndef CA_1201
	read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/server.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	read_file("/etc/server.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
#else

	#ifndef CA_1201_37
		read_file("/etc/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/etc/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	#else
		read_file("/home/zwl/work/zshu_37/ca.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/home/zwl/work/zshu_37/client.crt", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
		read_file("/home/zwl/work/zshu_37/client.key", local_file + strlen(local_file), sizeof(local_file)-strlen(local_file));
	#endif

#endif

	out("mqtt_type = %d, server_addr = %s, server_port = %d, mqtt_uid = %s, mqtt_pwd = %s\n", mqtt_type, server_addr, *server_port, mqtt_uid, mqtt_pwd);

	return 1;
}

int function_get_mqtt_connect_data_info_update(char *server_addr, int *server_port, char *mqtt_uid, char *mqtt_pwd, int mqtt_type)
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
	mb_enc_get_mqtt_connect_info_json(stream->key_s, stream->ter_id,mqtt_type, &json_data);
	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
#ifndef DF_OLD_MSERVER
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");
	parm.s_port = 20020;
	//wang ���ص���
#ifdef DF_BENDI_MSERVER
	sprintf(parm.s_ip, "%s", "192.168.0.139");
	parm.s_port = 8000;
#endif
#else
	sprintf(parm.s_ip, "%s", DF_ERR_SERVER_ADDRESS);
	parm.s_port = DF_ERR_SERVER_PORT;
#endif
	sprintf(parm.appid, "%s", DF_ERR_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_ERR_SERVER_APPSECERT);
	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	//g_buf = cJSON_Print(json);
	out("��ȡ֤���ļ�\n");
	ret = http_token_get_mqtt_connect_info(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		out("function_get_mqtt_connect_data_info_update �����������ʧ��,ret = %d\n", ret);
		free(json_data);
		return ret;
	}
	free(json_data);
	out("��ȡ֤���ļ��ɹ�\n");
	mb_dec_file_function(stream->key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	//out("�������ݳɹ�,���ܺ����ݣ�%s\n", outdata);
	free(r_buf);


	analysis_mqtt_info_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd,mqtt_type);
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

#ifdef RELEASE_SO
	so_common_reload_tax_auth(stream->key_s);
	so_common_get_cert_json_file(stream->key_s, &stream->cert_json);
#else
	function_common_reload_tax_auth(stream->key_s);
	function_common_get_cert_json_file(stream->key_s, &stream->cert_json);
#endif	

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
		}
	}
	return 0;
	

}

int fun_stop_plate_upload_offline_invoice(char *plate_num,int mode, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	//int result;
	stream = ty_usb_m_id;
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "˰������,�̺ţ�%s,���ڵ�ǰ%s����", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	if (mode == 1)//ֹͣ�ϴ�
	{
		state->port_info.offinv_stop_upload = 1;
		if (strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)
		{
			//out("��⵽����ռ�ã�ֱ��ǿ�ƽ�����\n");
			if (state->h_plate.hDev != NULL)
			{
				state->h_plate.hDev->bBreakAllIO = 1;
			}
		}
		else
		{
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
	}
	else
	{
		//�����������Ʊδ�ϴ��򴥷�����Ʊ�ϴ�
		if ((state->port_info.plate_infos.plate_tax_info.off_inv_num != 0) && (state->port_info.offinv_num_exit == 0))//��ֹ��Ʊ����ϴ�ʧ�ܣ������޿�Ʊ��������ζ��������ش�
			state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
		state->port_info.offinv_stop_upload = 0;
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
	}
	return 0;
}

//ȫ���½
/*
���������
	busid:�豸busid
	related_type:��½�����(��˰Ա������...)
	full_name:��½������
	mobile:�ֻ���
	idcard:���֤��
	sz_password:����
���������
	errinfo:������Ϣ
	lpgoto:һ����¼goto����
	lpcode:һ����¼code����
*/
int mbi_etax_login_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];

	do{
		if(0 == memcmp(login_method,"02",2))
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306��½�����Ϊ��");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306��½������Ϊ��");
			break;
		}
		if(0 == memcmp(login_method,"02",2))
		{
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_login_info(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,lpgoto,lpcode,lptoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#else
		result = function_common_etax_login_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,lpgoto,lpcode,lptoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#endif
		if(0 == memcmp(login_method,"02",2))
			port_unlock(state);
	}while(false);

	return result;
}

//�û���Ϣ
int mbi_etax_user_query_Interface(int port,char *busid,char *errinfo,char **outinfo,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];

	do{
		if(0 == memcmp(login_method,"02",2))
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
		}
		if(0 == memcmp(login_method,"02",2))
		{
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_user_query_info(busid,errinfo,outinfo,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#else
		result = function_common_etax_user_query_Interface(busid,errinfo,outinfo);
		#endif
		if(0 == memcmp(login_method,"02",2))
			port_unlock(state);
	}while(false);
	return result;
}

//��ҵ��Ϣ
int mbi_etax_relationlist_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char **outenterpriselist,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];

	do{
		if(0 == memcmp(login_method,"02",2))
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306��½�����Ϊ��");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306��½������Ϊ��");
			break;
		}
		if(0 == memcmp(login_method,"02",2))
		{
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_relationlist_info(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,outenterpriselist,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#else
		result = function_common_etax_relationlist_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,outenterpriselist);
		#endif
		if(0 == memcmp(login_method,"02",2))
			port_unlock(state);
	}while(false);
	return result;
}

//��ҵ�л�
int mbi_etax_relationchange_Interface(int port,char *busid,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,char *errinfo,char *uniqueIdentity,char *realationStatus,char *lpgoto,char *lpcode,char *lptoken,char *sys_type,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];

	do{
		if(0 == memcmp(login_method,"02",2))
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306��½�����Ϊ��");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306��½������Ϊ��");
			break;
		}
		if(0 == memcmp(login_method,"02",2))
		{
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_relationchange_info(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,lpgoto,lpcode,lptoken,sys_type,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#else
		result = function_common_etax_relationchange_Interface(busid,related_type,full_name,mobile,idcard,sz_password,errinfo,uniqueIdentity,realationStatus,lpgoto,lpcode,lptoken,sys_type);
		#endif
		if(0 == memcmp(login_method,"02",2))
			port_unlock(state);
	}while(false);
	return result;
}

// int mbi_etax_get_cookie_Interface_login(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data)
// {
// 	int result = -1;
// 	struct _ty_usb_m  *stream;
// 	struct _port_state   *state;
// 	int i;
// 	int nBusID = 0;
//     int nDevID = 0;
// 	stream = (struct _ty_usb_m  *)ty_usb_m_id;
// 	state = &stream->port[port - 1];
// 	do{
// 		if(0 == memcmp(login_method,"02",2))
// 		{
// 			if(NULL == busid){
// 				sprintf((char *)errinfo, "306�豸busidΪ��");
// 				break;
// 			}
// 		}
// 		if(NULL == related_type){
// 			sprintf((char *)errinfo, "306��½�����Ϊ��");
// 			break;
// 		}		
// 		if((NULL == mobile)&&(NULL == idcard)){
// 			sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
// 			break;
// 		}
// 		if(NULL == sz_password){
// 			sprintf((char *)errinfo, "306��½������Ϊ��");
// 			break;
// 		}
// 		if(0 == memcmp(login_method,"02",2))
// 		{
// 			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
// 			{
// 				char useinfo[1024]={0};
// 				memset(useinfo,0,1024);
// 				plate_useinfo_errinfo(state, useinfo);
// 				strcpy(errinfo,"306");
// 				strcat(errinfo,useinfo);
// 				break;
// 			}
// 		}
// 		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
// 		state->last_act = 1;
// 		#ifdef RELEASE_SO
// 		result = so_etax_get_cookie_info_login(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,etax_cookie,tpass_cookie,dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);
// 		#else
// 		result = function_common_etax_get_cookie_Interface(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,etax_cookie,tpass_cookie,dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
// 		#endif
// 		if(0 == memcmp(login_method,"02",2))
// 			port_unlock(state);
// 	}while(false);
// 	return result;
// }

int mbi_etax_get_cookie_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **etax_cookie,char **tpass_cookie,char **dppt_cookie,char *svr_container,char *soft_cert_ip,int soft_cert_port,char *login_method,char *area_code,char *temp_data)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	do{
		if(0 == memcmp(login_method,"02",2))
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306��½�����Ϊ��");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306��½������Ϊ��");
			break;
		}
		if(0 == memcmp(login_method,"02",2))
		{
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_get_cookie_info(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,etax_cookie,tpass_cookie,dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code,temp_data);
		#else
		result = function_common_etax_get_cookie_Interface(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,etax_cookie,tpass_cookie,dppt_cookie,svr_container,soft_cert_ip,soft_cert_port,login_method,area_code);
		#endif
		if(0 == memcmp(login_method,"02",2))
			port_unlock(state);
	}while(false);
	return result;
}


int mbi_etax_invoice_query_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_invoice_query_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_invoice_query_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_invoice_query_detail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_invoice_query_detail_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_invoice_query_detail_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_invoice_query_down_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxdata,char **fpcxname,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_invoice_query_down_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata,fpcxname,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_invoice_query_down_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxdata,fpcxname,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_rpa_invoice_issuance_Interface(int port,char *busid,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpkjurl){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_rpa_invoice_issuance_info(busid,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_rpa_invoice_issuance_Interface(busid,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_rpa_invoice_issuance_status_Interface(int port,char *busid,char *rzid,char *app_code,char *fpkjurl,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjdata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpkjurl){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_rpa_invoice_issuance_status_info(busid,rzid,app_code,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_rpa_invoice_issuance_status_Interface(busid,rzid,type,fpkjurl,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjdata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_credit_limit_Interface(int port,char *busid,char *nsrsbh,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **sxeddata,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == nsrsbh){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_credit_limit_info(busid,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,sxeddata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_credit_limit_Interface(busid,nsrsbh,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,sxeddata,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_blue_ink_invoice_issuance_Interface(int port,char *busid,char *fpkjjson,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpkjout,char **hqnsrjcxx,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	do{
		if(0 == use_cookie)
		{
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_blue_ink_invoice_issuance_info(busid,fpkjjson,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjout,hqnsrjcxx,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_blue_ink_invoice_issuance_Interface(busid,fpkjjson,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpkjout,hqnsrjcxx,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_hzqrxxSzzhQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_hzqrxxSzzhQuery_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_hzqrxxSzzhQuery_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_hzqrxxSave_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_hzqrxxSave_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_hzqrxxSave_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_hzqrxxQuery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_hzqrxxQuery_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_hzqrxxQuery_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_hzqrxxQueryDetail_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *full_name,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_hzqrxxQueryDetail_info(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_etax_hzqrxxQueryDetail_Interface(busid,fpcxsj,related_type,full_name,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_hcstaquery_Interface(int port,char *busid,char *fpcxsj,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_hcstaquery_info(busid,fpcxsj,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = etax_hcstaquery_invoice_issuance_Interface(busid,fpcxsj,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_fjxxpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_fjxxpeizhi_info(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_fjxxpeizhi_Interface(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}

int mbi_etax_cjmbpeizhi_Interface(int port,char *busid,char *fpcxsj,char *mode,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,char *new_etax_cookie,char *new_tpass_cookie,char *new_dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306��Ʊ��ѯ����Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		#ifdef RELEASE_SO
		result = so_etax_cjmbpeizhi_info(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#else
		result = function_common_cjmbpeizhi_Interface(busid,fpcxsj,mode,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,new_etax_cookie,new_tpass_cookie,new_dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}


int mbi_etax_get_nsrjcxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_get_nsrjcxx_info(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie);
		#else
		result = function_common_etax_get_nsrjcxx_Interface(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}


int mbi_etax_get_nsrfxxx_Interface(int port,char *busid,char *related_type,char *mobile,char *idcard,char *sz_password,int realationtype,char *uniqueIdentity,char *realationStatus,char *errinfo,char **fpcxout,char *area_code,char *etax_cookie,char *tpass_cookie,char *dppt_cookie,int use_cookie)
{
	int result = -1;
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int i;
	int nBusID = 0;
    int nDevID = 0;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	do{
		if(0 == use_cookie)
		{
			state = &stream->port[port - 1];
			if(NULL == busid){
				sprintf((char *)errinfo, "306�豸busidΪ��");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306��½�����Ϊ��");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306��½���ֻ����Լ����֤����Ϊ��");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306��½������Ϊ��");
				break;
			}
			if (port_lock(state, 20, 3, DF_PORT_USBD_QD_LOCK) < 0)
			{
				char useinfo[1024]={0};
				memset(useinfo,0,1024);
				plate_useinfo_errinfo(state, useinfo);
				strcpy(errinfo,"306");
				strcat(errinfo,useinfo);
				break;
			}
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
		}
		#ifdef RELEASE_SO
		result = so_etax_get_nsrfxxx_info(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie);
		#else
		result = function_common_etax_get_nsrfxxx_Interface(busid,related_type,mobile,idcard,sz_password,realationtype,uniqueIdentity,realationStatus,errinfo,fpcxout,area_code,etax_cookie,tpass_cookie,dppt_cookie,use_cookie);
		#endif
		if(0 == use_cookie)
			port_unlock(state);
	}while(false);
	return result;
}