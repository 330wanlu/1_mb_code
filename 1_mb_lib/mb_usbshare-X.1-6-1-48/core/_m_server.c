#define _m_server_c
#include "_m_server.h"
int _m_server_add()
{   struct _m_server  *stream;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	//out("添加socket sev文件\n");
    memset(name,0,sizeof(name));
	stream=malloc(sizeof(struct _m_server));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _m_server));
	_mfd_stream = stream;
	sprintf(name,"mq_sync");
	result = ty_file_add(DF_DEV_M_SERVER, stream, name, (struct _file_fuc *)&m_server_fuc);
	if(result<0)
		free(stream);
	return result;   
}

static int _m_server_open(struct ty_file	*file)
{   struct _m_server    *stream;
	//int result;
    //char name[DF_FILE_NAME_MAX_LEN];
    int j;
    stream=file->pro_data;
	if(stream==NULL)
		return DF_ERR_PORT_CORE_FD_NULL;	
    if(stream->state==0)
    {   
        stream->lock=_lock_open("m_server",NULL);
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
    out("打开_m_server_open成功:j=%d\n",j);
	return j+1;
}

static int message_anwser_queue(struct _m_server     *stream, struct mqtt_parm	*parm,char *data)
{
	int i;
	_lock_set(stream->lock);
	for (i = 0; i < DF_MESSAGE_LINE_NUM; i++)
	{
		//out("第%d片内存状态%d,流水号%s\n", i, stream->m_line[i].state, stream->m_line[i].qqlsh);
		if (stream->m_line[i].state != 1)
		{
			continue;
		}
		if (strcmp(parm->random, stream->m_line[i].qqlsh) == 0)
		{
			//out("匹配到相应的请求流水数据\n");
			stream->m_line[i].answer_flag = 1;
			stream->m_line[i].answer_data = calloc(1, strlen(data) + 100);
			memcpy(stream->m_line[i].answer_data, data, strlen(data));
			//out("服务器返回结果数据为%s\n", parm->result);
			if (strcmp(parm->result, "s") == 0)
			{
				stream->m_line[i].answer_result = 1;
			}
			else
			{
				stream->m_line[i].answer_result = 0;
			}
			_lock_un(stream->lock);
			return 0;
		}
	
	}
	//out("未匹配到相应的流水号信息直接丢弃消息,请求流水号为%s\n", parm->random);
	_lock_un(stream->lock);
	return 0;
}

int message_callback_deal_answer(struct mqtt_parm	*parm,char *message)
{
	struct _m_server     *stream;
	stream = _mfd_stream;
	//out("message %s\n", (char *)message);
	//out("json数据解析完成\n");
	message_anwser_queue(stream, parm, message);

	//out("应答消息处理完成\n");
	return 0;
}


static int _m_server_ctl(void *data, int fd, int cm, va_list args)
{
	struct _m_server        *stream;
	struct _m_server_fd     *id;
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

static int _m_server_close(void *data,int fd)
{   struct _m_server    *stream;
    struct _m_server_fd   *id;
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
    if(stream->state==0)			//全部关闭了
	{   _lock_close(stream->lock);
	}
    return 0;   
}

static int _mqtt_client_message_pub(char *topic, char *sbuf, int try_num)
{
	int i;
	int result;
	for (i = 0; i < try_num; i++)
	{
		result = _mqtt_client_pub(topic, sbuf,0);
		if (result != 0)
		{
			usleep(500000);
			continue;
		}
		break;
	}
	if (i == try_num)
	{
		//out("发送%d次均失败\n", try_num);
		return -1;
	}
	return 0;
}

static int _mqtt_invoice_message_pub(char *topic, char *sbuf, int try_num)
{
	int i;
	int result;
	for (i = 0; i < try_num; i++)
	{
		result = _mqtt_invoice_pub(topic, sbuf,0);
		if (result != 0)
		{
			usleep(500000);
			continue;
		}
		break;
	}
	if (i == try_num)
	{
		//out("发送%d次均失败\n", try_num);
		return -1;
	}
	return 0;
}

int _m_pub_no_answer(char *topic, char *message)
{
	//struct _m_server  *stream;
	//stream = fd_stream;
	_mqtt_client_message_pub(topic, message,3);
	return 0;
}
int _m_pub_need_answer(char *qqlsh, char *topic, char *message,char *g_data,int mqtt_type)
{
	struct _m_server  *stream;
	int i;
	int result = -1;
	stream = _mfd_stream;
	int mem_i;
	_lock_set(stream->lock);
	if (mqtt_type == DF_MQTT_FPKJXT)
		result = _mqtt_client_message_pub(topic, message, 3);
	else
		result = _mqtt_invoice_message_pub(topic, message, 3);
	if (result < 0)
	{
		_lock_un(stream->lock);
		return -1;
	}

	mem_i = find_available_queue_mem(stream);
	if (mem_i < 0)
	{
		out("无消息队列\n");
		_lock_un(stream->lock);
		return -1;
	}
	
	stream->m_line[mem_i].state = 1;
	memset(stream->m_line[mem_i].qqlsh, 0, sizeof(stream->m_line[mem_i].qqlsh));
	memcpy(stream->m_line[mem_i].qqlsh, qqlsh, strlen(qqlsh));
	stream->m_line[mem_i].answer_flag = 0;
	stream->m_line[mem_i].answer_data = NULL;
	//out("获取到第%d个消息队列可用,流水号%s\n", mem_i, stream->m_line[mem_i].qqlsh);
	_lock_un(stream->lock);
	for (i = 0; i < 1000; i++)
	{
		usleep(60000);
		if (stream->m_line[mem_i].answer_flag == 1)
		{
			//out("消息应答成功\n");
			_lock_set(stream->lock);
			//out("加锁操作数据\n");
			memcpy(g_data, stream->m_line[mem_i].answer_data, strlen(stream->m_line[mem_i].answer_data));
			result = stream->m_line[mem_i].answer_result;
			if (stream->m_line[mem_i].answer_data != NULL)
				free(stream->m_line[mem_i].answer_data);
			stream->m_line[mem_i].answer_data = NULL;
			memset(stream->m_line[mem_i].qqlsh, 0, sizeof(stream->m_line[mem_i].qqlsh));
			stream->m_line[mem_i].state = 0;
			stream->m_line[mem_i].answer_flag = 0;
			stream->m_line[mem_i].answer_result = 0;
			_lock_un(stream->lock);
			//out("解锁返回，result = %d\n",result);
			return result;
		}
	}
	//out("超时未收到消息应答\n");
	_lock_set(stream->lock);
	//memcpy(g_data, stream->m_line[mem_i].answer_data, strlen(stream->m_line[mem_i].answer_data));
	if (stream->m_line[mem_i].answer_data != NULL)
		free(stream->m_line[mem_i].answer_data);
	stream->m_line[mem_i].answer_data = NULL;
	memset(stream->m_line[mem_i].qqlsh, 0, sizeof(stream->m_line[mem_i].qqlsh));
	stream->m_line[mem_i].state = 0;
	stream->m_line[mem_i].answer_flag = 0;
	stream->m_line[mem_i].answer_result = 0;
	_lock_un(stream->lock);
	//out("消息发送处理完成，结果为失败\n");
	return -1;

}


//查找可用消息队列号
static int find_available_queue_mem(struct _m_server     *stream)
{
	int mem_i = -1;
	int i;
	for (i = 0; i < DF_MESSAGE_LINE_NUM; i++)
	{
		if (stream->m_line[i].state == 0)
		{
			mem_i = i;
			break;
		}
	}
	if (i == DF_MESSAGE_LINE_NUM)
	{
		out("无可以消息队列");
		return -1;
	}
	
	return mem_i;
}