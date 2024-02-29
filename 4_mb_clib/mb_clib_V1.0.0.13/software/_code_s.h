#ifndef _CODE_S_DEFINE
	#define _CODE_S_DEFINE
	#include "mb_typedef.h"
	/*================================立即数定义=====================================*/
	#ifdef _CODE_S_C
		#define _code_s_e
		#define out(s,arg...)				            
	#else
		#define _code_s_e								extern
	#endif
	#define TRANS_ASC_XML_T_SMALL						0			//全部小写
	#define TRANS_ASC_XML_T_BIG							1			//全部大写
	
	/*================================函数定义=======================================*/
    _code_s_e   int juge_usb_dev(char *name);
    _code_s_e   uint32 ismoney(uint8 *p);
	_code_s_e	uint32 ismoney2(uint8 *p);  
    _code_s_e   int get_dir(const char *dir,char *r_dir);
    _code_s_e   int get_dir_file(const char *dir,char *r_name);
    _code_s_e   int time_asc(uint8 *timer_bcd,uint8 *timer_asc,uint8 time_number);
    _code_s_e	void trans_asc_xml(char *buf,uint8 type);
	_code_s_e	void asc2hexbuf(char *asc,uint8 *hex_buf,uint32 asc_len);
	_code_s_e	uint8 asc2hex(uint8 asc);
	_code_s_e	uint32 asc2algorism(char *asc,uint8 asc_len);				//ASC转十进制
	_code_s_e	uint8 algorism2asc(uint32 algorism,uint8 *asc);
	
	_code_s_e	void asc2bcd(uint8 *asc,uint8 *bcd,uint8 size_asc);
	_code_s_e	void bcd2asc(uint8 *bcd,uint8 *asc,uint8 size_bcd);
	
	_code_s_e	uint32 ipaddress_asc(uint32 ip,uint16 port,char *asc);
	_code_s_e	void ipaddress_asc_hex(const char *asc,uint32 size_asc,uint32 *ip,uint16 *port);
	_code_s_e 	uint32 ip_asc2ip(const char *ip_asc,uint32 size_ip_asc);
	_code_s_e	uint32 ip2ip_asc(uint32 ip,char *ip_asc);
	
	
	_code_s_e	int32 j_asc1bcd(char *asc,uint16 asc_len);					//判断ASCBCD是否符合规则
	_code_s_e	int32 j_asc1hex(char *asc,int32 asc_len);
	_code_s_e	int32 j_ip_port_asc(char *asc,int32 asc_len);
	
	_code_s_e	char *strstr_n(const char *str,const char *search,uint8 n);		//在一串字符串内查找出第N个字符串
	_code_s_e	void cls_space(char *buf,uint32 len);
    _code_s_e   int get_feild(const char *scr,const char *f,char *r_f);
    _code_s_e   int get_feild_d(const char *scr,const char *f,char *r_f);
	_code_s_e	void cls_no_asc(char *buf,uint32 len);
	_code_s_e	void cls_character(char *buf, uint32 len, char del_char);
	_code_s_e	int str_replace(char *strRes, char *from, char *to);
	_code_s_e	int str_replace_once(char *strRes, char *from, char *to);
	_code_s_e	int StrFind(char *strRes, char *find);
	#ifdef _CODE_S_C
		static void trans_small(char *buf,int32 len);
		static void trans_big(char *buf,int32 len);
	#endif

#endif	

