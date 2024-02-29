#define _lib_file_c
#include "_lib_file.h"

int lib_file_extract(const char *file)
{   //uint8 *soft_buf;
    struct _lib_data_file     infor;
    //char soft_file[200];   
    int result;
    out("��ȡ�ļ��Ļ�����Ϣ\n");
    memset(&infor,0,sizeof(infor));
    if(lib_file_get_infor(file,&infor)<0)
    {   out("��ȡ��Ϣͷʧ��\n");
        return -1;
    }
	result = extract_all_lib_file(file,infor.size,infor.lib_num);
	if(result <0)
	{
		out("��ȡlib���ļ�ʧ��\n");
		return -1;
	}
    return 0;
}

static int extract_all_lib_file(const char *file,long size,int lib_num)
{
	int fd,i;
	int lib_fd;
	char libstart[10];
	char libend[10];
	char libsize[4];
	int lib_size;
	char lib_path[50];
	char lib_name[15];
	int offset=0;
	fd = open(file,O_RDWR); 
    if(fd<0)
    {   out("file=%s,�򿪴���\n",file);
        return fd;
    }
	uint8 buf[size+100];
	lseek(fd,102,SEEK_SET);
    memset(buf,0,sizeof(buf));
    read(fd,buf,sizeof(buf));
	out("�����ļ�ͷ��Ϣ:");
	//for(i=0;i<size-102;i++)
	//{
	//	printf("%02x ",buf[i]);
	//}
	//printf("\n\n\n\n");
    close(fd);
	for(i=0;i<lib_num;i++)
	{
		memset(libstart,0,sizeof(libstart));
		memset(libend,0,sizeof(libend));
		memset(lib_name,0,sizeof(lib_name));
		memset(libsize,0,sizeof(libsize));
		memcpy(libstart,buf+offset,10);
		out("��ӡ����ʼͷ��%s\n",libstart);
		memcpy(lib_name,buf+offset+10,15);
		out("��ӡ�����ƣ�%s\n",lib_name);
		memcpy(libsize,buf+offset+10+15,4);
		lib_size =  libsize[0]*0x1000000+libsize[1]*0x10000+libsize[2]*0x100+libsize[3];
		out("��ӡ���С:%d\n",lib_size);
		char lib_data[lib_size+100];
		memset(lib_data,0,sizeof(lib_data));
		memcpy(lib_data,buf+offset+10+15+4,lib_size);
		//for(j=0;j<lib_size;j++)
		//{
		//	printf("%02x ",lib_data[j]);
		//}
		//printf("\n");
		memcpy(libend,buf+offset+10+15+4+lib_size,10);
		out("��ӡ�������Ϣ��%s\n",libend);
		offset = offset+10+15+4+10+lib_size;
		out("���������еĶ�̬��\n");
		memset(lib_path,0,sizeof(lib_path));
		sprintf(lib_path,"/lib/%s.so",lib_name);
		lib_fd=open(lib_path,O_RDWR|O_CREAT);
        if(lib_fd<0)
        {   out("�����ļ�:%s,ʧ��\n",lib_path);
            return -1;
        }    
        write(lib_fd,lib_data,lib_size);
        close(lib_fd);
        file_mod(lib_path);
	}
	return 0;
}

int lib_file_get_infor(const char *file,struct	_lib_data_file *infor)
{   int fd;
	//int i; 
    struct stat pro;
    uint8 buf[102];
    fd=open(file,O_RDWR); 
    if(fd<0)
    {   out("file=%s,�򿪴���\n",file);
        return fd;
    }
    memset(&pro,0,sizeof(pro));
    stat(file,&pro);
	out("pro.st_size  = %d\n",pro.st_size);
    if(pro.st_size<100)
    {   out("��Ϣͷ���Ȳ���,pro.st_size=%d\n",pro.st_size);
		close(fd);
        return -2;
    }
	infor->size = pro.st_size;
    lseek(fd,0,SEEK_SET);
    memset(buf,0,sizeof(buf));
    read(fd,buf,sizeof(buf));
	out("�����ļ�ͷ��Ϣ:");
	//for(i=0;i<102;i++)
	//{
	//	printf("%02x ",buf[i]);
	//}
	//printf("\n\n\n\n");
    close(fd);
    if(get_s_infor(infor,buf)<0)
    {   out("��ȡ��Ϣͷʧ��\n");
        return -3;
    }
    out("��ȡ��Ϣͷ�ɹ�\n");
    return 0;
}



static int get_s_infor(struct	_lib_data_file		*infor,uint8 *buf)		//��ȡ��Ϣͷ
{	//int buf_len,i_size;
	//uint16 crc_val,crc;
	char tmp[200];
	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,buf,20);
	out("�����ļ�ͷ��Ϣ��%s\n",tmp);
	out("%s------------------��Ϣ�μ��------------------\n",DF_FORMATER_TAB);
	if(memcmp(tmp,"MB_DATA_FILE",strlen("MB_DATA_FILE"))!=0)
	{	out("%s%s��Ϣ�μ�����\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//��Ϣͷ����
	}
	memcpy(infor->head,tmp,20);

	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,buf+20,10);
	out("�����ļ��汾��Ϣ��%s\n",tmp);
	out("%s------------------��Ϣ�μ��------------------\n",DF_FORMATER_TAB);
	if(strlen(tmp)<6)
	{	out("%s%s��Ϣ�μ�����\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//��Ϣͷ����
	}
	memcpy(infor->version,tmp,10);

	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,buf+30,15);
	out("�����ļ�������Ϣ��%s\n",tmp);
	out("%s------------------��Ϣ�μ��------------------\n",DF_FORMATER_TAB);
	if(strlen(tmp)<10)
	{	out("%s%s��Ϣ�μ�����\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
		return _INFOR_ERR_CUT;							//��Ϣͷ����
	}
	memcpy(infor->time,tmp,15);

	memcpy(infor->hold,buf+45,55);

	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,buf+100,2);
	infor->lib_num = tmp[0]*0x100+tmp[1];
	out("���ļ�������%d\n",infor->lib_num);
	if(infor->lib_num<2)
	{
		out("���ļ���������\n");
		out("%s%s��Ϣ�μ�����\r\n",DF_FORMATER_TAB,DF_FORMATER_ERR);
		out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);

	}
   
	out("%s��Ϣ�μ��ɹ�\r\n",DF_FORMATER_SUC);
	out("%s----------------------------------------------\r\n",DF_FORMATER_TAB);
	return 0;
}












