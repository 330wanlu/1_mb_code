#include "../inc/machine_infor.h"

static int machine_infor_open(struct ty_file	*file);
static int machine_infor_ctl(void *data,int fd,int cm,va_list args);
static int machine_infor_close(void *data,int fd);        
static int creat_ter_info_file(struct _machine_infor  *stream);
static void delete_log_file(struct _machine_infor *stream);
static void delete_tax_log_file(struct _machine_infor *stream);
#ifdef RELEASE_SO
static void delete_invalid_file(void);
#endif
static int check_process_extend_spcace(struct _machine_infor      *stream);
static int check_process_mysql(struct _machine_infor	*stream);
static int check_process_sqlite(struct _machine_infor	*stream);
static int get_mysql_process(struct _machine_infor_fd *id, va_list args);
static int check_process_ftp(struct _machine_infor      *stream);
static int check_process_ssh(struct _machine_infor      *stream);
static int check_process_tf_card(struct _machine_infor      *stream);
static int check_process_careader(struct _machine_infor      *stream);
static int check_process_userload(struct _machine_infor      *stream);
static int check_process_ntpdate(struct _machine_infor      *stream);
static int get_ex_space_mnt();
static int mode_support(struct _machine_infor *stream, struct _mode_support *mode_support);
static void process_check_task(void *arg, int timer);
static int set_try_out_date(struct _machine_infor_fd *id,va_list args);
static int get_id(struct _machine_infor_fd *id,va_list args);
static int set_id(struct _machine_infor_fd *id,va_list args);
//static int get_hard(struct _machine_infor_fd *id,va_list args);
static int get_kernel(struct _machine_infor_fd *id,va_list args);
static int get_soft(struct _machine_infor_fd *id,va_list args);
static int get_dev_infor(struct _machine_infor_fd *id,va_list args);
static int power_off(struct _machine_infor_fd *id,va_list args);
static int power_reset(struct _machine_infor_fd *id,va_list args);
static int get_try_out_date(struct _machine_infor_fd *id,va_list args);
static int get_mqtt_server_info(struct _machine_infor_fd *id, va_list args);
static int get_mqtt_user_passwd(struct _machine_infor_fd *id, va_list args);
static int get_mqtt_user_passwd_mem(struct _machine_infor_fd *id, va_list args);
static int get_ter_information(struct _machine_infor_fd *id, va_list args);
static int get_mode_support(struct _machine_infor_fd *id, va_list args);
static int get_mode_enable(struct _machine_infor_fd *id, va_list args);

static int get_extend_space_process(struct _machine_infor_fd *id, va_list args);
static int get_ftp_process(struct _machine_infor_fd *id, va_list args);
static int get_ssh_process(struct _machine_infor_fd *id, va_list args);
static int get_ntpdate_process(struct _machine_infor_fd *id, va_list args);
static int get_tfcard_process(struct _machine_infor_fd *id, va_list args);
static int get_careader_process(struct _machine_infor_fd *id, va_list args);
static int get_userload_process(struct _machine_infor_fd *id, va_list args);

static void reset_systerm(void *arg);
static void off_systerm(void *arg);

static int get_server_par_set_app(char *address, int *port);
static int set_server_par_set_app(char *address,int port);
static int get_configure(char *path, char *in_data, char *out_data);
static float get_memoccupy(MEM_OCCUPY *mem);
/*static float cal_occupy(struct occupy *o, struct occupy *n);
static void get_occupy(struct occupy *o);*/
static int check_cpu(char  *cpu_per, char *scpu_num);
static int get_total_mem(int *mount, int *total, int *free);
static int get_sdcard_mount(void);

int read_ma_id(struct st_ma_id  *id);
int read_kernel(char *ker);
int get_kernel_vision(char *ker,int *kver1,int hard_type);

static struct _mode_enable mode_enable_a33_default =
{
	.eth_enable="1",
	.wifi_enable="1",
	.g4_enable="1",
	.mysql_enable="0",
	.sqlite_enable="0",
	.ftp_enable="0",
	.ssh_enable="1",
	.ntpdate_enable="1",
	.tf_card_enable="0",
	.event_log_enable="1",
	.usbshare_enable="1",
	.device_manage_enable="0",
	.mqtt_enable="1",
	.tax_invoice_enable="0",
	.extend_space_enable = "1",
#ifdef DF_SUPPORT_CA
	.careader_enable="1",
#else
	.careader_enable="0",
#endif
	.userload_enable = "0",
	.usb_port_enable = "1"
};

static struct _mode_enable mode_enable_a20_default =
{
	.eth_enable = "1",
	.wifi_enable = "1",
	.g4_enable = "1",
	.mysql_enable = "1",
	.sqlite_enable="1",
	.ftp_enable = "1",
	.ssh_enable = "1",
	.ntpdate_enable = "1",
	.tf_card_enable = "1",
	.event_log_enable = "1",
	.usbshare_enable = "1",
	.device_manage_enable = "1",
	.mqtt_enable = "1",
	.tax_invoice_enable = "0",
	.extend_space_enable = "0",
	.careader_enable = "1",
	.userload_enable = "0",
	.usb_port_enable = "1"
};

static const struct _machine_infor_fu ctl_fun[] =
{ { DF_MACHINE_INFOR_CM_GET_ID, get_id },
{ DF_MACHINE_INFOR_CM_SET_ID, set_id },
//{ DF_MACHINE_INFOR_CM_GET_HARD, get_hard },
{ DF_MACHINE_INFOR_CM_GET_KERNEL, get_kernel },
{ DF_MACHINE_INFOR_CM_GET_SOFT, get_soft },
{ DF_MACHINE_INFOR_CM_GET_MACHINE_S, get_dev_infor },
{ DF_MACHINE_INFOR_CM_POWER_OFF, power_off },
{ DF_MACHINE_INFOR_CM_RESET_POWER, power_reset },
{ DF_MACHINE_INFOR_CM_SET_TRY_TIME, set_try_out_date },
{ DF_MACHINE_INFOR_CM_GET_TRY_TIME, get_try_out_date },
{ DF_MACHINE_INFOR_CM_GET_MQTT_INFO, get_mqtt_server_info },
{ DF_MACHINE_INFOR_CM_GET_MQTT_USER, get_mqtt_user_passwd },
{ DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM, get_mqtt_user_passwd_mem },
{ DF_MACHINE_INFOR_CM_GET_INFOMATION, get_ter_information },
{ DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, get_mode_support },
{ DF_MACHINE_INFOR_CM_GET_MODE_ENABLE, get_mode_enable },
#ifndef MTK_OPENWRT
{ DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS, get_mysql_process },
#endif
{ DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS, get_extend_space_process },
{ DF_MACHINE_INFOR_CM_GET_FTP_PROCESS, get_ftp_process },
{ DF_MACHINE_INFOR_CM_GET_SSH_PROCESS, get_ssh_process },
{ DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS, get_ntpdate_process },
{ DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS, get_tfcard_process },
{DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS,get_careader_process},
{DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS,get_userload_process}
};

static const struct _file_fuc	machine_infor_fuc=
{	.open=machine_infor_open,
	.read=NULL,
	.write=NULL,
	.ctl=machine_infor_ctl,
	.close=machine_infor_close,
	.del=NULL
};

int machine_infor_add(const char *switch_name,const _so_note    *note,int hard_type)
{   struct _machine_infor  *stream;
	int result;
	stream=malloc(sizeof(struct _machine_infor));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
    //machine_info_out("加载:[machine_infor_add]模块\n");
	memset(stream,0,sizeof(struct _machine_infor));
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
	memcpy(stream->register_name,"/dev/register",sizeof("/dev/register"));
	_register_add(note);
    //stream->hard=(char *)hard;
    stream->note=note;
    //machine_info_out("[machine_infor_add]:stream->switch_name=[%s]\n",stream->switch_name);
	result = ty_file_add(DF_MACHINE_INFOR, stream, "machine", (struct _file_fuc *)&machine_infor_fuc);
	if(result<0)
		free(stream);
   // machine_info_out("加载:[machine_infor_add]成功\n");
	stream->hard_type = hard_type;
	return result;    
}
static int machine_infor_open(struct ty_file	*file)
{   struct _machine_infor  *stream;
    int result,j;
	int i;
	uint8 cmp[6];
	char ter_id[20] = { 0 };
	memset(cmp,0,sizeof(cmp));
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //machine_info_out("打开machine_infor_open模块\n");
	
    if(stream->state==0)
    {   //machine_info_out("machine_infor_open第一次打开事件文件\n");
		memset(stream->ker_ver, 0, sizeof(stream->ker_ver));
		get_kernel_vision(stream->ker_ver, &stream->ker_update, stream->hard_type);
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   machine_info_out("打开开关事件失败\n");
            return stream->switch_fd;
        }
		//machine_info_out("machine_infor_open第一次打开注册文件\n");
		stream->register_fd=ty_open(stream->register_name,0,0,NULL);
        if(stream->register_fd<0)
        {   machine_info_out("machine_infor_open打开注册文件失败\n");
            return stream->register_fd;
        }
		for(i=0;i<10;i++)
		{
			result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_ID,&stream->devid);
			if(result < 0)
			{	//machine_info_out("%d次获取终端ID失败,即将再次获取\n",i+1);
				logout(INFO, "SYSTEM", "INIT", "%d次获取终端ID失败,即将再次获取\r\n", i + 1);
				sleep(1);
				continue;
			}
			if(memcmp(stream->devid.id,cmp,sizeof(cmp))==0)
			{	//machine_info_out("%d次获取到的机器编号为0,即将再次获取\n",i+1);
				logout(INFO, "SYSTEM", "INIT", "%d次获取到的机器编号为0,即将再次获取\r\n", i + 1);
				sleep(1);
				continue;
			}
			break;
		}
		if(i==10)
		{
			machine_info_out("获取%d次均失败\n",i);
			logout(INFO, "SYSTEM", "INIT", "获取到的机器编号%d次均失败,退出程序\r\n", i);
			exit(0);
		}

		ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_CPUID,stream->cpu_id);
		bcd2asc(stream->devid.id, (uint8 *)ter_id, 6);

		if (access(DF_TER_ID_INI, 1) != 0)
		{
			write_file(DF_TER_ID_INI, ter_id, strlen(ter_id));
			sync();
		}
		else
		{
			char buf_data[50]={0};
			read_file(DF_TER_ID_INI, buf_data, sizeof(buf_data));

			if (strcmp(ter_id, buf_data) != 0)
			{
				delete_file(DF_TER_ID_INI);
				write_file(DF_TER_ID_INI, ter_id, strlen(ter_id));
				sync();
			}
		}


#ifdef MTK_OPENWRT
		
		machine_info_out("获取机器编号成功,设置SSID,ter_id =%s\n", ter_id);
		set_wireless_config_file("	option ssid ", (char *)ter_id);
#endif
		if (access("/etc/mode_enable", 0) != 0)
		{
			//使用默认配置
			if ((stream->hard_type == 1) || (stream->hard_type == 3))
				memcpy(&stream->mode_enable, &mode_enable_a33_default, sizeof(struct _mode_enable));
#ifdef MTK_OPENWRT
			else if (stream->hard_type == 2)
				memcpy(&stream->mode_enable, &mode_enable_mtk_default, sizeof(struct _mode_enable));
#endif
			else
				memcpy(&stream->mode_enable, &mode_enable_a20_default, sizeof(struct _mode_enable));
		}
		else
		{
			//读取配置
		}
		mode_support(stream, &stream->mode_support);

		if (access("/opt/Trolltech", 0) == 0)
		{
			system("rm -rf /opt/Trolltech");
			sync();
		}

		//delete_log_file(stream);
		//delete_tax_log_file(stream);
#ifdef RELEASE_SO
		delete_invalid_file();
#endif
		_scheduled_task_open("machine_info", process_check_task, stream, 1, NULL);
        stream->lock=_lock_open("machine_infor_open",NULL);
		//machine_info_out("machine_infor_open创建设备信息文件\n");
		creat_ter_info_file(stream);
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
    //machine_info_out("打开machine_infor_open成功:j=%d\n",j);
	return j+1;	       
}


static int creat_ter_info_file(struct _machine_infor  *stream)
{
	machine_info_out("creat_ter_info_file enter\n");
	char ter_info[2000] = {0};
	char ter_id[20] = {0};
	char src_data[2000] = {0};
	
	bcd2asc(stream->devid.id, (unsigned char *)ter_id, 6);
	sprintf(ter_info, "ver=%s\nterid=%s\nserver=%s\n", stream->note->ver, ter_id, BOOT_HTTP_SERVER);

	if (access(DF_TER_INFO_FILE, 0) != 0)
	{
		machine_info_out("文件不存在需要创建\n");
		goto CREAT;
	}

	read_file(DF_TER_INFO_FILE, src_data, sizeof(src_data));
	if (strcmp(src_data, ter_info) != 0)
	{
		machine_info_out("文件内容比较不一致需要创建\n");
		goto CREAT;
	}
	machine_info_out("文件存在且一致无需更新\n");
	return 0;
CREAT:
	write_file(DF_TER_INFO_FILE, ter_info, strlen(ter_info));
	return 0;
}
#ifdef MTK_OPENWRT
static int set_wireless_config_file( char *configName, char *configBuffer)
{ 
	int configLen;
	char confvalue[100];
	int configBufferLen;
	char lineBuff[25600];// 存储读取一行的数据  
	memset(lineBuff, 0, 256);
	FILE* fp = fopen("/etc/config/wireless", "r");
	if (fp == NULL)
	{
		machine_info_out("文件打开失败\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	configLen = ftell(fp); //配置文件长度  
	fseek(fp, 0, SEEK_SET);


	char sumBuf[configLen + 2000];
	memset(sumBuf, 0, sizeof(configBufferLen + 2000)); //原文件长度 + 修改字符长度  
	while (fgets(lineBuff, 256, fp) != NULL)
	{
		//machine_info_out("%s", lineBuff);
		if (strlen(lineBuff) < 4) // 空行  
		{
			strcat(sumBuf, lineBuff);
			continue;
		}
		//machine_info_out("非空行\n");
		if (memcmp(configName, lineBuff, strlen(configName)) == 0)
		{
			//machine_info_out("匹配到对应行\n");
			memset(confvalue, 0, sizeof(confvalue));
			memcpy(confvalue, lineBuff + strlen(configName), strlen(lineBuff) - strlen(configName));
			//machine_info_out("confvalue = %s\n", confvalue);
			char tmp_s[200] = {0};
			sprintf(tmp_s,"\tMB-%s",configBuffer);
			if (memcmp(confvalue, tmp_s, strlen(tmp_s)) == 0)
			{
				//machine_info_out("无线网SSID配置值一样，不需要修改\n");
				fclose(fp);
				return 0;
			}
			memset(lineBuff, 0, 256);
			sprintf(lineBuff, "%s\tMB-%s\n", configName, configBuffer);
			strcat(sumBuf, lineBuff);
			continue;
		}
		else
		{
			//machine_info_out("非所需配置\n");
			strcat(sumBuf, lineBuff);
		}
	}
	//machine_info_out("修改好的配置文件：%s\n\n\n\n\n",sumBuf);
	fclose(fp);
	remove("/etc/config/wireless");
	FILE* f = fopen("/etc/config/wireless", "w+");
	if(f == NULL)
		return -1;
	fputs(sumBuf, f);
	fclose(f);
	sync();
	char cmd[100] = { 0 };
	char buf1[100] = {0};
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig ppp0");
	mb_system_read(cmd, buf1, sizeof(buf1),"/tmp/ifconfig_ppp0_test1.txt");
	if (strstr(buf1, "inet addr:") != NULL)
		return 0;
	else{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "/etc/init.d/network restart");
		system(cmd);
		sleep(5);
	}
	

	return 0;
}


#endif

static int publish_log_file_to_server(struct _machine_infor *stream, char *path, int size)
{
	int log_len = 0;
	char *src_file = NULL;
	int src_len = size + 1000*2;
	src_file = calloc(1, src_len);
	log_len = read_file(path, src_file, src_len);

	char *compress = NULL;
	int dest_len = size + 1000*2;
	compress = calloc(1, dest_len);
	asc_compress_base64((const unsigned char *)src_file, log_len, (unsigned char *)compress, &dest_len);
	free(src_file);
	char ter_id[20] = {0};
	char source_topic[50] = { 0 };
	char random[50] = { 0 };
	memset(ter_id, 0, sizeof(ter_id));
	bcd2asc(stream->devid.id, (uint8 *)ter_id, 6);
	cJSON *dir1, *dir2, *dir3;		//组包使用
	cJSON *json = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", ter_id);
	get_radom_serial_number("zdsblsrz", random);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsblsrz");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());


	cJSON_AddStringToObject(dir3, "ter_id", ter_id);
	cJSON_AddStringToObject(dir3, "file_data", compress);
	free(compress);

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);

	//machine_info_out("发送日志：%s\n", tmp_json);
	_m_pub_no_answer("mb_equ_ter_history_log_up", tmp_json);
	free(tmp_json);
	cJSON_Delete(json);
	

	return 0;
}

static void delete_log_file(struct _machine_infor *stream)
{
	char log_file[100];
	int result;
	char cmd[150];
	struct stat pro;
	memset(log_file, 0, sizeof(log_file));
	sprintf(log_file, "/etc/terminal.log");
	result = access(log_file, 0);
	if (result <0)
	{
		//machine_info_out("日志文件terminal.log不存在，无需判断删除\n");
		return;
	}
	memset(&pro, 0, sizeof(pro));
	stat(log_file, &pro);
#ifdef MTK_OPENWRT
	if (pro.st_size < 500 * 1024)
#else
	if (pro.st_size < 1024 * 1024)
#endif
	{
		//machine_info_out("日志文件terminal.log小于1M,无需删除\n");
		return;
	}
	machine_info_out("需要上传历史terminal.log日志\n");
	publish_log_file_to_server(stream, log_file, pro.st_size);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "rm /etc/terminal.log");
	machine_info_out("操作命令：%s\n", cmd);
	system(cmd);
	return;
}

static void delete_tax_log_file(struct _machine_infor *stream)
{
	char log_file[100];
	int result;
	char cmd[150];
	struct stat pro;
	memset(log_file, 0, sizeof(log_file));
	sprintf(log_file, "/etc/terminal_tax.log");
	result = access(log_file, 0);
	if (result <0)
	{
		//machine_info_out("日志文件terminal_tax.log不存在，无需判断删除\n");
		return;
	}
	memset(&pro, 0, sizeof(pro));
	stat(log_file, &pro);
	if (pro.st_size < 1024 * 1024)
	{
		//machine_info_out("日志文件terminal_tax.log小于1M,无需删除\n");
		return;
	}
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "rm /etc/terminal_tax.log");
	machine_info_out("操作命令：%s\n", cmd);
	system(cmd);
	return;
}

#ifdef RELEASE_SO
static void delete_invalid_file(void)
{
	int result;
	char cmd[150];
	result = access("/mnt/MB2409201CX.bin", 0);
	if (result ==0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/MB2409201CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/mnt/MB2409202CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/MB2409202CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/mnt/MB2409203CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/MB2409203CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/mnt/masstore/MB2409201CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/masstore/MB2409201CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/mnt/masstore/MB2409202CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/masstore/MB2409202CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/mnt/masstore/MB2409203CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /mnt/masstore/MB2409203CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/home/share/exe/MB2409201CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /home/share/exe/MB2409201CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/home/share/exe/MB2409202CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /home/share/exe/MB2409202CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	result = access("/home/share/exe/MB2409203CX.bin", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm /home/share/exe/MB2409203CX.bin");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}	
	#ifndef MTK_OPENWRT
	result = access("/etc/vsftpd.conf", 0);
	if (result == 0)
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "mv /etc/vsftpd.conf /etc/vsftpd.conf-back");
		machine_info_out("操作命令：%s\n", cmd);
		system(cmd);
	}
	#endif
	return;
}
#endif




static int mode_support(struct _machine_infor *stream, struct _mode_support *mode_support)
{
	int wifi_support = 0;
	if (stream->hard_type == 1)//DF_MB_HARDWARE_A20_A33值为3的机型方支持wifi，如wifi不能正常启动视为不支持wifi
	{
		wifi_support = jude_dev_support_wifi();
	}

	memset(mode_support, 0, sizeof(struct _mode_support));
	sprintf(mode_support->eth_support, "1");
	if (wifi_support == 1)
		sprintf(mode_support->wifi_support, "1");
	else
		sprintf(mode_support->wifi_support, "0");

	//printf("wifi_support == %d\n",wifi_support);
	sprintf(mode_support->g4_support, "0");
	sprintf(mode_support->mysql_support, "1");
	sprintf(mode_support->sqlite_support, "1");
	sprintf(mode_support->ftp_support, "1");
	sprintf(mode_support->ssh_support, "1");
	sprintf(mode_support->ntpdate_support, "1");
	sprintf(mode_support->tf_card_support, "1");
	sprintf(mode_support->event_log_support, "1");
	sprintf(mode_support->usbshare_support, "1");
	sprintf(mode_support->device_manage_support, "1");
	sprintf(mode_support->mqtt_support, "1");

	sprintf(mode_support->tax_invoice_support, "1");

	sprintf(mode_support->usb_port_support, "1");

	sprintf(mode_support->careader_support, "1");

	sprintf(mode_support->userload_support, "1");

	if ((stream->hard_type == 1) || ((stream->hard_type == 3)))
	{
		sprintf(mode_support->extend_space_support, "1");
	}
	else
	{
		sprintf(mode_support->extend_space_support, "0");
	}
	return 0;
}



static void process_check_task(void *arg, int timer)
{
	struct _machine_infor      *stream;
	sleep(10);
	//查询功能是否支持
	stream = arg;
	if (atoi(stream->mode_support.extend_space_support) != 0 && atoi(stream->mode_enable.extend_space_enable) != 0)
	{
		check_process_extend_spcace(stream);
	}
#ifndef MTK_OPENWRT
	if (atoi(stream->mode_support.mysql_support) != 0 && atoi(stream->mode_enable.mysql_enable) != 0)
	{
		check_process_mysql(stream);
	}
	if (atoi(stream->mode_support.sqlite_support) != 0 && atoi(stream->mode_enable.sqlite_enable) != 0)
	{
		check_process_sqlite(stream);
	}
#endif
	if (atoi(stream->mode_support.ftp_support) != 0 && atoi(stream->mode_enable.ftp_enable) != 0)
	{
		check_process_ftp(stream);
	}
	if (atoi(stream->mode_support.ssh_support) != 0 && atoi(stream->mode_enable.ssh_enable) != 0)
	{
		check_process_ssh(stream);
	}
	if (atoi(stream->mode_support.tf_card_support) != 0 && atoi(stream->mode_enable.tf_card_enable) != 0)
	{
		check_process_tf_card(stream);
	}
	if (atoi(stream->mode_support.careader_support) != 0 && atoi(stream->mode_enable.careader_enable) != 0)
	{
		check_process_careader(stream);
	}
	if (atoi(stream->mode_support.userload_support) != 0 && atoi(stream->mode_enable.userload_enable) != 0)
	{
		check_process_userload(stream);
	}
	if (atoi(stream->mode_support.ntpdate_support) != 0 && atoi(stream->mode_enable.ntpdate_enable) != 0)
	{
		check_process_ntpdate(stream);
	}
	delete_log_file(stream);
	delete_tax_log_file(stream);

	return;
}

static int machine_infor_ctl(void *data,int fd,int cm,va_list args)
{   struct _machine_infor      *stream;
    struct _machine_infor_fd   *id;
    int i,result;
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
    {   
		if(cm==ctl_fun[i].cm)
        {   _lock_set(stream->lock);
            result=ctl_fun[i].ctl(id,args);
            _lock_un(stream->lock);
            return result;
        }    
    }
    return DF_ERR_PORT_CORE_CM;
}

static int machine_infor_close(void *data,int fd)
{   struct _machine_infor      *stream;
    struct _machine_infor_fd   *id;
    //int j;
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
    {   ty_close(stream->switch_fd);
        _lock_close(stream->lock);
    }
    return 0;
}

static int get_mqtt_server_info(struct _machine_infor_fd *id, va_list args)
{
	char ter_id[13];
	char *mqtt_server;
	int *mqtt_port;
	char *mqtt_user;
	char *mqtt_passwd;
	int result;
	mqtt_server = va_arg(args, char  *);
	mqtt_port = va_arg(args, int  *);
	mqtt_user = va_arg(args, char  *);
	mqtt_passwd = va_arg(args, char  *);

	memset(ter_id, 0, sizeof(ter_id));
	bcd2asc(id->dev->devid.id,(uint8 *)ter_id,6);
	memcpy(mqtt_user, ter_id, 12);
	result = ty_ctl(id->dev->register_fd, DF_TY_MQTT_REGISTER_CM_WHETHER, ter_id, mqtt_server, mqtt_port, mqtt_passwd);
	if (result<0)
	{
		machine_info_out("查询是否在mqtt服务器上注册成功失败\n");
		return result;
	}
	//memcpy(mqtt_user,ter_id,12);
	set_server_par_set_app(mqtt_server, *mqtt_port);

	memset(id->dev->mqtt_server, 0, sizeof(id->dev->mqtt_server));
	memset(id->dev->mqtt_user, 0, sizeof(id->dev->mqtt_user));
	memset(id->dev->mqtt_passwd, 0, sizeof(id->dev->mqtt_passwd));

	strcpy(id->dev->mqtt_server, mqtt_server);
	strcpy(id->dev->mqtt_user, mqtt_user);
	strcpy(id->dev->mqtt_passwd, mqtt_passwd);
	id->dev->mqtt_port = *mqtt_port;
	machine_info_out("服务器地址为%s，端口号为%d,用户名为%s,密码为：%s\n", mqtt_server, *mqtt_port, mqtt_user, mqtt_passwd);
	return 0;
}

static int get_mqtt_user_passwd(struct _machine_infor_fd *id, va_list args)
{
	machine_info_out("get_mqtt_user_passwd 11111111111111111111\n");
	char ter_id[13];
	char address[200];
	char *mqtt_server;
	int *mqtt_port;
	char *mqtt_user;
	char *mqtt_passwd;
	int result;
	mqtt_server = va_arg(args, char  *);
	mqtt_port = va_arg(args, int  *);
	mqtt_user = va_arg(args, char  *);
	mqtt_passwd = va_arg(args, char  *);
	memset(address, 0, sizeof(address));
	

	//*mqtt_port = 18883;
	//*mqtt_port = 8400;
	memset(ter_id, 0, sizeof(ter_id));
	bcd2asc(id->dev->devid.id, (uint8 *)ter_id, 6);
	memcpy(mqtt_user, ter_id, 12);
	result = ty_ctl(id->dev->register_fd, DF_TY_MQTT_USRER_NAME_PASSWD, ter_id, mqtt_passwd);
	if (result<0)
	{
		machine_info_out("查询是否在mqtt用户名密码失败,result = %d\n",result);
		return result;
	}

	result = get_server_par_set_app(address, mqtt_port);
	if (result < 0)
	{
		machine_info_out("没有获取到配置MQTT服务器和端口配置,使用默认配置\n");
		sprintf(address, "www.mengbaiservice.com");
		*mqtt_port = 18883;
	}
	//sprintf(address, "www.mengbaiservice.com");
	//sprintf(address, "172.81.251.171");
	//result = get_DNS_ip(address, mqtt_server);
	//if (result < 0)
	//{
	//	machine_info_out("域名:%s解析失败\n", address);
	//	memcpy(mqtt_server, address, strlen(address));
	//	//return 0;
	//}

	strcpy(mqtt_server,address);

	char tmp_buf[1024] = {0};
	sprintf(tmp_buf,"mqtt_server = %s\r\nmqtt_port = %d\r\nmqtt_user = %s\r\nmqtt_passwd = %s\r\n", mqtt_server, *mqtt_port, mqtt_user, mqtt_passwd);
	write_file("/etc/mqtt_tmp.conf", tmp_buf, strlen(tmp_buf));



	memset(id->dev->mqtt_server, 0, sizeof(id->dev->mqtt_server));
	memset(id->dev->mqtt_user, 0, sizeof(id->dev->mqtt_user));
	memset(id->dev->mqtt_passwd, 0, sizeof(id->dev->mqtt_passwd));
	strcpy(id->dev->mqtt_server, mqtt_server);
	strcpy(id->dev->mqtt_user, mqtt_user);
	strcpy(id->dev->mqtt_passwd, mqtt_passwd);
	id->dev->mqtt_port = *mqtt_port;

	//memcpy(mqtt_user,ter_id,12);
	machine_info_out("服务器地址为%s，端口号为%d,用户名为%s,密码为：%s\n", mqtt_server, *mqtt_port, mqtt_user, mqtt_passwd);
	return 0;
}

static int get_mqtt_user_passwd_mem(struct _machine_infor_fd *id, va_list args)
{

	char *mqtt_server;
	int *mqtt_port;
	char *mqtt_user;
	char *mqtt_passwd;

	mqtt_server = va_arg(args, char  *);
	mqtt_port = va_arg(args, int  *);
	mqtt_user = va_arg(args, char  *);
	mqtt_passwd = va_arg(args, char  *);




	strcpy(mqtt_server, id->dev->mqtt_server);
	strcpy(mqtt_user, id->dev->mqtt_user);
	strcpy(mqtt_passwd, id->dev->mqtt_passwd);
	*mqtt_port = id->dev->mqtt_port ;

	return 0;
}


static int get_server_par_set_app(char *address,int *port)
{
	char tmp[500];
	int result;
	//=======================================================================================//
	result = get_configure(DF_CONFIG_FILE_PATH, "server_address = ",address);
	if (result < 0)
	{
		machine_info_out("获取MQTT服务器地址失败\n");
		return result;
	}
	//machine_info_out("获取到的MQTT服务器地址为：%s\n", address);

	//=======================================================================================//
	memset(tmp, 0, sizeof(tmp));
	result = get_configure(DF_CONFIG_FILE_PATH, "server_port = ", tmp);
	if (result < 0)
	{
		machine_info_out("获取MQTT服务器端口号失败\n");
		return result;
	}
	*port = atoi(tmp);
	//machine_info_out("获取到的MQTT服务器端口号为：%d\n", *port);


	return 0;

}

static int set_server_par_set_app(char *address, int port)
{
	char tmp[1024] = {0};
	int result;
	char cmd[1024] = {0};
	//=======================================================================================//

	sprintf(tmp,"#=====MQTT SERVER SET=====#\r\n  #MQTT SERVER ADDRESS\r\nserver_address = %s\r\n  #MQTT SERVER PORT\r\nserver_port = %d\r\n",address,port);
	sprintf(cmd, "rm %s", DF_CONFIG_FILE_PATH);
	system(cmd);
	result = write_file(DF_CONFIG_FILE_PATH, tmp, strlen(tmp));
	if (result < 0)
	{
		machine_info_out("修改MQTT配置文件失败\n");
		return -1;
	}
	sync();
	return 0;
}


static int get_configure(char *path, char *in_data, char *out_data)
{
	//int configLen;
	int len;
	char tmp[256];
	char* linePos = NULL;
	char lineBuff[256];// 存储读取一行的数据
	memset(lineBuff, 0, 256);
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		machine_info_out("文件打开失败\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	ftell(fp); //配置文件长度  
	fseek(fp, 0, SEEK_SET);
	while (fgets(lineBuff, 256, fp) != NULL)
	{	
		if ((lineBuff[0] == '#') || (lineBuff[0] == ';'))
			continue;
		linePos = NULL;
		linePos = strstr(lineBuff, in_data);
		if (linePos == NULL)
		{	
			memset(lineBuff, 0, sizeof(lineBuff));
			continue;
		}
		linePos += strlen(in_data);
		len = strlen(lineBuff) - strlen(in_data);
		memset(tmp, 0, sizeof(tmp));
		strncpy(tmp, linePos, len);
		char *p = strtok(tmp, "\r\n");
		if (p == NULL)
		{
			fclose(fp);
			return -1;
		}
		sprintf(out_data, "%s", p);
		fclose(fp);
		return 0;
	}
	fclose(fp);
	machine_info_out("在配置文件中没找到\" %s \"数据\n", in_data);
	return -1;
}



static int get_id(struct _machine_infor_fd *id,va_list args)
{
	machine_info_out("get_id:获取设备信息1\n");
	struct _switch_dev_id  *mac_id;
	//int result;
	//int i;
	uint8 cmp[6];
	memset(cmp,0,sizeof(cmp));
    mac_id=va_arg(args,struct _switch_dev_id  *);
    
	memcpy(mac_id->id,id->dev->devid.id,sizeof(mac_id->id));
	memcpy(mac_id->date,id->dev->devid.date,sizeof(mac_id->date));
	machine_info_out("get_id:获取设备信息2\n");
    return 0;
}

static int get_try_out_date(struct _machine_infor_fd *id,va_list args)
{
	uint8 buf[8];
	//int i;
	uint8 tryouttime[8];
	uint8 *outdate;
	outdate = va_arg(args,uint8  *);
	memset(buf,0,sizeof(buf));
	if (read_file("/etc/gettry.conf", (char *)buf, sizeof(buf))<0)
	{	machine_info_out("读取文件:[%s]失败\n","/etc/gettry.conf");
		return -1;
	}
	/*machine_info_out("读取到数据：");
	for(i=0;i<4;i++)
	{	
		machine_info_out("%02x ",buf[i]);
	}
	machine_info_out("\n");*/
	bcd2asc(buf,tryouttime,4);

	/*machine_info_out("试用时间：");
	for(i=0;i<8;i++)
	{	
		machine_info_out("%02x ",tryouttime[i]);
	}
	machine_info_out("\n");*/
	memcpy(outdate,tryouttime,4);
	sprintf((char *)outdate + strlen((const char *)outdate), "-");
	memcpy(outdate + strlen((const char *)outdate), tryouttime + 4, 2);
	sprintf((char *)outdate + strlen((const char *)outdate), "-");
	memcpy(outdate + strlen((const char *)outdate), tryouttime + 6, 2);

	/*machine_info_out("准备好的数据：");
	for(i=0;i<10;i++)
	{	
		machine_info_out("%02x ",outdate[i]);
	}
	machine_info_out("\n");*/
	return 0;

}

static int set_try_out_date(struct _machine_infor_fd *id,va_list args)
{
	uint8 *datebuf;
	machine_info_out("set_try_out_date \n");
	datebuf=va_arg(args,uint8  *);
	_t_file_c_write("/etc/gettry.conf",(char *)datebuf,4);	
	sync();
	return 0;
}

static int set_id(struct _machine_infor_fd *id,va_list args)
{   struct _switch_dev_id   	*mac_id;
    struct _switch_dev_id   	old_id;
	struct _switch_dev_only 	udid;
	struct _stm32_infor         infor;
	uint8 sev_id_bcd[6] = { 0 };
	uint8 sev_id_asc[20] = { 0 };
	uint8 udid_num[20] = { 0 };
	uint8 hard_ware[20] = { 0 };
	uint8 soft_ware[20] = {0};
    int result;
    memset(&old_id,0,sizeof(old_id));
    result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_GET_ID,&old_id);
    if(result<0)
        return result;
	mac_id=va_arg(args,struct _switch_dev_id  *);
	memcpy(sev_id_bcd,mac_id->id,sizeof(sev_id_bcd));
	bcd2asc(sev_id_bcd,sev_id_asc,sizeof(sev_id_bcd));//获取到机器编号
	result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_GET_SERIAL_NUM,&udid);
    if(result<0)
	{	machine_info_out("获取唯一标识符失败\n");
		logout(INFO, "SYSTEM", "设置机器编号", "设置机器编号获取唯一标识符失败,错误代码%d\r\n", result);
        return result;
	}
	memcpy(udid_num, udid.serial_num, sizeof(udid.serial_num));//获取到唯一标识符
	
	result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_GET_INFOR,&infor);
    if(result<0)
	{	machine_info_out("获取硬件版本失败\n");
		logout(INFO, "SYSTEM", "设置机器编号", "设置机器编号获取硬件版本失败,错误代码%d\r\n", result);
        return result;
	}
	memcpy(hard_ware, infor.hard, sizeof(infor.hard));//获取到硬件版本

	memcpy(soft_ware,id->dev->note->ver,strlen(id->dev->note->ver));//获取软件版本
	
	result=ty_ctl(id->dev->register_fd,DF_TY_REGISTER_CM_WRITE,sev_id_asc,udid_num,hard_ware,soft_ware);
	if(result<0)
	{	
		machine_info_out("将注册信息送至注册服务器失败\n");
		logout(INFO, "SYSTEM", "设置机器编号", "将注册信息送至注册服务器失败,错误代码%d\r\n", result);
		return result;
	}	
    result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_SET_ID,mac_id);
    if(result<0)
        return result;
    if(memcmp(old_id.id,mac_id->id,sizeof(old_id.id))!=0)
    {   int fd;
        char name[DF_FILE_NAME_MAX_LEN];
        machine_info_out("设置新的id\n");
        memset(name,0,sizeof(name));
        sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
        fd=ty_open(name,0,0,NULL);       
        machine_info_out("写事件\n");
        //ty_ctl(fd,DF_EVENT_CM_WRITE,DF_EVENT_NAME_RESET,DF_EVENT_REASON_SET_MA);
        ty_close(fd);
        _delay_task_add("reset",reset_systerm,id->dev,10); 
    }
    return result;
}

static int get_dev_infor(struct _machine_infor_fd *id,va_list args)
{   struct _switch_dev_class    dev_class;
    struct _stm32_infor         infor;
    struct _machine_s           *machine_s;
    int result;
    memset(&dev_class,0,sizeof(dev_class));
    if((result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_GET_CLASS,DF_SWITCH_CM_CLASS_ALL,&dev_class,1))<0)
    {   machine_info_out("获取信息失败\n");
        return result;
    }
    memset(&infor,0,sizeof(infor));
    if((result=ty_ctl(id->dev->switch_fd,DF_SWITCH_CM_GET_INFOR,&infor))<0)
    {   machine_info_out("获取版本信息失败\n");
        return result;
    }
    machine_s=va_arg(args,struct _machine_s  *);
    if(machine_s==NULL)
        return -1;
    memcpy(machine_s->id,dev_class.id,sizeof(machine_s->id));
    machine_s->n=dev_class.n;
	printf("dev_class.id=%s,dev_class.n=%d,infor.r_date=%s,infor.soft=%s,infor.hard=%s\n",dev_class.id,dev_class.n,infor.r_date,infor.soft,infor.hard);
    memcpy(machine_s->r_date,infor.r_date,sizeof(machine_s->r_date));
    memcpy(machine_s->soft,infor.soft,sizeof(machine_s->soft));
    memcpy(machine_s->hard,infor.hard,sizeof(machine_s->hard));
    return 0;
}

static int power_off(struct _machine_infor_fd *id,va_list args)
{   int reason;
    int fd;
    char name[DF_FILE_NAME_MAX_LEN];
    reason=va_arg(args,int);
    machine_info_out("关机\n");
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
    fd=ty_open(name,0,0,NULL);      
    machine_info_out("写事件\n");
    //ty_ctl(fd,DF_EVENT_CM_WRITE,DF_EVENT_NAME_POWER_OFF,reason);
    ty_close(fd);
    _delay_task_add("power_off",off_systerm,id->dev,10); 
    return 0; 
}

static int power_reset(struct _machine_infor_fd *id,va_list args)
{   int reason;
    int fd;
    char name[DF_FILE_NAME_MAX_LEN];
    reason=va_arg(args,int);
    machine_info_out("关机\n");
	logout(INFO, "SYSTEM", "重启终端", "客户端发起重启命令!\r\n");
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_EVENT_FILE_NAME);
    fd=ty_open(name,0,0,NULL);      
    machine_info_out("写事件\n");
    //ty_ctl(fd,DF_EVENT_CM_WRITE,DF_EVENT_NAME_RESET,reason);
    ty_close(fd);
    _delay_task_add("reset_systerm",reset_systerm,id->dev,10); 
    return 0;   
}





//static int get_hard(struct _machine_infor_fd *id,va_list args)
//{   char *hard;
//    hard=va_arg(args,char *);
//    if(hard==NULL)
//        return -1;
//    memcpy(hard,id->dev->hard,strlen(id->dev->hard));
//    return strlen(id->dev->hard);
//}

static int get_kernel(struct _machine_infor_fd *id,va_list args)
{   char *kernel;
	struct _machine_infor *stream;
	stream = id->dev;
    kernel=va_arg(args,char *);
    if(kernel==NULL)
        return -1;
	sprintf(kernel,"%s",stream->ker_ver);
	//get_kernel_vision(kernel);	
	return 0;
}

static int get_soft(struct _machine_infor_fd *id,va_list args)
{
	machine_info_out("get_soft11111111111111\n");
	char *soft;
	char soft_tmp[20];
	//int tmp[5];
    soft=va_arg(args,char *);
    if(soft==NULL)
        return -1;
	memset(soft_tmp,0,sizeof(soft_tmp));
    memcpy(soft_tmp,id->dev->note->ver,strlen(id->dev->note->ver));
	
	
#ifdef RELEASE_SO
	#ifdef MTK_OPENWRT
		if ((access("/home/share/exe/ca_server", 0) != 0) || (access("/home/share/exe/mb_ca_reader_n_d", 0) != 0) || (access("/home/share/exe/LIBDATA_OVER.bin", 0) != 0))
	#else
		if ((access("/home/share/exe/ca_server", 0) != 0) || (access("/home/share/exe/mb_ca_reader_n_d", 0) != 0) || (access("/home/share/exe/LIBDATA.bin", 0) != 0))
	#endif
#else
	if ((access("/home/share/exe/ca_server", 0) != 0) || (access("/home/share/exe/MB2409113CX_N_d", 0) != 0) || (access("/home/share/exe/LIBDATA.bin", 0) != 0))
#endif
	{
		//machine_info_out("soft_tmp = %s\n", soft_tmp);
		soft_tmp[1] = '-';
		soft_tmp[3] = '-';
		//machine_info_out("soft_tmp = %s\n", soft_tmp);
	}
	
	memcpy(soft, soft_tmp,8);
	machine_info_out("get_soft update ver = %s\n", soft_tmp);
    return strlen(soft);   
}
static float get_memoccupy(MEM_OCCUPY *mem) //对无类型get函数含有一个形参结构体类弄的指针O
{
	FILE *fd;
	//int n;
	char buff[256];
	MEM_OCCUPY *m;
	m = mem;
	char name[20];      //定义一个char类型的数组名name有20个元素
	char uint[20];      //定义一个char类型的数组名name有20个元素
	unsigned long used;
	float g_mem_used;
	memset(name, 0, sizeof(name));
	memset(uint, 0, sizeof(uint));                                                                                               
	fd = fopen("/proc/meminfo", "r");
	if (fd == NULL)
		return 0;
	fgets(buff, sizeof(buff), fd);
	sscanf(buff, "%s %u %s", name, (unsigned int *)&m->total, uint);

	fgets(buff, sizeof(buff), fd); //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里 
	sscanf(buff, "%s %u %s", name, (unsigned int *)&m->free, uint);
	used = m->total - m->free;

	g_mem_used = (used*100.0) / (m->total*1.0);
	fclose(fd);     //关闭文件fd
	return g_mem_used;
}
static int get_ex_space_mnt()
{
	char tmp_file[1024];
	char order[200];
	sprintf(order, "df -h > /tmp/ex_mnt.txt");
	system(order);
	read_file("/tmp/ex_mnt.txt", tmp_file, sizeof(tmp_file));
	system("rm /tmp/ex_mnt.txt");
	if (StrFind(tmp_file, "/mnt/masstore") == 0)
	{
		return 0;
	}
	return -1;
}

static int get_sys_uptime(char *uptime)
{
	char tmp_file[1024];
	char order[200];
	sprintf(order, "uptime > /tmp/uptime.txt");
	system(order);
	read_file("/tmp/uptime.txt", tmp_file, sizeof(tmp_file));
	system("rm /tmp/uptime.txt");
	//strcpy(uptime, tmp_file);
	memcpy(uptime, tmp_file + 1+12, strlen(tmp_file)-2-32-12);
	return -1;
}

static int get_total_mem(int *mount,int *total,int *free)
{
	char tmp_file[1024];
	size_t total_size = 0;
	size_t total_free_size = 0;
	unsigned long long totalBlocks = 0;
	unsigned long long totalSize = 0;
	unsigned long long freeDisk = 0;
	size_t mbTotalsize = 0;
	size_t mbFreedisk = 0;

	char order[200];
	sprintf(order, "df -h > /tmp/blocksize.txt");
	system(order);
	read_file("/tmp/blocksize.txt", tmp_file, sizeof(tmp_file));
	system("rm /tmp/blocksize.txt");

	//StrFind(tmp_file,"/mnt/masstore");

	struct statfs diskInfo;
	memset(&diskInfo, 0, sizeof(struct statfs));
	statfs("/", &diskInfo);
	totalBlocks = diskInfo.f_bsize;
	totalSize = totalBlocks * diskInfo.f_blocks;
	mbTotalsize = totalSize >> 20;
	freeDisk = diskInfo.f_bfree*totalBlocks;
	mbFreedisk = freeDisk >> 20;
	//printf("/  total=%dMB, free=%dMB\n", mbTotalsize, mbFreedisk);
	total_size += mbTotalsize;
	total_free_size += mbFreedisk;

	if (StrFind(tmp_file, "/mnt/masstore") == 0)
	{
		*mount = 1;
		memset(&diskInfo, 0, sizeof(struct statfs));
		statfs("/mnt/masstore", &diskInfo);
		totalBlocks = diskInfo.f_bsize;
		totalSize = totalBlocks * diskInfo.f_blocks;
		mbTotalsize = totalSize >> 20;
		freeDisk = diskInfo.f_bfree*totalBlocks;
		mbFreedisk = freeDisk >> 20;
		//printf("/mnt/masstore  total=%dMB, free=%dMB\n", mbTotalsize, mbFreedisk);
		total_size += mbTotalsize;
		total_free_size += mbFreedisk;
	}
	else
	{
		*mount = 0;
	}
	*total = total_size;
	*free = total_free_size;

	return 0;
}

static int get_sdcard_mount(void)
{
	int mount;
	char tmp_file[1024];
	char order[200];
	sprintf(order, "df -h > /tmp/get_sdcard.txt");
	system(order);
	read_file("/tmp/get_sdcard.txt", tmp_file, sizeof(tmp_file));
	system("rm /tmp/get_sdcard.txt");
	if (StrFind(tmp_file, "/sdcard") == 0)
	{
		mount = 1;
	}
	else
	{
		mount = 0;
	}
	return mount;
}

static int check_cpu(char  *cpu_per, char *scpu_num)
{

	//struct occupy cpu_stat1;   //定义occupy结构体变量名是ocpu含10个元素
	//struct occupy cpu_stat2;   //定义occupy结构体变量名是ncpu含10个元素
	//float g_cpu_used;

	//int cpu_num = 1;//cpu数量1
	//cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	//get_occupy(&cpu_stat1);                       //调用get函数带回结构体数组第一次
	//usleep(50000);                               //等待1秒 
	//get_occupy(&cpu_stat2);                       //调用get函数带回结构体数组第二次
	//g_cpu_used = cal_occupy(&cpu_stat1, &cpu_stat2);
	////machine_info_out("cpu used : %4.2f \n", g_cpu_used);					//打印g_cpu_used的使用情况以6位小数显示

	sprintf(cpu_per, "%4.2f%%", 0.01);
	sprintf(scpu_num, "%d", 2);
	return 2;
}


int check_mem(char  *men_per)
{
	MEM_OCCUPY mem_stat;
	float g_mem_used = 0;
	float sum = 0;
	float avg = 0;
	int i;
	//int normal = 0;
	//int abnormal = 0;
	memset(&mem_stat, 0, sizeof(mem_stat));
	//machine_info_out("获取内存使用情况\n");
	for (i = 0; i < 10; i++)
	{
		g_mem_used = get_memoccupy((MEM_OCCUPY *)&mem_stat);
		//machine_info_out("Time %d mem used : %f \n", i + 1, g_mem_used);
		//mem_s[i] = g_mem_used;
		sum += g_mem_used;
	}
	avg = sum / 10;
	//machine_info_out("获取到平均内存使用率百分比为：%4.2f \n",avg);
	sprintf(men_per, "%4.2f%%", avg);
	if (avg >= 90.00)
	{

		return -1;
		
	}
	//打印内存使用情况以2位小数显示
	//machine_info_out("mem percent %s\n", men_per);
	return 0;
}
int get_boot_size(long *boot_size)
{
	FILE *fp;
	fp = fopen("/home/share/exe/mb_boot", "r");
	if (fp == NULL)
		return -1;
	fseek(fp, 0L, SEEK_END);
	*boot_size = ftell(fp);
	fclose(fp);
	return 0;

}
static int get_ter_information(struct _machine_infor_fd *id, va_list args)
{
	char date_s[20] = {0};
	struct _ter_info *ter_info;
	struct _machine_infor *stream;
	long boot_size = 0;
	int mount, total_size, free_size;
	stream = id->dev;
	ter_info = va_arg(args, struct _ter_info *);
	memset(ter_info, 0, sizeof(struct _ter_info));
	sprintf(ter_info->product_code, "T002");
	sprintf(ter_info->product_name,"usb机器人");
	sprintf(ter_info->product_rename, "MBITUSB");
	bcd2asc(stream->devid.id, (uint8 *)ter_info->ter_id, 6);
	time_asc(stream->devid.date, (uint8 *)ter_info->product_date, 2);
	strcpy(ter_info->cpu_id,stream->cpu_id);
	get_total_mem(&mount, &total_size, &free_size);
	get_boot_size(&boot_size);
	get_sys_uptime(ter_info->uptime);
	ter_info->boot_size = boot_size;
	sprintf(ter_info->total_space, "%dMB", total_size);
	sprintf(ter_info->left_space, "%dMB", free_size);
	sprintf(ter_info->ram, "512MB");
	check_mem(ter_info->mem_per);
	check_cpu(ter_info->cpu_per, ter_info->cpu_num);
	sprintf(ter_info->kernel_ver, "linux%s", stream->ker_ver);
	sprintf(ter_info->ter_version, "%s", stream->note->ver);
	/*sprintf(date_s, "%02x%02x%02x%02x%02x%02x%02x%02x", 
		stream->note->r_date[0], stream->note->r_date[1],
		stream->note->r_date[2], stream->note->r_date[3],
		stream->note->r_date[5], stream->note->r_date[6],
		stream->note->r_date[8], stream->note->r_date[9]);*/
	memcpy(date_s, stream->note->r_date,4);
	memcpy(date_s + strlen(date_s), stream->note->r_date + 5, 2);
	memcpy(date_s + strlen(date_s), stream->note->r_date + 8, 2);

	sprintf(ter_info->ter_version_detail, "%s_%s_%s_%s", stream->note->ver, date_s, stream->note->abdr, stream->note->v_num);
	sprintf(ter_info->ter_type, "%s", stream->note->type);
	return 0;
}

static int get_mode_support(struct _machine_infor_fd *id, va_list args)
{
	struct _mode_support *mode_support;
	struct _machine_infor *stream;
	stream = id->dev;
	mode_support = va_arg(args, struct _mode_support *);
	memcpy(mode_support, &stream->mode_support, sizeof(struct _mode_support));
	return 0;
}

static int get_mode_enable(struct _machine_infor_fd *id, va_list args)
{
	struct _mode_enable *mode_enable;
	struct _machine_infor *stream;
	stream = id->dev;
	mode_enable = va_arg(args, struct _mode_enable *);
	memcpy(mode_enable, &stream->mode_enable, sizeof(struct _mode_enable));
	//if (access("/etc/mode_enable", 0) != 0)
	//{
	//	//使用默认配置
	//	memcpy(mode_enable, &mode_enable_default, sizeof(struct _mode_enable));

	//}
	//else
	//{
	//	//读取配置

	//}
	return 0;
}
#ifndef MTK_OPENWRT
static int get_mysql_process(struct _machine_infor_fd *id, va_list args)
{
	struct _mysql_process *mysql_process;
	struct _machine_infor *stream;
	stream = id->dev;
	mysql_process = va_arg(args, struct _mysql_process *);
	memcpy(mysql_process, &stream->mysql_process, sizeof(struct _mysql_process));
	return 0;
}
#endif
static int get_extend_space_process(struct _machine_infor_fd *id, va_list args)
{
	struct _extend_space_process *extend_space_process;
	struct _machine_infor *stream;
	stream = id->dev;
	extend_space_process = va_arg(args, struct _extend_space_process *);
	memcpy(extend_space_process, &stream->extend_space_process, sizeof(struct _extend_space_process));
	return 0;
}
static int get_ftp_process(struct _machine_infor_fd *id, va_list args)
{
	struct _ftp_process *ftp_process;
	struct _machine_infor *stream;
	stream = id->dev;
	ftp_process = va_arg(args, struct _ftp_process *);
	memcpy(ftp_process, &stream->ftp_process, sizeof(struct _ftp_process));
	return 0;
}

static int get_ssh_process(struct _machine_infor_fd *id, va_list args)
{
	struct _ssh_process *ssh_process;
	struct _machine_infor *stream;
	stream = id->dev;
	ssh_process = va_arg(args, struct _ssh_process *);
	memcpy(ssh_process, &stream->ssh_process, sizeof(struct _ssh_process));
	return 0;
}

static int get_ntpdate_process(struct _machine_infor_fd *id, va_list args)
{
	struct _ntpdate_process *ntpdate_process;
	struct _machine_infor *stream;
	stream = id->dev;
	ntpdate_process = va_arg(args, struct _ntpdate_process *);
	memcpy(ntpdate_process, &stream->ntpdate_process, sizeof(struct _ntpdate_process));
	return 0;
}

static int get_tfcard_process(struct _machine_infor_fd *id, va_list args)
{
	struct _tfcard_process *tfcard_process;
	struct _machine_infor *stream;
	stream = id->dev;
	tfcard_process = va_arg(args, struct _tfcard_process *);
	memcpy(tfcard_process, &stream->tfcard_process, sizeof(struct _tfcard_process));
	return 0;
}

static int get_careader_process(struct _machine_infor_fd *id, va_list args)
{
	struct _careader_process *careader_process;
	struct _machine_infor *stream;
	stream = id->dev;
	careader_process = va_arg(args, struct _careader_process *);
	memcpy(careader_process, &stream->careader_process, sizeof(struct _careader_process));
	return 0;
}
static int get_userload_process(struct _machine_infor_fd *id, va_list args)
{
	struct _userload_process *userload_process;
	struct _machine_infor *stream;
	stream = id->dev;
	userload_process = va_arg(args, struct _userload_process *);
	memcpy(userload_process, &stream->userload_process, sizeof(struct _userload_process));
	return 0;
}


static void reset_systerm(void *arg)
{
	struct _machine_infor  *stream;
    char name[DF_FILE_NAME_MAX_LEN];
    int fd;
    stream=arg;
    printf("reset_systerm\n");
	system("reboot");
	ty_ctl(stream->switch_fd,DF_SWITCH_CM_RESET_MA);
	
    // memset(name,0,sizeof(name));
    // sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    // fd=ty_open(name,0,0,NULL);
    // ty_ctl(fd,DF_TY_PD_CM_RESET);
}

// static void reset_systerm(void *arg)
// {   //struct _deploy  *stream;
//     char name[DF_FILE_NAME_MAX_LEN];
//     int fd;
//     //stream=arg;
//     printf("reset_systerm\n");
//     memset(name,0,sizeof(name));
//     sprintf(name,"/dev/%s",DF_TY_PD_NAME);
//     fd=ty_open(name,0,0,NULL);
//     ty_ctl(fd,DF_TY_PD_CM_RESET);
// }

static void off_systerm(void *arg)
{   //struct _deploy  *stream;
    char name[DF_FILE_NAME_MAX_LEN];
    int fd;
    //stream=arg;
    printf("power_off\n");
    memset(name,0,sizeof(name));
    sprintf(name,"/dev/%s",DF_TY_PD_NAME);
    fd=ty_open(name,0,0,NULL);
    ty_ctl(fd,DF_TY_PC_CM_POWER_OFF);   
}






#ifndef MTK_OPENWRT
//重启mysql
static int start_process_mysql(struct _machine_infor	*stream)
{
	int time = 0;
	char oper_order[1024];	//操的命令
	time = stream->mysql_process.failed_time * 2;
	sleep(time);
	system("chmod 644 /etc/my.cnf");
	if (access("/etc/mv_mysql_datadir.sh", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/etc/mv_mysql_datadir.sh\n");
		system("rm /etc/mv_mysql_datadir.sh");
	}
	if (access("/mnt/masstore/mysql", 0) != 0)
	{
		machine_info_out("mysql运行目录不存在\n");
		if (access("/etc/mysql.tar", 0) == 0)
		{
			system("tar -xf /etc/mysql.tar -C /mnt/masstore/");
		}
	}
	//检测数据库启动所需的文件是否存在,如不存在则不启动
	if ((access(DF_MYSQL_DATA_DIR1, 0) == 0) && (access(DF_MYSQL_START_SHELL1, 0) == 0))
	{
		machine_info_out("启动老版本数据库\n");
		sleep(2);
		memset(oper_order, 0, sizeof(oper_order));
		system("chmod 777 /etc/init.d/mysqld");
		sprintf(oper_order, "/etc/init.d/mysqld restart &");
		//machine_info_out("要操作的命令：%s\n", oper_order);
		system(oper_order);
		sprintf(stream->mysql_process.version, "1.0");
	}
	else if ((access(DF_MYSQL_DATA_DIR2, 0) == 0) && (access(DF_MYSQL_START_SHELL2, 0) == 0))
	{
		machine_info_out("启动新版本数据库\n");
		sleep(2);
		if (atoi(stream->extend_space_process.mount) == 1)
		{
			memset(oper_order, 0, sizeof(oper_order));
			system("chmod 777 /etc/init.d/mysqld2.0");
			sprintf(oper_order, "/etc/init.d/mysqld2.0 restart &");
			//machine_info_out("要操作的命令：%s\n", oper_order);
			system(oper_order);
			sprintf(stream->mysql_process.version, "2.0");
		}
	}
	else
	{
		;// machine_info_out("MQSQL文件不存在无法启动\n");
	}
	return -1;
}
#endif
//重启扩展区挂载
static int start_process_extend_space(struct _machine_infor	*stream)
{
	int time = 0;
	char oper_order[1024];	//操的命令
	//先判断文件是否存在，如存在则直接调用
	if (access(DF_EXTEND_SPACE_SHELL, 0) != 0)
	{
		machine_info_out("扩展区挂载脚本不存在,直接创建\n");
#ifndef MTK_OPENWRT
		machine_info_out("新版本扩展区挂载脚本不存在,先删除massstore目录,直接创建\n");
		/*memset(oper_order, 0, sizeof(oper_order));
		sprintf(oper_order, "rm -rf /mnt/masstore");
		system(oper_order);
		sync();*/

		char sh_data[10240] = { 0 };
		sprintf(sh_data, "#!/bin/sh\n\n#-----------------ssh config-------------------------------\nexport partid=\"/dev/mmcblk0p1\"\nexport mountdir=\"/mnt/masstore\"\n\nif [ ! -d /mnt/masstore ]; then\necho \"mkfs.ext4***********************\"\nmkfs.ext4 \"$partid\"\nmkdir -p \"$mountdir\"\n\nfi\n\nmount -t ext4 -o rw \"$partid\" \"$mountdir\"\necho \"mass_storage\" > /sys/devices/virtual/android_usb/android0/functions\necho \"$partid\" > /sys/devices/virtual/android_usb/android0/f_mass_storage/lun/file\necho 1 > /sys/devices/virtual/android_usb/android0/enable");
		write_file(DF_EXTEND_SPACE_SHELL, sh_data, strlen(sh_data));

		memset(oper_order, 0, sizeof(oper_order));
		sprintf(oper_order, "chmod 777 %s", DF_EXTEND_SPACE_SHELL);
		system(oper_order);
		sync();
#else
	char sh_data[10240] = { 0 };
		sprintf(sh_data, "#!/bin/sh\n\n#-----------------ssh config-------------------------------\nexport partid=\"/dev/mmcblk0p1\"\nexport mountdir=\"/mnt/masstore\"\n\nif [ ! -d /mnt/masstore ]; then\necho \"mkfs.vfat***********************\"\nmkfs.vfat \"$partid\"\nmkdir -p \"$mountdir\"\n\nfi\n\nmount -t vfat -o rw \"$partid\" \"$mountdir\"\necho \"mass_storage\" > /sys/devices/virtual/android_usb/android0/functions\necho \"$partid\" > /sys/devices/virtual/android_usb/android0/f_mass_storage/lun/file\necho 1 > /sys/devices/virtual/android_usb/android0/enable");
		write_file(DF_EXTEND_SPACE_SHELL, sh_data, strlen(sh_data));

		memset(oper_order, 0, sizeof(oper_order));
		sprintf(oper_order, "chmod 777 %s", DF_EXTEND_SPACE_SHELL);
		system(oper_order);
		sync();
#endif
		}


	time = stream->extend_space_process.failed_time * 2;
	sleep(time);

	memset(oper_order, 0, sizeof(oper_order));
	sprintf(oper_order, "chmod 777 %s", DF_EXTEND_SPACE_SHELL);
	system(oper_order);

	memset(oper_order, 0, sizeof(oper_order));
	sprintf(oper_order, "%s &", DF_EXTEND_SPACE_SHELL);
	system(oper_order);
	return 0;
}

//重启ftp
static int start_process_ftp(struct _machine_infor	*stream)
{
	//目前vsftp由系统管理，本程序不关系此进程启动
	//if (access("/home/share/exe/start_ftp", 0) == 0)
	//{
		machine_info_out("外部要求启用ftp\n");
		system("vsftpd /etc/vsftpd.conf &");
		sleep(3);
	//}
	return 0;
}
//重启ssh
static int start_process_ssh(struct _machine_infor	*stream)
{
	//目前SSH由系统管理，本程序不关系此进程启动
	
	if (access("/home/share/exe/start_ssh", 0) == 0)
	{
		machine_info_out("外部要求启用ssh\n");
		system("/usr/local/sbin/sshd &");
		sleep(3);
	}
	return 0;
}

static int start_process_careader(struct _machine_infor	*stream)
{
	int time = 0;
	int result;
	char oper_order[1024];	//操的命令
	if (access("/home/share/exe/MB2409113CX_U", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/home/share/exe/MB2409113CX_U\n");
		system("rm /home/share/exe/MB2409113CX_U");
	}
	if (access("/home/share/exe/MB2409113CX_S", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/home/share/exe/MB2409113CX_S\n");
		system("rm /home/share/exe/MB2409113CX_S");
	}
	if (access("/lib/lib1432_07dc.so", 0) == 0)
	{
		machine_info_out("删除当前版本无效文件/lib/lib1432_07dc.so\n");
		system("rm /lib/lib1432_07dc.so");
	}
	if (access("/lib/lib101d_0003.so", 0) == 0)
	{
		machine_info_out("删除当前版本无效文件/lib/lib101d_0003.so\n");
		system("rm /lib/lib101d_0003.so");
	}
#ifdef RELEASE_SO
	if (access("/home/share/exe/MB2409113CX_N", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/home/share/exe/MB2409113CX_N\n");
		system("rm /home/share/exe/MB2409113CX_N");
	}
#endif
	time = stream->extend_space_process.failed_time * 2;
	sleep(time);

	memset(oper_order, 0, sizeof(oper_order));
	sprintf(oper_order,"chmod 777 /home/share/exe/*");
	system(oper_order);

	result = detect_process("/home/share/exe/ca_server", "tmpca_serverrrr.txt");
	if (result > 0)
	{
		machine_info_out("/home/share/exe/ca_server 进程存在,需结束\n");
		check_and_close_process("/home/share/exe/ca_server");
	}
#ifdef RELEASE_SO
	if ((access("/home/share/exe/ca_server", 0) == 0) && (access("/home/share/exe/mb_ca_reader_n", 0) == 0))
#else
	if ((access("/home/share/exe/ca_server", 0) == 0) && (access("/home/share/exe/MB2409113CX_N", 0) == 0))
#endif	
	{
		memset(oper_order, 0, sizeof(oper_order));
		sprintf(oper_order, "/home/share/exe/ca_server &");
		system(oper_order);
	}
	return 0;
}

static int start_process_userload(struct _machine_infor	*stream)
{
	int time = 0;
	char oper_order[1024];	//操的命令
	if (access("/home/share/exe/MB2409115CX_U", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/home/share/exe/MB2409115CX_U\n");
		system("rm /home/share/exe/MB2409115CX_U");
	}
	if (access("/home/share/exe/MB2409115CX_S", 0) == 0)
	{
		machine_info_out("删除当前版本无效脚本/home/share/exe/MB2409115CX_S\n");
		system("rm /home/share/exe/MB2409115CX_S");
	}
	time = stream->extend_space_process.failed_time * 2;
	sleep(time);
	sprintf(oper_order, "/home/share/exe/center_server &");
	system(oper_order);
	return 0;
}



//扩展空间管理
static int check_process_extend_spcace(struct _machine_infor      *stream)
{
	struct _extend_space_process extend_space_process;
	memset(&extend_space_process, 0, sizeof(struct _extend_space_process));
	if (get_ex_space_mnt() == 0)
	{
		stream->extend_space_process.failed_time = 0;
		sprintf(extend_space_process.mount, "1");
	}
	else
	{
		machine_info_out("扩展空间未挂载,之前失败次数为%d次\n", stream->extend_space_process.failed_time);
		if (stream->extend_space_process.failed_time < 5)//启动10次内再次重启,超过10次不在重试
		{
			start_process_extend_space(stream);
			usleep(50000);
			if (get_ex_space_mnt() == 0)
			{
				machine_info_out("扩展空间挂载成功\n");
				stream->extend_space_process.failed_time = 0;
				sprintf(extend_space_process.mount, "1");
			}
			else
			{
				machine_info_out("扩展空间挂载失败\n");
				stream->extend_space_process.failed_time += 1;
				sprintf(extend_space_process.mount, "0");
			}
		}
		else
		{
			if (access(DF_EXTEND_SPACE_SHELL, 0) == 0)
			{
				machine_info_out("扩展空间挂载脚本文件可能存在问题,删除挂载脚本\n");
				system("rm /etc/massStoreCtr.sh");
				//system("rm -rf /mnt/masstore");
				stream->extend_space_process.failed_time = 0;
			}
			sprintf(extend_space_process.mount, "0");
		}
	}
	sprintf(extend_space_process.mount_dir, "/mnt/masstore");
	sprintf(extend_space_process.space_size, "1.6G");

	if (strcmp(extend_space_process.mount, stream->extend_space_process.mount) != 0)
	{
		memset(stream->extend_space_process.mount, 0, sizeof(stream->extend_space_process.mount));
		memcpy(stream->extend_space_process.mount, extend_space_process.mount, strlen(extend_space_process.mount));
	}
	if (strcmp(extend_space_process.mount_dir, stream->extend_space_process.mount_dir) != 0)
	{
		memset(stream->extend_space_process.mount_dir, 0, sizeof(stream->extend_space_process.mount_dir));
		memcpy(stream->extend_space_process.mount_dir, extend_space_process.mount_dir, strlen(extend_space_process.mount_dir));
	}
	if (strcmp(extend_space_process.space_size, stream->extend_space_process.space_size) != 0)
	{
		memset(stream->extend_space_process.space_size, 0, sizeof(stream->extend_space_process.space_size));
		memcpy(stream->extend_space_process.space_size, extend_space_process.space_size, strlen(extend_space_process.space_size));
	}
	return 0;
}
#ifndef MTK_OPENWRT
//mysql数据库管理
static int check_process_mysql(struct _machine_infor	*stream)
{
	int result;
	struct _mysql_process mysql_process;
	memset(&mysql_process, 0, sizeof(struct _mysql_process));

	result = detect_process_path("mysqld", "/tmp/check_mysql1.log");
	if (result > 0)
	{
		stream->mysql_process.failed_time = 0;
		sprintf(mysql_process.process_run, "1");
	}
	else
	{
		if (stream->mysql_process.failed_time < 10)//启动10次内再次重启,超过10次不在重试
		{
			//machine_info_out("mysql进程未启动,即将重启,失败次数%d\n", stream->mysql_process.failed_time);
			start_process_mysql(stream);
			sleep(1);
			result = detect_process_path("/usr/local/mysql", "/tmp/check_mysql1.log");
			if (result > 0)
			{
				machine_info_out("mysql启动成功\n");
				stream->mysql_process.failed_time = 0;
				sprintf(mysql_process.process_run, "1");
			}
			else
			{
				//machine_info_out("mysql启动失败\n");
				stream->mysql_process.failed_time += 1;
				sprintf(mysql_process.process_run, "0");
			}
		}
		else
		{
			if (strcmp(stream->mysql_process.version, "2.0") == 0)
			{
				if (access(DF_MYSQL_DATA_DIR2, 0) == 0)
				{
					machine_info_out("新版数据库数据文件可能存在问题,删除数据库目录\n");
					//system("rm -rf /mnt/masstore/mysql");
					stream->mysql_process.failed_time = 0;
				}
				if (access(DF_MYSQL_START_SHELL2, 0) == 0)
				{
					machine_info_out("新版数据库启动脚本可能存在问题,删除数据库启动脚本\n");
					//system("rm /etc/init.d/mysqld2.0");
					stream->mysql_process.failed_time = 0;
				}
			}
			sprintf(mysql_process.process_run, "0");
		}
		//sprintf(mysql_process.process_run, "0");
	}

	char ps[128];
	char order[200];
	char tmp_file[40960];
	sprintf(ps, "ps | grep mysql | grep -v grep");
	memset(order, 0, sizeof(order));
	sprintf(order, "%s > %s", ps, "/tmp/check_mysql2.log");
	system(order);

	read_file("/tmp/check_mysql2.log", tmp_file, sizeof(tmp_file));
	system("rm /tmp/check_mysql2.log");
	if (StrFind(tmp_file, "/mnt/masstore/mysql") == 0)
	{
		sprintf(mysql_process.data_path, "/mnt/masstore/mysql");
	}
	else if (StrFind(tmp_file, "/sdcard/mysql") == 0)
	{
		sprintf(mysql_process.data_path, "/sdcard/mysql");
	}
	else
	{
		;//sprintf(mysql_process.data_path, "");
	}
	sprintf(mysql_process.port, "3306");
	sprintf(mysql_process.load_name, "root");
	sprintf(mysql_process.load_passwd, "123456");


	if (strcmp(mysql_process.process_run, stream->mysql_process.process_run) != 0)
	{
		memset(stream->mysql_process.process_run, 0, sizeof(stream->mysql_process.process_run));
		memcpy(stream->mysql_process.process_run, mysql_process.process_run, strlen(mysql_process.process_run));
	}
	if (strcmp(mysql_process.port, stream->mysql_process.port) != 0)
	{
		memset(stream->mysql_process.port, 0, sizeof(stream->mysql_process.port));
		memcpy(stream->mysql_process.port, mysql_process.port, strlen(mysql_process.port));
	}
	if (strcmp(mysql_process.load_name, stream->mysql_process.load_name) != 0)
	{
		memset(stream->mysql_process.load_name, 0, sizeof(stream->mysql_process.load_name));
		memcpy(stream->mysql_process.load_name, mysql_process.load_name, strlen(mysql_process.load_name));
	}
	if (strcmp(mysql_process.load_passwd, stream->mysql_process.load_passwd) != 0)
	{
		memset(stream->mysql_process.load_passwd, 0, sizeof(stream->mysql_process.load_passwd));
		memcpy(stream->mysql_process.load_passwd, mysql_process.load_passwd, strlen(mysql_process.load_passwd));
	}
	if (strcmp(mysql_process.data_path, stream->mysql_process.data_path) != 0)
	{
		memset(stream->mysql_process.data_path, 0, sizeof(stream->mysql_process.data_path));
		memcpy(stream->mysql_process.data_path, mysql_process.data_path, strlen(mysql_process.data_path));
	}
	return 0;
}

static int check_process_sqlite(struct _machine_infor	*stream)
{
	if (atoi(stream->extend_space_process.mount) == 0)
	{
		return 0;
	}
	if (access("/mnt/masstore/mbdb.db", 0) == 0)
	{		
		return 0;
	}

	//machine_info_out("第一次启动是sqlite数据库不存在,需备份\n");
	if (access("/etc/mbdb.db", 0) != 0)
	{
		//machine_info_out("备份目录中mbdb.db数据文件不存在\n");
		return 0;
	}

	system("cp /etc/mbdb.db /mnt/masstore/");
	sync();
	return 0;
}
#endif

//ftp管理
static int check_process_ftp(struct _machine_infor      *stream)
{
	int result;
	struct _ftp_process ftp_process;
	memset(&ftp_process, 0, sizeof(struct _ftp_process));

	result = detect_process_path("vsftpd", "/tmp/check_ftp1.log");
	if (result > 0)
	{
		stream->ftp_process.failed_time = 0;
		sprintf(ftp_process.process_run, "1");
	}
	else
	{
		if (stream->ftp_process.failed_time < 10)//启动10次内再次重启,超过10次不在重试
		{
			start_process_ftp(stream);
			result = detect_process_path("vsftpd", "/tmp/check_ftp1.log");
			if (result > 0)
			{
				stream->ftp_process.failed_time = 0;
				sprintf(ftp_process.process_run, "1");
				
			}
			else
			{
				stream->ftp_process.failed_time += 1;
				sprintf(ftp_process.process_run, "0");
			}
		}
		else
		{
			sprintf(ftp_process.process_run, "0");
		}
	}

	sprintf(ftp_process.port, "21");
	sprintf(ftp_process.load_name, "root");
	sprintf(ftp_process.load_passwd, "123456");
	sprintf(ftp_process.default_path, "/sdcard");

	if (strcmp(ftp_process.process_run, stream->ftp_process.process_run) != 0)
	{
		memset(stream->ftp_process.process_run, 0, sizeof(stream->ftp_process.process_run));
		memcpy(stream->ftp_process.process_run, ftp_process.process_run, strlen(ftp_process.process_run));
	}
	if (strcmp(ftp_process.port, stream->ftp_process.port) != 0)
	{
		memset(stream->ftp_process.port, 0, sizeof(stream->ftp_process.port));
		memcpy(stream->ftp_process.port, ftp_process.port, strlen(ftp_process.port));
	}
	if (strcmp(ftp_process.load_name, stream->ftp_process.load_name) != 0)
	{
		memset(stream->ftp_process.load_name, 0, sizeof(stream->ftp_process.load_name));
		memcpy(stream->ftp_process.load_name, ftp_process.load_name, strlen(ftp_process.load_name));
	}
	if (strcmp(ftp_process.load_passwd, stream->ftp_process.load_passwd) != 0)
	{
		memset(stream->ftp_process.load_passwd, 0, sizeof(stream->ftp_process.load_passwd));
		memcpy(stream->ftp_process.load_passwd, ftp_process.load_passwd, strlen(ftp_process.load_passwd));
	}
	if (strcmp(ftp_process.default_path, stream->ftp_process.default_path) != 0)
	{
		memset(stream->ftp_process.default_path, 0, sizeof(stream->ftp_process.default_path));
		memcpy(stream->ftp_process.default_path, ftp_process.default_path, strlen(ftp_process.default_path));
	}
	return 0;
}

//ssh管理
static int check_process_ssh(struct _machine_infor      *stream)
{
	int result;
	struct _ssh_process ssh_process;
	memset(&ssh_process, 0, sizeof(struct _ssh_process));

	result = detect_process_path("sshd", "/tmp/check_ssh1.log");
	if (result > 0)
	{
		stream->ssh_process.failed_time = 0;
		sprintf(ssh_process.process_run, "1");
	}
	else
	{
		if (stream->ssh_process.failed_time < 10)//启动10次内再次重启,超过10次不在重试
		{
			start_process_ssh(stream);
			result = detect_process_path("sshd", "/tmp/check_ssh1.log");
			if (result > 0)
			{
				stream->ssh_process.failed_time = 0;
				sprintf(ssh_process.process_run, "1");
			}
			else
			{
				stream->ssh_process.failed_time += 1;
				sprintf(ssh_process.process_run, "0");
			}
		}
		else
		{
			sprintf(ssh_process.process_run, "0");
		}
	}

	sprintf(ssh_process.port, "22");
	sprintf(ssh_process.load_name, "root");
	sprintf(ssh_process.load_passwd, "123456");

	if (strcmp(ssh_process.process_run, stream->ssh_process.process_run) != 0)
	{
		memset(stream->ssh_process.process_run, 0, sizeof(stream->ssh_process.process_run));
		memcpy(stream->ssh_process.process_run, ssh_process.process_run, strlen(ssh_process.process_run));
	}
	if (strcmp(ssh_process.port, stream->ssh_process.port) != 0)
	{
		memset(stream->ssh_process.port, 0, sizeof(stream->ssh_process.port));
		memcpy(stream->ssh_process.port, ssh_process.port, strlen(ssh_process.port));
	}
	if (strcmp(ssh_process.load_name, stream->ssh_process.load_name) != 0)
	{
		memset(stream->ssh_process.load_name, 0, sizeof(stream->ssh_process.load_name));
		memcpy(stream->ssh_process.load_name, ssh_process.load_name, strlen(ssh_process.load_name));
	}
	if (strcmp(ssh_process.load_passwd, stream->ssh_process.load_passwd) != 0)
	{
		memset(stream->ssh_process.load_passwd, 0, sizeof(stream->ssh_process.load_passwd));
		memcpy(stream->ssh_process.load_passwd, ssh_process.load_passwd, strlen(ssh_process.load_passwd));
	}
	return 0;
}

static int check_process_tf_card(struct _machine_infor      *stream)
{
	//使用默认配置
	struct _tfcard_process tfcard_process;
	memset(&tfcard_process, 0, sizeof(struct _tfcard_process));
	if (stream->hard_type == 1)
	{//A33
		if (access("/dev/mmcblk1p1", 0) != 0)
		{
			sprintf(tfcard_process.insert, "0");
		}
		else
		{
			sprintf(tfcard_process.insert, "1");
		}
	}
	else if ((stream->hard_type == 2) || (stream->hard_type == 3))
	{
		sprintf(tfcard_process.insert, "0");
	}
	else
	{//A20
		if (access("/dev/mmcblk0p1", 0) != 0)
		{
			sprintf(tfcard_process.insert, "0");
		}
		else
		{
			sprintf(tfcard_process.insert, "1");
		}
	}
	if (atoi(tfcard_process.insert) == 1)
	{
		if (get_sdcard_mount() == 1)
		{
			//machine_info_out("有sd卡插入,挂载成功\n");
			sprintf(tfcard_process.mount, "1");
		}
		else
		{
			machine_info_out("有sd卡插入,并未挂载\n");
			sprintf(tfcard_process.mount, "0");
			if (stream->hard_type == 1)
			{
				system("mount /dev/mmcblk1p1 /sdcard");
			}
			else
			{
				system("mount /dev/mmcblk0p1 /sdcard");
			}
		}
	}
	else
	{
		sprintf(tfcard_process.mount, "0");
	}
	if (atoi(tfcard_process.insert) && atoi(tfcard_process.mount))
		sprintf(tfcard_process.status, "1");
	else
		sprintf(tfcard_process.status, "0");

	if (strcmp(tfcard_process.insert, stream->tfcard_process.insert) != 0)
	{
		memset(stream->tfcard_process.insert, 0, sizeof(stream->tfcard_process.insert));
		memcpy(stream->tfcard_process.insert, tfcard_process.insert, strlen(tfcard_process.insert));
	}
	if (strcmp(tfcard_process.mount, stream->tfcard_process.mount) != 0)
	{
		memset(stream->tfcard_process.mount, 0, sizeof(stream->tfcard_process.mount));
		memcpy(stream->tfcard_process.mount, tfcard_process.mount, strlen(tfcard_process.mount));
	}
	if (strcmp(tfcard_process.status, stream->tfcard_process.status) != 0)
	{
		memset(stream->tfcard_process.status, 0, sizeof(stream->tfcard_process.status));
		memcpy(stream->tfcard_process.status, tfcard_process.status, strlen(tfcard_process.status));
	}
	return 0;
}

static int check_process_careader(struct _machine_infor      *stream)
{
	int result;
	struct _careader_process careader_process;
	memset(&careader_process, 0, sizeof(struct _careader_process));
#ifdef RELEASE_SO
	result = detect_process_path("mb_ca_reader", "/tmp/check_careader.log");
#else
	result = detect_process_path("MB2409113CX", "/tmp/check_careader.log");
#endif
	if (result > 0)
	{
		stream->careader_process.failed_time = 0;
		sprintf(careader_process.process_run, "1");
	}
	else
	{
		if (stream->careader_process.failed_time < 10)//启动10次内再次重启,超过10次不在重试
		{
			start_process_careader(stream);
			sleep(1);
		#ifdef RELEASE_SO
			result = detect_process_path("mb_ca_reader", "/tmp/check_careader.log");
		#else
			result = detect_process_path("MB2409113CX", "/tmp/check_careader.log");
		#endif
			
			if (result > 0)
			{
				stream->careader_process.failed_time = 0;
				sprintf(careader_process.process_run, "1");
			}
			else
			{
				stream->careader_process.failed_time += 1;
				sprintf(careader_process.process_run, "0");
			}
		}
		else
		{
			sprintf(careader_process.process_run, "0");
			//system("rm /home/share/exe/ca_server");
			//system("rm /home/share/exe/MB2409113CX*");
			//system("rm /home/share/exe/LIBDATA.bin");
		}
		//sprintf(mysql_process.process_run, "0");
	}

	sprintf(careader_process.err_describe, "正常");

	if (strcmp(careader_process.process_run, stream->careader_process.process_run) != 0)
	{
		memset(stream->careader_process.process_run, 0, sizeof(stream->careader_process.process_run));
		memcpy(stream->careader_process.process_run, careader_process.process_run, strlen(careader_process.process_run));
	}
	if (strcmp(careader_process.err_describe, stream->careader_process.err_describe) != 0)
	{
		memset(stream->careader_process.err_describe, 0, sizeof(stream->careader_process.err_describe));
		memcpy(stream->careader_process.err_describe, careader_process.err_describe, strlen(careader_process.err_describe));
	}
	return 0;
}

static int check_process_userload(struct _machine_infor      *stream)
{
	int result;
	struct _userload_process userload_process;
	memset(&userload_process, 0, sizeof(struct _userload_process));

	result = detect_process_path("MB2409115CX", "/tmp/check_userload.log");
	if (result > 0)
	{
		sprintf(userload_process.process_run, "1");
		stream->userload_process.failed_time = 0;
	}
	else
	{
		if (stream->userload_process.failed_time < 10)//启动10次内再次重启,超过10次不在重试
		{
			start_process_userload(stream);
			sleep(1);
		#ifdef RELEASE_SO
			result = detect_process_path("mb_ca_reader", "/tmp/check_userload.log");
		#else
			result = detect_process_path("MB2409113CX", "/tmp/check_userload.log");
		#endif
			
			if (result > 0)
			{
				sprintf(userload_process.process_run, "1");
				stream->userload_process.failed_time = 0;
			}
			else
			{
				stream->userload_process.failed_time += 1;
				sprintf(userload_process.process_run, "0");
			}
		}
		else
		{
			sprintf(userload_process.process_run, "0");
		}
		//sprintf(mysql_process.process_run, "0");
	}

	sprintf(userload_process.err_describe, "正常");

	if (strcmp(userload_process.process_run, stream->userload_process.process_run) != 0)
	{
		memset(stream->userload_process.process_run, 0, sizeof(stream->userload_process.process_run));
		memcpy(stream->userload_process.process_run, userload_process.process_run, strlen(userload_process.process_run));
	}
	if (strcmp(userload_process.err_describe, stream->userload_process.err_describe) != 0)
	{
		memset(stream->userload_process.err_describe, 0, sizeof(stream->userload_process.err_describe));
		memcpy(stream->userload_process.err_describe, userload_process.err_describe, strlen(userload_process.err_describe));
	}
	return 0;
}

static int check_process_ntpdate(struct _machine_infor      *stream)
{
	//int result;
	struct _ntpdate_process ntpdate_process;
	memset(&ntpdate_process, 0, sizeof(struct _ntpdate_process));

	if (access("/etc/autotime.sh", 0) == 0)
	{
		system("rm /etc/autotime.sh");
		sync();
	}
	if (access("/usr/sbin/ntpdate", 0) == 0)
	{
		sprintf(ntpdate_process.process_run, "1");
	}
	else
	{
		sprintf(ntpdate_process.process_run, "0");
	}
	sprintf(ntpdate_process.server,"ntp1.aliyun.com");
	timer_read_y_m_d_h_m_s(ntpdate_process.time_now);

	if (strcmp(ntpdate_process.process_run, stream->ntpdate_process.process_run) != 0)
	{
		memset(stream->ntpdate_process.process_run, 0, sizeof(stream->ntpdate_process.process_run));
		memcpy(stream->ntpdate_process.process_run, ntpdate_process.process_run, strlen(ntpdate_process.process_run));
	}
	if (strcmp(ntpdate_process.server, stream->ntpdate_process.server) != 0)
	{
		memset(stream->ntpdate_process.server, 0, sizeof(stream->ntpdate_process.server));
		memcpy(stream->ntpdate_process.server, ntpdate_process.server, strlen(ntpdate_process.server));
	}
	if (strcmp(ntpdate_process.time_now, stream->ntpdate_process.time_now) != 0)
	{
		memset(stream->ntpdate_process.time_now, 0, sizeof(stream->ntpdate_process.time_now));
		memcpy(stream->ntpdate_process.time_now, ntpdate_process.time_now, strlen(ntpdate_process.time_now));
	}
	return 0;
}

int read_ma_id(struct st_ma_id  *id)
{   memset(id,0,sizeof(struct st_ma_id));
    memcpy(id->hard,"YA1.00",6);
    memcpy(id->kernel,"linux 2.6.32-38-generic",strlen("linux 2.6.32-38-generic"));
    id->id[0]=0x01;id->id[1]=0x50;id->id[2]=0x20;
    id->id[3]=0x00;id->id[4]=0x00;id->id[5]=0x01;
    return 0; 
}

int read_kernel(char *ker)
{   FILE   *stream;
    char buf[2048];
    char *pbuf;
    stream = popen("uname -r","r");
    memset(buf,0,sizeof(buf));
    fread(buf,sizeof(char),sizeof(buf),stream); 
    pclose(stream);
    pbuf=strstr(buf,"\n");
    if(pbuf==0)
        return 0;
    memcpy(ker,buf,pbuf-buf);
    return 0;
}

static int get_config_file_head_end(const char *buf,const char *head,const char *end,char **p_head,char **p_end)
{	
	//char lan[100];
	//memset(lan,0,sizeof(lan));
	////sprintf(lan,"#!/bin/sh");
	//sprintf(lan,head);
	*p_head=strstr(buf,head);
	if(*p_head==NULL)
	{	/*printf("[%s]未找到\n",head);*/
		return -1;
	}
	*p_end=strstr(*p_head,end);
	if(*p_end==NULL)
	{	/*printf("未找到数据结束\n");*/
		return -2;
	}
	//*p_head+=strlen(lan);
	*p_end+=4;
	return 0;
}

static int get_config_data(char *str_s,char *end_s,char *key,char *value)
{	char *ps,*end_ps;	
	char *str,*end;
	char buf[30];
	//char mask[2];
	int i;
	//int netmask;
	str = str_s;
	end = end_s;
	ps=strstr(str,key);
	if((ps==NULL)||(ps>end))
	{	return -1;
	}
	ps+=strlen(key);
	str = ps;
	end_ps=strstr(ps,"\n");
	if((end_ps==NULL)||(end_ps>end))
	{	return -2;
	}
	i =  end_ps-ps;
	if(i>sizeof(buf))
	{	return -3;
	}
	memset(buf,0,sizeof(buf));
	memcpy(buf,ps,end_ps-ps);	
	cls_no_asc(buf,strlen(buf));
	//printf("buf %s\n",buf);
	memcpy(value,buf,strlen(buf));

	return 0;
}

static int get_file_info(char *ver)
{
	char buf[2048];
	//int i;
	char *p_file_head,*p_file_end;
	memset(buf,0,sizeof(buf));
	if(read_file("/etc/kerversion.cnf",buf,sizeof(buf))<0)
	{	
		return -1;
	}
	if(get_config_file_head_end(buf,"[usb-share kernel private version]","end.",&p_file_head,&p_file_end)<0)
	{	return -2;
	}
	//////////////////////////////////////////////////////////////////////////
	if(get_config_data(p_file_head,p_file_end,"private_ver=",ver)<0)
	{	return -3;
	}
	return 0;

}

static int get_private_version(char *ver,int hard_type)
{
	int result;
	//printf("先判断协议版本文件是否存在\n");
	//out("判断是否为A33版本");
	if ((hard_type == 1) || (hard_type == 3))
	{	//printf("a33 version\n");
		result = get_file_info(ver);
		if(result < 0)
		{	result = access("/sys/bus/usb/drivers/usbhid",0);
			if(result == 0)
			{	printf("/sys/bus/usb/drivers/usbhid exsit\n");
				printf("usbhid driver is exsit ,without update\n");
				return -1;
			}
			write_file("/etc/kerversion.cnf","[usb-share kernel private version]\nprivate_ver=v1.1\nend.\n",sizeof("[usb-share kernel private version]\nprivate_ver = v1.1\nend.\n"));	
			sync();
			result = get_file_info(ver);
			return result;
		}
		return result;
	}
	return -1;
}

int get_kernel_vision(char *ker,int *kver1,int hard_type)
{
    FILE *fp;
	char str[30] = {0};
	char kver[30] = { 0 };
	char ver[30] = { 0 };
	int result ;
    system ( "uname -r > /tmp/123" );
    fp = fopen ( "/tmp/123", "rb" );
    fgets ( str, 30, fp );
    fclose(fp);
    //printf ( "%s\n", str );
	//sprintf(kver,"%s",str);
	memset(ver, 0, sizeof(ver));
	memcpy(kver,str,strlen(str)-1);
	memcpy(ker, str, strlen(str) - 1);
	result = get_private_version(ver,hard_type);
	if(result >=0)
	{	//printf("ver %s\n\n\n\n\n\n\n\n\n",ver);
		//sprintf(ker,"%s-%s",kver,ver);
		//printf("kernel version %s\n",ker);
		*kver1=ver[3]-0x30;
		return 0;
	}
	//memcpy(ker,kver,30);
	printf("kernel version %s\n",ker);
    return 0;
}
