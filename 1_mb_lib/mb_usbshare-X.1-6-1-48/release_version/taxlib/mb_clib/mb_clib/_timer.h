#ifndef _TIMER_DEFINE
	#define _TIMER_DEFINE
	//#include "_linux_s.h"
	#include "mb_typedef.h"
	#include <linux/rtc.h>
	#include <unistd.h>
	#include "_code_s.h"
	#include<time.h>
	#include <sys/time.h>
	/*=============================立即数定义==========================================*/
	#ifdef _TIMER_C
		#define e_timer	
	#else
		#define e_timer							extern
	#endif
	#ifdef _TIMER_C
			#ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)
		#endif	
	#endif

	struct day_t
	{
		int year;
		int month;
		int day;
	};

	

	/*==============================函数定义==========================================*/
	
	e_timer	int clock_get(uint8 buf[8]);
	e_timer int SetSystemTime(uint8 *dt);
	e_timer	int timer_open(void);
	e_timer	int timer_read(int fd);
	e_timer	void timer_close(int fd);
	e_timer int dmax(struct day_t * dt1,struct day_t * dt2);
	//e_timer int print_time(uint8 *data);
	e_timer unsigned long get_time_sec(void);
	e_timer unsigned long long get_time_msec(void);
	e_timer unsigned long long get_time_usec(void);
	e_timer int timer_read_asc(char *timer_asc);	
	e_timer int timer_read_asc_ms(char *timer_asc);//年月日 时分秒 毫秒
	e_timer int timer_read_y_m_d_h_m_s(char *timer_asc);
	e_timer int timer_read_y_m_d(char *timer_asc);

	e_timer int get_radom_serial_number(char *cmd, char *data);
	//e_timer unsigned long get_second_from_date_time(char  *date_time);
	//e_timer void get_date_time_from_second(unsigned long lsec, char *date_time);

	e_timer unsigned long get_second_from_date_time_turn(char  *date_time);
	e_timer void get_date_time_from_second_turn(unsigned long lsec, char *date_time);
	void get_struct_from_second_turn(unsigned long lsec, struct tm *s);
	#ifdef _TIMER_C
		static int tim_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
		static	uint8 hex2bcd(uint8 hex);
	#endif
	

	
	
#endif

