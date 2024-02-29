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
	// else{
	// 	WriteDetachFile("usb-detach.note",result_str);
	// }
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
	result = attach_usbip(info->server_ip, info->server_port, info->dev_port, info->trans_type, info->user,&info->sockfd,info->oper_id,&info->virtual_port);//��������
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
		{
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
			
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
		{
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
			
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
				// if (memcmp(line_info[i].user,line_info[point].user,strlen(line_info[point].user)) == 0)//�ͷ��Ѿ�ʧЧ��������ʹ�õ�client_id
				// {
				// 	WriteLogFile("client_id ��ʧЧ�������»�ȡ�µ�client_id!");
				// 	callback("client_id  guoqu client_id!");
				// 	line_info[point].use_flag = 0;
				// 	line_info[point].thread_flag = 0;
				// 	memset(&line_info[point], 0, sizeof(struct transmit_line_info));
					
				// 	line_info[i].use_flag =0;
				// 	line_info[i].thread_flag =0;
				// 	memset(&line_info[i], 0, sizeof(struct transmit_line_info));
				// 	r_Call[point] = callback;
				// 	ReleaseMutex(Mutex);
				// 	return DLL_ERROR_PARAM;
				// }
				
				// WriteLogFile("__zwl__line_info[point].user = %s   strlen(line_info[point].user)=%d      point = %d\r\n",line_info[point].user,strlen(line_info[point].user),point);
				if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port)//�ҵ�ƥ����·
				//if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port  && (memcmp(line_info[i].user,line_info[point].user,strlen(line_info[point].user)) == 0 || memcmp(line_info[i].user,"anyone",strlen("anyone")) == 0) )//�ҵ�ƥ����·
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
		
		
		//Sleep(2*1000);
		for(wait_time = 0; wait_time < 60; wait_time++)
		{
			if(line_info[point].thread_flag == 1)//�ȴ�usbip�߳̿�������ٷ���
				break;
			Sleep(100);
		}
		WriteLogFile("parse info ok , point is:%d ",point);
		return 0;
}


static int open_port_zwl(cJSON *root,char *output)
{
	int wait_time,i;
	int point;
	cJSON *item;
	char tmpdata[64] = {0x00};
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
		{
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
			
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
		{
			line_info[point].use_flag = 0;
			line_info[point].thread_flag = 0;
			memset(&line_info[point], 0, sizeof(struct transmit_line_info));
			ReleaseMutex(Mutex);
			return DLL_ERROR_PARAM;
		}
			
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
			// WriteLogFile("__zwl__line_info[point].user = %s   strlen(line_info[point].user)=%d      point = %d\r\n",line_info[point].user,strlen(line_info[point].user),point);
			if(line_info[i].use_flag == 1 &&line_info[i].thread_flag == 1 && point != i)//��ǰ��·���ڱ�ʹ��
			{
				if (memcmp(line_info[i].user,line_info[point].user,strlen(line_info[point].user)) == 0)//�ͷ��Ѿ�ʧЧ��������ʹ�õ�client_id
				{
					WriteLogFile("client_id ��ʧЧ,�����»�ȡ�µ�client_id!");
					memset(tmpdata,0,sizeof(tmpdata));
					sprintf(tmpdata,"%s","client_id ��ʧЧ,�����»�ȡ�µ�client_id!");
					memcpy(output, tmpdata, strlen(tmpdata));
					line_info[point].use_flag = 0;
					line_info[point].thread_flag = 0;
					memset(&line_info[point], 0, sizeof(struct transmit_line_info));
					
					line_info[i].use_flag =0;
					line_info[i].thread_flag =0;
					memset(&line_info[i], 0, sizeof(struct transmit_line_info));
					ReleaseMutex(Mutex);
					return DLL_ERROR_PARAM;
				}
				
				// WriteLogFile("__zwl__line_info[point].user = %s   strlen(line_info[point].user)=%d      point = %d\r\n",line_info[point].user,strlen(line_info[point].user),point);
				//if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port)//�ҵ�ƥ����·
				if(memcmp(line_info[i].ter_id,line_info[point].ter_id,12) == 0 && line_info[i].dev_port == line_info[point].dev_port  && (memcmp(line_info[i].user,line_info[point].user,strlen(line_info[point].user)) == 0 || memcmp(line_info[i].user,"anyone",strlen("anyone")) == 0) )//�ҵ�ƥ����·
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
		// r_Call[point] = callback;
		ReleaseMutex(Mutex);
		
		
		//Sleep(2*1000);
		for(wait_time = 0; wait_time < 60; wait_time++)
		{
			if(line_info[point].thread_flag == 1)//�ȴ�usbip�߳̿�������ٷ���
				break;
			Sleep(100);
		}
		WriteLogFile("parse info ok , point is:%d ",point);
		return 0;
}

static int close_port(cJSON *root)
{
	cJSON *item;
	int i,ret = -1;
	int wait_time;
	int dev_port;
	char ter_id[13] = {0};
	char user[64] = {0x00};
	cJSON *json = NULL;
	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *dir3, *dir4, *dir4_1;
	char str_port[10] = {0};
	char sendbuf[1024] = {0};
	char *g_buf = NULL;
	int tmp_socket = -1;
	
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

	WriteLogFile("�ر���·ָ��:�������:%s---�˿ں�:%d---�û�:%s!",ter_id,dev_port,user);

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
					if(0 != memcmp(line_info[i].server_ip,"103.27.4.61",strlen("103.27.4.61")) && 0 != memcmp(line_info[i].server_ip,"60.204.150.108",strlen("60.204.150.108")))
					{
						memset(sendbuf,0x00,sizeof(sendbuf));
						memcpy(sendbuf,"\x01\x06\x80\x07",4);
						json = cJSON_CreateObject();
						cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
						cJSON_AddStringToObject(dir1, "protocol", NEW_USBSHARE_PROTOCOL);
						cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_REQUEST);
						cJSON_AddStringToObject(dir1, "cmd", "close_port");
						cJSON_AddStringToObject(dir1, "source_topic", "");
						cJSON_AddStringToObject(dir1, "random", "987654321");
						cJSON_AddStringToObject(dir1, "result", "s");
						cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
						cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
						sprintf(str_port,"%d",dev_port);
						cJSON_AddStringToObject(dir3, "usb_port", str_port);
						g_buf = cJSON_Print(json);
						sendbuf[6] = strlen(g_buf)/256;sendbuf[7] = strlen(g_buf)%256;
						memcpy(sendbuf+8,g_buf,strlen(g_buf));

						memset(str_port,0x00,sizeof(str_port));
						sprintf(str_port,"%d",line_info[i].server_port);
						tmp_socket = tcp_connect(line_info[i].server_ip,str_port);
						if (INVALID_SOCKET != tmp_socket)
						{
							ret = usbip_send(tmp_socket, (void *) sendbuf, strlen(g_buf)+8);
							if (ret < 0) {
								WriteLogFile("apply_for_port_use send  err");
								//return USBIP_ERROR_APPLY_TER_USE;
							}
						}
						else
						{
							ret = usbip_send(line_info[i].sockfd, (void *) sendbuf, strlen(g_buf)+8);
							if (ret < 0) {
								WriteLogFile("apply_for_port_use send  err");
								//return USBIP_ERROR_APPLY_TER_USE;
							}
						}

						free(g_buf);
						cJSON_Delete(json);
						shutdown(tmp_socket,SD_BOTH);
						closesocket(tmp_socket);
					}


					//Sleep(1000);
					shutdown(line_info[i].sockfd,SD_BOTH);
					closesocket(line_info[i].sockfd);

					
					for(wait_time = 0; wait_time < 10; wait_time++)
					{
						//WriteLogFile("use_flag = %d,thread_flag = %d",line_info[i].use_flag,line_info[i].thread_flag);
						if(line_info[i].use_flag == 0 && line_info[i].thread_flag == 0)
							break;
						Sleep(100);
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
					Sleep(200);	
					// WaitForSingleObject(Mutex, INFINITE);
					// line_info[i].use_flag = 0;
					// line_info[i].thread_flag = 0;
					// memset(&line_info[i], 0, sizeof(struct transmit_line_info));
					// ReleaseMutex(Mutex);

					/*WaitForSingleObject(Mutex, INFINITE);
					line_info[i].use_flag = 0;
					line_info[i].thread_flag = 0;
					memset(&line_info[i], 0, sizeof(struct transmit_line_info));
					ReleaseMutex(Mutex);*/
					
					// WriteLogFile("�ر���·:�������:%s---�˿ں�:%d---�û�:%s!",ter_id,\
					// 		dev_port,user);
					
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
	
	return 0;//û���ҵ���ƥ�����·���޷��ر�
}

static int forced_close_port(cJSON *root)
{
	cJSON *item;
	int ret = -1,server_port = 0,usb_port = 0;
	char ter_id[13] = {0};
	char server_ip[64] = {0x00};
	cJSON *json = NULL;
	cJSON *dir1, *dir2;		//���ʹ��
	cJSON *dir3, *dir4, *dir4_1;
	char str_port[10] = {0},server_port_str[10] = {0x00};
	char sendbuf[1024] = {0};
	char *g_buf = NULL;
	int tmp_socket = -1;

	item = cJSON_GetObjectItem(root, "server_ip");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get server_ip error!");
		return DLL_ERROR_PARAM;
	}
	memcpy(server_ip, item->valuestring, strlen(item->valuestring));

	item = cJSON_GetObjectItem(root, "server_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get server_port error!");
		return DLL_ERROR_PARAM;
	}
	server_port = atoi(item->valuestring);
	memcpy(server_port_str,item->valuestring,strlen(item->valuestring));
	item = cJSON_GetObjectItem(root, "oper_port");
	if (item == NULL)
	{
		cJSON_Delete(root);
		WriteLogFile("get oper_port error!");
		return DLL_ERROR_PARAM;
	}
	usb_port = atoi(item->valuestring);
	

	WriteLogFile("forced_close_port:server_ip:%s,server_port = %d,usb_port:%d!",server_ip,server_port,usb_port);

	memset(sendbuf,0x00,sizeof(sendbuf));
	memcpy(sendbuf,"\x01\x06\x80\x07",4);
	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", NEW_USBSHARE_PROTOCOL);
	cJSON_AddStringToObject(dir1, "code_type", NEW_USB_SHARE_SYNC_CODE_REQUEST);
	cJSON_AddStringToObject(dir1, "cmd", "close_port");
	cJSON_AddStringToObject(dir1, "source_topic", "");
	cJSON_AddStringToObject(dir1, "random", "987654321");
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());
	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());
	sprintf(str_port,"%d",usb_port);
	cJSON_AddStringToObject(dir3, "usb_port", str_port);
	g_buf = cJSON_Print(json);
	sendbuf[6] = strlen(g_buf)/256;sendbuf[7] = strlen(g_buf)%256;
	memcpy(sendbuf+8,g_buf,strlen(g_buf));

	tmp_socket = tcp_connect(server_ip,server_port_str);
	if (INVALID_SOCKET == tmp_socket)
	{
		return -1;
	}
	else
	{
		ret = usbip_send(tmp_socket, (void *) sendbuf, strlen(g_buf)+8);
		if (ret < 0) {
			WriteLogFile("apply_for_port_use send  err");
			//return USBIP_ERROR_APPLY_TER_USE;
		}
	}
	free(g_buf);
	cJSON_Delete(json);
	Sleep(500);
	shutdown(tmp_socket,SD_BOTH);
	closesocket(tmp_socket);
}


static int get_loaded_ports(cJSON *root,char *output)
{
	int i = 0;
	char *tmp = NULL,tmpdata[32] = {0x00};
	cJSON *p_layer,*json_root,*dir2;
	json_root = cJSON_CreateObject();
	cJSON_AddStringToObject(json_root, "cmd", "get_loaded_ports");
	cJSON_AddItemToObject(json_root, "data", dir2 = cJSON_CreateArray());
	
	for(i = 0;i<20;++i)
	{
		if(line_info[i].use_flag != 0)
		{
			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
			cJSON_AddStringToObject(p_layer, "ter_id", line_info[i].ter_id);
			
			cJSON_AddStringToObject(p_layer, "server_ip", line_info[i].server_ip);
			memset(tmpdata,0x00,sizeof(tmpdata));
			sprintf(tmpdata,"%d",line_info[i].server_port);
			cJSON_AddStringToObject(p_layer, "server_port", tmpdata);

			memset(tmpdata,0x00,sizeof(tmpdata));
			sprintf(tmpdata,"%d",line_info[i].dev_port);
			cJSON_AddStringToObject(p_layer, "dev_port", tmpdata);

			memset(tmpdata,0x00,sizeof(tmpdata));
			sprintf(tmpdata,"%d",line_info[i].virtual_port);
			cJSON_AddStringToObject(p_layer, "virtual_port", tmpdata);

			cJSON_AddStringToObject(p_layer, "user", line_info[i].user);
			cJSON_AddStringToObject(p_layer, "oper_id", line_info[i].oper_id);
			//count++;
		}
	}
	
	tmp = cJSON_PrintUnformatted(json_root);
		
	memcpy(output, tmp, strlen(tmp));
	//WriteLogFile("tmp:%s\n", tmp);
	cJSON_Delete(json_root);
	free(tmp);
	return 0;
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
		return DLL_ERROR_INIT;
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
		WriteLogFile("open open step1 \n");
		result = open_port(root,callback);
		// result = open_port_zwl(root,output);
		cJSON_Delete(root);
		return result;
	}
	else if(memcmp(item->valuestring,"open_new",strlen("open_new")) == 0)//�µļ��ض˿�ʹ��
	{
		WriteLogFile("open_new open step1 \n");
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
	else if(memcmp(item->valuestring,"forced_close_new",strlen("forced_close_new")) == 0)//ǿ���ͷŶ˿�
	{
		result = forced_close_port(root);
		if(result == DLL_ERROR_NO_AUTHORITY)
		{
			strcpy(output,"Not the same user, cannot be released");
		}
		cJSON_Delete(root);
		return result;
	}
	else if(memcmp(item->valuestring,"get_loaded_ports",strlen("get_loaded_ports")) == 0)//��ȡ�Ѽ��صĶ˿�
	{
		result = get_loaded_ports(root,output);
		cJSON_Delete(root);
		return result;
	}
	else if(memcmp(item->valuestring,"close",5) == 0)//�ͷŶ˿�ʹ��
	{
		result = close_port(root);
		cJSON_Delete(root);
		return result;
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