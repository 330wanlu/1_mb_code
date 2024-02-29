#ifndef _TY_PRINT_DEFINE
	#define _TY_PRINT_DEFINE
	#include "../file/_file.h"
	#include "../core/_printer.h"
	#include "../software/_xml.h"
	/*================================立即数定义======================================*/
	#ifdef ty_print_x_c
		#define out(s,arg...)				
		#define e_ty_print_x				
	#else
		#define e_ty_print_x				extern
	#endif
	/*================================函数定义========================================*/
	e_ty_print_x	int ty_print_xml(char *asc,struct _page	*page);
	#ifdef ty_print_x_c
		static void line_write(void *asc,void *page_arg,uint16 number);
		static void page_write(void *asc,void *page_arg,uint16 number);
		static void start_movement_write(void *asc,void *page_arg,uint16 number);
		static void end_movement_write(void *asc,void *page_arg,uint16 number);
		static void label_write(void *asc,void *page_arg,uint16 number);
		static void pic_write(void *asc,void *page_arg,uint16 number);
	#endif
	
	/*=================================常数定义=======================================*/
	#ifdef ty_print_x_c
		#include "_exm_p.h"
	#endif
#endif
