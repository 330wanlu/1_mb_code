#define _process_manage_c
#include "_process_manage.h"

int main_start_init()
{
	//��ʱ3�������,Ŀ���Ų�mountĿ¼��ʧ������δ������������״̬������������������mysql����ʧ�ܵ�����;
	//sleep(3);
	SetSystemTime((uint8 *)"2000-01-01 00:00:00");
#ifndef MTK_OPENWRT
	int result = 0;
	//out("���%s�����Ƿ����\n",DF_PROCESS_MANAGE_CA_SERVER);
	result = detect_process(DF_PROCESS_MANAGE_CA_SERVER,"tmp1.txt");
	if(result > 0 )
	{	out("%s���̴���,�����\n",DF_PROCESS_MANAGE_CA_SERVER);
		check_and_close_process(DF_PROCESS_MANAGE_CA_SERVER);
	}
	//out("���%s�����Ƿ����\n", DF_PROCESS_MANAGE_CA_PRO_N);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_N, "tmp2.txt");
	if(result > 0 )
	{
		out("%s���̴���,�����\n", DF_PROCESS_MANAGE_CA_PRO_N);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_N);
	}
	//out("���%s�����Ƿ����\n", DF_PROCESS_MANAGE_CA_PRO_S);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_S, "tmp3.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", DF_PROCESS_MANAGE_CA_PRO_S);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_S);
	}
	//out("���%s�����Ƿ����\n", DF_PROCESS_MANAGE_CA_PRO_U);
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_U, "tmp4.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", DF_PROCESS_MANAGE_CA_PRO_U);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_U);
	}
	result = detect_process(DF_PROCESS_MANAGE_CA_PRO_NEW, "tmp5.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", DF_PROCESS_MANAGE_CA_PRO_NEW);
		check_and_close_process(DF_PROCESS_MANAGE_CA_PRO_NEW);
	}

	//out("���%s�����Ƿ����\n",DF_PROCESS_MANAGE_OPENVPN);
	result = detect_process(DF_PROCESS_MANAGE_OPENVPN,"tmp6.txt");
	if(result > 0 )
	{	out("%s���̴���,�����\n",DF_PROCESS_MANAGE_OPENVPN);
		check_and_close_process(DF_PROCESS_MANAGE_OPENVPN);
	}

	//out("���%s�����Ƿ����\n", DF_PROCESS_MANAGE_UDHCPC);
	result = detect_process(DF_PROCESS_MANAGE_UDHCPC, "tmp7.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", DF_PROCESS_MANAGE_UDHCPC);
		check_and_close_process(DF_PROCESS_MANAGE_UDHCPC);
		system("killall udhcpc");
	}

	//out("���%s�����Ƿ����\n", DF_PROCESS_AUTO_NTPDATE);
	result = detect_process(DF_PROCESS_AUTO_NTPDATE, "tmp8.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", DF_PROCESS_AUTO_NTPDATE);
		check_and_close_process(DF_PROCESS_AUTO_NTPDATE);
	}

	//out("���%s�����Ƿ����\n",  "sleep");
	result = detect_process("sleep", "tmp9.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", "sleep");
		check_and_close_process("sleep");
	}

	result = detect_process("wpa_supplicant", "tmp10.txt");
	if (result > 0)
	{
		out("%s���̴���,�����\n", "wpa_supplicant");
		check_and_close_process("wpa_supplicant");
	}

	system("/etc/init.d/mysqld2.0 stop");
#ifdef RELEASE_SO
	if (detect_process("vsftpd", "tmp11.txt") > 0)
	{
		out("%s���̴���,�����\n", "vsftpd");
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
	unsigned char lineBuff[256];// �洢��ȡһ�е�����  

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
		out("�ļ���ʧ��,�ļ�����%s\n", path);
		sprintf(order, "rm %s",path);
		system(order);
		return 1;
	}
	fseek(fp, 0, SEEK_SET);
	num = 0;
	while (fgets((char *)lineBuff, 256, fp) != NULL)
	{

		if (strlen((const char*)lineBuff) < 4) // ����  
		{
			continue;
		}
		//out("%s\n", lineBuff);
		num += 1;
	}
	fclose(fp);
	if (num >= 1)
	{	//out("��⵽����(%s)���ڣ������ٴ�����,%s\n",process_name,buff);
		sprintf(order, "rm %s", path);
		system(order);
		return num;
	}
	//logout(INFO, "system", "���̹���", "��⵽����(%s)������,���ٴ�����,num = %d\r\n",process_name,atoi(buff));
	//out("��⵽����δ����\n");
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
	unsigned char lineBuff[256];// �洢��ȡһ�е�����  

	memset(order,0,sizeof(order));
	memset(tmp_file,0,sizeof(tmp_file));
	sprintf(tmp_file,"/tmp/%s",tmp);
	sprintf(order,"%s > %s",ps,tmp_file);
	//out("order=[%s]\n",order);
	system(order);

	FILE* fp = fopen(tmp_file, "r");
	if (fp == NULL)
	{
		out("�ļ���ʧ��,�ļ�����%s\n", tmp_file);
		sprintf(order, "rm %s", tmp_file);
		system(order);

		return 1;
	}
	fseek(fp, 0, SEEK_SET);
	num = 0;
	while (fgets((char *)lineBuff, 256, fp) != NULL)
	{
		
		if (strlen((const char*)lineBuff) < 4) // ����  
		{
			continue;
		}
		//out("%s\n", lineBuff);
		num += 1;
	}
	fclose(fp);
	if(num>=1)  
	{	//out("��⵽����(%s)���ڣ������ٴ�����,%s\n",process_name,buff);
		sprintf(order, "rm %s", tmp_file);
		system(order);
		return num;  
	}
	//out("��⵽����(%s)������,���ٴ�����,num = %d\n",process_name,num);
	//out("��⵽����δ����\n");
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
	char lineBuff[256];// �洢��ȡһ�е�����  
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
	{	out("�ļ���ʧ��\n");
		memset(order, 0, sizeof(order));
		sprintf(order, "rm %s", tmp_file);
		system(order);
		sync();
		return -1;
	}  
	fseek(fp,0,SEEK_END);  
	//configLen = ftell(fp); //�����ļ�����  
	fseek(fp,0,SEEK_SET);  

	//out("�����ļ��ܴ�СΪ��%d�ֽ�\n",configLen);
	memset(lineBuff,0,sizeof(lineBuff));
	while(fgets(lineBuff,256,fp) != NULL)  
	{	if(strlen(lineBuff) < 1) // ����  
	{	continue;  
	}
	i +=1;
	out("������%d�е�����Ϊ��%s\n",i,lineBuff);
	pidnum = atoi(lineBuff);
	out("[%s] process PID num = %d\n",process_name,pidnum);
	pID=(pid_t)pidnum;
	result=kill(pID,SIGKILL);//���ﲻ��Ҫɱ�����̣�������֤һ�½����Ƿ���Ĵ��ڣ�����0��ʾ��Ĵ���
	printf("process: %s exist!kill pid over,result = %d\n",process_name,result);
	}
	fclose(fp);  
	memset(order, 0, sizeof(order));
	sprintf(order, "rm %s", tmp_file);
	system(order);
	sync();
	return -1;
}