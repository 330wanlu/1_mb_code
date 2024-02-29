#define DF_CLIENT_PROTOCOL_C
#include "client_portocol.h"
//#include "log.h"
#include "typedef_data.h"
#include "terminal_manage.h"
//#include "port_manage.h"
#include <Windows.h>
//#include "InvoiceDetail.h"
//#include "system.h"
//#include "zlib.h"


//#define WriteLogFile(format,...)		log_out(format , ##__VA_ARGS__ )

static void  err_handle(SOCKET sockfd, int ret);
int  recv_client_portocol(SOCKET sockfd, unsigned char** out_buf);
static void  send_client_protocol(SOCKET sockfd, char cmd, char* in_buf, int inbuf_size);
static void err2client(int err, int *ret_err, char *err_des);



//老协议
static void old_open_port(SOCKET sockfd, int usbport);
static void old_close_port(SOCKET sockfd, int usbport, int force);
static void old_client_query_usbport_attribute(SOCKET sockfd, int usbport);
static void old_client_query_cabinet_attribute_A(SOCKET sockfd);
static void old_client_query_cabinet_attribute(SOCKET sockfd);
static void old_client_query_midware_setting(SOCKET sockfd);
static void old_client_updata_midware_setting(SOCKET sockfd, unsigned char*buf);

//新协议
static void  ClientOpenUSBPort(SOCKET sockfd, char* pc_terminalid , int usbport);
static void  client_close_port(SOCKET sockfd, char* pc_terminalid,int usbport, int force);
static void  client_query_usbport_attribute(SOCKET sockfd,char* terminal_id ,  int usbport);
static void  client_query_cabinet_attribute(SOCKET sockfd);
static void  client_query_midware_setting(SOCKET sockfd);
static void  client_restart_terminal(SOCKET sockfd, char* terminal_id);
static void  client_get_midware_state(SOCKET sockfd);
static void  client_get_cabinet_attribute_by_id(SOCKET sockfd, char* terminal_id);
static void  client_get_cabinet_attribute_by_ip(SOCKET sockfd, unsigned char *buf);
static void  client_query_cabinet_attribute_A(SOCKET sockfd);
static void  client_get_used_port_device(SOCKET sockfd);
static void  client_search_online_terminal(SOCKET sockfd);
static void  client_query_cabinet_network_param(SOCKET sockfd, char* terminal_id);
static void  client_set_cabinet_network_param(SOCKET sockfd, char* terminal_id, char *in_buf);
static void  client_upgrade_program(SOCKET sockfd, char* terminal_id);
static void  client_get_cabinet_attribute_by_ip_A(SOCKET sockfd, unsigned char * buf);
static void  client_query_midware_setting_A(SOCKET sockfd);
static void  client_updata_midware_setting(SOCKET sockfd, char*buf);
static void  client_updata_midware_setting_A(SOCKET sockfd, char*buf);
static void  client_get_cabinet_attribute_by_id_all(SOCKET sockfd, char* terminal_id);

static void  ClientSetVPN(SOCKET sockfd, unsigned char* buf);
static void  clientSetAutoUpgrade(SOCKET sockfd, unsigned char* buf);
static void  clientGetVPN(SOCKET sockfd, unsigned char* buf);
static void  clientGetAutoUpgrade(SOCKET sockfd, unsigned char* buf);
static void  clientGetTerminalCAInfo(SOCKET sockfd, char* terminal_id);

static void  ClientGetUSBListInfo(SOCKET sockfd, char* terminal_id);
static void  ClientGetInvoiceByTCP(SOCKET sockfd, unsigned char* buf);
static void  ClientGetInvoiceByDir(SOCKET sockfd, unsigned char* buf);
static void  ClientGetInvoiceSchedule(SOCKET sockfd, unsigned char* in_buf);
static void  ClientGetAreacode(SOCKET sockfd, unsigned char* buf);
static void  ClientGetInvoiceSum(SOCKET sockfd, unsigned char* buf);
static void  ClientGetTaxType(SOCKET sockfd, unsigned char* in_buf);
static void  ClientGetInvoiceSectionInfo(SOCKET sockfd, unsigned char* in_buf);
static void  ClientGetTaxGuardInfo(SOCKET sockfd, unsigned char* in_buf);
static void  ClientGetTaxPlateInfoUnify(SOCKET sockfd, unsigned char* buf);



void hex2str(unsigned char* in_buf, int in_size, char* out_buf)
{
	int i = 0;

	for (i = 0; i < in_size; i++){
		sprintf(out_buf + (i * 5), "0x%02x ", *(in_buf + i));
	}
}

/*********************************************************************
-   Function : cmd_handle
-   Description：命令处理
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
int  cmd_handle(SOCKET	sockfd, char *ip, int ip_port, void *arg, int *cmd, unsigned char* buf)
{
	HANDLE* param = NULL;
	int usbport = 0;
	char terminal_id[20] = { 0 };


	switch (*cmd){

	case 0x65://打开
		usbport = (((int)buf[0] << 24)&(0xFF000000)) + (((int)buf[1] << 16)&(0x00FF0000)) + (((int)buf[2] << 8)&(0x0000FF00)) + (((int)buf[3])&(0x000000FF));
		old_open_port(sockfd, usbport);
		break;
	case 0x66://关闭
		usbport = (((int)buf[0] << 24)&(0xFF000000)) + (((int)buf[1] << 16)&(0x00FF0000)) + (((int)buf[2] << 8)&(0x0000FF00)) + (((int)buf[3])&(0x000000FF));
		old_close_port(sockfd, usbport, buf[4]);
		break;
	case 0x67://查询usb端口属性
		usbport = (((int)buf[0] << 24)&(0xFF000000)) + (((int)buf[1] << 16)&(0x00FF0000)) + (((int)buf[2] << 8)&(0x0000FF00)) + (((int)buf[3])&(0x000000FF));
		old_client_query_usbport_attribute(sockfd, usbport);
		break;
	case 0x69://获取中间件设置
		old_client_query_midware_setting(sockfd);
		break;
	case 0x6a://更新中间件设置
		old_client_updata_midware_setting(sockfd, buf);
		break;
	case 0x6B://查询终端属性
		old_client_query_cabinet_attribute(sockfd);
		break;
	case 0x6E://查询终端属性A
		old_client_query_cabinet_attribute_A(sockfd);
		break;


	case 0x80://打开端口
		usbport = ((((int)buf[12] << 24)&(0xFF000000)) + (((int)buf[13] << 16)&(0x00FF0000)) + (((int)buf[14] << 8)&(0x0000FF00)) + (((int)buf[15])&(0x000000FF)));
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else{
			WriteLogFile("打开的usb端口是%d 终端ID=%s", usbport, terminal_id);
			ClientOpenUSBPort(sockfd, terminal_id, usbport );
		}
*/
		break;
	case 0x81://关闭端口
		usbport = (((int)buf[12] << 24)&(0xFF000000)) + (((int)buf[13] << 16)&(0x00FF0000)) + (((int)buf[14] << 8)&(0x0000FF00)) + (((int)buf[15])&(0x000000FF));
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			WriteLogFile("关闭的usb端口是%d 终端ID=%s", usbport, terminal_id);
			client_close_port(sockfd, terminal_id , usbport, (int)buf[16]);
		}
		
		break;
	case 0x82://查询端口属性
		usbport = (((int)buf[12] << 24)&(0xFF000000)) + (((int)buf[13] << 16)&(0x00FF0000)) + (((int)buf[14] << 8)&(0x0000FF00)) + (((int)buf[15])&(0x000000FF));
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_query_usbport_attribute(sockfd, terminal_id, usbport);
		}
		
		break;
	case 0x83://重启终端
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_restart_terminal(sockfd, terminal_id);
		}
		break;
	case 0x84://查询已设置的终端信息
		client_query_cabinet_attribute_A(sockfd);
		break;
	case 0x87://获取本机已使用的USB设备
		client_get_used_port_device(sockfd);
		break;
	case 0x88://获取中间件状态
		client_get_midware_state(sockfd);
		break;
	case 0x89://搜索在线终端
		client_search_online_terminal(sockfd);
		break;
	case 0x8C://查询终端网络参数
		memcpy(terminal_id, buf, 12);
	/*	if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_query_cabinet_network_param(sockfd, terminal_id);
		}
		break;
	case 0x8d://设置终端网络参数
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			WriteLogFile("设置终端网络参数");
			client_set_cabinet_network_param(sockfd, terminal_id, buf);
		}
		
		break;
	case 0x8E://升级终端程序
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_upgrade_program(sockfd, terminal_id);
		}
		break;
	case 0x8f://根据终端ID获取usb-share终端信息
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_get_cabinet_attribute_by_id(sockfd, terminal_id);
		}
		
		break;
	case 0x91://获取中间件设置_A
		
		client_query_midware_setting_A(sockfd);
		
		break;
	case 0x92://更新中间件设置_A 
		client_updata_midware_setting_A(sockfd, buf);
		break;
	case 0x93://按终端IP获取usb-share终端信息_A
		client_get_cabinet_attribute_by_ip_A(sockfd, buf);
		break;
	case 0x96://根据终端ID获取usb-share终端信息
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			client_get_cabinet_attribute_by_id_all(sockfd, terminal_id);
		}
		break;

	case 0x8A://获取中间件设置
		client_query_midware_setting(sockfd);
		break;
	case 0x8B://更新中间件设置
		client_updata_midware_setting(sockfd, buf);
		break;
	case 0x90://根据终端IP获取usb-share终端信息
		client_get_cabinet_attribute_by_ip(sockfd, buf);
		break;
	case 0x98://获取ca名称和税号
		memcpy(terminal_id, buf, 12);
	/*	if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			clientGetTerminalCAInfo(sockfd, terminal_id);
		}
		break;
	case 0x99://设置VPN功能
		ClientSetVPN(sockfd, buf);
		break;
	case 0x9a://设置是否关闭自动升级
		clientSetAutoUpgrade(sockfd, buf);
		break;
	case 0x9b://获取VPN功能
		clientGetVPN(sockfd, buf);
		break;
	case 0x9c://查询是否关闭自动升级
		clientGetAutoUpgrade(sockfd, buf);
		break;

	case 0x9d://获取usb设备(ca、税盘)信息(主要针对纳税人名称和识别号)
		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}*/
		ClientGetUSBListInfo(sockfd, terminal_id);
		break;
	case 0xA0://阻塞方式查询发票信息
		ClientGetInvoiceByTCP(sockfd, buf);
		break;
	case 0xA1://获取税盘发票种类
		ClientGetTaxType(sockfd, buf);
		break;
	case 0xA2://获取监控信息
		ClientGetTaxGuardInfo(sockfd, buf);
		break;
	case 0xA3://发票卷查询
		ClientGetInvoiceSectionInfo(sockfd, buf);
		break;
	case 0xA4://非阻塞方式获取发票信息
		ClientGetInvoiceByDir(sockfd, buf);
		break;
	case 0xA5://发票进度查询
		ClientGetInvoiceSchedule(sockfd, buf);
		break;

	case 0xA6://区域代码查询
		ClientGetAreacode(sockfd, buf);
		break;

	case 0xA7://获取某月发票总数
		ClientGetInvoiceSum(sockfd, buf);
		break;
	case 0xA8://获取税盘监控信息统一接口
		ClientGetTaxPlateInfoUnify(sockfd , buf);
		break;
	default:
		err_handle(sockfd, DF_ERR_CLIENT_PROTOCOL);
		break;
	}
	return 0;
}
/****************************************************************************新版本协议**************************************************************/


/*********************************************************************
-   Function : ClientGetTaxPlateInfoUnify
-   Description：获取税盘信息统一接口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetTaxPlateInfoUnify(sockfd, buf)
{
	char terminal_id[20] = { 0 };
	int postion = 0;
	struct st_TaxPlateInfoUnify st_UnifyInfo = {0};

	/*错误返回*/
	int i_ret = 0;

	memcpy(terminal_id, (void*)(buf + postion), 12); postion += 12;
	/*if (check_terminal_id(terminal_id) != 0){
		return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}*/

	i_ret = TerminalGetTaxPlateInfoUnify(terminal_id, &st_UnifyInfo);
	/*if (i_ret != 0){
		FreeTaxPlateInfoUnifySpace(&st_UnifyInfo);
		return err_handle(sockfd, i_ret);
	}*/

	//zlib解压缩(300倍的空间分配) 
	st_UnifyInfo.cp_TaxPlateInfoAfterUnzlib = calloc(1 , st_UnifyInfo.i_LenBeforeUnZlib * 300);
	/*if (st_UnifyInfo.cp_TaxPlateInfoAfterUnzlib == NULL){
		FreeTaxPlateInfoUnifySpace(&st_UnifyInfo);
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}*/
	st_UnifyInfo.i_LenAfterUnZlib = st_UnifyInfo.i_LenBeforeUnZlib * 300;
	/*if (uncompress(st_UnifyInfo.cp_TaxPlateInfoAfterUnzlib, &st_UnifyInfo.i_LenAfterUnZlib, st_UnifyInfo.cp_TaxPlateInfoBeforeUnZlib, st_UnifyInfo.i_LenBeforeUnZlib) != 0){
		FreeTaxPlateInfoUnifySpace(&st_UnifyInfo);
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}*/

	postion = strlen(st_UnifyInfo.cp_TaxPlateInfoAfterUnzlib);
	send_client_protocol(sockfd, 0xC8, st_UnifyInfo.cp_TaxPlateInfoAfterUnzlib, postion);

	//return FreeTaxPlateInfoUnifySpace(&st_UnifyInfo);
	

}
/*********************************************************************
-   Function : ClientGetInvoiceByTCP
-   Description：获取发票明细通过TCP传输(阻塞)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/

static void ClientGetInvoiceByTCP(SOCKET sockfd, unsigned char* buf)
{
	//unsigned char terminal_id[20] = { 0 }, date[20] = { 0 }, *send_client_buf = NULL;
	//int usbport = 0, postion = 0, ret = 0;
	//struct InvoiceDataHead *p_InvoiceDataHead = calloc(1 , sizeof(struct InvoiceDataHead));

	//if (p_InvoiceDataHead == NULL){
	//	return err_handle(sockfd, DF_SYSTEM_ERROR);
	//}
	//
	//memcpy(terminal_id, buf + postion, 12); postion += 12;
	//usbport = ((buf[postion] << 24) + (buf[postion + 1] << 16) + (buf[postion + 2] << 8) + buf[postion + 3]); postion += 4;
	//memcpy(date, buf + postion, 6); postion += 6;

	//ret = TerminalGetInvocieByMouth(terminal_id, usbport, date, NULL, p_InvoiceDataHead);
	//if (ret != 0){
	//	WriteLogFile("获取发票信息失败 ret = %d" , ret);
	//	ReleaseInvoiceData(p_InvoiceDataHead);
	//	return err_handle(sockfd, ret);
	//}
	//postion = 0;
	//send_client_buf = calloc(1 , 1000 * 1000 * 20);
	//if (send_client_buf == NULL) {
	//	ReleaseInvoiceData(p_InvoiceDataHead);
	//	return err_handle(sockfd, DF_SYSTEM_ERROR);
	//}
	////发票总数
	//send_client_buf[postion++] = (p_InvoiceDataHead->InvoiceSumNum >> 24) & 0xff;
	//send_client_buf[postion++] = (p_InvoiceDataHead->InvoiceSumNum >> 16) & 0xff;
	//send_client_buf[postion++] = (p_InvoiceDataHead->InvoiceSumNum >> 8) & 0xff;
	//send_client_buf[postion++] = (p_InvoiceDataHead->InvoiceSumNum) & 0xff;
	////解析发票明细
	//struct InvoiceDataNode* p_invoice = p_InvoiceDataHead->next;
	//while (p_invoice != NULL){
	//	memcpy(send_client_buf + postion, p_invoice->ca_FPDM, 20); postion += 20;//发票代码
	//	memcpy(send_client_buf + postion, p_invoice->ca_FPHM, 20); postion += 20;//发票号码
	//	send_client_buf[postion++] = (strlen(p_invoice->pc_JsonInvoice) >> 24) & 0xff;
	//	send_client_buf[postion++] = (strlen(p_invoice->pc_JsonInvoice) >> 16) & 0xff;
	//	send_client_buf[postion++] = (strlen(p_invoice->pc_JsonInvoice) >> 8) & 0xff;
	//	send_client_buf[postion++] = (strlen(p_invoice->pc_JsonInvoice)) & 0xff;
	//	memcpy(send_client_buf + postion, p_invoice->pc_JsonInvoice, strlen(p_invoice->pc_JsonInvoice)); postion += strlen(p_invoice->pc_JsonInvoice);
	//	p_invoice = p_invoice->next;
	//}
	//ReleaseInvoiceData(p_InvoiceDataHead);
	//send_client_protocol(sockfd, 0xC0, send_client_buf, postion);
	//if (send_client_buf)free(send_client_buf);
	return;
}
/*********************************************************************
-   Function : ClientGetInvoiceByDir
-   Description：获取发票明细通过文件存储(非阻塞)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetInvoiceByDir(SOCKET sockfd, unsigned char* buf)
{
	//unsigned char send_to_client_buf[1000] = { 0 };
	//unsigned char terminal_id[20] = { 0 }, dir[260] = { 0 }, date[20] = { 0 }, details_dir[260] = { 0 };
	//int usbport = 0, i_InvoiceNum = 0, postion = 0, ret = 0, cmd_len = 5;
	//struct CabinetInfo cabinet = { 0 };
	//BOOL InitDIrFirst = TRUE;

	//memcpy(terminal_id, buf, 12); postion += 12;
	//usbport = ((buf[postion] << 24) + (buf[postion + 1] << 16) + (buf[postion + 2] << 8) + buf[postion + 3]); postion += 4;
	//memcpy(date, buf + postion, 6); postion += 6;
	//
	//memcpy(dir, buf + postion, strlen(buf + postion));

	//struct UnBlockRetrieveGetInvoiceNode* UnBlockNode = NULL;

	////查找是否已有该查询任务
	//UnBlockNode = SearchNodeFromHeadTable(terminal_id, usbport, date);
	//if (UnBlockNode && (UnBlockNode->p_InvoiceHead->InvoiceSumNum != UnBlockNode->p_InvoiceHead->RecvedInvoiceNum)){
	//	WriteLogFile("总发票数 = %d 已接收的发票数= %d", UnBlockNode->p_InvoiceHead->InvoiceSumNum, UnBlockNode->p_InvoiceHead->RecvedInvoiceNum);
	//	err_handle(sockfd, DF_ERR_ON_SEARCHING);
	//}else{
	//	//首先获取该端口号税号
	//	ret = TerminalGetTerminalUSBListInfo(terminal_id, &cabinet);
	//	if (ret != 0){
	//		WriteLogFile("查询ca纳税人信息失败");
	//		err_handle(sockfd, ret);
	//	}
	//	else{
	//		//检查目录是否有效，如果无效重新组织有效存储地址(中间件运行目录)
	//		if (CheckDirUsed(dir) == FALSE){
	//			InitDefaultInvoiceSaveDir(NULL, cabinet.port_state[usbport - 1].tax_number, date, details_dir);
	//		}
	//		else{
	//			InitDefaultInvoiceSaveDir(dir, cabinet.port_state[usbport - 1].tax_number, date, details_dir);
	//		}

	//		//获取发票信息
	//		ret = TerminalGetInvoiceInfo(terminal_id, usbport, date, details_dir, &i_InvoiceNum);
	//		if (ret != 0){
	//			err_handle(sockfd, ret);
	//			return;
	//		}
	//		//组包返回给客户端
	//		cmd_len += strlen(dir);
	//		postion = 0;
	//		send_to_client_buf[postion++] = ((i_InvoiceNum) >> 24) & 0xff;
	//		send_to_client_buf[postion++] = ((i_InvoiceNum) >> 16) & 0xff;
	//		send_to_client_buf[postion++] = ((i_InvoiceNum) >> 8) & 0xff;
	//		send_to_client_buf[postion++] = (i_InvoiceNum)& 0xff;

	//		memcpy(send_to_client_buf + postion, details_dir, strlen(details_dir)); postion += 260;
	//		send_client_protocol(sockfd, 0xC4, send_to_client_buf, postion);
	//	}
	//}
}
/*********************************************************************
-   Function : ClientGetInvoiceSchedule
-   Description：获取发票明细查询进度
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/

static void ClientGetInvoiceSchedule(SOCKET sockfd, unsigned char* in_buf)
{
	//unsigned char terminal_id[20] = { 0 }, send_client_buf[100] = { 0 }, date[20] = { 0 };
	//int usbport = 0, postion = 0, SumInvoice = 0, RecvedInvoice = 0;

	//memcpy(terminal_id, in_buf, 12);
	//if (check_terminal_id(terminal_id) != 0){
	//	return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	//}

	//usbport = ((in_buf[12] << 24) + (in_buf[13] << 16) + (in_buf[14] << 8) + in_buf[15]);
	//memcpy(date, in_buf + 16, 6);

	//struct UnBlockRetrieveGetInvoiceNode* pst_UnBlockNode = NULL;

	////延时2s 防止二次开发频繁查询进度
	//Sleep(2000);
	///*
	//*搜索结点 如果没有找到，则说明没有该税盘该月的查询任务
	//          如果找到    ，发票总数和已接收的发票总数相等，表示查询任务完成 否则返回查询的进度
	//*
	//*/

	//pst_UnBlockNode = SearchNodeFromHeadTable(terminal_id, usbport, date);
	//if (!pst_UnBlockNode){
	//	err_handle(sockfd, DF_ERR_NO_SEARCH_SCHEDULE);
	//}/**/
	//else if ((pst_UnBlockNode->p_InvoiceHead->InvoiceSumNum == pst_UnBlockNode->p_InvoiceHead->RecvedInvoiceNum) && (pst_UnBlockNode->p_InvoiceHead->InvoiceSumNum != 0)){
	//	WriteLogFile("terminal_id = %s usbport = %d ,data = %s 已接收发票总数 = %d 搜索完成", terminal_id, usbport, date, pst_UnBlockNode->p_InvoiceHead->InvoiceSumNum);
	//	err_handle(sockfd, DF_SEARCH_COMPLETE);;
	//}
	//else{
	//	WriteLogFile("terminal_id = %s usbport = %d ,data = %s 发票总数= %d 已接收发票数量= %d ", terminal_id, usbport, date, pst_UnBlockNode->p_InvoiceHead->InvoiceSumNum, pst_UnBlockNode->p_InvoiceHead->RecvedInvoiceNum);
	//	//if (pst_UnBlockNode->p_InvoiceHead->i_ErrNum != 0){
	//	if (pst_UnBlockNode->i_ErrNum != 0){
	//		WriteLogFile("pst_UnBlockNode->p_InvoiceHead->i_ErrNum = %d", pst_UnBlockNode->i_ErrNum);
	//		err_handle(sockfd, pst_UnBlockNode->i_ErrNum);
	//	}
	//	else{
	//		SumInvoice = pst_UnBlockNode->p_InvoiceHead->InvoiceSumNum;
	//		RecvedInvoice = pst_UnBlockNode->p_InvoiceHead->RecvedInvoiceNum;
	//		send_client_buf[postion++] = ((SumInvoice >> 24) & 0xff);
	//		send_client_buf[postion++] = ((SumInvoice >> 16) & 0xff);
	//		send_client_buf[postion++] = ((SumInvoice >> 8) & 0xff);
	//		send_client_buf[postion++] = ((SumInvoice)& 0xff);
	//		send_client_buf[postion++] = ((RecvedInvoice >> 24) & 0xff);
	//		send_client_buf[postion++] = ((RecvedInvoice >> 16) & 0xff);
	//		send_client_buf[postion++] = ((RecvedInvoice >> 8) & 0xff);
	//		send_client_buf[postion++] = ((RecvedInvoice)& 0xff);

	//		send_client_protocol(sockfd, 0xC5, send_client_buf, postion);
	//	}
	//}
}







/*********************************************************************
-   Function : ClientGetTaxType
-   Description：获取金税盘发票种类
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetTaxType(SOCKET sockfd, unsigned char * in_buf)
{
	int ret = 0;
	char terminal_id[20] = { 0 }, result_buf[10] = { 0 };
	int usbport = 0;

	memcpy(terminal_id, in_buf, 12);
	/*if (check_terminal_id(terminal_id) != 0){
		return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}*/

	usbport = ((in_buf[12] << 24) + (in_buf[13] << 16) + (in_buf[14] << 8) + in_buf[15]);
	ret = TerminalGetInvoiceTypeNum(terminal_id, usbport, result_buf);
	if (ret != 0){
		return err_handle(sockfd, ret);
	}
	return send_client_protocol(sockfd, 0xC1, result_buf, 8);
}

/*********************************************************************
-   Function : ClientGetInvoiceSectionInfo
-   Description：获取金税盘发票卷信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
//金税盘
static void ClientGetInvoiceSectionInfo(SOCKET sockfd, unsigned char * in_buf)
{
	//int ret = 0, usbport = 0, postion = 0;
	//char terminal_id[20] = { 0 }, invoice_type = 0, return_client_buf[10240] = { 0 };
	//struct AllInvoiceSectionInfo invoices = {0};

	//memcpy(terminal_id, in_buf, 12);
	//usbport = (((int)in_buf[12] << 24) + ((int)in_buf[13] << 16) + ((int)in_buf[14] << 8) + (in_buf[15]));
	//invoice_type = ExchangeInvoiceNum(0, in_buf[16]);

	//ret = GetAllInvoiceSectionInfo(terminal_id, usbport, invoice_type, &invoices);
	//if (ret != 0){
	//	WriteLogFile("获取机柜发票卷信息失败 错误码= %d" , ret);
	//	return err_handle(sockfd, ret);
	//}
	//WriteLogFile("开始组包[发票卷信息]");
	////组包数据
	//return_client_buf[postion++] = invoices.section_all_num;
	//WriteLogFile("发票总卷数:%d" ,invoices.section_all_num);
	//for (int i = 0; i < invoices.section_all_num; i++){
	//	return_client_buf[postion++] = invoices.InvoiceSectionInfo[i].serial_num;
	//	memcpy(return_client_buf + postion, invoices.InvoiceSectionInfo[i].time, 20); postion += 20;
	//	memcpy(return_client_buf + postion, invoices.InvoiceSectionInfo[i].invoice_num, 4); postion += 4;
	//	memcpy(return_client_buf + postion, invoices.InvoiceSectionInfo[i].origin_num, 4); postion += 4;
	//	memcpy(return_client_buf + postion, invoices.InvoiceSectionInfo[i].remianing_num, 4); postion += 4;
	//}
	//WriteLogFile("组包结束[发票卷信息],准备发送数据到客户端socket");
	//return send_client_protocol(sockfd, 0xC3, return_client_buf, postion);
}

//发票类型转换 mode:0 开票软件定义转换成自定义类型
//             mode: 1 自定义类型转换成开票软件
int ExchangeInvoiceNum(int mode, int type)
{
	if (mode == 0){
		switch (type){
		case 0:
			return 0;
			break;
		case 1:
			return 2;
			break;
		case 2:
			return 11;
			break;
		case 3:
			return 12;
			break;
		case 4:
			return 41;
			break;
		case 5:
			return 42;
			break;
		case 6:
			return 51;
			break;
		default:
			return 255;
		}
	}
	else{
		switch (type){
		case 0:
			return 0;
			break;
		case 2:
			return 1;
			break;
		case 11:
			return 2;
			break;
		case 12:
			return 3;
			break;
		case 41:
			return 4;
			break;
		case 42:
			return 5;
			break;
		case 51:
			return 6;
			break;
		default:
			return 255;
		}
	}


}

/*********************************************************************
-   Function : ClientGetTaxGuardInfo
-   Description：获取航信盘监控信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetTaxGuardInfo(SOCKET sockfd, unsigned char* in_buf)
{
	//int ret = 0, postion = 0, usbport = 0;
	//char result_buf[200] = { 0 };
	//char terminal_id[20] = { 0 }, tax_type = 0;
	//struct InvoiceGuardInfo guardinfo = {0};

	//memcpy(terminal_id, in_buf, 12);
	//if (check_terminal_id(terminal_id) != 0){
	//	return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	//}
	//usbport = ((in_buf[12] << 24) + (in_buf[13] << 16) + (in_buf[14] << 8) + in_buf[15]);
	//tax_type = ExchangeInvoiceNum(0, in_buf[16]);

	//ret = TerminalGetTaxGuardInfo(terminal_id, usbport, tax_type, &guardinfo);
	//if (ret != 0){
	//	return err_handle(sockfd, ret);;
	//}

	////组包数据
	//result_buf[postion++] = (unsigned char)guardinfo.BSQ;
	//result_buf[postion++] = (unsigned char)guardinfo.SSQ;

	//memcpy(result_buf + postion, guardinfo.LastBSQ, 20); postion += 20;
	//memcpy(result_buf + postion, guardinfo.NextBSQ, 20); postion += 20;
	//memcpy(result_buf + postion, guardinfo.LockDate, 20); postion += 20;
	//memcpy(result_buf + postion, guardinfo.Limite, 20); postion += 20;
	//memcpy(result_buf + postion, guardinfo.OffLimite, 20); postion += 20;
	//return send_client_protocol(sockfd, 0xC2, result_buf, postion);

}


/*********************************************************************
-   Function : ClientOpenUSBPort
-   Description：处理客户端打开命令
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二: 打开的端口号
参数三: 终端ID
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientOpenUSBPort(SOCKET sockfd, char* terminal_id, int usbport)
{
	int ret = 0;
	ret = mid_open_port(terminal_id  , usbport);
	if (ret != 0){
		WriteLogFile("打开端口错误");
		return err_handle(sockfd, ret);
	}
	WriteLogFile("打开端口成功");
	return send_client_protocol(sockfd, 0xA0, NULL, 0);
}

/*********************************************************************
-   Function : client_close_port
-   Description：关闭端口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二: USB端口号
参数三: 终端ID
参数三: 是否强制关闭
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_close_port(SOCKET sockfd, char *terminal_id, int usbport, int force)
{
	int ret = 0;
	//(force == 0) ? (ret = mid_close_port(terminal_id, usbport)) : (ret = mid_close_port_force(terminal_id, usbport));
	//ret = mid_close_port_force(terminal_id, usbport);
	if (ret != 0){
		WriteLogFile("关闭端口错误");
		return err_handle(sockfd, ret);
	}

	WriteLogFile("关闭端口成功");
	return send_client_protocol(sockfd, 0xA1, NULL, 0);

}


/*********************************************************************
-   Function : client_query_usbport_attribute
-   Description：查询usb端口属性
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二: 终端ID
参数三：usb端口号
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_usbport_attribute(SOCKET sockfd, char* terminal_id, int usbport)
{
	int ret = 0, postion = 0;
	unsigned char out_buf[1024] = { 0 };
	struct Port_State port_state = {0};

	ret = terminal_query_usbport_state(terminal_id, usbport, &port_state);
	if (ret != 0){
		WriteLogFile("查询端口失败");
		return err_handle(sockfd, ret);
	}
	out_buf[postion++] = port_state.port_exist_device;//端口是否有设备
	out_buf[postion++] = port_state.c_port_state;//端口使用状态
	memcpy(out_buf + postion, port_state.busid, 20);
	postion += 20;

	out_buf[postion++] = port_state.port_device_type;
	memcpy(out_buf + postion, port_state.device_protocol_type, 2);
	postion += 2;

	memcpy(out_buf + postion, port_state.PID, 2);
	postion += 2;

	memcpy(out_buf + postion, port_state.VID, 2);
	postion += 2;

	out_buf[postion++] = port_state.interface_num;
	memcpy(out_buf + postion, port_state.driver, 50);
	postion += 50;

	memcpy(out_buf + postion, port_state.user_ip, 24);
	postion += 24;

	memcpy(out_buf + postion, port_state.client_id, 38);
	postion += 38;
	WriteLogFile("查询端口属性成功");
	return send_client_protocol(sockfd, 0xA2, out_buf, postion);
}



/*********************************************************************
-   Function : client_restart_terminal
-   Description：重启终端
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二: 终端ID
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_restart_terminal(SOCKET sockfd, char* terminal_id)
{
	int ret = 0;

	ret = terminal_restart_terminal(terminal_id);
	if (ret != 0){
		WriteLogFile("重启终端失败");
		return err_handle(sockfd, ret);
	}

	WriteLogFile("重启终端成功");
	return send_client_protocol(sockfd, 0xA3, NULL, 0);

}


/*********************************************************************
-   Function : client_query_cabinet_attribute_A
-   Description：获取已经设置的终端信息(配置表中存在的所有终端信息)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_cabinet_attribute_A(SOCKET sockfd)
{
	int ret = 0, ret1 = 0, i = 0, j = 0, postion = 0;
	unsigned char out_buf[40480] = { 0 }, opened_port[400] = { 0 };
	struct CabinetInfo *cabinet = (struct CabinetInfo*)calloc(1 , sizeof(struct CabinetInfo) * 128);
	struct file_setting	file[128] = {0};

	if (cabinet == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}

	ret = terminal_query_cabinet_attribute(cabinet, 128);
	out_buf[postion++] = ret;

	ret1 = read_setting(file, 128);
	for (i = 0; i < 128; i++){
		if (*cabinet[i].ca_SettingIP == 0){
			continue;
		}
		memcpy(out_buf + postion, cabinet[i].ca_SettingIP, 24);
		postion += 24;
		for (j = 0; j < 128; j++){
#ifdef NINGBO_BANK
			if (strcmp(cabinet[i].ca_SettingIP, file[j].terminal_ip) == 0 && cabinet[i].dataport == file[j].data_port){
#else
			if (strcmp(cabinet[i].ca_SettingIP, file[j].terminal_ip) == 0){
#endif
				out_buf[postion++] = ((file[j].terminal_port >> 24) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 16) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 8) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 0) & (0xFF));

				out_buf[postion++] = ((file[j].data_port >> 24) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 16) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 8) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 0) & (0xFF));
			}
		}
		out_buf[postion++] = cabinet[i].terminal_system_state;
		if (cabinet[i].terminal_system_state == 0){
			continue;
		}
		memcpy(out_buf + postion, cabinet[i].ca_TerminalID, 12);
		postion += 12;

		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 24) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 16) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 8) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 0) & (0xFF));


		memcpy(out_buf + postion, cabinet[i].ca_SoftVesion, 8);
		postion += 8;
		memcpy(out_buf + postion, cabinet[i].ca_TerminalModel, 10);
		postion += 10;

		for (j = 0; j < cabinet[i].i_USBSumNumber; j++){
			out_buf[postion++] = cabinet[i].port_state[j].c_port_state;
		}
		postion += (256 - cabinet[i].i_USBSumNumber);
	}
	if (cabinet != NULL)free(cabinet);
	WriteLogFile("查询机柜属性A成功");
	return send_client_protocol(sockfd, 0xA4, out_buf, postion);

}


/*********************************************************************
-   Function : client_query_cabinet_network_param
-   Description：获取中间件已打开的端口
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二：终端ID
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_used_port_device(SOCKET sockfd)
{
	unsigned char out_buf[40000] = {0};
	int ret = 0, out_size = sizeof(out_buf);

	//ret = mid_get_computer_opened_state_detail(out_buf, &out_size);
	if (ret < 0){
		return err_handle(sockfd, ret);
	}
	WriteLogFile("获取已使用的端口设备成功");
	return send_client_protocol(sockfd, 0xA7, out_buf, out_size);

	
}



/*********************************************************************
-   Function : client_get_midware_state
-   Description：获取中间件状态
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_midware_state(SOCKET sockfd)
{
	int state = 0, postion = 0, count = 0;
	unsigned char out_buf[1024] = { 0 };
	char client_id[40];

	/*获取计算机系统状态*/
	//state = check_driver_install();


	out_buf[postion++] = state;
	memcpy(out_buf + postion, SOFT_VERSION, strlen(SOFT_VERSION));
	postion += 50;
	//count = get_opened_port_num();
	if (count > 127 || count < 0){
		WriteLogFile("总线虚拟端口号超出范围");
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	out_buf[postion++] = count;
	get_client_id(client_id);
	memcpy(out_buf + postion, client_id, 38);
	postion += 38;
	return send_client_protocol(sockfd, 0xA8, out_buf, postion);
}



/*********************************************************************
-   Function : client_search_online_terminal
-   Description：搜索在线终端
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :终端ID 返回结果的指针
-   Output :打开端口的数量
-   Return :
-   Other :
***********************************************************************/
static void client_search_online_terminal(SOCKET sockfd)
{
	int count = 0, postion = 0, i = 0, cmdport = 0, dataport = 0;
	int n_ip = 0, guard = 0;
	int malloc_size = 1024;
	unsigned char *in_buf = (unsigned char*)calloc(1 , 1024);
	if (in_buf == NULL){
		goto ERR;
	}

	struct Cabinets *cabinets_setting = (struct Cabinets*)calloc(1 , sizeof(struct Cabinets));
	if (cabinets_setting == NULL){
		goto ERR;
	}

	count = terminal_udp_search_terminal(cabinets_setting);

	in_buf[postion++] = count;
	for (i = 0; i < count; i++){
		memcpy(in_buf + postion, cabinets_setting->attribute[i].ca_SettingIP, 24);
		postion += 24;

		cmdport = cabinets_setting->attribute[i].port;
		in_buf[postion++] = (cmdport >> 24)&(0xFF);
		in_buf[postion++] = (cmdport >> 16)&(0xFF);
		in_buf[postion++] = (cmdport >> 8)&(0xFF);
		in_buf[postion++] = (cmdport)&(0xFF);

		dataport = cabinets_setting->attribute[i].dataport;
		in_buf[postion++] = (dataport >> 24)&(0xFF);
		in_buf[postion++] = (dataport >> 16)&(0xFF);
		in_buf[postion++] = (dataport >> 8)&(0xFF);
		in_buf[postion++] = (dataport)&(0xFF);

		memcpy(in_buf + postion, cabinets_setting->attribute[i].ca_TerminalID, 12);

		postion += 12;

		in_buf[postion++] = (cabinets_setting->attribute[i].i_USBSumNumber >> 24) & 0xff;
		in_buf[postion++] = (cabinets_setting->attribute[i].i_USBSumNumber >> 16) & 0xff;
		in_buf[postion++] = (cabinets_setting->attribute[i].i_USBSumNumber >> 8) & 0xff;
		in_buf[postion++] = (cabinets_setting->attribute[i].i_USBSumNumber) & 0xff;

		memcpy(in_buf + postion, cabinets_setting->attribute[i].ca_SoftVesion, 8);
		postion += 8;
		memcpy(in_buf + postion, cabinets_setting->attribute[i].ca_TerminalModel, 10);
		postion += 10;

		if (malloc_size <= (postion * 2)){
			malloc_size *= 2;
			in_buf = realloc(in_buf, malloc_size);
			if (!in_buf){
				goto ERR;
			}
		}
	}
	send_client_protocol(sockfd, 0xA9, in_buf, postion);
	if (in_buf)free(in_buf);
	if (cabinets_setting)free(cabinets_setting);
	return;
ERR:
	if (in_buf)free(in_buf);
//	if (cabinets_setting)free(cabinets_setting);
	return err_handle(sockfd, DF_SYSTEM_ERROR);
}


/*********************************************************************
-   Function : client_query_cabinet_network_param
-   Description：获取终端网络参数
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二：终端ID
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_cabinet_network_param(SOCKET sockfd, char* terminal_id)
{
	struct Cabinet_Network_Param network_param = {0};
	unsigned char  out_buf[1024] = { 0 };
	int32 ret = 0, postion = 0;

	if ((ret = terminal_query_cabinet_network_param(&network_param, terminal_id)) < 0){
		WriteLogFile("查询终端网络参数失败");
		return err_handle(sockfd, ret);
	}
	if ((ret = terminal_query_cabinet_FTP_param(&network_param, terminal_id)) < 0){
		WriteLogFile("查询终端FTP失败");
		return err_handle(sockfd, ret);
	}

	out_buf[postion++] = network_param.network_type;
	memcpy(out_buf + postion, network_param.ip, strlen(network_param.ip));
	postion += 24;
	memcpy(out_buf + postion, network_param.mask, strlen(network_param.mask));
	postion += 24;
	memcpy(out_buf + postion, network_param.gate, strlen(network_param.gate));
	postion += 24;
	memcpy(out_buf + postion, network_param.DNS, strlen(network_param.DNS));
	postion += 24;
	memcpy(out_buf + postion, network_param.FTP_upgrade_address, 60);
	postion += 50;
	memcpy(out_buf + postion, network_param.FTP_user, 20);
	postion += 20;
	memcpy(out_buf + postion, network_param.FTP_secret, 20);
	postion += 20;
	WriteLogFile("查询终端网络参数成功");
	return send_client_protocol(sockfd, 0xAC, out_buf, postion);
}

/*********************************************************************
-   Function : client_set_cabinet_network_param
-   Description：获取机柜网络参数
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_set_cabinet_network_param(SOCKET sockfd, char* terminal_id,unsigned char *in_buf)
{
	struct Cabinet_Network_Param network_param = {0};
	int postion = 12, ret = 0;

	network_param.network_type = in_buf[postion++];
	memcpy(network_param.ip, in_buf + postion, 24);
	postion += 24;
	memcpy(network_param.mask, in_buf + postion, 24);
	postion += 24;
	memcpy(network_param.gate, in_buf + postion, 24);
	postion += 24;
	memcpy(network_param.DNS, in_buf + postion, 24);
	postion += 24;
	memcpy(network_param.FTP_upgrade_address, in_buf + postion, 50);
	postion += 50;
	memcpy(network_param.FTP_user, in_buf + postion, 20);
	postion += 20;
	memcpy(network_param.FTP_secret, in_buf + postion, 20);
	postion += 20;

	ret = TerminalSetCabinetNetwork(terminal_id, &network_param);
	if (ret < 0){
		WriteLogFile("终端网络参数设置失败");
		err_handle(sockfd, ret);
		return;
	}

	WriteLogFile("设置网络参数成功");
	return send_client_protocol(sockfd, 0xAd, NULL, 0);

}



/*********************************************************************
-   Function : client_upgrade_program
-   Description：更新机柜程序
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_upgrade_program(SOCKET sockfd, char* terminal_id)
{
	int ret = 0;

	ret = terminal_upgrade_program(terminal_id);
	if (ret != 0){
		WriteLogFile("升级终端失败");
	    return err_handle(sockfd, ret);
	}
	return send_client_protocol(sockfd, 0xAE, NULL, 0);
}







/*********************************************************************
-   Function : client_query_cabinet_attribute
-   Description：查询终端机柜属性
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_cabinet_attribute(SOCKET sockfd)
{
	int ret = 0, i = 0, postion = 0;
	unsigned char out_buf[20480] = {0};
	char port_asc[10] = {0};
	struct CabinetInfo *cabinet = (struct CabinetInfo*)calloc(1 ,sizeof(struct CabinetInfo) * 128);

	if (cabinet == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}

	ret = terminal_query_cabinet_attribute(cabinet, 128);

	for (i = 0; i < ret; i++){
		out_buf[postion++] = ret;
		memset(port_asc, 0, sizeof(port_asc));
		_itoa(cabinet[i].port, port_asc, 10);
		out_buf[postion++] = strlen(cabinet[i].ca_SettingIP) + strlen(port_asc) + 1;
		sprintf(out_buf + postion, "%s:%s", cabinet[i].ca_SettingIP, port_asc);
		postion += (strlen(cabinet[i].ca_SettingIP) + strlen(port_asc) + 1);

		out_buf[postion++] = cabinet[i].terminal_system_state;
		out_buf[postion++] = 1;

	}
	if (cabinet)free(cabinet);
	return send_client_protocol(sockfd, 0xE7, out_buf, postion);

}


/*********************************************************************
-   Function : client_query_midware_setting
-   Description：获取配置进中间件的ip/port
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_midware_setting(SOCKET sockfd)
{
	unsigned char out_buf[20480] = { 0 }, count = 0, ret = 0;
	int postion = 0;
	int cmd_port, data_port;
	unsigned char guid[38] = {0};
	struct file_setting file_setting[128] = {0};

	read_guid(guid, 38);

	ret = read_setting(file_setting, 128);
	if (ret < 0){
		WriteLogFile("读配置文件出错");
		return err_handle(sockfd, DF_ERR_READ_SETTING);
	}

	out_buf[postion++] = ret;
	for (count = 0; count < ret; count++){
		memcpy(out_buf + postion, file_setting[count].terminal_ip, 24);
		postion += 24;

		cmd_port = file_setting[count].terminal_port;
		data_port = file_setting[count].data_port;

		out_buf[postion++] = ((int)cmd_port >> 24)&(0xFF);
		out_buf[postion++] = ((int)cmd_port >> 16)&(0xFF);
		out_buf[postion++] = ((int)cmd_port >> 8)&(0xFF);
		out_buf[postion++] = ((int)cmd_port)&(0xFF);

		out_buf[postion++] = ((int)data_port >> 24)&(0xFF);
		out_buf[postion++] = ((int)data_port >> 16)&(0xFF);
		out_buf[postion++] = ((int)data_port >> 8)&(0xFF);
		out_buf[postion++] = ((int)data_port)&(0xFF);
	}
	WriteLogFile("查询中间件设置成功");
	return send_client_protocol(sockfd, 0xAA, out_buf, postion);
}


/*********************************************************************
-   Function : client_query_midware_setting_A
-   Description：获取中间件设置_A
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_query_midware_setting_A(SOCKET sockfd)
{
	int postion = 0;
	unsigned char* out_buf = (char*)calloc(1 , 200000), count = 0, ret = 0;
	int cmd_port, data_port;
	char guid[38] = {0};
	struct file_setting file_setting[128];

	read_guid(guid, 38);

	ret = read_setting(file_setting, 128);
	if (ret < 0){
		WriteLogFile("读配置文件出错");//释放内存
		return err_handle(sockfd, DF_ERR_READ_SETTING);
	}

	out_buf[postion++] = ret;
	for (count = 0; count < ret; count++){
		memcpy(out_buf + postion, file_setting[count].terminal_ip, 100);
		postion += 100;

		cmd_port = file_setting[count].terminal_port;
		data_port = file_setting[count].data_port;

		out_buf[postion++] = ((int)cmd_port >> 24)&(0xFF);
		out_buf[postion++] = ((int)cmd_port >> 16)&(0xFF);
		out_buf[postion++] = ((int)cmd_port >> 8)&(0xFF);
		out_buf[postion++] = ((int)cmd_port)&(0xFF);

		out_buf[postion++] = ((int)data_port >> 24)&(0xFF);
		out_buf[postion++] = ((int)data_port >> 16)&(0xFF);
		out_buf[postion++] = ((int)data_port >> 8)&(0xFF);
		out_buf[postion++] = ((int)data_port)&(0xFF);
	}
	WriteLogFile("查询中间件设置成功");
	send_client_protocol(sockfd, 0xB1, out_buf, postion);
	if(out_buf)free(out_buf);
	return;
}

/*********************************************************************
-   Function : client_updata_midware_setting
-   Description：更新中间件设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二：输入的ip 数据端口号 命令端口号数据
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_updata_midware_setting(SOCKET sockfd, char*buf)
{
	struct file_setting file_setting[100] = {0};
	int count = buf[0], ret = 0, i = 0, postion = 0, cmd_port = 0, data_port = 0;
	char buf_tmp[100] = {0};

	memcpy(buf_tmp, buf, 60);
	postion = 1;

	WriteLogFile("配置%d台设备进中间件", count);
	for (i = 0; i < count; i++){
		memcpy(file_setting[i].terminal_ip, &buf[postion], 24);
		WriteLogFile("ip = %s", file_setting[i].terminal_ip);
		postion += 24;
		cmd_port = (((int)buf[postion] << 24)&(0xFF000000)) + (((int)buf[postion + 1] << 16)&(0x00FF0000)) + ((((int)buf[postion + 2] << 8)&(0x0000FF00)) + (((int)buf[postion + 3])&(0xFF)));
		file_setting[i].terminal_port = cmd_port;
		WriteLogFile("cmd_port = %d" , cmd_port);
		postion += 4;
		data_port = ((((int)buf[postion] << 24)&(0xFF000000)) + (((int)buf[postion + 1] << 16)&(0x00FF0000)) + (((int)buf[postion + 2] << 8)&(0x0000FF00)) + (((int)buf[postion + 3])&(0xFF)));
		file_setting[i].data_port = data_port;
		WriteLogFile("data_port = %d", data_port);
		postion += 4;
	}
	ret = write_setting(file_setting, count);
	if (ret <= 0){
		WriteLogFile(" 写配置文件出错");
		return err_handle(sockfd, DF_ERR_NO_WRTITE_SETTING);
	}

	WriteLogFile(" 写配置文件成功");
	Sleep(2500);
	return send_client_protocol(sockfd, 0xAB, NULL, 0);

}



/*********************************************************************
-   Function : client_updata_midware_setting_A
-   Description：更新中间件设置_A
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二：输入的ip 数据端口号 命令端口号数据
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_updata_midware_setting_A(SOCKET sockfd, char*buf)
{
	struct file_setting *file_setting = (struct file_setting*)calloc(1 , sizeof(struct file_setting) * 128);
	int count = buf[0], ret = 0, i = 0, postion = 0, cmd_port = 0, data_port = 0;
	char buf_tmp[100] = { 0 }, out_buf[100] = {0};
	struct hostent *h;

	if (file_setting == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	postion = 1;

	if (count < 0 || count > 127){
		if (file_setting)free(file_setting);
		return err_handle(sockfd, DF_ERR_NO_WRTITE_SETTING);
	}

	for (i = 0; i < count; i++){
		memcpy(file_setting[i].terminal_ip, buf + postion, 100);
		postion += 100;

		h = gethostbyname(file_setting[i].terminal_ip);
		if (h == NULL){
			WriteLogFile("域名解析错误\n");
			postion += 8;
			continue;
		}
		cmd_port = (((int)buf[postion] << 24)&(0xFF000000)) + (((int)buf[postion + 1] << 16)&(0x00FF0000)) + ((((int)buf[postion + 2] << 8)&(0x0000FF00)) + (((int)buf[postion + 3])&(0xFF)));
		file_setting[i].terminal_port = cmd_port;
		postion += 4;
		data_port = ((((int)buf[postion] << 24)&(0xFF000000)) + (((int)buf[postion + 1] << 16)&(0x00FF0000)) + (((int)buf[postion + 2] << 8)&(0x0000FF00)) + (((int)buf[postion + 3])&(0xFF)));
		file_setting[i].data_port = data_port;
		postion += 4;
	}
	ret = write_setting(file_setting, count);
	if (ret <= 0){
		if (file_setting)free(file_setting);
		WriteLogFile(" 写配置文件出错");
		return err_handle(sockfd, DF_ERR_NO_WRTITE_SETTING);
	}

	if (file_setting)free(file_setting);
	WriteLogFile(" 写配置文件成功");
	return send_client_protocol(sockfd, 0xB2, NULL, 0);
}

/*********************************************************************
-   Function : clientGetTerminalCAInfo
-   Description：获取纳税人名称和识别号
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void clientGetTerminalCAInfo(SOCKET sockfd, char* terminal_id)
{
#define OUT_BUF_LEN 20000
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = malloc(OUT_BUF_LEN);
	struct CabinetInfo cabinet = { 0 };

	if (out_buf == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{
		memset(out_buf, 0, OUT_BUF_LEN);
		outbuf_size = OUT_BUF_LEN;
		ret = TerminalGetTerminalCAInfo(terminal_id, &cabinet);
		if (ret != 0){
			WriteLogFile("查询ca纳税人信息失败");
			err_handle(sockfd, ret);
		}
		else{
			out_buf[postion++] = cabinet.i_USBSumNumber;
			for (int i = 0; i < cabinet.i_USBSumNumber; i++){
				out_buf[postion++] = i + 1;
				out_buf[postion++] = cabinet.port_state[i].c_port_state;
				out_buf[postion++] = cabinet.port_state[i].supportCA;
				memcpy(out_buf + postion, cabinet.port_state[i].CAName, 150);
				postion += 150;
			}
			send_client_protocol(sockfd, 0xB8, out_buf, postion);
		}
		if (out_buf != NULL)free(out_buf);
	}
}
/*********************************************************************
-   Function : ClientGetUSBListInfo
-   Description：新版获取纳税人名称和识别号
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetUSBListInfo(SOCKET sockfd, char* terminal_id)
{
#define OUT_BUF_LEN 20000
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = calloc(1 , OUT_BUF_LEN);
	struct CabinetInfo cabinet = { 0 };

	if (!out_buf){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}

	outbuf_size = OUT_BUF_LEN;
	ret = TerminalGetTerminalUSBListInfo(terminal_id, &cabinet);
	if (ret != 0){
		if (out_buf)free(out_buf);
		WriteLogFile("查询ca纳税人信息失败");
		return err_handle(sockfd, ret);
	}

	out_buf[postion++] = cabinet.i_USBSumNumber;
	for (int i = 0; i < cabinet.i_USBSumNumber; i++){
		out_buf[postion++] = i + 1;//端口号
		out_buf[postion++] = cabinet.port_state[i].USB_Type;//USB类型

		memcpy(out_buf + postion, cabinet.port_state[i].VID, 2);//VID
		postion += 2;
		memcpy(out_buf + postion, cabinet.port_state[i].PID, 2);//PID
		postion += 2;
		out_buf[postion++] = cabinet.port_state[i].supportCA;//读到纳税人名称和识别号标志

		memcpy(out_buf + postion, cabinet.port_state[i].CAName, 110);//纳税人名称
		postion += 110;
		memcpy(out_buf + postion, cabinet.port_state[i].tax_type, 2);//盘类型
		postion += 2;
		memcpy(out_buf + postion, cabinet.port_state[i].tax_serial, 16);//税盘编号
		postion += 16;
		memcpy(out_buf + postion, cabinet.port_state[i].tax_extension, 2);//分盘号
		postion += 2;
		memcpy(out_buf + postion, cabinet.port_state[i].tax_number, 20);//纳税人识别号
		postion += 20;
	}
	send_client_protocol(sockfd, 0xBD, out_buf, postion);
	if (out_buf != NULL)free(out_buf);
	return;
}

/*********************************************************************
-   Function : ClientSetVPN
-   Description：设置VPN配置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientSetVPN(SOCKET sockfd, unsigned char* buf)
{
#define OUT_BUF_LEN 20000
	ST_ClientVPNSetting cleintvpndata = {0};
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = calloc(1 , OUT_BUF_LEN);

	if (out_buf == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	memcpy(cleintvpndata.terminal_id, buf, 12); postion += 12;
	/*if (check_terminal_id(cleintvpndata.terminal_id) != 0){
		if (out_buf != NULL)free(out_buf);
		return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}*/
		
	cleintvpndata.open = buf[postion++];
	cleintvpndata.login_type = buf[postion++];
	memcpy(cleintvpndata.VPNServerIP, buf + postion, 50); postion += 50;
	memcpy(cleintvpndata.VPNServerPort, buf + postion, 2); postion += 2;
	memcpy(cleintvpndata.account, buf + postion, 20); postion += 20;
	memcpy(cleintvpndata.passwd, buf + postion, 20); postion += 20;
	memcpy(cleintvpndata.FTPPort, buf + postion, 2); postion += 2;
	memcpy(cleintvpndata.FTPUsername, buf + postion, 20); postion += 20;
	memcpy(cleintvpndata.FTPPasswd, buf + postion, 20); postion += 20;

	outbuf_size = OUT_BUF_LEN;
	ret = terminal_set_vpn(cleintvpndata.terminal_id, &cleintvpndata);
	if (ret != 0){
		if (out_buf != NULL)free(out_buf);
		WriteLogFile("设置vpn参数失败");
		return err_handle(sockfd, ret);
	}

	postion = 0;
	send_client_protocol(sockfd, 0xB9, out_buf, postion);
	if (out_buf != NULL)free(out_buf);
	return;
}

/*********************************************************************
-   Function : clientSetAutoUpgrade
-   Description：设置自动升级配置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void clientSetAutoUpgrade(SOCKET sockfd, unsigned char* buf)
{
#define OUT_BUF_LEN 20000
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = calloc(1 , OUT_BUF_LEN), terminal_id[20];;
	struct CabinetInfo cabinet = { 0 };


	if (out_buf == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}

	outbuf_size = OUT_BUF_LEN;
	memset(out_buf, 0, OUT_BUF_LEN);
	memset(terminal_id, 0, sizeof(terminal_id));

	memcpy(terminal_id, buf, 12);
	/*if (check_terminal_id(terminal_id) != 0){
		if (out_buf != NULL)free(out_buf);
		return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}*/

	ret = terminal_set_autoupgrade(terminal_id, buf[12]);
	if (ret != 0){
		if (out_buf != NULL)free(out_buf);
		WriteLogFile("设置自动升级失败");
		return err_handle(sockfd, ret);
	}

	send_client_protocol(sockfd, 0xBA, out_buf, postion);
	if (out_buf != NULL)free(out_buf);
}
/*********************************************************************
-   Function : clientGetVPN
-   Description：获取VPN设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void clientGetVPN(SOCKET sockfd, unsigned char* buf)
{
#define OUT_BUF_LEN 20000
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = calloc(1 , OUT_BUF_LEN), terminal_id[20] = { 0 };;
	struct CabinetInfo cabinet = { 0 };
	ST_ClientVPNSetting vpn = {0};

	if (out_buf == NULL){
		return err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	memcpy(terminal_id, buf, 12);
	/*if (check_terminal_id(terminal_id) != 0){
		if (out_buf != NULL)free(out_buf);
		return err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}*/

	outbuf_size = OUT_BUF_LEN;
	ret = terminal_get_vpn(terminal_id, &vpn);
	if (ret != 0){
		WriteLogFile("获取VPN设置失败");
		if (out_buf != NULL)free(out_buf);
		return err_handle(sockfd, ret);
	}
	out_buf[postion++] = vpn.open;
	out_buf[postion++] = vpn.login_type;
	memcpy(out_buf + postion, vpn.VPNServerIP, 50); postion += 50;
	memcpy(out_buf + postion, vpn.VPNServerPort, 2); postion += 2;
	memcpy(out_buf + postion, vpn.account, 20); postion += 20;
	memcpy(out_buf + postion, vpn.passwd, 20); postion += 20;
	memcpy(out_buf + postion, vpn.FTPPort, 2); postion += 2;
	memcpy(out_buf + postion, vpn.FTPUsername, 20); postion += 20;
	memcpy(out_buf + postion, vpn.FTPPasswd, 20); postion += 20;
	send_client_protocol(sockfd, 0xBB, out_buf, postion);

	if (out_buf != NULL)free(out_buf);
}
/*********************************************************************
-   Function : clientGetAutoUpgrade
-   Description：获取是否自动升级VPN设置
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-           参数二：输入的终端ID指针
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void clientGetAutoUpgrade(SOCKET sockfd, unsigned char* buf)
{
#define OUT_BUF_LEN 20000
	int ret = 0, outbuf_size, postion = 0;
	unsigned char* out_buf = malloc(OUT_BUF_LEN), terminal_id[20];;
	struct CabinetInfo cabinet = { 0 };

	if (out_buf == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{
		memset(out_buf, 0, OUT_BUF_LEN);
		memset(terminal_id, 0, sizeof(terminal_id));

		memcpy(terminal_id, buf, 12);
		/*if (check_terminal_id(terminal_id) != 0){
			err_handle(sockfd, DF_ERR_TERMINAL_ID);
		}
		else*/
		{
			outbuf_size = OUT_BUF_LEN;
			ret = terminal_get_autoupgrade(terminal_id, out_buf, outbuf_size);
			if (ret != 0){
				WriteLogFile("获取设备自用升级功能失败");
				err_handle(sockfd, ret);
			}
			else{
				send_client_protocol(sockfd, 0xBC, out_buf, 1);
			}
		}
		if (out_buf != NULL)free(out_buf);
	}
}








/*********************************************************************
-   Function : client_query_terminal_soft_version
-   Description：获取终端软件版本
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static int client_query_terminal_soft_version(SOCKET sockfd, char* terminal_id)
{
	int ret = 0, i = 0, j = 0, postion = 0;
	char  flag = 0;
	unsigned char out_buf[20480] = { 0 };

	struct CabinetInfo  *cabinet = (struct CabinetInfo *)malloc(sizeof(struct CabinetInfo));

	if (cabinet == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{
		ret = terminal_query_terminal_soft_version(terminal_id, cabinet);
		if (ret < 0){
			WriteLogFile(" 没有连接的机柜");
			err_handle(sockfd, ret);
		}
		else{
			memcpy(out_buf, cabinet->ca_SoftName, strlen(cabinet->ca_SoftName));
			postion += 50;
			out_buf[postion++] = cabinet->ca_SoftType;
			memcpy(out_buf + postion, cabinet->ca_ReleaseData, strlen(cabinet->ca_ReleaseData));
			postion += 10;
			memcpy(out_buf + postion, cabinet->ca_SoftVersion, strlen(cabinet->ca_SoftVersion));
			postion += 100;
			memcpy(out_buf + postion, cabinet->ca_Author, strlen(cabinet->ca_Author));
			postion += 50;
			memcpy(out_buf + postion, cabinet->ca_SoftDescription, strlen(cabinet->ca_SoftDescription));
			postion += 50;
			memcpy(out_buf + postion, cabinet->ca_CodeType, strlen(cabinet->ca_CodeType));
			postion += 20;

			WriteLogFile("根据终端ID获取usb-share终端信息成功");
			send_client_protocol(sockfd, 0xb4, out_buf, postion);
		}
		free(cabinet);
	}
}


/*********************************************************************
-   Function : client_get_cabinet_attribute_by_id_all
-   Description：根据终端ID获取usb-share终端信息(包括终端软件版本)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_cabinet_attribute_by_id_all(SOCKET sockfd, char* terminal_id)
{
	int ret = 0, ret1 = 0, i = 0, j = 0, postion = 0;
	char flag = 0;
	unsigned char out_buf[20480] = {0};
	char opened_port[400] = {0};
	struct CabinetInfo  *cabinet = (struct CabinetInfo *)malloc(sizeof(struct CabinetInfo) * 128);
	struct file_setting	file[128] = {0};
	unsigned char terminal_ip[30] = { 0 };

	if (cabinet == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{
		ret = terminal_query_cabinet_attribute(cabinet, 128);

		ret1 = read_setting(file, 128);

		for (i = 0; i < ret; i++){
			if ((strcmp(terminal_id, cabinet[i].ca_TerminalID) != 0)){
				continue;
			}
			flag = 1;
			memcpy(out_buf + postion, cabinet[i].ca_SettingIP, 24);
			postion += 24;

			for (j = 0; j < 128; j++){
				if (strcmp(cabinet[i].ca_SettingIP, file[j].terminal_ip) == 0){
					out_buf[postion++] = ((file[j].terminal_port >> 24) & (0xFF));
					out_buf[postion++] = ((file[j].terminal_port >> 16) & (0xFF));
					out_buf[postion++] = ((file[j].terminal_port >> 8) & (0xFF));
					out_buf[postion++] = ((file[j].terminal_port >> 0) & (0xFF));

					out_buf[postion++] = ((file[j].data_port >> 24) & (0xFF));
					out_buf[postion++] = ((file[j].data_port >> 16) & (0xFF));
					out_buf[postion++] = ((file[j].data_port >> 8) & (0xFF));
					out_buf[postion++] = ((file[j].data_port >> 0) & (0xFF));
				}
			}


			out_buf[postion++] = (char)(cabinet[i].terminal_system_state);
			if (cabinet[i].terminal_system_state == 0){
				continue;
			}

			memcpy(out_buf + postion, cabinet[i].ca_TerminalID, 12);
			postion += 12;

			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 24) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 16) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 8) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 0) & (0xFF));


			memcpy(out_buf + postion, cabinet[i].ca_SoftVesion, 8);
			postion += 8;
			memcpy(out_buf + postion, cabinet[i].ca_TerminalModel, 10);
			postion += 10;
			for (j = 0; j < cabinet[i].i_USBSumNumber; j++){
				out_buf[postion++] = cabinet[i].port_state[j].c_port_state;
			}
			postion += (256 - cabinet[i].i_USBSumNumber);

			memcpy(out_buf + postion, cabinet[i].ca_SoftName, 50);//软件名称
			postion += 50;
			out_buf[postion++] = cabinet[i].ca_SoftType;//软件类型
			memcpy(out_buf + postion, cabinet[i].ca_ReleaseData, 10);//发布日期
			postion += 10;
			memcpy(out_buf + postion, cabinet[i].ca_SoftVersion, 100);//软件版本
			postion += 100;
			memcpy(out_buf + postion, cabinet[i].ca_Author, 50);//作者
			postion += 50;
			memcpy(out_buf + postion, cabinet[i].ca_SoftDescription, 50);//软件描述
			postion += 50;
			memcpy(out_buf + postion, cabinet[i].ca_CodeType, 20);//码字类型
			postion += 20;
		}
		if (flag == 0){
			err_handle(sockfd, DF_ERR_TERMINAL_OFFLINE);
		}
		else{
			WriteLogFile("根据终端ID获取usb-share终端信息成功");
			send_client_protocol(sockfd, 0xB6, out_buf, postion);
		}
		if (cabinet != NULL)free(cabinet);
	}
}



/*********************************************************************
-   Function : client_get_cabinet_attribute_by_id
-   Description：根据终端ID获取usb-share终端信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_cabinet_attribute_by_id(SOCKET sockfd, char* terminal_id)
{
	int  i = 0, j = 0, postion = 0;
	char flag = 0;
	unsigned char * out_buf = NULL;
	struct CabinetInfo  *cabinet = NULL;
	struct file_setting	*file = NULL;
	struct CabinetInfo  s_cabinet = { 0 };

	out_buf = (uint8*)malloc(20480);
	cabinet = (struct CabinetInfo *)malloc(sizeof(struct CabinetInfo) * 128);
	file = (struct file_setting *)malloc(sizeof(struct file_setting) * 128);
	
	if (!out_buf || !cabinet || !file){
		if (out_buf) free(out_buf);
		if (cabinet) free(cabinet);
		if (file) free(file);
		err_handle(sockfd, DF_SYSTEM_ERROR);
		return;
	}
	memset(out_buf, 0, sizeof(uint8)*20480);

	terminal_query_cabinet_attribute(cabinet, 128);
	read_setting(file, 128);


	for (i = 0; i < 128; i++){
		if ((strcmp(terminal_id, cabinet[i].ca_TerminalID) != 0)){
			continue;
		}
		flag = 1;
		memcpy(out_buf + postion, cabinet[i].ca_SettingIP, 24);
		postion += 24;

		for (j = 0; j < 128; j++){
			if ((strcmp(cabinet[i].ca_SettingIP, file[j].terminal_ip) == 0) && (cabinet[i].dataport == file[j].data_port) && (cabinet[i].port == file[j].terminal_port)){
				out_buf[postion++] = ((file[j].terminal_port >> 24) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 16) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 8) & (0xFF));
				out_buf[postion++] = ((file[j].terminal_port >> 0) & (0xFF));

				out_buf[postion++] = ((file[j].data_port >> 24) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 16) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 8) & (0xFF));
				out_buf[postion++] = ((file[j].data_port >> 0) & (0xFF));
			}
			//	WriteLogFile("file[%d].terminal_ip = %s , file[j].terminal_port = %d ,file[j].data_port = %d ", j, file[j].terminal_ip, file[j].terminal_port, file[j].data_port);
		}

		out_buf[postion++] = (char)(cabinet[i].terminal_system_state);
		if (cabinet[i].terminal_system_state == 0){
			continue;
		}

		memcpy(out_buf + postion, cabinet[i].ca_TerminalID, 12);
		postion += 12;

		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 24) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 16) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 8) & (0xFF));
		out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 0) & (0xFF));


		memcpy(out_buf + postion, cabinet[i].ca_SoftVesion, 8);
		postion += 8;
		memcpy(out_buf + postion, cabinet[i].ca_TerminalModel, 10);
		postion += 10;


		WriteLogFile("protocol_get_cabinet_all_usbport_state 11111");
		protocol_get_cabinet_all_usbport_state(cabinet[i].ca_SettingIP, cabinet[i].port, s_cabinet.port_state);

		for (j = 0; j < cabinet[i].i_USBSumNumber; j++){
			out_buf[postion++] = s_cabinet.port_state[j].c_port_state;
		}
		postion += (256 - cabinet[i].i_USBSumNumber);

	}
	if (flag == 0){
		err_handle(sockfd, DF_ERR_TERMINAL_OFFLINE);
	}
	else{
		WriteLogFile("根据终端ID获取usb-share终端信息成功");
		send_client_protocol(sockfd, 0xAf, out_buf, postion);
	}
	if (cabinet) free(cabinet);
	if (file) free(file);
	if (out_buf) free(out_buf);
}

/*********************************************************************
-   Function : client_get_cabinet_attribute_by_ip
-   Description：根据终端IP获取usb-share终端信息
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_cabinet_attribute_by_ip(SOCKET sockfd, uint8 * buf)
{
	char ip[200] = { 0 };
	int  cmdport = 0, dataport = 0;
	int ret = 0, i = 0, j = 0, postion = 0;
	unsigned char out_buf[20480] = { 0 };
	struct CabinetInfo  cabinet = { 0 };

	memcpy(ip, buf, 24);
	cmdport = ((((int)buf[24] << 24)&(0xff000000)) + (((int)buf[25] << 16)&(0x00ff0000)) + (((int)buf[26] << 8)&(0x0000ff00)) + (((int)buf[27])&(0x000000ff)));
	dataport = ((((int)buf[28] << 24)&(0xff000000)) + (((int)buf[29] << 16)&(0x00ff0000)) + (((int)buf[30] << 8)&(0x0000ff00)) + (((int)buf[31])&(0x000000ff)));
#ifdef NINGBO_BANK
	char remote_ip[100] = { 0 };
	for (int i = 1; i <= all_terminal_setting_table.count; i++){

		WriteLogFile("now配置中ip:%s\n", all_terminal_setting_table.terminal_setting[i].terminal_ip);
		if ((memcmp(all_terminal_setting_table.terminal_setting[i].terminal_ip, ip, strlen(ip)) == 0) && (cmdport == all_terminal_setting_table.terminal_setting[i].terminal_port) \
			&& (dataport == all_terminal_setting_table.terminal_setting[i].data_port))
		{
			WriteLogFile("配置中找到对应ip:%s\n", ip);
			memcpy(remote_ip, all_terminal_setting_table.terminal_setting[i].remote_ip, strlen(all_terminal_setting_table.terminal_setting[i].remote_ip));
			break;
		}
	}
	if (strlen(remote_ip) == 0)
	{
		WriteLogFile("未在配置中找到对应ip:%s",ip);
		err_handle(sockfd, DF_ERR_SOCKET_ERROR);
		return;
	}
	
#endif
	ret = protocol_get_cabinet_attribute(ip, cmdport, &cabinet);

	if (ret >= 0){
		memcpy(out_buf + postion, ip, strlen(ip));

		postion += 24;
		out_buf[postion++] = ((cmdport >> 24) & (0xFF));
		out_buf[postion++] = ((cmdport >> 16) & (0xFF));
		out_buf[postion++] = ((cmdport >> 8)  & (0xFF));
		out_buf[postion++] = ((cmdport >> 0)  & (0xFF));

		out_buf[postion++] = ((dataport >> 24) & (0xFF));
		out_buf[postion++] = ((dataport >> 16) & (0xFF));
		out_buf[postion++] = ((dataport >> 8)  & (0xFF));
		out_buf[postion++] = ((dataport >> 0)  & (0xFF));
		out_buf[postion++] = 1;
		memcpy(out_buf + postion, cabinet.ca_TerminalID, 12);
		postion += 12;

		out_buf[postion++] = ((cabinet.i_USBSumNumber >> 24) & (0xFF));
		out_buf[postion++] = ((cabinet.i_USBSumNumber >> 16) & (0xFF));
		out_buf[postion++] = ((cabinet.i_USBSumNumber >> 8) & (0xFF));
		out_buf[postion++] = ((cabinet.i_USBSumNumber >> 0) & (0xFF));


		memcpy(out_buf + postion, cabinet.ca_SoftVesion, 8);
		postion += 8;
		memcpy(out_buf + postion, cabinet.ca_TerminalModel, 10);
		postion += 10;

		WriteLogFile("protocol_get_cabinet_all_usbport_state 22222");
		ret = protocol_get_cabinet_all_usbport_state(ip, cmdport, cabinet.port_state);
		if (ret < 0){
			WriteLogFile(" 查询终端所有端口状态失败");
			err_handle(sockfd, ret);
		}
		else{
			for (j = 0; j < cabinet.i_USBSumNumber; j++){
				out_buf[postion++] = cabinet.port_state[j].c_port_state;
			}
			postion += (256 - cabinet.i_USBSumNumber);
		}
		WriteLogFile("根据终端IP获取usb-share终端信息成功");
		send_client_protocol(sockfd, 0xb0, out_buf, postion);
	}
	else{
		WriteLogFile(" 没有连接机柜");
		err_handle(sockfd, ret);
	}
}


/*********************************************************************
-   Function : client_get_cabinet_attribute_by_ip_A
-   Description：根据终端IP获取usb-share终端信息_A
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void client_get_cabinet_attribute_by_ip_A(SOCKET sockfd, unsigned char * buf)
{
	char ip[200] = {0};
	int  cmdport = 0, dataport = 0;
	int ret = 0, j = 0, postion = 0;
	char domain[100] = {0};
	unsigned char out_buf[20480] = {0};
	struct CabinetInfo  cabinet = { 0 };


	cmdport = ((((int)buf[100] << 24)&(0xff000000)) + (((int)buf[101] << 16)&(0x00ff0000)) + (((int)buf[102] << 8)&(0x0000ff00)) + (((int)buf[103])&(0x000000ff)));
	dataport = ((((int)buf[104] << 24)&(0xff000000)) + (((int)buf[105] << 16)&(0x00ff0000)) + (((int)buf[106] << 8)&(0x0000ff00)) + (((int)buf[107])&(0x000000ff)));

	memcpy(domain, buf, 100);
	//if (domain_resolve(domain, ip, sizeof(ip)) == -1){
	//	WriteLogFile("域名解析错误");
	//	err_handle(sockfd, DF_SYSTEM_ERROR);
	//}
	//else
	{
		ret = protocol_get_cabinet_attribute(ip, cmdport, &cabinet);
		if (ret >= 0){
			memcpy(out_buf + postion, domain, strlen(domain));

			postion += 100;
			out_buf[postion++] = ((cmdport >> 24) & (0xFF));
			out_buf[postion++] = ((cmdport >> 16) & (0xFF));
			out_buf[postion++] = ((cmdport >> 8)  & (0xFF));
			out_buf[postion++] = ((cmdport >> 0)  & (0xFF));

			out_buf[postion++] = ((dataport >> 24) & (0xFF));
			out_buf[postion++] = ((dataport >> 16) & (0xFF));
			out_buf[postion++] = ((dataport >> 8)  & (0xFF));
			out_buf[postion++] = ((dataport >> 0)  & (0xFF));

			out_buf[postion++] = 1;

			memcpy(out_buf + postion, cabinet.ca_TerminalID, 12);
			postion += 12;

			out_buf[postion++] = ((cabinet.i_USBSumNumber >> 24) & (0xFF));
			out_buf[postion++] = ((cabinet.i_USBSumNumber >> 16) & (0xFF));
			out_buf[postion++] = ((cabinet.i_USBSumNumber >> 8) & (0xFF));
			out_buf[postion++] = ((cabinet.i_USBSumNumber >> 0) & (0xFF));


			memcpy(out_buf + postion, cabinet.ca_SoftVesion, 8);
			postion += 8;
			memcpy(out_buf + postion, cabinet.ca_TerminalModel, 10);
			postion += 10;

			WriteLogFile("protocol_get_cabinet_all_usbport_state 33333");
			ret = protocol_get_cabinet_all_usbport_state(ip, cmdport, cabinet.port_state);
			if (ret < 0){
				WriteLogFile(" 查询终端所有端口状态失败");
				err_handle(sockfd, ret);
			}else{
				for (j = 0; j < cabinet.i_USBSumNumber; j++){
					out_buf[postion++] = cabinet.port_state[j].c_port_state;
				}
				postion += (256 - cabinet.i_USBSumNumber);
			}
			WriteLogFile("根据终端IP获取usb-share终端信息成功");
			send_client_protocol(sockfd, 0xb3, out_buf, postion);
		}
		else{
			WriteLogFile(" 没有连接机柜");
			err_handle(sockfd, ret);
		}
	}
}

/*********************************************************************
-   Function : ClientGetAreacode
-   Description：获取航信盘区域代码
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetAreacode(SOCKET sockfd, uint8* buf)
{
	unsigned char terminal_id[20] = { 0 }, areacode[100] = { 0 }, send_client_buf[100] = { 0 };
	int usbport = 0, ret = 0;


	memcpy(terminal_id, buf, 12);
	//if (check_terminal_id(terminal_id) != 0){
	//	err_handle(sockfd, DF_ERR_TERMINAL_ID);
	//}
	//else
	{
		usbport = (((int)buf[12] << 24)&(0xFF000000)) + (((int)buf[13] << 16)&(0x00FF0000)) + (((int)buf[14] << 8)&(0x0000FF00)) + (((int)buf[15])&(0x000000FF));

		ret = TerminalGetAreacode(terminal_id, usbport, areacode);
		if (ret < 0){
			err_handle(sockfd, ret);
		}
		else{
			memcpy(send_client_buf, areacode, 20);
			send_client_protocol(sockfd, 0xc6, send_client_buf, 20);
		}
	}
}

/*********************************************************************
-   Function : ClientGetInvoiceSum
-   Description：获取税盘某段时间已开发票发票总数
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void ClientGetInvoiceSum(SOCKET sockfd, unsigned char* buf)
{
	unsigned char terminal_id[20] = { 0 }, date[20] = { 0 }, send_client_buf[100] = { 0 };
	int usbport = 0, postion = 0, ret = 0;
	int i_InvoiceSum = 0;
	memcpy(terminal_id, buf + postion, 12); postion += 12;
	usbport = ((buf[postion] << 24) + (buf[postion + 1] << 16) + (buf[postion + 2] << 8) + buf[postion + 3]); postion += 4;
	memcpy(date, buf + postion, 6); postion += 6;

	/*if (check_terminal_id(terminal_id) != 0){
		err_handle(sockfd, DF_ERR_TERMINAL_ID);
	}
	else*/
	{
		ret = TerminalGetInvoiceSum(terminal_id, usbport, date, &i_InvoiceSum);
		if (ret < 0){
			err_handle(sockfd, ret);
		}
		else{
			send_client_buf[0] = (char)(i_InvoiceSum >> 24);
			send_client_buf[1] = (char)(i_InvoiceSum >> 16);
			send_client_buf[2] = (char)(i_InvoiceSum >> 8);
			send_client_buf[3] = (char)(i_InvoiceSum);
			send_client_protocol(sockfd, 0xc7, send_client_buf, 4);
		}
	}
}



int recv_client_portocol(SOCKET sockfd, unsigned char** out_buf)
{
	int len = 0, ret = 0, cmd = 0;
	unsigned char cmd_len[10] = {0};
	char* log_str = NULL;

	if ((ret = socket_recv(sockfd, cmd_len, 4)) != 4){
		if (ret == 0){
			return 0;
		}
		else{
			return -1;//-1
		}
	}

	len += ((((int)cmd_len[0]) << 24) & 0xFF000000);
	len += ((((int)cmd_len[1]) << 16) & 0x00FF0000);
	len += ((((int)cmd_len[2]) << 8) & 0x0000FF00);
	len += (((int)cmd_len[3]) & 0x000000FF);

	*out_buf = malloc(len + 10);
	if (*out_buf == NULL){
		return DF_SYSTEM_ERROR;
	}
	memset(*out_buf, 0, sizeof(char)*(len + 10));
	memcpy(*out_buf, cmd_len, 4);

	if ((ret = socket_recv(sockfd, *out_buf + 4, len)) != len){
		if (ret == 0){
			WriteLogFile("socket 链路已断开");
			free(*out_buf);
			return 0;
		}
		else{
			free(*out_buf);
			return DF_ERR_SOCKET_ERROR;//
		}
	}
	cmd = *(*out_buf + 4);
	switch (cmd){
	case 0x65:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收打开端口命令:0x65(101) %s", log_str);
		free(log_str);
		break;
	case 0x66:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收关闭端口命令:0x66(102) %s", log_str);
		free(log_str);
		break;
	case 0x67:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收查询usb端口属性命令:0x67(103) %s", log_str);
		free(log_str);
		break;
	case 0x69:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取中间件设置命令:0x69(105) %s", log_str);
		free(log_str);
		break;
	case 0x6a:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收更新中间件设置命令:0x6a(106) %s", log_str);
		free(log_str);
		break;
	case 0x6b:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("获取终端信息命令:0x6B(107) %s", log_str);
		free(log_str);
		break;
	case 0x6e:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取终端属性设置A命令:0x6e(110) %s", log_str);
		free(log_str);
		break;
	case 0x80:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收打开端口命令:0x80(128) %s", log_str);
		free(log_str);
		break;
	case 0x81:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收关闭端口命令:0x81(129) %s", log_str);
		free(log_str);
		break;
	case 0x82:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取终端指定usb端口详细信息命令:0x82(130) %s", log_str);
		free(log_str);
		break;
	case 0x83:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收重新启动指定终端命令:0x83(131) %s", log_str);
		WriteLogFile(log_str);
		free(log_str);
		break;
	case 0x84:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取已设置的终端信息命令:0x84(132) %s", log_str);
		free(log_str);
		break;
	case 0x87:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取中间件usb设备信息命令:0x87(135) %s", log_str);
		free(log_str);
		break;
	case 0x88:
		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取中间件的状态命令:0x88(136) %s", log_str);
		free(log_str);
		break;
	case 0x89:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收搜索终端命令:0x89(137) %s", log_str);
		free(log_str);
		break;
	case 0x8c:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取指定终端网络参数命令:0x8c(140) %s", log_str);
		free(log_str);
		break;
	case 0x8d:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收设置终端网络参数命令:0x8d(141) %s", log_str);
		free(log_str);
		break;
	case 0x8e:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收升级终端程序命令命令:0x8e(142) %s", log_str);
		free(log_str);
		break;
	case 0x8f:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取指定终端信息命令:0x8f(143) %s", log_str);
		free(log_str);
		break;
	case 0x91:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取中间件设置命令:0x91(145) %s", log_str);
		free(log_str);
		break;
	case 0x92:

		//log_str = malloc((len + 4) * 5 + 10);
		//memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		//hex2str(*out_buf, len + 4, log_str);
		//WriteLogFile("接收更新中间件设置命令:0x92(146) %s", log_str);
		//free(log_str);
		break;
	case 0x93:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收查询终端信息(支持域名解析)命令:0x93(147) %s", log_str);
		free(log_str);
		break;
	case 0x96:

		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收获取指定终端信息命令(包括终端软件版本信息)命令:0x96(150) %s", log_str);
		free(log_str);
		break;
	case 0x8a:
		break;
	case 0xA0:
		break;
	case 0xA1:
		break;
	case 0xA2:
		break;
	case 0xA3:
		log_str = malloc((len + 4) * 5 + 10);
		memset(log_str, 0, (sizeof(char)*(len + 4) * 5));
		hex2str(*out_buf, len + 4, log_str);
		WriteLogFile("接收税盘卷命令:0xA3 %s", log_str);
		free(log_str);
		break;
	case 0xA4:
		WriteLogFile("接收到发票查询进度命令");
		break;
	default:
		//WriteLogFile("未定义命令");
		break;
	}
	return len;

}

static void err2client(int err, int *ret_err, char *err_des)
{
	int i = 0;

	for (i = 0; i < sizeof(client_err) / sizeof(client_err[0]); i++){
		if (err == client_err[i].self_err){
			*ret_err = client_err[i].ret_err;
			if (err_des != NULL){
				memset(err_des, 0, strlen(err_des));
				memcpy(err_des, client_err[i].err_des, strlen(client_err[i].err_des));
				return;
			}
		}
	}
	return;
}

void err_handle(SOCKET sockfd, int err_num)
{

	char buf[1024] = { 0 }, ca_ErrDes[1024] = { 0 };
	int rt_err = 0;

	err2client(err_num, &rt_err, ca_ErrDes);

	buf[0] = (char)rt_err;//错误代码
	buf[1] = strlen(ca_ErrDes); //错误描述
	memcpy(buf + 2, ca_ErrDes, strlen(ca_ErrDes));

	send_client_protocol(sockfd, 255, buf, 2 + buf[1]);
	if (err_num == DF_SYSTEM_ERROR)//new add by whl
	{
		WriteLogFile("err_handle 系统错误!");
		exit(10);
	}
}



/*********************************************************************
-   Function : send_client_protocol
-   Description：返回给客户端命令
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void send_client_protocol(SOCKET sockfd, char cmd, char* in_buf, int inbuf_size)
{
	char* send_client_buf = malloc(inbuf_size + 1000);

	memset(send_client_buf, 0, sizeof(inbuf_size + 1000));
	send_client_buf[0] = (((inbuf_size + 1) >> 24)&(0xFF));
	send_client_buf[1] = (((inbuf_size + 1) >> 16)&(0xFF));
	send_client_buf[2] = (((inbuf_size + 1) >> 8)&(0xFF));
	send_client_buf[3] = (((inbuf_size + 1))&(0xFF));
	send_client_buf[4] = cmd;

	memcpy(send_client_buf + 5, in_buf, inbuf_size);
	socket_send(sockfd, send_client_buf, inbuf_size + 5);
	if (send_client_buf != NULL)
		free(send_client_buf);
	return;
}




/********************************************************************************老版本协议*************************************************************************/
/*********************************************************************
-   Function : old_open_port
-   Description：老命令 处理客户端打开命令
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
参数二: 打开的端口号
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_open_port(SOCKET sockfd, int usbport)
{
	int ret = 0;
	//ret = old_client_open_port(usbport);
	if (ret != 0){
		WriteLogFile("打开端口错误");
		return err_handle(sockfd, ret);
	}
	
	WriteLogFile("打开端口成功");
	return send_client_protocol(sockfd, 0xE5, NULL, 0);
	
}

/*********************************************************************
-   Function : old_close_port
-   Description：关闭端口(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_close_port(SOCKET sockfd, int usbport, int force)
{
	int ret = 0;

	/*if (force == 0){
		ret = old_client_close_port(usbport);
	}
	else{
		ret = old_client_close_port_force(usbport);
	}*/
	if (ret < 0){
		WriteLogFile("关闭端口错误");
		err_handle(sockfd, ret);
	}
	else{
		WriteLogFile("关闭端口成功");
		send_client_protocol(sockfd, 0xE6, NULL, 0);
	}

}
/*********************************************************************
-   Function : client_query_cabinet_attribute
-   Description：查询终端机柜属性(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_client_query_usbport_attribute(SOCKET sockfd, int usbport)
{
	int ret = 0, outbuf_size, postion = 0;
	unsigned char out_buf[1024] = { 0 };
	struct Port_State port_state = { 0 };

	outbuf_size = sizeof(out_buf);
	ret = old_terminal_query_usbport_state(usbport, &port_state);
	if (ret < 0){
		WriteLogFile("查询端口失败");
		err_handle(sockfd, ret);
	}
	else{
		out_buf[postion++] = port_state.c_port_state;
		out_buf[postion++] = strlen(port_state.device_ip);
		memcpy(out_buf + postion, port_state.device_ip, strlen(port_state.device_ip));
		postion += strlen(port_state.device_ip);

		out_buf[postion++] = 12;
		memcpy(out_buf + postion, port_state.cabinet_id, 12);
		postion += 12;
		out_buf[postion++] = ((port_state.usbport >> 24)&(0xFF));
		out_buf[postion++] = ((port_state.usbport >> 16)&(0xFF));
		out_buf[postion++] = ((port_state.usbport >> 8)&(0xFF));
		out_buf[postion++] = ((port_state.usbport)&(0xFF));

		if (port_state.c_port_state == 2){
			out_buf[postion++] = strlen(port_state.driver);
			memcpy(out_buf + postion, port_state.driver, strlen(port_state.driver));
			postion += strlen(port_state.driver);

			out_buf[postion++] = strlen(port_state.user_ip);
			memcpy(out_buf + postion, port_state.user_ip, strlen(port_state.user_ip));
			postion += strlen(port_state.user_ip);

			out_buf[postion++] = strlen(port_state.client_id);
			memcpy(out_buf + postion, port_state.client_id, 38);
			postion += 38;
		}
		WriteLogFile("查询usb端口属性成功");
		send_client_protocol(sockfd, 0xE7, out_buf, postion);
	}
}
/*********************************************************************
-   Function : old_client_query_cabinet_attribute_A
-   Description：查询终端机柜属性(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_client_query_cabinet_attribute_A(SOCKET sockfd)
{
	int ret = 0, i = 0, postion = 0;
	unsigned char out_buf[5000];
	char port_asc[100];
	struct CabinetInfo *cabinet = (struct CabinetInfo *)malloc(sizeof(struct CabinetInfo) * 128);
	if (cabinet == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{
		memset(cabinet, 0, sizeof(struct CabinetInfo) * 128);

		ret = old_query_cabinet_attribute(cabinet, 128);//返回内存表中配置的终端数量
		out_buf[postion++] = ret;

		for (i = 0; i < 128; i++){
			if (*cabinet[i].ca_SettingIP == 0){
				continue;
			}

			memset(port_asc, 0, sizeof(port_asc));
			sprintf(port_asc, "%s:%d", cabinet[i].ca_SettingIP, cabinet[i].port);
			out_buf[postion++] = strlen(port_asc);
			memcpy(out_buf + postion, port_asc, strlen(port_asc));
			postion += strlen(port_asc);
			out_buf[postion++] = 1;
			if (cabinet[i].terminal_system_state == 0){
				out_buf[postion++] = 1;
			}
			else{
				out_buf[postion++] = 0;
			}
			if (cabinet[i].terminal_system_state == 0)
			{
				continue;
			}
			out_buf[postion++] = 12;
			memcpy(out_buf + postion, cabinet[i].ca_TerminalID, 12);
			postion += 12;

			out_buf[postion++] = ((cabinet[i].i_USBOriginNum >> 24) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBOriginNum >> 16) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBOriginNum >> 8) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBOriginNum >> 0) & (0xFF));

			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 24) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 16) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 8) & (0xFF));
			out_buf[postion++] = ((cabinet[i].i_USBSumNumber >> 0) & (0xFF));
		}
		if (cabinet != NULL){
			free(cabinet);
		}
		send_client_protocol(sockfd, 0xEE, out_buf, postion);
	}
}
/*********************************************************************
-   Function : client_query_cabinet_attribute
-   Description：查询终端机柜属性(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_client_query_cabinet_attribute(SOCKET sockfd)
{
	int ret = 0, i = 0, postion = 0;
	unsigned char out_buf[20480] = { 0 };
	char port_asc[10] = { 0 };
	struct CabinetInfo *cabinet = (struct CabinetInfo *)malloc(sizeof(struct CabinetInfo) * 128);
	if (cabinet == NULL){
		err_handle(sockfd, DF_SYSTEM_ERROR);
	}
	else{

		memset(cabinet, 0, sizeof(struct CabinetInfo) * 128);

		ret = old_query_cabinet_attribute(cabinet, 128);
		if (ret == 0){
			free(cabinet);
			WriteLogFile(" 没有连接的机柜");
			err_handle(sockfd, DF_ERR_NO_ACCESS_CABINET);
		}
		else{
			out_buf[postion++] = ret;
			for (i = 0; i < 128; i++){
				if (cabinet[i].ca_SettingIP[0] == 0) continue;

				memset(port_asc, 0, sizeof(port_asc));
				_itoa(cabinet[i].port, port_asc, 10);
				out_buf[postion++] = strlen(cabinet[i].ca_SettingIP) + strlen(port_asc) + 1;
				sprintf(out_buf + postion, "%s:%s", cabinet[i].ca_SettingIP, port_asc);
				postion += (strlen(cabinet[i].ca_SettingIP) + strlen(port_asc) + 1);
				out_buf[postion++] = 1;
				out_buf[postion++] = 0;
			}
			if (cabinet != NULL){
				free(cabinet);
			}
			send_client_protocol(sockfd, 0xEB, out_buf, postion);
		}
	}
}
/*********************************************************************
-   Function : old_client_query_midware_setting
-   Description：获取中间件设置(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_client_query_midware_setting(SOCKET sockfd)
{
	unsigned char out_buf[20480] = { 0 };
	int cmd_port, data_port, postion = 0, count = 0, ret = 0;
	char guid[38] = { 0 };
	struct file_setting file_setting[128] = { 0 };

	read_guid(guid, 38);

	ret = old_read_setting(file_setting, 128);
	if (ret < 0){
		WriteLogFile("读配置文件出错");
		err_handle(sockfd, DF_ERR_READ_SETTING);
	}
	else{
		out_buf[postion++] = ret;
		for (count = 0; count < ret; count++){
			out_buf[postion] = strlen(file_setting[count].terminal_ip);
			postion++;
			memcpy(out_buf + postion, file_setting[count].terminal_ip, strlen(file_setting[count].terminal_ip));
			postion += strlen(file_setting[count].terminal_ip);

			cmd_port = file_setting[count].terminal_port;
			data_port = file_setting[count].data_port;

			out_buf[postion++] = (data_port >> 8)&(0xFF);
			out_buf[postion++] = (data_port)&(0xFF);
			out_buf[postion++] = (cmd_port >> 8)&(0xFF);
			out_buf[postion++] = (cmd_port)&(0xFF);
		}

		out_buf[postion++] = 38;
		memcpy(out_buf + postion, guid, 38);
		postion += 38;
		WriteLogFile("查询中间件设置成功");
		send_client_protocol(sockfd, 0xE9, out_buf, postion - 1);
	}
}
/*********************************************************************
-   Function : old_client_updata_midware_setting
-   Description：更新中间件设置(老版本协议)
-   Calls：
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :参数一: 客户端连接的socket描述符
-   Output :
-   Return :
-   Other :
***********************************************************************/
static void old_client_updata_midware_setting(SOCKET sockfd, unsigned char *buf)
{
	struct file_setting file_setting[100] = { 0 };
	int count = buf[0], ret = 0, i = 0, postion = 0, cmd_port = 0, data_port = 0;
	char buf_tmp[100];

	memcpy(buf_tmp, buf, 60);
	postion = 1;

	//解析上层命令
	for (i = 0; i < count; i++){
		memcpy(file_setting[i].terminal_ip, &buf[postion + 1], buf[postion]);
		postion += (1 + buf[postion]);
		data_port = ((((uint16)buf[postion] << 8)&(0xFF00)) + (((uint16)buf[postion + 1])&(0xFF)));
		if (data_port != 3240){
			data_port = 3240;
		}
		file_setting[i].data_port = data_port;
		postion += 2;
		cmd_port = ((((uint16)buf[postion] << 8)&(0xFF00)) + (((uint16)buf[postion + 1])&(0xFF)));
		file_setting[i].terminal_port = cmd_port;
		postion += 2;
	}
	//更新内存配置数据
	ret = old_write_setting(file_setting, count);
	if (ret <= 0){
		WriteLogFile(" 写配置文件出错");
		err_handle(sockfd, DF_ERR_NO_WRTITE_SETTING);
	}
	else{
		WriteLogFile("写配置文件成功");
		send_client_protocol(sockfd, 0xEa, NULL, 0);
	}
}

