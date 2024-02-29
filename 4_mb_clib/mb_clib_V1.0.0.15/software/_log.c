#define _log_c
#include "_log.h"



void log_out(char *name, const char* format, ...)
{
	va_list args;
	char buf[204800];
	char buf1[204800];
	//char time_buf[20];
	uint8 timer[8];
	uint8 timer_asc[50];
	int i;
	//char *pbuf;
	if (log.dir == 0xff)
		return;
	if (name != NULL)
	{
		for (i = 0; i<sizeof(log.file) / sizeof(struct name_pro); i++)
		{
			if (strlen(log.file[i].name) == 0)
				continue;
			if (strlen(log.file[i].name) != strlen(name))
				continue;
			if (memcmp(name, log.file[i].name, strlen(name)) != 0)
				continue;
			break;
		}
		if (i != sizeof(log.file) / sizeof(struct name_pro))
		{
			if (log.file[i].dir == DF_LOG_DIR_D)			//ÊÇ·ñ½ûÖ¹Êä³ö
				return;
		}
	}
	va_start(args, format);
	memset(buf1, 0, sizeof(buf1));
	vsprintf(buf1, format, args);
	va_end(args);
	memset(buf, 0, sizeof(buf));
	clock_get(timer);
	memset(timer_asc, 0, sizeof(timer_asc));
	time_asc(timer, timer_asc, 7);
	//print_time((uint8 *)time_buf);
	char time_m[100];
	memset(time_m, 0, sizeof(time_m));
	timer_read_asc_ms(time_m);
	//printf("%s\n", time_m);
	//sprintf(buf,"%s:---%s",timer_asc,buf1);
	sprintf(buf, "%s", buf1);
	if (log.dir == 0x00)
	{
		memset(buf1, 0, sizeof(buf1));
		gbk2utf8((uint8 *)buf, (uint8 *)buf1);
		printf("%s %s", time_m, buf1);

	}
}

int write_log(const char *format, ...) 
{	char buf[204800];
	char buf1[204800];
	//int len;
    va_list arg;
    //int done0;
	FILE* pFile = fopen(DF_TERMINAL_LOG_FILE, "a");
	if (pFile == NULL)
		return 0;
    va_start (arg, format);
    //done = vfprintf (stdout, format, arg);
	memset(buf1,0,sizeof(buf1));
	vsprintf(buf1,format,arg);

	char time_asc[200] = {0};
	timer_read_asc_ms(time_asc);
 /*   time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);*/
	fprintf(pFile, "%s ", time_asc);
	memset(buf,0,sizeof(buf));
	gbk2utf8((uint8 *)buf1, (uint8 *)buf);
	fprintf(pFile, "%s",buf);
	printf("%s", time_asc);
	printf("%s", buf);
    //done = vfprintf (pFile, format, arg);
    va_end (arg);

    fflush(pFile);
	fclose(pFile);
    return 0;
}



void printf_array(char *data, int len)
{
	int i = 0;
//#ifdef RELEASE
//	return ;
//#endif
	for (i = 0; i < len; i++)
		printf("%02x ", (unsigned char)data[i]);
	printf("\n");
}


int logout(uint8 bLoglevl, char *appcode, char *event, const char *format, ...)
{

	char *gbk_buf = NULL;
	char *utf8_buf = NULL;
	char *write_buf = NULL;
	char time_usec_s[50] = { 0 };
	char time_sec_s[50] = { 0 };
	char head_gbk[1024] = { 0 };
	char head_utf8[4096] = { 0 };
	struct timeval tv;
	long time_sec;
	char level[1024] = { 0 };

	char file_name[200] = {0};
	get_radom_serial_number("/tmp/log", file_name);

	FILE* tFile = fopen(file_name, "a");
	va_list arg;
	int str_len;
	int need_write = 1;
	va_start(arg, format);

	if (tFile == NULL)
		return 0;
	str_len = vfprintf(tFile, format, arg);
	gbk_buf = calloc(1, str_len * 2 + 1024);
	utf8_buf = calloc(1, str_len * 3 + 2048);
	write_buf = calloc(1, str_len * 3 + 4096);
	vsprintf(gbk_buf, format, arg);

	gbk2utf8((uint8 *)gbk_buf, (uint8 *)utf8_buf);



	switch (bLoglevl)
	{
	case OFF:
		sprintf(level, "off");
		need_write = 0;
		break;
	case DEBUG:
		sprintf(level, "debug");
		break;
	case INFO:
		sprintf(level, "info");
		break;
	case WARN:
		sprintf(level, "warn");
		break;
	case ERROR:
		sprintf(level, "error");
		break;
	case FATAL:
		sprintf(level, "fatal");
		break;
	default:
		sprintf(level, "off");
		need_write = 0;
		break;
	}
	time_sec = get_time_sec();
	get_date_time_from_second_turn(time_sec, time_sec_s);
	gettimeofday(&tv, 0);
	sprintf(time_usec_s, "%s.%03d", time_sec_s, (int)(tv.tv_usec) / 1000);


	sprintf(head_gbk, "¡¾%s|%s|%s|%s¡¿", time_usec_s, level, appcode, event);
	gbk2utf8((uint8 *)head_gbk, (uint8 *)head_utf8);

	char end[50] = { 0 };
	if (utf8_buf[strlen(utf8_buf) - 1] != '\n')
		strcpy(end, "\n");
	sprintf(write_buf, "%s:%s%s", head_utf8, utf8_buf, end);
	printf("%s", write_buf);

	if (need_write == 1)
	{
		FILE* pFile = fopen(DF_TERMINAL_LOG_FILE, "a");
		if (pFile == NULL)
		{
			printf("Open Log file failed,sleep 1 second and try again\n");
			sleep(1);
			FILE* pFile = fopen(DF_TERMINAL_LOG_FILE, "a");
			if (pFile == NULL)
			{
				printf("Open Log file failed,try again failed\n");
				return 0;
			}
			else
			{
				fprintf(pFile, "%s", write_buf);
				fflush(pFile);
				fclose(pFile);
			}
		}
		else
		{
			fprintf(pFile, "%s", write_buf);
			fflush(pFile);
			fclose(pFile);
		}
	}

	va_end(arg);
	free(gbk_buf);
	free(write_buf);
	free(utf8_buf);
	fflush(tFile);
	fclose(tFile);
	if (access(file_name, 0) == 0)
	{	
		//printf("DELETE FILE %s\n", file_name);
		char cmd_str[200] = {0};
		sprintf(cmd_str, "rm %s", file_name);
		system(cmd_str);
	}
	return str_len;
}