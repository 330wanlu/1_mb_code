#define _vpn_set_c
#include "_vpn_set.h"
int vpn_set_add(void)
{   struct _vpn_set *stream;
	int result;
	stream=malloc(sizeof(struct _vpn_set));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _vpn_set));
    //out("����:[vpn_set_add]ģ��\n");
	result = ty_file_add(DF_DEV_VPN_SET, stream, "vpn_set", (struct _file_fuc *)&vpn_set_fuc);
	if(result<0)
		free(stream);
    //out("����:[vpn_set_add]�ɹ�\n");
	return result;    
}



static int vpn_set_open(struct ty_file	*file)
{   struct _vpn_set  *stream;
    int j;
	//int fd;
	//char name[DF_FILE_NAME_MAX_LEN];
	//uint8 gbuf[1024];
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //out("��vpn_set_openģ��\n");
	if(stream->state==0)
    {   //out("����һ��������\n");
        stream->lock=_lock_open("_vpn_set.c",NULL);
        if(stream->lock<0)
        {   out("������ʧ��\n");
            return stream->lock;
        }
		stream->deploy_fd=ty_open("/dev/deploy",0,0,NULL); 
		
		ty_ctl(stream->deploy_fd,DF_DEPLOY_CM_GET_VPN_PAR,&stream->vpn_deploy);
		
		//out("����openvpn����\n");
		_task_open("_vpn_set",check_vpn_process_status,stream,3000,NULL);
		        
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
   
    out("��VPN�������ɹ�\n");
    return j+1;
}

static int vpn_set_ctl(void *data,int fd,int cm,va_list args)
{   struct _vpn_set  *stream;
    struct _vpn_set_fd  *id;
    int result;
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
    for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
    {   if(cm==ctl_fun[i].cm)
        {   _lock_set(stream->lock);
            result=ctl_fun[i].ctl(id,args);
            _lock_un(stream->lock);
            return result;
        }    
    }
    return DF_ERR_PORT_CORE_CM;    
}

static int vpn_set_close(void *data,int fd)
{   struct _vpn_set  *stream;
    struct _vpn_set_fd  *id;
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
    if(stream->state==0)
    {
        _lock_close(stream->lock);
    }
    return 0;
}


static void check_vpn_config_file(void *arg)
{	struct _vpn_set  *stream;    
	int result;
	stream=arg;
	result = ty_ctl(stream->deploy_fd,DF_DEPLOY_CM_SET_VPN_PAR,&stream->vpn_deploy);
	if(result < 0)
	{
		out("��ȡVPN����ʧ��");
		return ;
	}
	if(stream->vpn_deploy.enable == 0)
	{
		return ;
	}
	out("����������VPN\n");
	out("��Ⲣ���¿��ļ�\n");
	result = check_lib_so_update(stream, (uint8 *)"/lib", (uint8 *)"libcrypto.so.1.0.2");
	if(result < 0)
	{	out("���ض�̬��ʧ��\n");
		return ;
	}
	result = check_lib_so_update(stream, (uint8 *)"/lib", (uint8 *)"libssl.so.1.0.2");
	if(result < 0)
	{	out("���ض�̬��ʧ��\n");
		return ;
	}
	out("���CA�ļ��Ƿ����\n");
	result = check_file_and_ca(stream);
	if(result < 0)
	{
		out("��Ⲣ����CA�ļ�ʧ��\n");
		return ;
	}
	out("�������Ƿ��Ѿ�����\n");
	result = detect_process("/home/share/openvpn/openvpn","vpntmp.txt");
	if(result < 0)
	{
		out("openvpn����δ�����������ٴν�������\n");
		return ;
	}
	out("��⵽��������������Ҫ��������\n");
	result = check_and_close_process("/home/share/openvpn/openvpn");
	if(result != 0)
	{
		out("��������ʧ��,�������ն˺�VPN������Ч\n");
		return ;
	}
	return ;
}




static void check_vpn_process_status(void *arg,int timer)
{
	struct _vpn_set  *stream;    
	//int result;
	stream=arg;
	if(stream->vpn_deploy.enable == 0)
	{
		return ;
	}	
	jude_process_exsit_execute(stream);
	return ;
}


static int jude_process_exsit_execute(struct _vpn_set  *stream)
{	char path[200];
	int fd;	
	int result;
	//out("�жϿ�ִ�г����Ƿ����\n");
	result = access("/home/share/openvpn/openvpn",0);
	if(result < 0)
	{
		//out("boot�����ļ�������\n");
		return -1;
	}
	result = detect_process("/home/share/openvpn/openvpn","vpntmp.txt");
	if(result > 0)
	{
		//out("�����Ѿ�����\n");
		if(result > 3)
		{
			logout(INFO, "system", "vpn", "��⵽openvpn�ͻ��˳������������ر��ϴ������Ľ���\r\n");
			result = check_and_close_process("/home/share/openvpn/openvpn");
			if(result != 0)
			{
				out("��������ʧ��,�������ն˺�VPN������Ч\n");
				return -2;
			}
		}
		return 0;
	}
	out("��⵽����δ����\n");
	out("��Ҫ����VPN�ͻ��˳���\n");
	out("�޸��û����������ļ�\n");
	result = set_user_passwd_file(stream);
	if(result < 0)
	{	out("�����û�������ʧ��\n");
		return -3;
	}
	memset(path,0,sizeof(path));
	sprintf(path,"#!/bin/bash\n");
	sprintf(path+strlen(path),"/home/share/openvpn/openvpn /home/share/openvpn/openvpn-client.conf &");
	//sprintf(path+strlen(path),"exit 0");
	fd = open("/home/share/openvpn/run_vpn.sh",O_RDWR|O_CREAT);
	write(fd,path,strlen(path));
	close(fd);
	system("chmod 777 /home/share/openvpn/run_vpn.sh");
	system("/home/share/openvpn/run_vpn.sh");
	return 0;

}



static int set_user_passwd_file(struct _vpn_set *stream)
{
	char s_buf[1024];
	memset(s_buf,0,sizeof(s_buf));
	sprintf(s_buf,"%s\n%s",stream->vpn_deploy.user,stream->vpn_deploy.passwd);
	out("�û������ļ����ݣ�%s\n",s_buf);
	return _t_file_c_write("/home/share/openvpn/pass.txt",s_buf,strlen(s_buf));	
}

static int check_file_and_ca(struct _vpn_set  *stream)
{
	char folder[50];
	int result;
	out("����ļ��Ƿ����\n");
	sprintf(folder,"/home/share/openvpn");
	result = access(folder,0);
	if(result < 0)
	{
		out("/home/share/openvpn/�ļ��в����ڣ��ڴ˴������ļ���\n");
	}
	else
	{
		out("ɾ����Ŀ�겢�ؽ���Ŀ¼\n");
		system("rm -rf /home/share/openvpn");		
	}
	_folder_new("/home/share/","openvpn");
	out("ca.crt��������\n");
	result = check_lib_so_update(stream, (uint8 *)"/home/share/openvpn", (uint8 *)"ca.crt");
	if(result < 0)
	{
		out("����֤���ļ�ʧ��\n");
		return -1;
	}
	result = check_lib_so_update(stream, (uint8 *)"/home/share/openvpn", (uint8 *)"openvpn-client.conf");
	if(result < 0)
	{
		out("����vpn�����ļ�ʧ��\n");
		return -2;
	}
	result = check_lib_so_update(stream, (uint8 *)"/home/share/openvpn", (uint8 *)"pass.txt");
	if(result < 0)
	{
		out("����vpn�����ļ�ʧ��\n");
		return -3;
	}
	result = check_lib_so_update(stream, (uint8 *)"/home/share/openvpn", (uint8 *)"openvpn");
	if(result < 0)
	{
		out("����vpn�����ļ�ʧ��\n");
		return -4;
	}
	return 0;
}


//������޶�̬����Ҫ����
static int check_lib_so_update(struct _vpn_set  *stream,uint8 *path,uint8 *so_name)
{	int result;
	char oper_order[1024];//�ٵ�����
	char path_name[1024];
	char down_name[1024];
	struct stat st ;
	memset(down_name,0,sizeof(down_name));
    sprintf(down_name,"/home/share/exe/%s",so_name);
	sprintf(path_name,"%s/%s",path,so_name);
	out("Ҫ�����ļ�%s\n",path_name);
	result = access(path_name,0);
	if(result >= 0)
	{
		out("%s�������������\n",so_name);
		return 0;
	}
	out("ftp:(��[%s],�û���[%s],����[%s])�����ļ�:[%s]��[%s]\n",stream->vpn_deploy.addr,stream->vpn_deploy.ftp_user,stream->vpn_deploy.ftp_passwd,so_name,down_name);
	if(ftp_get_file(stream->vpn_deploy.addr,stream->vpn_deploy.ftp_user,stream->vpn_deploy.ftp_passwd,(char *)so_name,down_name,stream->vpn_deploy.ftp_port)<0)
	{   out("ftp:����:%sʧ��\n",down_name);
		return DF_ERR_PORT_CORE_FTP_DOWNLOAD_ERR;
	}
	out("���سɹ�\n");
	sync();
	stat(down_name, &st );
    out(" file size = %d\n", st.st_size);
	if(st.st_size == 0)
	{
		out("�ļ���СΪ0\n");
		memset(oper_order,0,sizeof(oper_order));
		sprintf(oper_order,"rm %s",down_name);
		out("�������%s\n",oper_order);
		system(oper_order);
		return -1;
	}
	memset(oper_order,0,sizeof(oper_order));
	sprintf(oper_order,"cp %s %s",down_name,path_name);
	out("�������%s\n",oper_order);
	system(oper_order);
	
	memset(oper_order,0,sizeof(oper_order));
	sprintf(oper_order,"chmod 777 %s",path_name);
	out("�������%s\n",oper_order);
	system(oper_order);
	sync();
	return 0;
}



/*==========================================ctlִ�к���==============================================*/
static int set_vpn_par_file(struct _vpn_set_fd  *id,va_list args)
{	struct _vpn_set  *stream;
	int result;
	int enable;
	int mode;
	uint8 *server;
	int port;
	uint8 *user;
	uint8 *passwd;
	int ftp_port;
	uint8 *ftp_user;
	uint8 *ftp_passwd;
	out("VPN���ú���\n");
	stream = id->dev;
	enable = va_arg(args,int);
	mode = va_arg(args,int);	
	server = va_arg(args,uint8 *);
	port = va_arg(args,int);
	user = va_arg(args,uint8 *);
	passwd = va_arg(args,uint8 *);
	ftp_port = va_arg(args,int);
	ftp_user = va_arg(args,uint8 *);
	ftp_passwd = va_arg(args,uint8 *);

	stream->vpn_deploy.enable = enable;
	stream->vpn_deploy.mode = mode;
	stream->vpn_deploy.port = port;
	stream->vpn_deploy.ftp_port = ftp_port;
	memcpy(stream->vpn_deploy.addr,server,sizeof(stream->vpn_deploy.addr));
	memcpy(stream->vpn_deploy.user,user,sizeof(stream->vpn_deploy.user));
	memcpy(stream->vpn_deploy.passwd,passwd,sizeof(stream->vpn_deploy.passwd));
	memcpy(stream->vpn_deploy.ftp_user,ftp_user,sizeof(stream->vpn_deploy.ftp_user));
	memcpy(stream->vpn_deploy.ftp_passwd,ftp_passwd,sizeof(stream->vpn_deploy.ftp_passwd));

	result =  ty_ctl(stream->deploy_fd,DF_DEPLOY_CM_SET_VPN_PAR,&stream->vpn_deploy);
	if(result < 0)
	{
		out("д��FLASHʧ��\n");
		return -1;
	}
	out("����VPN�������óɹ�\n\n\n");
	out("����ļ��������ļ�\n");
	_queue_add("set_ip",check_vpn_config_file,stream,200); 
	return 0;
}

static int get_vpn_par_file(struct _vpn_set_fd  *id,va_list args)
{	struct _vpn_set  *stream;
	//int result;
	int *enable;
	int *mode;
	uint8 *server;
	int *port;
	uint8 *user;
	uint8 *passwd;
	int *ftp_port;
	uint8 *ftp_user;
	uint8 *ftp_passwd;
	stream = id->dev;
	enable = va_arg(args,int *);
	mode = va_arg(args,int *);
	server = va_arg(args,uint8 *);
	port = va_arg(args,int *);
	user = va_arg(args,uint8 *);
	passwd = va_arg(args,uint8 *);
	ftp_port = va_arg(args,int *);
	ftp_user = va_arg(args,uint8 *);
	ftp_passwd = va_arg(args,uint8 *);

	*enable = stream->vpn_deploy.enable;
	*mode = stream->vpn_deploy.mode;
	*port = stream->vpn_deploy.port;
	*ftp_port = stream->vpn_deploy.ftp_port;
	memcpy(server,stream->vpn_deploy.addr,sizeof(stream->vpn_deploy.addr));
	memcpy(user,stream->vpn_deploy.user,sizeof(stream->vpn_deploy.user));
	memcpy(passwd,stream->vpn_deploy.passwd,sizeof(stream->vpn_deploy.passwd));
	memcpy(ftp_user,stream->vpn_deploy.ftp_user,sizeof(stream->vpn_deploy.ftp_user));
	memcpy(ftp_passwd,stream->vpn_deploy.ftp_passwd,sizeof(stream->vpn_deploy.ftp_passwd));
	return 0;
}