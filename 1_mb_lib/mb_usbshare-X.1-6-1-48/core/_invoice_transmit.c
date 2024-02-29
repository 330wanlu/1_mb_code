#define _invoice_transmit_c
#include "_invoice_transmit.h"
int _invoice_transmit_add(void)
{   struct _invoice_transmit  *stream;
	int result;
	stream=malloc(sizeof(struct _invoice_transmit));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _invoice_transmit));
	result = ty_file_add(DF_DEV_INVOICE_TRANSMIT, stream, "invoice_transmit", (struct _file_fuc *)&invoice_transmit_fuc);
	if(result<0)
		free(stream);
	return result;    
}
static int invoice_transmit_open(struct ty_file	*file)
{   struct _invoice_transmit  *stream;
    int j;
	int result;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("��invoice_transmit_openģ��\n");
	if(stream->state==0)
    {   
		stream->ty_usb_fd = ty_open("/dev/ty_usb", 0, 0, NULL);
		if (stream->ty_usb_fd<0)
		{
			out("���ļ�[%s]ʧ��:[%d]\n", "/dev/ty_usb", stream->ty_usb_fd);
			return stream->ty_usb_fd;
		}
		ty_ctl(stream->ty_usb_fd, DF_TY_USB_CM_GET_MACHINE_ID, stream->ter_id);

		stream->mqtt_lock = _lock_open("invoice_transmit_open", NULL);
		invoice_transmit_fd = stream;


		stream->machine_fd = ty_open("/dev/machine", 0, 0, NULL);
		if (stream->machine_fd<0)
		{
			logout(INFO, "SYSTEM", "INIT", "���ն���Ϣģ��ʧ��\r\n");
			return -6;
		}


		memset(&invoice_transmit_message_callback, 0, sizeof(struct _invoice_transmit_message_callback));
		stream->task = _task_open("deal_message", task_deal_message_queue, stream, 1, NULL);
		if (stream->task < 0)
		{
			logout(INFO, "system", "init", "����Ϣ�����߳�ʧ��\n");
			return -15;
		}

		stream->get_net_fd = ty_open("/dev/get_net_state", 0, 0, NULL);
		
		//out("��ȡ��¼�õ�MQTT�û�����ɹ�,server %s,usr_name %s,passwd %s\n", stream->mqtt_server, stream->user_name, stream->user_passwd);

		result = ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER, stream->mqtt_server, &stream->mqtt_port, stream->user_name, stream->user_passwd);
		if (result < 0)
		{
			out("��ȡ��¼�õ�MQTT�û�����ʧ��\n");
		}
		else
		{
			out("��ȡ��¼�õ�MQTT�û�����ɹ�,server %s,port %d,usr_name %s,passwd %s\n", stream->mqtt_server, stream->mqtt_port, stream->user_name, stream->user_passwd);
			char id_sub[200] = { 0 };
			char id_pub[200] = { 0 };
			memset(id_sub, 0, sizeof(id_sub));
			memset(id_pub, 0, sizeof(id_pub));
			sprintf(stream->topic, "mb_fptbxt_%s", stream->user_name);
			sprintf(id_sub, "sub%s", stream->user_name);
			sprintf(id_pub, "pub%s", stream->user_name);
			_mqtt_invoice_init(sub_message_callback_function_sync, sub_err_callback_deal_function, stream->mqtt_server, stream->mqtt_port, stream->topic, stream->user_name, stream->user_passwd, id_sub, id_pub);
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
static int invoice_transmit_ctl(void *data,int fd,int cm,va_list args)
{   struct _invoice_transmit      *stream;
    struct _invoice_transmit_fd   *id;
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
    {   if(cm==ctl_fun[i].cm)
        {   _lock_set(stream->mqtt_lock);
            result=ctl_fun[i].ctl(id,args);
            _lock_un(stream->mqtt_lock);
            return result;
        }    
    }
    return DF_ERR_PORT_CORE_CM;
}

static int invoice_transmit_close(void *data,int fd)
{   struct _invoice_transmit      *stream;
    struct _invoice_transmit_fd   *id;
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
    {  
        _lock_close(stream->mqtt_lock);
    }
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
		out("��ȡԴ��Ϣ����ʧ��\n");
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

//����Ҫ�������Ϣ���к�
static int find_deal_queue_mem(struct _invoice_transmit  *stream)
{
	int wait_deal_num = 0;
	int i = 0;
	int mem_i;
	//out("���ҿ�����Ϣ����\n");
	if (invoice_transmit_message_callback.deal_num == INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM)
		invoice_transmit_message_callback.deal_num = 0;
	mem_i = invoice_transmit_message_callback.deal_num;
	if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].state == 0)
	{

		for (i = 0; i < INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM; i++)
		{
			if ((invoice_transmit_message_callback.invoice_transmit_message_line[i].state == 1) && (invoice_transmit_message_callback.invoice_transmit_message_line[i].inuse == 0))
				wait_deal_num += 1;
		}
		if (wait_deal_num >= 1)
		{
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ڴ�����������,��ǰ�������Ϊ��,�������������������Ҫ����,������ǰ�������\r\n");
			invoice_transmit_message_callback.deal_num += 1;
		}
		return -1;
	}
	if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse != 0)
	{
		//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ڴ�����������,�ϸ�ѭ�����ڴ���,������ǰ�������\r\n");
		invoice_transmit_message_callback.deal_num += 1;
		return -1;
	}
	invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse = 1;
	return mem_i;
}
//���ҿ�����Ϣ���к�
static int find_available_queue_mem(struct _invoice_transmit     *stream)
{
	int mem_i;
	//out("���ҿ�����Ϣ����\n");
	if (invoice_transmit_message_callback.insert_num == INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM)
		invoice_transmit_message_callback.insert_num = 0;
	mem_i = invoice_transmit_message_callback.insert_num;
	if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].state == 1)
	{
		if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse == 1)
		{
			//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ʱ,��ǰҪ����Ķ������Ϊ�ϴ�ѭ��δ���������,�жϺ������޿��ö���,��ǰ�����ڴ�ţ�%d\r\n", message_callback.insert_num);
			mem_i++;
			for (; mem_i < INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM; mem_i++)
			{
				if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].state == 0)
				{
					invoice_transmit_message_callback.insert_num = mem_i;
					//logout(INFO, "SYSTEM", "mqtt_queue", "��Ϣ���д�����ʱ,�ҵ������ڴ�ţ�%d\r\n", message_callback.insert_num);
					return mem_i;
				}
			}
		}
		return -1;
	}
	invoice_transmit_message_callback.insert_num += 1;
	return mem_i;
}
//MQTT�ͻ��˶����쳣����ص�
static void sub_err_callback_deal_function(struct _mqtt_parms  *mqtt_client, int errnum)
{
	struct _invoice_transmit *stream;
	stream = invoice_transmit_fd;
	logout(INFO, "SYSTEM", "mqtt_err", "���붩���쳣����ص�,�������Ϊ%d\r\n", errnum);

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
	default:
		break;
	}

	sleep(10);
	out("���ӷ������쳣,�û�����������֤�����\n");

	sleep(stream->connect_time);
	if ((stream->connect_time > INVOICE_TRANSMIT_DEAFULT_CONNECT_TIME * 20) || (stream->connect_time < INVOICE_TRANSMIT_DEAFULT_CONNECT_TIME))//����20���ӣ�������ʱ���ٴθ�Ϊ60��һ��
	{
		stream->connect_time = INVOICE_TRANSMIT_DEAFULT_CONNECT_TIME;
	}
	stream->connect_time += 10;
	return;

}


//MQTT�ͻ�����Ϣ���ջص�
static void sub_message_callback_function_sync(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct _invoice_transmit     *stream;
	struct mqtt_parm parm;
	int result;
	int mem_i;
	stream = invoice_transmit_fd;
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

	//logout(INFO, "MQTT", "mqtt_message", "��ˮ�ţ�%s,���յ���Ϣ���������\r\n", parm.random);
	mem_i = find_available_queue_mem(stream);
	if (mem_i < 0)
	{
		out("��Ϣ��������,�޷�����\n");
		logout(INFO, "MQTT", "mqtt_err", "��Ϣ%s,��Ϣ���������޷��洢,����������\r\n", parm.random);
		return;
	}
	invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message = malloc(strlen(message->payload) + 100);
	if (invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message == NULL)
	{
		out("�ڴ�����ʧ��,Ҫ������ڴ��СΪ%d,������%s\n", strlen(message->payload) + 100, DF_MQTT_SYNC_REPORT_TER_STATUS);
		return;
	}
	memset(&invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm, 0, sizeof(struct mqtt_parm));
	memcpy(&invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm, &parm, sizeof(struct mqtt_parm));
	memset(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message, 0, strlen(message->payload) + 100);
	memcpy(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message, message->payload, strlen(message->payload));
	invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse = 0;
	invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].state = 1;
	invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message_time = get_time_sec();
	//logout(INFO, "MQTT", "mqtt_message", "��ˮ�ţ�%s,�����%d����Ϣ����,��ǰ�����߳�����Ϊ%d��\r\n", parm.random, mem_i, message_callback.task_num);
	return;
}

void task_deal_message_queue(void *arg, int o_timer)
{
	struct _invoice_transmit *stream;
	//char name[200];
	//char plate_num[20];
	//char fplxdm[6];
	//char json_buf[1024*1024*2];
	int mem_i;
	//int result;
	stream = arg;
	if (invoice_transmit_message_callback.task_num > INVOICE_TRANSMIT_MAX_MESSAGE_QUEUE_NUM)
	{
		logout(INFO, "SYSTEM", "mqtt_err", "��ǰ����ִ�е���Ϣ�����������쳣,������������\r\n");
		sleep(5);
		sync();
		ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
		sleep(10);
		return;
	}

	//out("===============================================================\n���뵽��Ϣ�����߳�\n\n\n\n\n\n");
	if (invoice_transmit_message_callback.task_num > INVOICE_TRANSMIT_MAX_DEAL_MESSAGE_TASK)
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
	if (strcmp(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm.cmd, DF_MQTT_SYNC_REQUEST_CXDLXX) == 0)
	{
		logout(INFO, "mqtt", invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm.cmd, "��ˮ�ţ�%s,��ѯ��Ϣ������Ϣ\r\n", invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm.random);
		//deal_with_cmd(stream, mem_i, analysis_request_cxdlxx);
	}
	else
	{
		//out("�����ֲ�֧��,�ڴ����Ϊ%d\n",mem_i);
		deal_with_cmd(stream, mem_i, analysis_request_cmd_not_support);
	}


	return;
}

//��������������
static void  deal_with_cmd(struct _invoice_transmit *stream, int mem_i, void *fucntion)
{
	unsigned int time_now;
	time_now = get_time_sec();
	if ((time_now - invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message_time) > 120)
	{
		logout(INFO, "SYSTEM", "message_queue", "������Ϣ����ʱ���ֵ�ǰ��Ϣ�ѳ�ʱ,���ٴ������Ϣ\r\n");
		invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].state = 0;
		invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse = 0;
		memset(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].fpqqlsh, 0, sizeof(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].fpqqlsh));
		memset(&invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].parm, 0, sizeof(struct mqtt_parm));
		free(invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message);
		invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message = NULL;
		invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].message_time = 0;
		invoice_transmit_message_callback.deal_num += 1;
		return;
	}
	if (_queue_add("0", fucntion, &invoice_transmit_message_callback.invoice_transmit_message_line[mem_i], 0)<0)
	{
		invoice_transmit_message_callback.invoice_transmit_message_line[mem_i].inuse = 0;
		invoice_transmit_message_callback.err_count += 1;
		logout(INFO, "SYSTEM", "mqtt_err", "������Ϣ��������ʧ��,�Ժ��ٴδ���,���������%d\r\n", invoice_transmit_message_callback.err_count);
		if (invoice_transmit_message_callback.err_count > 100)
		{
			logout(INFO, "SYSTEM", "mqtt_err", "�����߳��쳣�ۻ�����100��,������������\r\n");
			sleep(5);
			sync();
			ty_ctl(stream->machine_fd, DF_MACHINE_INFOR_CM_RESET_POWER, DF_EVENT_REASON_NET);
			sleep(10);
		}
		return;
	}

	//logout(INFO, "SYSTEM", "message_queue", "�������������ɣ���ˮ�ţ�%s\r\n", message_callback.message_line[mem_i].parm.random);
	invoice_transmit_message_callback.err_count = 0;
	invoice_transmit_message_callback.deal_num += 1;
	invoice_transmit_message_callback.task_num += 1;
	return;
	//out("������Ϣ���гɹ�\n");
}

//�����������������쳣����
static void analysis_request_cmd_not_support(void *arg)
{
	struct _invoice_transmit     *stream;
	//int size;
	struct _invoice_transmit_message_line *message_line;
	message_line = arg;
	stream = invoice_transmit_fd;
	//cJSON *arrayItem, *item; //����ʹ��
	//cJSON *data_array;		 //���ʹ��
	//char errinfo[2048] = { 0 };

	//out("message_line->message = %s\n", message_line->message);
	//out("cmd  = %s\n", message_line->parm.cmd);
	//out("������%s���ⲿ����������,����Ϊ�ⲿӦ��������,����M����ģ�鴦��\n", message_line->parm.cmd);


	message_callback_deal_answer(&message_line->parm, message_line->message);

	//out("Mģ�鴦�����\n");

	free(message_line->message);
	//mqtt_response_errno(DF_TAX_ERR_CODE_DATA_PARSING_FAILED, &message_line->parm, errinfo, DF_MQ_QOS0);
	goto End;

End:
	_lock_set(stream->mqtt_lock);
	if (invoice_transmit_message_callback.task_num != 0)
		invoice_transmit_message_callback.task_num -= 1;
	message_line->state = 0;
	message_line->inuse = 0;
	memset(message_line->fpqqlsh, 0, sizeof(message_line->fpqqlsh));
	memset(&message_line->parm, 0, sizeof(struct mqtt_parm));
	message_line->message = NULL;
	_lock_un(stream->mqtt_lock);
	//out("�����ѯ˰��˰��ʱ�����\n");
	return;
}