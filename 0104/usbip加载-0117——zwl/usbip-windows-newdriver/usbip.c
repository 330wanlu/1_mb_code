/*
	$Id: usbip.c 176 2011-04-02 21:25:46Z arjanmels $
*/
#include "usbip.h"
#include "getopt.h"
#define _GNU_SOURCE

static const char version[] = "usbip for windows ($Id: usbip.c 176 2011-04-02 21:25:46Z arjanmels $)";

int usbip_use_debug=1;
int usbip_use_syslog=1;
int usbip_use_stderr=1;




static const struct option longopts[] = {
	{"attach",	no_argument,	NULL, 'a'},
	{"attachall",	no_argument,	NULL, 'x'},
	{"detach",	no_argument,	NULL, 'd'},
	{"port",	no_argument,	NULL, 'p'},
	{"list",	no_argument,	NULL, 'l'},
	{"version",	no_argument,	NULL, 'v'},
	{"help",	no_argument,	NULL, 'h'},
	{"debug",	no_argument,	NULL, 'D'},
	{"syslog",	no_argument,	NULL, 'S'},
	{NULL,		0,		NULL,  0}
};

enum {
	CMD_ATTACH = 1,
	CMD_ATTACHALL,
	CMD_DETACH,
	CMD_PORT,
	CMD_LIST,
	CMD_HELP,
	CMD_VERSION
};


/**
*@desc:sockets5代理C语言实现
*@parm_in:sockfd:connect连接服务器后的socket套接字	ip:socket5报文头中要绑定的目的IP; port:sockets5报文头中要绑定的端口 flag:测试用
*@parm_out:NULL
*@return:
*/

int socket_send(SOCKET socketfd , unsigned char *buf,int len)
{
	int i_AllSendLen = len , i_SendedLen = 0;
	int i = 0;
	while (i_SendedLen < i_AllSendLen){
		if ((i = send(socketfd, (const char*)buf + i_SendedLen, len - i_SendedLen, 0)) == SOCKET_ERROR){
			return -1;
		}
		i_SendedLen += i;
		Sleep(10);
	}
	return len;
}

int socket_recv(SOCKET sockfd, unsigned char *buf,int len)
{
	int recv_len = 0, totoal = 0;
	if (!len)
		return -1;
	do{
		recv_len = recv(sockfd, (const char*)buf, len, 0);
		//<=-1 错误码判断
		if (recv_len <= SOCKET_ERROR){
			/*if (errno == WSAEINTR || errno == WSAEWOULDBLOCK || errno == EAGAIN)*/
			return -1;
		}
		if (recv_len == 0)
			return 0;
		buf = buf + recv_len;
		len -= recv_len;
		totoal += recv_len;
		Sleep(10);
	} while (len > 0);
	return totoal;
}

void socket_close(SOCKET socketfd)
{
	shutdown(socketfd, SD_BOTH);
	closesocket(socketfd);
}

int deal_sockets5(SOCKET sockfd, unsigned char *ip,int port,int flag)
{
	unsigned char init_send[4] = { 0 };
	unsigned char init_recv[3] = { 0 };
	unsigned char bind_pack[100] = { 0 };
	unsigned char bind_recv[140] = { 0 };
	int ip1, ip2, ip3, ip4;
	int addr_type = 0;
	int len = 0;
	memcpy(init_send, "\x05\x01\x00", 3);
	if (socket_send(sockfd, init_send, 3) != 3)
	{
		WriteLogFile("deal_sockets5 send init packet err\n");
		goto ERR;
	}
	//接收应答包
	if (socket_recv(sockfd, init_recv, 2) != 2)
	{
		WriteLogFile("deal_sockets5 recv init packet err\n");
		goto ERR;
	}
	//判断应答结果
	if (memcmp(init_recv, "\x05\x00", 2) != 0)
	{
		WriteLogFile("deal_sockets5 recv init packet not success\n");
		goto ERR;
	}
	memcpy(bind_pack, "\x05\x01\x00\x01", 4);
	
	sscanf((const char*)ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	bind_pack[4] = ip1;
	bind_pack[5] = ip2;
	bind_pack[6] = ip3;
	bind_pack[7] = ip4;
	bind_pack[8] = port / 256;
	bind_pack[9] = port % 256;
	if (socket_send(sockfd, bind_pack, 10) != 10)
	{
		WriteLogFile("deal_sockets5 send bind packet err\n");
		goto ERR;
	}
	//接收应答并判断是否成功且判断返回ip为什么类型进一步的读取
	socket_recv(sockfd, bind_recv, 4);

	if (memcmp(bind_recv, "\x05\x00", 2) != 0)
	{
		WriteLogFile("deal_sockets5 recv bind packet sucess but the result is not right\n");
		goto ERR;
	}
	addr_type = bind_recv[3];
	
	switch (addr_type)
	{
	case 1://IPV4
		WriteLogFile("sockets5 server response addr type is ipv4");
		socket_recv(sockfd, bind_recv+4, 6);
		break;
	case 3://domain 
		WriteLogFile("sockets5 server response addr type is domain");
		socket_recv(sockfd, bind_recv + 4, 1);
		len = bind_recv[4];
		socket_recv(sockfd, bind_recv + 5, len+2);
		break;
	case 4://IPV6
		WriteLogFile("sockets5 server response addr type is ipv6");
		socket_recv(sockfd, bind_recv + 4, 18);
		break;
	default:
		break;
	}
	//下面是通过select方式一个字节一个字节接收完所有的应答包，为非常规手段
	/*struct timeval tmOut;
	tmOut.tv_sec = 0;
	tmOut.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	int nRet;
	char tmp[2];
	memset(tmp, 0, sizeof(tmp));
	while (1)
	{
		out("still have data!");
		nRet = select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
		if (nRet == 0)
			break;
		recv(sockfd, tmp, 1, 0);
	}*/
	return 0;
ERR:
	socket_close(sockfd);
	return -1;
}

unsigned int parse_opt(int argc, char *argv[])
{
	int cmd = 0;

	for (;;) {
		int c;
		int index = 0;
		if((argc == 6)&&(memcmp(argv[1],"-a",2)==0))
		{	info("attach new cmd");
			cmd = CMD_ATTACH;
			break;
		}


		c = getopt_long(argc, argv, "adplvhDSx", longopts, &index);

		if (c == -1)
		{	info("getopt_long c == -1");
			break;
		}

		switch(c) {
			case 'a':
				if (!cmd)
					cmd = CMD_HELP;//老命令不支持
				else
					cmd = CMD_HELP;
				break;
			case 'd':
				if (!cmd)
					cmd = CMD_DETACH;
				else
					cmd = CMD_HELP;
				break;
			case 'p':
				if (!cmd)
					cmd = CMD_PORT;
				else
					cmd = CMD_HELP;
				break;
			case 'l':
				if (!cmd)
					cmd = CMD_LIST;
				else
					cmd = CMD_HELP;
				break;
			case 'v':
				if (!cmd)
					cmd = CMD_VERSION;
				else
					cmd = CMD_HELP;
				break;
			case 'x':
				if(!cmd)
					cmd = CMD_ATTACHALL;
				else
					cmd = CMD_HELP;
				break;
			case 'h':
				cmd = CMD_HELP;
				break;
			case 'D':
				usbip_use_debug = 1;
				break;
			case 'S':
				usbip_use_syslog = 1;
				break;
			case '?':
				break;
			default:
				err("getopt");
		}
	}
	if(!cmd)
		cmd = CMD_HELP;
	return cmd;
}

//获取busid
static int get_busid(SOCKET sockfd,int port,unsigned char *busid)
{
	char sendbuf[1024] = {0};
	unsigned int ret;
	unsigned long len_r;
	unsigned char head[4] = {0};
	unsigned char recv_len[4] = {0};
	char str_port[5] = {0};
	sprintf(str_port,"%04d",port);
	memcpy(sendbuf,"\x01\x06\x80\x07",4);
	memcpy(sendbuf+4,"\x00\x00\x00\x04",4);
	memcpy(sendbuf+8,str_port,4);
	ret = usbip_send(sockfd, (void *) sendbuf, 12);
	if (ret < 0) {
		WriteLogFile("send get busid err");
		return -1;
	}

	//接收返回消息
	ret = usbip_recv(sockfd, (void *)head, 4);
	if (ret < 0) {
		WriteLogFile("recv busid head err");
		return -1;
	}
	if(memcmp(head,"\x01\x06\x00\x07",4) != 0)
	{
		WriteLogFile("recv busid head protocol err");
		return -1;
	}

	// len
	ret = usbip_recv(sockfd, (void *)recv_len, 4);
	if (ret < 0) {
		WriteLogFile("recv busid len err");
		return -1;
	}
	len_r =(unsigned long)recv_len[0]*0x1000000+(unsigned long)recv_len[1]*0x10000+(unsigned long)recv_len[2] * 256 + (unsigned long)recv_len[3];
	//busid
	ret = usbip_recv(sockfd, (void *)busid, len_r);
	if (ret < 0) {
		WriteLogFile("recv busid  err");
		return -1;
	}
	return 0;
}
//static int ParseErrCode(int err_code)
//{
//	int ret = 0;
//	switch(err_code){
//	case -103:
//		break;
//	case -104:
//		break;
//	case -105:
//		break;
//	default:
//		ret = 0;
//	}
//}
static int ResultJsonParse(char *JsonBuf)
{
	
	cJSON * root = NULL;
	cJSON *array_obj,*arr_item,*item = NULL;

	if(JsonBuf == NULL)
		return -1;
	root = cJSON_Parse(JsonBuf);
	if(root == NULL)
		return -1;
	array_obj = cJSON_GetObjectItem(root, "data");
	if(array_obj == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	arr_item = cJSON_GetArrayItem(array_obj,0);
	if(arr_item == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	item = cJSON_GetObjectItem(arr_item, "err_code");
	if(item == NULL)//表示无错误
	{
		WriteLogFile("err_code:NULL\n");
		cJSON_Delete(root);
		return 0;
	}
	
	WriteLogFile("ResultJsonParse err code:%s\n",item->valuestring);


	return atoi(item->valuestring);

}
//发送申请使用端口指令
static int apply_for_port_use(SOCKET sockfd,int port,char *trans_server,char *trans_port,char * clientid,char * oper_id)
{
	char sendbuf[1024] = {0};
	char str_port[10] = {0};
	char head[5] = {0};
	unsigned char recv_len[5] = {0};
	char recvbuf[1024] = {0};
	unsigned long len_r;
	char *g_buf = NULL;
	int ret;
	cJSON *json = NULL;
	cJSON *dir1, *dir2;		//组包使用
	cJSON *dir3, *dir4, *dir4_1;
	memcpy(sendbuf,"\x01\x06\x80\x07",4);

	
	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USBSHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_REQUEST);
	cJSON_AddStringToObject(dir1, "cmd", "apply_use");
	cJSON_AddStringToObject(dir1, "source_topic", "");
	cJSON_AddStringToObject(dir1, "random", "987654321");
	cJSON_AddStringToObject(dir1, "result", "s");

	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	sprintf(str_port,"%d",port);
	cJSON_AddStringToObject(dir3, "usb_port", str_port);

	cJSON_AddStringToObject(dir3, "line_type", "1");
	/*cJSON_AddStringToObject(dir3, "tran_server", trans_server);
	cJSON_AddStringToObject(dir3, "tran_port", trans_port);*/
	cJSON_AddStringToObject(dir3, "client_id", clientid);
	cJSON_AddStringToObject(dir3, "oper_id", oper_id);


	g_buf = cJSON_Print(json);

	sendbuf[6] = strlen(g_buf)/256;sendbuf[7] = strlen(g_buf)%256;
	memcpy(sendbuf+8,g_buf,strlen(g_buf));
	ret = usbip_send(sockfd, (void *) sendbuf, strlen(g_buf)+8);
	if (ret < 0) {
		WriteLogFile("apply_for_port_use send  err");
		return USBIP_ERROR_APPLY_TER_USE;
	}
	//接收返回消息
	ret = usbip_recv(sockfd, (void *)head, 4);
	if (ret < 0) {
		WriteLogFile("apply_for_port_use recv  err");
		return -1;
	}
	if(memcmp(head,"\x01\x06\x00\x07",4) != 0)
	{
		WriteLogFile("apply_for_port_use recv head protocol err");
		return USBIP_ERROR_APPLY_TER_USE;
	}
	//json len
	ret = usbip_recv(sockfd, (void *)recv_len, 4);
	if (ret < 0) {
		WriteLogFile("apply_for_port_use recv len err");
		return USBIP_ERROR_APPLY_TER_USE;
	}
//	WriteLogFile("recv len:%02x %02x %02x %02x ",recv_len);
	len_r =(unsigned long)recv_len[0]*0x1000000+(unsigned long)recv_len[1]*0x10000+(unsigned long)recv_len[2] * 256 + (unsigned long)recv_len[3];

	WriteLogFile("recv len :%d",len_r);
	// json buf
	ret = usbip_recv(sockfd, (void *) &recvbuf, len_r);
	if (ret < 0) {
		WriteLogFile("apply_for_port_use recv jsonbody err");
		return USBIP_ERROR_APPLY_TER_USE;
	}
	return ResultJsonParse(recvbuf);
	//WriteLogFile("recv 0007 :%s\n",recvbuf);
	//return 0;
}
static int query_connect_server(SOCKET sockfd, char * random)
{
	int ret;
	char sendbuf[100] = { 0 };
	char rep[8]={0};
	sendbuf[0] = 0x01;
	sendbuf[1] = 0x06;
	sendbuf[2] = 0x00;
	sendbuf[3] = ((strlen(random)+6) & 0xff);
	sendbuf[4] = 0x00;
	sendbuf[5] = 0x02;
	memcpy(sendbuf + 6, random, strlen(random));
	ret = usbip_send(sockfd, (void *) sendbuf, strlen(random)+6);
	if (ret < 0) {
		WriteLogFile("send connect  err");
		err("send connect err");
		return -1;
	}

	ret = usbip_recv(sockfd, (void *) &rep, sizeof(rep));
	if (ret < 0) {
		WriteLogFile("recv connect err");
		return -1;
	}

	info("query_connect_server success\n");
	return 0;
}
static int query_interface_8009(SOCKET sockfd, char * busid, struct usb_interface * uinf0,int trans_type,int  dev_port,char *user,char *oper_id)
{
	int ret;
	char sendbuf[128] = {0};
	char head[50] = {0};
	struct op_devlist_reply rep;
	uint16_t code = OP_REP_DEVLIST;
	uint32_t i,j,k;
	char product_name[100];
	char class_name[100];
	struct usb_device udev;
	struct usb_interface uinf;
	int found=0,len = 0;

	memset(&rep, 0, sizeof(rep));

	WriteLogFile("dev_port = %d",dev_port);
	memcpy(sendbuf,"\x01\x06\x80\x09\x00\x00\x00\x00",8);
	sendbuf[8] = dev_port / 0x1000000; sendbuf[9] = dev_port / 0x10000;
	sendbuf[10] = dev_port/ 256; sendbuf[11] = dev_port % 256;
	// len = strlen(user) + 12;
	// if(len > sizeof(sendbuf))
	// 	len = sizeof(sendbuf);
	// len = 32;
	memcpy(&sendbuf[12],user,32);
	memcpy(&sendbuf[12+32],oper_id,32);
	// WriteLogFile("111111111");
	ret = usbip_send(sockfd, (void *) sendbuf, 12 + 32 + 32);
	if (ret < 0) {
		WriteLogFile("send connect1 err");
		return -1;
	}
	WriteLogFile("22222222		\n");
	ret = usbip_recv(sockfd, (void *) head, 8);
	if (ret < 0) {
		WriteLogFile("recv op_devlist!!!");
		return -2;
	}
	WriteLogFile("888888   head = %s\n",head);
	
	for (k = 0; k < 8; k++)
	{
		WriteLogFile("%02x \t",head[k]);
	}
	WriteLogFile("\r\n");

	if(memcmp(head,"\x01\x06\x00\x09\x00\x00\x00\x00",8)!= 0)
	{
		if(memcmp(head,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01",8) == 0)//端口未授权返回
		{
			WriteLogFile("55555");
			return -6;
		}

		if(memcmp(head,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02",8) == 0)//端口是机械臂
		{
			WriteLogFile("666666");
			return -7;
		}
		WriteLogFile("777777");
		return -3;
	}
	memset(&udev, 0, sizeof(udev));
	// WriteLogFile("33333333");
	ret = usbip_recv(sockfd, (void *) &udev, sizeof(udev));
	if (ret < 0) {
		
		WriteLogFile("recv usb_device ");
		return -4;
	}
	// WriteLogFile("44444444");
	pack_usb_device(0, &udev);
	usbip_names_get_product(product_name, sizeof(product_name),
			udev.idVendor, udev.idProduct);
	usbip_names_get_class(class_name, sizeof(class_name), udev.bDeviceClass,
			udev.bDeviceSubClass, udev.bDeviceProtocol);

	WriteLogFile("busid %8s:    product_name%s", udev.busid, product_name);
	WriteLogFile("path %8s: %s", " ", udev.path);
	WriteLogFile("class_name %8s: %s", " ", class_name);
	for (j=0; j < udev.bNumInterfaces; j++) {

		ret = usbip_recv(sockfd, (void *) &uinf, sizeof(uinf));
		if (ret < 0) {
			WriteLogFile("recv usb_interface[%d]", j);
			return -5;
		}

		pack_usb_interface(0, &uinf);
		/*if(!strcmp(udev.busid, busid)&&j==0){
			memcpy(uinf0, &uinf, sizeof(uinf));
			found=1;
		}*/

		printf("###########copy#########\n");
		memcpy(uinf0, &uinf, sizeof(uinf));
		usbip_names_get_class(class_name, sizeof(class_name),
				uinf.bInterfaceClass,
				uinf.bInterfaceSubClass,
				uinf.bInterfaceProtocol);

		dbg("%8s: %2d - %s", " ", j, class_name);
		WriteLogFile("%8s: %2d - %s", " ", j, class_name);
	}
	memcpy(busid,udev.busid,strlen(udev.busid));
	
	return 0;
}

static int query_interface0(SOCKET sockfd, char * busid, struct usb_interface * uinf0,int trans_type,int  dev_port)
{
	int ret;
	struct op_devlist_reply rep;
	uint16_t code = OP_REP_DEVLIST;
	uint32_t i,j;
	char product_name[100];
	char class_name[100];
	struct usb_device udev;
	struct usb_interface uinf;
	int found=0;

	memset(&rep, 0, sizeof(rep));


	/*ret = usbip_send_op_common(sockfd, OP_ONLINE, dev_port);

	if (ret < 0) {
		WriteLogFile("send dev_open_port err");
		return -1;
	}*/


	ret = usbip_send_op_common(sockfd, OP_REQ_DEVLIST, 0);
	if (ret < 0) {
		WriteLogFile("send op_common");
		return -1;
	}

	ret = usbip_recv_op_common(sockfd, &code);
	if (ret < 0) {
		WriteLogFile("recv op_common");
		return -1;
	}

	ret = usbip_recv(sockfd, (void *) &rep, sizeof(rep));
	if (ret < 0) {
		WriteLogFile("recv op_devlist");
		return -1;
	}

	PACK_OP_DEVLIST_REPLY(0, &rep);
	dbg("exportable %d devices", rep.ndev);
	
	for (i=0; i < rep.ndev; i++) {

		memset(&udev, 0, sizeof(udev));

		ret = usbip_recv(sockfd, (void *) &udev, sizeof(udev));
		if (ret < 0) {
			WriteLogFile("recv usb_device[%d]", i);
			return -1;
		}
		pack_usb_device(0, &udev);
		usbip_names_get_product(product_name, sizeof(product_name),
				udev.idVendor, udev.idProduct);
		usbip_names_get_class(class_name, sizeof(class_name), udev.bDeviceClass,
				udev.bDeviceSubClass, udev.bDeviceProtocol);

		dbg("%8s: %s", udev.busid, product_name);
		dbg("%8s: %s", " ", udev.path);
		dbg("%8s: %s", " ", class_name);
		WriteLogFile("%8s: %s", udev.busid, product_name);
		WriteLogFile("%8s: %s", " ", udev.path);
		WriteLogFile("%8s: %s", " ", class_name);
		WriteLogFile("----------------lbc vid = %.02x,pid = %.02x",udev.idVendor,udev.idProduct);
		for (j=0; j < udev.bNumInterfaces; j++) {

			ret = usbip_recv(sockfd, (void *) &uinf, sizeof(uinf));
			if (ret < 0) {
				WriteLogFile("recv usb_interface[%d]", j);
				return -1;
			}

			pack_usb_interface(0, &uinf);
			if(!strcmp(udev.busid, busid)&&j==0){
				memcpy(uinf0, &uinf, sizeof(uinf));
				found=1;
			}
			usbip_names_get_class(class_name, sizeof(class_name),
					uinf.bInterfaceClass,
					uinf.bInterfaceSubClass,
					uinf.bInterfaceProtocol);

			dbg("%8s: %2d - %s", " ", j, class_name);
			WriteLogFile("%8s: %2d - %s", " ", j, class_name);
		}

		dbg(" ");
	}
	if(found)
		return 0;
	return -1;
}

static int import_device(int sockfd, struct usb_device *udev,
		struct usb_interface *uinf0,
		HANDLE *devfd,usbip_wudev_t *wudev)
{
	HANDLE fd;
	int port, ret;

	fd = usbip_vbus_open();
	if (INVALID_HANDLE_VALUE == fd) {
		WriteLogFile("driver not exist or errors!\n");
		return -1;
	}

	port = usbip_vbus_get_free_port(fd);
	if (port <= 0) {
		WriteLogFile("no free port,虚拟总线获取所有的端口状态失败 原因:可能安装了其他虚拟总线驱动(请卸载或者禁用)");
		CloseHandle(fd);
		return -1;
	}

	//WriteLogFile("call from attch here port %d\n",port);
	//ret = usbip_vbus_attach_device(fd, port, udev, uinf0);

	ret = usbip_vhci_attach_device(fd, port, wudev);
	//WriteLogFile("return from attch here\n");

	if (ret < 0) {
		WriteLogFile("err import device");
		CloseHandle(fd);
		return -1;
	}
	WriteLogFile("devfd:%p\n",devfd);
	*devfd=fd;

	return port;
}

static int query_import_device(int sockfd, char *busid,
		struct usb_interface *uinf0, HANDLE * fd)
{
	int ret;
	struct op_import_request request;
	struct op_import_reply   reply;
	uint16_t code = OP_REP_IMPORT;
	usbip_wudev_t	wuDev;
	memset(&request, 0, sizeof(request));
	memset(&reply, 0, sizeof(reply));

	/* send a request */
	ret = usbip_send_op_common(sockfd, OP_REQ_IMPORT, 0);
	if (ret < 0) {
		WriteLogFile("send op_common");
		return -1;
	}

	strncpy(request.busid, busid, sizeof(request.busid));
	request.busid[sizeof(request.busid)-1]=0;

	PACK_OP_IMPORT_REQUEST(0, &request);

	ret = usbip_send(sockfd, (void *) &request, sizeof(request));
	if (ret < 0) {
		WriteLogFile("send op_import_request");
		return -1;
	}

	/* recieve a reply */
	ret = usbip_recv_op_common(sockfd, &code);
	if (ret < 0) {
		WriteLogFile("recv op_common");
		return -1;
	}

	ret = usbip_recv(sockfd, (void *) &reply, sizeof(reply));
	if (ret < 0) {
		WriteLogFile("recv op_import_reply");
		return -1;
	}

	PACK_OP_IMPORT_REPLY(0, &reply);

	/* check the reply */
	if (strncmp(reply.udev.busid, busid, sizeof(reply.udev.busid))) {
		WriteLogFile("recv different busid %s", reply.udev.busid);
		return -1;
	}
	if((reply.udev.bNumInterfaces == 1|| (reply.udev.idVendor == 0x0525 && reply.udev.idProduct == 0xa4a2)) )
		//&& (reply.udev.idVendor !=0x1432 && reply.udev.idProduct == 0x07dc)//目前只针对单接口处理  不知道双接口的两个接口的这些信息是否一致
	{
		reply.udev.bDeviceClass = uinf0->bInterfaceClass;
		reply.udev.bDeviceSubClass = uinf0->bInterfaceSubClass;
		reply.udev.bDeviceProtocol = uinf0->bInterfaceProtocol;
	}
	get_wudev(sockfd, &wuDev, &reply.udev);

	WriteLogFile("after get_wudev:wuDev.proto:%d --wuDev.class:%d -- wuDev.subclass:%d \n", wuDev.bDeviceProtocol, wuDev.bDeviceClass, wuDev.bDeviceSubClass);
	/* import a device */
	//info("uinf0",uinf0->);
	return import_device(sockfd, &reply.udev, uinf0, fd, &wuDev);
	//return import_device(sockfd, &reply.udev, uinf0, fd);
}
static int close_sock(int sockfd)
{
	shutdown(sockfd,SD_BOTH);
	closesocket(sockfd);
}
static int attach_device(char * host,char * port ,char *random,int trans_type,int dev_port,int *sockfd,char *oper_id,int *virtual_port)
{
	//SOCKET sockfd;
	int rhport,result = -1;
	HANDLE devfd=INVALID_HANDLE_VALUE;
	
	HANDLE sckfd=INVALID_HANDLE_VALUE;
	struct usb_interface uinf;
	int ret;
	char bus_id[32] = {0};


    // struct timeval timeout;
    // timeout.tv_sec = 15;  // 设置超时时间为10秒
    // timeout.tv_usec = 0;

    // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    //     // perror("setsockopt");
	// 	WriteLogFile("		setsockopt   RCV\r\n");
	// 	return USBIP_ERR_CONNECT_SERVER;
    //     // exit(EXIT_FAILURE);
    // }
    // if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    //     // perror("setsockopt");
	// 	WriteLogFile("		setsockopt   SND\r\n");
	// 	return USBIP_ERR_CONNECT_SERVER;
	// 	// exit(EXIT_FAILURE);
    // }
	WriteLogFile("--------------------zwl---------------------0202\r\n");

	WriteLogFile("attach_device server %s port %s ,random = %s  type :%d\n", host,port,random,trans_type);
	if (trans_type == 1)//公网转发
		*sockfd = tcp_connect(host, port);
	else
		*sockfd = tcp_connect(host, port);//USBIP_PORT_STRING);
	if (INVALID_SOCKET == *sockfd) {
		WriteLogFile("tcp connect err");
		return USBIP_ERR_CONNECT_SERVER;//USBIP_ERROR_CONNECT_SERVER
	}
	if (trans_type == 1)//公网转发
	{
		if(query_connect_server(*sockfd,random)<0)
		{
			WriteLogFile("cannot connect server");
			close_sock(*sockfd);
			return USBIP_ERR_CONNECT_SERVER;//USBIP_ERROR_CONNECT_SERVER
		}

		ret = apply_for_port_use(*sockfd,dev_port,host,port,random,oper_id);//公网转发申请端口使用
		if(ret != 0)
		{
			close_sock(*sockfd);
			return ret;//USBIP_ERR_PORT_ERR || USBIP_ERR_NOT_AUTH || USBIP_ERR_PORT_BUSY
		}
		
	}
	if (trans_type == 1)//公网转发
	{
		ret = get_busid(*sockfd,dev_port,(unsigned char *)bus_id);//获取端口对应busid
		if(ret != 0)
		{

			close_sock(*sockfd);
			return -3;
		}
		WriteLogFile("get_busid is:%s\n",bus_id);
		if(query_interface0(*sockfd, bus_id, &uinf, trans_type,dev_port)){
			
			WriteLogFile("cannot find device");
			close_sock(*sockfd);
			return USBIP_ERROR_GET_DEVICE;//USBIP_ERROR_GET_DEVICE
		}
	}
	else
	{
		WriteLogFile("query_import_device\n");
		ret = query_interface_8009(*sockfd, bus_id, &uinf, trans_type,dev_port,random,oper_id);
		if(ret <0)
		{
			if(ret == -6)//未授权
				return USBIP_ERR_INNER_PORT_AUTH;
			if(ret == -7)//此端口是机械臂
				return USBIP_ERR_INNER_PORT_ERR1;
			WriteLogFile("query_interface_8009 err!,ret = %d\n",ret);
			close_sock(*sockfd);
			return USBIP_ERR_INNER_PORT_ERR;
		}
		WriteLogFile("get_busid is:%s\n",bus_id);
	}
	
	if(trans_type != 1)//内网加载
	{
		close_sock(*sockfd);
		*sockfd = tcp_connect(host, port);
		if (INVALID_SOCKET == *sockfd) {
			WriteLogFile("err tcp connect2");
			return USBIP_ERR_CONNECT_SERVER;
		}
	}
	WriteLogFile("query_import_device\n");
	rhport = query_import_device(*sockfd, bus_id, &uinf, &devfd);
	if (rhport < 0) {
		WriteLogFile("err query");
		close_sock(*sockfd);
		return USBIP_ERR_IMPORT_PORT;
	}
	WriteLogFile("new usb device attached to usbvbus port %d,sockfd = %d\n", rhport,*sockfd);

	*virtual_port = rhport;

	result = usbip_forward(devfd,*sockfd,FALSE);

	WriteLogFile("detaching device reslut = %d",result);
	usbip_vbus_detach_device(devfd,rhport);

	if(result < 0)
	{
		WriteLogFile("lbc last error %d\n",GetLastError());
	}

	//WriteLogFile("closing connection to device,result = %d\n",result);
	CloseHandle(devfd);

	if(result == -2)   //重新枚举
	{
		WriteLogFile("重新枚举");
		// ret = get_busid(*sockfd,dev_port,(unsigned char *)bus_id);//获取端口对应busid
		// if(ret != 0)
		// {

		// 	close_sock(*sockfd);
		// 	return -3;
		// }
		// WriteLogFile("重新枚举 get_busid is:%s\n",bus_id);
		if (trans_type == 1)//公网转发
		{
			if(query_interface0(*sockfd, bus_id, &uinf, trans_type,dev_port)){
				
				WriteLogFile("cannot find device");
				close_sock(*sockfd);
				return USBIP_ERROR_GET_DEVICE;//USBIP_ERROR_GET_DEVICE
			}
		}
		else
		{
			WriteLogFile("query_import_device\n");
			ret = query_interface_8009(*sockfd, bus_id, &uinf, trans_type,dev_port,random,oper_id);
			if(ret <0)
			{
				if(ret == -6)//未授权
					return USBIP_ERR_INNER_PORT_AUTH;
				if(ret == -7)//此端口是机械臂
					return USBIP_ERR_INNER_PORT_ERR1;
				WriteLogFile("query_interface_8009 err!,ret = %d\n",ret);
				close_sock(*sockfd);
				return USBIP_ERR_INNER_PORT_ERR;
			}
			WriteLogFile("get_busid is:%s\n",bus_id);
		}
		if(trans_type != 1)//内网加载
		{
			close_sock(*sockfd);
			*sockfd = tcp_connect(host, port);
			if (INVALID_SOCKET == *sockfd) {
				WriteLogFile("err tcp connect2");
				return USBIP_ERR_CONNECT_SERVER;
			}
		}
		rhport = query_import_device(*sockfd, bus_id, &uinf, &devfd);
		if (rhport < 0) {
			WriteLogFile("err query");
			close_sock(*sockfd);
			return USBIP_ERR_IMPORT_PORT;
		}
		WriteLogFile("重新枚举 new usb device attached to usbvbus port %d\n", rhport);

		result = usbip_forward(devfd,*sockfd,FALSE);
		// WriteLogFile("detaching device reslut = %d",result);
		usbip_vbus_detach_device(devfd,rhport);
		// WriteLogFile("closing connection to device");
		CloseHandle(devfd);
		// WriteLogFile("close_sock");
		close_sock(*sockfd);
		return USBIP_ERR_PORT_DETTACH;
	}

	if((*sockfd) > 0)
		close_sock(*sockfd);
	WriteLogFile("done *sockfd = %d",*sockfd);
	return USBIP_ERR_PORT_DETTACH;
}

const char help_message[] = "\
Usage: %s [options]				\n\
	-a, --attach [host] [bus_id]		\n\
		Attach a remote USB device.	\n\
						\n\
	-x, --attachall [host]		\n\
		Attach all remote USB devices on the specific host.	\n\
						\n\
	-d, --detach [ports]			\n\
		Detach an imported USB device.	\n\
						\n\
	-l, --list [hosts]			\n\
		List exported USB devices.	\n\
						\n\
	-p, --port				\n\
		List virtual USB port status. 	\n\
						\n\
	-D, --debug				\n\
		Print debugging information.	\n\
						\n\
	-v, --version				\n\
		Show version.			\n\
						\n\
	-h, --help 				\n\
		Print this help.		\n";

static void show_help(char *name)
{
	printf(help_message, name);
}

static int detach_port(char *port)
{
	signed char addr=atoi(port);
	HANDLE fd;
	int ret;

	fd = usbip_vbus_open();
	if (INVALID_HANDLE_VALUE == fd) {
		err("open vbus driver");
		return -1;
	}
	ret = usbip_vbus_detach_device(fd, addr);
	CloseHandle(fd);
	return ret;
}

static int show_port_status(void)
{
	HANDLE fd;
	int i;
	char buf[128];

	fd = usbip_vbus_open();
	if (INVALID_HANDLE_VALUE == fd) {
		err("open vbus driver");
		return -1;
	}
	if(usbip_vbus_get_ports_status(fd, buf, sizeof(buf))){
		err("get port status");
		return -1;
	}
	info("max used port:%d\n", buf[0]);
	for(i=1; i<=buf[0]; i++){
		if(buf[i])
			info("port %d: used\n", i);
		else
			info("port %d: idle\n", i);
	}
	CloseHandle(fd);
	return 0;
}

static int query_exported_devices(SOCKET sockfd)
{
	int ret;
	struct op_devlist_reply rep;
	uint16_t code = OP_REP_DEVLIST;
	uint32_t i,j;
	char product_name[100];
	char class_name[100];
	struct usb_device udev;

	memset(&rep, 0, sizeof(rep));

	ret = usbip_send_op_common(sockfd, OP_REQ_DEVLIST, 0);
	if (ret < 0) {
		err("send op_common");
		return -1;
	}

	ret = usbip_recv_op_common(sockfd, &code);
	if (ret < 0) {
		err("recv op_common");
		return -1;
	}

	ret = usbip_recv(sockfd, (void *) &rep, sizeof(rep));
	if (ret < 0) {
		err("recv op_devlist");
		return -1;
	}

	PACK_OP_DEVLIST_REPLY(0, &rep);
	dbg("exportable %d devices", rep.ndev);

	for (i=0; i < rep.ndev; i++) {

		memset(&udev, 0, sizeof(udev));

		ret = usbip_recv(sockfd, (void *) &udev, sizeof(udev));
		if (ret < 0) {
			err("recv usb_device[%d]", i);
			return -1;
		}
		pack_usb_device(0, &udev);
		usbip_names_get_product(product_name, sizeof(product_name),
				udev.idVendor, udev.idProduct);
		usbip_names_get_class(class_name, sizeof(class_name), udev.bDeviceClass,
				udev.bDeviceSubClass, udev.bDeviceProtocol);

		info("%8s: %s", udev.busid, product_name);
		info("%8s: %s", " ", udev.path);
		info("%8s: %s", " ", class_name);

		for (j=0; j < udev.bNumInterfaces; j++) {
			struct usb_interface uinf;

			ret = usbip_recv(sockfd, (void *) &uinf, sizeof(uinf));
			if (ret < 0) {
				err("recv usb_interface[%d]", j);
				return -1;
			}

			pack_usb_interface(0, &uinf);
			usbip_names_get_class(class_name, sizeof(class_name),
					uinf.bInterfaceClass,
					uinf.bInterfaceSubClass,
					uinf.bInterfaceProtocol);

			info("%8s: %2d - %s", " ", j, class_name);
		}

		info(" ");
	}
	return rep.ndev;
}

static void show_exported_devices(char *host)
{
	int ret;
	SOCKET sockfd;

	sockfd = tcp_connect(host, USBIP_PORT_STRING);
	if (INVALID_SOCKET == sockfd){
		info("- %s failed", host);
		return;
	}
	info("- %s", host);

	ret = query_exported_devices(sockfd);
	if (ret < 0) {
		err("query");
	}
	closesocket(sockfd);
}

static void attach_devices_all(char *host)
{
	return;
}

int attach_usbip(char * host, int serv_port, int dev_port, int trans_type,char *user,int *sockfd,char *oper_id,int *virtual_port)
{
	int ret = 0;
	
	char ser_port_str[64]={ 0 };
	WriteLogFile("trans_type = %d\n", trans_type);
	sprintf(ser_port_str, "%d", serv_port);
	ret = attach_device(host, ser_port_str, user, trans_type, dev_port,sockfd,oper_id,virtual_port);

	return ret;

}
