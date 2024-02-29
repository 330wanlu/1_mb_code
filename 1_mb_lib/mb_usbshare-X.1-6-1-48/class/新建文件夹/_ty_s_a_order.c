#define _ty_s_a_order_c
#include "_ty_s_a_order.h"
int ty_s_a_order(struct _ty_core	*core,int order,uint8 *in_buf,uint8 **out_buf,int in_len,
				int *out_len)
{	int i;
	int result;
	*out_len=0;
	out("执行服务命令:\r\n");
	if(order==DF_ORDER_ACTION_READ_BUF)
	{	out("从本地BUF都内容\n");
		return soft_read_buf(core,in_buf,out_buf,in_len,out_len);
	}	
	for(i=0;i<sizeof(s_a_order_buf)/sizeof(no_buf);i++)
	{	if(s_a_order_buf[i].order==order)
		{	out("执行[%s]\n",s_a_order_buf[i].name);
			out("core->mem=%d\n",core->mem);
			result=s_a_order_buf[i].oper(core,in_buf,in_len);
			out("命令返回,result=%d,order=%d\n",result,order);
			return result;
		}	
	}
	return DF_ERR_CLASS_NO_ORDER;

}	
/*===============================动作类函数=======================================*/
	static int soft_infor(struct _ty_core	*core,uint8 *buf,int len)		//获取应用信息
	{	uint8 *pbuf;
		_so_note		*note;
		note=&core->note;
		pbuf=buf;
		len=strlen(pbuf);
		note->name=mem_malloc(len+1);
		memset(note->name,0,len+1);
		memcpy(note->name,pbuf,len);
		pbuf=(pbuf+len+1);
		
		len=strlen(pbuf);
		note->r_date=mem_malloc(len+1);
		memset(note->r_date,0,len+1);
		memcpy(note->r_date,pbuf,len);
		pbuf=(pbuf+len+1);
		
		len=strlen(pbuf);
		note->ver=mem_malloc(len+1);
		memset(note->ver,0,len+1);
		memcpy(note->ver,pbuf,len);
		pbuf=(pbuf+len+1);
		
		len=strlen(pbuf);
		note->writer=mem_malloc(len+1);
		memset(note->writer,0,len+1);
		memcpy(note->writer,pbuf,len);
		pbuf=(pbuf+len+1);
		
		len=strlen(pbuf);
		note->note=mem_malloc(len+1);
		memset(note->note,0,len+1);
		memcpy(note->note,pbuf,len);
		pbuf=(pbuf+len+1);
		
		len=strlen(pbuf);
		note->code=mem_malloc(len+1);
		memset(note->code,0,len+1);
		memcpy(note->code,pbuf,len);
		
		out("note->name=[%s]\n",note->name);
		out("note->r_date=[%s]\n",note->r_date);
		out("note->ver=[%s]\n",note->ver);
		out("note->note=[%s]\n",note->note);
		out("note->code=[%s]\n",note->code);
		return 0;	
	}
	static int soft_err(struct _ty_core	*core,uint8 *buf,int len)  //获取错误代码
	{	int err;
		char e_err[100];
		memset(e_err,0,sizeof(e_err));
		memcpy(&err,buf,sizeof(err));
		memcpy(e_err,buf+sizeof(err),len-sizeof(err));
		out("err->number=%d\n",err);
		out("err->buf=[%s]\n",e_err);
		return 0;
	}

	static int soft_log(struct _ty_core	*core,uint8 *buf,int len)
	{	uint8 cut[2048];
		memset(cut,0,sizeof(cut));
		sprintf(cut,"【%s】:%s",core->dev_name,buf);
		log_out(core->dev_name,cut);
		return 0;
	}
	
	static int soft_read_buf(struct _ty_core	*core,uint8 *buf,uint8 **o_buf,
								int in_buf_len,int *o_buf_len)
	{	int i;
		core=core;
		//memcpy(&fd,buf,sizeof(int));
		memcpy(o_buf,buf,sizeof(int));
		memcpy(o_buf_len,buf+sizeof(int),sizeof(int));
		out("read_buf,buf=%d,buf_len=%d\n",*o_buf,*o_buf_len);
		for(i=0;i<(*o_buf_len);i++)
		{	out("%2x\n",(*o_buf)[i]);
		}
		return *o_buf_len;
	}
	
	static int soft_write_buf(struct _ty_core	*core,uint8 *buf,int len)
	{	uint8 *w_buf;
		int w_buf_len;
		//int fd;
		//memcpy(&fd,buf,sizeof(fd));
		memcpy(&w_buf,buf,sizeof(w_buf));
		memcpy(&w_buf_len,buf+sizeof(w_buf),sizeof(w_buf_len));
		memcpy(w_buf,buf+sizeof(w_buf)+sizeof(w_buf_len),w_buf_len);
		return w_buf_len;
	}
	
	static int soft_hook_ctl(struct _ty_core	*core,uint8 *buf,int len)
	{	void *arg;
		void (*hook)(void *arg);
		//memcpy(&fd,buf,sizeof(fd));
		memcpy(&hook,buf,sizeof(hook));
		memcpy(&arg,buf+sizeof(hook),sizeof(arg));
		
		out("回调:hook=%d,arg=%d\n",hook,arg);
		if(hook!=NULL)
			hook(arg);
		return 0;
	}
	
	static int soft_state(struct _ty_core *core,uint8 *buf,int len)
	{	int state;
		memcpy(&core->s_state,buf,sizeof(core->s_state));
		out("信息状态:%d\n",core->s_state);
		if(core->s_state==DF_SOFT_STATE_OVER)		//该通道已经初始化完成
		{	if(core->m_state==DF_SOFT_STATE_OVER)
				return 0;
			if(core->self_m!=NULL)
				core->self_m(core);
			//_queue_add("state",core->self_m,core,1);
		}
		return 0;
	}
/*================================文件系统=========================================*/
	static int new_dev(struct _ty_core	*core,uint8 *buf,int len)
	{	struct _ty_core_add_file		*file;
		struct _ty_core_add_file		*o_file;
		struct _ty_file_add_in			*file_creat;
		int i,result;
		file=mem_malloc(sizeof(struct _ty_core_add_file));
		if(file==NULL)
			return DF_ERR_MEM_ERR;
		memset(file,0,sizeof(struct _ty_core_add_file));
		memcpy(&file->pro,buf,sizeof(file->pro));
		file->core=core;
		
		sprintf(file->self_name,"%s/%s",core->dev_name,buf+sizeof(file->pro));
		sprintf(file->name,"%s",buf+sizeof(file->pro));
		file_creat=core->file_add;
		if(file_creat==NULL)
		{	result=ty_file_core_add(file->pro,file->name,core->ty_m_fd);
			if(result<0)
			{	mem_free(file);
				return result;	
			}
		}
		else 
		{	for(i=0;i<file_creat->n;i++)
			{	if(file_creat->file_add[i].pro==file->pro)
				{	out("执行文件登记\n");
					result=file_creat->file_add[i].add(file->name,file);
					break;
				}
			}
			if(i==file_creat->n)
			{	result=ty_file_core_add(file->pro,file->self_name,core->ty_m_fd);
			}
			if(result<0)
			{	mem_free(file);
				return result;	
			}
		}
		if(core->add_file==NULL)
			core->add_file=file;
		else
		{	for(o_file=core->add_file;o_file->next!=NULL;o_file=o_file->next);
			o_file->next=file;
		}	
		out("文件登记成功\n");
		return 0;
	}
	
	static int del_dev(struct _ty_core	*core,uint8 *buf,int len)
	{
		return 0;
	}
	
	static int open_dev(struct _ty_core	*core,uint8 *buf,int len)
	{	int fd,i;
		struct _ty_core_open_file		*open_file;
		struct _ty_core_open_file		*o_file;
		char name[DF_FILE_NAME_MAX_LEN];
		memset(name,0,sizeof(name));
		sprintf(name,"%s/%s",DF_DEV_HEAD_NAME,buf);
		out1("========================执行文件打开命令============================\n");
		out1("打开文件名:[%s]\n",name);
		out("open[%s]\n",name);
		fd=ty_open(name,core->ty_m_fd,NULL,NULL);
		if(fd<0)
		{	out("open err =%d\n",fd);
			return fd;
		}
		out1("打开文件:fd=%d\n",fd);	
		open_file=mem_malloc(sizeof(struct _ty_core_open_file));
		if(open_file==NULL)
		{	ty_close(fd);
			return DF_ERR_MEM_ERR;
		}
		memset(open_file,0,sizeof(struct _ty_core_open_file));
		open_file->fd=fd;
		memcpy(open_file->name,name,len);
		open_file->core=core;
		out("文件fd=%d\n",open_file->fd);
		out("申请到新的文件名:%s,fd=%d\n",open_file->name,open_file->fd);
		if(core->open_file==NULL)
		{	core->open_file=open_file;
		}
		else 
		{	for(o_file=core->open_file;o_file->next!=NULL;o_file=o_file->next);
			o_file->next=open_file;
		}
		out1("------------------------协议栈内的文件列表-----------------------------\n");
		for(o_file=core->open_file,i=1;o_file!=NULL;o_file=o_file->next,i++)
		{	out1("文件序号:%d,名称:%s,fd:%d\n",i,o_file->name,o_file->fd);
			
		}
		out1("-----------------------------------------------------------------------\n");
		out("打开文件成功\n");
		return fd;
	}
	
	static int close_dev(struct _ty_core	*core,uint8 *buf,int buf_len)
	{	int fd,i,result;
		struct _ty_core_open_file		*o_file;
		struct _ty_core_open_file		*b_file;
		memcpy(&fd,buf,sizeof(fd));
		out1("========================执行文件关闭命令============================\n");
		out1("-------------------------关闭前文件列表-----------------------------\n");
		for(o_file=core->open_file,i=1;o_file!=NULL;o_file=o_file->next,i++)
		{	out1("文件序号:%d,名称:%s,fd:%d\n",i,o_file->name,o_file->fd);
		}
		out1("--------------------------------------------------------------------\n");
		out1("-------------------------寻找与fd=%d对应的文件----------------------\n");
		for(o_file=core->open_file,b_file=NULL;o_file!=NULL;o_file=o_file->next)
		{	if(fd==o_file->fd)
				break;
			b_file=o_file;
		}
		if(o_file==NULL)
		{	out1("没有找到与之对应的文件\n");
			out1("--------------------------------------------------------------------\n");
			return DF_ERR_CLASS_FILE;
		}	
		if(b_file==NULL)
		{	core->open_file=o_file->next;

		}
		else
		{	b_file->next=o_file->next;

		}
		out1("关闭文件:%s,fd=%d\n",o_file->name,o_file->fd);
		mem_free(o_file);
		result=ty_close(fd);
		out1("------------------------协议栈内的文件列表-----------------------------\n");
		for(o_file=core->open_file,i=1;o_file!=NULL;o_file=o_file->next,i++)
		{	out1("文件序号:%d,名称:%s,fd:%d\n",i,o_file->name,o_file->fd);
			
		}
		out1("-----------------------------------------------------------------------\n");
		out("文件关闭成功\n");
		return result;	
	}

	static int read_dev(struct _ty_core	*core,uint8 *buf,int buf_len)
	{	int fd;
		void *pbuf;
		int len;
		core=core;
		memcpy(&fd,buf,sizeof(fd));
		memcpy(&pbuf,buf+sizeof(fd),sizeof(pbuf));
		memcpy(&len,buf+sizeof(fd)+sizeof(pbuf),sizeof(len));
		out("读文件:fd=%d,pbuf=%d,len=%d\n",fd,pbuf,len);
		return ty_read(fd,pbuf,len);
	}

	static int write_dev(struct _ty_core	*core,uint8 *buf,int buf_len)
	{	int fd;
		void *pbuf;
		int len;
		core=core;
		memcpy(&fd,buf,sizeof(fd));
		memcpy(&pbuf,buf+sizeof(fd),sizeof(pbuf));
		memcpy(&len,buf+sizeof(fd)+sizeof(pbuf),sizeof(len));
		out("写文件:fd=%d,pbuf=%d,len=%d\n",fd,pbuf,len);
		len=ty_write(fd,pbuf,len);
		out("写文件结束:len=%d\n",len);
		return len;
	}

	static int ctl_dev(struct _ty_core	*core,uint8 *buf,int buf_len)
	{	int fd;
		int cm;
		int par[DF_TY_MAX_PAR-2];
		int i;
		core=core;
		memcpy(&fd,buf,sizeof(fd));
		memcpy(&cm,buf+sizeof(fd),sizeof(cm));
		for(i=0;i<sizeof(par)/sizeof(int);i++)
		{	memcpy(&par[i],buf+sizeof(fd)+sizeof(cm)+i*sizeof(int),sizeof(int));
		}
		out("控制文件:fd=%d,cm=%d\n",fd,cm);
		return ty_ctl(fd,cm,par[0],par[1],par[2],par[3],par[4],par[5]);
	}

