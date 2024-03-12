#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
//#include <cmath>
#define DF_OUTPUT_NAME   "USBShareServer.exe: "
/*
struct day{
	int mouth;
	int days;
};


struct day calender[12] = {
	{ 1, 31  } ,  { 2, 28  } , { 3, 31 },
	{ 4, 30  } ,  { 5, 31  } , { 6, 30 },
	{ 7, 31  } ,  { 8, 31  } , { 9, 30 },
	{ 10, 31 } ,  { 11, 30 } , { 12,31 },
};*/
void log_out(const char* format, ...);

int log_init();

#endif

