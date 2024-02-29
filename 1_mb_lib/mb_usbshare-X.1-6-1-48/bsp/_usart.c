#define _usart_c
#include "_usart.h"
//int need_log;
void _linux_usart_load(int hard_type)
{	
	if (hard_type == 2)
	{
		seria_add("seria0",&us_fuc,"/dev/ttyS1");
	}
	else if ((hard_type == 1) || (hard_type == 3))
	{
		//printf("hard_type = %d\n", hard_type);
		seria_add("seria0", &us_fuc, "/dev/ttyS1");
		seria_add("seria1", &us_fuc, "/dev/ttyS2");
	}
	else
	{
		seria_add("seria0",&us_fuc,"/dev/ttyS4");
		seria_add("seria1",&us_fuc,"/dev/ttyS3");
	}
}

static int _usart_open(struct _seria	*data,void *o_arg)
{	struct _usart	*stream;
	int result;
	stream=malloc(sizeof(struct _usart));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _usart	));
	out("_usart open start\n");
	stream->fd=open(o_arg,O_RDWR|O_NOCTTY|O_NDELAY);
	if(stream->fd<0)
	{	out("linux usart open err=%d\n",stream->fd);
		result=stream->fd;
		free(stream);
		return result;
	}
	if((result=SetDevice(stream->fd,9600,8,1,'n'))<0)
	{	out("linux usart open bps err=%d\n",result);
		close(stream->fd);
		free(stream);
		return result;
	}
	out("creat task\n");
	stream->task=_task_open((char *)o_arg,_usart_hook,stream,1,NULL);
	if(stream->task<0)
	{	result=stream->task;
		out("linux usart open task err=%d\n",stream->task);
		close(stream->fd);
		free(stream);
		return result;
	}
	stream->seria=data;
	stream->write_data=data->write;
	data->bsp_data=stream;
	out("linux usart open suc \n");
	return 0;
}

static  int _usart_set_bps(void *bsp_data,int bps)
{	struct _usart	*stream;
	int result;	
	stream=bsp_data;
	out("_usart_set_bps bps=%d\n",bps);
	if((result=SetDevice(stream->fd,bps,8,1,'n'))<0)
	{	
		out("_usart_set_bps  err=%d\n",result);
		return result;
	}	
	out("_usart_set_bps suc\n");
	return 0;	
}



static void _usart_close(void *bsp_data)
{	struct _usart	*stream;
	stream=bsp_data;
	out("_usart_close in \n");
	_task_close(stream->task);
	close(stream->fd);
	free(stream);
	out("_usart_close suc\n");
	return ;	
}

static int _usart_write(void *bsp_data,uint8 *buf,int buf_len)
{	struct _usart	*stream;
	int result;
	stream=bsp_data;
	//out("_usart_write buf_len=%d\n",buf_len);
	//uint8 cmd = buf[4];
	//if ((cmd == 0x01) || //����
	//	//(cmd == 0x2c) || //ȡӲ����Ϣ
	//	(cmd == 0x2f) || //û����֪ͨ
	//	(cmd == 0x30) || //û����֪ͨ
	//	(cmd == 0x40) || //���Ƶ�
	//	//(cmd == 0x49) || //���Ʒ���
	//	//(cmd == 0x52) || //����
	//	//(cmd == 0x31) || //cpuid
	//	//(cmd == 0x29) || //��������
	//	//(cmd == 0x08) || //���������������
	//	//(cmd == 0x13) || //����Ⱥ�������������
	//	//(cmd == 0x18) || //����汾
	//	//(cmd == 0x14) || //������Ϣ
	//	//(cmd == 0x26) || //������hub
	//	//(cmd == 0x2b) || //��flash
	//	(cmd == 0x2a)) //дflash
	//{

	//	char log_data[1024] = {0};
	//	int i;
	//	for (i = 0; i < buf_len; i++)
	//	{
	//		if (strlen(log_data)>sizeof(log_data)-24)
	//			break;
	//		sprintf(log_data + strlen(log_data), "%02x ", buf[i]);
	//	}
	//	logout(INFO, "SYSTEM", "USART", "��Ƭ����������:%s\r\n", log_data);
	//	need_log = 1;
	//}


	result=write(stream->fd,buf,buf_len);
	if(result<0)
		return result;
	return buf_len;
}


static void _usart_hook(void *arg,int o_timer)
{	uint8 buf[512];
	struct _usart	*stream;
	int len;
	stream=arg;
	out("_usart_hook oper\n");
	for(;stream->write_data==NULL;usleep(1000));
	out("_usart_hook in\n");
	if(stream->aa!=0xaa)
	{	
		stream->aa=0xaa;
	}
	
	for(;;)
	{	len=read(stream->fd,buf,sizeof(buf));
		/*if (len != 0)
		{
			if (need_log == 1)
			{
				char log_data[1024] = { 0 };
				int i;
				for (i = 0; i < len; i++)
				{
					sprintf(log_data + strlen(log_data), "%02x ", buf[i]);
				}
				logout(INFO, "SYSTEM", "USART", "��Ƭ����������:%s\r\n", log_data);
				need_log = 0;
			}
		}*/
		if(len>0)
		{	stream->write_data(stream->seria,buf,len);
			continue;
		}
		if(len<0)
		{	
			perror("����Ϊ\n");

		}
		break;
	}
}

