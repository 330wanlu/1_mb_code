#include "../inc/file.h"

struct ty_file_n		file_stream;

int ty_open(char *name,int ty_fd,int c,...)
{
	char *p = NULL;
	int i = 0,len = 0,fd = -1,j = 0;
	struct ty_file	*file = NULL;
	if((memcmp(name,"dev/",4)!=0)&&(memcmp(name,"/dev/",5)!=0))
		return DF_ERR_FILE_NO_DEV;
	p = strstr(name,"dev/");
	p += 4;
	len = strlen(p);
	for(i = 0;i < sizeof(file_stream.file_state)/sizeof(struct ty_file_state);i++)
	{
		if(file_stream.file_state[i].file == NULL)
			continue;
		if(strlen(file_stream.file_state[i].file->name) != len)
			continue;
		if(memcmp(file_stream.file_state[i].file->name,p,len) == 0)
			break;
	}
	file_out("�ļ�i=%d\n",i,sizeof(file_stream.file_state)/sizeof(struct ty_file_state));
	if(i == sizeof(file_stream.file_state)/sizeof(struct ty_file_state))
	{
		file_out("û������ļ�\n");
		return DF_ERR_FILE_NO_FILE;
	}
	file = file_stream.file_state[i].file;
	_lock_set(file->lock);
	fd = file->fuc->open(file);
	if(fd < 0)
	{
		_lock_un(file->lock);
		return fd;
	}
	j = fd;
	file->state = 1;
	fd = (i+1)*65536+file_stream.file_state[i].state*256+fd;
	file->fd[j-1].s_fd = j;
	file->fd[j-1].m_fd = fd; 	
	file->fd[j-1].ty_fd = ty_fd; 
	_lock_un(file->lock);
	return fd;
}

int ty_read(int fd,void *buf,int buf_len)
{
	int i = 0,result = -1;
	struct ty_file *file = NULL;
	uint8 state = 0;
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
{
	int i = 0,result = -1;
	struct ty_file *file = NULL;
	uint8 state = 0;
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
{
	int i = 0,result = -1;
	struct ty_file	*file = NULL;
	uint8 state = 0;
	i=(fd&0xffff0000);
	i=i/65536;
	file_out("�ļ����:fd=%d\n",fd);
	if((i==0)||(i>sizeof(file_stream.file_state)/sizeof(struct ty_file_state)))
	{	file_out("�ļ�:fd����\n");
		return DF_ERR_FILE_FD_ERR;
	}	
	i--;
	file=file_stream.file_state[i].file;
	if(file==NULL)
	{	file_out("�ļ�Ϊ��\n");
		return -1;
	}	
	if(file->state==0)
	{	file_out("�ļ�δ��ʹ��\n");
		return DF_ERR_FILE_NO_OPEN;
	}	
	state=((fd&0x0000ff00)>>8);
	if(state!=file_stream.file_state[i].state)
	{	file_out("�ļ�״̬����\n");
		return DF_ERR_FILE_FD_ERR;
	}	
	i=(fd&0xff);
	//file_out("close in \n");
	_lock_set(file->lock);
	//file_out("close file_out \n");
	file_out("ִ��ɾ���ļ�\n");
	result=0;
	if(file->fuc->close!=NULL)
	{	file_out("����COREɾ���ļ�\n");
		result=file->fuc->close(file->pro_data,i);
	}	
	memset(&file->fd[i-1],0,sizeof(struct ty_file_fd));
	_lock_un(file->lock);
	file_out("ɾ���ļ����\n");
	//file->fd[i-1]=0;
	return result;
}

int ty_ctl(int fd,int cm,...)
{
	int i = 0,result = -1;
	va_list args;
	struct ty_file *file = NULL;
	char *name = NULL;
	uint8 state = 0;
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
{
	int i = 0;
	va_list args;
	struct ty_file *file = NULL;
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
	if(cm==DF_FILE_GET_CM_NAME)				//��ȡ�ļ�������
	{	char *name;
		va_start(args,cm);
		name=va_arg(args,char *);
		va_end(args);
		memcpy(name,file->name,strlen(file->name));
		return strlen(file->name);
	}
	return -1;
}




