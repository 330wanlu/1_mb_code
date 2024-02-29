#define _process_manage_c
#include "_process_manage.h"

int main_start_init()
{
	//延时3秒后启动,目的排查mount目录丢失、程序未进入正常工作状态、网卡不正常启动、mysql启动失败等问题;
	//sleep(3);
	SetSystemTime((uint8 *)"2000-01-01 00:00:00");
#ifndef MTK_OPENWRT
	int result = 0;
	//out("检测%s进程是否存在\n",DF_PROCESS_MANAGE_CA_SERVER);
	result = detect_process(DF_PROCESS_MANAGE_CA_SERVER,"tmp1.txt");
	if(result > 0 )
	{	out("%s进程存在,需结束\n",DF_PROCESS_MANAGE_CA_SERVER);
		check_and_close_process(DF_PROCESS_MANAGE_CA_SERVER);
	}
	//out("检测%s进程是否存在\n", DF_PROCESS_MANAGE_CA_PRO_N);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_N, "tmp2.txt");
	if(result > 0 )
	{
		out("%s进程存在,需结束\n", DF_PROCESS_MANAGE_CA_PRO_N);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_N);
	}
	//out("检测%s进程是否存在\n", DF_PROCESS_MANAGE_CA_PRO_S);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_S, "tmp3.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", DF_PROCESS_MANAGE_CA_PRO_S);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_S);
	}
	//out("检测%s进程是否存在\n", DF_PROCESS_MANAGE_CA_PRO_U);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_U, "tmp4.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", DF_PROCESS_MANAGE_CA_PRO_U);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_U);
	}
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_NEW, "tmp5.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", DF_PROCESS_MANAGE_CA_PRO_NEW);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_NEW);
	}

	//out("检测%s进程是否存在\n",DF_PROCESS_MANAGE_OPENVPN);
	result = detect_process(DF_PROCESS_MANAGE_OPENVPN,"tmp6.txt");
	if(result > 0 )
	{	out("%s进程存在,需结束\n",DF_PROCESS_MANAGE_OPENVPN);
		check_and_close_process(DF_PROCESS_MANAGE_OPENVPN);
	}

	//out("检测%s进程是否存在\n", DF_PROCESS_MANAGE_UDHCPC);
	result = detect_process(DF_PROCESS_MANAGE_UDHCPC, "tmp7.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", DF_PROCESS_MANAGE_UDHCPC);
		check_and_close_process(DF_PROCESS_MANAGE_UDHCPC);
		system("killall udhcpc");
	}

	//out("检测%s进程是否存在\n", DF_PROCESS_AUTO_NTPDATE);
	result = detect_process(DF_PROCESS_AUTO_NTPDATE, "tmp8.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", DF_PROCESS_AUTO_NTPDATE);
		check_and_close_process(DF_PROCESS_AUTO_NTPDATE);
	}

	//out("检测%s进程是否存在\n",  "sleep");
	result = detect_process("sleep", "tmp9.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", "sleep");
		check_and_close_process("sleep");
	}

	result = detect_process("wpa_supplicant", "tmp10.txt");
	if (result > 0)
	{
		out("%s进程存在,需结束\n", "wpa_supplicant");
		check_and_close_process("wpa_supplicant");
	}

	system("/etc/init.d/mysqld2.0 stop");
#ifdef RELEASE_SO
	if (detect_process("vsftpd", "tmp11.txt") > 0)
	{
		out("%s进程存在,需结束\n", "vsftpd");
		system("killall vsftpd");
	}
#endif
#endif

	return 0;
}

int detect_process_path(char * process_name,char *path)
{
	//int   stream;
	//char buff[512];  
	char ps[128];
	char order[200];
	int num;
	sprintf(ps, "ps | grep %s | grep -v grep", process_name);
	//out("cmd %s\n",ps);
	unsigned char lineBuff[256];// 存储读取一行的数据  

	/*memset(order, 0, sizeof(order));
	sprintf(order, "rm -rf %s", path);
	system(order);
	sync();*/
	memset(order, 0, sizeof(order));
	sprintf(order, "%s > %s", ps, path);
	//out("order=[%s]\n",order);
	system(order);

	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		out("文件打开失败,文件名：%s\n", path);
		sprintf(order, "rm %s",path);
		system(order);
		return 1;
	}
	fseek(fp, 0, SEEK_SET);
	num = 0;
	while (fgets((char *)lineBuff, 256, fp) != NULL)
	{

		if (strlen((const char*)lineBuff) < 4) // 空行  
		{
			continue;
		}
		//out("%s\n", lineBuff);
		num += 1;
	}
	fclose(fp);
	if (num >= 1)
	{	//out("检测到进程(%s)存在，无需再次启动,%s\n",process_name,buff);
		sprintf(order, "rm %s", path);
		system(order);
		return num;
	}
	//logout(INFO, "system", "进程管理", "检测到进程(%s)不存在,需再次启动,num = %d\r\n",process_name,atoi(buff));
	//out("检测到进程未启动\n");
	sprintf(order, "rm %s", path);
	system(order);
	return -1;
}

int detect_process(char * process_name, char *tmp)
{    
	//int   stream;
	//char buff[512];  
	char ps[128];  
	char order[200];
	char tmp_file[100];
	int num;
	sprintf(ps,"ps | grep \'%s\' | grep -v grep",process_name);  
	//out("cmd %s\n",ps);
	unsigned char lineBuff[256];// 存储读取一行的数据  

	memset(order,0,sizeof(order));
	memset(tmp_file,0,sizeof(tmp_file));
	sprintf(tmp_file,"/tmp/%s",tmp);
	sprintf(order,"%s > %s",ps,tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	FILE* fp = fopen(tmp_file, "r");
	if (fp == NULL)
	{
		out("文件打开失败,文件名：%s\n", tmp_file);
		sprintf(order, "rm %s", tmp_file);
		system(order);

		return 1;
	}
	fseek(fp, 0, SEEK_SET);
	num = 0;
	while (fgets((char *)lineBuff, 256, fp) != NULL)
	{
		
		if (strlen((const char*)lineBuff) < 4) // 空行  
		{
			continue;
		}
		//out("%s\n", lineBuff);
		num += 1;
	}
	fclose(fp);
	if(num>=1)  
	{	//out("检测到进程(%s)存在，无需再次启动,%s\n",process_name,buff);
		sprintf(order, "rm %s", tmp_file);
		system(order);
		return num;  
	}
	//out("检测到进程(%s)不存在,需再次启动,num = %d\n",process_name,num);
	//out("检测到进程未启动\n");
	sprintf(order, "rm %s", tmp_file);
	system(order);
	return -1;
} 

int check_and_close_process(char * process_name)
{
	//int   stream;
	//int configLen;
	//char buff[512];  
	char ps[128];  
	char order[200];
	char tmp_file[100];
	int pidnum;
	int result;
	int i=0;
	char lineBuff[256];// 存储读取一行的数据  
	pid_t pID;
	sprintf(ps,"ps | grep \'%s\' | grep -v grep",process_name);  
	//out("cmd %s\n",ps);


	memset(order,0,sizeof(order));
	memset(tmp_file,0,sizeof(tmp_file));
	sprintf(tmp_file,"/tmp/closetmp.txt");
	sprintf(order,"%s > %s",ps,tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	//stream=open(tmp_file,O_RDWR);;
	//memset(buff,0,sizeof(buff));
	//read(stream,buff,sizeof(buff));
	//close(stream);
	//out("buf = %s\n\n\n",buff);
	FILE* fp  = fopen(tmp_file,"r");  
	if(fp == NULL)  
	{	out("文件打开失败\n");
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", tmp_file);
		system(order);
		sync();
		return -1;
	}  
	fseek(fp,0,SEEK_END);  
	//configLen = ftell(fp); //配置文件长度  
	fseek(fp,0,SEEK_SET);  

	//out("配置文件总大小为：%d字节\n",configLen);
	memset(lineBuff,0,sizeof(lineBuff));
	while(fgets(lineBuff,256,fp) != NULL)  
	{	if(strlen(lineBuff) < 1) // 空行  
	{	continue;  
	}
	i +=1;
	out("读到第%d行的数据为：%s\n",i,lineBuff);
	pidnum = atoi(lineBuff);
	out("[%s] process PID num = %d\n",process_name,pidnum);
	pID=(pid_t)pidnum;
	result=kill(pID,SIGKILL);//这里不是要杀死进程，而是验证一下进程是否真的存在，返回0表示真的存在
	printf("process: %s exist!kill pid over,result = %d\n",process_name,result);
	}
	fclose(fp);  
	memset(order, 0, sizeof(order));
	sprintf(order, "rm %s", tmp_file);
	system(order);
	sync();
	return -1;
}