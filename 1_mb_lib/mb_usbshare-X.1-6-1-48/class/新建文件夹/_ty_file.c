#define _ty_file_c
#include "_ty_file.h"
static int ty_file_open(struct _ty_core *core,char *name)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_OPEN,name);	
}

static int ty_file_close(struct _ty_core *core,int fd)
{	if(core->close!=255)
		return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_CLOSE,fd);
	else
		return 0;
}

static int ty_file_read(struct _ty_core *core,int fd,uint8 *buf,int buf_len)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_READ,fd,buf,buf_len);
}

static int ty_file_write(struct _ty_core *core,int fd,uint8 *buf,int buf_len)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_WRITE,fd,buf,buf_len);
}

static int ty_file_ctl(struct _ty_core *core,int fd,int cm,...)
{	va_list args;
	int par[8];
	int i;
	va_start(args,cm);
	for(i=0;i<8;i++)
	{	par[i]=va_arg(args,int);
	}
	va_end(args);
	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_CTL,fd,cm,
					par[0],par[1],par[2],par[3],par[4],par[5],par[6],par[7]);
}
static int ty_core_read_buf(void *mem_dev,void *ty,void *buf,int len)
{	struct _ty_core	*core;
	core=mem_dev;
	out("ty_core_read_buf,buf=%d,len=%d\n",buf,len);
	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_READ_BUF,buf,len,ty);	
}

static int ty_core_write_buf(void *mem_dev,void *ty,void *buf,int len)
{	struct _ty_core	*core;
	core=mem_dev;
	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_WRITE_BUF,buf,len,ty);
}

static int ty_core_hook(void *mem_dev,void *hook,void *arg)
{	struct _ty_core	*core;
	core=mem_dev;
	out("·¢ËÍ»Øµ÷ÃüÁî:hook=%d,arg=%d\n",hook,arg);
	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_HOOK,hook,arg);
}

static int ty_core_state(struct _ty_core *core,int state)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_STATE,state);
}

static int ty_core_note(struct _ty_core	*core,_so_note	*note)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_SOFT_INFOR,note);
}

static int ty_core_err(struct _ty_core *core,int err,char *err_c)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_SOFT_ERR,err,err_c);
}

static int ty_core_new_file(struct _ty_core	*core,int pro,char *name)
{	return ty_ctl(core->ty_m_fd,DF_TY_M_CM_ORDER,DF_ORDER_FILE_NEW,pro,name);
}

