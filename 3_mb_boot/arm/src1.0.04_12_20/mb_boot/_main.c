#define _main_c
#include "_main.h"
 unsigned char update_msg[1024];
 unsigned char auth_msg[1024];
 int dns_err_count=0;
 //int main_app_run = 1;
 pthread_t   thread_update;
 pthread_t   thread_main_app;

void recvSignal(int sig)
{
	printf("================================received signal %d !!!================================\n", sig);
}
int main(int argc, char *argv[])
{   
	int result;
	char cmd[100] = { 0 };
	char buf_read[1024] = { 0 };
	unsigned char config_path[200] = { 0 };
	struct http_parm parm;
	memset(&parm, 0, sizeof(struct http_parm));
	signal(SIGPIPE, recvSignal);
	signal(SIGSEGV, SIG_IGN);
	printf("BOOT 程序启动：版本:1.0.04  发布时间：2021-06-18\n");
	//先正常检测启动系统中的程序

	if (access(TAR_PATH, 0) != 0)
	{
		mb_create_file_dir(TAR_PATH);
	}
	if (access(UPDATE_FILE_RECORD_PATH_APP, 0) == 0)
	{
		mb_delete_file_dir(UPDATE_FILE_RECORD_PATH_APP, 0);
	}
	if (access(UPDATE_FILE_RECORD_PATH_AUTH, 0) == 0)
	{
		mb_delete_file_dir(UPDATE_FILE_RECORD_PATH_AUTH, 0);
	}
	while (!access(TAR_PATH, 0))
	{
		
		memset(auth_msg, 0, sizeof(auth_msg));
		memset(update_msg, 0, sizeof(update_msg));
		if ( !access(TAR_FILE_PATH_APP, 0))
		{
			if (!access(TAR_FILE_APP_DECTECT_PATH, 0))
				mb_delete_file_dir(TAR_FILE_APP_DECTECT_PATH, 1);
			mb_create_file_dir(TAR_FILE_APP_DECTECT_PATH);
			

			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "tar -xf %s -C %s/ ", TAR_FILE_PATH_APP, TAR_FILE_APP_DECTECT_PATH);
			mb_system_read(cmd, buf_read, sizeof(buf_read));
			
			if (strstr(buf_read, "tar:") != NULL)//压缩包有问题  不能正常解压，正常解压无输出,直接以系统环境中的程序启动
			{
				printf("app压缩包有问题,不能正常解压,正常解压无输出,直接以系统环境中的程序启动\n");
				break;
			}
			//进行与系统环境中对应文件进行比较，MD5不同则替换
			sprintf((char *)config_path, "%s/mb_config.cnf", TAR_FILE_APP_DECTECT_PATH);
			result = parse_config_copy_file(config_path, (unsigned char *)TAR_FILE_APP_DECTECT_PATH,CHECK);
			if (result < 0)//解析比较tar包与环境中的文件出现错误异常--未替换（以原有的程序执行），上报一下信息?
			{
				printf("app解析比较tar包与环境中的文件出现错误异常!\n");
				break;
			}
		}
		if (!access(TAR_FILE_PATH_AUTH, 0))
		{
			if (!access(TAR_FILE_AUTH_DECTECT_PATH, 0))
				mb_delete_file_dir(TAR_FILE_AUTH_DECTECT_PATH, 1);
			mb_create_file_dir(TAR_FILE_AUTH_DECTECT_PATH);
			memset(cmd, 0, sizeof(cmd));
			memset(buf_read, 0, sizeof(buf_read));
			sprintf(cmd, "tar -xf %s -C %s/ ", TAR_FILE_PATH_AUTH, TAR_FILE_AUTH_DECTECT_PATH);
			mb_system_read(cmd, buf_read, sizeof(buf_read));
			printf("get cmd return !:\n%s\n", buf_read);

			if (strstr(buf_read, "tar:") != NULL)//压缩包有问题  不能正常解压，正常解压无输出,直接以系统环境中的程序启动
			{
				printf("auth压缩包有问题,不能正常解压,正常解压无输出,直接以系统环境中的程序启动\n");
				break;
			}
			//进行与系统环境中对应文件进行比较，MD5不同则替换
			sprintf((char *)config_path, "%s/mb_config.cnf", TAR_FILE_AUTH_DECTECT_PATH);
			result = parse_config_copy_file(config_path, (unsigned char*)TAR_FILE_AUTH_DECTECT_PATH, CHECK);
			if (result < 0)//解析比较tar包与环境中的文件出现错误异常--未替换（以原有的程序执行），上报一下信息?
			{
				printf("auth解析比较tar包与环境中的文件出现错误异常!\n");
				break;
			}
		}
		break;

	}
	//printf("创建线程\n");

	result = pthread_create(&thread_main_app, NULL, main_app_thread, &parm);
	if (result != 0)
	{
		printf("create thread thread_main_app err!\n");

		return -1;
	}

	result = pthread_create(&thread_update, NULL, update_thread, &parm);
	if (result != 0)
	{
		printf("create thread thread_update err!\n");
		
		return -1;
	}

	//printf("线程剥离\n");
	for (;;)
		sleep(1);
	return 0;
}

void *main_app_thread(void *arg)
{
	int result;
	//printf("检测到主程序开始运行\n");
	result = detect_process(BOOT_APP_PATH_RUN, "tmpmain.txt");
	if (result > 0)
	{
		printf("%s进程存在,需结束\n", BOOT_APP_PATH_RUN);
		check_and_close_process(BOOT_APP_PATH_RUN);
	}
	while (1)
	{
		result = detect_process(BOOT_APP_PATH_RUN, "tmpmain.txt");
		if (result > 0)
		{
			sleep(10);
			continue;
		}
		printf("运行主程序\n");
		result = run_main_app();
		if (result < 0)
		{
			printf("主程序运行失败\n");
		}
		sleep(10);
	}
}

void *update_thread(void *arg)
{
	struct http_parm *parm;
	char errinfo[300] = { 0 };
	int result = 0;
	int sock = 0;
	int flag_app = 0;//本次更新标识位 0 表示APP 和auth都无更新，1表示只有APP，2表示只有AUTH，3表示都有
	struct timeval tv;
	char cmd[100] = { 0 };
	char ip[30] = { 0 };
	char hostname[100] = { 0 };
	char real_host[100] = { 0 };
	parm = (struct http_parm *)arg;
	
	sleep(30);


	while (1)
	{
		
		memset(parm, 0, sizeof(struct http_parm));
		memset(real_host, 0, sizeof(real_host));
		memset(hostname, 0, sizeof(hostname));
		flag_app = 0;
		memset(parm->ter_id, 0, sizeof(parm->ter_id));

		result = get_ter_id_ver((char *)TER_ID_INFO_PATH, (char *)parm->ter_id, (char *)parm->app_ver, hostname,&parm->port);
		if (result < 0)
		{
			printf("获取terinfo内容错误!\n");
			sleep(60);
			continue;
		}
		memset(parm->ip, 0, sizeof(parm->ip));
		if (get_real_host(hostname, real_host, parm->ip,parm->port) < 0)
		{
			printf("主域名，备份域名均无效!\n");
			sleep(60);
			continue;
		}
		////printf("获取到的真实HOST为:%s\n", real_host);
		//if (GetDnsIp("www.baidu.com", testip) < 0 || GetDnsIp(real_host, ip) < 0)//解析百度域名，成功则代表网络通畅
		//{
		//	printf("检测下载地址域名解析失败\n");
		//	if (dns_err_count < 10)
		//	{
		//		sleep(2 * 60);
		//		dns_err_count++;
		//		continue;
		//	}
		//	else
		//	{
		//		dns_err_count = 0;
		//		goto WAIT_NEXT;
		//	}	
		//	
		//}
		//memset(parm->ip, 0, sizeof(parm->ip));
		//memcpy(parm->ip, ip, strlen(ip));

		//http://www.njmbxx.com:20020
		result = http_socket_open(&sock, parm->ip, parm->port);
		if (result <0)
		{
			printf("socket create failed\n");//socket 创建失败直接返回
			goto WAIT_NEXT;
		}
		memcpy(parm->appid, parm->ter_id, strlen((char *)parm->ter_id));
		//printf("机器编号获取成功:%s\n", parm->appid);			//wang
		result = http_get_token(sock, parm);
		if (result < 0)
		{
			printf("http get token err!\n");
			shutdown(sock, SHUT_RDWR);
			close(sock);
			goto WAIT_NEXT;
		}
		//printf("The token is :%s\n", parm->token);				//wang
		memset(parm->tar_md5, 0, sizeof(parm->tar_md5));
		result = get_local_tar_mad5(TAR_FILE_PATH_APP, (char*)parm->tar_md5);
		if (result < 0)
		{
			memcpy(parm->tar_md5, " ", 1);
		}

		gettimeofday(&tv, NULL);
		memset(parm->timestamp, 0, sizeof(parm->timestamp));
		sprintf((char*)parm->timestamp, "%ld", tv.tv_sec);
RE_GETAPP:
		result = http_get_ter_update(sock, parm);
		if (result < 0)
		{
			if (result == HTTP_TOKEN_ERR)//重新获取token
			{
				result = http_get_token(sock, parm);
				if (result < 0)
				{
					printf("http get token err1!\n");
					shutdown(sock, SHUT_RDWR);
					close(sock);
					goto WAIT_NEXT;
				}
				goto RE_GETAPP;
				
			}
			printf("http get ter update err!\n");
			parse_errcode_upload(sock, parm, result);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			goto WAIT_NEXT;
		}
		if (result == 1)
		{
			//本次没有需要更新的app内容
			flag_app = NO_UPDATE;
		}
		else 
			flag_app = ONLY_APP;


		//更新下载授权文件
		memset(parm->tar_md5, 0, sizeof(parm->tar_md5));
		result = get_local_tar_mad5(TAR_FILE_PATH_AUTH, (char*)parm->tar_md5);
		if (result < 0)
		{
			memcpy(parm->tar_md5, " ", 1);
		}
RE_GETAUTH:
		result = http_get_ter_auth(sock, parm);
		//printf("http_get_ter_auth result=%d\r\n",result);			//wang
		if (result < 0)
		{
			printf("http get ter auth err!\n");
			if (result == HTTP_TOKEN_ERR)//重新获取token
			{
				result = http_get_token(sock, parm);
				if (result < 0)
				{
					printf("http get token err2!\n");
					shutdown(sock, SHUT_RDWR);
					close(sock);
					goto WAIT_NEXT;
				}
				goto RE_GETAUTH;

			}
			parse_errcode_upload(sock, parm, result);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			goto WAIT_NEXT;
		}
		if (result == 2)//表示没有要更新的东西
		{
			if (flag_app == NO_UPDATE)//app也无更新内容
			{
				shutdown(sock, SHUT_RDWR);
				close(sock);
				goto WAIT_NEXT;
			}
		}
		else
			flag_app += ONLY_AUTH;
		result = dectec_app(errinfo);
		//printf("dectec_app result=%d\r\n",result);		//wang
		if (result < 0)//检测失败，进行本次更新回退
		{
			printf("err app_file_detect!\n");
			parse_config_back_oldver("/home/share/exe/tar/tmpapp/mb_config.cnf", "/home/share/exe/tar/tmpapp");
			parse_config_back_oldver("/home/share/exe/tar/tmpauth/mb_config.cnf", "/home/share/exe/tar/tmpauth");

			parse_errcode_upload(sock, parm, BOOT_ERROR_OPER_DETECT_FILE_CHECK_ERR);
			shutdown(sock, SHUT_RDWR);
			close(sock);
			goto WAIT_NEXT;
		}
		else//删除.back文件
		{
			parse_config_del_backfile("/home/share/exe/tar/tmpapp/mb_config.cnf", "/home/share/exe/tar/tmpapp");
			parse_config_del_backfile("/home/share/exe/tar/tmpauth/mb_config.cnf", "/home/share/exe/tar/tmpauth");
		}
		printf("int end!!!\n");
		memset(cmd, 0, sizeof(cmd));
		//没有问题 从tmpapp复制该tar包到exe/tar目录下用于开机校验用
		printf("mb_copy_file_path start\r\n");			//wang
		if (flag_app == ONLY_APP || flag_app == BOTH_APP_AUTH)
		{
			if (access("/home/share/exe/tar/tmpapp/app.tar", 0) == 0)
				mb_copy_file_path("/home/share/exe/tar/tmpapp/app.tar", TAR_PATH);
		}
		printf("mb_copy_file_path over\r\n");			//wang
		//没有问题 从tmpauth目录复制该tar包到exe/tar目录下用于开机校验用
		if (flag_app == ONLY_AUTH || flag_app == BOTH_APP_AUTH)
		{
			if (access("/home/share/exe/tar/tmpauth/auth.tar", 0) == 0)
				mb_copy_file_path("/home/share/exe/tar/tmpauth/auth.tar", TAR_PATH);
		}
		printf("mb_copy_file_path1 over\r\n");			//wang
		//此处进行有更新且已经正常安装的重启主程序处理:写一个更新文件给主程序响应	
		if (strlen((char *)update_msg) != 0)
		{
			printf("update_msg:%s\n", update_msg);
			result = write_update_record(UPDATE_FILE_RECORD_PATH_APP, (char*)update_msg);
			if (result < 0)
			{
				printf("err write_update_record:%d\n", result);
			}
		}
		if (strlen((char *)auth_msg) != 0)
		{
			result = write_update_record(UPDATE_FILE_RECORD_PATH_AUTH, (char*)auth_msg);
			if (result < 0)
			{
				printf("err write_update_record:%d\n", result);
			}
		}
WAIT_NEXT:
		sleep_hour(1);//此处等待1小时
	}
	return NULL;
}

int dectec_app(char *errinfo)
{
	int i;
	//int s_ver = 0;
	int result;
	int flag_found_app = 0;
	struct _app_file_note   file_note;
	mb_chmod_file_path("777", BOOT_APP_PATH);
	for (i = 0; i < sizeof(soft_d) / sizeof(const char *); i++)
	{
		result = app_file_get_new(soft_d[i], &file_note);
		if (result < 0)
			continue;
		else
		{
			flag_found_app = 1;
			break;
		}

	}

	if (flag_found_app == 1)
	{
		mb_chmod_file_path("777", BOOT_APP_PATH);
		result = app_file_detect(soft_d[i], errinfo);
		if (result < 0)
			return result;
		return 0;
	}
	return -1;
}

int run_main_app()
{
	int i;
	//int s_ver = 0;
	int result;
	int flag_found_app = 0;
	struct _app_file_note   file_note;
	mb_chmod_file_path("777", BOOT_APP_PATH);
	for (i = 0; i < sizeof(soft_d) / sizeof(const char *); i++)
	{
		result = app_file_get_new(soft_d[i], &file_note);
		if (result < 0)
			continue;
		else
		{
			flag_found_app = 1;
			break;
		}
		
	}
	
	if (flag_found_app == 1)
	{
		//out("检测%s进程是否存在\n",DF_PROCESS_MANAGE_CA_SERVER);
		mb_chmod_file_path("777", BOOT_APP_PATH);
		result = app_file_oper(soft_d[i]);
		if (result < 0)//程序未正常运行，标识位置为0
		{
			return -1;
		}
		return 0;
	}
	
	return -1;
}

int parse_errcode_upload(int socketfd, struct http_parm *parm, int errcode)
{
	int i;
	for (i = 0; i < sizeof(err_info) / sizeof(struct err_n); i++)
	{
		if (err_info[i].err_no == errcode)
		{
			memset(parm->errmsg, 0, sizeof(parm->errmsg));
			memcpy(parm->errmsg, err_info[i].err_info, strlen((char *)err_info[i].err_info));
			http_upload_err(socketfd, parm);
			return 0;
		}
	}
	memset(parm->errmsg, 0, sizeof(parm->errmsg));
	sprintf((char *)parm->errmsg, "unkown err happened!");
	http_upload_err(socketfd, parm);
	return 0;
}


int get_real_host(char *config_host, char *real_host,char *outip,int port)
{
	char ip[100] = { 0 };
	int result = 0;
	int sock;
	if (GetDnsIp(config_host, ip) >= 0)
	{
		
		//return 0;
		result = http_socket_open(&sock, ip, port);
		if (result <0)
		{
			printf("socket create failed1\n");//socket 创建失败直接返回
		}
		result = http_api_test(sock, ip,port);
		if (result < 0)
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
		}
		else//主域名可用直接返回
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			memcpy(real_host, config_host, strlen(config_host));
			memcpy(outip, ip, strlen(ip));
			return 0;
		}
		
	}
	memset(ip, 0, sizeof(ip));
	if (GetDnsIp(HTTP_ADDR, ip) >= 0)
	{
		
		result = http_socket_open(&sock, ip,port);
		if (result <0)
		{
			printf("socket create failed2\n");//socket 创建失败直接返回
		}
		result = http_api_test(sock, ip,port);
		if (result < 0)
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
		}
		else//代码写死的备份域名可用
		{
			shutdown(sock, SHUT_RDWR);
			close(sock);
			//memset(real_host, 0, sizeof(real_host));
			memcpy(real_host, HTTP_ADDR, strlen(HTTP_ADDR));
			memcpy(outip, ip, strlen(ip));
			return 0;
		}
	}
	return -1;
}