#define file_ac_c
#include "../inc/file_ac.h"

struct ty_file_n file_stream;

static _err	file_err[]=
{
	{	.err=DF_ERR_FILE_LOADED,
		.name="文件已经加载"
	},
	{	.err=DF_ERR_FILE_NO_SPACE,
		.name="文件已经没有可用的空间"
	},
	{	.err=DF_ERR_FILE_NO_DEV,
		.name="不是DEV文件"
	},
	{	.err=DF_ERR_FILE_NO_FILE,
		.name="没有该文件"
	},
	{	.err=DF_ERR_FILE_FD_ERR,
		.name="文件fd错误"
	},
	{	.err=DF_ERR_FILE_NO_OPEN,
		.name="文件未打开"
	},
	{   .err=DF_DEV_USB_M,
		.name="ty_usb_m"
	},
};

static const _err_buf	file_err_buf=
{
	file_err,
	sizeof(file_err)/sizeof(_err),
	0
};

int ty_file_load()
{
	if(file_stream.lock!=0)
		return DF_ERR_FILE_LOADED;
	memset(&file_stream,0,sizeof(file_stream));	
	file_stream.lock=_lock_open("ty_file",NULL);
	if(file_stream.lock<0)
		return file_stream.lock;
	return sizeof(file_stream.file_state)/sizeof(struct ty_file_state	);
}

/*===============================文件基本创建和删除=======================================*/
int ty_file_add(int pro,void *pro_data,char *name,struct _file_fuc	*fuc)
{
	int i,len,result;
	struct ty_file	*file;
	file=malloc(sizeof(struct ty_file));
	if(file==NULL)
		return DF_ERR_MEM_ERR;
	memset(file,0,sizeof(struct ty_file));
	file->pro=pro;
	file->pro_data=pro_data;
	file->fuc=fuc;
	file->state=1;
	len=strlen(name);
	if(len>sizeof(file->name)/sizeof(char ))
		len=sizeof(file->name)/sizeof(char );
	memcpy(file->name,name,len);
	file->lock=_lock_open(name,NULL);
	if(file->lock<0)
	{	result=file->lock;
		free(file);
		return result;
	}
	_lock_set(file_stream.lock);
	for(i=0;i<sizeof(file_stream.file_state)/sizeof(struct ty_file_state);i++)
	{	if(file_stream.file_state[i].file==NULL)
			break;
	}
	if(i==sizeof(file_stream.file_state)/sizeof(struct ty_file_state))
		return DF_ERR_FILE_NO_SPACE;
	file_stream.file_state[i].state++;
	file_stream.file_state[i].file=file;
	_lock_un(file_stream.lock);
	file_ac_out("file load [%s]\n",name);
	return 0;
}

int ty_file_del(char *name)				//删除文件
{
	struct ty_file	*file;
	int len,i;
	char *p;
	file_ac_out("删除文件:%s\n",name);
	if((memcmp(name,"dev/",4)!=0)&&(memcmp(name,"/dev/",5)!=0))		
		return DF_ERR_FILE_NO_DEV;
	p=strstr(name,"dev/");
	p+=4;
	len=strlen(p);
	for(i=0;i<sizeof(file_stream.file_state)/sizeof(struct ty_file_state);i++)
	{	if(file_stream.file_state[i].file==NULL)
			continue;
		if(strlen(file_stream.file_state[i].file->name)!=len)
			continue;
		if(memcmp(file_stream.file_state[i].file->name,p,len)==0)
			break;
	}
	if(i==sizeof(file_stream.file_state)/sizeof(struct ty_file_state))
		return DF_ERR_FILE_NO_FILE;
	file=file_stream.file_state[i].file;
	_lock_set(file->lock);
	if(file->fuc->del!=NULL)
	{	file->fuc->del(file->pro_data,0);
		memset(file->fd,0,sizeof(file->fd));
	}
	else
	{	for(len=0;len<sizeof(file->fd)/sizeof(int);len++)
		{	if(file->fd[len].s_fd!=0)
			{	file->fuc->close(file->pro_data,file->fd[len].s_fd);
				memset(&file->fd[len],0,sizeof(struct ty_file_fd));	
			}	
		}
	}
	_lock_un(file->lock);
	_lock_close(file->lock);
	free(file);
	file_stream.file_state[i].file=NULL;
	file_ac_out("file del,i=%d\n",i);
	return 0;
}

/*===============================获取当前文件===========================================*/
void _ty_file_get_file(struct ty_file_n	*file_n)
{
	memcpy(file_n,&file_stream,sizeof(file_stream));
}

struct _ty_file_s_add *ty_get_file(void)
{
	struct _ty_file_s_add		*file;
	struct _ty_file_s_add		**file_add;
	int i;
	file=NULL;
	for(i=0,file_add=&file;i<sizeof(file_stream.file_state)/sizeof(struct ty_file_state);i++)
	{	if(file_stream.file_state[i].file!=NULL)
		{	*file_add=malloc(sizeof(struct _ty_file_s_add));
			if(*file_add==NULL)
			{	;
				return NULL;
			}
			memset(*file_add,0,sizeof(struct _ty_file_s_add));
			memcpy((*file_add)->name,file_stream.file_state[i].file->name,
			strlen(file_stream.file_state[i].file->name));
			(*file_add)->pro=file_stream.file_state[i].file->pro;
			file_add=&((*file_add)->next);
		}
	}
	return file;
}


void ty_file_free(struct _ty_file_s_add *file)
{
	struct _ty_file_s_add 	*data;
	struct _ty_file_s_add	*s;
	for(data=file;data!=NULL;)
	{	s=data;
		data=s->next;
		free(s);
	}
}




