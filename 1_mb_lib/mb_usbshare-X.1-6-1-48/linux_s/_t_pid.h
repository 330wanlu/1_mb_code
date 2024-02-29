#ifndef _T_PID_DEFINE
    #define _T_PID_DEFINE
    #include "_linux_s.h"
#ifndef RELEASE_SO
    #include "../software/_code_s.h"
#endif
    /*====================================����������==================================================*/
        #define DF_T_PID_CM_N                   100                     //��ȡ��ǰ���̺�
        #define DF_T_PID_CM_PATH                DF_T_PID_CM_N+1         //��ȡ��ǰ����·��
        /*--------------------------------�����ʼ����---------------------------------------------*/
        #ifdef _t_pid_c
            #define e_t_pid
            #define out(s,arg...)               
        #else
            #define e_t_pid
        #endif
        /*--------------------------------�������ڲ�����-------------------------------------------*/
        #ifdef _t_pid_c
            #define DF_T_PID_DIR                            "/proc"
        #endif
    /*====================================�ṹ�嶨��==================================================*/
    
    
    /*=====================================��������===================================================*/
    e_t_pid  int _t_pid_n(void);                                    //��ȡ���̺�
    e_t_pid  int _t_pid_path(char *r_path);                         //��ȡ��ǰ����Ŀ¼
    e_t_pid  int _t_hpid_get_file_name(char *r_file_name);          //��ȡ����ȫ��
    e_t_pid  int _t_pid_name(char *name);                           //��ȡ�����������
    e_t_pid  int test_t_pid(int cm);
    


#endif
