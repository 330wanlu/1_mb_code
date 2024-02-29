#define _file_c
#include "_file.h"
int ty_open(char *name,int ty_fd,int c,...)
{	char *p;
	int i,len,fd,j;
	struct ty_file	*file;
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
	out("文件i=%d\n",i,sizeof(file_stream.file_state)/sizeof(struct ty_file_state));
	if(i==sizeof(file_stream.file_state)/sizeof(struct ty_file_state))
	{	out("没有这个文件\n");
		return DF_ERR_FILE_NO_FILE;
	}	
	file=file_stream.file_state[i].file;
	_lock_set(file->lock);
	fd=file->fuc->open(file);
	if(fd<0)
	{	_lock_un(file->lock);
		return fd;
	}
	j=fd;
	
	
	//file->fd[fd-1]=fd;
	file->state=1;
	fd=(i+1)*65536+file_stream.file_state[i].state*256+fd;
	file->fd[j-1].s_fd=j;
	file->fd[j-1].m_fd=fd; 	
	file->fd[j-1].ty_fd=ty_fd; 
	/////////////////添加通云的FD
	_lock_un(file->lock);
	
	return fd;
}

int ty_read(int fd,void *buf,int buf_len)
{	int i,result;
	struct ty_file	*file;
	uint8 state;
	i=(fd&0xffff0000);
	i=i/65536;
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
		return DF_ERR_FILE_FD_ERR;
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
		return -1;
	if(file->state==0)
		return DF_ERR_FILE_NO_OPEN;
	state=((fd&0x0000ff00)>>8);
	if(state!=file_stream.file_state[i].state)
		return DF_ERR_FILE_FD_ERR;
	i=(fd&0xff);
	if(file->fuc->read==NULL)
		return -1;
	_lock_set(file->lock);
	result=file->fuc->read(file->pro_data,i,buf,buf_len);
	_lock_un(file->lock);
	return result;	
}

int ty_write(int fd,void *buf,int buf_len)
{	int i,result;
	struct ty_file	*file;
	uint8 state;
	i=(fd&0xffff0000);
	i=i/65536;
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
		return DF_ERR_FILE_FD_ERR;
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
		return -1;
	if(file->state==0)
		return DF_ERR_FILE_NO_OPEN;
	state=((fd&0x0000ff00)>>8);
	if(state!=file_stream.file_state[i].state)
		return DF_ERR_FILE_FD_ERR;
	i=(fd&0xff);
	if(file->fuc->write==NULL)
		return -1;
	_lock_set(file->lock);
	result=file->fuc->write(file->pro_data,i,buf,buf_len);
	_lock_un(file->lock);
	return result;
}

int ty_close(int fd)
{	int i,result;
	struct ty_file	*file;
	uint8 state;
	i=(fd&0xffff0000);
	i=i/65536;
	out("文件序号:fd=%d\n",fd);
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
	{	out("文件:fd出错\n");
		return DF_ERR_FILE_FD_ERR;
	}	
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
	{	out("文件为空\n");
		return -1;
	}	
	if(file->state==0)
	{	out("文件未被使用\n");
		return DF_ERR_FILE_NO_OPEN;
	}	
	state=((fd&0x0000ff00)>>8);
	if(state!=file_stream.file_state[i].state)
	{	out("文件状态不对\n");
		return DF_ERR_FILE_FD_ERR;
	}	
	i=(fd&0xff);
	//out("close in \n");
	_lock_set(file->lock);
	//out("close out \n");
	out("执行删除文件\n");
	result=0;
	if(file->fuc->close!=NULL)
	{	out("进入CORE删除文件\n");
		result=file->fuc->close(file->pro_data,i);
	}	
	memset(&file->fd[i-1],0,sizeof(struct ty_file_fd));
	_lock_un(file->lock);
	out("删除文件完成\n");
	//file->fd[i-1]=0;
	return result;
}

int ty_ctl(int fd,int cm,...)
{	int i,result;
	va_list args;
	struct ty_file	*file;
	char *name;
	uint8 state;
	i=(fd&0xffff0000);
	i=i/65536;
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
		return DF_ERR_FILE_FD_ERR;
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
		return -1;
	if(file->state==0)
		return DF_ERR_FILE_NO_OPEN;
	state=((fd&0x0000ff00)>>8);
	if(state!=file_stream.file_state[i].state)
		return DF_ERR_FILE_FD_ERR;
	i=(fd&0xff);
	_lock_set(file->lock);
	va_start(args,cm);
	if(cm==DF_FILE_GET_CM_NAME)
	{	name=va_arg(args,char *);
		va_end(args);
		memcpy(name,file->name,strlen(file->name));
		_lock_un(file->lock);
		return strlen(file->name);
	}
	else
	{	if(file->fuc->ctl!=NULL)
		{	result=file->fuc->ctl(file->pro_data,i,cm,args);
			va_end(args);
			_lock_un(file->lock);
		}
		else 
		{	va_end(args);
			_lock_un(file->lock);
		}
	}	
	return result;
}

int ty_get(int fd,int cm,...)
{	int i;
	va_list args;
	struct ty_file	*file;
	//uint8 state;
	i=(fd&0xffff0000);
	i=i/65536;
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
		return DF_ERR_FILE_FD_ERR;
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
		return -1;
	if(file->state==0)
		return DF_ERR_FILE_NO_OPEN;
	_lock_set(file->lock);
	if(cm==DF_FILE_GET_CM_NAME)				//获取文件的名字
	{	char *name;
		va_start(args,cm);
		name=va_arg(args,char *);
		va_end(args);
		memcpy(name,file->name,strlen(file->name));
		return strlen(file->name);
	}
	return -1;
}




