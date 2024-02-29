#ifndef _DEPLOY_DEFINE
    #define _DEPLOY_DEFINE
    #include "_switch_dev.h"
    #include "_ty_usb.h"
    #include "_ty_pd.h"
    #include "_event_file.h"
#ifdef MTK_OPENWRT
	#include "../software/_mb_tools.h"
	#include "../linux_sys/_linux_ip_mtk.h"
#else
    #include "../linux_sys/_linux_ip.h"
#endif
	#include "_get_net_time.h"
	#include "../linux_s/_t_pid.h"
	#include "../_opt.h"
	#include "_ty_usb_m.h"
    /*==================================����������=================================================*/
    #define DF_DEPLOY_CM_SET_BUSID          100                             //����busid
    #define DF_DEPLOY_CM_SET_PORT_N         (DF_DEPLOY_CM_SET_BUSID+1)      //����PORT_N
    #define DF_DEPLOY_CM_SET_ENUM_TIMER     (DF_DEPLOY_CM_SET_PORT_N+1)     //����ö�����ʱ��
    #define DF_DEPLOY_CM_SET_IP             (DF_DEPLOY_CM_SET_ENUM_TIMER+1) //����IP
    #define DF_DEPLOY_CM_SET_START_PORT     (DF_DEPLOY_CM_SET_IP+1)         //������ʼ�˿ں�
    #define DF_DEPLOY_CM_SET_NET_SOFT       (DF_DEPLOY_CM_SET_START_PORT+1) //������������
	#define DF_DEPLOY_CM_SET_VPN_PAR		(DF_DEPLOY_CM_SET_NET_SOFT+1)	//����VPN����
	#define DF_DEPLOY_CM_SET_CLOSE_UPDATE	(DF_DEPLOY_CM_SET_VPN_PAR+1)	//�����Ƿ�ر��Զ�����
    #define DF_DEPLOY_CM_GET_BUSID          (DF_DEPLOY_CM_SET_CLOSE_UPDATE+1)    //��ȡbusid
    #define DF_DEPLOY_CM_GET_PORT_N         (DF_DEPLOY_CM_GET_BUSID+1)      //��ȡport_n
    #define DF_DEPLOY_CM_GET_ENUM_TIMER     (DF_DEPLOY_CM_GET_PORT_N+1)     //��ȡö�����ʱ��
    #define DF_DEPLOY_CM_GET_IP             (DF_DEPLOY_CM_GET_ENUM_TIMER+1) //��ȡIP
    #define DF_DEPLOY_CM_GET_START_PORT     (DF_DEPLOY_CM_GET_IP+1)         //��ȡ��ʼ�˿�
    #define DF_DEPLOY_CM_GET_NET_SOFT       (DF_DEPLOY_CM_GET_START_PORT+1) //��ȡ�������� 
    #define DF_DEPLOY_CM_GET_REDIT          (DF_DEPLOY_CM_GET_NET_SOFT+1)   //��ȡע����Ϣ  
    #define DF_DEPLOY_CM_SET_REDIT          (DF_DEPLOY_CM_GET_REDIT+1)      //ע��    
    #define DF_DEPLOY_CM_GET_NET_PAR        (DF_DEPLOY_CM_SET_REDIT+1)      //��ȡ�������
	#define	DF_DEPLOY_CM_GET_WIFI_PAR		(DF_DEPLOY_CM_GET_NET_PAR+1)	//��ȡWIFI�������
	#define DF_DEPLOY_CM_GET_FTP_DATA		(DF_DEPLOY_CM_GET_WIFI_PAR+1)	//��ȡFTP��������
	#define DF_DEPLOY_CM_GET_NET_ADDR		(DF_DEPLOY_CM_GET_FTP_DATA+1)	//��ȡ������������Ϣ
	#define DF_DEPLOY_CM_SET_IP_UPDATE		(DF_DEPLOY_CM_GET_NET_ADDR+1)	//ˢ��IP����
	#define DF_DEPLOY_CM_GET_VPN_PAR		(DF_DEPLOY_CM_SET_IP_UPDATE+1)	//��ȡVPN����
	#define DF_DEPLOY_CM_GET_CLOSE_UPDATE	(DF_DEPLOY_CM_GET_VPN_PAR+1)	//��ȡ�Ƿ�ر��Զ�����
	#define DF_DEPLOY_CM_SET_AUTH			(DF_DEPLOY_CM_GET_CLOSE_UPDATE+1)
	#define DF_DEPLOY_CM_SET_TER_POW_MODE	(DF_DEPLOY_CM_SET_AUTH + 1)
	#define DF_DEPLOY_CM_GET_TER_POW_MODE	(DF_DEPLOY_CM_SET_TER_POW_MODE + 1)
#ifdef MTK_OPENWRT
#define DF_DEPLOY_CM_SET_IP_W			(DF_DEPLOY_CM_GET_TER_POW_MODE+1)			//����wifi
#endif  
    #ifdef _deploy_c
		 #ifdef DEBUG
			#define out(s,arg...)       				//log_out("all.c",s,##arg) 
		#else
			#define out(s,arg...)						//log_out("all.c",s,##arg)
		#endif	
        #define DF_DEPLOY_PAR_EDIT_ODE              0X00000001
        #define DF_DEPLOY_PAR_EDIT                  0X00000002        
		#define DF_DEPLOY_PAR_EDIT_2                0X00000003
		#ifdef MTK_OPENWRT
		#define DF_NET_DEV                          "eth0.2"
		#define DF_OTHER_NET_FIlE_PATH					"/home/share/exe/other_net.txt"
		#else
        #define DF_NET_DEV                          "eth0"
		#endif
        #define DF_DEPLOY_TASK_TIMER                1000    
        #define e_deploy
		struct _deploy  *fd_stream;
    #else
        #define e_deploy                    extern
    #endif
    /*====================================�ṹ��===================================================*/
	struct _vpn_deploy
		{
			int enable;
			int mode;
			char addr[50];
			int port;
			char user[20];
			char passwd[20];
			int ftp_port;
			char ftp_user[20];
			char ftp_passwd[20];
		};
    #ifdef _deploy_c
        struct _deploy_file
        {   int edit;                   //�汾��
            int port_n;                 //�˿�����
            int enum_timer;             //ö�����ʱ��
			char busid[30];             //U�������˿�
			int start_port;             //��ʼ�˿ں� 
            uint32 ip;                     //maid               
            uint32 mask;                //��������
            uint32 getway;              //����
			uint32 DNS;
			int ip_type;				//IP����DHCP��̶�
            char net_soft[200];         //����������ַ          
			int vpn_enable;
			int vpn_mode;
			char vpn_addr[50];
			int vpn_port;
			char vpn_user[20];
			char vpn_passwd[20];
			int vpn_ftp_port;
			char vpn_ftp_user[20];
			char vpn_ftp_passwd[20];
			int close_auto_update;		//�ر��Զ����� 0���ر� 1�ر�
			#ifdef MTK_OPENWRT
			int lan_in;//�Ƿ������߲���
			int flag_dhcp;//dhcp�Ƿ�ִ����� 1 ��� 0δ���
			#endif
			
        };
		#ifdef MTK_OPENWRT
		struct _other_net_file
		{
			int ip_mode;//0:lan 1:wifi 2;4g
			char wifi_name[50];
			char wifi_pwd[20];
			char wifi_mode[30];
		};
		#endif
        struct _deploy_fd
        {	int state;
            struct ty_file_mem	*mem;
            void *mem_dev;
            struct ty_file_fd	*mem_fd;
            struct _deploy   *dev;
        };
        
        struct _deploy
        {   int switch_fd;                                  //����ģ��
            int ftp_fd;                                     //FTPģ��
			int sd_fd;
            int ty_usb_fd;                                  //
			int net_time_fd;
			int ty_usb_m_fd;
            int ftp_task;                                   //ftp����
            int lock;
			int hardtype;
            char switch_name[DF_FILE_NAME_MAX_LEN];         //����ģ��
            //char ftp_name[DF_FILE_NAME_MAX_LEN];            //FTP����
            char ty_usb_name[DF_FILE_NAME_MAX_LEN];         //TY_USB����
			//char net_time_name[DF_FILE_NAME_MAX_LEN];		//�����ʱ
            struct _deploy_fd    fd[DF_FILE_OPEN_NUMBER];
            int state;
            struct _deploy_file     file;
            //struct _download_net_arg    net_file;
			//struct download_sd_data_arg sd_file;

#ifdef MTK_OPENWRT
			struct _other_net_file     file_o;
#endif
            _so_note    *note;
        };
        struct _deloy_ctl_fu
        {   int cm;
            int (*ctl)(struct _deploy_fd   *id,va_list args);
        };
        
    #endif
    /*=======================================��������============================================*/   
    e_deploy    int deploy_add(_so_note    *note,const char *switch_name,const char *usb_name,int hard_type);
	//e_deploy int set_ip_by_overall(uint32 ip,uint32 mask,uint32 getway,uint32 DNS,int ip_type);
	
	
    #ifdef _deploy_c
        static int deploy_open(struct ty_file	*file);  
        static int deploy_ctl(void *data,int fd,int cm,va_list args); 
        static int deploy_close(void *data,int fd);    
        

        
        static int set_busid(struct _deploy_fd  *id,va_list args);
        static int set_ip_ip(struct _deploy_fd  *id,va_list args);
        static int set_enum_timer(struct _deploy_fd  *id,va_list args);
        static int set_port_n(struct _deploy_fd  *id,va_list args);
        static int set_start_port(struct _deploy_fd  *id,va_list args);
		static int set_vpn_par(struct _deploy_fd  *id,va_list args);
		//static int set_close_update(struct _deploy_fd  *id,va_list args);
		static int get_wifi_par(struct _deploy_fd  *id, va_list args);
        static int get_net_par(struct _deploy_fd  *id,va_list args);
        //static int get_sd_soft_par(struct _deploy *stream,struct download_sd_data_arg *data);
        //static int get_ftp_soft_par(struct _deploy  *stream,struct _download_net_arg    *data);
		//static int get_net_soft_host_user_pwd(struct _deploy_fd  *id,va_list args);
		//static int get_ftp_data(struct _deploy_fd  *id,va_list args);			
        static int get_busid(struct _deploy_fd  *id,va_list args);
        static int get_port_n(struct _deploy_fd  *id,va_list args);
        static int get_enum_timer(struct _deploy_fd  *id,va_list args);
        static int get_ip_ip(struct _deploy_fd  *id,va_list args);
        static int get_start_port(struct _deploy_fd  *id,va_list args);
        static int get_net_soft(struct _deploy_fd  *id,va_list args);
		static int get_vpn_par(struct _deploy_fd  *id,va_list args);
		static int get_close_update(struct _deploy_fd  *id,va_list args);
		static int set_auth(struct _deploy_fd  *id, va_list args);
		
        static void reset_systerm(void *arg);
        static void set_ip_q(void *arg);
        static int update_ip_ip(struct _deploy_fd *id,va_list args);
		static int write_power_mode_flash(struct _deploy_fd  *id, va_list args);
		static int read_power_mode_flash(struct _deploy_fd  *id, va_list args);
#ifdef MTK_OPENWRT
		static void set_ip_w(void *arg);
		static void set_ip_4g(void *arg);
		static int read_other_net_file(struct _deploy *stream);
		static int set_other_net_file(int net_mode, struct _deploy  *stream);
		static int set_ip_wifi_wifi(struct _deploy_fd  *id, va_list args);
        

		static int check_lan_ip_get(struct _deploy  *stream);
		static int check_wifi_ip_get(struct _deploy  *stream);
		static int check_4g_ip_get(struct _deploy  *stream);

#endif
        
        
    #endif    
    /*=======================================��������============================================*/
    #ifdef _deploy_c
		//Ĭ��ΪA33����,�жϴ���Ӳ�����ͺ��޸�
        static struct _deploy_file soft_cut=
        {   .edit=DF_DEPLOY_PAR_EDIT_2,         //�汾��
            .port_n=0,                          //Ĭ������˳��
            .enum_timer=5,                      //Ĭ��ö��ʱ��
            .busid={'1','-','1'},       		//U��������ַ             
            .start_port=1,						//��ʼ�˿ں�Ϊ1
			.ip=0,                              //Ĭ�ϲ�����ip
            .mask=0,                            //��������
            .getway=0,                          //����
			.DNS=0,								//Ĭ��Ϊ0
			.ip_type=0,							//Ĭ��IP����Ϊ�̶�IP
			.net_soft="host=[156d721f79.iok.la],user=[njmb],pwd=[123456],timer=[0]",
			.vpn_enable=0,
			.vpn_mode=0,
			.vpn_ftp_port=21,
			.vpn_ftp_user="vpnuser",
			.vpn_ftp_passwd="vpnuser123",
			.close_auto_update=0
        };
        static const struct _deloy_ctl_fu ctl_fun[]=
        {   {   DF_DEPLOY_CM_SET_BUSID,         set_busid       },
            {   DF_DEPLOY_CM_SET_ENUM_TIMER,    set_enum_timer  },
            {   DF_DEPLOY_CM_SET_PORT_N,        set_port_n      },
            {   DF_DEPLOY_CM_SET_IP,            set_ip_ip       },
            {   DF_DEPLOY_CM_SET_START_PORT,    set_start_port  },
			{	DF_DEPLOY_CM_SET_VPN_PAR,		set_vpn_par		},			
            {   DF_DEPLOY_CM_GET_BUSID,         get_busid       },
            {   DF_DEPLOY_CM_GET_PORT_N,        get_port_n      },
            {   DF_DEPLOY_CM_GET_ENUM_TIMER,    get_enum_timer  },
            {   DF_DEPLOY_CM_GET_IP,            get_ip_ip       },
            {   DF_DEPLOY_CM_GET_START_PORT,    get_start_port  },
            {   DF_DEPLOY_CM_GET_NET_SOFT,      get_net_soft    },
			{	DF_DEPLOY_CM_GET_WIFI_PAR,		get_wifi_par	},
            {   DF_DEPLOY_CM_GET_NET_PAR,       get_net_par     },
			//{	DF_DEPLOY_CM_GET_FTP_DATA,		get_ftp_data	},
			//{	DF_DEPLOY_CM_GET_NET_ADDR,		get_net_soft_host_user_pwd},
			{	DF_DEPLOY_CM_SET_IP_UPDATE,		update_ip_ip	},
			{	DF_DEPLOY_CM_GET_VPN_PAR,		get_vpn_par		},
			{	DF_DEPLOY_CM_GET_CLOSE_UPDATE,	get_close_update},
			{	DF_DEPLOY_CM_SET_AUTH,			set_auth },
			{	DF_DEPLOY_CM_SET_TER_POW_MODE,	write_power_mode_flash },
			{	DF_DEPLOY_CM_GET_TER_POW_MODE,	read_power_mode_flash },
			#ifdef MTK_OPENWRT
			{	DF_DEPLOY_CM_SET_IP_W,			set_ip_wifi_wifi },
			#endif
        };
        
        static const struct _file_fuc	deploy_fuc=
		{	.open=deploy_open,
			.read=NULL,
			.write=NULL,
			.ctl=deploy_ctl,
			.close=deploy_close,
			.del=NULL
		};
        
        

    #endif        
        

#endif
