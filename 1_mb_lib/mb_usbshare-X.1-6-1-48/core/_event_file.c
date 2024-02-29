#define _event_file_c
#include "_event_file.h"
int event_file_add(char *switch_name)
{   struct _event_file  *stream;
	int result;
	stream=malloc(sizeof(struct _event_file));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
    //out("加载:[event_file_add]模块\n");
	memset(stream,0,sizeof(struct _event_file));
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
    //out("[event_file_add]:stream->switch_name=[%s]\n",stream->switch_name);
	result = ty_file_add(DF_DEV_TY_USB, stream, DF_EVENT_FILE_NAME, (struct _file_fuc *)&event_file_fuc);
	if(result<0)
		free(stream);
    //out("加载:[event_file_add]成功\n");
	return result;    
}

static int event_file_open(struct ty_file	*file)
{   struct _event_file  *stream;
    int j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("打开event_file模块\n");
    if(stream->state==0)
    {   out("第一次打开事件文件\n");
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   out("打开开关事件失败\n");
            return stream->switch_fd;
        }
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    out("打开event_file成功:j=%d\n",j);
	return j+1;	       
}

static int event_file_ctl(void *data,int fd,int cm,va_list args)
{   struct _event_file      *stream;
    struct _event_file_fd   *id;
    int i;
    stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    if(cm==DF_EVENT_CM_WRITE)
    {   out("写事件\n");
        struct _switch_event    event;
        memset(&event,0,sizeof(event));
        event.event=va_arg(args,int);
        event.reason=va_arg(args,int);
        clock_get(event.clock);
        out("事件:stream->switch_fd=%d\n",stream->switch_fd);
        return ty_ctl(stream->switch_fd,DF_SWITCH_CM_ADD_RECODER,&event);
    }
    else if(cm==DF_EVENT_CM_READ)
    {   struct _switch_event_all    *event_all;
        out("读出全部事件\n");
        event_all=va_arg(args,struct _switch_event_all  *);
        if(event_all==NULL)
            return -1;
        return ty_ctl(stream->switch_fd,DF_SWITCH_CM_READ_RECODER,event_all);    
    }
    else if(cm==DF_EVENT_CM_EVENT_NOTE)
    {   char *event_note;
        int event_v;
        out("翻译事件名称\n");
        event_v=va_arg(args,int);
        event_note=va_arg(args,char *);
        if(event_note==NULL)
            return -1;
        for(i=0;i<sizeof(event_name)/sizeof(event_name[0]);i++)
        {   if(event_name[i].c==event_v)
            {   memcpy(event_note,event_name[i].e,strlen(event_name[i].e));
                return strlen(event_name[i].e);
            }   
        }
        sprintf(event_note,"未知事件[%d]",event_v);
        return strlen(event_note);
    }
    else if(cm==DF_EVENT_CM_REASON_NOTE)
    {   char *reason_note;
        int reason_v;
        out("翻译事件发生原因\n");
        reason_v=va_arg(args,int);
        out("reason_v=%d\n",reason_v);
        reason_note=va_arg(args,char *);
        if(reason_note==NULL)
            return -1;
        for(i=0;i<sizeof(event_reason)/sizeof(event_reason[0]);i++)
        {   if(event_reason[i].c==reason_v)
            {   memcpy(reason_note,event_reason[i].e,strlen(event_reason[i].e));
                return strlen(event_reason[i].e);
            }   
        }
        sprintf(reason_note,"未知原因[%d]",reason_v);
        return strlen(reason_note);
    }
    return DF_ERR_PORT_CORE_CM;
}

static int _event_file_close(void *data,int fd)
{   struct _event_file      *stream;
    struct _event_file_fd   *id;
    //int j;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
	memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)			//全部关闭了
	{   ty_close(stream->switch_fd);
	}
    return 0;     
}







