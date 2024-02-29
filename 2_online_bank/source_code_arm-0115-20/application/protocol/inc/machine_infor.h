#ifndef _MACHINE_INFOR_H
#define _MACHINE_INFOR_H

#include "opt.h"

#include "switch_dev.h"
#include "register.h"
#include "process_manage.h"

#ifdef RELEASE_TEST
#define BOOT_HTTP_SERVER	"103.45.250.88"
#else
#define BOOT_HTTP_SERVER	"www.njmbxx.com"
#endif

#define DF_MACHINE_INFOR_CM_GET_ID				100
#define DF_MACHINE_INFOR_CM_SET_ID				(DF_MACHINE_INFOR_CM_GET_ID+1)
#define DF_MACHINE_INFOR_CM_GET_HARD			(DF_MACHINE_INFOR_CM_SET_ID+1)
#define DF_MACHINE_INFOR_CM_GET_KERNEL			(DF_MACHINE_INFOR_CM_GET_HARD+1)
#define DF_MACHINE_INFOR_CM_GET_SOFT			(DF_MACHINE_INFOR_CM_GET_KERNEL+1)
#define DF_MACHINE_INFOR_CM_GET_MACHINE_S		(DF_MACHINE_INFOR_CM_GET_SOFT+1)
#define DF_MACHINE_INFOR_CM_POWER_OFF			(DF_MACHINE_INFOR_CM_GET_MACHINE_S+1) 
#define DF_MACHINE_INFOR_CM_RESET_POWER			(DF_MACHINE_INFOR_CM_POWER_OFF+1)    
#define DF_MACHINE_INFOR_CM_SET_TRY_TIME		(DF_MACHINE_INFOR_CM_RESET_POWER+1)
#define DF_MACHINE_INFOR_CM_GET_TRY_TIME		(DF_MACHINE_INFOR_CM_SET_TRY_TIME+1)
#define DF_MACHINE_INFOR_CM_GET_MQTT_INFO		(DF_MACHINE_INFOR_CM_GET_TRY_TIME+1)
#define	DF_MACHINE_INFOR_CM_GET_MQTT_USER		(DF_MACHINE_INFOR_CM_GET_MQTT_INFO+1)
#define	DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM	(DF_MACHINE_INFOR_CM_GET_MQTT_USER+1)
#define DF_MACHINE_INFOR_CM_GET_INFOMATION		(DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM+1)
#define DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT	(DF_MACHINE_INFOR_CM_GET_INFOMATION+1)
#define DF_MACHINE_INFOR_CM_GET_MODE_ENABLE		(DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT+1)
#define DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS	(DF_MACHINE_INFOR_CM_GET_MODE_ENABLE+1)
#define DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS	(DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_FTP_PROCESS		(DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_SSH_PROCESS		(DF_MACHINE_INFOR_CM_GET_FTP_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS		(DF_MACHINE_INFOR_CM_GET_SSH_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS		(DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS		(DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS+1)
#define DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS		(DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS+1)
#define DF_CONFIG_FILE_PATH			"/etc/mqttserver.conf"

#define machine_info_out(s,arg...)       		//log_out("all.c",s,##arg) 

#define DF_MYSQL_START_SHELL1 "/etc/init.d/mysqld"
#define DF_MYSQL_START_SHELL2 "/etc/init.d/mysqld2.0"
#define DF_MYSQL_DATA_DIR1 "/sdcard/mysql"
#define DF_MYSQL_DATA_DIR2 "/mnt/masstore/mysql"
#define DF_EXTEND_SPACE_SHELL	"/etc/massStoreCtr.sh"

struct st_ma_id
{
	char hard[8];
	char kernel[30];
	uint8 id[6];
};

struct occupy        //声明一个occupy的结构体
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned int user;  //定义一个无符号的int类型的user
	unsigned int nice;  //定义一个无符号的int类型的nice
	unsigned int system;//定义一个无符号的int类型的system
	unsigned int idle;  //定义一个无符号的int类型的idle
};

typedef struct PACKED        //定义一个mem occupy的结构体
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned long total;
	char name2[20];
	unsigned long free;
}MEM_OCCUPY;

struct _machine_s
{   uint8 id[6];
	uint8 r_date[10];
	char soft[8];
	char hard[8];
	int n;
};

struct _ter_info
{
	char product_code[50];
	char product_name[50];
	char product_rename[50];
	char ter_id[20];
	char product_date[20];
	char total_space[20];
	char left_space[20];
	char ram[20];
	char mem_per[50];
	char cpu_per[50];
	char cpu_num[20];
	char uptime[50];
	char kernel_ver[20];
	char ter_version[50];
	char ter_version_detail[100];
	char tax_ver[50];
	char mbc_ver[50];
	char ter_type[50];
	char tax_so_load[2];
	char auth_file_load[2];
	char cpu_id[50];
	long boot_size;
};

struct _mode_support
{
	char eth_support[10];
	char wifi_support[10];
	char g4_support[10];
	char mysql_support[10];
	char sqlite_support[10];
	char ftp_support[10];
	char ssh_support[10];
	char ntpdate_support[10];
	char tf_card_support[10];
	char event_log_support[10];
	char usbshare_support[10];
	char device_manage_support[10];
	char mqtt_support[10];
	char tax_invoice_support[10];		
	char extend_space_support[10];
	char careader_support[10];
	char userload_support[10];
	char usb_port_support[10];
};

struct _mode_enable
{
	char eth_enable[10];
	char wifi_enable[10];
	char g4_enable[10];
	char mysql_enable[10];
	char sqlite_enable[10];
	char ftp_enable[10];
	char ssh_enable[10];
	char ntpdate_enable[10];
	char tf_card_enable[10];
	char event_log_enable[10];
	char usbshare_enable[10];
	char device_manage_enable[10];
	char mqtt_enable[10];
	char tax_invoice_enable[10];		
	char extend_space_enable[10];
	char careader_enable[10];
	char userload_enable[10];
	char usb_port_enable[10];
};

struct _userload_process
{
	int failed_time;
	char process_run[10];
	char err_describe[100];
};

struct _careader_process
{
	int failed_time;
	char process_run[10];
	char err_describe[100];
};

struct _tfcard_process
{
	int failed_time;
	char insert[10];
	char mount[10];
	char status[10];
};

struct _ntpdate_process
{
	int failed_time;
	char process_run[10];
	char server[200];
	char time_now[20];
};

struct _ssh_process
{
	int failed_time;
	char process_run[10];
	char port[10];
	char load_name[50];
	char load_passwd[50];
};

struct _ftp_process
{
	int failed_time;
	char process_run[10];
	char port[10];
	char load_name[50];
	char load_passwd[50];
	char default_path[100];
};

struct _extend_space_process
{
	int failed_time;
	char space_size[20];
	char mount[10];
	char mount_dir[100];
};

struct _mysql_process
{
	int failed_time;
	char version[50];
	char process_run[10];
	char port[10];
	char load_name[50];
	char load_passwd[50];
	char data_path[100];
};

struct _machine_infor_fd
{	int state;
	struct ty_file_mem	*mem;
	void *mem_dev;
	struct ty_file_fd	*mem_fd;
	struct _machine_infor  *dev;
};

struct _machine_infor
{   int state;
	int lock;
	struct _machine_infor_fd    fd[DF_FILE_OPEN_NUMBER];
	char switch_name[DF_FILE_NAME_MAX_LEN]; 
	char register_name[DF_FILE_NAME_MAX_LEN]; 
	//char *hard;    
	const _so_note    *note;
	int switch_fd;  
	int register_fd;
	char ker_ver[DF_FILE_NAME_MAX_LEN];
	struct _switch_dev_id devid;
	char cpu_id[50];
	int ker_update;
	int hard_type;
	char mqtt_server[256];
	int mqtt_port;
	char mqtt_user[50];
	char mqtt_passwd[50];
	struct _mode_support mode_support;
	struct _mode_enable mode_enable;
	struct _mysql_process mysql_process;
	struct _extend_space_process extend_space_process;
	struct _ftp_process ftp_process;
	struct _ssh_process ssh_process;
	struct _ntpdate_process ntpdate_process;
	struct _tfcard_process tfcard_process;
	struct _careader_process careader_process;
	struct _userload_process userload_process;
};

struct _machine_infor_fu
{   int cm;
	int(*ctl)(struct _machine_infor_fd   *id, va_list args);
};

int machine_infor_add(const char *switch_name,const _so_note    *note,int hard_type);
int check_mem(char  *men_per);
int get_boot_size(long *boot_size);

#endif
