#define _ty_m_c
#include "_ty_m.h"
int ty_m_add(struct _ty_core	*core)
{	struct _ty_m	*ty_m;
	int result;
	char name[DF_FILE_NAME_MAX_LEN];
	ty_m=malloc(sizeof(struct _ty_m));
	if(ty_m==NULL)
		return DF_ERR_MEM_ERR;
	memset(ty_m,0,sizeof(struct _ty_m));
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s",core->dev_name,core->ty_m_name);
	core->ty_m=ty_m;
	ty_m->core=core;
	result=ty_file_add(DF_DEV_TY_M,ty_m,name,&ty_m_fuc);	
	if(result<0)
		free(ty_m);
	return result;
}

static int ty_m_del(void *data,int c)
{	struct _ty_m	*ty_m;
	ty_m=data;
	if(ty_m->state!=0)
		ty_m_close(data,1);
	free(data);
	return 0;
}


static int ty_m_open(struct ty_file	*file)
{	struct _ty_m	*ty_m;
	struct _ty_core	*core;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	ty_m=file->pro_data;
	core=ty_m->core; 
	if(ty_m->state!=0)
		return 1;
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s",DF_DEV_HEAD_NAME,core->ty_class_m_name);
	ty_m->class_fd=ty_open(name,NULL,NULL);
	if(ty_m->class_fd<0)
		return ty_m->class_fd;
	ty_ctl(ty_m->class_fd,DF_TY_CLASS_DATA_CM_PRO,DF_FILE_MAST);
	ty_m->state=1;
	return 1;	
}

static int ty_m_ctl(void *data,int fd,int cm,va_list args)
{	struct _ty_m	*ty_m;
	struct _ty_core	*core;
	struct _ty_heart	*heart;
	uint8 buf[DF_COM_MAX_LEN],re_buf[DF_COM_MAX_LEN];
	int order,len,result,t_re;
	ty_m=data;
	core=ty_m->core; 
	if(ty_m->state!=1)
		return DF_ERR_CLASS_DEV_NO_OPEN;
	heart=&core->heart;
	if(heart->err_n<0)
	{	return heart->err_n;
	}
	if(cm==DF_TY_M_CM_ORDER)						//执行双机通讯
	{	order=va_arg(args,int);
		memset(buf,0,sizeof(buf));
		len=core->s_pack(order,buf,&args);
		if(len<0)
			return len;
		t_re=ty_ctl(ty_m->class_fd,DF_TY_CLASS_DATA_CM_M_WRITE,
		order,buf,len,re_buf,&result,&len);
		if(t_re<0)					//设备运行失败
		{	out("双机通讯失败:发送错误命令\n");
			heart->err_n=t_re;
			if(heart->err!=NULL)
			{	out("执行错误回调\n");
				_queue_add("heart_err",heart->err,heart->arg,0);	
			}	
			return t_re;
		}	
		if(len>0)
			core->m_a_order(order,re_buf,len,args);
		return result;
	}
	return -1;
}


static int ty_m_close(void *data,int fd)
{	struct _ty_m	*ty_m;
	ty_m=data;
	if(ty_m->state==0)
		return 0;
	ty_close(ty_m->class_fd);
	ty_m->state=0;	
	return 0;
}
