#define _ty_m_s_order_c
#include "_ty_m_s_order.h"
/*===============================pack up data===================================*/
int ty_m_s_pack(int order,uint8 *buf,va_list *args)
{	uint8 *pbuf,*pbuf1;
	int i;
	int len,buf_len;	
	for(i=0;i<sizeof(s_order_pack)/sizeof(struct s_order);i++)
	{	if(order==s_order_pack[i].order)
			break;
	}
	if(i==sizeof(s_order_pack)/sizeof(struct s_order))		//没有这个命令
	{	printf("没有这个命令,order=%d\n",order);
		return -2;
	}	
	out("\r\nty_m_s_pack in \n");
	pbuf=s_order_pack[i].pack(args,&len);
	if(pbuf==NULL)
		return DF_ERR_MEM_ERR;
	memcpy(buf,pbuf,len);
	mem_free(pbuf);
	return len;
}


/*===============================动作类函数=======================================*/
	static uint8 *s_order_soft_infor(va_list *args,int *len)
	{	_so_note	*note;
		uint8 *pbuf;
		int s_len;
		note=va_arg(*args,_so_note *);
		(*len)=strlen(note->name)+1+strlen(note->r_date)+1+strlen(note->ver)+1+
			 strlen(note->writer)+1+strlen(note->note)+1+strlen(note->code)+1;
		pbuf=mem_malloc(*len);
		if(pbuf==NULL)
			return NULL;
		memset(pbuf,0,*len);
		memcpy(pbuf,note->name,strlen(note->name));
		s_len=(strlen(note->name)+1);						//本软件名称
		memcpy(pbuf+s_len,note->r_date,strlen(note->r_date));
		s_len+=(strlen(note->r_date)+1);					//本软件发布日期
		memcpy(pbuf+s_len,note->ver,strlen(note->ver));
		s_len+=(strlen(note->ver)+1);						//本软件版本
		memcpy(pbuf+s_len,note->writer,strlen(note->writer));	
		s_len+=(strlen(note->writer)+1);					//本软件作者
		memcpy(pbuf+s_len,note->note,strlen(note->note));
		s_len+=(strlen(note->note)+1);						//发布备注
		memcpy(pbuf+s_len,note->code,strlen(note->code));
		s_len+=(strlen(note->code)+1);						//采用的编码方式
		return pbuf;
	}

	static uint8 *s_order_soft_err(va_list *args,int *len)
	{	int err;
		char *s;
		uint8 *pbuf;
		err=va_arg(*args,int);
		s=va_arg(*args,char *);
		(*len)=(sizeof(err)+strlen(s)+1);
		pbuf=mem_malloc(*len);
		if(pbuf==NULL)
			return NULL;
		memset(pbuf,0,*len);
		memcpy(pbuf,&err,sizeof(err));
		memcpy(pbuf+sizeof(err),s,strlen(s));
		return pbuf;
		
	}

	static uint8 *s_order_soft_log(va_list *args,int *len)
	{	char *s;
		uint8 *pbuf;
		s=va_arg(*args,char *);
		*len=(strlen(s)+1);
		pbuf=mem_malloc(*len);
		if(pbuf==NULL)
			return NULL;
		memcpy(pbuf,s,*len);
		return pbuf;
	}

	static uint8 *s_order_read_buf(va_list *args,int *len)
	{	uint8 *buf;
		//int fd;					//2015-01-08增加
		int buf_len;
		uint8 *pbuf;
		//fd=va_arg(*args,int );
		buf=va_arg(*args,uint8 *);
		buf_len=va_arg(*args,int );
		pbuf=mem_malloc(sizeof(buf)+sizeof(buf_len));
		if(pbuf==NULL)
			return NULL;
		//memcpy(pbuf,&fd,sizeof(fd));
		memcpy(pbuf,&buf,sizeof(buf));
		memcpy(pbuf+sizeof(buf),&buf_len,sizeof(buf_len));
		*len=(sizeof(buf)+sizeof(buf_len));
		return pbuf;
	}

	static uint8 *s_order_write_buf(va_list *args,int *len)
	{	uint8 *buf;
		//int fd;
		int buf_len;
		uint8 *cut;
		uint8 *pbuf;
		//fd=va_arg(*args,int );
		buf=va_arg(*args,uint8 *);
		buf_len=va_arg(*args,int );
		cut=va_arg(*args,uint8 *);
		pbuf=mem_malloc(sizeof(buf)+sizeof(buf_len)+buf_len);
		if(pbuf==NULL)
			return NULL;
		//memcpy(pbuf,&fd,sizeof(fd));
		memcpy(pbuf,&buf,sizeof(buf));
		memcpy(pbuf+sizeof(buf),&buf_len,sizeof(buf_len));
		memcpy(pbuf+sizeof(buf)+sizeof(buf_len),cut,buf_len);
		*len=(sizeof(buf)+sizeof(buf_len)+buf_len);
		return pbuf;
	}

	static uint8 *s_order_hook_ctl(va_list *args,int *len)
	{	//int fd;
		void *hook;
		void *arg;
		uint8 *buf;
		//fd=va_arg(*args,int );
		hook=va_arg(*args,void *);
		arg=va_arg(*args,void *);
		buf=mem_malloc(sizeof(hook)+sizeof(arg));
		if(buf==NULL)
			return NULL;
		//memcpy(buf,&fd,sizeof(fd));
		memcpy(buf,&hook,sizeof(hook));
		memcpy(buf+sizeof(hook),&arg,sizeof(arg));
		*len=sizeof(hook)+sizeof(arg);
		return buf;
	}
	
	static uint8 *s_order_state(va_list *args,int *len)
	{	int state;
		uint8 *buf;
		out("打包state\n");
		state=va_arg(*args,int);
		out("state=%d",state);
		buf=mem_malloc(sizeof(state));
		if(buf==NULL)
		{	out("内存分配失败\n");
			return NULL;
		}	
		memcpy(buf,&state,sizeof(state));
		*len=sizeof(state);
		out("buf\n",buf);
		return buf;
	}

/*=============================文件操作类函数======================================*/
	static uint8 *s_order_new_dev(va_list *args,int *len)
	{	char *buf;
		char *pbuf;
		int pro;
		pro=va_arg(*args,int );
		buf=va_arg(*args,char *);
		out("注册文件:pro=%d,name=%s\n",pro,buf);
		*len=(strlen(buf)+1+sizeof(pro));
		pbuf=mem_malloc(*len);
		if(pbuf==NULL)
			return NULL;
		memset(pbuf,0,*len);
		memcpy(pbuf,&pro,sizeof(pro));
		memcpy(pbuf+4,buf,strlen(buf));
		out("注册文件:pbuf=%d,len=%d\n",pbuf,*len);
		return pbuf;
	}	

	static uint8 *s_order_del_dev(va_list *args,int *len)
	{	char *buf;
		char *pbuf;
		buf=va_arg(*args,char *);
		*len=(strlen(buf)+1);
		pbuf=mem_malloc(*len);
		if(pbuf==NULL)
			return NULL;
		memset(pbuf,0,*len);
		memcpy(pbuf,buf,*len);
		return pbuf;
	}

	static uint8 *s_order_open(va_list *args,int *len)
	{	char *buf;
		char *name;
		buf=va_arg(*args,char *);
		*len=(strlen(buf)+1);
		name=mem_malloc(*len);
		if(name==NULL)
			return NULL;
		memset(name,0,*len);
		memcpy(name,buf,*len);
		return name;
	}

	static uint8 *s_order_read(va_list *args,int *len)
	{	int fd,buf_len;
		uint8 *buf;
		uint8 *pbuf;
		fd=va_arg(*args,int );
		buf=va_arg(*args,uint8 *);
		buf_len=va_arg(*args,int );
		pbuf=mem_malloc(sizeof(int )+sizeof(uint8 *)+sizeof(int ));
		if(pbuf==NULL)
			return NULL;
		memcpy(pbuf,&fd,sizeof(fd));
		memcpy(pbuf+sizeof(fd),&buf,sizeof(buf));
		memcpy(pbuf+sizeof(fd)+sizeof(buf),&buf_len,sizeof(buf_len));
		*len=(sizeof(int )+sizeof(uint8 *)+sizeof(int ));
		return pbuf;
	}

	static uint8 *s_order_write(va_list *args,int *len)
	{	int fd,buf_len;
		uint8 *buf;
		uint8 *pbuf;
		fd=va_arg(*args,int );
		buf=va_arg(*args,uint8 *);
		buf_len=va_arg(*args,int );
		pbuf=mem_malloc(sizeof(int )+sizeof(uint8 *)+sizeof(int ));
		if(pbuf==NULL)
			return NULL;
		memcpy(pbuf,&fd,sizeof(fd));
		memcpy(pbuf+sizeof(fd),&buf,sizeof(buf));
		memcpy(pbuf+sizeof(fd)+sizeof(buf),&buf_len,sizeof(buf_len));
		*len=(sizeof(int )+sizeof(uint8 *)+sizeof(int ));
		return pbuf;
	}

	static uint8 *s_order_ctl(va_list *args,int *len)
	{	int fd;
		int cm,i;
		int par[DF_TY_MAX_PAR];
		uint8 *buf;
		fd=va_arg(*args,int );
		cm=va_arg(*args,int );
		for(i=0;i<DF_TY_MAX_PAR;i++)
		{	par[i]=va_arg(*args,int );
		}
		buf=mem_malloc(sizeof(fd)+sizeof(cm)+sizeof(par));
		if(buf==NULL)
			return NULL;
		memcpy(buf,&fd,sizeof(fd));
		memcpy(buf+sizeof(fd),&cm,sizeof(cm));
		memcpy(buf+sizeof(fd)+sizeof(cm),par,sizeof(par));
		*len=(sizeof(fd)+sizeof(cm)+sizeof(par));
		return buf;
	}

	static uint8 *s_order_close(va_list *args,int *len)
	{	int fd;
		uint8 *buf;
		fd=va_arg(*args,int );
		buf=mem_malloc(sizeof(fd ));
		if(buf==NULL)
			return NULL;
		memcpy(buf,&fd,sizeof(fd));
		*len=sizeof(fd);
		return buf;
	}





