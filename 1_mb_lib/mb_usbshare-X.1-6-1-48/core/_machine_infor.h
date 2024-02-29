#ifndef _MACHINE_INFOR_DEFINE
    #define _MACHINE_INFOR_DEFINE
    #include "_switch_dev.h"
    #include "_ty_pd.h"
    #include "_event_file.h"
	#include "_register.h"
	#include <sys/vfs.h>
	#include "../manage/_process_manage.h"

#ifdef RELEASE_TEST
#define BOOT_HTTP_SERVER	"103.45.250.88"
#else
#define BOOT_HTTP_SERVER	"www.njmbxx.com"
#endif
    /*===================================����������=============================================*/
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
    #ifdef _machine_infor_c
         #ifdef DEBUG
			#define out(s,arg...)       				log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						log_out("all.c",s,##arg) 
		#endif	
        #define e_machine_infor
    #else
        #define e_machine_infor                 extern
    #endif



#define DF_MYSQL_START_SHELL1 "/etc/init.d/mysqld"
#define DF_MYSQL_START_SHELL2 "/etc/init.d/mysqld2.0"
#define DF_MYSQL_DATA_DIR1 "/sdcard/mysql"
#define DF_MYSQL_DATA_DIR2 "/mnt/masstore/mysql"
#define DF_EXTEND_SPACE_SHELL	"/etc/massStoreCtr.sh"
    /*===================================�ṹ�嶨��============================================*/
	struct occupy        //����һ��occupy�Ľṹ��
	{
		char name[20];      //����һ��char���͵�������name��20��Ԫ��
		unsigned int user;  //����һ���޷��ŵ�int���͵�user
		unsigned int nice;  //����һ���޷��ŵ�int���͵�nice
		unsigned int system;//����һ���޷��ŵ�int���͵�system
		unsigned int idle;  //����һ���޷��ŵ�int���͵�idle
	};
	typedef struct PACKED        //����һ��mem occupy�Ľṹ��
	{
		char name[20];      //����һ��char���͵�������name��20��Ԫ��
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
#ifndef MTK_OPENWRT
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
#endif 
    #ifdef _machine_infor_c
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
			#ifndef MTK_OPENWRT
			struct _mysql_process mysql_process;
			#endif
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

		static struct _mode_enable mode_enable_a33_default =
		{ 
			.eth_enable="1",
			.wifi_enable="1",
			.g4_enable="1",
			.mysql_enable="1",
			.sqlite_enable="1",
			.ftp_enable="0",
			.ssh_enable="1",
			.ntpdate_enable="1",
			.tf_card_enable="1",
			.event_log_enable="1",
			.usbshare_enable="1",
			.device_manage_enable="0",
			.mqtt_enable="1",
			.tax_invoice_enable="1",
			.extend_space_enable = "1",
#ifdef ALL_SHARE
			.careader_enable="1",
#else
			.careader_enable="0",
#endif
			.userload_enable = "0",
			.usb_port_enable = "1"
		};

		static struct _mode_enable mode_enable_a20_default =
		{
			.eth_enable = "1",
			.wifi_enable = "1",
			.g4_enable = "1",
			.mysql_enable = "1",
			.sqlite_enable="1",
			.ftp_enable = "1",
			.ssh_enable = "1",
			.ntpdate_enable = "1",
			.tf_card_enable = "1",
			.event_log_enable = "1",
			.usbshare_enable = "1",
			.device_manage_enable = "1",
			.mqtt_enable = "1",
			.tax_invoice_enable = "0",
			.extend_space_enable = "0",
			.careader_enable = "1",
			.userload_enable = "0",
			.usb_port_enable = "1"
		};
		#ifdef MTK_OPENWRT
		static struct _mode_enable mode_enable_mtk_default =
		{
			.eth_enable = "1",
			.wifi_enable = "1",
			.g4_enable = "1",
			.mysql_enable = "0",
			.sqlite_enable="0",
			.ftp_enable = "1",
			.ssh_enable = "1",
			.ntpdate_enable = "1",
			.tf_card_enable = "1",
			.event_log_enable = "1",
			.usbshare_enable = "1",
			.device_manage_enable = "0",
			.mqtt_enable = "1",
			.tax_invoice_enable = "1",
			.extend_space_enable = "0",
			.careader_enable = "0",
			.userload_enable = "0",
			.usb_port_enable = "1"
		};
		#endif
    
    #endif
    /*====================================��������=============================================*/
    e_machine_infor int machine_infor_add(const char *switch_name,const _so_note    *note,int hard_type);
	e_machine_infor int check_mem(char  *men_per);
	e_machine_infor int get_boot_size(long *boot_size);
    #ifdef _machine_infor_c
        static int machine_infor_open(struct ty_file	*file);
        static int machine_infor_ctl(void *data,int fd,int cm,va_list args);
        static int machine_infor_close(void *data,int fd);        

		static int creat_ter_info_file(struct _machine_infor  *stream);
		static void delete_log_file(struct _machine_infor *stream);
		static void delete_tax_log_file(struct _machine_infor *stream);
#ifdef RELEASE_SO
		static void delete_invalid_file(void);
#endif
		static int check_process_extend_spcace(struct _machine_infor      *stream);
		#ifndef MTK_OPENWRT
		static int check_process_mysql(struct _machine_infor	*stream);
		static int check_process_sqlite(struct _machine_infor	*stream);
		static int get_mysql_process(struct _machine_infor_fd *id, va_list args);
		#endif
		static int check_process_ftp(struct _machine_infor      *stream);
		static int check_process_ssh(struct _machine_infor      *stream);
		static int check_process_tf_card(struct _machine_infor      *stream);
		static int check_process_careader(struct _machine_infor      *stream);
		static int check_process_userload(struct _machine_infor      *stream);
		static int check_process_ntpdate(struct _machine_infor      *stream);


		static int get_ex_space_mnt();
		static int mode_support(struct _machine_infor *stream, struct _mode_support *mode_support);
		static void process_check_task(void *arg, int timer);
	

        static int set_try_out_date(struct _machine_infor_fd *id,va_list args);
        static int get_id(struct _machine_infor_fd *id,va_list args);
        static int set_id(struct _machine_infor_fd *id,va_list args);
        //static int get_hard(struct _machine_infor_fd *id,va_list args);
        static int get_kernel(struct _machine_infor_fd *id,va_list args);
        static int get_soft(struct _machine_infor_fd *id,va_list args);
        static int get_dev_infor(struct _machine_infor_fd *id,va_list args);
        static int power_off(struct _machine_infor_fd *id,va_list args);
        static int power_reset(struct _machine_infor_fd *id,va_list args);
        static int get_try_out_date(struct _machine_infor_fd *id,va_list args);
		static int get_mqtt_server_info(struct _machine_infor_fd *id, va_list args);
		static int get_mqtt_user_passwd(struct _machine_infor_fd *id, va_list args);
		static int get_mqtt_user_passwd_mem(struct _machine_infor_fd *id, va_list args);
		static int get_ter_information(struct _machine_infor_fd *id, va_list args);
		static int get_mode_support(struct _machine_infor_fd *id, va_list args);
		static int get_mode_enable(struct _machine_infor_fd *id, va_list args);
		
		static int get_extend_space_process(struct _machine_infor_fd *id, va_list args);
		static int get_ftp_process(struct _machine_infor_fd *id, va_list args);
		static int get_ssh_process(struct _machine_infor_fd *id, va_list args);
		static int get_ntpdate_process(struct _machine_infor_fd *id, va_list args);
		static int get_tfcard_process(struct _machine_infor_fd *id, va_list args);
		static int get_careader_process(struct _machine_infor_fd *id, va_list args);
		static int get_userload_process(struct _machine_infor_fd *id, va_list args);

        static void reset_systerm(void *arg);
        static void off_systerm(void *arg);
		#ifdef MTK_OPENWRT
		static int set_wireless_config_file(char *configName, char *configBuffer);
		#endif
		static int get_server_par_set_app(char *address, int *port);
		static int set_server_par_set_app(char *address,int port);
		static int get_configure(char *path, char *in_data, char *out_data);
		static float get_memoccupy(MEM_OCCUPY *mem);
		/*static float cal_occupy(struct occupy *o, struct occupy *n);
		static void get_occupy(struct occupy *o);*/
		static int check_cpu(char  *cpu_per, char *scpu_num);
		
		static int get_total_mem(int *mount, int *total, int *free);
		static int get_sdcard_mount(void);
    #endif
    /*====================================��������============================================*/
    #ifdef _machine_infor_c
		static const struct _machine_infor_fu ctl_fun[] =
		{ { DF_MACHINE_INFOR_CM_GET_ID, get_id },
		{ DF_MACHINE_INFOR_CM_SET_ID, set_id },
		//{ DF_MACHINE_INFOR_CM_GET_HARD, get_hard },
		{ DF_MACHINE_INFOR_CM_GET_KERNEL, get_kernel },
		{ DF_MACHINE_INFOR_CM_GET_SOFT, get_soft },
		{ DF_MACHINE_INFOR_CM_GET_MACHINE_S, get_dev_infor },
		{ DF_MACHINE_INFOR_CM_POWER_OFF, power_off },
		{ DF_MACHINE_INFOR_CM_RESET_POWER, power_reset },
		{ DF_MACHINE_INFOR_CM_SET_TRY_TIME, set_try_out_date },
		{ DF_MACHINE_INFOR_CM_GET_TRY_TIME, get_try_out_date },
		{ DF_MACHINE_INFOR_CM_GET_MQTT_INFO, get_mqtt_server_info },
		{ DF_MACHINE_INFOR_CM_GET_MQTT_USER, get_mqtt_user_passwd },
		{ DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM, get_mqtt_user_passwd_mem },
		{ DF_MACHINE_INFOR_CM_GET_INFOMATION, get_ter_information },
		{ DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, get_mode_support },
		{ DF_MACHINE_INFOR_CM_GET_MODE_ENABLE, get_mode_enable },
		#ifndef MTK_OPENWRT
		{ DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS, get_mysql_process },
		#endif
		{ DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS, get_extend_space_process },
		{ DF_MACHINE_INFOR_CM_GET_FTP_PROCESS, get_ftp_process },
		{ DF_MACHINE_INFOR_CM_GET_SSH_PROCESS, get_ssh_process },
		{ DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS, get_ntpdate_process },
		{ DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS, get_tfcard_process },
		{DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS,get_careader_process},
		{DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS,get_userload_process}
        };
        
        static const struct _file_fuc	machine_infor_fuc=
		{
			#ifndef DF_1201_1	
			.open=machine_infor_open,
			#endif
			.read=NULL,
			.write=NULL,
			.ctl=machine_infor_ctl,
			.close=machine_infor_close,
			.del=NULL
		};

    #endif        

#endif
