//if (num == 1){
//	HANDLE old_handle, new_handle;
//	struct ip_info info_ip;
//	memset(&info_ip, 0, sizeof(struct ip_info));
//	memcpy(info_ip.ip, ip_group[0], strlen(ip_group[0]));
//	info_ip.port_old = 12312;
//	old_handle = CreateThread(NULL, 0, dev_find_thread_old, &info_ip, 0, NULL);
//	if (old_handle == NULL)
//	{
//		WriteLogFile("Create old_handle Thread err");
//		return DLL_ERROR_SYSTEM;
//	}
//	/*new_handle = CreateThread(NULL, 0, dev_find_thread_new,NULL, 0, NULL);
//	if (old_handle == NULL)
//	{
//		WriteLogFile("Create new_handle Thread err");
//		return DLL_ERROR_SYSTEM;
//	}*/

//	WaitForSingleObject(old_handle, 10000);
//	//WaitForSingleObject(new_handle, INFINITE);
//	printf("out thread!\n");
//	cJSON  *dir2, *dir3;
//	int j, repeat_flag = 0;
//	root = cJSON_CreateObject();

//	cJSON_AddItemToObject(root, "data", dir2 = cJSON_CreateArray());
//	for (i = 0; i < MAX_NUM; i++)
//	{
//	
//		if (dev_info_new[i].state == 1)
//		{
//			cJSON *p_layer;
//			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());

//			cJSON_AddStringToObject(p_layer, "ter_ip", dev_info_new[i].ip);
//			cJSON_AddStringToObject(p_layer, "ter_id", dev_info_new[i].ter_id);
//			memset(port_num, 0, sizeof(port_num));
//			sprintf(port_num, "%d", dev_info_new[i].usb_port_num);
//			cJSON_AddStringToObject(p_layer, "usb_port_num", port_num);
//			cJSON_AddStringToObject(p_layer, "ter_version", dev_info_new[i].ter_version);
//			cJSON_AddStringToObject(p_layer, "ter_type", dev_info_new[i].ter_type);
//			cJSON_AddStringToObject(p_layer, "protocol_ver", dev_info_new[i].protocol_ver);
//			cJSON_AddStringToObject(p_layer, "online","1");
//		}
//	}
//	for (i = 0; i < MAX_NUM; i++)
//	{

//		if (dev_info_old[i].state == 1)
//		{
//			for (j = 0; j < MAX_NUM; j++)
//			{
//				if (memcmp(dev_info_old[i].ter_id, dev_info_new[j].ter_id, 12) == 0)
//				{
//					repeat_flag = 1;
//					break;
//				}
//			}
//			if (repeat_flag == 1)
//			{
//				repeat_flag = 0;
//				continue;
//			}
//			cJSON *p_layer;
//			cJSON_AddItemToObject(dir2, "dira", p_layer = cJSON_CreateObject());
//			cJSON_AddStringToObject(p_layer, "ter_ip", dev_info_old[i].ip);
//			cJSON_AddStringToObject(p_layer, "ter_id", dev_info_old[i].ter_id);
//			memset(port_num, 0, sizeof(port_num));
//			sprintf(port_num, "%d", dev_info_old[i].usb_port_num);
//			cJSON_AddStringToObject(p_layer, "usb_port_num", port_num);
//			cJSON_AddStringToObject(p_layer, "ter_version", dev_info_old[i].ter_version);
//			cJSON_AddStringToObject(p_layer, "ter_type", dev_info_old[i].ter_type);
//			cJSON_AddStringToObject(p_layer, "protocol_ver", dev_info_old[i].protocol_ver);
//			cJSON_AddStringToObject(p_layer, "online", "1");
//		}
//	}

//	tmp = cJSON_Print(root);
//	memcpy(output, tmp, strlen(tmp));
//	cJSON_Delete(root);
//}
//else