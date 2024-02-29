#include "../inc/usb_file.h"

static int read_ep(const char *path,struct _usb_file_port *ep);
static int read_link(const char *path,const char *name,char *link_name);
static int read_uint8(const char *path,const char *name,uint8 *r_val);
static int read_int(const char *path,const char *name);
static int read_uint16(const char *path,const char *name,uint16 *r_val);
static int read_string(const char *path,const char *name,char *r_string);

static int read_interface(const char *path,struct _usb_file_interface *interface)
{
    char ep_name[DF_USB_FILE_PORT_SIZE][DF_TY_PATH_MAX];
    int i = 0,result = 0;
    char ep_path[DF_TY_PATH_MAX] = {0x00};
    usbfile_out("====在[%s]目录下获取接口描述====\n",path);   
    if(read_uint8(path,"bInterfaceNumber",&interface->bInterfaceNumber) < 0)
    {
        usbfile_out("获取接口号失败\n");
        return -1;
    }
    usbfile_out("接口号:bInterfaceNumber=%d\n",interface->bInterfaceNumber);
    if(read_uint8(path,"bAlternateSetting",&interface->bAlternateSetting) < 0)
    {
        usbfile_out("获取可选设置索引失败\n");
        return -1;
    }
    usbfile_out("可选设置索引:bAlternateSetting=%d\n",interface->bAlternateSetting);
    if(read_uint8(path,"bNumEndpoints",&interface->bNumEndpoints) < 0)
    {
        usbfile_out("获取所用的端点数量失败\n");
        return -1;
    }
    usbfile_out("所用的端点数量:bNumEndpoints=%d\n",interface->bNumEndpoints);
    if(read_uint8(path,"bInterfaceClass",&interface->bInterfaceClass) < 0)
    {
        usbfile_out("获取接口所属类值失败\n");
        return -1;
    }
    usbfile_out("接口所属类值:bInterfaceClass=%d\n",interface->bInterfaceClass);
    if(read_uint8(path,"bInterfaceSubClass",&interface->bInterfaceSubClass) < 0)
    {
        usbfile_out("获取子类码失败\n");
        return -1;
    }
    usbfile_out("子类码:bInterfaceSubClass=%d\n",interface->bInterfaceSubClass);
    if(read_uint8(path,"bInterfaceProtocol",&interface->bInterfaceProtocol) < 0)
    {
        usbfile_out("获取协议失败\n");
        return -1;
    }
    usbfile_out("协议:bInterfaceProtocol=%d\n",interface->bInterfaceProtocol);
    for(i = 0;i < DF_USB_FILE_PORT_SIZE;i++)
    {
        memset(ep_name[i],0,DF_TY_PATH_MAX);
    }
    result = _folder_ls_seek(path,"ep_",ep_name,interface->bNumEndpoints);
    if(result != interface->bNumEndpoints)
    {
        usbfile_out("获取端点目录失败\n");
        return -1;
    }
    for(i = 0;((i < interface->bNumEndpoints)&&(i < DF_USB_FILE_PORT_SIZE));i++)
    {
        memset(ep_path,0,sizeof(ep_path));
        sprintf(ep_path,"%s/%s",path,ep_name[i]);
        if(read_ep(ep_path,&interface->ep[i]) < 0)
        {
            usbfile_out("获取端点[%s],失败\n",ep_name[i]);
            return -3;
        }
    }
    if(read_link(path,"driver",interface->driver)<0)
    {
        usbfile_out("获取驱动失败\n");
        return -2;
    }
    usbfile_out("驱动:driver=%s\n",interface->driver);
    return 0;
}

static int read_ep(const char *path,struct _usb_file_port *ep)
{
    usbfile_out("====在[%s]目录下获取端点描述====\n",path);
    if(read_uint8(path,"bEndpointAddress",&ep->bEndpointAddress) < 0)
    {
        usbfile_out("获取端点描述失败\n");
        return -1;
    }
    usbfile_out("端点描述:bEndpointAddress=0x%02x\n",ep->bEndpointAddress);
    if(read_uint8(path,"bmAttributes",&ep->bmAttributes) < 0)
    {
        usbfile_out("获取传输类型失败\n");
        return -1;
    }
    usbfile_out("传输类型:bmAttributes=0x%02x\n",ep->bmAttributes);
    if(read_uint16(path,"wMaxPacketSize",&ep->wMaxPacketSize) < 0)
    {
        usbfile_out("获取数据包的大小失败\n");
        return -1;
    }
    usbfile_out("数据包的大小:wMaxPacketSize=%d\n",ep->wMaxPacketSize);
    if(read_uint8(path,"bInterval",&ep->bInterval) < 0)
    {
        usbfile_out("获取时间间隙失败\n");
        return -1;
    }
    usbfile_out("时间间隙:bInterval=%d\n",ep->bInterval);
    return 0;
}

static int read_dev(const char *path,struct _usb_file_dev *dev)
{
    usbfile_out("====在[%s]目录下获取设备描述符====\n",path);   
    if(read_uint8(path,"bDeviceClass",&dev->bDeviceClass) < 0)
    {
        usbfile_out("设备类读取失败\n");
        return -1;
    }
    usbfile_out("设备类:bDeviceClass=%d\n",dev->bDeviceClass);
    if(read_uint8(path,"bDeviceSubClass",&dev->bDeviceSubClass) < 0)
    {
        usbfile_out("设备子类读取失败\n");
        return -1;
    }
    usbfile_out("设备子类:bDeviceSubClass=%d\n",dev->bDeviceSubClass);
    if(read_uint8(path,"bDeviceProtocol",&dev->bDeviceProtocol) < 0)
    {
        usbfile_out("协议读取失败\n");
        return -1;
    }
    usbfile_out("协议:bDeviceProtocol=%d\n",dev->bDeviceProtocol);
    if(read_uint8(path,"bMaxPacketSize0",&dev->bMaxPacketSize0) < 0)
    {
        usbfile_out("包大小读取失败\n");
        return -1;
    }
    usbfile_out("包大小:bMaxPacketSize0=%d\n",dev->bMaxPacketSize0);
    if(read_uint8(path,"bNumInterfaces",&dev->bNumInterfaces) < 0)
    {
        usbfile_out("所支持的接口数读取失败\n");
        return -1;
    }
    usbfile_out("[read_dev]interface number :bNumInterfaces= %d \n\n",dev->bNumInterfaces);
    if(read_uint8(path,"bConfigurationValue",&dev->bConfigurationValue) < 0)
    {
        usbfile_out("接口数读取失败\n");
        return -1;
    }
    usbfile_out("接口数:bConfigurationValue=%d\n",dev->bConfigurationValue);
    if(read_uint8(path,"bmAttributes",&dev->bmAttributes) < 0)
    {
        usbfile_out("电源属性读取失败\n");
        return -1;
    }
    usbfile_out("电源属性:bmAttributes=%d\n",dev->bmAttributes);
    if(read_uint8(path,"bNumConfigurations",&dev->bNumConfigurations) < 0)
    {
        usbfile_out("可能配置的描述符数目读取失败\n");
        return -1;
    }
    usbfile_out("可能配置的描述符数目:bNumConfigurations=%d\n",dev->bNumConfigurations);
    if(read_string(path,"speed",dev->speed) < 0)
    {
        usbfile_out("运行速度读取失败\n");
        return -1;
    }
    usbfile_out("运行速度:speed=%s\n",dev->speed);
    if((dev->busnum = read_int(path,"busnum")) < 0)
    {
        usbfile_out("总线号读取失败\n");
        return -1;
    }
    usbfile_out("总线号:busnum=%d\n",dev->busnum);
    if((dev->dev = read_int(path,"busnum")) < 0)
    {
        usbfile_out("设备号读取失败\n");
        return -1;
    }
    usbfile_out("设备号:dev=%d\n",dev->dev);
    if((dev->devnum=read_int(path,"devnum")) < 0)
    {
        usbfile_out("次设备号读取失败\n");
        return -1;
    }
    usbfile_out("次设备号 :devnum=%d\n",dev->devnum);
    if(read_uint16(path,"idVendor",&dev->idVendor) < 0)
    {
        usbfile_out("厂商标志读取失败\n");
        return -1;
    }
    usbfile_out("厂商标志:idVendor=%d\n",dev->idVendor);
    if(read_uint16(path,"idProduct",&dev->idProduct) < 0)
    {
        usbfile_out("产品标志读取失败\n");
        return -1;
    }
    usbfile_out("产品标志:idProduct=%d\n",dev->idProduct);
    if(read_uint16(path,"bcdDevice",&dev->bcdDevice) < 0)
    {
        usbfile_out("设备发行号读取失败\n");
        return -1;
    }
    usbfile_out("设备发行号:bcdDevice=%d\n",dev->bcdDevice);
    if(read_link(path,"driver",dev->driver) < 0)
    {
        usbfile_out("获取驱动失败\n");
        return -2;
    }
    usbfile_out("驱动:driver=%s\n",dev->driver);
    usbfile_out("====在[%s]目录下获取设备描述成功====\n",path);
	usbfile_out("读设备的接口信息\n");
    return 0;
}

static int read_link(const char *path,const char *name,char *link_name)
{
    char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(readlink(file_name, buf, sizeof(buf)) < 0)
    {
        usbfile_out("获取驱动失败\n");
        memcpy(link_name,"none",4);
        return 4;//4418内核不加载大容量存储设备，所以此处无法获取到驱动信息，所以传一个正确值 4为长度
    }
    usbfile_out("read_link=%s\n",buf);
    get_dir_file(buf,link_name);
    return strlen(link_name);
}

static int read_int(const char *path,const char *name)
{
    char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    int result = 0;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf)) < 0)
    {
        return -1;
    }
    sscanf(buf, "%d\n", &result);
    return result;
}

static int read_uint8(const char *path,const char *name,uint8 *r_val)
{
    char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    int result = 0;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf)) < 0)
    {
        usbfile_out("读取文件[%s]失败\n",file_name);
        return -1;
    }
    usbfile_out("buf=%s\n",buf);
    sscanf(buf,"%02x\n",&result);
    *r_val = result;
    return result;
}

static int read_uint16(const char *path,const char *name,uint16 *r_val)
{
    char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    int result = 0;
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf)) < 0)
    {
        usbfile_out("读取文件[%s]失败\n",file_name);
        return -1;
    }
    usbfile_out("==buf=%s\n",buf);
    sscanf(buf,"%04x\n",&result);
    *r_val=result;
    return result;
}

static int read_string(const char *path,const char *name,char *r_string)
{
    char buf[DF_TY_PATH_MAX];
    char file_name[DF_TY_PATH_MAX];
    memset(buf,0,sizeof(buf));
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,name);
    if(read_file(file_name,buf,sizeof(buf)) < 0)
    {
        usbfile_out("读取文件[%s]失败\n",file_name);
        return -1;
    }
    sscanf(buf,"%s\n",r_string);
    return strlen(r_string);
}

int _usb_file_ls_busid(const char *busid)
{
    char path[DF_TY_PATH_MAX] = {0x00};
    memset(path,0,sizeof(path));
    return _folder_ls_seek(DF_USB_FILE_SYS_DIR,busid,NULL,100);
}

int _usb_file_get_bus_dev_n(const char *bus)
{
    DIR *dp = NULL;
    struct dirent *entp = NULL;
    int i = 0,len = 0;
    usbfile_out("获取bus树上设备总数\n");
    if((dp = opendir(DF_USB_FILE_SYS_DIR)) == NULL)
    {
        usbfile_out("该文件夹不存在或者为空\n");
        return 0;
    }
    for(len = strlen(bus),i = 0;;)
    {
        entp = readdir(dp);
        if(entp == NULL)
            break;
        if(memcmp(bus,entp->d_name,len) != 0)
            continue;
        if(juge_usb_dev(entp->d_name) < 0)
            continue;
        i++;
    }
    closedir(dp);
    return i;
}

int _usb_file_exist(const char *busid)
{
	int result;
	char usbpath[DF_TY_PATH_MAX];
	sprintf(usbpath,"%s/%s",DF_USB_FILE_SYS_DIR,busid);
    // printf("lbc ------------------ _usb_file_exist = %s\n",usbpath);
	result = access(usbpath,0);
	return result;
}

int _usb_file_get_infor(const char *busid,struct _usb_file_dev *dev)
{
    char path[DF_TY_PATH_MAX] = {0x00};
    char ep_00[DF_TY_PATH_MAX] = {0x00};
    char interface_path[DF_TY_PATH_MAX] = {0x00};
    int i = 0;
    memset(path,0,sizeof(path));
    sprintf(path,"%s/%s",DF_USB_FILE_SYS_DIR,busid);
    usbfile_out("判断该文件[%s]是否为目录文件\n",path);
    if(_file_pro_type(path) != DF_FILE_PRO_DIR)
    {
        usbfile_out("该文件不是目录文件\n");
        return -1;
    }
    if(read_dev(path,dev) < 0)
    {
        usbfile_out("获取设备描述失败\n");
        return -1;
    }
    memset(ep_00,0,sizeof(ep_00));
    sprintf(ep_00,"%s/%s",path,DF_USB_FILE_EP_00);
    usbfile_out("获取设备的端口0\n");
    if(read_ep(ep_00,&dev->ep) < 0)
    {
        usbfile_out("获取端点0失败\n");
        return -1;
    }
    for(i = 0;(i < dev->bConfigurationValue)&&(i < sizeof(dev->interface)/sizeof(dev->interface[0]));i++)
    {
        memset(interface_path,0,sizeof(interface_path));
        sprintf(interface_path,"%s/%s:%d.%d",DF_USB_FILE_SYS_DIR,busid,dev->bConfigurationValue,i);
        if(read_interface(interface_path,&dev->interface[i]) < 0)
        {
            usbfile_out("获取接口失败\n");
            return -1;
        }
    }
    usbfile_out("获取接口成功\n");
    return 0;
}

int get_busid_busnum(char *busid)
{
	int busnum = 0;
	char path[DF_TY_PATH_MAX] = {0x00};
	sprintf(path, "%s/%s", DF_USB_FILE_SYS_DIR, busid);
	busnum = read_int(path, "busnum");
    
	return busnum;
}

int get_busid_devnum(char *busid)
{
	int devnum = 0;
	char path[DF_TY_PATH_MAX] = {0x00};
	sprintf(path, "%s/%s", DF_USB_FILE_SYS_DIR, busid);
	devnum = read_int(path, "devnum");
    // printf("get_busid_devnum devnum = %d\n",devnum);
	return devnum;
}
