#define _ty_s_c
#include "_ty_s.h"
int ty_s_add(struct _ty_core	*core)
{	struct _ty_s	*ty_s;
	char name[DF_FILE_NAME_MAX_LEN];
	int result;
	ty_s=mem_malloc(sizeof(struct _ty_s));
	if(ty_s==NULL)
		return DF_ERR_MEM_ERR;
	out("加载通云服务协议栈\n");
	memset(ty_s,0,sizeof(struct _ty_s));
	ty_s->core=core;
	core->ty_s=ty_s; 
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s",core->dev_name,core->ty_s_name);
	out("服务协议栈名称为:%s\n",name);
	result=ty_file_add(DF_DEV_TY_S,ty_s,name,&_ty_s_file_fuc);
	if(result<0)
	{	mem_free(ty_s);
		return result;
	}
	out("服务协议栈加载成功\n");
	return result;
}

static int ty_s_del(void *data,int c)
{	struct _ty_s	*ty_s;
	ty_s=data;
	if(ty_s->state!=0)
		ty_s_close(data,1);
	mem_free(data);	
	return 0;
}

static int ty_s_open(struct ty_file	*file)
{	struct _ty_s	*ty_s;
	struct _ty_core	*core;
	struct _ty_s_re	*s_re;
	int result;
	char name[DF_FILE_NAME_MAX_LEN];
	ty_s=file->pro_data;
	if(ty_s->class_fd>0)
		return 1;
	core=ty_s->core;
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s",DF_DEV_HEAD_NAME,core->ty_class_name);
	out("服务协议栈打开串口名称为:%s\n",name);
	ty_s->class_fd=ty_open(name,NULL,NULL);
	if(ty_s->class_fd<0)
		return ty_s->class_fd; 
	ty_ctl(ty_s->class_fd,DF_TY_CLASS_DATA_CM_PRO,DF_FILE_SLAVE,seria_hook,ty_s);
	ty_s->state=1;
	out("服务协议栈打开成功\n");
	return 1;
}

static int ty_s_close(void *pro_data,int fd)
{	struct _ty_s	*ty_s;
	struct _ty_s_pbuf	*pbuf;
	ty_s=pro_data;
	if(ty_s->state==0)
		return 0;
	if(ty_s->class_fd>0)
	{	ty_close(ty_s->class_fd);
		ty_s->state=0;
		ty_s->class_fd=0;
		sleep(1);				//等待1秒时间
	}
	return 0;	
}

static void ty_s_task(void *arg)				//根据命令执行程序
{	struct _ty_s	*ty_s;
	struct	_ty_agree_class *agree;
	struct _ty_core	*core;
	uint8 buf[DF_COM_MAX_LEN];
	uint8 *s_buf;
	int result,len,re_len;
	ty_s=arg;	
	core=ty_s->core;
	agree=ty_s->buf;
	if((agree->order==DF_ORDER_ACTION_STATE)||(agree->order==DF_ORDER_ACTION_HOOK))
	{	ty_ctl(ty_s->class_fd,DF_TY_CLASS_DATA_CM_S_WRITE,agree->order,
		agree->ls,result,NULL,NULL);
	}
	out1("执行服务命令order=%d\n",agree->order);
	re_len=0;result=0;
	result=core->s_a_order(core,agree->order,agree->buf,&s_buf,agree->len,&re_len);
	out1("完成服务命令\n");
	if((agree->order!=DF_ORDER_ACTION_STATE)&&(agree->order!=DF_ORDER_ACTION_HOOK))
	{	out1("class_fd=%d\n",ty_s->class_fd);
		ty_ctl(ty_s->class_fd,DF_TY_CLASS_DATA_CM_S_WRITE,agree->order,
		agree->ls,result,s_buf,re_len);
	}
	out1("命令执行成功\n");
	mem_free(agree);
	mem_free(arg);
}

static void seria_hook(void *data,struct	_ty_agree *buf)
{	struct _ty_s	*ty_s;
	struct	_ty_agree_class *agree;
	out("接受到服务程序\n");
	if(data==NULL)
		return ;
	//if(ty_s->core->heart.err_n<0)
		//return ;
	ty_s=mem_malloc(sizeof(struct _ty_s));
	if(ty_s==NULL)
	{	printf("内存申请错误");
		exit(0);
	}
	memcpy(ty_s,data,sizeof(struct _ty_s));
	agree=mem_malloc(sizeof(struct _ty_agree_class));
	if(agree==NULL)
	{	printf("内存申请错误");
		exit(0);
	}
	memcpy(agree,buf,sizeof(struct	_ty_agree_class));
	ty_s->buf=agree;
	out("命令:%d\n",ty_s->buf->order);
	_queue_add("1",ty_s_task,ty_s,0);
}	

