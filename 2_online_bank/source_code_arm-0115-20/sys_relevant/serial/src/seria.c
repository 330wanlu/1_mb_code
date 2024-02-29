#include "../inc/seria.h"

static int seria_open(struct ty_file *file);
static int seria_read(void *data,int fd,uint8 *buf,int buf_len);
static int seria_write(void *data,int fd,uint8 *buf,int buf_len);
static int seria_ctl(void *data,int fd,int cm,va_list args);
static int seria_close(void *data,int fd);
static int seria_del(void *data,int c);
static void write_hook(struct _seria *data,uint8 *buf,int buf_len);

static const struct _file_fuc seria_fuc =
{
	.open=seria_open,
	.read=seria_read,
	.write=seria_write,
	.ctl=seria_ctl,
	.close=seria_close,
	.del=seria_del
};

int seria_add(char *name,const struct _seria_bsp_fuc *fuc,void *o_arg)
{
	struct _seria *seria = NULL;
	int result = -1;
	seria = (struct _seria *)malloc(sizeof(struct _seria));
	if(seria == NULL)
		return DF_ERR_MEM_ERR;
	memset(seria,0,sizeof(struct _seria));
	seria->fuc = (struct _seria_bsp_fuc	*)fuc;
	seria->write = write_hook;
	seria->o_arg = o_arg;
	result = ty_file_add(DF_DEV_TY_USART, seria, name, (struct _file_fuc *)&seria_fuc);
	printf("lbc seria_add\n");
	if(result < 0)
		free(seria);
	return result;
}

static int seria_del(void *data,int c)
{
	struct _seria *seria = NULL;
	int i = 0;
	c = c;
	seria = (struct _seria *)data;
	if((seria == NULL)||(seria->fuc == NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	for(i = 0;i < sizeof(seria->fd)/sizeof(struct _seria_fd);i++)
	{
		if(seria->fd[i].state == 0)
			continue;
		seria_close(data,i+1);
	}
	if(seria->fuc->del != NULL)
		seria->fuc->del(seria->o_arg);
	free(seria);
	return 0;
}

static int seria_open(struct ty_file *file)
{
	struct _seria *seria = NULL;
	int result = -1,j = 0;
	seria = (struct _seria *)file->pro_data;
	if((seria == NULL)||(seria->fuc == NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if(seria->state == 0)//第一次使用
	{
		if(seria->fuc->open==NULL)
		{
			return DF_ERR_PORT_CORE_OPEN_NULL;
		}
		result=seria->fuc->open(seria,seria->o_arg);
		if(result<0)
		{
			return result;
		}
		seria->a_buf = (uint8 *)malloc(DF_SERIA_A_BUF_MAX_LEN);
		memset(seria->a_buf, 0, DF_SERIA_A_BUF_MAX_LEN);
		seria->a_buf_len = DF_SERIA_A_BUF_MAX_LEN;
		seria->tail = 0;
		seria->bps = UART_BOUDS;
	}
	for(j = 0;j < sizeof(seria->fd)/sizeof(struct _seria_fd);j++)
	{
		if(seria->fd[j].state==0)
			break;
	}
	if(j == sizeof(seria->fd)/sizeof(struct _seria_fd))
	{
		return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&seria->fd[j],0,sizeof(struct _seria_fd));
	seria->fd[j].head = seria->tail;
	seria->fd[j].seria = seria;
	seria->fd[j].state = 1;
	seria->fd[j].mem_fd = &file->fd[j];
	seria->state++;
	return j+1;
}

static int seria_read(void *data,int fd,uint8 *buf,int buf_len)
{
	struct _seria *seria = NULL;
	struct _seria_fd *id = NULL;
	uint8 *pbuf = NULL;
	int n = 0;
	seria = (struct _seria *)data;
	if((seria == NULL)||(seria->fuc == NULL) ||(buf == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd == 0)||(fd > sizeof(seria->fd)/sizeof(struct _seria_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &seria->fd[fd];
	if(id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	pbuf = buf;
	if(id->mem_fd->ty_fd!=0)
	{
		pbuf = (uint8 *)malloc(buf_len);
		if(pbuf == NULL)
			return DF_ERR_MEM_ERR;
		memset(pbuf, 0, buf_len);
	}
	for(n = 0;n < buf_len;n++)
	{
		if(id->head == seria->tail)
			break;
		pbuf[n] = seria->a_buf[id->head++];
		if(id->head == seria->a_buf_len)
			id->head=0;
	}
	if(id->mem_fd->ty_fd != 0)
	{
		mem_write(id->mem_fd,pbuf,buf,n);
		free(pbuf);
	}
	return n;
}

static int seria_write(void *data,int fd,uint8 *buf,int buf_len)
{
	struct _seria *seria = NULL;
	struct _seria_fd *id = NULL;
	uint8 *pbuf = NULL;
	int result = -1;
	seria = (struct _seria *)data;
	if((seria == NULL)||(seria->fuc == NULL)||(buf == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd == 0)||(fd > sizeof(seria->fd)/sizeof(struct _seria_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &seria->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	pbuf = buf;
	if(id->mem_fd->ty_fd != 0)
	{
		pbuf = (uint8 *)malloc(buf_len);
		if(pbuf == NULL)
			return DF_ERR_MEM_ERR;
		memset(pbuf,0,buf_len);
		serial_out("获取外部缓存,id->mem=%d,buf_len\n",id->mem,buf_len);
		mem_read(id->mem_fd,pbuf,buf,buf_len);
	}
	result = seria->fuc->write(seria->bsp_data,pbuf,buf_len);
	if(id->mem_fd->ty_fd != 0)
	{
		serial_out("释放缓存\n");
		free(pbuf);
	}
	serial_out("返回:len=%d\n",result);
	return result;
}

static int seria_ctl(void *data,int fd,int cm,va_list args)
{
	struct _seria *seria = NULL;
	struct _seria_fd *id = NULL;
	int result = -1,bps = 0,tail = 0,head = 0;
	seria = (struct _seria *)data;
	if((seria == NULL)||(seria->fuc == NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd == 0)||(fd > sizeof(seria->fd)/sizeof(struct _seria_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &seria->fd[fd];
	if(id->state == 0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	if(cm == DF_SERIA_CM_HOOK)
	{
		id->arg=va_arg(args,void *);
		id->hook=va_arg(args,void *);
		serial_out("seria_ctl hook suc\n");
		return 0;
	}
	else if(cm == DF_SERIA_CM_BPS)
	{	
		if(seria->fuc->bps == NULL)
			return DF_ERR_PORT_CORE_CM_BPS;
		bps = va_arg(args,int);
		result = seria->fuc->bps(seria->bsp_data,bps);
		if(result >= 0)
			seria->bps = bps;
		serial_out("seria ctl bps=%d\n",bps);
		return result;
	}
	else if(cm == DF_SERIA_CM_CLS)
	{
		id->head = seria->tail;
		return 0;
	}
	else if(cm == DF_SERIA_CM_GET)
	{
		tail = seria->tail;
		head = id->head;
		if(tail >= head)
		{
			return tail-head;
		}	
		tail = tail+seria->a_buf_len-head;
		return tail;
	}
	return DF_ERR_PORT_CORE_CM;
}

static int seria_close(void *data,int fd)
{
	struct _seria *seria = NULL;
	struct _seria_fd *id = NULL;
	seria = (struct _seria *)data;
	if((seria == NULL)||(seria->fuc == NULL))
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd == 0)||(fd > sizeof(seria->fd)/sizeof(struct _seria_fd)))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id = &seria->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state = 0;
	id->arg = NULL;
	id->head = 0;
	id->hook = NULL; 	
	id->mem = NULL;
	seria->state--;
	memset(id,0,sizeof(struct _seria_fd));
	if(seria->state == 0)	//全部关闭了
	{
		seria->fuc->close(seria->bsp_data);
		seria->bsp_data=NULL;
		free(seria->a_buf);
		seria->a_buf_len=0;
		seria->tail=0;
	}
	return 0;
}

static void write_hook(struct _seria	*seria,uint8 *buf,int buf_len)
{
	int i = 0;
	void *arg = NULL;
	void (*hook)(void *arg) = NULL;
	for(i=0;i<buf_len;i++)
	{
		seria->a_buf[seria->tail++] = buf[i];
		if(seria->tail == seria->a_buf_len)
			seria->tail=0;
	}
	for(i = 0;i < sizeof(seria->fd)/sizeof(struct _seria_fd);i++)
	{
		if(seria->fd[i].state == 0)
			continue;
		arg = seria->fd[i].arg;
		hook = seria->fd[i].hook;
		if(seria->fd[i].mem_fd->ty_fd == 0)
		{
			if(hook != NULL)
			{
				hook(arg);
			}
		}
		else
		{
			mem_hook(seria->fd[i].mem_fd,seria->fd[i].hook,arg);
		}
	}
}
