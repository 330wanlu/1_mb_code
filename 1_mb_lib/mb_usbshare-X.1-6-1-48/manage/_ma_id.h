#ifndef _MA_ID_DEFINE
    #define _MA_ID_DEFINE
    /*======================================包含文件============================================*/
    #include "../_opt.h"
#ifndef RELEASE_SO
	#include "../linux_s/_t_file_c.h"
	#include "../software/_code_s.h"
#endif
    /*=====================================立即数定义===========================================*/
    #ifdef _ma_id_c
        #define e_ma_id
    #else
        #define e_ma_id                     extern
    #endif
    /*=====================================结构体定义===========================================*/
    struct st_ma_id
    {   char hard[8];
        char kernel[30];
        uint8 id[6];
    };
    /*======================================函数定义============================================*/
    e_ma_id int read_ma_id(struct st_ma_id  *id);
    e_ma_id int read_kernel(char *ker);
	e_ma_id int get_kernel_vision(char *ker,int *kver1,int hard_type);
    





#endif

