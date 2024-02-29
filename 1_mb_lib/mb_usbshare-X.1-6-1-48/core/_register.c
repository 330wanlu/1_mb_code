#define _register_c
#include "_register.h"
int _register_add(const _so_note    *note)
{	struct _register  *stream;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	//out("添加register文件\n");
    memset(name,0,sizeof(name));
	stream=malloc(sizeof(struct _register));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _register));
	sprintf(name,"register");
	stream->note = (_so_note *)note;
	result = ty_file_add(DF_DEV_REGISTER, stream, name, (struct _file_fuc *)&register_fuc);
	if(result<0)
		free(stream);

	reg_stream_fd = stream;
	//out("添加socket client文件成功\n");
	return result;
}

static int _register_open(struct ty_file	*file)
{   struct _register  *stream;
	//int result;
    int j;
    //out("打开文件:register\n");
    stream=file->pro_data;
	if(stream==NULL)
		return DF_ERR_PORT_CORE_FD_NULL;        
	if(stream->state==0)
    {   //out("申请一个数据锁\n");
        stream->lock=_lock_open("_register.c",NULL);
        if(stream->lock<0)
        {   out("申请锁失败\n");
            return stream->lock;
        }
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


static int _register_ctl(void *data, int fd, int cm, va_list args)
{
	struct _register        *stream;
	struct _register_fd     *id;
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
	for (i = 0; i<sizeof(ctl_fun) / sizeof(ctl_fun[0]); i++)
	{
		if (cm == ctl_fun[i].cm)
			return ctl_fun[i].ctl(id, args);
	}
	return DF_ERR_PORT_CORE_CM;
}



int data_3des_base(uint8 *in_data, uint8 *out_data)
{
	uint8 key[16];
	uint8 buf[1024];
	uint8 base_buf[1024];
	int base_len;
	int buf_len;
	int buyu;
	//int i;
	key[0] = 'z'; key[1] = 'h'; key[2] = 'a'; key[3] = 'n'; key[4] = 'g'; key[5] = 'x'; key[6] = 'i'; key[7] = 'n';
	key[8] = 'y'; key[9] = 'u'; key[10] = 'a'; key[11] = 'n'; key[12] = 'h'; key[13] = 'u'; key[14] = 'i'; key[15] = 'j';
	memset(buf, 0x01, sizeof(buf));
	buf_len = strlen((const char *)in_data);
	//out("[data_des3_base:]buf_len = %d\n",buf_len);
	memcpy(buf, in_data, buf_len);
	buyu = buf_len % 8;
	//out("[data_des3_base:]buyu = %d\n",buyu);
	if (buyu > 0)
	{
		buf_len = buf_len + 8 - buyu;
	}
	//out("读到%d个字节及补齐\n",buf_len);
	//for(i=0;i<buf_len;i++)
	//{
	//	out("%02X ",buf[i]);
	//}
	//out("\n");
	des3_in(buf, buf_len, key);
	//out("加密数据：\n");
	//for(i=0;i<buf_len;i++)
	//{
	//	out("%02X",buf[i]);
	//}
	//out("\n");

	encode((char *)buf, buf_len, (char *)base_buf);
	base_len = strlen((const char *)base_buf);
	//out("base64datalen=%d\n",base_len);
	//out("base64data:   %s\n",base_buf);
	memcpy(out_data, base_buf, base_len);
	//out_data=base_buf;
	//out("加密数据完成\n");
	return base_len;
}

int get_reg_err_info(char *err_info)
{
	struct _register *stream;
	stream = (struct _register *)reg_stream_fd;
	memcpy(err_info, stream->reg_err_info, strlen(stream->reg_err_info));
	return 0;
}
static int _register_terminal_to_server(struct _register_fd *id, va_list args)
{
	struct _register *stream;
	uint8 *sev_id;
	uint8 *udid_num;
	uint8 *hard_ware;
	uint8 *soft_ware;
	uint8 baseudid[30];
	int result;
	stream = id->dev;
	out("机器编号\n");
	sev_id=va_arg(args,uint8 *);
	printf("sev_id:%s\n",sev_id);
	out("唯一标识符\n");
	udid_num=va_arg(args,uint8 *);
	printf("udid_num:%s\n",udid_num);
	out("硬件版本\n");
	hard_ware=va_arg(args,uint8 *);
	printf("hard_ware:%s\n",hard_ware);
	out("软件版本\n");
	soft_ware=va_arg(args,uint8 *);
	printf("soft_ware:%s\n",soft_ware);

	char passwd[50] = {0};
	data_3des_base((uint8 *)sev_id, (uint8 *)passwd);
			
	memset(baseudid,0,sizeof(baseudid));
	encode((char *)udid_num, 12, (char *)baseudid);

	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
	sprintf(parm.s_ip, "%s",DF_M_SERVER_ADDRESS);
	parm.s_port = DF_M_SERVER_PORT;
	sprintf(parm.appid, "%s", DF_M_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_M_SERVER_APPSECERT);

	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "ter_id", (char *)sev_id);
	cJSON_AddStringToObject(json, "ter_pwd", (char *)passwd);
	cJSON_AddStringToObject(json, "cpuid", (char *)baseudid);
	cJSON_AddStringToObject(json, "ter_type", (char *)stream->note->type);
	cJSON_AddStringToObject(json, "soft_verison", (char *)soft_ware);
	cJSON_AddStringToObject(json, "hard_version", (char *)hard_ware);
	char *g_buf;
	g_buf = cJSON_PrintUnformatted(json);
	out("注册查询组包数据：%s\n", g_buf);

	result = http_token_register_dev_to_server(&parm, g_buf, strlen(g_buf), stream->reg_err_info);
	cJSON_Delete(json);
	free(g_buf);
	if (result == 0)
	{
#ifndef MTK_OPENWRT
		char tmp_buf[1024] = { 0 };
		sprintf(tmp_buf, "mqtt_server = %s\r\nmqtt_port = %d\r\nmqtt_user = %s\r\nmqtt_passwd = %s\r\n", "www.mengbaiservice.com", 18883, sev_id, passwd);
		write_file("/etc/mqtt_tmp.conf", tmp_buf, strlen(tmp_buf));
		delete_file(DF_TER_MAC_ADDR_FILE);
		sync();
#endif
	}
	return result;		
}
static int _get_mqtt_server_info_from_local(struct _register_fd *id, va_list args)
{
	char passwd[50];
	char user[20];
	char *ter_id;
	char *ter_passwd;
	ter_id = va_arg(args, char *);
	ter_passwd = va_arg(args, char *);
			
	memset(passwd, 0, sizeof(passwd));
	memset(user, 0, sizeof(user));
	memcpy(user, ter_id, 12);
	//out("通过加密算法获取出密码\n");
	data_3des_base((uint8 *)user, (uint8 *)passwd);
	//out("计算出的密码为%s\n", passwd);
	memcpy(ter_passwd, passwd, strlen(passwd));
	return 0;
}
static int _get_mqtt_server_info_from_server(struct _register_fd *id, va_list args)
{
	out("机器编号\n");
	char *ter_id;
	char s_data[1024];
	char *server_addr;
	int *port;
	char *passwd;
	int result;
	char r_buf[4096] = {0};
	int r_len = sizeof(r_buf);
	memset(s_data,0,sizeof(s_data));
	ter_id = va_arg(args, char *);
	server_addr = va_arg(args, char *);
	port = va_arg(args, int *);
	passwd = va_arg(args, char *);
	out("组json数据包\n");
	data_3des_base((uint8 *)ter_id, (uint8 *)passwd);
	struct _http_comon parm;
	memset(&parm, 0, sizeof(struct _http_comon));
	sprintf(parm.s_ip, "%s", DF_M_SERVER_ADDRESS);
	parm.s_port = DF_M_SERVER_PORT;
	sprintf(parm.appid, "%s", DF_M_SERVER_APPID);
	sprintf(parm.appsecret, "%s", DF_M_SERVER_APPSECERT);


	sprintf((char*)parm.timestamp, "%ld", get_time_sec());

	cJSON *json = cJSON_CreateObject();
	cJSON_AddStringToObject(json, "ter_id", (char *)ter_id);
	char *g_buf;
	g_buf = cJSON_PrintUnformatted(json);
	out("注册查询组包数据：%s\n", g_buf);

	result = http_token_get_mqtt_server_info(&parm, g_buf, strlen(g_buf),r_buf,&r_len);
	
	free(g_buf);
	
	//out("netPost收到的数据%s\n", r_buf);
	result = analysis_deal_respone_data((char *)r_buf, server_addr, port);
	if (result < 0)
	{
		out("解析应答数据失败,result = %d\n",result);
		return result;
	}
	return 0;
}

static int analysis_deal_respone_data(char *data, char *server_addr, int *port)
{

	char protocol[50];
	char code_type[10];
	char cmd[50];
	char result_s[2];
	char mqtt_server[200];
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
	cJSON *arrayItem = cJSON_GetObjectItem(root, "response");
	if (arrayItem == NULL)
	{
		out("Error before: [%s]\n", cJSON_GetErrorPtr());
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(arrayItem, "server_addr");
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
	item = cJSON_GetObjectItem(arrayItem, "server_port");
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
	item = cJSON_GetObjectItem(arrayItem, "ca_crt");
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

	item = cJSON_GetObjectItem(arrayItem, "client_crt");
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

	item = cJSON_GetObjectItem(arrayItem, "client_key");
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
	memset(tmp, 0, sizeof(tmp));
	
	if (get_file_base64("/etc/ca.crt", tmp, sizeof(tmp))<0)
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
	if (get_file_base64("/etc/client.crt", tmp, sizeof(tmp))<0)
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
	if (get_file_base64("/etc/client.key", tmp, sizeof(tmp))<0)
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
	cJSON_Delete(root);
	return 0;
}

static int get_file_base64(char *file, char *buf, int len)
{
	int file_len;
	char tmp[4000];
	memset(tmp, 0, sizeof(tmp));
	file_len = read_file(file, tmp, sizeof(tmp));
	encode((char *)tmp, file_len, (char *)buf);
	return 0;
}

static int save_cert_file(char *base_file, char *path)
{
	char buf[4000];
	char in_buf[4000];
	int  base_len;
	//int  buf_len;
	int i;
	int j;
	base_len = strlen(base_file);
	out("base64解码\n");
	memset(in_buf, 0, sizeof(in_buf));
	memcpy(in_buf, base_file, base_len);
	//out("要base64的数据长度为%d\n", base_len);
	////decode_string(in_data,buf);
	////base64_dec(buf,in_data,base_len);
	//for (i = 0; i<base_len; i++)
	//{
	//	printf("%02x ", in_buf[i]);
	//}
	//out("\n");
	for (i = 0, j = 0; i <= base_len; i = i + 4, j = j + 3)
	{
		decode((char *)in_buf + i, 4, (char *)buf + j);
	}
	//buf_len = (base_len / 4) * 3;
	//out("base64后的数据长度为%d\n", buf_len);
	out("BASE64后数据：%s\n",buf);
	//for (i = 0; i<buf_len; i++)
	//{
	//	out("%02X ", buf[i]);
	//}
	//out("\n");
	remove(path);
	FILE* f = fopen(path, "w+");
	if (f == NULL)
		return 0;
	fputs(buf, f);
	fclose(f);
	sync();
	return 0; 
}




static int _register_close(void *data,int fd)
{   struct _register 	*stream;
    struct _register_fd 	*id;
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
    {   
		out("关闭本模块\n");
        _lock_close(stream->lock);
        out("关闭成功\n");
    }    
    return 0;
}









