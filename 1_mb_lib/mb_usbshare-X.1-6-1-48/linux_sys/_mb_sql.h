#ifndef _MB_SQL_DEFINE
    #define _MB_SQL_DEFINE
#include<stdio.h>
#include<string.h>
#include"mysql/mysql.h"
#include<syslog.h>
#include <unistd.h>
#include <stdlib.h>


struct _mysql_par
{
	char ip[20]; 
	int mysql_port;
	char user[50];
	char passwd[50];
	char db_name[50];
	char tb_name[50];
};

#ifdef _mb_sql_c
        #define e_mb_sql
        #define out(s,arg...)                //log_out("_linux_ip.c",s,##arg)        
    
    #else
        #define e_mb_sql                  extern
    
    #endif
/*====================================º¯Êý¶¨Òå===================================================*/
	e_mb_sql	int mb_mysql_fun_connect(MYSQL* mysql, char *ip, char *user, char *pwd, char *_db, int port);
	e_mb_sql	int mysql_fun_connect(MYSQL* mysql);
    e_mb_sql	void mysql_fun_free_res(MYSQL_RES* res);
	e_mb_sql	void mysql_fun_close(MYSQL* mysql);
	e_mb_sql	int mysql_fun_send(MYSQL* mysql,const char* command);
	e_mb_sql	int mysql_fun_get_int_res(MYSQL* mysql,const char* command);
	e_mb_sql	int mysql_fun_get_char_res(MYSQL* mysql,char* command,char* str);
	e_mb_sql	int mysql_fun_get_num(MYSQL* mysql,char* command);
	e_mb_sql	int get_mysql_db_array(struct _mysql_par *mysql_par, char *select, char *field, char *value,int * id);
	e_mb_sql	int get_mysql_db_data(struct _mysql_par *mysql_par, char *select, char *field, char *value, char *out_data);
	e_mb_sql	int wirte_mysql_db_data(struct _mysql_par *mysql_par,char *field, char *value,char *select,char *key);

#endif