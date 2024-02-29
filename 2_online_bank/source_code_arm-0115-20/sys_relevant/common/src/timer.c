#include "../inc/timer.h"

static int tim_subtract(struct timeval *result,struct timeval *x,struct timeval *y);

static uint8 hex2bcd(uint8 hex)
{
	if(hex>99)
		return 0;
	return hex%10+((hex/10)<<4);
}

int clock_get(uint8 *buf)
{
	struct tm s;
	long time_sec = get_time_sec();
	get_struct_from_second_turn(time_sec, &s);
	buf[0] = hex2bcd(((s.tm_year)/100));
	buf[1] = hex2bcd(((s.tm_year)%100));	//年
	buf[2] = hex2bcd(s.tm_mon);			//月
	buf[3] = hex2bcd(s.tm_mday);			//日
	buf[4] = hex2bcd(s.tm_hour);			//小时			
	buf[5] = hex2bcd(s.tm_min);			//分
	buf[6] = hex2bcd(s.tm_sec);			//秒
	return 0;
}

//系统时间设置的是UTC时间非北京时间
int SetSystemTime(uint8 *dt)
{
    struct rtc_time tm;
    struct tm _tm;
    struct timeval tv;
    time_t timep;
    sscanf((const char *)dt,"%d-%d-%d %d:%d:%d",&tm.tm_year,&tm.tm_mon,&tm.tm_mday,&tm.tm_hour,&tm.tm_min,&tm.tm_sec);
    _tm.tm_sec = tm.tm_sec;
    _tm.tm_min = tm.tm_min;
    _tm.tm_hour = tm.tm_hour;
    _tm.tm_mday = tm.tm_mday;
    _tm.tm_mon = tm.tm_mon - 1;
    _tm.tm_year = tm.tm_year - 1900;
    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
    {
    	printf("Set system datatime error!/n");
    	return -1;
    }
	//system("hwclock -w");
    return 0;
}

int timer_open(void)
{
	struct timeval *start = NULL;
	start=malloc(sizeof(struct timeval));
	if(start == NULL)
	{
		timer_out("timer_open err\r\n");
		return 0;
	}
    memset(start,0,sizeof(struct timeval));
	gettimeofday(start,0);
	return (int)start;
}

int timer_read(int fd)
{
	struct timeval stop;
	struct timeval val;
	struct timeval *start = NULL;
	int result = 0;
	if(fd == 0)
        return 0;
    //if(fd<=0)
		//return DF_ERR_LINUX_FD;
	memset(&stop,0,sizeof(struct timeval));
	memset(&val,0,sizeof(struct timeval));
	start = (struct timeval *)fd;
	gettimeofday(&stop,0);
	timer_out("start:tv_sec=%d,tv_usec=%d\r\n",start->tv_sec,start->tv_usec);
	timer_out("stop:tv_sec=%d,tv_usec=%d\r\n",stop.tv_sec,stop.tv_usec);
	tim_subtract(&val,start,&stop);
	result = val.tv_sec*1000;
	result = val.tv_usec/1000+result;
	return result;
}

unsigned long get_time_sec(void)
{
	struct timeval stop;
	memset(&stop,0,sizeof(struct timeval));
	gettimeofday(&stop,0);
	return stop.tv_sec + 8*3600;
	//return (unsigned long)get_second_from_date_time_turn(timer_asc)+8*3600;//UTC时间加8个小时变成北京时间的时间戳
}

unsigned long long get_time_msec(void)
{
	struct timeval stop;
	memset(&stop,0,sizeof(struct timeval));
	gettimeofday(&stop,0);
	long long sec_t;
	long long usec_t;
	sec_t = stop.tv_sec + (8*3600);
	usec_t = stop.tv_usec;
	long long usec = (sec_t*1000) + (usec_t/1000);
	return usec;
}

unsigned long long get_time_usec(void)
{
	struct timeval stop;
	memset(&stop,0,sizeof(struct timeval));
	gettimeofday(&stop,0);
	long long sec_t;
	long long usec_t;
	sec_t = stop.tv_sec + (8*3600);
	usec_t = stop.tv_usec;
	long long usec = (sec_t*1000000) + usec_t;
	return usec;
}

int get_radom_serial_number(char *cmd,char *data)
{
	srand((unsigned)time(NULL));
	//printf("%lld\n", get_time_usec());
	sprintf(data,"%s_%lld",cmd,get_time_usec());
	return 0;
}

int timer_read_asc(char *timer_asc)//年月日 时分秒 毫秒 微秒
{
	struct timeval tv;
	char log_time[128] = {0};
	char tmp[256];
	long time_sec = get_time_sec();
	get_date_time_from_second_turn(time_sec, log_time);
	gettimeofday(&tv, 0);
	sprintf(tmp, "[%s.%06d]", log_time, (int)(tv.tv_usec));
	memcpy(timer_asc,tmp,strlen(tmp));
	return 0;
}

int timer_read_asc_ms(char *timer_asc)//年月日 时分秒 毫秒
{
	struct timeval tv;
	char tmp[256];
	char log_time[128] = {0x00};
	long time_sec = get_time_sec();
	get_date_time_from_second_turn(time_sec, log_time);
	gettimeofday(&tv, 0);
	sprintf(tmp, "[%s.%03d]", log_time, (int)(tv.tv_usec)/1000);
	memcpy(timer_asc, tmp, strlen(tmp));
	return 0;
}

int timer_read_y_m_d_h_m_s(char *timer_asc)//年月日 时分秒
{
	long time_sec = get_time_sec();
	get_date_time_from_second_turn(time_sec, timer_asc);
	return 0;
}

int timer_read_y_m_d(char *timer_asc)
{
	char time_asc[128] = {0x00};
	long time_sec = get_time_sec();
	get_date_time_from_second_turn(time_sec, time_asc);
	memcpy(timer_asc, time_asc,4);
	memcpy(timer_asc + 4, time_asc + 5,2);
	memcpy(timer_asc + 6, time_asc + 8, 2);
	return 0;
}

void timer_close(int fd)
{
	struct timeval *data;
	data=(struct timeval *)fd;
    if(fd==0)
        return;
	free(data);	
}

static int tim_subtract(struct timeval *result,struct timeval *x,struct timeval *y)
{
  if ( x->tv_sec > y->tv_sec )
  	return   -1;
  if ((x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec))
  	return   -1;
  result->tv_sec = ( y->tv_sec-x->tv_sec );
  result->tv_usec = ( y->tv_usec-x->tv_usec );
  if (result->tv_usec<0)
  {
	result->tv_sec--;
	result->tv_usec+=1000000;
  }
  return   0;
}

//比较日期的前后，dt1<dt2时返回值为-1
int dmax(struct day_t *dt1,struct day_t *dt2) 
{
	if(dt1->year!=dt2->year)
	{
		if(dt1->year>dt2->year) 
			return 1;                    //判断年相等否
		if(dt1->year<dt2->year) 
			return -1;
	}
	else if (dt1->month!=dt2->month)    //判断月相等否
	{
		if(dt1->month>dt2->month) 
			return 1;
		if(dt1->month<dt2->month) 
			return -1;
	}
	else if (dt1->day>dt2->day) 
		return 1;
	else if (dt1->day<dt2->day) 
		return -1;
	else 
		return 0;							//相等返回0
	return 0;
}

unsigned long get_second_from_date_time_turn(char  *date_time)
{
	int iYear = 0,iMon = 0,iDay = 0,iHour = 0,iMin = 0,iSec = 0;
	uint8 day_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int ret = sscanf(date_time,"%04d-%02d-%02d %02d:%02d:%02d",&iYear,&iMon,&iDay,&iHour,&iMin,&iSec);
	//sscanf("2023-09-07 11:03:28","%04d-%02d-%02d %02d:%02d:%02d",&iYear,&iMon,&iDay,&iHour,&iMin,&iSec);
	//printf("22222222222222222222 ret = %d,date_time = %s,iYear = %d,iMon = %d,iDay = %d,iHour = %d,iMin = %d,iSec = %d\n",ret,date_time,iYear,iMon,iDay,iHour,iMin,iSec);
	uint16 i = 0,Cyear = 0;
	unsigned long CountDay = 0;
	for (i = 1970; i<iYear; i++)
	{
		if (((i%4 == 0)&&(i%100 != 0)) || (i%400 == 0))
			Cyear++;
	}
	//printf("1111111111111111111111 date_time = %s,iYear = %d,Cyear = %d,ret = %d\n",date_time,iYear,Cyear,ret);
	CountDay = Cyear*366 + (iYear - 1970 - Cyear)*365;
	for (i = 1; i<iMon; i++)
	{
		if ((i == 2)&&(((iYear%4 == 0)&&(iYear%100 != 0))||(iYear%400 == 0)))
			CountDay += 29;
		else
			CountDay += day_month[i - 1];
	}
	CountDay += (iDay - 1);
	CountDay = CountDay*SECOND_OF_DAY + (unsigned long)iHour*3600 + (unsigned long)iMin*60 + iSec;
	//printf("get_second_from_date_time_turn date_time = %s,CountDay = %ld\n",date_time,CountDay);
	return CountDay;
}

void get_date_time_from_second_turn(unsigned long lsec,char *date_time)
{
	uint16 iYear = 0,iMon = 0,iDay = 0,iHour = 0,iMin = 0,iSec = 0;
	uint8 day_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	uint16 i = 0,j = 0,Day = 0;
	unsigned long lDay = 0;
	lDay = lsec/SECOND_OF_DAY;
	lsec = lsec%SECOND_OF_DAY;
	i = 1970;
	while (lDay > 365)
	{
		if (((i%4 == 0)&&(i%100 != 0))||(i%400 == 0))
			lDay -= 366;
		else
			lDay -= 365;
		i++;
	}
	if ((lDay == 365)&&!(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)))
	{
		lDay -= 365;
		i++;
	}
	iYear = i;
	//printf("get_date_time_from_second_turn lDay = %d,iYear = %d\n",lDay,iYear);
	for (j = 0; j<12; j++)
	{
		if ((j == 1) && (((i%4 == 0)&&(i%100 != 0)) || (i%400 == 0)))
			Day = 29;
		else
			Day = day_month[j];
		if (lDay >= Day)
			lDay -= Day;
		else 
			break;
	}
	iMon = j + 1;
	iDay = lDay + 1;
	iHour = ((lsec/3600))%24;//这里注意，世界时间已经加上北京时间差8，
	iMin = (lsec%3600)/60;
	iSec = (lsec%3600)%60;
	sprintf(date_time,"%04d-%02d-%02d %02d:%02d:%02d",iYear,iMon,iDay,iHour,iMin,iSec);
	return;
}


void get_struct_from_second_turn(unsigned long lsec,struct tm *s)
{
	uint16 iYear = 0,iMon = 0,iDay = 0,iHour = 0,iMin = 0,iSec = 0;
	uint8 day_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	uint16 i = 0,j = 0,Day = 0;
	unsigned long lDay = 0;

	lDay = lsec/SECOND_OF_DAY;
	lsec = lsec%SECOND_OF_DAY;

	i = 1970;
	while (lDay > 365)
	{
		if (((i%4 == 0) && (i%100 != 0)) || (i%400 == 0))
			lDay -= 366;
		else
			lDay -= 365;
		i++;
	}
	if ((lDay == 365) && !(((i%4 == 0) && (i%100 != 0)) || (i%400 == 0)))
	{
		lDay -= 365;
		i++;
	}
	iYear = i;
	for (j = 0; j<12; j++)
	{
		if ((j == 1) && (((i%4 == 0) && (i%100 != 0)) || (i%400 == 0)))
			Day = 29;
		else
			Day = day_month[j];
		if (lDay >= Day)
			lDay -= Day;
		else
			break;
	}
	iMon = j + 1;
	iDay = lDay + 1;
	iHour = ((lsec/3600)) % 24;
	iMin = (lsec%3600) / 60;
	iSec = (lsec%3600) % 60;
	//sprintf(date_time, "%04d-%02d-%02d %02d:%02d:%02d", iYear, iMon, iDay, iHour, iMin, iSec);
	s->tm_year = iYear;
	s->tm_mon = iMon;
	s->tm_mday = iDay;
	s->tm_hour = iHour;
	s->tm_min = iMin;
	s->tm_sec = iSec;
	return;
}