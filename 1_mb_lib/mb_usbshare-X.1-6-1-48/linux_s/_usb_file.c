#define _usb_file_c
#include "_usb_file.h"
int _usb_file_ls_busid(const char *busid)
{   char path[DF_TY_PATH_MAX];
    memset(path,0,sizeof(path));
    return _folder_ls_seek(DF_USB_FILE_SYS_DIR,busid,NULL,100);
}

int _usb_file_get_bus_dev_n(const char *bus)
{   DIR *dp;
    struct dirent *entp;
    int i,len;
    out("��ȡbus�����豸����\n");
    if((dp=opendir(DF_USB_FILE_SYS_DIR))==NULL)
    {   out("���ļ��в����ڻ���Ϊ��\n");
        return 0;
    }
    for(len=strlen(bus),i=0;;)
    {   entp=readdir(dp);   
        if(entp==NULL)
            break;
        if(memcmp(bus,entp->d_name,len)!=0)
            continue;
        if(juge_usb_dev(entp->d_name)<0)
            continue;
        i++;
    }
    closedir(dp);
    return i;    
}


int _usb_file_exist(const char *busid)
{
	int result;
	char usbpath[100];
	sprintf(usbpath,"%s/%s",DF_USB_FILE_SYS_DIR,busid);
	//printf("path : %s\n",usbpath);
	result = access(usbpath,0);
	//printf("access result = %d\n",result);
	return result;

}




int _usb_file_get_infor(const char *busid,struct _usb_file_dev    *dev)
{   char path[DF_TY_PATH_MAX];   
    char ep_00[DF_TY_PATH_MAX];
    char interface_path[DF_TY_PATH_MAX];
    int i;
    memset(path,0,sizeof(path));
    sprintf(path,"%s/%s",DF_USB_FILE_SYS_DIR,busid);
    out("�жϸ��ļ�[%s]�Ƿ�ΪĿ¼�ļ�\n",path);
    if(_file_pro_type(path)!=DF_FILE_PRO_DIR)
    {   out("���ļ�����Ŀ¼�ļ�\n");
        return -1;
    }
    if(read_dev(path,dev)<0)
    {   out("��ȡ�豸����ʧ��\n");
        return -1;
    }
    memset(ep_00,0,sizeof(ep_00));
    sprintf(ep_00,"%s/%s",path,DF_USB_FILE_EP_00);
    out("��ȡ�豸�Ķ˿�0\n");
    if(read_ep(ep_00,&dev->ep)<0)
    {   out("��ȡ�˵�0ʧ��\n");
        return -1;
    }
    for(i=0;(i<dev->bConfigurationValue)&&(i<sizeof(dev->interface)/sizeof(dev->interface[0]));i++)
    {   memset(interface_path,0,sizeof(interface_path));
        sprintf(interface_path,"%s/%s:%d.%d",DF_USB_FILE_SYS_DIR,busid,dev->bConfigurationValue,i);
        if(read_interface(interface_path,&dev->interface[i])<0)
        {   out("��ȡ�ӿ�ʧ��\n");
            return -1;
        } 
    }
    out("��ȡ�ӿڳɹ�\n");
    return 0;
}


static int read_interface(const char *path,struct _usb_file_interface   *interface)
{   char ep_name[DF_USB_FILE_PORT_SIZE][DF_TY_PATH_MAX];
    int i;
    char ep_path[DF_TY_PATH_MAX];
    int result;
    out("====��[%s]Ŀ¼�»�ȡ�ӿ�����====\n",path);   
    if(read_uint8(path,"bInterfaceNumber",&interface->bInterfaceNumber)<0)
    {   out("��ȡ�ӿں�ʧ��\n");
        return -1;
    }
    out("�ӿں�:bInterfaceNumber=%d\n",interface->bInterfaceNumber);
    if(read_uint8(path,"bAlternateSetting",&interface->bAlternateSetting)<0)
    {   out("��ȡ��ѡ��������ʧ��\n");
        return -1;
    }
    out("��ѡ��������:bAlternateSetting=%d\n",interface->bAlternateSetting);
    if(read_uint8(path,"bNumEndpoints",&interface->bNumEndpoints)<0)
    {   out("��ȡ���õĶ˵�����ʧ��\n");
        return -1;
    }
    out("���õĶ˵�����:bNumEndpoints=%d\n",interface->bNumEndpoints);
    if(read_uint8(path,"bInterfaceClass",&interface->bInterfaceClass)<0)
    {   out("��ȡ�ӿ�������ֵʧ��\n");
        return -1;
    }
    out("�ӿ�������ֵ:bInterfaceClass=%d\n",interface->bInterfaceClass);
    if(read_uint8(path,"bInterfaceSubClass",&interface->bInterfaceSubClass)<0)
    {   out("��ȡ������ʧ��\n");
        return -1;
    }
    out("������:bInterfaceSubClass=%d\n",interface->bInterfaceSubClass);
    if(read_uint8(path,"bInterfaceProtocol",&interface->bInterfaceProtocol)<0)
    {   out("��ȡЭ��ʧ��\n");
        return -1;
    }
    out("Э��:bInterfaceProtocol=%d\n",interface->bInterfaceProtocol);
    for(i=0;i<DF_USB_FILE_PORT_SIZE;i++)
        memset(ep_name[i],0,DF_TY_PATH_MAX);
    result=_folder_ls_seek(path,"ep_",ep_name,interface->bNumEndpoints);
    if(result!=interface->bNumEndpoints)
    {   out("��ȡ�˵�Ŀ¼ʧ��\n");
        return -1;
    }
    for(i=0;((i<interface->bNumEndpoints)&&(i<DF_USB_FILE_PORT_SIZE));i++)
    {   memset(ep_path,0,sizeof(ep_path));
        sprintf(ep_path,"%s/%s",path,ep_name[i]);
        if(read_ep(ep_path,&interface->ep[i])<0)
        {   out("��ȡ�˵�[%s],ʧ��\n",ep_name[i]);
            return -3;
        } 
    }
    if(read_link(path,"driver",interface->driver)<0)
    {   out("��ȡ����ʧ��\n");
        return -2;
    }
    out("����:driver=%s\n",interface->driver);
    return 0;
}

static int read_ep(const char *path,struct _usb_file_port   *ep)
{   out("====��[%s]Ŀ¼�»�ȡ�˵�����====\n",path);   
    if(read_uint8(path,"bEndpointAddress",&ep->bEndpointAddress)<0)
    {   out("��ȡ�˵�����ʧ��\n");
        return -1;    
    }
    out("�˵�����:bEndpointAddress=0x%02x\n",ep->bEndpointAddress);
    if(read_uint8(path,"bmAttributes",&ep->bmAttributes)<0)
    {   out("��ȡ��������ʧ��\n");
        return -1;    
    }
    out("��������:bmAttributes=0x%02x\n",ep->bmAttributes);
    if(read_uint16(path,"wMaxPacketSize",&ep->wMaxPacketSize)<0)
    {   out("��ȡ���ݰ��Ĵ�Сʧ��\n");
        return -1;    
    }
    out("���ݰ��Ĵ�С:wMaxPacketSize=%d\n",ep->wMaxPacketSize);
    if(read_uint8(path,"bInterval",&ep->bInterval)<0)
    {   out("��ȡʱ���϶ʧ��\n");
        return -1;    
    }
    out("ʱ���϶:bInterval=%d\n",ep->bInterval);
    return 0;
}

static int read_dev(const char *path,struct _usb_file_dev    *dev)
{   out("====��[%s]Ŀ¼�»�ȡ�豸������====\n",path);   
    if(read_uint8(path,"bDeviceClass",&dev->bDeviceClass)<0)
    {   out("�豸���ȡʧ��\n");
        return -1;
    }
    out("�豸��:bDeviceClass=%d\n",dev->bDeviceClass);
    if(read_uint8(path,"bDeviceSubClass",&dev->bDeviceSubClass)<0)
    {   out("�豸�����ȡʧ��\n");
        return -1;
    }
    out("�豸����:bDeviceSubClass=%d\n",dev->bDeviceSubClass);
    if(read_uint8(path,"bDeviceProtocol",&dev->bDeviceProtocol)<0)
    {   out("Э���ȡʧ��\n");
        return -1;
    }
    out("Э��:bDeviceProtocol=%d\n",dev->bDeviceProtocol);
    if(read_uint8(path,"bMaxPacketSize0",&dev->bMaxPacketSize0)<0)
    {   out("����С��ȡʧ��\n");
        return -1;
    }
    out("����С:bMaxPacketSize0=%d\n",dev->bMaxPacketSize0);
    if(read_uint8(path,"bNumInterfaces",&dev->bNumInterfaces)<0)
    {   out("��֧�ֵĽӿ�����ȡʧ��\n");
        return -1;
    }
    out("[read_dev]interface number :bNumInterfaces= %d \n\n",dev->bNumInterfaces);
    if(read_uint8(path,"bConfigurationValue",&dev->bConfigurationValue)<0)
    {   out("�ӿ�����ȡʧ��\n");
        return -1;
    }
    out("�ӿ���:bConfigurationValue=%d\n",dev->bConfigurationValue);
    if(read_uint8(path,"bmAttributes",&dev->bmAttributes)<0)
    {   out("��Դ���Զ�ȡʧ��\n");
        return -1;
    }
    out("��Դ����:bmAttributes=%d\n",dev->bmAttributes);
    if(read_uint8(path,"bNumConfigurations",&dev->bNumConfigurations)<0)
    {   out("�������õ���������Ŀ��ȡʧ��\n");
        return -1;
    }
    out("�������õ���������Ŀ:bNumConfigurations=%d\n",dev->bNumConfigurations);
    if(read_string(path,"speed",dev->speed)<0)
    {   out("�����ٶȶ�ȡʧ��\n");
        return -1;
    }
    out("�����ٶ�:speed=%s\n",dev->speed);
    dev->busnum=read_int(path,"busnum");
    if(dev->busnum<0)
    {   out("���ߺŶ�ȡʧ��\n");
        return -1;
    }
    out("���ߺ�:busnum=%d\n",dev->busnum);
    dev->dev=read_int(path,"busnum");
    if(dev->dev<0)
    {   out("�豸�Ŷ�ȡʧ��\n");
        return -1;
    }
    out("�豸��:dev=%d\n",dev->dev);
    dev->devnum=read_int(path,"devnum");
    if(dev->devnum<0)
    {   out("���豸�Ŷ�ȡʧ��\n");
        return -1;
    }
    out("���豸�� :devnum=%d\n",dev->devnum);
    if(read_uint16(path,"idVendor",&dev->idVendor)<0)
    {   out("���̱�־��ȡʧ��\n");
        return -1;
    }
    out("���̱�־:idVendor=%d\n",dev->idVendor);
    if(read_uint16(path,"idProduct",&dev->idProduct)<0)
    {   out("��Ʒ��־��ȡʧ��\n");
        return -1;
    }
    out("��Ʒ��־:idProduct=%d\n",dev->idProduct);
    if(read_uint16(path,"bcdDevice",&dev->bcdDevice)<0)
    {   out("�豸���кŶ�ȡʧ��\n");
        return -1;
    }
    out("�豸���к�:bcdDevice=%d\n",dev->bcdDevice);
    if(read_link(path,"driver",dev->driver)<0)
    {   out("��ȡ����ʧ��\n");
        return -2;
    }
    out("����:driver=%s\n",dev->driver);
    out("====��[%s]Ŀ¼�»�ȡ�豸�����ɹ�====\n",path);  
	out("���豸�Ľӿ���Ϣ\n");

    return 0;   
}



static int read_link(const char *path,const char *name,char *link_name)
{   char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    //int result;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(readlink(file_name, buf, sizeof(buf))<0)
    {   out("��ȡ����ʧ��\n");
        memcpy(link_name,"none",4);
        return 4;//4418�ں˲����ش������洢�豸�����Դ˴��޷���ȡ��������Ϣ�����Դ�һ����ȷֵ 4Ϊ����
    }    
    out("read_link=%s\n",buf);
    get_dir_file(buf,link_name);
    return strlen(link_name); 
}

static int read_int(const char *path,const char *name)
{   char buf[200];
    char file_name[DF_TY_PATH_MAX];
    int result;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf))<0)
    {   //out("��ȡ�ļ�[%s]ʧ��\n",file_name);
        return -1;
    }
    sscanf(buf, "%d\n", &result);
    return result;
}

static int read_uint8(const char *path,const char *name,uint8 *r_val)
{   char buf[200];
    char file_name[DF_TY_PATH_MAX];
    int result;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf))<0)
    {   out("��ȡ�ļ�[%s]ʧ��\n",file_name);
        return -1;
    }
    out("buf=%s\n",buf);
    sscanf(buf,"%02x\n",&result);
    *r_val=result;
    return result;
}

static int read_uint16(const char *path,const char *name,uint16 *r_val)
{   char buf[200];
    char file_name[DF_TY_PATH_MAX];
    int result;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf))<0)
    {   out("��ȡ�ļ�[%s]ʧ��\n",file_name);
        return -1;
    }
    out("==buf=%s\n",buf);
    sscanf(buf,"%04x\n",&result);
    *r_val=result;
    return result;
}




static int read_string(const char *path,const char *name,char *r_string)
{   char buf[200];
    char file_name[DF_TY_PATH_MAX];
    //int result;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf))<0)
    {   out("��ȡ�ļ�[%s]ʧ��\n",file_name);
        return -1;
    }
    sscanf(buf,"%s\n",r_string);
    return strlen(r_string);
}



//static int read_int(const char *path, const char *name)
//{
//	char buf[200];
//	char file_name[DF_TY_PATH_MAX];
//	int result;
//	memset(buf, 0, sizeof(buf));
//	memset(file_name, 0, sizeof(file_name));
//	sprintf(file_name, "%s/%s", path, name);
//	if (read_file(file_name, buf, sizeof(buf))<0)
//	{
//		out("��ȡ�ļ�[%s]ʧ��\n", file_name);
//		return -1;
//	}
//	sscanf(buf, "%d", &result);
//	return result;
//}

int get_busid_busnum(char *busid)
{
	int busnum = 0;
	char path[200];
	sprintf(path, "%s/%s", DF_USB_FILE_SYS_DIR, busid);
	busnum = read_int(path, "busnum");
	return busnum;
}

int get_busid_devnum(char *busid)
{
	int devnum = 0;
	char path[200];
	sprintf(path, "%s/%s", DF_USB_FILE_SYS_DIR, busid);
	devnum = read_int(path, "devnum");
	return devnum;
}