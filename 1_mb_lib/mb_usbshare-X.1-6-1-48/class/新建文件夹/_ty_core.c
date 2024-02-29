#define _ty_core_c
#include "_ty_core.h"
int ty_core_add(char *use_port,struct _ty_core_out	*app_in)
{	int result;
	struct _ty_core	*core;
	char name[DF_FILE_NAME_MAX_LEN];
	core=mem_malloc(sizeof(struct _ty_core));
	if(core==NULL)
	{	return DF_ERR_MEM_ERR;
	}
	memset(core,0,sizeof(struct _ty_core));
	memcpy(core->use_port,use_port,strlen(use_port));
	/*-------------------------------外来文件处理------------------------------------*/
	core->file_add=app_in->file_add;
	core->file=app_in->file;
	core->m_note=app_in->m_note;
	core->i_o=app_in->i_o;
	core->f_s=app_in->f_s;
	core->heart.o_timer=app_in->heart_timer;
	core->heart.err=app_in->err;
	core->heart.arg=app_in->arg;
	out("core->file_add=%d\n",core->file_add);
	out("core->file=%d\n",core->file);
	out("core->m_note=%d\n",core->m_note);
	out("core->i_o=%d\n",core->i_o);
	out("core->f_s=%d\n",core->f_s);
	out("core->heart.o_timer=%d\n",core->heart.o_timer);
	out("core->heart.err=%d\n",core->heart.err);
	out("core->heart.arg=%d\n",core->heart.arg);
	
	/*-------------------------------处理本机文件------------------------------------*/
	core->s_a_order=ty_s_a_order;
	core->s_pack=ty_m_s_pack;
	core->m_a_order=ty_m_a_order;
	core->explain=ty_explain;
	core->core_file=&core_file;
	core->mem=&ty_mem;
	core->action=&ty_action;
	core->self_m=ty_core_hook;
	/*-------------------------------命名规则---------------------------------------*/
	sprintf(core->dev_name,"[%s]",core->use_port);
	sprintf(core->ty_m_name,"%s",DF_TY_CORE_M_NAME);
	sprintf(core->ty_s_name,"%s",DF_TY_CORE_S_NAME);
	sprintf(core->ty_name,"%s",DF_TY_CORE_NAME);
	sprintf(core->ty_class_name,"%s/ty_class",core->dev_name);
	sprintf(core->ty_class_m_name,"%s/ty_m_class",core->dev_name);
	/*-----------------------------处理文件加载------------------------------------*/
	result=ty_m_add(core);
	if(result<0)
	{	mem_free(core);
		return result;
	}
	result=ty_s_add(core);
	if(result<0)
	{	mem_free(core);
		return result;
	}
	result=ty_class_data_add(core->use_port,core->ty_class_name);
	if(result<0)
	{	mem_free(core);
		return result;
	}
	result=ty_class_data_add(core->use_port,core->ty_class_m_name);
	if(result<0)
	{	mem_free(core);
		return result;
	}
	/*----------------------------加载主处理文件----------------------------------*/
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s",core->dev_name,core->ty_name);
	result=ty_file_add(DF_DEV_TY,core,name,&ty_core_fuc);
	if(result<0)
	{	mem_free(core);
		return result;	
	}
	return result;
}

static int ty_core_del(void *data,int c)
{	struct _ty_core	*core;
	char name[DF_FILE_NAME_MAX_LEN];
	core=data;
	if(core->state!=0)
		ty_core_close(data,1);
	memset(name,0,sizeof(name));
	sprintf(name,"dev/%s/%s",core->dev_name,core->ty_m_name);
	ty_file_del(name);
	memset(name,0,sizeof(name));
	sprintf(name,"dev/%s/%s",core->dev_name,core->ty_s_name);
	ty_file_del(name);
	memset(name,0,sizeof(name));
	sprintf(name,"dev/%s",core->ty_class_m_name);
	ty_file_del(name);
	memset(name,0,sizeof(name));
	sprintf(name,"dev/%s",core->ty_class_name);
	ty_file_del(name);
	mem_free(core);
	return 0;		
}


static int ty_core_open(struct ty_file	*file)
{	char name[DF_FILE_NAME_MAX_LEN];
	struct _ty_core	*core;
	int result;
	core=file->pro_data;
	if(core->state!=0)
		return DF_ERR_CLASS_FD_ED;
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s/%s",DF_DEV_HEAD_NAME,core->dev_name,core->ty_s_name);
	core->ty_s_fd=ty_open(name,NULL,NULL); 
	out("core->ty_s_fd=%d\n",core->ty_s_fd);
	if(core->ty_s_fd<0)
	{	result=core->ty_s_fd;
		core->ty_s_fd=0;
		out("result=%d\n");
		return result;
	}
	memset(name,0,sizeof(name));
	sprintf(name,"%s/%s/%s",DF_DEV_HEAD_NAME,core->dev_name,core->ty_m_name);
	core->ty_m_fd=ty_open(name,NULL,NULL); 
	if(core->ty_m_fd<0)
	{	result=core->ty_m_fd;
		core->ty_m_fd=0;
		return result;
	}
	core->state=1;
	if(core->f_s==DF_SOFT_FRIST)
		return ty_core_init(core);
	struct _ty_heart	*heart;
	heart=&core->heart;
	heart->state=DF_SOFT_STATE_OVER;
	heart->core=core;
	if((heart->task==0)&&(heart->o_timer!=0))
	{	heart->task=_task_open("heart",ty_core_heart,heart,heart->o_timer,NULL);
		out("heart->task=%d\n",heart->task);
	}	
	return 1;
}


static int ty_core_close(void *data,int fd)
{	struct _ty_core	*core;
	struct _ty_core_add_file		*file;
	struct _ty_core_add_file		*file1;
	struct _ty_core_open_file		*open_file;
	struct _ty_core_open_file		*open_file1;
	char name[DF_FILE_NAME_MAX_LEN];
	core=data;
	out("ty_core_close start\n");
	if(core->state==0)
		return 0;
	if(core->heart.task>0)
	{	out("关闭心跳任务,core->heart.task=%d\n",core->heart.task);
		_task_close(core->heart.task);
		core->heart.task=0;
	}	
	core->close=255;
	out("关闭服务协议栈\n");
	ty_close(core->ty_s_fd);
	out("关闭主协议栈\n");
	ty_close(core->ty_m_fd);
	out("删除添加的文件\n");
	for(file=core->add_file;file!=NULL;)
	{	memset(name,0,sizeof(name));
		out("文件名:%s\n",file->self_name);
		sprintf(name,"dev/%s",file->self_name);
		out("删除文件:%s\n",name);
		ty_file_del(name);
		file1=file->next;
		mem_free(file);
		file=file1;
	}
	out("删除打开文件\n");
	for(open_file=core->open_file;open_file!=NULL;)
	{	ty_close(open_file->fd);
		open_file1=open_file->next;
		mem_free(open_file);
		open_file=open_file1;
	}
	core->state=0;
	return 0;
}

static int ty_core_init(struct _ty_core	*core)
{	int i;
	struct _ty_file_s_add	*add_file;
	int result;
	for(i=0;i<3;i++)
	{	if((result=core->action->state(core,DF_SOFT_STATE_APP))<0)
		{	continue;
		}
		break;	
	}
	if(i==3)
	{	return DF_ERR_CLASS_NO_DEV;
	}	
	out("发送文件信息core->i_o=%d\n",core->i_o);
	out("发送文件信息DF_SOFT_EDIT_INFOR=%d\n",DF_SOFT_EDIT_INFOR);
	if(core->i_o==DF_SOFT_EDIT_INFOR)
	{	add_file=core->file;
		out("add_file=%d\n",add_file);
		if(add_file!=NULL)
		{	char name[DF_FILE_NAME_MAX_LEN];
			char dev_name[DF_FILE_NAME_MAX_LEN];
			memset(name,0,sizeof(name));
			memset(dev_name,0,sizeof(dev_name));
			sprintf(dev_name,"%s",core->dev_name);
			sprintf(name,"%s",core->use_port);
			for(;add_file!=NULL;add_file=add_file->next)
			{	if((memcmp(name,add_file->name,strlen(name))==0)||
				(memcmp(dev_name,add_file->name,strlen(name))==0))
					continue;
				result=core->action->new_file(core,add_file->pro,add_file->name);
				if(result<0)
				{	out("加载文件失败\n");
					
				}
			}
		}	
	}
	core->m_state=DF_SOFT_STATE_OVER;
	if((result=core->action->state(core,DF_SOFT_STATE_OVER))<0)
	{	return result;
	}	
	struct _ty_heart	*heart;
	heart=&core->heart;
	heart->state=DF_SOFT_STATE_OVER;
	heart->core=core;
	if((heart->task==0)&&(heart->o_timer!=0))
	{	heart->task=_task_open("heart",ty_core_heart,heart,heart->o_timer,NULL);
		out("heart->task=%d\n",heart->task);
	}	
	return 1;
}

static void ty_core_hook(void *arg)
{	struct _ty_core	*core;
	core=arg;
	ty_core_init(core);
}

static void ty_core_heart(void *arg ,int o_timer)
{	struct _ty_heart	*heart;
	struct _ty_core	*core;
	heart=arg;
	if(heart==NULL)
		return ;
	if(heart->err_n<0)
		return ;
	if(heart->state!=DF_SOFT_STATE_OVER)
		return ;
	out("执行心跳命令\n");
	core=heart->core;
	core->action->state(core,DF_SOFT_STATE_OVER);	
}

