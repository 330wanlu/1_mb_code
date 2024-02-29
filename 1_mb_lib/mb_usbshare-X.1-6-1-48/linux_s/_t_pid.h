#ifndef _T_PID_DEFINE
    #define _T_PID_DEFINE
    #include "_linux_s.h"
#ifndef RELEASE_SO
    #include "../software/_code_s.h"
#endif
    /*====================================立即数定义==================================================*/
        #define DF_T_PID_CM_N                   100                     //获取当前进程号
        #define DF_T_PID_CM_PATH                DF_T_PID_CM_N+1         //获取当前进程路径
        /*--------------------------------程序初始定义---------------------------------------------*/
        #ifdef _t_pid_c
            #define e_t_pid
            #define out(s,arg...)               
        #else
            #define e_t_pid
        #endif
        /*--------------------------------本程序内部定义-------------------------------------------*/
        #ifdef _t_pid_c
            #define DF_T_PID_DIR                            "/proc"
        #endif
    /*====================================结构体定义==================================================*/
    
    
    /*=====================================函数定义===================================================*/
    e_t_pid  int _t_pid_n(void);                                    //获取进程号
    e_t_pid  int _t_pid_path(char *r_path);                         //获取当前运行目录
    e_t_pid  int _t_hpid_get_file_name(char *r_file_name);          //获取运行全称
    e_t_pid  int _t_pid_name(char *name);                           //获取运行软件名称
    e_t_pid  int test_t_pid(int cm);
    


#endif
