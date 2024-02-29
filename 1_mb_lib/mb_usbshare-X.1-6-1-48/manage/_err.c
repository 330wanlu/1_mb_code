#define _err_c
#include "_err.h"
void _err_load(void *par)
{	err_name=NULL;
	memset(&table_err,0,sizeof(table_err));
	lock_fd=_lock_open("err",NULL);
	_err_add("linux_s.c",&linux_err_buf);
}
void _err_add(char *name,const _err_buf	*buf)
{	struct _err_name		*err;
	struct _err_name		**pbuf;
	_lock_set(lock_fd);
	for(err=err_name,pbuf=&err_name;err!=NULL;pbuf=&err->next,err=err->next);		
	err=malloc(sizeof(struct _err_name));
	memset(err,0,sizeof(struct _err_name));
	memcpy(err->name,name,strlen(name));
	err->buf = (_err_buf	*)buf;
	*pbuf=err;
	out("[%s]ÊäÈë³É¹¦\n",name);
	_lock_un(lock_fd);
}

void _err_out(int number,char *o_name,char *note)
{	struct _err_name		*err;
	_err_buf		*buf;
	int i;
	out("number=%d\n",number);
    #if 1
	for(err=err_name;err!=NULL;err=err->next)
	{	buf=err->buf;
		out("buf->number=%d\n",buf->number);
		for(i=0;i<buf->number;i++)
		{	out("buf->err[%d].err=%d\n",i,buf->err[i].err);
			if(number==buf->err[i].err)
				break;
		}
		if(i!=buf->number)
		{	if(note!=NULL)
				memcpy(note,buf->err[i].name,strlen(buf->err[i].name));
			if(o_name!=NULL)
				memcpy(o_name,err->name,strlen(err->name));
			return ;	
		}
	}	
    #endif
}



