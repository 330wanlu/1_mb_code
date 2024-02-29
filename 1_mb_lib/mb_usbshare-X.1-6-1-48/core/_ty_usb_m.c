#define _ty_usb_m_c
#include "_ty_usb_m.h"

int ty_usb_m_add(const char *usb_name,int dev_type)
{   struct _ty_usb_m  *stream;
	int result;
	stream=malloc(sizeof(struct _ty_usb_m));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
    //out("加载:ty_usb_m模块\n");
	memset(stream,0,sizeof(struct _ty_usb_m));
    memcpy(stream->ty_usb_name,usb_name,strlen(usb_name));
    //out("获取到的usbip_name[%s],usb_name[%s]\n",stream->usbip_name,stream->ty_usb_name);
	result = ty_file_add(DF_DEV_USB_M, stream, "ty_usb_m", (struct _file_fuc *)&ty_usb_m_fuc);
	if(result<0)
		free(stream);
	ty_usb_m_id = stream;
    //out("加载:ty_usb_m模块成功\n");
	return result; 
}

static int ty_usb_m_open(struct ty_file	*file)
{   struct _ty_usb_m  *stream;
    int j;
	int i;
	int readkeylen = 0;
	//获取单片机密-钥组;
	int result;
	uint16 crc_ret = 0;
	
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    if(stream->state==0)
    {   //out("打开:[%s]文件\n",stream->ty_usb_name);
		//out("首次启动读取纳税人名称进程\n\n\n\n");

#ifndef DF_1201_1
        stream->ty_usb_fd=ty_open(stream->ty_usb_name,0,0,NULL);
        if(stream->ty_usb_fd<0)
        {   
			logout(INFO, "system", "init", "打开文件失败 ty_usb_m_open\r\n");//wang
			out("打开文件[%s]失败:[%d]\n",stream->ty_usb_name,stream->ty_usb_fd);
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
			logout(INFO, "system", "init", "读单片机密钥失败\r\n");//wang
			out("读单片机密钥失败\n");
			stream->keyfile_load_err = 1;
		}
		if(stream->keyfile_load_err == 1)		//读密钥失败了
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
		logout(INFO, "system", "init", "单片机密钥校验值为%x\r\n",crc_ret);//wang
#ifdef RELEASE_SO			
		//解密动态库
		/*result = load_tax_lib_so(stream->key_s);
		if (result < 0)
		{
			logout(INFO, "system", "init", "解密动态库失败\r\n");//wang
			out("解密动态库失败\n");
			stream->lib_dec_err = 1;
			stream->auth_dec_err = 1;
			//return -100;
		}*/

		int k;
		for ( k = 0; k < 3; k++)   
		{
			result = load_tax_lib_so(stream->key_s);  //正常返回0 
						
			if(result == 0)
			{
				out("解密动态库--第%d次成功\r\n",k+1);
				break;
			}
				
			if (result < 0 && k == 2)
			{
				logout(INFO, "system", "init", "解密动态库失败\r\n");//wang
				out("解密动态库失败\n");
				stream->lib_dec_err = 1;
				stream->auth_dec_err = 1;
				//return -100;
			}

		}

		//动态加载库
		//wang 02 13
		if (stream->lib_dec_err != 1)
		{
			result = load_tax_lib_so_fun();
			if (result < 0)
			{
				logout(INFO, "system", "init", "动态加载库失败\r\n");//wang
				out("动态加载库失败\n");
				stream->lib_load_err= 1;
				//return -101;
			}
		}
		else 
		{
			result = load_tax_lib_so_fun_wang();
			if (result < 0)
			{
				logout(INFO, "system", "init", "动态加载库二次失败\r\n");//wang
				out("动态加载库失败\n");
				stream->lib_load_err= 1;
				//return -101;
			}	
			else{
				stream->lib_dec_err = 0;	//这里再次加载成功后释放该标志位
				logout(INFO, "system", "init", "动态加载库二次成功,但可能导致【授权端口数为0】,需重启机柜\r\n");//wang
			}		
		}
		//初始化动态库
		//logout(INFO, "system", "init", "动态库标志位状态lib_dec_err=%d,lib_load_err=%d\r\n", stream->lib_dec_err,stream->lib_load_err);//wang
		logout(INFO, "system", "init", "动态库标志位状态lib_dec_err=%d\r\n", stream->lib_dec_err);// zwl 0802
		if (stream->lib_dec_err != 1 && stream->lib_load_err != 1)
		{
			out("正在初始化动态库!\n");
			result = so_common_init_tax_lib(stream->key_s, stream->tax_so_ver);
			if (result <= 0)
			{
				out("初始化动态库\n");
				stream->auth_dec_err= 1;

			}
			//so_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
		}
		//logout(INFO, "system", "init", "******获取到的加密库版本号为[%s]******\r\n", stream->tax_so_ver);			//wang
		out("******获取到的加密库版本号为[%s]******\r\n", stream->tax_so_ver); //zwl out 

#else
		result = function_common_init_tax_lib(stream->key_s, stream->tax_so_ver);

		//function_common_init_tax_lib_log_hook((void *)logout_hook, NULL);
#endif
		sprintf(stream->mbc_so_ver, "%s", MB_CLIB_VERSION);
		out("授权端口数量：%d\n", result);
		//logout(INFO, "system", "init", "授权端口数量:%d\r\n", result);
		out( "授权端口数量:%d\r\n", result); //zwl 0802
        //out("打开:[%s]文件\n",stream->usbip_name);
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
		
		stream->last_report_time_o = get_time_sec();//老心跳
		stream->last_report_time_h = get_time_sec();//中台心跳
		stream->last_report_time_s = get_time_sec();//基础信息
        //stream->usbip_fd=ty_open(stream->usbip_name,0,0,NULL);
        //if(stream->usbip_fd<0)
        //{   out("打开文件[%s]失败:[%d]\n",stream->usbip_name,stream->usbip_fd);
        //    ty_close(stream->ty_usb_fd);
        //    return stream->usbip_fd;
        //}
#ifndef DF_1201_1
		stream->tran_fd=ty_open("/dev/tran_data",0,0,NULL);
		if(stream->tran_fd<0)
		{
			logout(INFO, "system", "init", "打开转发模块失败\r\n");
			out("打开文件：/dev/tran_fd失败\n");
			return stream->tran_fd;
		}
		stream->deploy_fd = ty_open("/dev/deploy", 0, 0, NULL);
		if (stream->deploy_fd<0)
		{
			logout(INFO, "system", "init", "打开配置模块失败\r\n");
			out("打开文件：/dev/deploy_fd失败\n");
			return stream->deploy_fd;
		}

		stream->event_file_fd = ty_open("/dev/event", 0, 0, NULL);
		if (stream->event_file_fd < 0)
		{
			logout(INFO, "system", "init", "打开事件模块失败\r\n");
			out("打开文件：/dev/event_file_fd失败\n");
			return stream->event_file_fd;
		}
		stream->switch_fd = ty_open("/dev/switch", 0, 0, NULL);
		if (stream->switch_fd<0)
		{
			logout(INFO, "system", "init", "打开单片机控制模块失败\r\n");
			out("打开开关事件失败\n");
			return stream->switch_fd;
		}
		stream->get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
		
		ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_SUPPORT, &stream->dev_support);
		check_stop_upload(&stream->stop_aisino_up, &stream->stop_nisec_up, &stream->stop_cntax_up, &stream->stop_mengbai_up);
		//out("打开tran_data成功\n");
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

		if (read_power_mode == 1)//强制非节电版本
		{
			stream->dev_support.support_power = 0;
		}

		#if (defined DF_2212_1) || (defined DF_2212_8) || (defined DF_2216_1) || (defined DF_2216_8) //zwl 0504  
			if(stream->dev_support.support_power == 0)  //wang 0428   //主要针对120 240口 ，问题系统376
			{
				stream->dev_support.support_power = 1;
			}
			read_power_mode = 0;
			out("---zwl--节电模式！--1--\n");  //zwl 0504
		#endif

		out("启动端口上电\n");
		all_port_power_open(stream);
		out("启动端口检测线程\n");
		if (stream->dev_support.support_power == 1)
		{
			for (j = 0; j < stream->usb_port_numb; j++)
			{		
				stream->port[j].power_mode = Saving_Mode;//节电机柜，所有端口全部默认为节电
			}
			_task_open("ty_usb_m", task_check_port_ad_status, stream, 200, NULL);	
			_task_open("ty_usb_m", task_port_power_up_down, stream, 200, NULL);
			_task_open("ty_usb_m", task_port_repair_ad_error, stream, 200, NULL);
			_task_open("ty_usb_m", task_check_power_down_dev_remove, stream, 200, NULL);			
		}
		
		//out("开启线程task_m\n");
		_task_open("ty_usb_m",task_m,stream,400,NULL);	

		//out("启动检测端口设备插拔动作线程");
		_task_open("ty_usb_m", task_check_port, stream, 30, NULL);      


		if (memcmp(stream->ter_id, "068180", 6) == 0) //仅
		{
			//068180010960 - 068180021950
			//068180005925 - 068180005941
			int ter_id_num;
			char ter_id_num_str[200] = { 0 };
			memcpy(ter_id_num_str, stream->ter_id + 7, 4);
			ter_id_num = atoi(ter_id_num_str);
			out("机器编号序号为%d\n", ter_id_num);

			if (((ter_id_num >= 1096) && (ter_id_num <= 2195)) || ((ter_id_num >= 592) && (ter_id_num <= 594)) || ((ter_id_num >= 468) && (ter_id_num <= 470)))
			{
				_task_open("ty_usb_m", task_check_port_repair_51err, stream, 1000, NULL);
			}
		}
		//out("启动读CA线程及开启读税务信息线程\n");
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
		//out("启动面板灯管理线程\n");
		_task_open("ty_usb_m", task_port_led_manage, stream, 500, NULL);

#ifndef PRODUCE_PROGRAM		
		sem_init(&stream->cb_qk_sem, 0, 0);//10月底统一修改,取消信号量

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
    {   out("要求全部关闭\n");
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
    uint16 crc = 0x0000;//初始值
    int i;  
    for (; num > 0; num--)             
    {  
        crc = crc ^ (*addr++) ;     //低8位异或
        for (i = 0; i < 8; i++)             
        {  
            if (crc & 0x0001)       //由于前面和后面省去了反转，所以这里是右移，且异或的值为多项式的反转值
                crc = (crc >> 1) ^ 0xA001;//右移后与多项式反转后异或
            else                   //否则直接右移
                crc >>= 1;                    
        }                               
    }
    return(crc^0xffff);            //返回校验值 
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
		out("加密库解析失败\n");
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

	//==通用接口==//
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

	//==不需要授权且不需要口令验证通过亦可调用的接口==//
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

	//==需要授权未验证口令或口令错误可调用的接口==//
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

	//======需要授权且验证口令正确后方可操作的接口======//
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
	//so_aisino_get_download_net_invoice_coil = dlsym(handle, "function_aisino_get_download_net_invoice_coil");//接口停用
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

	//==通用接口==//
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

	//==不需要授权且不需要口令验证通过亦可调用的接口==//
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

	//==需要授权未验证口令或口令错误可调用的接口==//
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

	//======需要授权且验证口令正确后方可操作的接口======//
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
	//so_aisino_get_download_net_invoice_coil = dlsym(handle, "function_aisino_get_download_net_invoice_coil");//接口停用
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
	if (stream->dev_support.support_power == 1)//省电机型
	{
		//前面模块已经关闭无需重复关电
	}
	else
	{
		for (i = 1; i <= port_number; i++)
		{
			state = &stream->port[i - 1];
			//out("[ty_usb_m.c]按端口号打开端口 port = %d\n",i);
			//前面模块已经关闭无需重复关电
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, i);
			state->power = 1;
			if (result<0)
			{
				logout(INFO, "SYSTEM", "INIT", "通知单片机打开端口电源失败,端口号%d\r\n",i);
				err_port += 1;
				//out("task_open:打开端口失败,端口号%d\n", i);
				continue;
			}
		}
	}
	if (err_port == 0)
	{
		logout(INFO, "SYSTEM", "INIT", "所有USB端口初始化电源完成\r\n");
	}
	else
	{
		logout(INFO, "SYSTEM", "INIT", "所有USB端口初始化过程中存在异常,异常次数为%d次\r\n", err_port);
	}
	return 0;
}


int check_port_auth_info(struct _ty_usb_m  *stream)
{
	struct _port_state   *state;
	char busid[32] = {0};
	int aisino_sup;
	int nisec_sup;
	char start_time[20] = {0};//起始时间
	char end_time[20] = {0};//结束时间
	int size;
	int port;
	int i;
	char num[10] = {0};
	cJSON *arrayItem, *item;
	if (stream->cert_json == NULL)
	{
		out("授权文件为空\n");
		return -1;
	}

	//out("授权文件内容:%s\n", stream->cert_json);

	cJSON *root = cJSON_Parse(stream->cert_json);
	if (!root)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "version");
	if (get_json_value_can_not_null(item, tax_auth.version, 0, 128) < 0) {
		out("解析授权文件版本号失败\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "serial_num");
	if (get_json_value_can_not_null(item, tax_auth.serial_num, 0, 128) < 0) {
		out("解析授权文件序列号失败\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "name");
	if (get_json_value_can_not_null(item, tax_auth.name, 0, 128) < 0) {
		out("解析授权文件名称失败\n");
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
			out("解析USB端口号失败,i=%d\n", i);
			goto End;
		}
		port = atoi(num);
		state = &stream->port[port - 1];

		item = cJSON_GetObjectItem(arrayItem, "busid");
		if (get_json_value_can_not_null(item, busid, 0, 32) < 0) {
			out("解析busid失败,i=%d\n", i);
			goto End;
		}

		if (strcmp(tax_auth.version,"V1.1.01") <= 0)
		{
			memset(num, 0, sizeof(num));
			item = cJSON_GetObjectItem(arrayItem, "aisino_sup");
			if (get_json_value_can_not_null(item, num, 0, 10) < 0) {
				out("解析航信盘支持失败,i=%d\n", i);
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
				out("解析百旺盘支持失败,i=%d\n", i);
				goto End;
			}
			nisec_sup = atoi(num);
			state->nisec_sup = nisec_sup;

			item = cJSON_GetObjectItem(arrayItem, "start_time");
			if (get_json_value_can_not_null(item, start_time, 0, 32) < 0) {
				out("解析起始时间失败,i=%d\n", i);
				goto End;
			}
			memset(state->start_time, 0, sizeof(state->start_time));
			strcpy(state->start_time, start_time);

			item = cJSON_GetObjectItem(arrayItem, "end_time");
			if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
				out("解析起始时间失败,i=%d\n", i);
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
				out("解析起始时间失败,i=%d\n", i);
				continue;
			}
			memset(state->start_time, 0, sizeof(state->start_time));
			strcpy(state->start_time, start_time);

			item = cJSON_GetObjectItem(arrayItem, "end_time");
			if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
				out("解析截止时间失败,i=%d\n", i);
				continue;
			}
			memset(state->end_time, 0, sizeof(state->end_time));
			strcpy(state->end_time, end_time);

			item = cJSON_GetObjectItem(arrayItem, "frozen_ago_time");
			memset(state->frozen_ago_time,0,20);
			if (get_json_value_can_not_null(item, state->frozen_ago_time, 0, 32) < 0) {
				out("解析冻结时间失败,i=%d\n", i);
				continue;
			}
			//金税盘
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "aisino_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析金税盘授权信息失败,i=%d\n", i);
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

			//税控盘
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "nesic_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析金税盘授权信息失败,i=%d\n", i);
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

			//税务ukey
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "cntax_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析税务ukey授权信息失败,i=%d\n", i);
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

			//蒙柏
			memset(tmp_str, 0, sizeof(tmp_str));
			item = cJSON_GetObjectItem(arrayItem, "mengbai_auth");
			if (get_json_value_can_not_null(item, tmp_str, 0, sizeof(tmp_str)) < 0) {
				out("解析模拟盘授权信息失败,i=%d\n", i);
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
		//logout(INFO, "SYSTEM", "释放税盘内存", "原先%d端口即将下电,即将关闭句柄\r\n", state->port);
		common_close_usb_device(state,&state->h_plate);
	}
	return 0;
}
/*
	异常立即端口下电
	下电完毕将端口置为已下电无设备状态
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
	//out("port_power_down 执行单片机关闭%d号端口电源\n",state->port);
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);//执行下电操作
	if (result < 0)
	{
		logout(INFO, "DEV", "PORT_POWER", "port_power_down:单片机执行下电失败%d\n", state->port);
		usleep(20000);
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);
	}
	state->need_down_power = 0;
	if (state->need_up_power == 1)
	{
		//logout(INFO, "DEV", "PORT_POWER", "%d号端口执行下电操作时发现，系统检测线程正在执行上电，防止冲突清除正在上电状态\n", state->port);
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
		out("端口号有误\n");
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
			out("%d号端口未完成初始化暂不允许使用,state->port_init == %d,usedinfo = %s\n", port, state->port_init,state->port_info.used_info);
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
			out("检测到%d号端口已经上电,但USBbus信息存在问题，需重新下电上电\n",port);
			state->sys_reload = 1;//关闭设备保留信息
			port_power_down(state);
		}
		else
		{
			//out("检测到%d号端口已经上电,直接继续使用\n",port);
			return 0;
		}				
	}
	if (stream->onlineport > DF_FULL_LOAD_NUM)
	{
		return POWER_ERR_FULL_LOAD;
	}
	//需判断节电版满载数量
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
	if (result<0)
	{
		logout(INFO, "DEV", "上电", "打开端口失败, 端口号%d\n", port);
		state->sys_reload = 0;//关闭设备不保留信息
		port_power_down(state);//设备已被拔除
		return POWER_ERR_NO_DEV;
	}
	state->power = 1;

	

	for (time = 0; time < 5000; time++)
	{
		busnum = get_busid_busnum(busid);
		devnum = get_busid_devnum(busid);
		if (((busnum == -1) || (devnum == -1)) || ((busnum == 0) || (devnum == 0)))
		{
			//logout(INFO, "port_lock", "端口申请使用", "USB端口%d,原记录中busnum = %d,devnum = %d,可能遇到端口发生重启情况\r\n", state->port,state->h_plate.nBusID, state->h_plate.nDevID);
			usleep(1000);
			continue;
		}
		break;
	}
	if (time == 5000)
	{
		state->sys_reload = 0;//关闭设备不保留信息
		out("打开%d端口号设备等待枚举超时\n",port);
		port_power_down(state);
		return POWER_ERR_NO_DEV;
	}
	state->sys_reload = 0;//系统重载结束
	if ((state->port_info.usb_app == DEVICE_MENGBAI) || (state->port_info.usb_app == DEVICE_MENGBAI2))
		sleep(5);

	if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)//加载不打开句柄
	{
		return 0;
	}
	if (strcmp(state->port_info.used_info, DF_PORT_USED_DQMCSHXX) == 0)//读基础信息不需要打开句柄
	{
		return 0;
	}
	state->h_plate.nBusID = busnum;
	state->h_plate.nDevID = devnum;
	//out("打开%d端口号busnum = %04x devnum = %04x\n", port,state->h_plate.nBusID, state->h_plate.nDevID);
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
		logout(INFO, "DEV", "上电", "打开端口句柄失败, 端口号%d ,result:%d,use info:%s\n", port, result, state->port_info.used_info);
		state->sys_reload = 0;//关闭设备不保留信息
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

	//wang 修改读取AD逻辑
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
		tmp += 2;//当前端口位偏移过去
		now_status = *tmp;
		tmp += 1;

		//out("端口号：%d,状态：%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//正常模式，不进行下电端口检测
		{
			continue;
		}

		if (now_status == 0)
		{

			if ((state->status != 0))
			{
				if ((state->sys_reload == 1) || (state->port_init != 0))
				{
					if (state->status_counts < 2)//可能存在误报，需进行防抖动
					{
						if(state->port_info.ad_status > 2000)			//wang 02 23 防抖动
							state->status_counts += 1;
						else{
							printf("status_counts port=%d\r\n",state->port);
						}
						continue;
					}
					out("%d号端口检测到关电状态设备被拔除\n", state->port);
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

	//wang 修改读取AD逻辑
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
		tmp += 2;//当前端口位偏移过去
		now_status = *tmp;
		tmp += 1;

		//out("端口号：%d,状态：%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//正常模式，不进行下电端口检测
		{
			continue;
		}
		
		if ((now_status == 0)&&(state->port_info.ad_status > 2000)) //wang 02 24 防抖动
		{
			long time_now = get_time_sec();
			if (time_now < 1656671934)//时间未更新
			{
				continue;
			}
			if (state->ad_repair_down_time == 0)
			{
				state->ad_repair_down_time = time_now - 10;//防止系统上下电部分设备单片机检测不到，再次上电会慢的情况
				continue;
			}
			if ((state->status != 0)) //等待其他线程处理完
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
				//端口当前为上电状态，等待；
				continue;
			}
			if ((state->power == 0) && (state->h_plate.need_free == 1))
			{
				//端口未完全释放完成，等待
				continue;
			}
			if (time_now - state->ad_repair_down_time > 15)
			{
				//logout(INFO, "SYSTEM", "AD_CHECK", "%d号端口AD值为：%d,单片机认为没有插设备,开始上电检测\r\n", down_port, state->port_info.ad_status);
				//需判断节电版满载数量
				result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, down_port);
				state->power = 1;
				if (result < 0)
				{
					logout(INFO, "DEV", "上电", "打开端口失败, 端口号%d\n", down_port);
					state->sys_reload = 0;//关闭设备不保留信息
					port_power_down(state);//设备已被拔除
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

	//wang 修改读取AD逻辑 保证读到AD值后再执行判断
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
		tmp += 2;//当前端口位偏移过去
		now_status = *tmp;
		tmp += 1;

		//out("端口号：%d,状态：%d\n", down_port, now_status);
		state = &stream->port[down_port - 1];
		if (state->power_mode == Normal_Mode)//正常模式，不进行下电端口检测
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
			if (state->sys_reload == 1)//已进入节电
			{
				continue;
			}
			//out("单片机告知%d号端口有设备,当前设备数量为%d,上电端口数量为%d\n", down_port, stream->onlineport, get_powered_port_num());
			if (state->ad_repair_check == 1) //无设备端口再上电检测过程中如果有设备插入检测到了，则立即停止AD修复检测
			{
				state->ad_repair_check = 0;
				state->ad_repair_up_time = 0;
				state->ad_repair_down_time = get_time_sec();
				state->power_mode = Saving_Mode;
			}
			if (stream->onlineport > DF_FULL_LOAD_NUM)
			{
				out("给%d号端口上电，超过支持最大满载设备数量\n", down_port);
				for (;;)
				{
					usleep(10000);
					if (stream->onlineport < DF_FULL_LOAD_NUM)
						break;
				}
			}
			if (get_powered_port_num() > DF_FULL_POWER_NUM)
			{
				out("给%d号端口上电，超过支持最大上电端口数量\n", down_port);
				for (;;)
				{
					usleep(10000);
					if (get_powered_port_num() < DF_FULL_POWER_NUM)
						break;
				}
			}
			if (state->power != 0)
			{
				//端口当前为上电状态，等待；
				//out("给%d号端口上电，当前端口为上电状态\n", down_port);
				continue;
			}
			if ((state->power == 0) && (state->h_plate.need_free == 1))
			{
				//端口未完全释放完成，等待
				out("给%d号端口上电，当前端口为下电状态且句柄未清除\n", down_port);
				continue;
			}
			//out("给%d号端口上电，port_init = %d, need_up_power = %d\n", down_port, state->port_init, state->need_up_power);
			if ((state->port_init == 0) && (state->need_up_power != 1) && (state->need_down_power!=1))
			{
				//out("单片机告知%d号端口有设备,当前设备数量为%d,上电端口数量为%d\n", down_port, stream->onlineport, get_powered_port_num());
				state->need_up_power = 1;//防止与下电线程冲突
				//out("%d号端口有设备需要读取名称税号\n", down_port);
				//需判断节电版满载数量
				result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, down_port);
				state->power = 1;
				if (result<0)
				{
					state->need_up_power = 0;//正在上电中，避免重复操作
					logout(INFO, "DEV", "上电", "打开端口失败, 端口号%d\n", down_port);
					state->sys_reload = 0;//关闭设备不保留信息
					port_power_down(state);//设备已被拔除
					continue;
				}
				state->need_up_power = 1;
			}
		}
		else if (now_status == 2)
		{
			out("单片机告知%d号端口短路\n", down_port);
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
	//out("ad值\n");
	//printf_array(ad_status,600);
	if (result != 2 * stream->usb_port_numb)
	{
		out("获取到的ad返回长度错误:result =%d\n", result);
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
	//out("状态值\n");
	//printf_array(all_port_status,302);//100只有302有效

	
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
			logout(INFO, "SYSTEM", "AD_CHECK", "%d号端口AD值为：%d,单片机认为没有插设备,实际检测有设备,将此端口改为非节电模式\r\n", i+1, state->port_info.ad_status);
		}
		else
		{
			long time_now = get_time_sec();
			if (time_now - state->ad_repair_up_time > 5)//上电时间超过5秒，判断没有设备
			{
				state->ad_repair_check = 0;
				state->ad_repair_up_time = 0;
				state->ad_repair_down_time = get_time_sec();
				state->sys_reload = 0;
				//out("%d号端口上电超时未枚举关闭端口电源\n", i+1);
				port_power_down(state);
			}
		}
	}
}




//端口检查处理线程
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

//等待事件检测进程有无运行
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
		//out("外网功能未就绪,稍后重试上报终端状态\n");
		return;
	}

	//out("机器编号序号在生成故障的51单片机批次需处理\n");
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
		//out("检测到第%d号端口没有设备,可能为51版本的异常问题,判断断电时长是否超过3秒\n", port);
		if (state->last_without_time == 0)
		{
			state->last_without_time = get_time_sec();
			continue;
		}
		//out("当前时间：%ld,上次离线时间：%ld\n", now_time , state->last_without_time);
		if (now_time - state->last_without_time > 3)
		{

			//out("检测到第%d号端口没有设备,可能为51版本的异常问题,断电时长超过3秒,重新给端口上电\n", port);
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
		out("关闭句柄释放内存,税盘类设备,端口号:%d\n", state->port);
		//usb_device_close_serssion(h_plate->hUSB, h_plate->hDev, 0);
		if (h_plate->hUSB != NULL)
		{
			if (h_plate->hUSB->handle)
			{
				//printf("if (h_plate->hUSB->handle)\r\n");
				libusb_close(h_plate->hUSB->handle);
				h_plate->hUSB->handle = NULL;		//wang 02 07 尝试解决机柜重启问题
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
			h_plate->hUSB->context = NULL;			//wang 02 07 尝试解决机柜重启问题

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
	//out("关闭句柄释放内存完成,端口号:%d\n", state->port);
	return 0;
}

/*
static int common_close_usb_device(struct _port_state   *state,struct HandlePlate *h_plate)
{

	int usb_type = h_plate->usb_type;

	if ((usb_type == DEVICE_MENGBAI)||(usb_type == DEVICE_NISEC) || (usb_type == DEVICE_AISINO) || (usb_type == DEVICE_CNTAX) || (usb_type == DEVICE_SKP_KEY))
	{
		//out("关闭句柄释放内存,税盘类设备,端口号:%d\n", state->port);
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
	//out("关闭句柄释放内存完成,端口号:%d\n", state->port);
	return 0;
}*/
//监测端口插拔线程
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
	//检测线程给予心跳
	ty_pd_update_task_status(DF_TASK_PORT_CHECK, get_time_sec());
	//检测1-1总线是否存在
#ifndef MTK_OPENWRT
	if (access("/sys/bus/usb/devices/1-1", 0) != 0)
	{
		logout(INFO, "SYSTEM", "CHECK_PORT", "检测到1-1总线丢失,即将重启机柜\r\n");
		sync();
		sleep(5);
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
	}
#endif
	int onlineport = 0;
	for (port = 1; port <= port_number; port++)
	{	//out("检测第%d号端口\n",i);

		state = &stream->port[port - 1];
		if (state->port != port)
			state->port = port;
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
		if (result<0)
		{
			if (state->sys_reload == 1)
			{ //进入节电
				if (state->h_plate.need_free == 1)
				{
					//out("%d号端口进入节电\n",port);
					if (state->port_info.port_used == 1)//端口占用
					{
						_lock_set(state->lock);
						if (state->h_plate.hDev != NULL)
						{
							if (state->h_plate.hDev->bBreakAllIO != 1)
								logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,当前使用信息为：%s,要求中断使用并关闭端口\r\n", state->port_info.port_str, port, state->port_info.used_info);
							state->h_plate.hDev->bBreakAllIO = 1;
						}
						_lock_un(state->lock);
						state->port_info.inv_query = 0;
						continue;
					}
					//out("%d号端口进入节电释放句柄\n", port);
					free_h_plate(state);
				}
				continue;
			}
			if (state->port_info.with_dev == 1) //之前有设备现在没有了
			{
				//out("下电有无设备:%d---是否已经上电:%d\n", state->status, state->power);
				if (state->h_plate.hDev != NULL)
				{
					if (state->h_plate.hDev->bBreakAllIO != 1)
						logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,将当前设备信息备份,need_free = %d\r\n", state->port_info.port_str, port, state->h_plate.need_free);
				}
				//此处需释放税盘已打开的句柄
				if (state->h_plate.need_free == 1)
				{
					if (state->port_info.port_used == 1)//端口占用
					{
						_lock_set(state->lock);
						if (state->h_plate.hDev != NULL)
						{
							if (state->h_plate.hDev->bBreakAllIO != 1)
								logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,当前使用信息为：%s\r\n", state->port_info.port_str, port, state->port_info.used_info);
							state->h_plate.hDev->bBreakAllIO = 1;
						}
						_lock_un(state->lock);
						state->port_info.inv_query = 0;
						continue;
					}
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,即将关闭句柄\n", state->port_info.port_str, port);
					free_h_plate(state);
					logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,关闭句柄成功\n", state->port_info.port_str, port);
				}
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,现在检测到设备被拔除了,当前使用信息为：%s,清除内存\r\n", state->port_info.port_str, port, state->port_info.used_info);
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
				state->need_getinv = 0;//已上传发票统计查询
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
				logout(INFO, "SYSTEM", "CHECK_PORT", "%s,原先%d端口有设备,清除port_info\n", state->port_info.port_str, port);

				if ((state->power_mode == Saving_Mode) && (state->power == 1))//如果是节电版，在端口通电是设备被拔除，需给端口下电
				{
					state->sys_reload = 0;
					//out("%d号端口开着电但无设备存在\n",state->port);
					port_power_down(state);
				}
			}
			continue;
		}//检测到没有设备结束
		onlineport += 1;//在线通电设备数量加1
		//以下为文件系统检测到有设备
		state->port_info.with_dev = 1;

		if (state->port_init == 0 && state->h_plate.need_free != 1)//判断设备是否发生过变化,0变化 1未变,读取过端口设备信息后将此状态改为1,软件开关不再进入此操作  
		{
			//out("%d号端口设备初次上电,需读取服务信息\n", port);
			memset(state->port_info.busid, 0, sizeof(state->port_info.busid));
			//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_BUSID, port, state->port_info.busid);
			result = get_busid_info(port, state->port_info.busid);
			if (result < 0)
			{
				out("%d号端口，获取设备busid信息失败\n", state->port);
				//state->port_info.with_dev = 0;
				continue;
			}
			memset(&dev, 0, sizeof(dev));
			result = _usb_file_get_infor(state->port_info.busid, &dev);
			if (result < 0)
			{
				out("%d号端口，获取设备dev信息失败\n",state->port);
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
				if ((state->power_mode == Saving_Mode) && (state->power == 1) && (state->port_info.port_used == 0))//如果是节电版，在端口通电是设备被拔除，需给端口下电
				{
					state->port_init = 0;
					state->sys_reload = 1;
					state->need_up_power = 0;//正在上电中，避免重复操作
					//out("%d号端口是节电版端口不识别的税盘类设备进入节电功能",state->port);
					port_power_down(state);
				}
#endif
			}
			//out("【%d号口】检测到有设备插入\n", port);
		}
	}

	stream->onlineport = onlineport;
}



#ifdef ALL_SHARE

//base64解码3des解密--明文
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
//printf("去除base64补位\n");

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
//printf("最后面的补1\n");
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
//解析读名称税号应答JSON数据
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
	{	//out("获取协议版本\n");
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
	//out("获取版本信息\n");
	memset(edit, 0, sizeof(edit));
	memset(len_buf, 0, sizeof(len_buf));
	memset(order, 0, sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
	if (ty_socket_read(fd, edit, sizeof(edit))<0)
	{
		out("获取版本信息出错\n");
		return -1;
	}
	if ((edit[0] != 0x01) && (edit[1] != 0x10))
	{
		out("获取到的版本信息不对%02x %02x\n", edit[0], edit[1]);
		return -2;
	}
	//out("版本信息成功 %02x %02x\n",edit[0],edit[1]);
	//out("获取长度信息\n");
	if (ty_socket_read(fd, len_buf, sizeof(len_buf))<0)
	{
		out("长度信息获取失败\n");
		return -1;
	}
	buf_len = len_buf[0] * 256 + len_buf[1];
	//out("本帧长度:%d\n",buf_len);
	if (buf_len<6)
	{
		out("长度小于6不对\n");
		return -3;
	}
	if (buf_len - 6>(*out_len))
	{
		out("长度过长：%d\n", buf_len - 6);
		return -4;
	}
	if (ty_socket_read(fd, order, sizeof(order))<0)
	{
		out("获取命令字失败\n");
		return -1;
	}
	order_c = order[0] * 256 + order[1];
	if (order_c != cmd)
	{	//out("命令字不一致或为错误代码\n");
		return -1;
	}
	if (ty_socket_read(fd, out_buf, buf_len - 6)<0)
	{
		out("获取数据失败\n");
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
		out("找不到匹配ca/税盘库:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
		return CA_READ_ERR_NOT_FOUND_SO;
	}
	out("找到匹配ca/税盘库:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
	inbuf[0] = 0x01; inbuf[1] = 0x10;
	inbuf[2] = 0x00; inbuf[3] = 0x26;
	inbuf[4] = 0x05; inbuf[5] = 0x01;
	memcpy(inbuf + 6, state->port_info.busid, sizeof(state->port_info.busid));

	socketfd = socket_tcp_open_send_recv_timeout("127.0.0.1", 10003, inbuf, 38);
	if (socketfd < 0)
	{
		//out("创建并发送数据失败\n");
		//out("read_ukey_ca_name_1 在此处检测读CA进程是否启动\n");
		//stream->execute_pro = 1;	 //标记需要启动读CA进程
		state->port_info.ca_read = 1;//标记此端口需要进行读CA证书信息操作
		return -1;
	}
	out_len = sizeof(outbuf);
	//out("发送完成，等待接收\n");
	result = unpack_ca_data(socketfd, outbuf, &out_len, 0x1501);
	if (result < 0)
	{
		//out("获取数据失败或超时\n");
		close(socketfd);
		//out("在此处检测读CA进程是否启动\n");
		return -2;
	}
	//out("读到的CA加密信息长度为%d,数据为%s\n",out_len,outbuf);
	close(socketfd);
	memcpy(name_tmp, outbuf, out_len);
	memset(key, 0, sizeof(key));
	sprintf(key, "%04x%04x", state->port_info.vid, state->port_info.pid);
	memset(json_data, 0, sizeof(json_data));
	base_3des_data((uint8 *)name_tmp, (uint8 *)json_data, strlen(name_tmp), (unsigned char *)key);
	//out("MAIN 解密后的数据:\n%s\n",json_data);

	_lock_set(state->lock);
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "result", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 获取执行结果失败1\n");
		state->port_info.ca_read = 1;//标记此端口需要进行读CA证书信息操作
		return -3;
	}
	out("result = %d\n", valueInt);
	state->port_info.ca_ok = valueInt;
	result = analysis_json_data(json_data, "encording", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 获取执行结果失败2\n");
		state->port_info.ca_read = 1;//标记此端口需要进行读CA证书信息操作
		return -4;
	}
	out("encording = %d\n", valueInt);
	state->port_info.encording = valueInt;
	memset(valueString, 0, sizeof(valueString));
	result = analysis_json_data(json_data, "cert_num", &valueInt, valueString);
	if (result <0)
	{
		_lock_un(state->lock);
		out("read_ukey_ca_name_1 获取执行结果失败3\n");
		state->port_info.ca_read = 1;//标记此端口需要进行读CA证书信息操作
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
		out("utf8编码\n");
		memset(valueString, 0, sizeof(valueString));
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		//out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		utf82gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	else if (state->port_info.encording == 0)
	{
		out("unicode编码\n");
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
	//{	out("获取ukey类型失败\n");
	//	state->port_info.plate_type = 0;
	//}
	//else
	//{	out("ukey_type = %d\n",valueInt);
	//	state->port_info.plate_type = valueInt;
	//}

	result = analysis_json_data(json_data, "extension", &valueInt, valueString);
	if (result <0)
	{
		out("获取分盘盘号失败\n");
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
		out("获取税盘编号失败\n");
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
		//out("设备刚启动,延时5秒后再读取证书名称\n");
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
			//out("等待对时完成\n");
			return;
		}
		stream->frist_start = 2;
		return;
	}
	//out("task_check_port 总数 %d\n",stream->usb_port_numb);
	for (i = 1; i <= stream->usb_port_numb; i++)
	{	//out("检测第%d号端口\n",i);
		state = &stream->port[i - 1];



		if ((state->port_info.with_dev == 0) || (state->retry_forbidden))
		{
			continue;
		}
		if (state->in_read_base_tax != 0)
		{
			//out("税盘上传使用中\n");
			continue;
		}
		if (state->port_info.tax_read == 1)
		{
						
			if (state->power_mode == Saving_Mode)
			{
				if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port) < 0)
				{
					//out("%d端口读监控信息前被拔出了!\n", state->port);
					logout(INFO, "SYSTEM", "CHECK_PORT", "【%d号口】读取监控信息前被拔出了2\r\n", state->port);
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

				//蒙柏模拟盘需延时8秒方可读取信息
				long time_now;
				time_now = get_time_sec();
				if ((time_now - state->insert_time) < 8)
				{
					continue;
				}
			}
		


			//out("%d号端口发票上传判断\n", state->port);
			
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
			//out("%d号端口需要执行基础信息读取操作\n",state->port);
			//logout(INFO, "system", "基础信息读取", "%d号端口需要读取基础信息,即将开启线程队列\r\n", i);
			_queue_add("up_m_server", deal_read_plate_base_tax_info, state, 10);
			continue;
		}
#ifdef ALL_SHARE
		else if (state->port_info.ca_read == 1)
		{
			out("%d端口需要读取CA信息!\n", state->port);

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if (usb_type !=0)
			{
				state->port_info.ca_read = 0;
				continue;
			}


			if (port_lock(state, 0, 2, DF_PORT_USED_DQMCSHXX) < 0)		
			{
				out("端口%d,在使用中,无法读取证书名称\n",i);
				continue;
			}
			struct _careader_process careader_process;
			memset(&careader_process, 0, sizeof(struct _careader_process));
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			//读取非税盘的纳税人名称和税号
			if (atoi(careader_process.process_run) != 1)//CA进程未启动
			{
				sleep(8); //out("等待进程启动,不清除读名称税号标志,下次继续读取\n");
				out("等待进程启动,不清除读名称税号标志,下次继续读取\n");
			}
			else//进程已经启动
			{
				if (state->port_info.vid == 0x096e && state->port_info.pid == 0x0702)
				{
					if (access("/etc/ca_read", 0) != 0)//不需要读取该ca名称税号
					{
						state->port_info.usb_app = 5;
						state->fail_num = 30;
						state->port_info.ca_read = 0;
						sleep(1);
						out("证书读取失败，上报【%d】端口其他usb设备\n", i);
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						goto END;
					}
				}
				int result = read_ukey_ca_name_1(state);
				if (result < 0)
				{
					//out("CA读取证书失败--端口号:[%d] fail_num:%d\n", i,state->fail_num);
					state->port_info.usb_app = 5;
					state->fail_num += 1;
					if (result == CA_READ_ERR_NOT_FOUND_SO)
					{

						state->fail_num = 30;
						state->port_info.ca_read = 0;
					}
					sleep(1);
					out("证书读取失败，上报【%d】端口其他usb设备\n", i);
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					goto END;
				}
				else
				{
					if (judge_dev_down_power(state) == 0)
					{
						out("电子口岸设备特殊处理!\n");

					}
					out("证书读取成功，上报【%d】端口ca设备\n", i);
					state->port_info.usb_app = 4;
					state->up_report = 1;//端口状态更新需要上报
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

//模拟盘命令字
#define DF_PC_USB_ORDER_CONNECT			0X01				//联机命令
#define DF_PC_USB_ORDER_GET_CARD_IFNFO	0X11				//读取卡信息

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
		out("未联机\n");
		sprintf(errinfo, "未联机");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFE:
		out("时间格式错误\n");
		sprintf(errinfo, "时间格式错误");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFD:
		out("卡编号不符合规范\n");
		sprintf(errinfo, "卡编号不符合规范");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFC:
		out("未注册\n");
		sprintf(errinfo, "未注册");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFB:
		out("未开卡\n");
		sprintf(errinfo, "未开卡");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xFA:
		out("此卡已格式化，请重新上电\n");
		sprintf(errinfo, "此卡已格式化，请重新上电");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF9:
		out("开卡错误,该盘已经开卡\n");
		sprintf(errinfo, "开卡错误,该盘已经开卡");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF8:
		out("无此发票类型\n");
		sprintf(errinfo, "无此发票类型");
		errnum = DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
		break;
	case 0xF7:
		out("无可用发票\n");
		sprintf(errinfo, "无可用发票");
		errnum = DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		break;
	case 0xF6:
		out("口令错误\n");
		sprintf(errinfo, "口令错误");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF5:
		out("数字证书口令为8位\n");
		sprintf(errinfo, "数字证书口令为8位");
		errnum = DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
		break;
	case 0xF4:
		out("该发票类型代码已存在\n");
		sprintf(errinfo, "该发票类型代码已存在");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF3:
		out("该授权税率已存在\n");
		sprintf(errinfo, "该授权税率已存在");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF2:
		out("该发票段存在重复\n");
		sprintf(errinfo, "该发票段存在重复");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF1:
		out("发票库存已满\n");
		sprintf(errinfo, "发票库存已满");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xF0:
		out("无此发票段\n");
		sprintf(errinfo, "无此发票段");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xEF:
		out("购货单位是必填项\n");
		sprintf(errinfo, "购货单位是必填项");
		errnum = DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
		break;
	case 0xEE:
		out("超过开票截至日期\n");
		sprintf(errinfo, "超过开票截至日期");
		errnum = DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		break;
	case 0xED:
		out("超过单张发票限额\n");
		sprintf(errinfo, "超过单张发票限额");
		errnum = DF_TAX_ERR_CODE_EXCEED_SINGLE_LIMIT;
		break;
	case 0xEC:
		out("超过离线时长\n");
		sprintf(errinfo, "超过离线时长");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT_TIME;
		break;
	case 0xEB:
		out("超过离线累计金额\n");
		sprintf(errinfo, "超过离线累计金额");
		errnum = DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
		break;
	case 0xEA:
		out("无此授权税率\n");
		sprintf(errinfo, "无此授权税率");
		errnum = DF_TAX_ERR_CODE_TAX_RATE_UNSUPPORTED;
		break;
	case 0xE9:
		out("数据库无此发票\n");
		sprintf(errinfo, "数据库无此发票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE8:
		out("未到抄报时间\n");
		sprintf(errinfo, "未到抄报时间");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE7:
		out("无需监控信息回传\n");
		sprintf(errinfo, "无需监控信息回传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE6:
		out("记录超出离线票范围\n");
		sprintf(errinfo, "记录超出离线票范围");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE5:
		out("此票已作废\n");
		sprintf(errinfo, "此票已作废");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE4:
		out("不能作废隔月票\n");
		sprintf(errinfo, "不能作废隔月票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE3:
		out("系统时钟错误\n");
		sprintf(errinfo, "系统时钟错误");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE2:
		out("未抄报，不能进行监控信息回传\n");
		sprintf(errinfo, "未抄报，不能进行监控信息回传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE1:
		out("监控信息回传，开票截至日期应往后顺延\n");
		sprintf(errinfo, "监控信息回传，开票截至日期应往后顺延");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xE0:
		out("有作废发票未上传，先上传作废发票\n");
		sprintf(errinfo, "有作废发票未上传，先上传作废发票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDF:
		out("有离线票未上传，请先上传离线票\n");
		sprintf(errinfo, "有离线票未上传，请先上传离线票");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDE:
		out("占未支持该命令\n");
		sprintf(errinfo, "占未支持该命令");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDD:
		out("有红票未上传\n");
		sprintf(errinfo, "有红票未上传");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	case 0xDC:
		out("空白废不支持作废\n");
		sprintf(errinfo, "空白废不支持作废");
		errnum = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		break;
	default:
		out("错误代码未定义\n");
		sprintf(errinfo, "模拟盘错误代码未定义");
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
			out("数据发送失败,result = %d\n", result);
			sprintf(errinfo, "模拟盘底层数据写入超时失败");
			free(r_data);
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}

	}
	//out("准备进行数据接收\n");	
	result = libusb_interrupt_transfer(husb->handle, 0x82, r_data, 64, &size, 2000);
	if (result < 0)
	{
		out("数据接收失败,result = %d\n", result);
		sprintf(errinfo, "模拟盘底层数据应答超时失败");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	if ((r_data[0] != 0x4d) || (r_data[1] != 0x42))
	{
		out("版本号错误\n");
		sprintf(errinfo, "模拟盘底层数据应答版本号错误");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	total_len = r_data[2] * 0x100 + r_data[3];
	left_len = total_len - 22;
	if (total_len > *out_len)
	{
		out("接收缓冲去内存太小\n");
		sprintf(errinfo, "模拟盘底层数据接收缓冲区太小");
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
		out("crc校验错误\n");
		sprintf(errinfo, "模拟盘底层应答CRC数据校验错误");
		free(r_data);
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	if (r_data[4] != 0)
	{
		out("USB操作应答为错误代码,%02x\n", r_data[4]);

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
	//联机命令
	//////////////////////////////////////////////////////////////////////////
	//out("联机命令\n");
	char time_now[20];
	memset(time_now, 0, sizeof(time_now));

	timer_read_y_m_d_h_m_s(time_now);
	//out("设置当前时间%s\n", time_now);

	strcpy(HDev->szDeviceTime, time_now);

	//////////print_array((char *)usb_data, size);
	//////////////////////////////////////////////////////////////////////
	memset(s_data, 0, sizeof s_data);
	in_len = pack_s_buf(DF_PC_USB_ORDER_CONNECT, 0, (unsigned char *)time_now, s_data, strlen(time_now));
	out_len = sizeof(r_data);

	result = usb_data_write_read(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("联机失败\n");
		return result;
	}

	//获取卡信息
	////////////////////////////////////////////////////////////////////////
	//out("获取卡信息\n");
	memset(s_data, 0, sizeof(s_data));
	memset(r_data, 0, sizeof(r_data));
	in_len = pack_s_buf(DF_PC_USB_ORDER_GET_CARD_IFNFO, 0, NULL, s_data, 0);
	out_len = sizeof(r_data);
	result = usb_data_write_read(device, s_data, in_len, r_data, &out_len, errinfo);
	if (result < 0)
	{
		out("获取卡信息失败\n");
		return -1;
	}
	//out("获取卡信息成功\n");

	HDev->uICCardNo = r_data[122] * 0x1000000 + r_data[123] * 0x10000 + r_data[124] * 0x100 + r_data[125] * 0x1;
	memcpy(HDev->szDeviceID, r_data, 12);
	memcpy(HDev->szCompanyName, r_data + 32, 80);
	//out("获取到的纳税人名称：%s\n", device->szCompanyName);
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
			out("设备打开成功,联机失败\n");
			usb_hid_close_serssion(hUSB, hDev);
			return -3;
		}
	}
	else
	{
		out("设备打开失败,result = %d\n", nhUsbType);
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
			out("设备打开失败,result = %d\n", result);
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
		out("当前端口%d操作USB时出现过错误,延时3秒再进行读取\n", state->port);
		if (state->power_mode == Normal_Mode)
		{
			//logout(INFO, "system", "基础信息读取", "端口：%d上的USB设备基础信息读取失败,重启端口电源后再次重试\r\n", state->port);
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
			out("【%d】税盘读取名称税号失败\n", state->port);

			state->usb_err = 1;//记录USB出现过错误
			if (passwd_right == 0)
			{
				out("记录到税盘设备口令有误!\n");
				state->retry_forbidden = 1;
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "税盘设备密码错误");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				state->port_info.usb_app = 4;
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
			}
			port_unlock(state);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		_lock_set(state->lock);
		state->usb_err = 0;//操作成功清除USB错误记录
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
		memcpy(state->port_info.ca_name, ca_name, strlen(ca_name));
		memcpy(state->port_info.ca_serial, ca_serial, strlen(ca_serial));
		memcpy(state->port_info.sn, sn, strlen(sn));
		state->port_info.usb_app = 4;
		state->port_info.ca_ok = 3;
		state->port_info.extension = 0;
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		out("【端口号：%d,盘号：%s,名称：%s】\n", state->port, state->port_info.sn, state->port_info.ca_name);
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
		//out("设备刚启动,延时5秒后再读取证书名称\n");
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
			//out("等待对时完成\n");
			return;
		}
		stream->frist_start = 2;
		return;
	}
	//out("task_check_port 总数 %d\n",stream->usb_port_numb);
	for (i = 1; i <= stream->usb_port_numb; i++)
	{	//out("检测第%d号端口\n",i);
		state = &stream->port[i - 1];



		if ((state->port_info.with_dev == 0) || (state->retry_forbidden))
		{
			continue;
		}
		if (state->in_read_base_tax != 0)
		{
			//out("税盘上传使用中\n");
			continue;
		}
		if (state->port_info.tax_read == 1)
		{

			if (state->power_mode == Saving_Mode)
			{
				if (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port) < 0)
				{
					out("%d端口读监控信息前被拔出了!\n", state->port);
					logout(INFO, "SYSTEM", "CHECK_PORT", "【%d号口】读取监控信息前被拔出了1\r\n", state->port);
					state->port_info.tax_read = 0;
					port_power_down(state);
					continue;

				}
			}

			usb_type = vid_pid_jude_plate(state->port_info.vid, state->port_info.pid);
			//out("vid = %04x,pid = %04x\n", state->port_info.vid, state->port_info.pid);
			if (usb_type == DEVICE_MENGBAI)
			{

				//蒙柏模拟盘需延时8秒方可读取信息
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


			//out("%d号端口发票上传判断\n", state->port);

			if (stream->base_read_queue_num > 4)
			{
				//out("上传队列大于5个,stream->up_queue_num = %d\n", stream->up_queue_num);
				continue;
			}
			if (state->port_info.used_level == 1)
			{
				continue;
			}

			stream->base_read_queue_num += 1;
			state->in_read_base_tax = 1;
			//out("%d号端口需要执行基础信息读取操作\n",state->port);
			//logout(INFO, "system", "基础信息读取", "%d号端口需要读取基础信息,即将开启线程队列\r\n", i);
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
		if (state->port_info.usb_app == DEVICE_AISINO)//金税盘
		{
#ifdef RELEASE_SO
			result = so_aisino_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_aisino_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_NISEC)//税控盘
		{
#ifdef RELEASE_SO
			result = so_nisec_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_nisec_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_CNTAX)//税务UKEY
		{
#ifdef RELEASE_SO
			result = so_cntax_get_auth_passwd(&state->h_plate, &plate_infos);
#else
			result = function_cntax_get_auth_passwd(&state->h_plate, &plate_infos);
#endif
		}
		else if (state->port_info.usb_app == DEVICE_MENGBAI)//模拟盘
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
				logout(INFO, "system", "passwd_auth", "%s,该端口税盘证书口令初次验证失败,可能原因为税盘口令非默认口令,剩余口令验证次数%d\r\n", state->port_info.port_str, state->port_info.plate_infos.plate_basic_info.cert_left_num);
				//state->cert_err = -1;
				update_cert_passwd_err_port((char *)state->port_info.sn, state->port_info.plate_infos.plate_basic_info.cert_left_num);
			}
			else
			{
				logout(INFO, "system", "passwd_auth", "%s,该端口税盘证书口令初次验证失败,可能原因为USB操作失败\r\n", state->port_info.port_str);
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
	

	
	//out("证书口令处理完成\n");
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
		out("当前端口%d操作USB时出现过错误,延时3秒再进行读取\n",state->port);
		if(state->power_mode == Normal_Mode)
		{
			//logout(INFO, "system", "基础信息读取", "端口：%d上的USB设备基础信息读取失败,重启端口电源后再次重试\r\n", state->port);
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
			//out("端口%d,在使用中,无法读取证书名称\n",i);
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
			out("【%d】税盘读取名称税号失败\n", state->port);
			state->usb_err = 1;//记录USB出现过错误
			if (plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_ERROR)
			{
				//out("记录到税盘设备口令有误!\n");
				logout(INFO, "system", "基础信息读取", "记录到%d号端口,税盘设备口令有误!,剩余尝试次数%d次\r\n", state->port,plate_infos.plate_basic_info.dev_left_num);
				state->retry_forbidden = 1;
				update_dev_passwd_err_port(state->port, plate_infos.plate_basic_info.dev_left_num);
				state->need_getinv = 0;//已上传发票统计查询
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "税盘设备密码错误");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "税盘设备密码错误");
				memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
				memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);				
				state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
				state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
				state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
				state->port_info.tax_read = 0;
			}
			else if (plate_infos.plate_basic_info.dev_err_reason == ERR_DEVICE_PASSWORD_LOCK)
			{
				//out("记录到税盘设备口令有误!\n");
				logout(INFO, "system", "基础信息读取", "记录到%d号端口,税盘设备口令有误!,剩余尝试次数%d次\r\n", state->port, plate_infos.plate_basic_info.dev_left_num);
				//state->retry_forbidden = 1;//端口已锁定可以重试
				state->need_getinv = 0;//已上传发票统计查询
				memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
				sprintf((char *)state->port_info.ca_name, "税盘设备密码错误");
				memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
				sprintf((char *)state->port_info.ca_serial, "000000000000000");
				memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
				sprintf((char *)state->port_info.sn, "USB Port %03d", state->port);
				memset(state->port_info.plate_infos.plate_basic_info.ca_name, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_name));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_name, "税盘设备密码错误");
				memset(state->port_info.plate_infos.plate_basic_info.ca_number, 0, sizeof(state->port_info.plate_infos.plate_basic_info.ca_number));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.ca_number, "000000000000000");
				memset(state->port_info.plate_infos.plate_basic_info.plate_num, 0, sizeof(state->port_info.plate_infos.plate_basic_info.plate_num));
				sprintf((char *)state->port_info.plate_infos.plate_basic_info.plate_num, "USB Port %03d", state->port);
				state->port_info.usb_app = plate_infos.plate_basic_info.plate_type;
				state->port_info.plate_infos.plate_basic_info.dev_err_reason = plate_infos.plate_basic_info.dev_err_reason;
				state->port_info.plate_infos.plate_basic_info.cert_passwd_right = 0;
				state->up_report = 1;//端口状态更新需要上报
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
		state->usb_err = 0;//操作成功清除USB错误记录
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
		sprintf(state->port_info.port_str,"【端口号：%d,盘号：%s,名称：%s】",state->port,state->port_info.sn,state->port_info.ca_name);
		for (i = 0; i < DF_MAX_INV_TYPE; i++)
		{
			memset(&state->port_info.plate_infos.invoice_type_infos[i], 0, sizeof(struct _invoice_type_infos));
			memcpy(&state->port_info.plate_infos.invoice_type_infos[i], &plate_infos.invoice_type_infos[i], sizeof(struct _invoice_type_infos));

		}


		//wang 02 03
		//目前这边暂时这样处理 但是代码后面还会继续运行
		//已通知运维出现报错后立即拔盘 防止state->port_info.plate_sys_time获取错误值运行到后面功能导致其他问题
		if (strlen((char *)state->port_info.plate_time) != 0)
		{
			// wang 02 03 防止读到税盘时间错误，计算后出现段错误
			// eg 20a0-03-01 02:12:26
			unsigned int iYear, iMon, iDay, iHour, iMin, iSec;
			sscanf((char *)state->port_info.plate_time, "%04d-%02d-%02d %02d:%02d:%02d", &iYear, &iMon, &iDay, &iHour, &iMin, &iSec);
			if((iYear < 1970)||(iMon < 1)||(iMon > 12)||(iDay < 1)||(iDay > 31)){
				out("%d号端口税盘时间读取错误,读取到时间为%s,请拔出该盘并联系客户在电脑上确认税盘时间!\r\n", state->port,(char *)state->port_info.plate_time);
				logout(INFO, "system", "基础信息读取", "%d号端口税盘时间读取错误,读取到时间为%s,请拔出该盘并联系客户在电脑上确认税盘时间!\r\n", state->port,(char *)state->port_info.plate_time);		
			}
			else 
			{
				long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
				long time_now = get_time_sec();
				//out("税盘时间戳为%ld,当前系统时间戳为%ld\n", plate_time, time_now);
				state->port_info.plate_sys_time = plate_time - time_now;
				//out("税盘与当前系统时间差为%d\n", state->port_info.plate_sys_time);
			}
		}

		/*if (strlen((char *)state->port_info.plate_time) != 0)
		{
			long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
			long time_now = get_time_sec();
			//out("税盘时间戳为%ld,当前系统时间戳为%ld\n", plate_time, time_now);
			state->port_info.plate_sys_time = plate_time - time_now;
			//out("税盘与当前系统时间差为%d\n", state->port_info.plate_sys_time);
		}*/

		state->port_init = 2;
		_lock_un(state->lock);


		


		result = deal_read_plate_cert_passwd(state);
		if (result < 0)
		{
			state->usb_err = 1;//记录USB出现过错误
			port_unlock(state);
			stream->base_read_queue_num -= 1;
			state->in_read_base_tax = 0;
			return;
		}
		port_unlock(state);

	
		
		_lock_set(state->lock);
		//out("%d号端口的金税盘全部税务信息获取完成,唤醒抄报清卡线程\n", state->port);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		state->port_info.need_chaoshui = 1;
		state->port_info.need_huizong = 1;
		state->port_info.need_fanxiejiankong = 1;
		_lock_un(state->lock);
		sem_post(&stream->cb_qk_sem);

#ifndef DF_OLD_MSERVER			
		state->last_report_time_t = 0;
#endif
		state->need_getinv = 1;//已上传发票统计查询
		
		state->port_info.tax_read = 0;
		logout(INFO, "system", "基础信息读取", "%s,税号：%s,分机号：%d,税盘类型：%d  tax_read:%d\r\n", state->port_info.port_str, state->port_info.ca_serial, state->port_info.extension, state->port_info.usb_app, state->port_info.tax_read);
		//upfile_flag = 1;

	}
	else
	{
	//	logout(INFO, "system", "基础信息读取", "%d端口为其他设备\r\n", state->port);
		state->up_report = 1;//端口状态更新需要上报
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
		return DEVICE_AISINO;//还有可能是税务ukey DEVICE_CNTAX
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


//启动面板灯管理
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
			if (state->retry_forbidden == 1)//端口插过设备密码不正确的税盘，此端口即锁定，不在允许使用
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
			if (state->retry_forbidden == 1)//端口插过设备密码不正确的税盘，此端口即锁定，不在允许使用
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
				goto SET_LED;
			}
#ifdef ALL_SHARE
			else if ((( (jude_port_led_auth(state) == -1) || (jude_port_led_auth(state) == -2) ) || (state->port_info.usb_app != 4 && jude_port_led_auth(state) < 0) ) && state->port_info.plate_infos.plate_basic_info.dev_err_reason >=0)//未授权或授权到期 
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
		

	//out("检测有无端口被USBshare 加载\n");
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

	//out("检测端口有无正在操作税务信息\n");
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
			//out("lcd 指示无设备操作税务信息\n");
			lcd_print_info.tax_state = 0;
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_tax_state = 10;
		}

	}
	else
	{
		//out("无端口操作税务信息\n");
		if (stream->lcd_tax_state != 20)
		{
			//out("lcd 指示有设备操作税务信息\n");
			lcd_print_info.tax_state = 1;			
			ty_ctl(stream->lcd_fd, DF_LCD_STATE_UPDATE_STATE);
			stream->lcd_tax_state = 20;
		}

	}
}




/*==========================================ctl函数===============================================*/

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
//切换整机节电模式
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

//改变端口电源模式 port 端口号  mode:模式 0为非省电  1 为省电
int change_port_power_mode(int port,int mode)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int result;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	state = &stream->port[port - 1];
	if (stream->dev_support.support_power == 0)//非节电版本不支持
		return -1;
	if (mode == 0)
	{
		state->power_mode = Normal_Mode;
		//out("change_port_power_mode mode 0 关闭%d号端口\n",state->port);
		//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);

		port_power_down(state);
		//if (result < 0)
		//{
		//	logout(INFO, "SYSTEM", "INIT", "通知单片机关闭端口电源失败,端口号%d\r\n", state->port);
		//}
		//state->power = 0;
		usleep(10000);
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, state->port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "INIT", "通知单片机打开闭端口电源失败,端口号%d\r\n", state->port);
		}
		state->power = 1;
	}
	else if (mode == 1)
	{
		//out("change_port_power_mode mode 1 关闭%d号端口\n", state->port);
		/*result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, state->port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "INIT", "通知单片机关闭端口电源失败,端口号%d\r\n", state->port);
		}
		state->power = 0;*/
		port_power_down(state);
		usleep(10000);
		state->power_mode = Saving_Mode;
	}
	return 0;
}

//返回值负数或者载入字节数
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
        pDst = (uint8 *)pBuff;  //静态数组第一个首地址就是数组地址
    }
    FILE *fp = fopen(szPath, "r");
    if (fp == NULL)
        goto ReadFileFromPath_Finish;
    int nByteIO = fread(pDst, 1, nReadMax, fp);
    fclose(fp);
    // 4096时有可能时对齐大小，实际文件并无4096大小，不判断
    if (nReadMax != 4096) {
        //如果是文件才判断大小，否则设备类的都是4096大小
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
        char szStr[4096] = "";  // ReadBinFile 读取大小4k
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
        *nBusID = atoi(szStr);  //末尾有个\n不影响
        sprintf(szMountPath, "/sys/bus/usb/devices/1-%s/devnum", busid);
        memset(szStr, 0, sizeof(szStr));
        if (ReadBinFileIn(szMountPath, (void **)szStr, sizeof(szStr)) < 0) {
            //printf("Read mount path failed :%s", szMountPath);
            break;
        }
        *nDevID = atoi(szStr);  //末尾有个\n不影响
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

	stream->port[port - 1].up_report = 1;//端口状态更新需要上报
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
	out("[open_port_ip]按端口号打开端口 port = %d\n", port);
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
		if (state->port_info.usb_app == DEVICE_CNTAX && (state->port_info.vid == 0x101d && state->port_info.pid == 0x0003))//税务Ukey
		{
			out("开始加载端口上下电:%d\n");
			state->sys_reload = 1;
			/*ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			state->power = 0;*/
			port_power_down(state);
			usleep(20000);
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			state->power = 1;
			sleep(2);
			state->sys_reload = 0;
			out("结束加载端口上下电\n");
		}
	}
	if (stream->port[port - 1].port_info.with_dev == 0)
	{
		out("[open_port_ip]没有发现该设备，最多超时等待5秒\n");

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
			out("[open_port_ip]该端口已经打开,没有发现该设备\n");
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
	}
	//if(stream->port[port-1].port_info.port_status!=0)
	//{   _lock_un(stream->port[port-1].lock);
	//    out("[open_port_ip]该端口已经打开在使用\n");
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
	//	out("系统正在占用此USB设备,端口号：%d\n", port);
	//	return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	//}

	if ((stream->port[port - 1].port_info.port_status != 0) && (stream->port[port - 1].port_info.used_level == 1))
	{
		out("端口已经被加载\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}

	if (port_lock(&stream->port[port - 1], 10, 1, DF_PORT_USED_USB_SHARE) < 0)
	{
		out("加载端口锁定端口失败\n");
		return DF_ERR_PORT_CORE_TY_USB_PORT_MANAGE_PORT_USEED;
	}
	if ((result = open_port(stream, port, 0x7f000001, NULL))<0)
	{
		stream->port[port - 1].port_info.port_status = 0x00;
		out("[open_port_ip]打开端口失败\n");
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
	logout(INFO, "usbshare", "attach", "%s,10001链路加载USB端口,端口号：%d,使用者IP：%s\r\n", state->port_info.port_str, port, ip_str);
	if (strlen(client) != 0)
		memcpy(stream->port[port - 1].port_info.client, client, strlen(client));
	*port_fd = stream->port[port - 1].port_info.app_fd;
	stream->port[port - 1].port_info.last_open_time = get_time_sec();
	stream->port[port - 1].up_report = 1;//端口状态更新需要上报
	stream->port[port - 1].last_act = 1;
	_lock_un(stream->port[port - 1].lock);
	return port;
}
//强制关闭端口
int close_port_dettach(int port)
{
	struct _ty_usb_m  *stream;
	//int result;
	struct _port_state  *state;
	stream = (struct _ty_usb_m  *)ty_usb_m_id;
	out("使用强制关闭端口,端口号【%d】\n", port);
	if (port<0 || port>stream->usb_port_numb)
	{
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}

	state = &stream->port[port - 1];
	//if (state->power_mode == Saving_Mode)
	//{
	//	out("使用强制关闭端口,释放USB句柄端口号【%d】\n", port);
	//	free_h_plate(state);
	//}
	close_port_socket(stream, port);
	logout(INFO, "10001", "dettach", "%s,强制释放USB端口,端口号：%d\r\n", stream->port[port - 1].port_info.port_str, port);
	if (state->power_mode == Saving_Mode)
	{		
#ifdef ALL_SHARE
		state->port_info.ca_read = 1;
#endif		
	}
	return 0;
}

//端口关闭处理逻辑
static int close_port_socket(struct _ty_usb_m *stream, int port)
{
	struct _port_state   *state;
	state = &stream->port[port - 1];
	out("close_port_socket进入关闭流程,端口号【%d】\n", port);
	_lock_set(stream->lock);
	state->port_info.with_dev = 1;//操作类型为关闭
	_lock_un(stream->lock);
	out("***************进行关闭socket操作**************\n");

	if ((state->port_info.port_status == 0) && (state->port_info.used_level != 1))
	{
		out("端口加载已经关闭，无需再次关闭\n");
		return 0;
	}

	state->port_info.port_used = 0;
	if (state->power_mode == Normal_Mode)
	{
		state->sys_reload = 0;
		close_port_ty_usb(stream, port);	//不判断直接关闭电源

	}
	else
	{
		state->sys_reload = 0;
		port_power_down(state);
	}

	out("关闭转发链路,关闭端口=%d\n", port);
	ty_ctl(stream->tran_fd, DF_CLOSE_LINE_TRAN_TASK, port);

	_lock_set(stream->lock);
	state->port_info.usbip_state = 0;
	state->port_info.check_num = 0;
	state->port_info.port_status = 0x00;//端口状态为打开过,但现已关闭   
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
	//解除设备的使用和更新端口状态,port_unlock会造成死锁


	//out("证书变化状态= %d\n", stream->port[port - 1].port_info.change);
	out("[close_port_socket]关闭%d端口成功\n", port);
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
	//out("[get_usb_port_information]获取%d号USB信息\n",port);
	if (vid == NULL || pid == NULL)
	{	out("USB端口参数错误\n");\

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
			state->up_report = 1;//端口状态更新需要上报
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
			//out("第%d条内存记录中盘号为：%s,要查询的盘号为:%s\n", i, stream->port[i].port_info.sn, palte_num);
			if ((memcmp(stream->port[i].port_info.sn, palte_num, 12) == 0) && (stream->port[i].in_read_base_tax == 0))//防止刚读完基础信息还未完成口令验证就进行其他操作报口令异常错误
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



//STM32打开端口
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
        out("在运行过程中,发现主hub没有找到,系统要求重新启动...\n");
        memset(name,0,sizeof(name));
        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
        fd=ty_open(name,0,0,NULL);       
        out("写事件\n");
        ty_ctl(fd,DF_EVENT_NAME_RESET,DF_EVENT_REASON_MAIN_HUB);
        ty_close(fd);
        _queue_add("reset",reset_systerm,stream,10); 
    }
    return result;
}



static int close_port_ty_usb(struct _ty_usb_m  *stream,int port)//仅非节电版会使用
{	
	int result;
	struct _port_state   *state;
	state = &stream->port[port - 1];

	state->h_plate.nBusID = -1;
	state->h_plate.nDevID = -1;
	out("close_port_ty_usb 即将关闭端口电源\n");
	//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
	//state->power = 0;
	port_power_down(state);
    usleep(80000);
	//if(result==DF_ERR_PORT_CORE_TY_USB_CLOSE)
	//{	//int fd;
	//	out("电源关闭后文件系统显示USB设备依然在线,端口号%d\n",port);
	//}
	out("关闭电源后即将重新开启端口电源\n");
	result = ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_OPEN_PORT_POWER,port);
	state->power = 1;
	if(result < 0)
	{
		out("关闭端口电源后,重新打开端口电源失败,端口号%d\n",port);
		logout(INFO, "system", "closeport", "关闭端口电源后,重新打开端口电源失败,再次打开端口电源,端口号%d\r\n", port);
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
			logout(INFO, "system", "check_port", "%s操作【%s】时长超过1分钟,系统判断该盘可能存在异常,即将对%d端口进行断电重启动作\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
			out("重启%d号端口电源\n", state->port);

			restart_plate_power(state, state->port_info.sn, state->port, 0);			
		}

	}
	else if ((strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USED_HZSC) == 0)||
		(strcmp(state->port_info.used_info, DF_PORT_USED_FXJK) == 0))
	{
		
		if (time_now - state->port_info.used_time > 120)
		{
			logout(INFO, "system", "check_port", "%s操作【%s】时长超过2分钟,系统判断该盘可能存在异常,即将对%d端口进行断电重启动作\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
			out("重启%d号端口电源\n", state->port);
			restart_plate_power(state, state->port_info.sn, state->port, 0);
		}
		if ((time_now - state->port_info.used_time > 600) && (state->port_info.used_time != 0))
		{
			logout(INFO, "system", "check_port", "%s操作【%s】时长超过10分钟,系统判断该盘可能存在异常卡死,即将重启机柜程序\r\n", state->port_info.port_str, state->port_info.used_info);
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
				logout(INFO, "system", "check_port", "%s操作【%s】时长超过20分钟,系统判断该盘可能存在异常,即将对%d端口进行断电重启动作\r\n", state->port_info.port_str, state->port_info.used_info, state->port);
				out("重启%d号端口电源\n", state->port);
				restart_plate_power(state, state->port_info.sn, state->port, 0);
			}
			if ((time_now - state->port_info.used_time > 1500) && (state->port_info.used_time != 0))
			{
				logout(INFO, "system", "check_port", "%s操作【%s】时长超过25分钟,系统判断该盘可能存在异常卡死,即将重启机柜程序\r\n", state->port_info.port_str, state->port_info.used_info);
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
			logout(INFO, "system", "check_port", "%s操作【%s】时长超过1分钟,系统判断该盘可能存在异常,即将对%d端口进行断电重启动作\r\n", state->port_info.port_str, state->port_info.used_info,state->port);
			out("重启%d号端口电源\n", state->port);
			restart_plate_power(state, state->port_info.sn, state->port, 0);
		}
		if ((time_now - state->port_info.used_time > 300) && (state->port_info.used_time != 0))
		{
			logout(INFO, "system", "check_port", "%s操作【%s】时长超过5分钟,系统判断该盘可能存在异常卡死,即将重启机柜程序\r\n", state->port_info.port_str, state->port_info.used_info);
			sleep(10);
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		}
	}


	return 0;
}

//端口检查处理逻辑
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
		logout(INFO, "system", "check", "%s,端口打开着检测到设备被拔出\r\n", state->port_info.port_str);
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
			logout(INFO, "system", "check", "%s,使用之后,status变成了err\r\n", state->port_info.port_str);
			close_port_dettach(state->port);
			_lock_un(state->lock);
			return 0;
		}
	}	

	_lock_un(state->lock);
    return 0;
}
//端口打开处理逻辑
static int open_port(struct _ty_usb_m  *stream,int port,uint32 ip,uint8 *client_id)
{   
	char busid[DF_TY_USB_BUSID] = {0};
	int result;
	memset(busid,0,sizeof(busid));

	result=ty_ctl(stream->ty_usb_fd,DF_TY_USB_CM_PORT_BUSID,port,busid);
	if(result<0)
	{   out("[open_port]获取端口[%d]busid失败\n",port);
		return result;
	}

	result=open_port_ty_usb(stream,port);
	if(result<0)
	{   
		out("[open_port]打开失败\n");
		return result;
	}

	return 1;
}


//锁定端口使用
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
				//out("端口已被锁定,无法锁定\n");
				//out("当前等级%d,正在使用时的等级%d\n", used_level, state->port_info.used_level);
				if (used_level < state->port_info.used_level)
				{
					//out("当前等级说明【%s】,正在使用时的等级【%s】,DF_PORT_USED_FPKJ = %s,DF_PORT_USED_FPCX_MQTT= %s\n", used_info, state->port_info.used_info);
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
								logout(INFO, "system", "中断", "%s,原操作为:%s,现操作为:%s,随即中断发票查询及同步\r\n",state->port_info.port_str, state->port_info.used_info, used_info);
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
								//out("检测到正在占用，直接强制结束！\n");
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
							//out("检测到正在占用，直接强制结束！\n");
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
				//端口未完全释放完成，等待
				continue;
			}
			_lock_set(state->lock);
			if (state->port_info.port_used == 1)
			{
				_lock_un(state->lock);//防止同时进入
				continue;
			}
			
			state->port_info.used_time = get_time_sec();
			state->port_info.port_used = 1;
			if (used_level == 1) //USB-SHARE使用
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
				//logout(INFO, "port_lock", "端口申请使用", "port_lock 超时模式 %d号端口再次上电获取新USB句柄【%s】\n", state->port, used_info);
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
				//logout(INFO, "port_lock", "端口申请使用", "port_lock 超时模式 %d号端口再次上电获取新USB句柄成功【%s】\n", state->port, used_info);
				
			}
			return 0;
		}
	}
	else
	{
		_lock_set(state->lock);
		if (state->port_info.port_used != 0)
		{
			//out("端口已被锁定,无法锁定\n");
			if (strcmp(used_info, DF_PORT_USED_DQMCSHXX) == 0)
			{
				//out("%d端口号读取基础信息锁定端口\n",state->port);;
			}
			else
			{
				_lock_un(state->lock);
				return -1;
			}
		}
		state->port_info.used_time = get_time_sec();
		state->port_info.port_used = 1;
		if (used_level == 1) //USB-SHARE使用
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
			//logout(INFO, "port_lock", "端口申请使用", "port_lock 非超时模式 %d号端口再次上电获取新USB句柄【%s】\n", state->port, used_info);
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
			//logout(INFO, "port_lock", "端口申请使用", "port_lock 非超时模式 %d号端口再次上电获取新USB句柄成功【%s】\n", state->port, used_info);
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
		out("%d号端口释放时遇其他接口使用，暂不关闭端口电源\n",state->port);
		return;
	}
	state->sys_reload = 1; //关闭设备保留信息
	state->ad_repair_down_time = get_time_sec();
	//out("%d号端口使用结束，关闭端口电源\n",state->port);
	port_power_down(state);
	//out("%d号端口使用结束，关闭端口电源完成\n", state->port);
	
}


//解锁端口使用
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
		//out("%d号端口，在节电模式，即将进入休眠\n",state->port);
		_queue_add("up_m_server", close_port_power_saving_mode, state, 10);		
	}
	return 0;
}

//***================================全局函数===================================***//
//***===========================================================================***//
//设置端口USB有数据传输指示led亮
int set_port_usb_data_tran(int port)
{
	struct _ty_usb_m  *stream;
	//out("设置有USB数据,端口号%d\n",port);
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

	result = ty_ctl(stream->tran_fd, DF_OPEN_TRANSPORT_LINE, parm);//开启监控共享usb指令
	if (result < 0)
	{
		return result;
		out("open_port_transport error DF_MQTT_REQUEST_SHARE_USB_PORT!\n");
	}
	return result;
}

//按月获取发票数据JSON传出
int fun_get_plate_usb_info(int port_num, struct _usb_port_infor *port_info)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	stream = ty_usb_m_id;
	state = &stream->port[port_num - 1];
	update_plate_info_cbqkzt(state);
	//out("端口号：%d,电源状态：%d,是否存在设备：%d\n", state->port_info.port, state->port_info.power, state->port_info.with_dev);
	memcpy(port_info, &state->port_info, sizeof(struct _usb_port_infor));
	port_info->port_power = state->power;
	port_info->port = state->port;//端口结构体信息中与状态结构体可能不同步
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
	
	//out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 5, DF_PORT_USED_AYCXFPSLMQTT) != 0)
	{
		sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取月发票总数\n");		
#ifdef RELEASE_SO
		len = so_aisino_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_aisino_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取月发票总数\n");
#ifdef RELEASE_SO
		len = so_nisec_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_nisec_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税控盘获取月发票总数\n");
#ifdef RELEASE_SO
		len = so_cntax_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_cntax_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("蒙柏盘获取月发票总数\n");
#ifdef RELEASE_SO
		len = so_mb_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#else
		len = function_mengbai_get_month_invoice_num(&state->h_plate, (unsigned char *)month, fp_num, fp_len);
#endif
	}
	else
	{
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		return -1;
	}
	port_unlock(state);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (len < 0)
	{
		out("获取发票总数失败\n");
	}	
	return 0;
}

//发票明细查询
int fun_get_invoice_detail_mqtt(char *plate_num, unsigned char *month, void * function, void * arg, char *errinfo, int old_new, int sfxqqd)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;

	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 5, DF_PORT_USED_AYCXFPSJMQTT) != 0)
	{
		sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘发票明细查询\n");
#ifdef RELEASE_SO
		result = so_aisino_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_aisino_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘发票明细查询\n");
#ifdef RELEASE_SO
		result = so_nisec_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_nisec_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY发票明细查询\n");
#ifdef RELEASE_SO
		result = so_cntax_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#else
		result = function_cntax_get_inv_details_mqtt(&state->h_plate, month, function, arg, errinfo, old_new, &state->port_info.inv_query, sfxqqd);
#endif
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("蒙柏盘发票明细查询\n");
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
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("发票查询失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "按月获取发票数据MQTT", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n",state->port_info.port_str);
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return result;
	}
	out("发票查询成功\n");
	return 0;
}

int fun_get_inv_from_fpdm_fphm(char *plate_num, char *fpdm, char *fphm, char **inv_data, int dzsyh, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream,(unsigned char *) plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#else
		result = function_nisec_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		if (result < 0)
		{
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#else
		result = function_cntax_fpdm_fphm_get_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		if (result < 0)
		{
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_aisino_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#else
		result = function_aisino_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		if (result < 0)
		{
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm,dzsyh, inv_data,kp_bin,kp_str);
#else
		result = function_nisec_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		if (result < 0)
		{
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
			return result;
		}
		return 0;
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey发票代码号码获取发票信息\n");
		if (port_lock(state, 20, 5, DF_PORT_USED_AFPHMCXFP) != 0)
		{
			sprintf(errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
			out("%s\n", errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#else
		result = function_cntax_fpdm_fphm_get_invs_kpstr(&state->h_plate, fpdm, fphm, dzsyh, inv_data, kp_bin, kp_str);
#endif
		port_unlock(state);
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
		if (result < 0)
		{
			out("发票查询失败\n");
			sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf(errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];

	if (state->port_info.inv_query != 0)
	{
		state->port_info.inv_query = 0;
		sleep(2);



		out("延时3秒,给%d号端口重新上下电\n",port);
		logout(INFO, "MQTT", "停止发票查询", "%s,停止发票查询动作,即将重启端口电源\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);

		
	}
	return 0;
}


//定时上传终端状态数据线程
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
		//out("外网功能未就绪,稍后重试上报终端状态\n");
		return ;
	}
#endif

	time_now = get_time_sec();
	
	//out("开始执行心跳等上报\n");
	if (time_now - stream->last_report_time_o > DF_TY_USB_MQTT_REPORT_STATUS-1)//超过15秒会触发一次上报,高灯在用代码
	{
		//out("触发一次状态上报\n");
		if (stream->report_ter_flag != 1)
		{
			if (stream->report_o_used == 0)
			{
				stream->report_o_used = 1;//标识正在使用改队列,需结束在再次启用
				//pack_report_json_data(stream);
				//out("触发一次终端老状态信息上报\n");
				if (_queue_add("0", pack_report_json_data, stream, 0)<0)
				{
					out("创建队列失败\n");
				}
			}
		}
		//如果已经上报M服务信息成功则不在进行老版本心跳数据上报,保留此代码目的为兼容高灯直接MQTT对接
		
	}
	if (time_now - stream->last_report_time_h > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT - 1)//超过60秒会触发一次上报
	{
		//out("触发一次M服务心跳\n");
		//out("触发一次终端心跳信息上报\n");
		if (stream->report_h_used == 0)
		{
			stream->report_h_used = 1;//标识正在使用改队列,需结束在再次启用
			if (_queue_add("0", pack_report_m_server_heart_data, stream, 0)<0)
			{
				out("创建队列失败\n");
			}
		}
	}
	if (time_now - stream->last_report_time_s > DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT - 1)
	{
	
		//out("检测一次M服务终端基础信息上报\n");
		if ((stream->report_s_used == 0) && (stream->report_ter_flag == 0))
		{
			//out("触发一次M服务终端基础信息上报\n");
			stream->report_s_used = 1;//标识正在使用改队列,需结束在再次启用
#ifndef DF_1201_1
			if (_queue_add("0", pack_report_m_server_status_data, stream, 0)<0)
			{
				out("创建队列失败\n");
			}
#else
			if (_queue_add("0", pack_report_m_server_status_data_linux64, stream, 0)<0)
			{
				out("创建队列失败\n");
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
	

	cJSON *dir1, *dir2, *dir3, *dir4;		//组包使用
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
	stream->last_report_time_o = get_time_sec();//更新老心跳上报时间

	stream->report_o_used = 0;
	//out("心跳发送完成\n");
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

	cJSON *dir1, *dir2, *dir3 ,*dir4;		//组包使用
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
	cJSON_AddStringToObject(dir4, "ter_version", "0-0-7-1");			//0-0预留，7-1 表示7月第1版
	cJSON_AddStringToObject(dir4, "software_version_time", "23-7-19");
	cJSON_AddStringToObject(dir4, "server_ip", "mb_192.168.0.21");
	cJSON_AddStringToObject(dir4, "server_name", "mbi");


	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	
	out("		1		g_buf = %s\n",g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[5000] = {0};
	
	out("		2		触发一次M服务终端基础信息上报\n");
	
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	stream->last_report_time_s = get_time_sec();
	if (result < 0)
	{		
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,服务器响应超时\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,服务器应答处理失败\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End ;
	}
	else
	{
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,成功\r\n");
		stream->report_ter_flag = 1;
		//out("上报M服务终端状态数据,服务器应答处理成功\n");
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
	
	cJSON *dir1, *dir2,*dir3;		//组包使用
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

	cJSON *dir1, *dir2, *dir3;		//组包使用
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
	//out("触发一次M服务终端基础信息上报\n");
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	stream->last_report_time_s = get_time_sec();
	if (result < 0)
	{		
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,服务器响应超时\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,服务器应答处理失败\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End ;
	}
	else
	{
		logout(INFO, "SYSTEM", "M服务机柜同步", "同步终端信息,成功\r\n");
		stream->report_ter_flag = 1;
		//out("上报M服务终端状态数据,服务器应答处理成功\n");
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	stream->report_s_used = 0;
	return ;
}













#ifndef DF_OLD_MSERVER

//定时上报端口信息线程
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
		//out("终端状态未上传成功不进行端口上报\n");
		return;
	}
	char *port_data;

	cJSON *port_array, *port_layer;		//组包使用
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
			state->off_up_flag = 1;//操作离线上传
		else if ((state->off_report == 0) && (state->up_report == 1))
			state->off_up_flag = 2;//操作在线上传
		else if ((state->off_report == 1) && (state->up_report == 1))
		{
			if (state->last_act == 1)//最后动作在线则先操作离线上传
				state->off_up_flag = 1;
			if (state->last_act == 2)//最后动作离线则先操作在线上传
				state->off_up_flag = 2;
		}
		else
		{
			state->off_up_flag = 2;
		}

		if (state->off_up_flag == 2)
		{
			//out("处理第%d号端口上报\n", port);
			state->up_report = 0;
			
			if (state->port_info.usb_app == DEVICE_AISINO ||
				state->port_info.usb_app == DEVICE_NISEC ||
				state->port_info.usb_app == DEVICE_MENGBAI ||
				state->port_info.usb_app == DEVICE_MENGBAI2 ||
				state->port_info.usb_app == DEVICE_CNTAX)
			{
				//out("第%d号端口触发一次税盘更新上报\n", state->port);
				pack_report_plate_info_json(state, state->off_up_flag, &port_data);
				//logout(INFO, "system", "report_plate", "【端口号：%d】上报税盘状态,report_serial = %04x\r\n", state->port, state->now_report_counts);
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
				out("第%d号端口触发一次税盘离线上报\n", state->port);
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

	//out("整体组包后压缩前数据大小：%d,压缩后数据大小：%d\n", strlen(tmp_json), json_zlib_len);

	free(tmp_json);


	char source_topic[50] = { 0 };
	char random[50] = { 0 };
	cJSON *head_layer, *data_array, *data_layer;		//组包使用
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
	//out("上报税盘发送数据：%s\n",g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_equ_ports_status_up", g_buf, g_data, DF_MQTT_FPKJXT);
	free(g_buf);
	cJSON_Delete(root);
	if (result <= 0)
	{
		if (result < 0)
		{
			logout(INFO, "system", "report_plate", "【上报税盘状态】服务器接收超时失败,即将重试\r\n");
		}
		else
		{
			logout(INFO, "SYSTEM", "report_plate", "【上报税盘状态】服务器应答处理失败,即将重试\r\n");			
		}

		fun_deal_all_port_report();//服务器应答失败，重新处理所有税盘信息
		return;
	}
	//out("获取到的返回数据为%s\n", g_data);
	analysis_ports_report_response(state, g_data);

	//logout(INFO, "system", "report_plate", "【上报税盘状态】服务器应答成功,解析处理完成\r\n");
	
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
		//out("获取到的所有端口数据为%s\n", ports_report_serial);
		if (strlen(ports_report_serial) != true_return_len)
		{
			out("返回的端口信息数据长度有误\n");
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
			//out("%d号端口获取到的上报序列号为：%02x,内存中当前序列号为:%02x\n",port, port_report_serial,state->now_report_counts);

			if (port_report_serial != state->now_report_counts)
			{
				logout(INFO, "system", "report_plate", "【端口号：%d】获取到的上报序列号为：%04x,内存中当前序列号为:%04x,内存数据不一致,需要重新上传,str = %s\r\n", port, port_report_serial, state->now_report_counts, tmp);
				//out("%d号端口获取的上报序列号不一致,需要重新上传\n",port);
				//out("%d号端口获取到的上报序列号为：%02x,内存中当前序列号为:%02x\n",port, port_report_serial,state->now_report_counts);
				if (state->now_report_counts == 0)
				{
					state->off_report = 1;
					state->last_act = 2;					
				}
				else
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
				}
			}
			else
			{
				//out("%d号端口获取的上报序列号一致,不需要重新上传\n", port);
				if (state->in_report != 0)
				{
					//out("上传税盘信息【%d号口：%s】税盘信息服务器应答处理成功\n", state->port, state->port_info.ca_name);
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


//定时获取税盘当前入库信息
static void timely_tigger_get_plate_invoice_db_info(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
		//	out("终端状态未上传成功不进行端口上报\n");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		if (stream->report_ter_flag == 0)
		{
			//	out("终端状态未上传成功不进行端口上报\n");
			return;
		}
		state = &stream->port[port - 1];
		if (state->now_report_counts == 0)
		{
			//	out("端口状态未上传成功不进行查询\n");
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
			//	//out("端口状态上传中\n");
			//	continue;
			//}
			long time_now = get_time_sec();

			if (time_now - state->last_report_time_t > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT - 1)//超过60秒会触发一次
			{				
				if (stream->report_t_used == 0)
				{
					out("查询间隔时间：%d\n", time_now - state->last_report_time_t);
					stream->report_t_used = 1;//标识正在使用改队列,需结束在再次启用
					stream->sync_queue_num += 1;
					out("触发一次获取税盘已上传发票总数查询\n");
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
		//离线上报
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
		//更新上报
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
	out("税盘设备数据组包后压缩前数据大小：%d,压缩后数据大小：%d\n", strlen(tmp_json), json_zlib_len);
	free(tmp_json);
	*g_buf = json_zlib;
	//out("触发一次税盘信息上报,端口号：%d\n",state->port);
	//out("g_buf = %s\n", *g_buf);
	return 0;
}

static int pack_report_other_info_json(struct _port_state   *state, int off_up_flag, char **g_buf)
{
	struct _ty_usb_m  *stream;
	char *tmp_json;
	char tmp[20] = { 0 };
	stream = ty_usb_m_id;	
	//out("触发一次其他USB上报,端口号：%d\n",state->port);
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json, "port_num", tmp);

	if (off_up_flag == 1)
	{
		//离线上报
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
		//更新上报
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
	out("未知设备数据组包后压缩前数据大小：%d,压缩后数据大小：%d\n", strlen(tmp_json), json_zlib_len);
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
		//离线上报
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
		//更新上报
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
	out("CA设备数据组包后压缩前数据大小：%d,压缩后数据大小：%d\n", strlen(tmp_json), json_zlib_len);
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

	cJSON *dir1, *dir2, *dir3;		//组包使用
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
		logout(INFO, "system", "M服务查询同步", "%s,端口号数值异常或盘号异常\r\n", state->port_info.port_str);
		state->need_getinv = 0; //可能税盘已经拔除，清除此处需查询请求
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
		logout(INFO, "SYSTEM", "M服务查询同步", "%s,查询税盘发票同步信息,服务器响应超时\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M服务查询同步", "%s,查询税盘发票同步信息,服务器应答处理失败\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else
	{		
		//out("服务器应答处理查询月发票张数成功,应答反馈数据为%s\n\n\n", g_data);
		result = analysis_get_inv_num_response(state,g_data);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "M服务查询同步", "%s,查询税盘发票同步信息,应答数据解析失败\r\n", state->port_info.port_str);
			free(g_buf);
			cJSON_Delete(json);
			goto End;
		}
		state->need_getinv = 0;

	}
	//sleep(5);
	//out("心跳发送完成\n");
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
			out("获取到的盘号为空,丢弃此数据\n");
			goto End;
		}
		//out("获取到的月份为%s\n", state->port_info.upload_inv_num[i].month);
		if (strcmp(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num) != 0)
		{
			out("获取到的盘号%s,与当前盘号：%s不一致\n", plate_num, state->port_info.plate_infos.plate_basic_info.plate_num);
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
			out("获取dmhmzfsbstr数据失败\n");
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
//定时处理税盘信息线程
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
		//out("终端状态未上传成功不进行发票同步\n");
		return;
	}
	if (stream->frist_up_inv == 0)
	{
		//out("设备刚启动,延时5秒后再读取证书名称\n");
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
		//out("存在多个端口正在读取证书及监控信息,系统中存在税盘类设备读取证书名称或监控信息不程序,未防止冲突,暂不上传离线票");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		//int i;
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;

		if (state->port_info.with_dev == 0)
		{
			//out("端口没有设备不进行发票同步\n");
			continue;
		}
		//out("%d号端口发票上传判断\n", state->port);
		if (state->port_info.upload_inv_num.in_used != 0)
		{
			//out("税盘上传使用中\n");
			continue;
		}		
		if ((sp_type != DEVICE_AISINO) && (sp_type != DEVICE_NISEC) && (sp_type != DEVICE_MENGBAI) && (sp_type != DEVICE_CNTAX))
		{
			//out("税盘类型错误,sp_type = %d\n", sp_type);
			continue;
		}
		if (state->port_info.upload_inv_num.state != 1)
		{
			//out("%d号端口无发票需要上传\n", sp_type);
			continue;
		}
		if (stream->up_queue_num > 9)
		{
			//out("上传队列大于5个,或者正在读取基础信息的队列大于3个,stream->up_queue_num = %d\n", stream->up_queue_num);
			continue;
		}
		state->port_info.upload_inv_num.in_used = 1;
		//out("%d号端口需要执行发票上传\n",state->port);
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
			//out("%03d票种，实际销售金额%4.2Lf,实际销售税额%4.2Lf,期初库存份数为%d\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum->type_sum[sum_cout].sjxsje, now_month_sum->type_sum[sum_cout].sjxsse, now_month_sum->type_sum[sum_cout].qckcfs);
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
	//logout(INFO, "system", "发票上传M服务", "%s,要求同步M服务发票数据,当前线程队列数量：%d\r\n", state->port_info.port_str, stream->up_queue_num);
	if (state->port_info.upload_inv_num.state == 1)
	{
		
		//logout(INFO, "system", "发票上传M服务", "%s,需查询%s月发票统计\n", state->port_info.port_str, state->port_info.upload_inv_num.invoice_month);
		if (port_lock(state, 0, 6, DF_PORT_USED_FPCX_SCMB) != 0)
		{
			//logout(INFO, "system", "发票上传M服务", "%s,同步发票锁定税盘失败,税盘正在执行：%s\n", state->port_info.port_str, state->port_info.used_info);
			
			stream->up_queue_num = stream->up_queue_num - 1;
			sleep(5);
			state->port_info.upload_inv_num.in_used = 0;
			state->port_info.queue_state.use_flag = 2;
			memset(state->port_info.queue_state.end_time,0x00,sizeof(state->port_info.queue_state.end_time));
			timer_read_asc(state->port_info.queue_state.end_time);
			//out("deal_plate_upload_m_server_queue 上传队列数量%d\n", stream->up_queue_num);
			return ;
		}
		logout(INFO, "system", "发票上传M服务", "%d号端口,%s,开始同步M服务发票数据\r\n", state->port,state->port_info.port_str);
		state->up_report = 1;//端口状态更新需要上报
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
		//logout(INFO, "system", "发票上传M服务", "%s,查询数据结束,即将释放端口占用\r\n", state->port_info.port_str);
		port_unlock(state);
		timer_read_asc(end_time);
		

		
		_lock_set(state->lock);
		state->need_getinv = 0;//1即请求当月发票数据   2即请求历史月发票数据（第一次固定为请求当月,请求当月结束后若无插拔动作,则执行历史月发票请求）

		state->port_info.upload_inv_num.state = 0;
		if (state->port_info.upload_inv_num.dmhmzfsbstr != NULL)
		{
			free(state->port_info.upload_inv_num.dmhmzfsbstr);
			state->port_info.upload_inv_num.dmhmzfsbstr = NULL;
		}

		if (result < 0)//非中断方式结束上传
		{
			state->port_info.m_inv_state = -1;
			logout(INFO, "system", "发票上传M服务", "%s,发票数据上传M服务过程中出现异常,即将与服务器同步后重传\r\n", state->port_info.port_str);
			state->need_getinv = 1;
		}
		else
		{
			//out("将数据保存至内存中\n");
			trun_month_sum_to_plate_info(&now_month_sum, &state->port_info.plate_infos);

			state->port_info.m_inv_state = 2;
			logout(INFO, "system", "发票上传M服务", "%s,同步开始时间：%s,同步结束时间：%s\r\n", state->port_info.port_str, start_time, end_time);
		}
		_lock_un(state->lock);
	}
	state->port_info.upload_inv_num.in_used = 0;

	stream->up_queue_num = stream->up_queue_num -1;
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
	{
		logout(INFO, "SYSTEM", "上传M服务", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
		restart_plate_power(state, (uint8 *)state->port_info.sn, state->port, 1);
	}

	//fp_sync_report_json_data((void*)stream,(char*)(&state->port_info.sn),state->port_info.upload_inv_num.summary_month);

	//out("上传结束,上传队列数量%d\n", stream->up_queue_num);
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
	cJSON *head_layer, *data_array, *data_layer;		//组包使用
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
		out("上报发票数据组包完成，长度：%d字节\n", strlen(g_buf));
		//printf("pub data : %s\n", g_buf);
	}
	else
	{
		out("上报汇总数据组包完成，长度：%d字节\n", strlen(g_buf));
		//printf("pub data : %s\n", g_buf);
	}
	char g_data[5000] = { 0 };
	
	result = _m_pub_need_answer(random, dest_topic, g_buf, g_data, DF_MQTT_FPTBXT);

	free(g_buf);
	cJSON_Delete(root);
	if (result < 0)
	{
		if (inv_sum == DF_UPLOAD_INV)
			logout(INFO, "system", "report_invoice", "【上报发票】服务器接收超时失败,即将重试\r\n");
		else
			logout(INFO, "system", "report_invoice", "【上报汇总】服务器接收超时失败,即将重试\r\n");
		state->need_getinv = 1;//已上传发票统计查询
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
	stream->report_p_used = 1;//标识正在使用改队列,需结束在再次启用
	memset(source_topic, 0, sizeof(source_topic));
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		if (state->last_act == 1)//最后动作在线则先操作离线上传
			off_up_flag = 1;
		if (state->last_act == 2)//最后动作离线则先操作在线上传
			off_up_flag = 2;
	}
	else if ((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//操作离线上传
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//操作在线上传
	else
	{
		goto End;
	}
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("zdsbcaxx", random);
	cJSON *dir1, *dir2, *dir3;		//组包使用
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
		//离线上报
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
		//更新上报
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
		out("服务器接收超时失败,增加错误次数及增加重试时长\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		out("服务器应答处理失败,增加错误次数及增加重试时长\n");
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
		out("上传【%d号口：%s】CA信息服务器应答处理成功\n\n\n", state->port, state->port_info.ca_name);
	}
	sleep(5);
	//out("心跳发送完成\n");
	free(g_buf);
	cJSON_Delete(json);

End:
	out("结束ca上报队列1\n");
	stream->report_p_used = 0;
	return;
}
#endif

//定时上报端口信息线程
static void timely_tigger_report_port_status(void *arg, int timer)
{

	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	stream = arg;
	if (stream->report_ter_flag == 0)
	{
	//	out("终端状态未上传成功不进行端口上报\n");
		return;
	}
	
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		if (stream->report_ter_flag == 0)
		{
			//	out("终端状态未上传成功不进行端口上报\n");
			return;
		}
		
		state = &stream->port[port - 1];
		if ((state->off_report == 0) && (state->up_report == 0) && (state->need_getinv == 0))
		{
			continue;
		}

		usleep(5000);
		//out("端口号%d,up_report = %d,off_report = %d,need_getinv = %d,now_usb_app = %d,last_usb_app = %d,stream->report_p_used = %d\n", port, state->up_report, state->off_report, state->need_getinv, state->port_info.usb_app, state->port_last_info.usb_app, stream->report_p_used);

				
		if (((state->up_report == 1) && (state->port_info.usb_app == DEVICE_AISINO || state->port_info.usb_app == DEVICE_NISEC || state->port_info.usb_app == DEVICE_MENGBAI || state->port_info.usb_app == DEVICE_MENGBAI2 || state->port_info.usb_app >= 101)) || \
			((state->off_report == 1) && (state->port_last_info.usb_app == DEVICE_AISINO || state->port_last_info.usb_app == DEVICE_NISEC || state->port_last_info.usb_app == DEVICE_MENGBAI || state->port_last_info.usb_app == DEVICE_MENGBAI2 || state->port_last_info.usb_app >= 101)))
		{
			if (stream->report_p_used != 0)
			{
				//out("端口状态上传中\n");
				continue;
			}
			stream->report_p_used = 1;//标识正在使用改队列,需结束在再次启用
			//out("第%d号端口触发一次税盘更新或离线上报\n", state->port);
			_queue_add("0", pack_report_plate_info, state, 0);
			continue;

		}
		else if ((state->off_report == 1) && (state->port_last_info.usb_app == 0 || state->port_last_info.usb_app == 5))
		{
			if (stream->report_p_used != 0)
			{
				//out("端口状态上传中\n");
				continue;
			}
			stream->report_p_used = 1;//标识正在使用改队列,需结束在再次启用
			//out("触发一次其他USB更新或离线上报\n");
			_queue_add("0", pack_report_other_info, state, 0);
			continue;
		}
#ifdef ALL_SHARE
		else if (state->up_report == 1 || state->off_report == 1)
		{
			if (stream->report_p_used != 0)
			{
				//out("端口状态上传中：%d\n", port);
				continue;
			}
			out("触发一次[%d]端口状态更新或离线上报--\n", port);
			if (stream->port[port - 1].port_info.usb_app == 4)
			{
				//state->up_report = 0;
				stream->report_p_used = 1;
				_queue_add("0", pack_report_ca_info, state, 0);
			}
			else
			{
				stream->report_p_used = 1;
				_queue_add("0", pack_report_other_info, state, 0);//其他usb端口信息上报
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
				//out("端口状态上传中\n");
				continue;
			}
			stream->report_i_used = 1;//标识正在使用改队列,需结束在再次启用
			//out("触发一次获取税盘已上传发票总数查询,状态：%d,1为当前月,2为历史月\n", state->need_getinv);
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
		if (state->last_act == 1)//最后动作在线则先操作离线上传
			off_up_flag = 1;
		if (state->last_act == 2)//最后动作离线则先操作在线上传
			off_up_flag = 2;
	}
	else if((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//操作离线上传
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//操作在线上传
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
		//logout(INFO, "system", "report_plate", "【上报税盘状态】端口号：%d,盘号：%s,服务器接收超时失败,增加错误次数及增加重试时长,重试时长为%d\r\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name, stream->report_time_s);
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
		//out("上传【%d号口：%s】税盘信息服务器应答处理成功\n\n\n", state->port, state->port_info.ca_name);
	}
	
	//out("心跳发送完成\n");
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


	cJSON *dir1, *dir2, *dir3;		//组包使用
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
		//离线上报
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
		//更新上报
		cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(dir3, "offline", "1");
		fun_get_port_data_json(dir3, state->port,1);

	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);
	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	//out("触发一次税盘信息上报,端口号：%d\n",state->port);
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
	//out("触发一次其他USB上报,端口号：%d\n",state->port);
	int off_up_flag = 0;
	if ((state->off_report == 1) && (state->up_report == 1))
	{
		//out("state->last_act = %d\n", state->last_act);
		if (state->last_act == 1)//最后动作在线则先操作离线上传
			off_up_flag = 1;
		if (state->last_act == 2)//最后动作离线则先操作在线上传
			off_up_flag = 2;
	}
	else if ((state->off_report == 1) && (state->up_report == 0))
		off_up_flag = 1;//操作离线上传
	else if ((state->off_report == 0) && (state->up_report == 1))
		off_up_flag = 2;//操作在线上传
	else
	{
		goto End;
	}

	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	get_radom_serial_number("zdsbqtusb", random);

	cJSON *dir1, *dir2, *dir3;		//组包使用
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
		//离线上报
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
		//更新上报
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
		out("服务器接收超时失败,增加错误次数及增加重试时长,重试时长为%d\n", 5);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	//else if (result == 0)
	//{
	//	
	//	out("服务器应答处理失败,增加错误次数及增加重试时长,重试时长为%d\n", stream->report_time_s);
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
		//out("上传【%d号口】其他USB信息服务器应答处理成功\n\n\n", state->port);
	}
	sleep(5);
	//out("心跳发送完成\n");
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

	cJSON *dir1, *dir2, *dir3;		//组包使用
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
		logout(INFO, "system", "M服务查询同步", "%s,端口号数值异常\r\n", state->port_info.port_str);
		cJSON_Delete(json);
		goto End;
	}

	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(dir3, "port_num", tmp);
	//更新上报
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
	cJSON_AddStringToObject(dir3, "usb_app", tmp);
	cJSON_AddStringToObject(dir3, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(dir3, "number", (char *)state->port_info.ca_serial);
	cJSON_AddStringToObject(dir3, "plate_num", (char *)state->port_info.sn);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.extension);
	cJSON_AddStringToObject(dir3, "plate_extension", tmp);
	cJSON_AddStringToObject(dir3, "current_month", "0");//版本修改,后续此字段仅做兼容

	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	//out("g_buf = %s\n", g_buf);
	//_mqtt_client_pub("mb_fpkjxt_kpfw", g_buf);
	char g_data[10000] = { 0 };
	result = _m_pub_need_answer(random, "mb_inv_up_get", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		//out("g_buf = %s\n", g_buf);
		logout(INFO, "SYSTEM", "M服务查询同步", "%s,查询税盘发票同步信息,服务器响应超时\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		state->need_getinv = 0;
		logout(INFO, "SYSTEM", "M服务查询同步", "%s,查询税盘发票同步信息,服务器应答处理失败\r\n", state->port_info.port_str);
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else
	{
		state->need_getinv = 0;
		//out("服务器应答处理查询月发票张数成功,应答反馈数据为%s\n\n\n", g_data);
		analysis_get_inv_num_response(state,g_data);
	}
	//sleep(5);
	//out("心跳发送完成\n");
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
		//out("获取到的月份为%s\n", state->port_info.upload_inv_num[i].month);
		if (strcmp(plate_num, (char *)state->port_info.plate_infos.plate_basic_info.plate_num) != 0)
		{
			out("获取到的盘号%s,与当前盘号：%s不一致\n", plate_num, state->port_info.plate_infos.plate_basic_info.plate_num);
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
		//out("获取到的月份为%s\n", month);

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
				//out("analysis_get_inv_num_response 不支持的发票种类：%03d\n", fpzl);
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
		//out("获取的作废发票总数为%d\n", state->port_info.upload_inv_num[i].cancel_num);
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
//定时处理税盘信息线程
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
		//out("终端状态未上传成功不进行发票同步\n");
		return;
	}
	if (stream->frist_up_inv == 0)
	{
		//out("设备刚启动,延时5秒后再读取证书名称\n");
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
		//out("存在多个端口正在读取证书及监控信息,系统中存在税盘类设备读取证书名称或监控信息不程序,未防止冲突,暂不上传离线票");
		return;
	}
	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		//int i;
		state = &stream->port[port - 1];
		int sp_type = state->port_info.usb_app;

		if (state->port_info.with_dev == 0)
		{
			//out("端口没有设备不进行发票同步\n");
			continue;
		}
		//out("%d号端口发票上传判断\n", state->port);
		if (state->port_info.upload_inv_num.in_used != 0)
		{
			//out("税盘上传使用中\n");
			continue;
		}		
		if ((sp_type != DEVICE_AISINO) && (sp_type != DEVICE_NISEC) && (sp_type != DEVICE_MENGBAI) && (sp_type != DEVICE_CNTAX))
		{
			//out("税盘类型错误,sp_type = %d\n", sp_type);
			continue;
		}
		if ((state->port_info.upload_inv_num.state != 1) && (state->port_info.upload_inv_num.need_fpdmhm_flag != 1))
		{
			//out("%d号端口无发票需要上传\n", sp_type);
			continue;
		}
		if (stream->up_queue_num > 4)
		{
			//out("上传队列大于5个,或者正在读取基础信息的队列大于3个,stream->up_queue_num = %d\n", stream->up_queue_num);
			continue;
		}
		stream->up_queue_num += 1;
		state->port_info.upload_inv_num.in_used = 1;
		//out("%d号端口需要执行发票上传\n",port);
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
			//out("%03d票种，实际销售金额%4.2Lf,实际销售税额%4.2Lf\n", plate_infos->invoice_type_infos[mem_i].fplxdm, now_month_sum->type_sum[sum_cout].sjxsje, now_month_sum->type_sum[sum_cout].sjxsse);
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


	//logout(INFO, "system", "发票上传M服务", "%s,要求同步M服务发票数据,当前线程队列数量：%d\r\n", state->port_info.port_str, stream->up_queue_num);
	if ((state->port_info.upload_inv_num.state == 1) || (state->port_info.upload_inv_num.need_fpdmhm_flag == 1))
	{
		
		//logout(INFO, "system", "发票上传M服务", "%s,需查询%s月发票统计\n", state->port_info.port_str, state->port_info.upload_inv_num.year_month);
		if (port_lock(state, 20, 6, DF_PORT_USED_FPCX_SCMB) != 0)
		{
			//logout(INFO, "system", "发票上传M服务", "%s,同步发票锁定税盘失败,税盘正在执行：%s\n", state->port_info.port_str, state->port_info.used_info);
			state->port_info.upload_inv_num.in_used = 0;
			stream->up_queue_num = stream->up_queue_num - 1;
			sleep(5);
			//out("deal_plate_upload_m_server_queue 上传队列数量%d\n", stream->up_queue_num);
			return ;
		}
		logout(INFO, "system", "发票上传M服务", "%s,开始同步M服务发票数据\r\n", state->port_info.port_str);
		state->up_report = 1;//端口状态更新需要上报
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
		//logout(INFO, "system", "发票上传M服务", "%s,查询数据结束,即将释放端口占用\r\n", state->port_info.port_str);
		port_unlock(state);
		timer_read_asc(end_time);
		

		
		_lock_set(state->lock);
		state->need_getinv = 0;//1即请求当月发票数据   2即请求历史月发票数据（第一次固定为请求当月,请求当月结束后若无插拔动作,则执行历史月发票请求）

		state->port_info.upload_inv_num.state = 0;
		state->port_info.upload_inv_num.need_fpdmhm_flag = 0;
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.upload_inv_type) / sizeof(state->port_info.upload_inv_num.upload_inv_type[0]); i++)
			memset(&state->port_info.upload_inv_num.upload_inv_type[i], 0, sizeof(struct _upload_inv_type));
		for (i = 0; i < sizeof(state->port_info.upload_inv_num.need_upload_dmhm) / sizeof(state->port_info.upload_inv_num.need_upload_dmhm[0]); i++)
			memset(&state->port_info.upload_inv_num.need_upload_dmhm[i], 0, sizeof(struct _need_upload_dmhm));

		if (result < 0)//非中断方式结束上传
		{
			state->port_info.m_inv_state = -1;
			logout(INFO, "system", "发票上传M服务", "%s,发票数据上传M服务过程中出现异常,即将与服务器同步后重传\r\n", state->port_info.port_str);
			state->need_getinv = 1;
		}
		else
		{

			//out("将数据保存至内存中\n");
			trun_month_sum_to_plate_info(&now_month_sum, &state->port_info.plate_infos);

			state->port_info.m_inv_state = 2;
			logout(INFO, "system", "发票上传M服务", "%s,同步开始时间：%s,同步结束时间：%s\r\n", state->port_info.port_str, start_time, end_time);
			state->port_info.upload_inv_num.auto_up_end = 1;
		}
		_lock_un(state->lock);
	}
	state->port_info.upload_inv_num.in_used = 0;

	stream->up_queue_num = stream->up_queue_num -1;
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
	{
		logout(INFO, "SYSTEM", "上传M服务", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
		restart_plate_power(state, (uint8 *)state->port_info.sn, state->port, 1);
	}
	//out("上传结束,上传队列数量%d\n", stream->up_queue_num);
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
	//out("发票JSON complete = %d,nowcount = %d\n", complete, now_count);
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
	//out("更新状态信息\n");
	update_plate_info_cbqkzt(state);
	//out("更新状态信息结束\n");
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
		logout(INFO, "system", "发票上传M服务", "%s,发票数据同步M服务,最新报税日期为空,税盘端口可能发生变化,停止发票同步\r\n", state->port_info.port_str);
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
		logout(ERROR, "system", "发票上传M服务", "%s,发票上传M服务数据组包,month字段异常1\r\n", state->port_info.port_str);
	}

	cJSON *json = cJSON_CreateObject();
	cJSON *head_layer;
	cJSON *data_array;		 //组包使用
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
		cJSON_AddStringToObject(data_layer, "upload_type", "0");//查询上传
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
				logout(ERROR, "system", "发票上传M服务", "%s,发票上传M服务数据组包,month字段异常2\r\n", state->port_info.port_str);
			}
			cJSON_AddStringToObject(data_layer, "upload_type", "0");//查询上传
		
			//cJSON_AddStringToObject(data_layer, "total_num", "0");//此字段已无效

			now_num = s_data[index] * 0x1000000 + s_data[index + 1] * 0x10000 + s_data[index + 2] * 0x100 + s_data[index + 3] * 0x1;
			//out("当前发票序号%d\n",now_num);
			memset(str_tmp, 0, sizeof(str_tmp));
			sprintf(str_tmp, "%d", now_num);
			cJSON_AddStringToObject(data_layer, "now_num", str_tmp);
			if (strlen((char *)month) == 0)
			{
				logout(ERROR, "system", "发票上传M服务", "%s,发票上传M服务数据组包,month字段异常3\r\n", state->port_info.port_str);
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
			//out("当前发票数据长度%d\n", invlen);
			index += 4;
			inv_data = (char *)malloc(invlen + 100);
			memset(inv_data, 0, invlen + 100);
			memcpy(inv_data, s_data + index, invlen);
			index += invlen;
			//out("当前发票数据%s\n", inv_data);
			cJSON_AddStringToObject(data_layer, "invoice_data", inv_data);
			cJSON_AddStringToObject(data_layer, "statistics_data", "");
			cJSON_AddStringToObject(data_layer, "complete_err", "0");
			free(inv_data);

			//out("组发票应答数据\n");
		}
	}

	char *g_buf;
	g_buf = cJSON_Print(json);
	//printf("mqtt  = %s\n", g_buf);
	cJSON_Delete(json);

	if ((strcmp(plate_num, (char *)state->port_info.sn) != 0) || (ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, state->port)<0))
	{
		free(g_buf);
		logout(INFO, "system", "发票上传M服务", "%s,发票数据同步M服务,发现税盘端口发生变化,停止发票同步\r\n", state->port_info.port_str);
		return -1;
	}

	_lock_set(state->lock);//此中断逻辑在此处意义不大,理论上mqtt发送很快，不会占用太长时间，或判断数据量大小，数据量超过100k时执行中断
	if (state->h_plate.hDev != NULL)
	{
		if ((state->h_plate.hDev->bBreakAllIO == 1) && (strlen(g_buf)>100 * 1024))
		{
			_lock_un(state->lock);
			free(g_buf);
			logout(INFO, "system", "发票上传M服务", "%s,发票数据同步M服务,外部要求中断,可能为接口调用或税盘掉线,停止发票同步\r\n", state->port_info.port_str);
			return -1;
		}
	}
	_lock_un(state->lock);
	//out("MQTT回调发送数据\n");
	//char g_data[40960] = {0};
	char g_data[5000] = { 0 };
	//out("触发一次M服务终端基础信息上报\n");
	result = _m_pub_need_answer(random, "mb_inv_month_upload", g_buf, g_data, DF_MQTT_FPKJXT);
	if (result < 0)
	{
		logout(INFO, "system", "发票上传M服务", "%s,发票数据同步M服务,服务器应答超时\r\n", state->port_info.port_str);
		out("发票数据回调发送失败\n");
	}
	//out("MQTT回调发送数据完成,response len = %d\n", strlen(g_data));
	free(g_buf);

	return result;
}
#endif
//定时上报离线票
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
		out("是否禁用上传配置文件不存在\n");
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
	
	//out("获取协议版本\n");
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
	out("金税盘：%d,税控盘：%d,模拟盘：%d\n", *stop_aisino_up, *stop_nisec_up, *stop_mengbai_up);
	cJSON_Delete(root);
	return 0;
}


//等待信号上报离线票
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
		//out("存在多个端口正在读取证书及监控信息,系统中存在税盘类设备读取证书名称或监控信息不程序,未防止冲突,暂不上传离线票");
		return;
	}

	for (port = 1; port <= stream->usb_port_numb; port++)
	{
		state = &stream->port[port - 1];
		//out("第%d号端口有离线票需要上传共%d张\n", port,state->port_info.off_inv_num);
		if (state->port_info.offinv_num_exit <= 0)
			continue;
		if (state->port_info.offinv_stop_upload == 1)
			continue;

		if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 
				"存在离线发票,当前税盘在高版本软件上使用过,存在降版本使用风险,请移至高版本软件使用");
			continue;
		}

		//out("第%d号端口有离线票需要上传 state->port_info.offinv_num_exit = %d\n\n\n", port, state->port_info.offinv_num_exit);
		int sp_type = state->port_info.usb_app;

		if (sp_type == DEVICE_AISINO)
		{
			//out("开始上传金税盘离线发票\n");
			if (stream->stop_aisino_up == 1)
			{
				out("已暂停金税盘发票上传功能\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("当前税盘时间%lld,最后税盘开票时间%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("创建金税盘离线发票上传队列\n");
				_queue_add("0", get_offline_inv_and_upload_aisino, state, 0);
			}
		}
		else if (sp_type == DEVICE_NISEC)
		{
			//out("开始上传税控盘离线发票\n");
			if (stream->stop_nisec_up == 1)
			{
				out("已暂停税控盘发票上传功能\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("当前税盘时间%lld,最后税盘开票时间%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("创建税控盘离线发票上传队列\n");
				_queue_add("0", get_offline_inv_and_upload_nisec, state, 0);
			}
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			//out("开始上传税控盘离线发票\n");
			if (stream->stop_cntax_up == 1)
			{
				out("已暂停税控盘发票上传功能\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("当前税盘时间%lld,最后税盘开票时间%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue;
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				//out("创建税控盘离线发票上传队列\n");
				_queue_add("0", get_offline_inv_and_upload_cntax, state, 0);
			}
		}
		else if (sp_type == DEVICE_MENGBAI)
		{
			if (stream->stop_mengbai_up == 1)
			{
				out("已暂停模拟盘发票上传功能\n");
				continue;
			}
			if (state->port_info.off_up_state == 0)
			{
				unsigned long long now_time;
				now_time = get_time_msec();
				//out("当前税盘时间%lld,最后税盘开票时间%lld\n", now_time, state->port_info.last_kp_time);
				if (now_time - state->port_info.last_kp_time < DF_TY_USB_MQTT_OFFLINE_UP_TIME)
				{
					continue; 
				}
				if (state->port_info.port_used == 1)
				{
					continue;
				}
				out("创建模拟盘离线发票上传队列\n");
				_queue_add("0", get_offline_inv_and_upload_mengbai, state, 0);
			}
		}
		else
		{
			state->port_info.offinv_num_exit = 0;
			out("此USB设备不支持离线发票信息上传\n");
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
			(memcmp(time_s, state->port_info.plate_infos.invoice_type_infos[types_count].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
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


//定时触发税盘同步企业信息
//static void timely_tigger_sync_plate_cpy_info(void *arg, int timer)
//{
//	struct _ty_usb_m  *stream;
//	stream = arg;
//	struct _port_state   *state;
//	int port_count;
//	char errinfo[2048];
//	int result;
//	//out("判断时间，如果时间再夜间的话不进行同步\n");
//	long time_now = get_time_sec();
//	long plate_now;
//	char plate_time_s[20];
//	char hour_min[10];
//
//	out("2小时触发税盘同步企业信息线程\n");
//	for (port_count = 1; port_count <= stream->usb_port_numb; port_count++)
//	{
//		state = &stream->port[port_count - 1];
//		if (state->port_info.with_dev == 0)
//		{
//			//out("端口没有设备不进行企业信息同步\n");
//			continue;
//		}
//		plate_now = time_now + state->port_info.plate_sys_time;
//		memset(plate_time_s, 0, sizeof(plate_time_s));
//		memset(hour_min, 0, sizeof(hour_min));
//		get_date_time_from_second_turn(plate_now, plate_time_s);
//		out("当前税盘：%s,时间为%s\n", state->port_info.port_str, plate_time_s);
//		memcpy(hour_min, plate_time_s+11,5);
//		//out("当前时刻：%s\n", hour_min);
//		if (strcmp(hour_min, "07:00") < 0)
//		{
//			continue;
//		}
//		if (strcmp(hour_min, "20:00") > 0)
//		{
//			continue;
//		}
//		
//		if (state->port_info.usb_app == DEVICE_AISINO)//目前仅支持金税盘
//		{
//			if (port_lock(state, 0, 4, DF_PORT_USED_CSLJ) != 0)
//			{
//				continue;
//			}
//			out("开始同步税盘：%s企业信息,锁定税盘成功\n", state->port_info.plate_infos.plate_basic_info.plate_num);
//			state->up_report = 1;//端口状态更新需要上报
//			state->last_act = 1;
//			memset(errinfo, 0, sizeof(errinfo));
//#ifdef RELEASE_SO
//			result = so_aisino_test_server_connect(&state->h_plate, NULL, errinfo);
//#else
//			result = function_aisino_test_server_connect(&state->h_plate, NULL, errinfo);
//#endif
//			if (result < 0)
//			{
//				out("同步税盘：%s企业信息失败,errinfo %s\n", state->port_info.plate_infos.plate_basic_info.plate_num);
//			}
//			port_unlock(state);
//			state->up_report = 1;//端口状态更新需要上报
//			state->last_act = 1;
//		}
//	}
//}
//定时触发抄报及监控信息回传
static void timely_tigger_auto_cb_jkxxhc(void *arg, int timer)
{
	struct _ty_usb_m  *stream;
	stream = arg;
	struct _port_state   *state;
	int port_count;
	out("15分钟触发抄报清卡线程\n");
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

		//如果存在离线票未上传则触发离线票上传
		if ((state->port_info.plate_infos.plate_tax_info.off_inv_num != 0) && (state->port_info.offinv_num_exit == 0))//防止发票多次上传失败，后续无开票或其他插拔动作不再重传
		{
			out("2小时触发抄报清卡线程,顺带触发离线发票上传\n");
			state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
		}

	}

	sem_post(&stream->cb_qk_sem);
}

//等待信息进行抄报及监控信息回传
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
		//out("已暂停发票上传功能\n");
		//remove("/etc/stop_cb_hcjk");//41版本临时发布
		return;
	}

	sleep(30);
	//out("抄税、汇总及监控回传监控接口已被唤醒\n");
	for (port_count = 1; port_count <= stream->usb_port_numb; port_count++)
	{
		state = &stream->port[port_count - 1];
		if ((state->port_info.need_chaoshui == 0) && (state->port_info.need_huizong == 0) && (state->port_info.need_fanxiejiankong == 0))
		{
			continue;
		}
		int sp_type = state->port_info.usb_app;

		//out("%d号口判断是否需要抄报清卡\n", port_count);
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
			out("%d号口是金税盘,进行抄报清卡动作\n", port_count);			
			aisino_get_cb_data_and_upload(state, time_s, plate_time);
		}
		//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
		else if (sp_type == DEVICE_NISEC)
		{
			out("%d号口是税控盘,进行抄报清卡动作\n", port_count);			
			nisec_get_cb_data_and_upload(state, time_s, plate_time);
			//out("此USB设备为税控盘,暂不支持抄报清卡\n");
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("%d号口是税务ukey,进行抄报清卡动作\n", port_count);			
			cntax_get_cb_data_and_upload(state, time_s, plate_time);
		}

		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("%d号口是模拟盘,进行抄报清卡动作\n", port_count);			
			mengbai_get_cb_data_and_upload(state, time_s, plate_time);			
		}
		timer_read_y_m_d_h_m_s((char *)state->port_info.plate_infos.plate_tax_info.current_report_time);
		timer_read_now_time_add_hour_asc(1, state->port_info.plate_infos.plate_tax_info.next_report_time);

	}
	return;


}


//发票开具成功,更新监控信息数据
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
	out("税盘开票完成，更新税盘状态信息\n");
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;

	int time = 0;
	for (time = 0; time < 3 * 1000; time++)
	{
		usleep(1000);
		if (state->up_report == 0)
		{
			out("税盘状态上报成功\n");
			break;
		}
	}
	if (time == 3 * 1000)
	{
		out("税盘状态上报超时\n");
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


//发票开具成功,更新监控信息数据
static int make_invoice_ok_update_monitor_status(struct _port_state   *state, int read_flag)
{
	//sleep(1);
	//out("要求重新读取监控信息\n");
	//state->port_info.tax_read = 1;
	//return 0;

	//int i;
	//int result;
	//unsigned short busnum;
	//unsigned short devnum;

	if (read_flag == 0)
	{
		//out("因速度问题开票完成不直接读取监控信息,交由线程处理\n");
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
		out("发票上传：负数或作废不纳入离线正数剩余金额计算\n");
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
		out("发票作废：负数作废不纳入离线正数剩余金额计算\n");
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
		//预判断端口是否被占用，占用直接退出
		state->port_info.off_up_state = 0;
		return;
	}
	out("开始上传税盘：%s的离线票,进入发票上传队列\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = {0};
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("金税盘默认口令不正确,不执行离线发票上传操作\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}


	out("开始上传税盘：%s的离线票,即将锁定税盘\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//通过盘号取出发票数据
	for (;;)
	{
		//开始发票上传逻辑
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("税盘正在使用中无法使用,税盘正在执行：%s\n", state->port_info.used_info);
			goto go_end;//线程前部已加判断，此处退出不会循环报送数据
		}
		out("开始上传税盘：%s的离线票,锁定税盘成功\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "离线发票上传", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n",state->port_info.port_str);				
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "证书已挂失或已作废或状态未知") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			
			sleep(3);
			out("第%d号端口离线票上传失败\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "发票上传过程中被其他接口中断") == NULL)
			{
				state->port_info.upfailed_count += 1;//失败次数加1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//测试盘
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//没有离线票
		{
			state->port_info.upfailed_count = 0;//清除离线票上传出错次数
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//离线票数量为0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("第%d号端口税盘中不存在离线发票,不更新监控信息直接上传状态\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//清除离线票上传出错次数
		out("%s,端口发票上传成功及回写成功\n", state->port_info.port_str);

		//out("将此报送成功数据上传至M服务\n");
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
	out("第%d号端口获取端口离线票并上传全部完成\n", state->port);
	state->port_info.off_up_state = 0;
	state->up_report = 1;//端口状态更新需要上报
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
		//预判断端口是否被占用，占用直接退出
		state->port_info.off_up_state = 0;
		return;
	}
	out("开始上传税盘：%s的离线票,进入发票上传队列\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("税控盘默认口令不正确,不执行离线发票上传操作\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("开始上传税盘：%s的离线票,即将锁定税盘\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//通过盘号取出发票数据
	for (;;)
	{
		//开始发票上传逻辑
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("税盘正在使用中无法使用,税盘正在执行：%s\n", state->port_info.used_info);
			goto go_end;//线程前部已加判断，此处退出不会循环报送数据
		}
		out("开始上传税盘：%s的离线票,锁定税盘成功\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "离线发票上传", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n",state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "证书已挂失或已作废或状态未知") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			sleep(3);
			out("第%d号端口离线票上传失败\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "发票上传过程中被其他接口中断") == NULL)
			{
				state->port_info.upfailed_count += 1;//失败次数加1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//测试盘
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//没有离线票
		{
			state->port_info.upfailed_count = 0;//清除离线票上传出错次数
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//离线票数量为0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("第%d号端口税盘中不存在离线发票,不更新监控信息直接上传状态\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//清除离线票上传出错次数
		out("第%d号端口发票上传成功及回写成功\n", state->port);

		out("将此报送成功数据上传至M服务\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);
		inv_change_notice_upload(state, DF_INV_ACTION_SC, (char *)offline_upload_result.fpdm, (char *)offline_upload_result.fphm, (char *)offline_upload_result.scfpsj, (char *)offline_upload_result.kpsj);

		if (offline_upload_result.scfpsj != NULL)
			free(offline_upload_result.scfpsj);



		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("第%d号端口获取端口离线票并上传全部完成\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//端口状态更新需要上报
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
		//预判断端口是否被占用，占用直接退出
		state->port_info.off_up_state = 0;
		return;
	}
	out("开始上传税盘：%s的离线票,进入发票上传队列\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("税务ukey默认口令不正确,不执行离线发票上传操作\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("开始上传税盘：%s的离线票,即将锁定税盘\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//通过盘号取出发票数据
	for (;;)
	{
		//开始发票上传逻辑
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("税盘正在使用中无法使用,税盘正在执行：%s\n", state->port_info.used_info);
			goto go_end;//线程前部已加判断，此处退出不会循环报送数据
		}
		out("开始上传税盘：%s的离线票,锁定税盘成功\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "离线发票上传", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			if (strstr((char *)offline_upload_result.errinfo, "证书已挂失或已作废或状态未知") != NULL)
			{
				state->port_info.offinv_num_exit = 0;
				break;
			}
			sleep(3);
			out("第%d号端口离线票上传失败\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "发票上传过程中被其他接口中断") == NULL)
			{
				state->port_info.upfailed_count += 1;//失败次数加1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//测试盘
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//没有离线票
		{
			state->port_info.upfailed_count = 0;//清除离线票上传出错次数
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//离线票数量为0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("第%d号端口税盘中不存在离线发票,不更新监控信息直接上传状态\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//清除离线票上传出错次数
		out("第%d号端口发票上传成功及回写成功\n", state->port);

		out("将此报送成功数据上传至M服务\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);
		inv_change_notice_upload(state, DF_INV_ACTION_SC, (char *)offline_upload_result.fpdm, (char *)offline_upload_result.fphm, (char *)offline_upload_result.scfpsj, (char *)offline_upload_result.kpsj);

		if (offline_upload_result.scfpsj != NULL)
			free(offline_upload_result.scfpsj);



		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("第%d号端口获取端口离线票并上传全部完成\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//端口状态更新需要上报
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
		//预判断端口是否被占用，占用直接退出
		state->port_info.off_up_state = 0;
		return;
	}
	out("开始上传税盘：%s的离线票,进入发票上传队列\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	char errinfo[1024] = { 0 };
	result = jude_port_auth_passwd(state, errinfo);
	if (result < 0)
	{
		out("===================================================\n");
		out("模拟盘默认口令不正确,不执行离线发票上传操作\n");
		out("===================================================\n");
		state->port_info.offinv_num_exit = 0;
		goto go_end;
	}
	out("开始上传税盘：%s的离线票,即将锁定税盘\n", state->port_info.plate_infos.plate_basic_info.plate_num);
	//通过盘号取出发票数据
	for (;;)
	{
		//开始发票上传逻辑
		if (port_lock(state, 0, 4, DF_PORT_USED_LXSC) != 0)
		{
			out("税盘正在使用中无法使用,税盘正在执行：%s\n", state->port_info.used_info);
			goto go_end;//线程前部已加判断，此处退出不会循环报送数据
		}
		out("开始上传税盘：%s的离线票,锁定税盘成功\n", state->port_info.plate_infos.plate_basic_info.plate_num);
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "离线发票上传", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n",state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return;
			}
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			sleep(3);
			out("第%d号端口离线票上传失败\n", state->port);
			if (strstr((char *)offline_upload_result.errinfo, "发票上传过程中被其他接口中断") == NULL)
			{
				state->port_info.upfailed_count += 1;//失败次数加1
			}
			if (state->port_info.upfailed_count == 10)
			{
				break;
			}

			goto go_end;
		}
		if (result == 99)//测试盘
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, "存在离线发票未上传成功,发票代码：%s,发票号码：%s,开票时间：%s,可能原因：【%s】", offline_upload_result.fpdm, offline_upload_result.fphm, offline_upload_result.kpsj, offline_upload_result.errinfo);
			state->port_info.offinv_num_exit = 0;
			break;
		}
		if (result == 100)//没有离线票
		{
			state->port_info.upfailed_count = 0;//清除离线票上传出错次数
			state->port_info.offinv_num_exit = 0;
			state->port_info.plate_infos.plate_tax_info.off_inv_num = 0;//离线票数量为0

			state->port_info.plate_infos.plate_tax_info.off_inv_dzsyh = 0;
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fpdm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_fphm, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_fphm));
			memset(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj, 0, sizeof(state->port_info.plate_infos.plate_tax_info.off_inv_kpsj));


			out("第%d号端口税盘中不存在离线发票,不更新监控信息直接上传状态\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.offline_inv_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.offline_inv_status));
			break;
		}
		state->port_info.upfailed_count = 0;//清除离线票上传出错次数
		out("第%d号端口发票上传成功及回写成功\n", state->port);

		out("将此报送成功数据上传至M服务\n");
		//update_upload_success_invoice_to_m_server(state, &make_invoice_result);




		//sleep(1);

		//set_plate_invoice_up_success(state, sql_id);
	}
	state->port_info.offinv_num_exit = 0;
go_end:
	out("第%d号端口获取端口离线票并上传全部完成\n", state->port);
	state->port_info.off_up_state = 0;

	state->up_report = 1;//端口状态更新需要上报
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

//金税盘抄报数据上传及监控信息回写
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
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘所在端口未授权,暂不执行抄报清卡");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘证书口令不正确,暂不执行抄报清卡");
		}
		return 0;
	}
	out("【%d号口:%s】可以进行抄报清卡\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("存在除专普电票之外的其他票种，暂不支持自动抄报\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "存在除专普电票之外的其他票种，暂不支持自动抄报");

			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			out("清卡状态【%s】\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;

			return 0;
		}
		if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
			if (zp_pp_ok)  //专普票视为一种,只报送一次
				continue;
			zp_pp_ok = true;
		}

		out("【%d号口】发票类型：%03d,判断是否需要进行抄税,最新报税日期：%s,开票截止日期：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行抄税处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要抄报税\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				logout(INFO, "MQTT", "抄报清卡", "%d端口,金税盘进行抄报税\r\n", state->port);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘正在使用中,无法使用r\n", state->port);
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘占用中,暂时未进行抄报清卡");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘抄税失败,返回值为%d\r\n", state->port,result);
					out("【%d号口】发票类型：%03d,获取抄报数据失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("金税盘汇总、监控反写失败【%s】\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,抄税成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			out("【%d号口】发票类型：%03d,未到抄税时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("【%s】发票类型：%03d,判断是否需要汇总及反写监控信息,税盘当前时间：%s,开票截止时间：%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行汇总及反写处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要执行汇总及反写监控动作\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				//logout(INFO, "MQTT", "抄报清卡", "%d端口,执行汇总及反写监控动作\r\n", state->port);
				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘正在使用中,无法使用\r\n", state->port);
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					//logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘清卡失败,返回值为%d\r\n", state->port,result);
					out("【%d号口】发票类型：%03d,清卡失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("汇总成功,但清卡失败\n");						
						out("金税盘汇总、监控反写失败【%s】\n", errinfo);
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
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,上报汇总及清卡成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("【%d号口】发票类型：%03d,未到监控信息回写时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("【%d号口】需更新内存数据,执行断电操作\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,抄报或清卡完成需重启端口电源\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}


	//logout(INFO, "system", "cb_qk","清卡状态【%s】\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);

	//wang 04 03 
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;

	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================当前端口%d抄报清卡结束\n",state->port+1);
	return 0;

}
//税控盘抄报数据上传及监控信息回写
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
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘所在端口未授权,暂不执行抄报清卡");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘证书口令不正确,暂不执行抄报清卡");
		}

		return 0;
	}
	out("【%d号口:%s】可以进行抄报清卡\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("存在除专普电票之外的其他票种，暂不支持自动抄报\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "存在除专普电票之外的其他票种，暂不支持自动抄报");

			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			out("清卡状态【%s】\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		//if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
		//	if (zp_pp_ok)  //专普票视为一种,只报送一次
		//		continue;
		//	zp_pp_ok = true;
		//}
		out("【%d号口】发票类型：%03d,判断是否需要进行抄税,最新报税日期：%s,开票截止日期：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行抄税处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				logout(INFO, "MQTT", "抄报清卡", "%d端口,税控盘需要抄报税\r\n", state->port);
				out("【%d号口】发票类型：%03d,需要抄报税\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘正在使用中,无法使用\r\n", state->port);
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘占用中,暂时未进行抄报清卡");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘抄报税失败,返回值为%d\r\n", state->port,result);
					out("【%d号口】发票类型：%03d,获取抄报数据失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("税控盘汇总、监控反写失败【%s】\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,抄税成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			out("【%d号口】发票类型：%03d,未到抄税时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("【%s】发票类型：%03d,判断是否需要汇总及反写监控信息,税盘当前时间：%s,开票截止时间：%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行汇总及反写处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				//logout(INFO, "MQTT", "抄报清卡", "%d端口,需要执行汇总及反写监控动作\r\n", state->port);
				out("【%d号口】发票类型：%03d,需要执行汇总及反写监控动作\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);

				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘正在使用中,无法使用\r\n", state->port);
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					//logout(INFO, "MQTT", "抄报清卡", "%d端口,税盘清卡失败,返回值为%d\r\n", state->port,result);
					out("【%d号口】发票类型：%03d,清卡失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("汇总成功,但清卡失败\n");						
						out("税控盘汇总、监控反写失败【%s】\n", errinfo);
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
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,上报汇总及清卡成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("【%d号口】发票类型：%03d,未到监控信息回写时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("【%d号口】需更新内存数据,执行断电操作\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,抄报或清卡完成需重启端口电源\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}

	//logout(INFO, "system", "cb_qk","清卡状态【%s】\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	//wang 04 03 
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;

	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================当前端口%d抄报清卡结束\n",state->port+1);
	return 0;

}

//税务ukey抄报数据上传及监控信息回写
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
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘所在端口未授权,暂不执行抄报清卡");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘证书口令不正确,暂不执行抄报清卡");
		}
		return 0;
	}
	out("【%d号口:%s】可以进行抄报清卡\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];
		if (t_info->state == 0)
			continue;
		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26 && t_info->fplxdm != 28)
		{
			out("存在除专普电票之外的其他票种，暂不支持自动抄报\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "存在除专普电票之外的其他票种，暂不支持自动抄报");

			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			out("清卡状态【%s】\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		out("税盘时间为 time = %s\r\n",plate_time);
		out("【%d号口】发票类型：%03d,判断是否需要进行抄税,最新报税日期：%s,开票截止日期：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行抄税处理\n");
			memset(plate_time_s,0x00,sizeof(plate_time_s));
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要抄报税\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				need_cb_flag++;   //需要抄报标志自增
			}
		}
		else
		{
			out("【%d号口】发票类型：%03d,未到抄税时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("【%s】发票类型：%03d,判断是否需要汇总及反写监控信息,税盘当前时间：%s,开票截止时间：%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行汇总及反写处理\n");
			memset(plate_time_s,0x00,sizeof(plate_time_s));
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要清卡\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				need_qk_flag++;    //需要清卡标志自增
			}
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("【%d号口】发票类型：%03d,未到监控信息回写时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
	}
	if(need_cb_flag > 0)  //进行抄报
	{
		//抄报动作
		logout(INFO, "MQTT", "抄报清卡", "%d端口,税务ukey进行抄报\r\n", state->port);
		if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
		{
			out("【%d号口】准备抄报税,税盘正在使用中,无法使用\n", state->port);
			logout(INFO, "MQTT", "抄报清卡", "%d端口,准备抄报,税盘正在使用中,无法使用\r\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘占用中,暂时未进行抄报清卡");
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
#ifdef RELEASE_SO
		result = so_cntax_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
#else				                                              //发票类型不再需要，随便传个第0个元素
		result = function_cntax_copy_report_data(&state->h_plate, state->port_info.plate_infos.invoice_type_infos[0].fplxdm, errinfo);
#endif				
		port_unlock(state);
		update_tax_business_info(state, DF_PORT_USED_HZSC, REFER_TO_TAX_ADDRESS, result, errinfo);
		if (result < 0 && result != -3)
		{
			if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
			{
				logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return 0;
			}
			logout(INFO, "MQTT", "抄报清卡", "%d端口,抄报错误，错误值为%d\r\n", state->port,result);
			out("【%d号口】获取抄报数据失败\n", state->port);
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			if (strlen(errinfo)<200)
				sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
			out("税务ukey汇总、监控反写失败【%s】\n", errinfo);
			state->port_info.cb_state = -1;
		}
		else
		{
			logout(INFO, "system", "cb_qk", "%s,抄税成功\r\n", state->port_info.port_str);
			result = 0;
			state->port_info.cb_state = 1;
			qk_flag += 1;
		}
	}
	if(need_qk_flag > 0) //进行清卡
	{
		logout(INFO, "MQTT", "抄报清卡", "%d端口,税务ukey进行清卡\r\n", state->port);
		if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
		{
			logout(INFO, "MQTT", "抄报清卡", "%d端口,准备清卡,税盘正在使用中,无法使用\r\n", state->port);
			out("【%d号口】准备清卡,税盘正在使用中,无法使用\n", state->port);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
	#ifdef RELEASE_SO												 //发票类型不再需要，随便传个第0个元素
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
				logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
				restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
				return 0;
			}
			logout(INFO, "MQTT", "抄报清卡", "%d端口,清卡失败，错误值为%d\r\n", state->port,result);
			out("【%d号口】清卡失败\n", state->port);
			if (result == -4)
			{
				//out("汇总成功,但清卡失败\n");						
				out("税务ukey汇总、监控反写失败【%s】\n", errinfo);
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
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
			logout(INFO, "system", "cb_qk", "%s,上报汇总及清卡成功\r\n", state->port_info.port_str);
			result = 0;
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			qk_flag += 1;
		}
	}
	if (qk_flag != 0)
	{
		logout(INFO, "system", "cb_qk", "%s,抄报或清卡完成需重启端口电源\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}

	//logout(INFO, "system", "cb_qk","清卡状态【%s】\n",(char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	//wang 04 03 
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	
	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	return 0;
}

//模拟盘抄报数据上传及监控信息回写
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
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘所在端口未授权,暂不执行抄报清卡");
		}
		else if (result == DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR)
		{
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘证书口令不正确,暂不执行抄报清卡");
		}
		return 0;
	}
	out("【%d号口:%s】可以进行抄报清卡\n", state->port, state->port_info.plate_infos.plate_basic_info.ca_name);
	for (type_count = 0; type_count < DF_MAX_INV_TYPE; type_count++)
	{
		t_info = &state->port_info.plate_infos.invoice_type_infos[type_count];

		if (t_info->state == 0)
			continue;

		if (t_info->fplxdm != 4 && t_info->fplxdm != 7 && t_info->fplxdm != 26)
		{
			out("存在除专普电票之外的其他票种，暂不支持自动抄报\n");
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "存在除专普电票之外的其他票种，暂不支持自动抄报");

			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			out("清卡状态【%s】\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
			state->port_info.need_chaoshui = 0;
			state->port_info.need_huizong = 0;
			state->port_info.need_fanxiejiankong = 0;
			return 0;
		}
		//if (t_info->fplxdm == 4 || t_info->fplxdm == 7) {
		//	if (zp_pp_ok)  //专普票视为一种,只报送一次
		//		continue;
		//	zp_pp_ok = true;
		//}
		
		out("【%d号口】发票类型：%03d,判断是否需要进行抄税,最新报税日期：%s,开票截止日期：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if ((memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.zxbsrq, 6) > 0) && \
			(memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0))//当前月大于最新报税日期月,且要小于开票截止日期,需要报税处理
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行抄税处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要抄报税\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
				if (port_lock(state, 0, 4, DF_PORT_USED_HZSC) != 0)
				{
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "税盘占用中,暂时未进行抄报清卡");
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					out("【%d号口】发票类型：%03d,获取抄报数据失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
					if (strlen(errinfo)<200)
						sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "%s", errinfo);
					out("模拟盘汇总、监控反写失败【%s】\n", errinfo);
					state->port_info.cb_state = -1;
				}
				else
				{
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,抄税成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.cb_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			out("【%d号口】发票类型：%03d,未到抄税时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
			state->port_info.cb_state = 1;
		}
		out("【%s】发票类型：%03d,判断是否需要汇总及反写监控信息,税盘当前时间：%s,开票截止时间：%s\n", state->port_info.plate_infos.plate_basic_info.ca_name, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj);
		if (memcmp(plate_time, state->port_info.plate_infos.invoice_type_infos[type_count].monitor_info.kpjzsj, 6) >= 0)
		{
			//out("判断是在月初第一天的凌晨1点之前,如是则不进行汇总及反写处理\n");
			char plate_time_s[20] = { 0 };
			get_date_time_from_second_turn(plate_time_l, plate_time_s);
			memset(month_start, 0, sizeof(month_start));
			memcpy(month_start, plate_time_s, 11);
			sprintf(month_start + strlen(month_start), "00:05:00");
			if (memcmp(plate_time_s, month_start, 19) > 0)
			{
				out("【%d号口】发票类型：%03d,需要执行汇总及反写监控动作\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);

				if (port_lock(state, 0, 4, DF_PORT_USED_FXJK) != 0)
				{
					out("【%d号口】发票类型：%03d,税盘正在使用中,无法使用\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					return DF_TAX_ERR_CODE_PLATE_IN_USED;
				}
				state->up_report = 1;//端口状态更新需要上报
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
						logout(INFO, "MQTT", "抄报清卡", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
						restart_plate_power(state, (uint8 *)state->port_info.plate_infos.plate_basic_info.plate_num, state->port, 1);
						return 0;
					}
					out("【%d号口】发票类型：%03d,清卡失败\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					if (result == -4)
					{
						//out("汇总成功,但清卡失败\n");						
						out("税控盘汇总、监控反写失败【%s】\n", errinfo);
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
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
					logout(INFO, "system", "cb_qk", "%s,发票类型：%03d,上报汇总及清卡成功\r\n", state->port_info.port_str, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm);
					state->port_info.hz_state = 1;
					state->port_info.qk_state = 1;
					qk_flag += 1;
				}
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		else
		{
			memset((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, 0, sizeof(state->port_info.plate_infos.plate_tax_info.report_tax_status));
			sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功");
			state->port_info.hz_state = 1;
			state->port_info.qk_state = 1;
			out("【%d号口】发票类型：%03d,未到监控信息回写时间,税盘时间为：%s\n", state->port, state->port_info.plate_infos.invoice_type_infos[type_count].fplxdm, plate_time);
		}
		printf("\n\n");

	}
	if (qk_flag != 0)
	{
		//out("【%d号口】需更新内存数据,执行断电操作\n",state->port);
		logout(INFO, "system", "cb_qk", "%s,抄报或清卡完成需重启端口电源\r\n", state->port_info.port_str);
		restart_plate_power(state, state->port_info.sn, state->port, 1);
		qk_flag = 0;
	}
	out("清卡状态【%s】\n", (char *)state->port_info.plate_infos.plate_tax_info.report_tax_status);
	state->port_info.need_chaoshui = 0;
	state->port_info.need_huizong = 0;
	state->port_info.need_fanxiejiankong = 0;
	//out("\n\n\n==================================================================================当前端口%d抄报清卡结束\n",state->port+1);
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
	unsigned int nBytes = 0;  // GBK可用1-2个字节编码,中文两个
	// ,英文一个
	uint8 chr = *str;
	int bAllAscii = 1;  //如果全部都是ASCII,
	int i = 0;
	for (i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
		if ((chr & 0x80) != 0 && nBytes == 0) {  // 判断是否ASCII编码,如果不是,说明有可能是GBK
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
	if (nBytes != 0) {  //违返规则
		return 0;
	}
	if (bAllAscii) {  //如果全部都是ASCII,
		//也是GBK
		return 1;
	}
	return 1;
}

//释放发票信息(包括商品明细)结构体
int InvFreeFpxx(HFPXX stp_Root)
{
	if (stp_Root == NULL)
		return -1;
	//循环释放每张发票的明细
	HFPXX stp_FpxxNode = stp_Root;
	if (!stp_FpxxNode)
		return -2;
	struct Spxx *stp_MxxxNode = stp_FpxxNode->stp_MxxxHead, *stp_TmpSpxxNode = NULL;
	//释放商品明细
	while (stp_MxxxNode) {
		stp_TmpSpxxNode = stp_MxxxNode->stp_next;
		free(stp_MxxxNode);
		stp_MxxxNode = stp_TmpSpxxNode;
	}
	//如果包含了原始发票信息，则释放
	if (stp_FpxxNode->pRawFPBin)
		free(stp_FpxxNode->pRawFPBin);
	//释放商品明细头
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
		out("税盘时间误差大于12小时\n");
		out("税盘时间差 %d\n", state->port_info.plate_sys_time);
		if (state->port_info.plate_infos.plate_basic_info.plate_test != 1)
		{
			out("税盘时间误差太大不允许开票\n");
			sprintf(errinfo, "税盘时间误差太大不允许开票");
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
	}	

	if (state->port_info.plate_infos.plate_basic_info.use_downgrade_version == 1)
	{		
		sprintf(errinfo, "当前税盘在高版本软件上使用过,存在降版本使用风险,请移至高版本软件使用");
		out("%s\n",errinfo);
		return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	}
	
	for (i = 0; i < DF_MAX_INV_TYPE; i++)
	{
		//out("内存%d发票类型代码：%d,inv_type = %d\n", i, state->port_info.tax_infos[i].fplxdm, inv_type);
		if (state->port_info.plate_infos.invoice_type_infos[i].fplxdm != inv_type)
		{
			continue;
		}
		//out("找到对应的发票类型 %03d\n", inv_type);
		//out("判断是否超过开票截止日期%s\n", state->port_info.plate_infos.invoice_type_infos[i].monitor_info.kpjzsj);
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

		sprintf(kpjzsj + strlen(kpjzsj), " 23:50:00");//原时间为00：00：00现改为23：50：00，放宽23小时
		//out("开票截止日期为：%s\n",kpjzsj);

		//out("获取当前时间\n");
		memset(time_now, 0, sizeof(time_now));
		timer_read_y_m_d_h_m_s(time_now);
		//out("当前时间：%s\n", time_now);
		//out("判断当前时间是否合法\n");
		if (memcmp(time_now, SYSTEM_LEAST_TIME, strlen(time_now)) <= 0)
		{
			out("当前系统时间错误,无法开票\n");
			sprintf(errinfo,"当前系统时间错误,无法开票");
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
		if (memcmp(time_now, kpjzsj, strlen(time_now)) >= 0)
		{
			out("超过开票截止日期,无法开票,time_now = %s,kpjzsj = %s\n", time_now, kpjzsj);
			sprintf(errinfo, "超过开票截止日期,无法开票,time_now = %s,kpjzsj = %s", time_now, kpjzsj);
			return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
		}
		if ((state->port_info.usb_app == DEVICE_NISEC) || (state->port_info.usb_app == DEVICE_CNTAX))
		{
			if (state->port_info.offinv_num_exit != 0)
			{
				if (state->port_info.plate_infos.invoice_type_infos[i].monitor_info.lxkpsc == 0)
				{
					sprintf(errinfo, "税盘离线时长为0时不允许开具多张离线发票");
					out("%s\n",errinfo);
					return DF_TAX_ERR_CODE_EXCEED_OFFLINE_LIMIT;
				}
			}
		}

		if (state->port_info.plate_infos.invoice_type_infos[i].invoice_coils_info.wsyfpjsl == 0)
		{
			out("无可用发票卷,wsyfpjsl = 0,i = %d\n", i);
			sprintf(errinfo, "无可用发票卷");
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
			out("无可用发票卷,剩余发票数量为0\n");
			sprintf(errinfo, "无可用发票卷,剩余发票数量为0");
			return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
		}
		//out("找到有可用发票\n");
		break;
	}
	if (i == DF_MAX_INV_TYPE)
	{
		out("发票类型不支持\n");
		sprintf(errinfo, "发票类型不支持");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}

	str_replace(inv_data, "\\", "[@*br/*@]");//排查json中的\转义字符

	result = InvCheckInvoiceStr((unsigned char *)inv_data);
	if (result < 0)
	{
		sprintf(errinfo, "发票JSON数据中存在特殊字符无法解析,例如&和\\");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;
	}

	result = InvDataIsGBK(inv_data);
	if (result != 1)
	{
		sprintf(errinfo, "发票JSON数据汉字编码格式错误,非GBK编码");
		return DF_TAX_ERR_CODE_USE_ILL_CHAR_OR_UTF8;//编码格式错误
	}

	//out("\n开始校验发票数据\n");

	struct Fpxx *stp_fpxx = NULL;
	stp_fpxx = malloc(sizeof(struct Fpxx));
	if (stp_fpxx == NULL)
	{
		printf("stp_fpxx malloc Err\n");
		sprintf(errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
		return DF_TAX_ERR_CODE_SYSTEM_MEM_INSUFFICIENT;
	}
	memset(stp_fpxx, 0, sizeof(struct Fpxx));
	stp_fpxx->hDev = state->h_plate.hDev;
	stp_fpxx->stp_MxxxHead = malloc(sizeof(struct Spxx));
	if (stp_fpxx->stp_MxxxHead == NULL)
	{
		printf("stp_fpxx->stp_MxxxHead malloc Err\n");
		sprintf(errinfo, "系统内存申请失败,机柜可能存在故障,请重启机柜");
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
		logout(INFO, "MQTT", "发票开具", "发票解析失败,错误代码%d\r\n", result);
		InvFreeFpxx(stp_fpxx);
		return DF_TAX_ERR_CODE_INV_DATA_ANALYSIS_ERR;
	}
	InvFreeFpxx(stp_fpxx);
	//out("\n校验发票数据结束\n");
	return 0;
}





//获取税盘所在端口号
int fun_get_plate_usb_port(unsigned char *plate_num)
{
	
	struct _ty_usb_m  *stream;
	int port;
	stream = ty_usb_m_id;
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("该盘号未在当前内存记录中找到\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	return port;
}

//获取税盘当前时间
int fun_get_plate_now_time(unsigned char *plate_num, char *time)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	
	out("通过盘号查找所在的机柜USB端口号\n");
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("该盘号未在当前内存记录中找到\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 3, DF_PORT_USED_HQSPDQSJ) != 0)
	{
		out("税盘正在使用中无法使用\n");
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取税盘当前时间\n");
#ifdef RELEASE_SO
		result = so_aisino_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_aisino_get_tax_time(&state->h_plate, (char *)time);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取税盘当前时间\n");
#ifdef RELEASE_SO
		result = so_nisec_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_nisec_get_tax_time(&state->h_plate, (char *)time);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY获取税盘当前时间\n");
#ifdef RELEASE_SO
		result = so_cntax_get_tax_time(&state->h_plate, (char *)time);
#else
		result = function_cntax_get_tax_time(&state->h_plate, (char *)time);
#endif		
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("蒙柏盘获取税盘当前时间\n");
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
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取当前税盘时间", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
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


	cJSON *inv_array, *inv_layer;		//组包使用
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "inv_data", inv_array = cJSON_CreateArray());

	cJSON_AddItemToObject(inv_array, "dira", inv_layer = cJSON_CreateObject());

	uint16 crc = crc_8005((uint8 *)scfpsj, strlen(scfpsj), 0);
	char invoice_crc[10] = {0};
	sprintf(invoice_crc,"%04x",crc);
	//out("要上传的发票数据,crc 为：%s\n", invoice_crc);

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

	out("整体组包后压缩前数据大小：%d,压缩后数据大小：%d\n", strlen(tmp_json), json_zlib_len);

	free(tmp_json);


	char source_topic[50] = { 0 };
	char random[50] = { 0 };
	cJSON *head_layer, *data_array, *data_layer;		//组包使用
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
	//out("上报发票发送数据：%s\n", g_buf);
	char g_data[5000] = { 0 };
	result = _m_pub_need_answer(random, "mb_invoice_data_upload", g_buf, g_data, DF_MQTT_FPKJXT);
	free(g_buf);
	cJSON_Delete(root);
	if (result < 0)
	{
		logout(INFO, "system", "report_invoice", "【上报发票】服务器接收超时失败,即将重试\r\n");
		state->need_getinv = 1;//已上传发票统计查询
		return 0;
	}
 	return 0;
}

static int inv_change_notice_upload(struct _port_state   *state,int act_type, char *fpdm, char *fphm,char *scfpsj,char *kpsj)
{
	
	switch (act_type)
	{
	case DF_INV_ACTION_KP:
		logout(INFO, "system", "发票上传M服务", "%s,发票开具动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);//开票结果自带
		break;
	case DF_INV_ACTION_ZF:
		logout(INFO, "system", "发票上传M服务", "%s,发票作废动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);//作废结果自带
		break;
	case DF_INV_ACTION_SC:
		logout(INFO, "system", "发票上传M服务", "%s,发票上传动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	default:
		//out("发票动作类型错误\n");
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
	out("发票开具、作废和离线上传后直接同步发票数据,开票时间：%s\n", kpsj);
	send_data = malloc(max_len); //发票数据缓冲区1M
	memset(send_data, 0, max_len);
	send_data[0] = (((1) >> 24) & 0xff); //发票序号
	send_data[1] = (((1) >> 16) & 0xff);
	send_data[2] = (((1) >> 8) & 0xff);
	send_data[3] = (((1) >> 0) & 0xff);
	if (scfpsj != NULL)
		inv_len = strlen((const char *)scfpsj);
	send_data[4] = ((inv_len >> 24) & 0xff); send_data[5] = ((inv_len >> 16) & 0xff);//发票长度
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
		logout(INFO, "system", "发票上传M服务", "%s,发票开具动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	case DF_INV_ACTION_ZF:
		logout(INFO, "system", "发票上传M服务", "%s,发票作废动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	case DF_INV_ACTION_SC:
		logout(INFO, "system", "发票上传M服务", "%s,发票上传动作同步单张发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
		break;
	default:
		//out("发票动作类型错误\n");
		return -1;
	}
	
	if (state->port_info.upload_inv_num.auto_up_end != 1)
	{
		logout(INFO, "system", "发票上传M服务", "%s,直接由系统自动上传处理,无需单独上传该发票,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
		return 0;
	}
	if (state->port_info.usb_app == 3)//模拟盘仅支持整月查询
	{
		state->need_getinv = 1;
		return 0;
	}
	if (strlen(fpdm) == 0 || (strlen(fphm) == 0))
	{
		logout(INFO, "system", "发票上传M服务", "%s,需要上传单张发票,但传入的发票代码号码长度有误,直接触发整月上传,发票代码：%s 发票号码：%s\r\n", state->port_info.port_str, fpdm, fphm);
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
				out("此发票代码:%s号码:%s已在缓存队列\n", fpdm, fphm);
				return 0;
			}
		}
		out("此发票代码:%s号码:%s不在缓存队列\n", fpdm, fphm);
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
			out("缓存队列已满,无法存入,直接触发整月上传\n");
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
			out("缓存队列可能已经乱序,不能存入改数据,直接触发整月上传\n");
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
		out("发票开具、上传或作废需上传至M服务\n");
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
		if ((strcmp((char *)plate_num, (char *)state->port_info.sn) == 0) && (state->port_info.tax_read == 0))//此处判断读取已经完成
		{
			if (state->up_report == 0)//此处判断上报已经完成
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
	logout(INFO, "SYSTEM", "重启端口电源", "%s,即将重启%d号端口电源,盘号：%s\r\n", state->port_info.port_str, port, plate_num);
//	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
//	if (result < 0)
//	{
//		sprintf(errinfo, "要求单片机关闭%d号端口电源失败,result = %d", port, result);
//		logout(INFO, "SYSTEM", "重启端口电源", "%s\r\n", errinfo);
//#ifdef RELEASE_SO
//		so_common_report_event((char *)plate_num, "单片机端口关电失败", errinfo, -1);
//#else
//		function_common_report_event((char *)plate_num, "单片机端口关电失败", errinfo, -1);
//#endif	
//		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
//		if (result < 0)
//		{
//			memset(errinfo,0,sizeof(errinfo));
//			sprintf(errinfo, "要求单片机重试关闭%d号端口电源失败,result = %d", port, result);
//			logout(INFO, "SYSTEM", "重启端口电源", "%s\r\n", errinfo);
//#ifdef RELEASE_SO
//			so_common_report_event((char *)plate_num, "单片机端口重试关电失败", errinfo, -1);
//#else
//			function_common_report_event((char *)plate_num, "单片机端口重试关电失败", errinfo, -1);
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
		logout(INFO, "SYSTEM", "重启端口电源", "重启%d号端口电源后,税盘缓存未清除,盘号：%s,busid : %s,used_info:%s\r\n", port, plate_num,busid,state->port_info.used_info);

		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "重启端口电源", "重启%d号端口电源后,USB的busid:%s已在文件系统消失,盘号：%s,可能为线程检测故障\r\n", port, busid, plate_num);
		}
		else
		{
			sprintf(errinfo,"重启%d号端口电源后,USB的busid:%s未在文件系统消失,盘号：%s,可能为USB文件系统故障或未正常关电", port, busid, plate_num);
			logout(INFO, "SYSTEM", "重启端口电源", "%s\r\n", errinfo);
#ifdef RELEASE_SO
			so_common_report_event((char *)plate_num, "硬件端口关电失败", errinfo, -1);
#else
			function_common_report_event((char *)plate_num, "硬件端口关电失败", errinfo, -1);
#endif	
			state->h_plate.nBusID = nBusID;
			state->h_plate.nDevID = nDevID;
			state->power = 1; // 关电未成功，把电源标志至为有电
			//state->port_info.tax_read = 1;
			waiting_for_plate_install((uint8 *)plate_num, state, 10);
			return 0;

		}
	}

	//out("税盘已被卸载\n");
	result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
	if (result < 0)
	{
		sprintf(errinfo, "要求单片机打开%d号端口电源失败,result = %d", port, result);
		logout(INFO, "SYSTEM", "重启端口电源", "%s\r\n", errinfo);
#ifdef RELEASE_SO
		so_common_report_event((char *)plate_num, "单片机端口开电失败", errinfo, -1);
#else
		function_common_report_event((char *)plate_num, "单片机端口开电失败", errinfo, -1);
#endif	
		result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
		if (result < 0)
		{
			sprintf(errinfo, "要求单片机重试打开%d号端口电源失败,result = %d", port, result);
			logout(INFO, "SYSTEM", "重启端口电源", "%s\r\n", errinfo);
#ifdef RELEASE_SO
			so_common_report_event((char *)plate_num, "单片机端口重试开电失败", errinfo, -1);
#else
			function_common_report_event((char *)plate_num, "单片机端口重试开电失败", errinfo, -1);
#endif	
		}

	}
	state->power = 1;

	if (wait_install == 1)
	{
		result = waiting_for_plate_install((uint8 *)plate_num, state, 10);
		if (result < 0)
		{
			logout(INFO, "SYSTEM", "重启端口电源", "重启%d号端口电源后,USB的未正常重新加载,盘号：%s\r\n", port, plate_num);
		}
	}
	//out("税盘已加载成功\n");
	return 0;
}

static int plate_useinfo_errinfo(struct _port_state   *state, char *errinfo)
{
	char used_now[1024] = { 0 };
	if (strlen((char *)state->port_info.sn) == 0)
	{
		sprintf(errinfo, "操作过程中税盘离线请重试");
		return 0;
	}
	if (strcmp(state->port_info.used_info, DF_PORT_USED_FPKJ) == 0)
	{
		sprintf(used_now, "正在开具其他发票");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_USB_SHARE) == 0)
	{
		sprintf(used_now, "正在使用usb-share加载功能");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USBD_QD_LOCK) == 0)
	{
		sprintf(used_now, "正在使用全电功能");
	}	
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)
	{
		sprintf(used_now, "正在上传离线发票");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_FPCX_SCMB) == 0)
	{
		sprintf(used_now, "正在同步M服务发票数据");
	}
	else if (strcmp(state->port_info.used_info, DF_PORT_USED_AYCXFPSJMQTT) == 0)
	{
		sprintf(used_now, "正在按月查询发票数据");
	}
	else
	{
		logout(INFO, "SYSTEM", "SYS", "正在执行其他占用税盘操作,%s\r\n", (char *)state->port_info.used_info);
		sprintf(used_now, "正在执行其他占用税盘操作");
	}
	sprintf(errinfo, "税盘正在使用中,当前%s", used_now);
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
	//out("发票数据[%s]", inv_json);
	cJSON *item;
	cJSON *root = cJSON_Parse(inv_json);
	if (!root)
	{
		//out("发票数据解析失败[%s]", inv_json);
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
		//out("期末库存份数减1\n");
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
			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
		else
			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
	return 0;
}

//发票开具
int fun_make_invoice_to_plate(unsigned char *plate_num, int inv_type, unsigned char *inv_data, struct _make_invoice_result *make_invoice_result)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;	
	int port;
	int result;
	struct _plate_infos plate_infos;
	memset(&plate_infos, 0, sizeof(struct _plate_infos));
	stream = ty_usb_m_id;

	
	//out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)make_invoice_result->errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
	//out("判断是税盘是否能允许开票\n");
	result = judge_plate_allow_kp(state, inv_type, (char *)inv_data,(char *) make_invoice_result->errinfo);
	if (result < 0)
	{
		return result;
	}

	//logout(INFO, "MQTT", "FPKJ", "%s,发票开具税盘状态及发票数据预校验通过\r\n", state->port_info.port_str);
	int sp_type = state->port_info.usb_app;
	//out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (sp_type == DEVICE_AISINO)
	{
		if ((result = port_lock(state, 40, 3, DF_PORT_USED_FPKJ)) != 0)
		{
			if (state->power_mode == Saving_Mode)
			{
				if (result == POWER_ERR_NO_DEV)
				{
					out("税盘正在使用中无法开票\n");
					sprintf((char *)make_invoice_result->errinfo, "该盘号：%s上电失败,请重试！", plate_num);
				}
				else if (result == POWER_ERR_FULL_LOAD)
				{
					out("超过满载上限\n");
					sprintf((char *)make_invoice_result->errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
				}
				else
				{
					out("税盘正在使用中无法开票\n");
					plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
				}
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			out("税盘正在使用中无法开票\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "FPKJ", "%s,发票开具底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port,1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税盘正在使用中无法使用\n");
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
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("金税盘发票开具失败\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
				out("金税盘发票开具失败\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//已在开具结果中处理发票同步,此处可取消
	}
	else if (sp_type == DEVICE_NISEC)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("税盘正在使用中无法开票\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "FPKJ", "%s,发票开具底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税盘正在使用中无法使用\n");
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
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税控盘发票开具失败\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
				out("税控盘发票开具失败\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//已在开具结果中处理发票同步,此处可取消
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("税盘正在使用中无法开票\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "FPKJ", "%s,发票开具底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税盘正在使用中无法使用\n");
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
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税务ukey发票开具失败\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
				out("税务ukey发票开具失败\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
			restart_plate_power(state, plate_num, port, 1);
		}
		//已在开具结果中处理发票同步,此处可取消
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		if (port_lock(state, 60, 3, DF_PORT_USED_FPKJ) != 0)
		{
			out("税盘正在使用中无法开票\n");
			plate_useinfo_errinfo(state, (char *)make_invoice_result->errinfo);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		state->up_report = 1;//端口状态更新需要上报
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
				logout(INFO, "MQTT", "FPKJ", "%s,发票开具底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
				if (port_lock(state, 20, 3, DF_PORT_USED_FPKJ) != 0)
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("税盘正在使用中无法使用\n");
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
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					out("模拟盘发票开具失败\n");
					return result;
				}
			}
			else
			{
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;
				out("模拟盘发票开具失败\n");
				return result;
			}
		}
		make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
		kj_zf_update_sjxsje_se(state, make_invoice_result->scfpsj);
		state->port_info.offinv_num_exit += 1;
		if (make_invoice_result->need_restart == 1)
		{
			logout(INFO, "MQTT", "FPKJ", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)cancel_invoice_result->errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)cancel_invoice_result->errinfo, "当前税盘在高版本软件上使用过,存在降版本使用风险,请移至高版本软件使用");
		out("%s\n", (char *)cancel_invoice_result->errinfo);
		return DF_TAX_ERR_CODE_EXCEED_KP_END_DATE;
	}
	
	if (zflx == 1)
	{
		out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);

		//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
		if (sp_type == DEVICE_AISINO)
		{
			out("金税盘发票作废\n");
			if ((result = port_lock(state, 20, 3, DF_PORT_USED_FPZF)) != 0)
			{
				if (state->power_mode == Saving_Mode)
				{
					if (result == POWER_ERR_NO_DEV)
					{
						out("税盘正在使用中无法开票\n");
						sprintf((char *)cancel_invoice_result->errinfo, "该盘号：%s上电失败,请重试！", plate_num);
						return DF_TAX_ERR_CODE_PLATE_IN_USED;
					}
					else if (result == POWER_ERR_FULL_LOAD)
					{
						out("超过满载上限\n");
						sprintf((char *)cancel_invoice_result->errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
					}
				}
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
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
					logout(INFO, "MQTT", "FPKJ", "%s,发票作废底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
					out("作废失败提示底层错误则断电后重试\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("发票作废失败\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("发票作废失败\n");
					return result;
				}

			}
			out("发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			aisino_inv_cancel_complete_update_lxzssyje(state, cancel_invoice_result);
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			return 0;
		}
		else if (sp_type == DEVICE_NISEC)
		{
			out("百旺盘发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
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
					logout(INFO, "MQTT", "FPKJ", "%s,发票作废底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
					out("作废失败提示底层错误则断电后重试\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("发票作废失败\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("发票作废失败\n");
					return result;
				}

			}
			out("发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			return 0;
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("税务UKEY发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
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
					logout(INFO, "MQTT", "FPKJ", "%s,发票作废底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
					out("作废失败提示底层错误则断电后重试\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("发票作废失败\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("发票作废失败\n");
					return result;
				}

			}
			out("发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, (char *)cancel_invoice_result->scfpsj, (char *)cancel_invoice_result->kpsj);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			return 0;
		}
		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("蒙柏盘发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
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
					logout(INFO, "MQTT", "FPKJ", "%s,发票作废底层失败,重启端口电源后在重试一次\r\n", state->port_info.port_str);
					out("作废失败提示底层错误则断电后重试\n", port);
					restart_plate_power(state, plate_num, port, 1);
					if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
					{
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
						state->up_report = 1;//端口状态更新需要上报
						state->last_act = 1;
						result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
						out("发票作废失败\n");
						return result;
					}
				}
				else
				{
					state->up_report = 1;//端口状态更新需要上报
					state->last_act = 1;
					result = DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
					out("发票作废失败\n");
					return result;
				}

			}
			out("发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
#else
			inv_change_notice_upload(state, DF_INV_ACTION_ZF, (char *)cancel_invoice_result->fpdm, (char *)cancel_invoice_result->fphm, NULL, NULL);
#endif
			kj_zf_update_sjxsje_se(state, cancel_invoice_result->scfpsj);
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			return 0;
		}
	}
	else if (zflx == 0)
	{
		out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
		//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
		if (sp_type == DEVICE_AISINO)
		{
			out("金税盘发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 空白发票作废最后一张应用层修改 
			//wang  03 31 轮寻查找所有票种的发票起始号码是否等于作废号码
			//相同后比较作废张数和剩余份数，相同则作废成功后重启端口
			logout(INFO, "MQTT", "FPZF", "发票种类=%d,发票作废号码=%s,发票作废号码=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 轮询查找对应作废的发票号码里面的发票卷剩余数量
				//判断发票卷剩余数据是否等于作废数量
				//相等就重启端口
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "AISINO空白发票作废发票号码%d,当前发票剩余张数=%d,作废张数=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "AISINO空白发票作废剩余发票卷%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
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
				out("发票作废失败\n");
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;

				//作废失败也要判断是否需要重启
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}
				return result;
			}
			logout(INFO,"MQTT", "FPZF", "空白发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//清除上次查询时间，立即查询并同步发票
#endif
			state->need_getinv = 1;
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}

		//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
		else if (sp_type == DEVICE_NISEC)
		{
			out("税控盘发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 空白发票作废最后一张应用层修改 
			//wang  03 31 轮寻查找所有票种的发票起始号码是否等于作废号码
			//相同后比较作废张数和剩余份数，相同则作废成功后重启端口
			logout(INFO, "MQTT", "FPZF", "发票种类=%d,发票作废号码=%s,发票作废号码=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 轮询查找对应作废的发票号码里面的发票卷剩余数量
				//判断发票卷剩余数据是否等于作废数量
				//相等就重启端口
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "NISEC空白发票作废发票号码%d,当前发票剩余张数=%d,作废张数=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "NISEC空白发票作废剩余发票卷%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
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
				out("发票作废失败\n");
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;

				//作废失败也要判断是否需要重启
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "空白发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//清除上次查询时间，立即查询并同步发票
#endif
			state->need_getinv = 1;
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
		else if (sp_type == DEVICE_CNTAX)
		{
			out("税务ukey发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 空白发票作废最后一张应用层修改 
			//wang  03 31 轮寻查找所有票种的发票起始号码是否等于作废号码
			//相同后比较作废张数和剩余份数，相同则作废成功后重启端口
			logout(INFO, "MQTT", "FPZF", "发票种类=%d,发票作废号码=%s,发票作废号码=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 轮询查找对应作废的发票号码里面的发票卷剩余数量
				//判断发票卷剩余数据是否等于作废数量
				//相等就重启端口
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "CNTAX空白发票作废发票号码%d,当前发票剩余张数=%d,作废张数=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "CNTAX空白发票作废剩余发票卷%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
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
				out("发票作废失败\n");
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;

				//作废失败也要判断是否需要重启
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "空白发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);	
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//清除上次查询时间，立即查询并同步发票
#endif
			state->need_getinv = 1;
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
		//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
		else if (sp_type == DEVICE_MENGBAI)
		{
			out("蒙柏盘发票作废\n");
			if (port_lock(state, 20, 3, DF_PORT_USED_FPZF) != 0)
			{
				sprintf((char *)cancel_invoice_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
				out("%s\n", cancel_invoice_result->errinfo);
				return DF_TAX_ERR_CODE_PLATE_IN_USED;
			}
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;

			fpzf_fphm = atoi((const char *)cancel_invoice_result->fphm);
			//wang  02 24 空白发票作废最后一张应用层修改 
			//wang  03 31 轮寻查找所有票种的发票起始号码是否等于作废号码
			//相同后比较作废张数和剩余份数，相同则作废成功后重启端口
			logout(INFO, "MQTT", "FPZF", "发票种类=%d,发票作废号码=%s,发票作废号码=%d\r\n",cancel_invoice_result->fplxdm,cancel_invoice_result->fphm,fpzf_fphm);
			#ifdef RELEASE_SO
				so_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#else
				function_common_find_inv_type_mem(cancel_invoice_result->fplxdm, &mem_i);
			#endif
			for(z = 0;z < DF_MAX_INV_COILS;z++)	
			{
				//wang 02 13 轮询查找对应作废的发票号码里面的发票卷剩余数量
				//判断发票卷剩余数据是否等于作废数量
				//相等就重启端口
				if(plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpqshm == fpzf_fphm)
				{
					logout(INFO, "MQTT", "FPZF", "MENGBAI空白发票作废发票号码%d,当前发票剩余张数=%d,作废张数=%d\r\n",fpzf_fphm,plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs,cancel_invoice_result->zfzs);
					if(cancel_invoice_result->zfzs == plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs)
					{
						logout(INFO, "MQTT", "FPZF", "MENGBAI空白发票作废剩余发票卷%d\r\n",plate_infos.invoice_type_infos[mem_i].invoice_coils_info.invoice_coil[z].fpsyfs);
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
				out("发票作废失败\n");
				state->up_report = 1;//端口状态更新需要上报
				state->last_act = 1;

				//作废失败也要判断是否需要重启
				if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
				{
					logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
					restart_plate_power(state, plate_num, port, 1);
				}

				return result;
			}
			logout(INFO,"MQTT", "FPZF", "空白发票作废成功,发票代码：%s,发票号码：%s\n", cancel_invoice_result->fpdm, cancel_invoice_result->fphm);
			
			state->port_info.offinv_num_exit += 1;
			state->port_info.last_kp_time = get_time_msec();
#ifndef DF_OLD_MSERVER
			state->last_report_time_t = 0;//清除上次查询时间，立即查询并同步发票
#endif
			state->need_getinv = 1;
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
			make_invoice_ok_update_monitor_status_and_notice_server(state, &plate_infos);
			if ((cancel_invoice_result->need_restart == 1)||(fpzf_need_restart == 1))
			{
				logout(INFO, "MQTT", "FPZF", "%s,发票卷最后一张发票开具完成重启端口电源\r\n", state->port_info.port_str);
				restart_plate_power(state, plate_num, port, 1);
			}
			return 0;
		}
	}
	return -1;
}
//设置上报频率
int function_set_report_time(int report_time)
{
	//struct _ty_usb_m  *stream;
	//stream = ty_usb_m_id;
	out("设置上报频率为%d\n", report_time);
	//stream->report_time_o = report_time;
	return 0;

}
//验证证书密钥是否正确
int fun_check_cert_passwd(unsigned char *plate_num, unsigned char *cert_passwd)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream,plate_num);
	if (port <= 0)
	{
		out("该盘号未在当前内存记录中找到\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;

	if (state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0)
	{
		out("记录的默认口令错误\n");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
	
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if (port_lock(state, 20, 3, DF_PORT_USED_MRKLRZ) != 0)
	{
		out("税盘正在使用中无法开票\n");
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘盘证书密码认证\n");		
#ifdef RELEASE_SO
		result = so_aisino_verify_cert_passwd(&state->h_plate,(char *)cert_passwd);
#else
		result = function_aisino_verify_cert_passwd(&state->h_plate,(char *)cert_passwd);
#endif
	}
	//else if ((state->port_info.vid == 0x1432) && (state->port_info.pid == 0x07dc))
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘证书密码认证\n");
#ifdef RELEASE_SO
		result = so_nisec_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#else
		result = function_nisec_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY证书密码认证\n");
#ifdef RELEASE_SO
		result = so_cntax_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#else
		result = function_cntax_verify_cert_passwd(&state->h_plate, (char *)cert_passwd);
#endif
	}
	//else if ((state->port_info.vid == 0x0680) && (state->port_info.pid == 0x1901))
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("蒙柏盘证书密码认证\n");
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
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result <= 0)
	{
		out("证书密码认证失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "证书密码认证", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf(errinfo, "不支持此票种");
		return DF_TAX_ERR_CODE_UNSUPPORT_INV_TYPE;
	}
	if (state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.state == 0)
	{
		sprintf(errinfo, "未找到对应发票种类的当前发票代码、号码,请确认该票种是否存在发票!");
		return DF_TAX_ERR_CODE_HAVE_NO_INVOICE;
	}
	if (state->port_info.plate_infos.invoice_type_infos[mem_i].invoice_coils_info.wsyfpjsl == 0)
	{
		sprintf(errinfo, "未找到对应发票种类的当前发票代码、号码,请确认该票种是否存在发票!");
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, plate_num);
	if (port <= 0)
	{
		out("该盘号未在当前内存记录中找到\n");
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	//if ((state->port_info.vid == 0x101d) && (state->port_info.pid == 0x0003))
	if (sp_type == DEVICE_AISINO)
	{
		out("%d号口是金税盘,进行抄报清卡动作\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "抄报清卡1", "%d号口是金税盘,进行抄报清卡动作\n", port);	
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
		out("%d号口是税控盘,进行抄报清卡动作\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "抄报清卡1", "%d号口是税控盘,进行抄报清卡动作\n", port);	
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
		out("%d号口是税务UKEY,进行抄报清卡动作\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "抄报清卡1", "%d号口是税务UKEY,进行抄报清卡动作\n", port);
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
		out("%d号口是模拟盘,进行抄报清卡动作\n", port);
		time_now = get_time_sec();
		plate_time = time_now + state->port_info.plate_sys_time;
		memset(time_s, 0, sizeof(time_s));
		get_date_time_from_second_turn(plate_time, time_s);
		cls_character((char *)time_s, strlen((const char*)time_s), 0X2D);
		logout(INFO, "MQTT", "抄报清卡1", "%d号口是模拟盘,进行抄报清卡动作\n", port);	
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
	out("通过判断找到的端口号为：%d,vid = %04x pid = %04x\n", port, state->port_info.vid, state->port_info.pid);
	if ((result = port_lock(state, 20, 4, DF_PORT_USED_FPHM_SCFP)) != 0)
	{
		if (result == POWER_ERR_NO_DEV)
		{
			sprintf(errinfo, "该盘号：%s上电失败,请重试！", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("超过满载上限\n");
			sprintf((char *)errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
		}
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘发票代码号码获取发票信息\n");
#ifdef RELEASE_SO
		result = so_aisino_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#else
		result = function_aisino_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh, inv_data);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘发票代码号码获取发票信息\n");
#ifdef RELEASE_SO
		result = so_nisec_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#else
		result = function_nisec_fpdm_fphm_update_invs(&state->h_plate, fpdm, fphm, dzsyh,inv_data);
#endif
	}
	else 
	{
		sprintf(errinfo, "税盘类型不支持发票代码号码上传发票");
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_FPHM_SCFP, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("发票查询失败\n");
		sprintf(errinfo, "该盘号：%s设备,发票代码：%s发票号码：%s查询发票失败", plate_num, fpdm, fphm);
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "按发票号码上传发票", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return result;
	}
	make_invoice_ok_update_monitor_status(state, 0);//控盘指定发票上传后更新监控信息
	return 0;
}

//申请红字信息表
int fun_upload_hzxxb(char *plate_num, uint8 inv_type,char* inv_data, struct _askfor_tzdbh_result *askfor_tzdbh_result)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)askfor_tzdbh_result->errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf((char *)askfor_tzdbh_result->errinfo, "该盘号：%s上电失败,请重试！", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("超过满载上限\n");
			sprintf((char *)askfor_tzdbh_result->errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
		}
		sprintf((char *)askfor_tzdbh_result->errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", askfor_tzdbh_result->errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘申请红字信息表信息\n");		
#ifdef RELEASE_SO
		result = so_aisino_upload_hzxxb(&state->h_plate,inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_aisino_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘申请红字信息表信息\n");
#ifdef RELEASE_SO
		result = so_nisec_upload_hzxxb(&state->h_plate, inv_type,inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_nisec_upload_hzxxb(&state->h_plate, inv_type,inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY申请红字信息表信息\n");
#ifdef RELEASE_SO
		result = so_cntax_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#else
		result = function_cntax_upload_hzxxb(&state->h_plate, inv_type, inv_data, &state->port_info.plate_infos, askfor_tzdbh_result);
#endif
	}
	else
	{
		sprintf((char *)askfor_tzdbh_result->errinfo, "该盘号：%s设备,暂不支持红字信息表申请", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_SQHZXXB, REFER_TO_TAX_ADDRESS, result, (char *)askfor_tzdbh_result->errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("红字信息表申请失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "红字信息表申请", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf((char *)askfor_tzdbh_result->errinfo, "USB操作失败,请重试");
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf(errinfo, "该盘号：%s上电失败,请重试！", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("超过满载上限\n");
			sprintf((char *)errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
		}
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘红票开具前对蓝票校验\n");		
#ifdef RELEASE_SO
		result = so_aisino_check_lzfpdm_lzfphm_allow(&state->h_plate, inv_type, lzfpdm, lzfphm, dzsyh,errinfo);
#else
		result = function_aisino_check_lzfpdm_lzfphm_allow(&state->h_plate, inv_type, lzfpdm, lzfphm, dzsyh,errinfo);
#endif
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持红票开具前对蓝票校验", plate_num);
		result =  -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_HPKJLZHMJY, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("红票开具前对蓝票校验失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "红票开具前蓝票校验", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf(errinfo, "该盘号：%s上电失败,请重试！", plate_num);
			return DF_TAX_ERR_CODE_PLATE_IN_USED;
		}
		else if (result == POWER_ERR_FULL_LOAD)
		{
			out("超过满载上限\n");
			sprintf((char *)errinfo, "该盘号：%s上电失败,超过满载上限", plate_num);
		}
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线通过红字信息表获取申请的发票\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_rednum(&state->h_plate, rednum,inv_json, errinfo);
#else
		result = function_aisino_download_hzxxb_from_rednum(&state->h_plate, rednum,inv_json, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线通过红字信息表获取申请的发票\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#else
		result = function_nisec_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey在线通过红字信息表获取申请的发票\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#else
		result = function_cntax_download_hzxxb_from_rednum(&state->h_plate, rednum, inv_json, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线发票领用", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_AXXBBHCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("税控盘在线发票领用失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "红字信息表查询", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	if (strcmp(invoice_month, summary_month) != 0)
	{
		sprintf((char *)errinfo, "通知税盘上传发票和汇总信息时月份不一致,上传发票月份：%s,上传汇总月份：%s", invoice_month,summary_month);
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
	sprintf(errinfo, "该盘号：%s正在上传其他月份发票,[year_month = %s],请稍后重试", plate_num, state->port_info.upload_inv_num.invoice_month);
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
	sprintf(errinfo, "该盘号：%s正在上传其他月份发票,[year_month = %s,summary_month = %s],请稍后重试", plate_num, state->port_info.upload_inv_num.year_month, state->port_info.upload_inv_num.summary_month);
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_MRKLRZ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘修改证书口令\n");
#ifdef RELEASE_SO
		result = so_aisino_change_certpassword(&state->h_plate, old_passwd,new_passwd, errinfo);
#else
		result = function_aisino_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘修改证书口令\n");
#ifdef RELEASE_SO
		result = so_nisec_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#else
		result = function_nisec_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif
	}
//	else if (sp_type == DEVICE_CNTAX) //证书口令与设备密码已合并，此接口已无意义
//	{
//		out("税务UKEY修改证书口令\n");
//#ifdef RELEASE_SO
//		result = so_cntax_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
//#else
//		result = function_cntax_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
//#endif
//	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("模拟盘修改证书口令\n");
#ifdef RELEASE_SO
		result = so_mb_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#else
		result = function_mengbai_change_certpassword(&state->h_plate, old_passwd, new_passwd, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持修改证书口令", plate_num);
		result =  -1;
	}
	port_unlock(state);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("金税盘修改证书口令失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "修改证书口令", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	

	char cmd[100] = { 0 };
	sprintf(cmd, "rm /tmp/passwd_err/%s.txt", (char *)plate_num);
	system(cmd);


	make_invoice_ok_update_monitor_status(state, 0);//修改证书口令后更新证书信息
	return 0;
}

int fun_cancel_hzxxb_online(char *plate_num, char *rednum, char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线撤销红字信息表申请\n");
#ifdef RELEASE_SO
		result = so_aisino_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_aisino_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线撤销红字信息表申请\n");
#ifdef RELEASE_SO
		result = so_nisec_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_nisec_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey在线撤销红字信息表申请\n");
#ifdef RELEASE_SO
		result = so_cntax_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#else
		result = function_cntax_cancel_hzxxb(&state->h_plate, rednum, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线撤销红字信息表", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("税控盘在线撤销红字信息表失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "撤销红字信息表", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
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

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线查询红字信息表", plate_num);
		result = -1;
	}
	out("在线按时间段查询红字信息表结束\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_ASJDCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("税盘在线按时间段查询红字信息表失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "按时间查询红字信息表", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	*rednum_count = result;
	return result;//此处返回值为红字信息表总数，后续会有判断
}


int fun_query_rednum_from_date_range_new(char *plate_num, char *date_range,char *gfsh,uint8 **rednum_data, int *rednum_count,char *errinfo)
{
	struct _ty_usb_m  *stream;
	struct _port_state   *state;
	int port;
	int result;
	stream = ty_usb_m_id;

	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_aisino_download_hzxxb_from_date_range_new(&state->h_plate, date_range,gfsh,rednum_data, errinfo);
#else
		result = function_aisino_download_hzxxb_from_date_range(&state->h_plate, date_range,gfsh, rednum_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_nisec_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey在线按时间段查询红字信息表\n");
#ifdef RELEASE_SO
		result = so_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#else
		result = function_cntax_download_hzxxb_from_date_range(&state->h_plate, date_range, rednum_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线查询红字信息表", plate_num);
		result = -1;
	}
	out("在线按时间段查询红字信息表结束\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_ASJDCXHZXXB, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("税盘在线按时间段查询红字信息表失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "按时间查询红字信息表", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	*rednum_count = result;
	return result;//此处返回值为红字信息表总数，后续会有判断
}

int fun_query_auth_file(char **auth_file)
{
	struct _ty_usb_m  *stream;

	stream = ty_usb_m_id;
	if (stream->cert_json == NULL)
	{
		out("授权文件为空\n");
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线发票领用查询\n");		
#ifdef RELEASE_SO
		result = so_aisino_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_aisino_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线发票领用查询\n");
#ifdef RELEASE_SO
		result = so_nisec_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_nisec_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey在线发票领用查询\n");
#ifdef RELEASE_SO
		result = so_cntax_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#else
		result = function_cntax_query_net_invoice_coil(&state->h_plate, date_range, inv_data, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线发票领用查询", plate_num);
		result = -1;
	}
	out("在线发票领用查询结束\n");
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_DLYFPCX, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("在线发票领用查询失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "发票领用查询", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}
//判断待下载发票票源是否在库存中存在
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
					out("找到相同的发票代码,判断发票号码在区间\n");
					logout(INFO, "MQTT", "发票领用下载", "%s,要下载的票源在库存中存在,待下载信息【发票类型：%03d,发票代码：%s,发票起始号码：%s,发票张数：%d】,"
						"库存中信息【发票类型：%03d,发票代码：%s,发票起始号码：%08d,发票终止号码：%08d,发票张数：%d】\r\n", state->port_info.port_str,fplxdm, fpdm, fpqshm, fpzs, state->port_info.plate_infos.invoice_type_infos[mem_i].fplxdm, \
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
	out("通过税号：%s获取端口号\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		//logout(INFO, "MQTT", "发票领用下载", "要下载的票源在库存中存在\r\n");
		return 0;
	}
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_DLYFPXZAZ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘在线发票领用下载\n");
#ifdef RELEASE_SO
		result = so_aisino_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_aisino_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘在线发票领用下载\n");
#ifdef RELEASE_SO
		result = so_nisec_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_nisec_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY在线发票领用下载\n");
#ifdef RELEASE_SO
		result = so_cntax_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#else
		result = function_cntax_net_invoice_coil_download_unlock(&state->h_plate, date_range, fplxdm, fpdm, fpqshm, fpzs, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持在线发票领用下载", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_DLYFPXZAZ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("在线发票领用下载失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "发票领用下载", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		if (strlen(errinfo) == 0)
		{
			sprintf(errinfo, "发票领用下载失败,请重试");
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1);
	//make_invoice_ok_update_monitor_status(state, 0);//下载指定发票卷后更新监控信息
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
	out("通过税号：%s检查服务器连接情况\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘检测服务器连接\n");
#ifdef RELEASE_SO
		result = so_aisino_test_server_connect(&state->h_plate,splxxx,errinfo);
#else
		result = function_aisino_test_server_connect(&state->h_plate, splxxx,errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘检测服务器连接\n");
#ifdef RELEASE_SO
		result = so_nisec_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_nisec_test_server_connect(&state->h_plate, splxxx,errinfo);
#endif
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务UKEY检测服务器连接\n");
#ifdef RELEASE_SO
		result = so_cntax_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_cntax_test_server_connect(&state->h_plate, splxxx, errinfo);
#endif
	}
	else if (sp_type == DEVICE_MENGBAI)
	{
		out("模拟盘检测服务器连接\n");
#ifdef RELEASE_SO
		result = so_mb_test_server_connect(&state->h_plate,splxxx, errinfo);
#else
		result = function_mengbai_test_server_connect(&state->h_plate, splxxx, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持检测服务器连接", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CSLJ,REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("检测服务器连接失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "检查服务器连接", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
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
		out("规避因为网络存储问题,导致zxbsrq不变的-3问题。");
		result = 0; //规避因为网络存储问题，导致的-3问题。重启正常
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
		sprintf(errinfo, "要操作的USB端口号不合法");
		return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	}
	if (power_state == 0)
	{
		out("关闭%d号端口电源\n",port);
		if (state->power_mode == Saving_Mode)
		{
			logout(INFO, "SYSTEM", "重启端口电源", "节电版不支持关闭端口电源\r\n");
		}
		else
		{
			//result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_PORT_CLOSE_PORT_POWER, port);
			//if (result <0)
			//{
			//	logout(INFO, "SYSTEM", "重启端口电源", "要求单片机关闭%d号端口电源失败,result = %d\r\n", port, result);
			//}
			//state->power = 0;
			port_power_down(state);
		}
		
	}
	else if (power_state == 1)
	{
		out("打开%d号端口电源\n", port);
		if (state->power_mode == Saving_Mode)
		{
			logout(INFO, "SYSTEM", "重启端口电源", "节电版不支持打开端口电源\r\n");
		}
		else
		{
			result = ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_OPEN_PORT_POWER, port);
			if (result <0)
			{
				logout(INFO, "SYSTEM", "重启端口电源", "要求单片机打开%d号端口电源失败,result = %d\r\n", port, result);
			}
			state->power = 1;
		}
	}
	else if(power_state == 2)
	{
		out("重启%d号端口电源\n", port);
		restart_plate_power(state, state->port_info.sn, state->port, 0);
	}
	else 
	{
		sprintf(errinfo, "操作端口电源指令错误");
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}

	return 0;
}
//获取库版本号
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
		out("要求清除全部错误信息缓存\n");
		system("rm /tmp/passwd_err/*");
		for (port = 1; port <= stream->usb_port_numb; port++)
		{
			out("情况%d号口信息\n",port);
			state = &stream->port[port - 1];
			make_invoice_ok_update_monitor_status(state, 0);//清除证书口令错误缓存后更新证书信息
		}
	}
	else
	{
		out("要求清除%s税盘缓存信息\n", plate_num);
		char cmd[100] = {0};
		sprintf(cmd, "rm /tmp/passwd_err/%s.txt", plate_num);
		system(cmd);
		port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
		if (port <= 0)
		{
			out("该盘号未在当前内存记录中找到\n");
			return 0;
		}
		state = &stream->port[port - 1];
		make_invoice_ok_update_monitor_status(state, 0);//清除证书口令错误缓存后更新证书信息
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
	//out("通过税号：%s检查服务器连接情况\n", plate_num);
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	int sp_type = state->port_info.usb_app;
	if (sp_type == DEVICE_MENGBAI2)
	{
		if (port_lock(state, 20, 3, DF_PORT_USED_HQFWSJMXX) != 0)
		{
			sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
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
				logout(INFO, "MQTT", "授权key信息", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
				sprintf(errinfo, "USB操作失败,请重试");
				restart_plate_power(state, (uint8 *)plate_num, port, 1);
			}
			sprintf(errinfo, "获取授权密钥信息失败");
			return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
		}
		return result;
	}
	sprintf(errinfo, "该卡号：%s不支持", plate_num);
	return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;

}

//接入公共服务平台
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘接入公共服务平台\n");
#ifdef RELEASE_SO
		result = so_aisino_connect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_aisino_connect_pubservice(&state->h_plate, inv_type, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘接入公共服务平台\n");
#ifdef RELEASE_SO
		result = so_nisec_connect_pubservice(&state->h_plate, inv_type,errinfo);
#else
		result = function_nisec_connect_pubservice(&state->h_plate, inv_type, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持接入公共服务平台接口", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_SQJRGGFWPT, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("接入公共服务平台失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "接入公共服务平台", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1);//重启电源更新税盘状态
	return 0;
}
//取消接入公共服务平台
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘取消接入公共服务平台\n");
#ifdef RELEASE_SO
		result = so_aisino_disconnect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_aisino_disconnect_pubservice(&state->h_plate, inv_type, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘取消接入公共服务平台\n");
#ifdef RELEASE_SO
		result = so_nisec_disconnect_pubservice(&state->h_plate,inv_type, errinfo);
#else
		result = function_nisec_disconnect_pubservice(&state->h_plate, inv_type, errinfo);
#endif		
	}
	else 
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持取消接入公共服务平台接口", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXJRGGFWPT, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("取消接入公共服务平台失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "取消接入公共服务平台", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo,"USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	restart_plate_power(state, (uint8 *)plate_num, port, 1); //重启电源更新税盘状态
	return 0;
}

//服务器连接握手--hello
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
		else
			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->h_plate.hDev->postMessage_flag = postMessage_flag;
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	/*if (NULL == state->h_plate)								//wang 01-17 查看 盘号：917005374751,名称：无锡市悦世维电子有限公司 Busnum、Devnum信息发生变更问题
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "h_plate 句柄发生变化\r\n");
	}*/
	if (NULL == state->h_plate.hDev)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hDev 句柄发生变化\r\n");
	}
	if (NULL == state->h_plate.hDev->hUSB)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hUSB 句柄发生变化\r\n");
	}	
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取与服务器连接握手数据\n");
#ifdef RELEASE_SO
		result = so_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取与服务器连接握手数据\n");
#ifdef RELEASE_SO
		result = so_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取与服务器连接握手数据\n");		
#ifdef RELEASE_SO
		result = so_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取与服务器连接握手数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取与服务器连接握手数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取与服务器连接握手数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	return 0;
}

//服务器连接认证--auth
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
		else
			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJRZ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_nisec_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_nisec_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_cntax_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_cntax_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取与服务器连接认证数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取与服务器连接认证数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取与服务器连接认证数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	return 0;
}

//获取证书
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
		else
			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
	}
#endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取证书数据\n");
#ifdef RELEASE_SO
		result = so_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取证书数据\n");
#ifdef RELEASE_SO
		result = so_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取证书数据\n");
#ifdef RELEASE_SO
		result = so_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取证书数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取证书数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取证书数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	return 0;
}



/////
//服务器连接握手--hello
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
// 			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
// 		else
// 			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->h_plate.hDev->postMessage_flag = postMessage_flag;
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	/*if (NULL == state->h_plate)								//wang 01-17 查看 盘号：917005374751,名称：无锡市悦世维电子有限公司 Busnum、Devnum信息发生变更问题
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "h_plate 句柄发生变化\r\n");
	}*/
	if (NULL == state->h_plate.hDev)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hDev 句柄发生变化\r\n");
	}
	if (NULL == state->h_plate.hDev->hUSB)
	{
		logout(INFO, "TAXLIB", "fun_client_hello : INTERFACE", "hUSB 句柄发生变化\r\n");
	}	
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取与服务器连接握手数据\n");
#ifdef RELEASE_SO
		result = so_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_aisino_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取与服务器连接握手数据\n");
#ifdef RELEASE_SO
		result = so_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_nisec_client_hello(&state->h_plate, client_hello, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取与服务器连接握手数据\n");		
#ifdef RELEASE_SO
		result = so_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#else
		result = function_cntax_client_hello(&state->h_plate, client_hello, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取与服务器连接握手数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取与服务器连接握手数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取与服务器连接握手数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	return 0;
}

//服务器连接认证--auth
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
// 			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
// 		else
// 			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJRZ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_aisino_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_nisec_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_nisec_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取与服务器连接认证数据\n");
#ifdef RELEASE_SO
		result = so_cntax_client_auth(&state->h_plate, server_hello,client_auth, errinfo);
#else
		result = function_cntax_client_auth(&state->h_plate, server_hello, client_auth, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取与服务器连接认证数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取与服务器连接认证数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取与服务器连接认证数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	return 0;
}

//获取证书
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
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
// 			sprintf(errinfo, "税盘默认证书口令不正确,确认证书口令是否为12345678,如已修改请重启机柜");
// 		else
// 			sprintf(errinfo, "税盘证书存在异常,如确认税盘无异常,请检查税盘线缆和接口");
// 		return DF_TAX_ERR_CODE_PLATE_CERT_PASSWD_ERR;
// 	}
// #endif
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 3, DF_PORT_USED_FWQLJWS) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘获取证书数据\n");
#ifdef RELEASE_SO
		result = so_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_aisino_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘获取证书数据\n");
#ifdef RELEASE_SO
		result = so_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_nisec_get_cert(&state->h_plate, cert_data, errinfo);
#endif	
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey获取证书数据\n");
#ifdef RELEASE_SO
		result = so_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#else
		result = function_cntax_get_cert(&state->h_plate, cert_data, errinfo);
#endif
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持获取证书数据", plate_num);
		result = -1;
	}
	port_unlock(state);
	if (result < 0)
	{
		out("获取证书数据失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "获取证书数据", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	state->up_report = 1;//端口状态更新需要上报
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
		//out("http数据有误，不存在回车换行符,检测是否存在换行符\r\n");
		p = strstr(http_data, "\n");
		if (p == NULL)
		{
			out("http数据有误，不存在换行符\r\n");
			return -1;
		}
		
	}
	int buf_len = p - http_data;
	if (buf_len > size)
	{
		out("http数据有误，单行数据超过缓存大小\r\n");
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
		//out("http数据有误，不存在两个回车换行符,检测是否存在两个换行符\r\n");
		p = strstr(http_data, "\n\n");
		if (p == NULL)
		{
			out("http数据有误，不存在两个换行符\r\n");
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
		out("http数据有误，单行数据超过缓存大小\r\n");
		return -2;
	}
	memcpy(buf, p, buf_len);
	return buf_len;
}

static int response_order_tran(char *buf, int buf_len, char *http_base)
{   //uint8 s_buf[409600];
	char *s_buf = NULL;
	//int len,result;
	//out("回复数据：\n");
	s_buf = calloc(1, buf_len + 1024);
	if (s_buf == NULL)
	{
		out("分配接收内存失败\n");
		return -1;
	}
	memset(s_buf, 0, buf_len + 1024);
	/*回应客户端正确的 HTTP 请求 */
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
	//out("回复数据：\n");
	s_buf = calloc(1, buf_len + 1024);
	if (s_buf == NULL)
	{
		out("分配接收内存失败\n");
		return -1;
	}
	memset(s_buf, 0, buf_len + 1024);
	/*回应客户端正确的 HTTP 请求 */
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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);

	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("电子税务局方式登录\n");
		postMessage_flag = 1;
	}

	cJSON *json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "获取版本号成功");
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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("电子税务局方式登录\n");
		postMessage_flag = 1;
	}
	cJSON *json = cJSON_CreateObject();
	stream = ty_usb_m_id;
	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "打开设备失败-未插USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		cJSON_AddNumberToObject(json, "code", 0);
		cJSON_AddStringToObject(json, "msg", "获取证书信息成功");
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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);
	stream = ty_usb_m_id;
	cJSON *json = cJSON_CreateObject();

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "打开设备失败-未插USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		if ((state->port_info.plate_infos.plate_basic_info.cert_passwd_right == 0) || 
			(strstr(body_data,"password=12345678&dwProvType=2050&strContainer=%2F%2FSM2%2FSM2CONTAINER0002") == NULL))
		{
			out("记录的默认口令错误\n");
			cJSON_AddNumberToObject(json, "code", 805306370);
			cJSON_AddStringToObject(json, "msg", "验证口令失败--未知错误(0x30000002)");
		}
		else
		{
			cJSON_AddNumberToObject(json, "code", 0);
			cJSON_AddStringToObject(json, "msg", "验证口令成功");
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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start != NULL)
	{
		//out("电子税务局方式登录\n"); authType=0
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
		out("获取client_hello失败，errinfo：%s\n", errinfo);
	}
	//printf("exit fun_client_hello\n");
	cJSON *json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "code", 0);
	if (auth_flag == 0)
		cJSON_AddStringToObject(json, "msg", "生成客户端认证请求--单项认证方式成功");
	else
		cJSON_AddStringToObject(json, "msg", "生成客户端认证请求--双项认证方式成功");
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
	out("接收数据完成,长度为：%d\n", strlen(body_data));
	out("接收数据：%s\n", body_data);
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
		out("传入的数据解析失败,缺少serverHello=\n");
		goto End;
	}
	p_start += strlen("serverHello=");
	char *p_end;
	p_end = strstr(p_start,"&");
	if (p_end == NULL)
	{
		out("传入的数据解析失败,缺少&\n");
		goto End;
	}
	// if (postMessage_flag == 0)
	// {
		
	// 	p_end = strstr(p_start, "&dwProvType");
	// 	if (p_end == NULL)
	// 	{
	// 		out("传入的数据解析失败,缺少&dwProvType\n");
	// 		goto End;
	// 	}
	// }
	// else
	// {
	// 	p_end = strstr(p_start, "&password");
	// 	if (p_end == NULL)
	// 	{
	// 		out("传入的数据解析失败,缺少&password\n");
	// 		goto End;
	// 	}
	// }

	memcpy(server_hello, p_start, p_end - p_start);

	//out("server_hello = %s\n", server_hello);


	result = fun_client_auth(plate_num, server_hello, client_auth, errinfo);
	if (result < 0)
	{
		out("获取client_hello失败，errinfo：%s\n", errinfo);
		goto End;
	}

End:
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "生成客户端认证凭据成功");
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

	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("传入的数据解析失败,缺少crosFlag=\n");
		return -1;
	}
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "设备已插入计算机");
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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("传入的数据解析失败,缺少crosFlag=\n");
		return -1;
	}

	port = find_usb_port_by_plate_num(stream, (unsigned char *)plate_num);
	if (port <= 0)
	{
		cJSON_AddNumberToObject(json, "code", 167);
		cJSON_AddStringToObject(json, "msg", "打开设备失败-未插USBKEY(0xA7)");
	}
	else
	{
		state = &stream->port[port - 1];
		cJSON_AddNumberToObject(json, "code", 0);
		cJSON_AddStringToObject(json, "msg", "获取设备号成功");


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
	//out("接收数据完成,长度为：%d\n", strlen(body_data));
	//out("接收数据：%s\n", body_data);

	cJSON *json = cJSON_CreateObject();
	char *p_start;
	p_start = strstr(body_data, "crosFlag=1");
	if (p_start == NULL)
	{
		out("传入的数据解析失败,缺少crosFlag=\n");
		return -1;
	}
	fun_get_cert(plate_num, cert_data, errinfo);
	
	cJSON_AddNumberToObject(json, "code", 0);
	cJSON_AddStringToObject(json, "msg", "读取加密证书成功");
	cJSON_AddStringToObject(json, "cert", cert_data);//此cert数据来源需查明
	
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
	
	out("fun_zzsfpzhfwpt_auth_hello_data_tran http请求数据：%s\n", http_data);

	memset(method, 0, sizeof(method));
	memset(url, 0, sizeof(url));
	memset(buf, 0, sizeof(buf));

	get_http_first_line(http_data, buf, sizeof(buf));

	get_http_body_line(http_data, body_data, sizeof(body_data));

	i = 0; j = 0;
	/*把客户端的请求方法存到 method 数组*/
	while (!ISspace(buf[j]) && (i < sizeof(method)-1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';
	//out("method = %s\n", method);

	/*如果不是 POST 则无法处理 */
	if (strcasecmp(method, "POST") != 0)
	{
		//unimplemented(client);  
		return -1;
	}

	/*读取 url 地址*/
	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url)-1) && (j < sizeof(buf)))
	{
		/*存下 url */
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';
	//out("接收到的请求URL为：%s\n", url);
	out("请求命令为%s\n", url);


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
		out("命令不支持，url = %s\n", url);
		return -1;
	}
	return 0;	
}




//按月获取税盘汇总信息接口
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXHZSJ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		out("金税盘按月获取税盘汇总信息接口\n");
#ifdef RELEASE_SO
		result = so_aisino_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_aisino_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘按月获取税盘汇总信息接口\n");
#ifdef RELEASE_SO
		result = so_nisec_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_nisec_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey按月获取税盘汇总信息接口\n");
#ifdef RELEASE_SO
		result = so_cntax_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#else
		result = function_cntax_query_invoice_month_all_data(&state->h_plate, month, data_json, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持按月获取税盘汇总信息接口", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZSJ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("取消接入公共服务平台失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "取消接入公共服务平台", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
			restart_plate_power(state, (uint8 *)plate_num, port, 1);
		}
		return DF_TAX_ERR_CODE_PLATE_OPER_FAILED;
	}
	return 0;
}

//读取历史购票信息
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	int sp_type = state->port_info.usb_app;
	if (port_lock(state, 20, 4, DF_PORT_USED_CXHZSJ) != 0)
	{
		sprintf((char *)errinfo, "税盘正在使用中,请稍后再试,Grade code:%d", state->port_info.used_level);
		out("%s\n", errinfo);
		return DF_TAX_ERR_CODE_PLATE_IN_USED;
	}
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (sp_type == DEVICE_AISINO)
	{
		sprintf((char *)errinfo, "金税盘暂不支持历史购票信息查询");
		return DF_TAX_ERR_CODE_TER_SYSTEM_ERROR;
	}
	else if (sp_type == DEVICE_NISEC)
	{
		out("税控盘历史购票信息查询接口\n");
#ifdef RELEASE_SO
		result = so_nisec_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#else
		result = function_nisec_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#endif		
	}
	else if (sp_type == DEVICE_CNTAX)
	{
		out("税务ukey历史购票信息查询接口\n");
#ifdef RELEASE_SO
		result = so_cntax_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#else
		result = function_cntax_read_inv_coil_number_history(&state->h_plate, data_json, errinfo);
#endif		
	}
	else
	{
		sprintf(errinfo, "该盘号：%s设备,暂不支持历史购票信息查询", plate_num);
		result = -1;
	}
	port_unlock(state);
	update_tax_business_info(state, DF_PORT_USED_CXHZSJ, REFER_TO_TAX_ADDRESS, result, errinfo);
	state->up_report = 1;//端口状态更新需要上报
	state->last_act = 1;
	if (result < 0)
	{
		out("历史购票信息查询失败\n");
		if (result == DF_TAX_ERR_CODE_USB_IO_HANDLE_ERROR)
		{
			logout(INFO, "MQTT", "历史购票信息查询", "%s,Busnum、Devnum信息发生变更,需重启端口\r\n", state->port_info.port_str);
			sprintf(errinfo, "USB操作失败,请重试");
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

		//out("终端基本信息组包完成\n");
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
		//out("终端模块支持组包完成\n");
	
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
	//out("终端网口管理组包完成\n");
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




		//out("终端mysql管理组包完成\n");
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
		//out("终端扩展内存管理组包完成\n");
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
		//out("终端ftp管理组包完成\n");
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
		//out("终端ssh管理组包完成\n");
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
		//out("终端ntpdate管理组包完成\n");
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
		//out("终端tfcard管理组包完成\n");
		if (atoi(mode_support.event_log_support) != 0)
		{

			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "event_log_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "event_log_enable", mode_enable.event_log_enable);
			cJSON_AddStringToObject(layer, "event_log_num", "0");
			cJSON_AddStringToObject(layer, "event_log", "");
		}
		//out("终端事件日志管理组包完成\n");
		if (atoi(mode_support.usbshare_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "usbshare_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "usbshare_enable", mode_enable.usbshare_enable);
			cJSON_AddStringToObject(layer, "port", "3240");
			cJSON_AddStringToObject(layer, "stop_use", "0");
		}
		//out("终端usbshare管理组包完成\n");
		if (atoi(mode_support.device_manage_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "device_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "device_manage_enable", mode_enable.device_manage_enable);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
		}
		//out("终端设备管理管理组包完成\n");
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
		//out("终端mqtt管理组包完成\n");
		if (atoi(mode_support.tax_invoice_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tax_invoice_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tax_invoice_enable", mode_enable.tax_invoice_enable);
		}
		//out("终端税务发票管理组包完成\n");
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
		//out("终端CA进程管理组包完成\n");
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
	//out("终端用户登录进程管理组包完成\n");
	cJSON *usblayer;
	

	int port;
	int err_num = 0;
	char usb_port_num[10] = { 0 };
	char without_dev_port_num[10] = { 0 };
	char without_dev_ports[500] = { 0 };
	cJSON_AddItemToObject(data_layer, "usb_port_manage", usblayer = cJSON_CreateObject());
	int port_num = stream->usb_port_numb;
	//out("该终端一共有%d个USB端口\n");
	for (port = 1; port<= port_num; port++)
	{
		if (stream->port[port - 1].port_info.with_dev != 1)
		{
			//out("%d号端口没有设备\n", port_count + 1);
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



	cJSON *port_array;		 //组包使用
	
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
				cJSON_AddStringToObject(data_layer, "report_tax_status", "抄报清卡均成功");
			else
				cJSON_AddStringToObject(data_layer, "report_tax_status", "税盘刚启动,暂未执行到抄报清卡动作");
		}
		//wang 04 03 
		else
		{
			//cJSON_AddStringToObject(data_layer, "report_tax_status", "税盘已执行抄报清卡动作,未收到反馈数据");
		}
	}
	else
	{
		if (strcmp((const char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "抄报清卡均成功") == 0)
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
					sprintf((char *)state->port_info.plate_infos.plate_tax_info.report_tax_status, "票种%03d反写监控不成功", state->port_info.plate_infos.invoice_type_infos[types_count].fplxdm);
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
			logout(ERROR, "system", "税盘同步", "%s,税盘信息组包过程中发现开票截止时间异常\r\n", state->port_info.port_str);
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
			else//目前高灯方式
			{
				cJSON_AddItemToObject(type_layer, "unused_invoices", coil_array = cJSON_CreateArray());
				//out("未使用发票卷总数:%d,port = %d\n", stream->port[port_num].port_info.tax_infos[types_num].wsyfpjsl, port_num+1);
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


	out("服务器反馈的当前mqtt证书文件md5值为：%s\n", crt_md5);
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
	out("获取到的MQTT服务器地址为%s\n", mqtt_server);
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
	out("获取到的MQTT服务器端口号为%s\n", mqtt_port);



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
	out("获取到的MQTT uid%s\n", uid);
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
	out("获取到的MQTT pwd%s\n", pwd);



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
	//out("获取到的ca.crt证书文件为%s\n", ca_cert_base);

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
	//out("获取到的client.crt证书文件为%s\n", client_cert_base);

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
	//out("获取到的client.key证书文件为%s\n", client_key_base);

	char tmp[4000];
	if (mqtt_type == DF_MQTT_FPKJXT)
	{
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/ca.crt", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/ca.crt");
			save_cert_file(ca_cert_base, "/etc/ca.crt");
		}
		else
		{
			if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("ca.cert文件不一致需要更新\n");
				save_cert_file(ca_cert_base, "/etc/ca.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/client.crt", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/client.crt");
			save_cert_file(client_cert_base, "/etc/client.crt");
		}
		else
		{
			if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("client.cert文件不一致需要更新\n");
				save_cert_file(client_cert_base, "/etc/client.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/client.key", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/client.key");
			save_cert_file(client_key_base, "/etc/client.key");
		}
		else
		{
			if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
			{
				out("client.key文件不一致需要更新\n");
				save_cert_file(client_key_base, "/etc/client.key");
			}
		}
	}
	else
	{
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_ca.crt", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/fptbxt_ca.crt");
			save_cert_file(ca_cert_base, "/etc/fptbxt_ca.crt");
		}
		else
		{
			if (memcmp(ca_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_ca.cert文件不一致需要更新\n");
				save_cert_file(ca_cert_base, "/etc/fptbxt_ca.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_client.crt", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/fptbxt_client.crt");
			save_cert_file(client_cert_base, "/etc/fptbxt_client.crt");
		}
		else
		{
			if (memcmp(client_cert_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_client.cert文件不一致需要更新\n");
				save_cert_file(client_cert_base, "/etc/fptbxt_client.crt");
			}
		}
		memset(tmp, 0, sizeof(tmp));
		if (read_file("/etc/fptbxt_client.key", tmp, sizeof(tmp)) < 0)
		{
			out("获取文件失败:[%s]\n", "/etc/fptbxt_client.key");
			save_cert_file(client_key_base, "/etc/fptbxt_client.key");
		}
		else
		{
			if (memcmp(client_key_base, tmp, sizeof(tmp)) != 0)
			{
				out("fptbxt_client.key文件不一致需要更新\n");
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
		out("组包数据有误\n");
		return -1;
	}

#ifndef DF_OLD_MSERVER
	sprintf(parm.s_ip, "%s", "middle.mbizj.com");
	parm.s_port = 20020;
	//wang 本地调试
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
		out("function_get_mqtt_connect_data_md5_jude_update 与服务器交互失败,ret = %d\n",ret);
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
		out("function_get_mqtt_connect_data_md5_jude_update 返回json数据解析失败,ret = %d,data %s\n", ret, outdata);
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


	out("开票服务器本地mqtt证书文件md5值为：%s\n", md5_data);

	if (strcmp(md5_data, crt_md5) == 0)
	{
		out("本地证书文件与服务器证书文件md5值一致,不需要更新 mqtt_type= %d\n", mqtt_type);
		return 0;
	}

	out("本地证书文件与服务器证书文件md5值不一致,需要更新 mqtt_type= %d\n", mqtt_type);
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
		sprintf(parm.s_ip, "%s", "192.168.0.139"); //本地测试
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
	//wang 本地调试
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
	out("获取证书文件\n");
	ret = http_token_get_mqtt_connect_info(&parm, json_data, strlen(json_data), &r_buf, &r_len);
	if (ret < 0)
	{
		out("function_get_mqtt_connect_data_info_update 与服务器交互失败,ret = %d\n", ret);
		free(json_data);
		return ret;
	}
	free(json_data);
	out("获取证书文件成功\n");
	mb_dec_file_function(stream->key_s, r_buf, strlen(r_buf), &outdata, &outdatalen);
	//out("解密数据成功,解密后数据：%s\n", outdata);
	free(r_buf);


	analysis_mqtt_info_data(outdata, server_addr, server_port, mqtt_uid, mqtt_pwd,mqtt_type);
	free(outdata);
	if (strcmp(mqtt_uid, stream->ter_id) != 0)
	{
		logout(INFO, "system", "mqtt信息获取", "服务器返回的终端ID与本机不符\r\n");
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
			//	state->up_report = 1;//端口状态更新需要上报
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
		sprintf((char *)errinfo, "税盘离线,盘号：%s,不在当前%s机柜", plate_num, stream->ter_id);
		out("%s\n", (char *)errinfo);
		return DF_TAX_ERR_CODE_PLATE_NON_EXISTENT;
	}
	state = &stream->port[port - 1];
	
	if (mode == 1)//停止上传
	{
		state->port_info.offinv_stop_upload = 1;
		if (strcmp(state->port_info.used_info, DF_PORT_USED_LXSC) == 0)
		{
			//out("检测到正在占用，直接强制结束！\n");
			if (state->h_plate.hDev != NULL)
			{
				state->h_plate.hDev->bBreakAllIO = 1;
			}
		}
		else
		{
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
	}
	else
	{
		//如果存在离线票未上传则触发离线票上传
		if ((state->port_info.plate_infos.plate_tax_info.off_inv_num != 0) && (state->port_info.offinv_num_exit == 0))//防止发票多次上传失败，后续无开票或其他插拔动作不再重传
			state->port_info.offinv_num_exit = state->port_info.plate_infos.plate_tax_info.off_inv_num;
		state->port_info.offinv_stop_upload = 0;
		state->up_report = 1;//端口状态更新需要上报
		state->last_act = 1;
	}
	return 0;
}

//全电登陆
/*
输入参数：
	busid:设备busid
	related_type:登陆人身份(办税员、法人...)
	full_name:登陆人姓名
	mobile:手机号
	idcard:身份证号
	sz_password:密码
输出参数：
	errinfo:错误信息
	lpgoto:一键登录goto参数
	lpcode:一键登录code参数
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306登陆人身份为空");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306登陆人密码为空");
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
		state->up_report = 1;//端口状态更新需要上报
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

//用户信息
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
				sprintf((char *)errinfo, "306设备busid为空");
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
		state->up_report = 1;//端口状态更新需要上报
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

//企业信息
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306登陆人身份为空");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306登陆人密码为空");
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
		state->up_report = 1;//端口状态更新需要上报
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

//企业切换
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306登陆人身份为空");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306登陆人密码为空");
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
		state->up_report = 1;//端口状态更新需要上报
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
// 				sprintf((char *)errinfo, "306设备busid为空");
// 				break;
// 			}
// 		}
// 		if(NULL == related_type){
// 			sprintf((char *)errinfo, "306登陆人身份为空");
// 			break;
// 		}		
// 		if((NULL == mobile)&&(NULL == idcard)){
// 			sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
// 			break;
// 		}
// 		if(NULL == sz_password){
// 			sprintf((char *)errinfo, "306登陆人密码为空");
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
// 		state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
		}
		if(NULL == related_type){
			sprintf((char *)errinfo, "306登陆人身份为空");
			break;
		}		
		if((NULL == mobile)&&(NULL == idcard)){
			sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
			break;
		}
		if(NULL == sz_password){
			sprintf((char *)errinfo, "306登陆人密码为空");
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
		state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpkjurl){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpkjurl){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == nsrsbh){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
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
		state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
				break;
			}
			if(NULL == fpcxsj){
				sprintf((char *)errinfo, "306发票查询条件为空");
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
			state->up_report = 1;//端口状态更新需要上报
			state->last_act = 1;
		}
		state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
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
			state->up_report = 1;//端口状态更新需要上报
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
				sprintf((char *)errinfo, "306设备busid为空");
				break;
			}
			if(NULL == related_type){
				sprintf((char *)errinfo, "306登陆人身份为空");
				break;
			}		
			if((NULL == mobile)&&(NULL == idcard)){
				sprintf((char *)errinfo, "306登陆人手机号以及身份证号码为空");
				break;
			}
			if(NULL == sz_password){
				sprintf((char *)errinfo, "306登陆人密码为空");
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
			state->up_report = 1;//端口状态更新需要上报
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