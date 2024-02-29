#define _mem_oper_c
#include "_mem_oper.h"
int mem_read(void *mem_dev,void *ty,void *buf,int len)
{	struct ty_file_fd	*file_fd;
	int result,i_size,i;
	file_fd=mem_dev;
	for(i=0;i<len;i+=i_size)
	{	i_size=DF_MEM_BUF_MAX_NUMBER;
		if(i_size>(len-i))
			i_size=(len-i);
		result=ty_ctl(file_fd->ty_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_READ_BUF,
		buf+i,i_size,ty+i);
		if(result<0)
			return result;
		if(result<i_size)
		{	i+=result;
			break;
		}
	}
	return i; 	
}

int mem_write(void *mem_dev,void *ty,void *buf,int len)
{	struct ty_file_fd	*file_fd;
	int result,i_size,i;
	file_fd=mem_dev;
	for(i=0;i<len;i+=i_size)
	{	i_size=DF_MEM_BUF_MAX_NUMBER;
		if(i_size>(len-i))
			i_size=(len-i);
		result=ty_ctl(file_fd->ty_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_WRITE_BUF,
		buf+i,i_size,ty+i);
		if(result<0)
			return result;
		if(result<i_size)
		{	i+=result;
			break;
		}
	}
	return i; 
}

int mem_hook(void *mem_dev,void *hook,void *arg)
{	struct ty_file_fd	*file_fd;
	file_fd=mem_dev;
	return ty_ctl(file_fd->ty_fd,DF_TY_M_CM_ORDER,DF_ORDER_ACTION_HOOK,
	hook,arg);
}

