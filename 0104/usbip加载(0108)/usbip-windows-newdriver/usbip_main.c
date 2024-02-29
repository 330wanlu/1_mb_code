#include "usbip_main.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)                                    //DLL��������ڣ���ʱĬ�ϲ���
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
				break;
		}
		case DLL_PROCESS_DETACH:
		{
				break;
		}
		case DLL_THREAD_ATTACH:
		{
				break;
		}
		case DLL_THREAD_DETACH:
		{
				break;
		}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}


/*
�ص�����ʵ������


*/
DWORD  Mycallfun( int i, char *result_str)
{
	if (r_Call[i]){

		WriteLogFile("r_Call!\n");
		//r_Call[i] = (result_CallBack)pfun;
		r_Call[i](result_str);
		return 0;
	}
	else{
		WriteDetachFile("usb-detach.note",result_str);
	}
	return -1;
}

DWORD prepare_error_callback(char *errbuf,char *errinfo)
{
	cJSON *root = NULL;
	char *tmp = NULL;
	int i;
	if(errbuf == NULL)
		return DLL_ERROR_PARAM;
	
	root = cJSON_CreateObject();
	if(root == NULL)
		return -1;

	cJSON_AddStringToObject(root,"cmd","openport");
	cJSON_AddStringToObject(root,"err",errinfo);
	tmp = cJSON_PrintUnformatted(root);
	memcpy(errbuf,tmp,strlen(tmp));
	free(tmp);
	cJSON_Delete(root);
	return 0;
}
DWORD JugdeErr2Msg(int errcode,char *errinfo)
{
	int i;
	WriteLogFile("errcode is:%d\n",errcode);
	for(i = 0; i < sizeof(usbip_err_n)/sizeof(struct _usbip_errinfo); i++)
	{
		if(errcode == usbip_err_n[i].ErrCode)
		{	
			strcpy(errinfo,usbip_err_n[i].ErrInfo);
			
			return 0;
		}

	}
	strcpy(errinfo,"Unkown err");
	return 0;

	/*switch(errcode){
	case -1:
		strcpy(errinfo,"tcp���ӷ�����ʧ��");
		break;
	case -2:
		strcpy(errinfo,"���������豸����ʧ�ܣ�����ԭ��δ��Ȩ��˿ں��������·�쳣");
		break;
	case -3:
		strcpy(errinfo,"���������ȡbusidʧ�ܣ�����ԭ����·�Ͽ�");
		break;
	case -4:
		strcpy(errinfo,"�˿�busidƥ��ʧ�ܣ�����ԭ����·�Ͽ����˿����豸");
		break;
	case -5:
		strcpy(errinfo,"δ��Ȩ��˿��豸���ڱ�ʹ�û�˿������豸!");
		break;
	case -6:
		strcpy(errinfo,"tcp�������ӷ�����ʧ��");
		break;
	case -7:
		strcpy(errinfo,"��������usb�豸ʧ�ܣ�����ԭ����·�Ͽ���������װ���޿�������˿ڵ�");
		break;
	case -8:
		strcpy(errinfo,"tcp����ת��������һ��ͨѶʧ��");
		break;
	case -99:
		strcpy(errinfo,"usb��·�Ѿ��ر�");
		break;
	default:
		strcpy(errinfo,"err unknown");
		break;
	
	}
	WriteLogFile("errcode is:%d\n",errcode);
	return 0;*/

}


/*********************************************************************
-   Function : Deal_Thread
-   Description�������������̣߳����߳��д����ľ��������·�򿪺ͼ����豸������Ĵ����߳�
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
DWORD WINAPI Deal_Thread(LPVOID lpParam)
{
	
	int result = 0;
	char logbuf[500] = { 0 };
	struct transmit_line_info * info = (struct transmit_line_info *)lpParam;
	//����ת��IP PORT ��PORT �ص�����
	int point = info->point;
	
	WriteLogFile("Deal_Thread,ip:%s---port:%d", info->server_ip, info->server_port);
	result = attach_usbip(info->server_ip, info->server_port, info->dev_port, info->trans_type, info->user,&info->sockfd,info->oper_id);//��������
	if (result != 0)
	{
		char errbuf[1000] = {0};
		char errmsg[300] = {0};
		JugdeErr2Msg(result,errmsg);
		WriteLogFile("call back in!--point:%d ,callback_addr:%p\n",point,r_Call[point]);
		result = prepare_error_callback(errbuf,errmsg);
		if(result < 0)//cjson operate err !
		{
			sprintf(errbuf,"{\"cmd\": \"openport\",\"err\": \"%s\"}",errmsg);
			Mycallfun(point, errbuf);
		}
		else
			Mycallfun(point, errbuf);

		WriteLogFile("call back finish!\n");
	}
	WaitForSingleObject(Mutex, INFINITE);
	info->use_flag = 0;
	info->thread_flag = 0;
	memset(info, 0, sizeof(struct transmit_line_info));
	ReleaseMutex(Mutex);
	return 0;

}


/*********************************************************************
-   Function : Main_Thread
-   Description�����̼߳�����ʵʱ����Ƿ����µ���������������̻߳�����
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
DWORD WINAPI Main_Thread(LPVOID lpParam)
{
	int i = 0;
	HANDLE deal_handle[20];
	WriteLogFile("Main_Thread in!");
	for (i = 0; i < 20; i++)
	{
		deal_handle[i] = NULL;
	}
	while (api_nThreadFlag == 1)
	{
		for (i = 0; i < 20; i++)
		{

			if (line_info[i].use_flag == 1 &&line_info[i].thread_flag!= 1 )
			{
				WriteLogFile("Deal_Thread in ,point is:%d!",i);
				WaitForSingleObject(Mutex, INFINITE);
				line_info[i].thread_flag = 1;
				ReleaseMutex(Mutex);
				deal_handle[i] = CreateThread(NULL, 0, Deal_Thread, &line_info[i], 0, NULL);
			}
		}
		Sleep(300);
	}
	for (i = 0; i < 20; i++)
	{
		if (deal_handle != NULL){
			WaitForSingleObject(deal_handle[i], INFINITE);
			CloseHandle(deal_handle[i]);
		}
	}
	return 0;
}



// խ�ַ���ת��Ϊ���ַ���,������ݱ��浽�ڶ��������У����Ҫ�ڵ��ú���֮ǰ����ÿռ�
//
BOOL WINAPI Ansi2Unicode(__in PCHAR pAnsi, __out PWCHAR pUnicode, __in DWORD dwStrLen)
{
	DWORD dwNum = 0;
	BOOL bRet = FALSE;
 
	do
	{
		// ������
		if (pAnsi == NULL || pUnicode == NULL || lstrlenA(pAnsi) == 0)
		{
			printf("��������...\n");
			break;
		}
 
		dwNum = MultiByteToWideChar(CP_ACP,0,pAnsi,-1,NULL,0);
		if (MultiByteToWideChar(CP_ACP,0, pAnsi, -1, pUnicode, dwNum))
		{
			bRet = TRUE;
		}
		else
		{
			printf("MultiByteToWideCharִ��ʧ��...�����룺%d\n", GetLastError());
			break;
		}
	}while (FALSE);
 
	return bRet;
}

// ���ַ���ת��Ϊխ�ַ���,������ݱ��浽�ڶ��������У����Ҫ�ڵ��ú���֮ǰ����ÿռ�
//
BOOL WINAPI Unicode2Ansi(__in PWCHAR pUnicode, __out PCHAR pAnsi, __in DWORD dwStrLen)
{
	DWORD dwNum = 0;
	BOOL bRet = FALSE;

	do
	{
		// ������
		if (pAnsi == NULL || pUnicode == NULL || lstrlenW(pUnicode) == 0)
		{
		//	printf("��������...\n");
			break;
		}

		dwNum = WideCharToMultiByte(CP_ACP, 0, pUnicode, dwStrLen, NULL, 0, NULL, NULL);
		if (WideCharToMultiByte(CP_ACP, 0, pUnicode, dwStrLen, pAnsi, dwNum, NULL, NULL))
		{
			bRet = TRUE;
		}
		else
		{
			//printf("WideCharToMultiByteִ��ʧ��...�����룺%d\n", GetLastError());
			break;
		}
	} while (FALSE);

	return bRet;
}


/*********************************************************************
-   Function : judge_driver_exit
-   Description���ж�ָ�������Ƿ�װ�ڱ��������
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ��driver_name:Ҫ����Ƿ���ڵ���������
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
int judge_driver_exsit(char *driver_name)
{

	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;
	// �õ������豸 HDEVINFO      
	hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return 0;
	// ѭ���о�     
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		char szClassBuf[MAX_PATH] = { 0 };
		char szDescBuf[MAX_PATH] = { 0 };
	//	char *ddBuf = (char *)malloc(500);
	//	char *dBuf = (char *)malloc(500);

		// ��ȡ����  
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS, NULL, (PBYTE)szClassBuf, MAX_PATH - 1, NULL))
		{
		//	free(ddBuf);
		//	free(dBuf);
			continue;
		}
		//��ȡ�豸������Ϣ
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)szDescBuf, MAX_PATH - 1, NULL))
		{
//			free(ddBuf);
//			free(dBuf);
			continue;
		}


		
		//memset(ddBuf, 0, 500);
		//Unicode2Ansi((PWCHAR)szClassBuf, ddBuf, -1);
		//printf("Class:%s\r\n\r\n", ddBuf);
	//	printf("Class:%s\r\nDesc:%s\r\n\r\n", szClassBuf, szDescBuf);
		
		
		
	//	memset(dBuf, 0, 500);
		//Unicode2Ansi((PWCHAR)szDescBuf, dBuf, -1);
	//	printf("Class:%s\r\nDesc:%s\r\n\r\n", ddBuf, dBuf);
	//	free(ddBuf);
		if (memcmp(szDescBuf, driver_name, strlen(driver_name)) == 0)
		{
		//	free(dBuf);
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return 0;
		}
	//	free(dBuf);
	}
	//  �ͷ�     
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return -1;
}

/*********************************************************************
-   Function : init_dll_interface
-   Description����ʼ����dll��һЩȫ�֡�������Դ�����̼߳��������
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
EXPORT_DLL int init_dll_interface()
{
	int ret = 0;
	int i = 0;
	unsigned short version = 0x202; /* winsock 2.2 */
	WSADATA data;

	char driver_name[50] = {0};

	// /*�����ļ����ʼ��*/
	// if (!terminal_setting_init()){
	// 	WriteLogFile("�����ļ���ʼ��ʧ��");
	// 	return 0;
	// }

	memcpy(driver_name,"USBSHARE VHCI",13);
	////memcpy(driver_name,"USB/IP VHCI",11);
	ret = judge_driver_exsit(driver_name);
	if(ret != 0){
		memset(driver_name,0,sizeof(driver_name));
		memcpy(driver_name,"USB/IP VHCI",11);
		ret = judge_driver_exsit(driver_name);
		if(ret != 0){
			WriteLogFile("judge_driver_exsit err");
			return DLL_ERROR_DRIVER;
		}
	}
	WriteLogFile("init_dll_interface ok");
	api_nThreadFlag = 1;

	

	ret = WSAStartup( version, &data);
	if (ret)
	{
		ret = GetLastError();
		WriteLogFile("WSAStartup err");
		return ret;
	}
	for (i = 0; i < 20; i++)
	{
		r_Call[i] = NULL;
		memset(&line_info[i], 0, sizeof(struct transmit_line_info));
	}
	Mutex = CreateMutex(NULL,FALSE,"main_mutex");
	if (Mutex == NULL)
	{
		WriteLogFile("create mutex error!");
		return DLL_ERROR_SYSTEM;
	}
	//�������������̣߳�ʵʱ����������������

	main_Handle = CreateThread(NULL, 0, Main_Thread, NULL, 0, NULL);
	if(main_Handle == NULL)
	{
		WriteLogFile("Create MAIN Thread err");
		return DLL_ERROR_SYSTEM;
	}
	dll_inint_flag = 1;
	return 0;
}


/*********************************************************************
-   Function : clean_dll_interface
-   Description���ͷű�dll�д򿪵�������Դ���̷߳���
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
EXPORT_DLL int clean_dll_interface()
{
	int i;
	int ret = WSACleanup();
	if (ret != 0){
		ret = GetLastError();
	}
	api_nThreadFlag = 0;
	for(i = 0; i <20;i++)//�رյ�ǰ�����Ѿ���������·
	{
		if(line_info[i].use_flag == 1&&line_info[i].thread_flag == 1)
		{
			shutdown(line_info[i].sockfd,SD_BOTH);
			closesocket(line_info[i].sockfd);
		}
	}
	WaitForSingleObject(main_Handle, INFINITE);
	CloseHandle(main_Handle);//�ȴ����̹߳ر�
	CloseHandle(Mutex);//�رջ�����
	return ret;
}

/*********************************************************************
-   Function : get_online_dev
-   Description����ȡ�����豸��Ϣ
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: output:���������ն˵�json����
-			
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
int get_online_dev(char *output)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	char *tmp = NULL;
	cJSON * c_array = NULL;
	int i;
	if(output == NULL)
		return DLL_ERROR_PARAM;
	
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"cmd","que_online");
	c_array = cJSON_CreateArray();
	cJSON_AddItemToObject(root,"data",c_array);
	
	for(i = 0; i < 20; i++)
	{
		if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1)
		{
			cJSON_AddItemToArray(c_array, item = cJSON_CreateObject());
			cJSON_AddStringToObject(item,"ter_id",line_info[i].ter_id);
			cJSON_AddStringToObject(item,"ip",line_info[i].server_ip);
			cJSON_AddNumberToObject(item,"dev_port",line_info[i].dev_port);

		}
	
	}
	tmp = cJSON_PrintUnformatted(root);
	memcpy(output,tmp,strlen(tmp));
	free(tmp);
	cJSON_Delete(root);
	return 0;
}

static int open_port(cJSON *root,result_CallBack callback)
{
	int wait_time,i;
	int point;
	cJSON *item;
	for (point = 0; point < 20; point++)
		{
			if (line_info[point].use_flag != 1)
				break;
		}
		if (point == 20)
			return DLL_ERROR_MEM;
		WaitForSingleObject(Mutex, INFINITE);
		
		item = cJSON_GetObjectItem(root, "server_ip");
		if (item == NULL)
		{
			WriteLogFile("get server_ip error!");
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
		memcpy(line_info[point].server_ip, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "server_port");
		if (item == NULL)
		{
			WriteLogFile("get _serv_port error!");
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
		WriteLogFile("get server_port %s\n",item->valuestring);
		if(item->type==cJSON_String)
			line_info[point].server_port = atoi(item->valuestring);
		else if(item->type==cJSON_Number)
			line_info[point].server_port = item->valueint;
		else
			return DLL_ERROR_PARAM;
		// line_info[point].server_port = item->valueint;//atoi(item->valuestring);
	
		item = cJSON_GetObjectItem(root, "dev_port");
		if (item == NULL)
		{
			WriteLogFile("get dev_port error!");
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
		if(item->type==cJSON_String)
			line_info[point].dev_port = atoi(item->valuestring);
		else if(item->type==cJSON_Number)
			line_info[point].dev_port = item->valueint;
		else
			return DLL_ERROR_PARAM;
		//line_info[point].dev_port = item->valueint; //atoi(item->valuestring);
		WriteLogFile("get dev_port %d\n",line_info[point].dev_port);

		item = cJSON_GetObjectItem(root, "ter_id");
		if (item == NULL)
		{
			WriteLogFile("get ter_id error!");
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
		memcpy(line_info[point].ter_id, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "user");
		if (item == NULL)  //ΪNULL��Ĭ��Ϊ ��anyone��
		{
			// WriteLogFile("get user error!");
			// line_info[point].use_flag = 0;
			// line_info[point].thread_flag = 0;
			// memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			// ReleaseMutex(Mutex);
			// return DLL_ERROR_PARAM;
			memcpy(line_info[point].user, "anyone", strlen("anyone"));
		}
		else
			memcpy(line_info[point].user, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "oper_id");
		if (item == NULL)  //ΪNULL��Ĭ��Ϊ ��anyone��
		{
			memcpy(line_info[point].oper_id, "anyone", strlen("anyone"));
		}
		else
			memcpy(line_info[point].oper_id, item->valuestring, strlen(item->valuestring));

		item = cJSON_GetObjectItem(root, "trans_type");
		if (item == NULL)  //û��trans_type ʱ��Ĭ��Ϊ0
		{
			// WriteLogFile("get trans_type error!");
			// line_info[point].use_flag = 0;
			// line_info[point].thread_flag = 0;
			// memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			// ReleaseMutex(Mutex);
			// return DLL_ERROR_PARAM;
			line_info[point].trans_type = 0;
		}
		else
		{
			if(item->type==cJSON_String)
				line_info[point].trans_type = atoi(item->valuestring);
			else if(item->type==cJSON_Number)
				line_info[point].trans_type = item->valueint;

			//  line_info[point].trans_type = item->valueint;
		}
		for(i = 0; i < 20; i++)
		{
			if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1 && point != i)//��ǰ��·���ڱ�ʹ��
			{
				//WriteLogFile();
				if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port)//�ҵ�ƥ����·
				{
					// WriteLogFile("�ö˿��Ѿ���ʹ����,ǿ�ƹر�֮ǰʹ��!");

					// shutdown(line_info[i].sockfd,SD_BOTH);
					// closesocket(line_info[i].sockfd);
					// Sleep(2000);
					// WaitForSingleObject(Mutex, INFINITE);
					// line_info[i].use_flag = 0;
					// line_info[i].thread_flag = 0;
					// memset(&line_info[i], 0, sizeof(struct transmit_line_info));
					// ReleaseMutex(Mutex);
					/*
					line_info[point].use_flag = 0;
					line_info[point].thread_flag = 0;
					memset(&line_info[point], 0, sizeof(struct transmit_line_info));
					ReleaseMutex(Mutex);
					return DLL_ERROR_SYSTEM;
					*/
					WriteLogFile("�ö˿��Ѿ���ʹ����!");
					ReleaseMutex(Mutex);
					//callback("The port is already used!!!!");
					return DLL_ERROR_DEVICE_USED;
				}
			}
		}

		line_info[point].point = point;
		line_info[point].use_flag = 1;
		r_Call[point] = callback;
		ReleaseMutex(Mutex);

		Sleep(2*1000);
		for(wait_time = 0; wait_time < 4; wait_time++)
		{
			if(line_info[point].thread_flag == 1)//�ȴ�usbip�߳̿�������ٷ���
				break;
			Sleep(1000);
		}
		WriteLogFile("parse info ok , point is:%d ",point);
		return 0;
}

static int close_port(cJSON *root)
{
	cJSON *item;
	int i;
	int wait_time;
	int dev_port;
	char ter_id[13] = {0};
	char user[64] = {0x00};
	
	item = cJSON_GetObjectItem(root, "ter_id");
	if (item == NULL)
	{
		WriteLogFile("close get ter_id error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(ter_id,item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "dev_port");
	if (item == NULL)
	{
		WriteLogFile("close get dev_port error!");
		return DLL_ERROR_PARAM;
	}
	if(item->type==cJSON_String)
		dev_port = atoi(item->valuestring);
	else if(item->type==cJSON_Number)
		dev_port = item->valueint;
	else
		return DLL_ERROR_PARAM;
	// dev_port = item->valueint;;//atoi(item->valuestring);

	item = cJSON_GetObjectItem(root, "user");
	if (item == NULL)
	{
		// WriteLogFile("close get user error!");
		// return DLL_ERROR_PARAM;
		strcpy(user,"anyone");
	}
	else
	{
		strcpy(user,item->valuestring);
	}

	WriteLogFile("�ر���·ָ��:�������:%s---�˿ں�:%d---�û�:%s!",ter_id,\
						dev_port,user);
	for(i = 0; i < 20; i++)
	{

		if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1)//��ǰ��·���ڱ�ʹ��
		{
			//WriteLogFile();

		//	close_port_share();
			if(memcmp(line_info[i].ter_id,ter_id,12) == 0 && line_info[i].dev_port == dev_port && line_info[i].sockfd != 0)//�ҵ�ƥ����·�����йر�
			{
				if((memcmp(user,line_info[i].user,(strlen(user)>strlen(line_info[i].user)?strlen(user):strlen(line_info[i].user))) == 0 || memcmp(user,"admin",strlen("admin")) == 0 || 0 == strlen(line_info[i].user)))
				{
					shutdown(line_info[i].sockfd,SD_BOTH);
					closesocket(line_info[i].sockfd);
					for(wait_time = 0; wait_time < 10; wait_time++)
					{
						if(line_info[i].use_flag == 0 && line_info[i].thread_flag == 0)
							break;
						Sleep(1000);
					}
					if(wait_time == 10)
					{
						WriteLogFile("�ر���·:�������:%s---�˿ں�:%d---�û�:%s--socket:%d���ܴ������⣬ǿ�ƹر�!",line_info[i].ter_id,\
							line_info[i].dev_port,line_info[i].user,line_info[i].sockfd);
						WaitForSingleObject(Mutex, INFINITE);
						line_info[i].use_flag = 0;
						line_info[i].thread_flag = 0;
						memset(&line_info[i], 0, sizeof(struct transmit_line_info));
						ReleaseMutex(Mutex);
					}
					/*WaitForSingleObject(Mutex, INFINITE);
					line_info[i].use_flag = 0;
					line_info[i].thread_flag = 0;
					memset(&line_info[i], 0, sizeof(struct transmit_line_info));
					ReleaseMutex(Mutex);*/
						
					return 0;
				}
				else
				{
					WriteLogFile("������û���ͬһ�û���Ҳ��ǿ�ƹرգ��رն˿�ʧ��\n");
					return DLL_ERROR_NO_AUTHORITY;
				}
			}
		}
	}
	WriteLogFile("�ͷŶ˿�δ�ҵ�ƥ����·\n");
	return DLL_ERROR_NOT_FOUND;//û���ҵ���ƥ�����·���޷��ر�
}



/*********************************************************************
-   Function : send_cmd_deal
-   Description������ָ�������ش���Ŀǰָ��Ϊ��ת����·ͨ�������豸���أ��͹ر���·ͨ���ͷ��豸���أ�
-   Calls��
-   Calles By:
-   Table Accessed :
-   Table Update :
-   Input  :����һ: Json_In:����ָ�����Ϣ��json����
-			������: callback:��Ӧ��Ϣ�Ļص�����
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����(ͨ���ĵ�������Ӧ�Ĵ�������)
-   Other :
***********************************************************************/
EXPORT_DLL int  send_cmd_deal(char * Json_In, result_CallBack callback,char *output)
//EXPORT_DLL int    send_cmd_deal(char * Json_In)
{
//	result_CallBack callback;
	cJSON *root = NULL;
	cJSON*item = NULL;
	int point,dev_port,i;
	int result = 0;
	char ter_id[13] = {0};
	int wait_time;
	
	if(dll_inint_flag!= 1)
	{
		WriteLogFile("error dll_inint_flag");
		return DLL_ERROR_PARAM;
	}
	if (Json_In == NULL )
	{
		WriteLogFile("error parm:Json_In is NULL!");
		return DLL_ERROR_PARAM;
	}
	
	root = cJSON_Parse(Json_In);
	item = cJSON_GetObjectItem(root, "cmd");
	if (item == NULL)
	{
		WriteLogFile("get cmd error!");
		return DLL_ERROR_PARAM;
	}

	if(memcmp(item->valuestring,"open",4) == 0)//���ض˿�ʹ��
	{
		WriteLogFile("open step1 \n");
		result = open_port(root,callback);
		cJSON_Delete(root);
		return result;
		
		//for (point = 0; point < 20; point++)
		//{
		//	if (line_info[point].use_flag != 1)
		//		break;
		//}
		//if (point == 20)
		//	return DLL_ERROR_MEM;
		//WriteLogFile("open step2 \n");
		//WaitForSingleObject(Mutex, INFINITE);
		//

	
		//WriteLogFile("open step3 \n");
		//item = cJSON_GetObjectItem(root, "server_ip");
		//if (item == NULL)
		//{
		//	WriteLogFile("get server_ip error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//memcpy(line_info[point].server_ip, item->valuestring, strlen(item->valuestring));

		//WriteLogFile("open step4 \n");
		//item = cJSON_GetObjectItem(root, "server_port");
		//if (item == NULL)
		//{
		//	WriteLogFile("get _serv_port error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//line_info[point].server_port = item->valueint;
	
		//WriteLogFile("open step5 \n");
		//item = cJSON_GetObjectItem(root, "dev_port");
		//if (item == NULL)
		//{
		//	WriteLogFile("get dev_port error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//line_info[point].dev_port = item->valueint;

		//WriteLogFile("open step6 \n");
		//item = cJSON_GetObjectItem(root, "ter_id");
		//if (item == NULL)
		//{
		//	WriteLogFile("get ter_id error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//memcpy(line_info[point].ter_id, item->valuestring, strlen(item->valuestring));

		//WriteLogFile("open step7 \n");
		//item = cJSON_GetObjectItem(root, "user");
		//if (item == NULL)
		//{
		//	WriteLogFile("get user error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//memcpy(line_info[point].user, item->valuestring, strlen(item->valuestring));

		//item = cJSON_GetObjectItem(root, "trans_type");
		//if (item == NULL)
		//{
		//	WriteLogFile("get trans_type error!");
		//	line_info[point].use_flag = 0;
		//	line_info[point].thread_flag = 0;
		//	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//	ReleaseMutex(Mutex);
		//	return DLL_ERROR_PARAM;
		//}
		//for(i = 0; i < 20; i++)
		//{
		//	if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1 && point != i)//��ǰ��·���ڱ�ʹ��
		//	{
		//		//WriteLogFile();
		//		if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port)//�ҵ�ƥ����·
		//		{
		//			WriteLogFile("�ö˿��Ѿ���ʹ����,ǿ�ƹر�֮ǰʹ��!");

		//			shutdown(line_info[i].sockfd,SD_BOTH);
		//			closesocket(line_info[i].sockfd);
		//			Sleep(2000);
		//			WaitForSingleObject(Mutex, INFINITE);
		//			line_info[i].use_flag = 0;
		//			line_info[i].thread_flag = 0;
		//			memset(&line_info[i], 0, sizeof(struct transmit_line_info));
		//			ReleaseMutex(Mutex);
		//			/*
		//			line_info[point].use_flag = 0;
		//			line_info[point].thread_flag = 0;
		//			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
		//			ReleaseMutex(Mutex);
		//			return DLL_ERROR_SYSTEM;
		//			*/
		//		}
		//	}
		//}
		//WriteLogFile("open step8 \n");
		//line_info[point].trans_type = item->valueint;
		//line_info[point].point = point;
		//line_info[point].use_flag = 1;
		//r_Call[point] = callback;
		//ReleaseMutex(Mutex);
		//
		//
		//Sleep(2*1000);
		//for(wait_time = 0; wait_time < 4; wait_time++)
		//{
		//	if(line_info[point].thread_flag == 1)//�ȴ�usbip�߳̿�������ٷ���
		//		break;
		//	Sleep(1000);
		//}
		//WriteLogFile("parse info ok , point is:%d ",point);
	}
	else if(memcmp(item->valuestring,"open_new",strlen("open_new")) == 0)//�µļ��ض˿�ʹ��
	{
		WriteLogFile("open step1 \n");
		result = open_port(root,callback);
		cJSON_Delete(root);
		return result;
	}
	else if(memcmp(item->valuestring,"close_new",strlen("close_new")) == 0)//�µ��ͷŶ˿�ʹ��
	{
		result = close_port(root);
		if(result == DLL_ERROR_NO_AUTHORITY)
		{
			strcpy(output,"Not the same user, cannot be released");
		}
		cJSON_Delete(root);
		return result;
	}
	else if(memcmp(item->valuestring,"close",5) == 0)//�ͷŶ˿�ʹ��
	{
		result = close_port(root);
		cJSON_Delete(root);
		return result;
		//item = cJSON_GetObjectItem(root, "ter_id");
		//if (item == NULL)
		//{
		//	WriteLogFile("close get ter_id error!");
		//	return DLL_ERROR_PARAM;
		//}
		//memcpy(ter_id,item->valuestring, strlen(item->valuestring));

		//item = cJSON_GetObjectItem(root, "dev_port");
		//if (item == NULL)
		//{
		//	WriteLogFile("close get dev_port error!");
		//	return DLL_ERROR_PARAM;
		//}
		//dev_port = item->valueint;

		//item = cJSON_GetObjectItem(root, "user");
		//if (item == NULL)
		//{
		//	WriteLogFile("close get user error!");
		//	return DLL_ERROR_PARAM;
		//}

		//WriteLogFile("�ر���·ָ��:�������:%s---�˿ں�:%d---�û�:%s!",ter_id,\
		//					dev_port,item->valuestring);
		//for(i = 0; i < 20; i++)
		//{

		//	if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1)//��ǰ��·���ڱ�ʹ��
		//	{
		//		//WriteLogFile();

		//	//	close_port_share();
		//		if(memcmp(line_info[i].ter_id,ter_id,12) == 0 && line_info[i].dev_port == dev_port&& \
		//			memcmp(item->valuestring,line_info[i].user,strlen(item->valuestring)) == 0 && line_info[i].sockfd != 0)//�ҵ�ƥ����·�����йر�
		//		{
		//			shutdown(line_info[i].sockfd,SD_BOTH);
		//			closesocket(line_info[i].sockfd);
		//			for(wait_time = 0; wait_time < 10; wait_time++)
		//			{
		//				if(line_info[i].use_flag == 0 && line_info[i].thread_flag == 0)
		//					break;
		//				Sleep(1000);
		//			}
		//			if(wait_time == 10)
		//			{
		//				WriteLogFile("�ر���·:�������:%s---�˿ں�:%d---�û�:%s--socket:%d���ܴ������⣬ǿ�ƹر�!",line_info[i].ter_id,\
		//					line_info[i].dev_port,line_info[i].user,line_info[i].sockfd);
		//				WaitForSingleObject(Mutex, INFINITE);
		//				line_info[i].use_flag = 0;
		//				line_info[i].thread_flag = 0;
		//				memset(&line_info[i], 0, sizeof(struct transmit_line_info));
		//				ReleaseMutex(Mutex);
		//			}
		//			/*WaitForSingleObject(Mutex, INFINITE);
		//			line_info[i].use_flag = 0;
		//			line_info[i].thread_flag = 0;
		//			memset(&line_info[i], 0, sizeof(struct transmit_line_info));
		//			ReleaseMutex(Mutex);*/
		//			
		//			return 0;
		//		}
		//	}
		//}
		//WriteLogFile("�ͷŶ˿�δ�ҵ�ƥ����·\n");
		//return DLL_ERROR_NOT_FOUND;//û���ҵ���ƥ�����·���޷��ر�
	}
	else if(memcmp(item->valuestring,"que_online",10) == 0)
	{
		char online_buf[1024*5] = {0};
		result = get_online_dev(online_buf);
		if(result != 0)
		{

			//callback("NOT FOUND");
			WriteLogFile("get_online_dev not found!");
			//return 0;
		}
		memset(output,0,strlen(output));//add by whl 0814
		memcpy(output,online_buf,strlen(online_buf));
		WriteLogFile("online_buf:%s",output);
		//callback(online_buf);
	}
	else
	{
		return DLL_ERROR_CMD;
	}
	return 0;
}