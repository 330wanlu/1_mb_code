#ifndef _TIMER_H
#define _TIMER_H

#include "opt.h"

#include "linux_s.h"

#define SECOND_OF_DAY   86400

#define timer_out(s,arg...)		//log_out("all.c",s,##arg)

struct day_t
{
	int year;
	int month;
	int day;
};

int clock_get(uint8 *buf);
int SetSystemTime(uint8 *dt);
int timer_open(void);
int timer_read(int fd);
void timer_close(int fd);
int dmax(struct day_t * dt1,struct day_t * dt2);
//int print_time(uint8 *data);
unsigned long get_time_sec(void);
unsigned long long get_time_msec(void);
unsigned long long get_time_usec(void);
int timer_read_asc(char *timer_asc);
int timer_read_asc_ms(char *timer_asc);//年月日 时分秒 毫秒
int timer_read_y_m_d_h_m_s(char *timer_asc);
int timer_read_y_m_d(char *timer_asc);

int get_radom_serial_number(char *cmd, char *data);
//unsigned long get_second_from_date_time(char  *date_time);
//void get_date_time_from_second(unsigned long lsec, char *date_time);
unsigned long get_second_from_date_time_turn(char  *date_time);
void get_date_time_from_second_turn(unsigned long lsec, char *date_time);
void get_struct_from_second_turn(unsigned long lsec, struct tm *s);

#endif

