#define _load_c
#include "_load.h"
void load(void)
{   //_linux_load();
	memset(&linux_s, 0, sizeof(struct _linux_s));
	_lock_load(&linux_s.lock);
	_task_load(&linux_s.task);
	_queue_load(&linux_s.queue);
	_err_load(NULL);//最后修改剔除err
	mosquitto_lib_init();

    ty_file_load();
    _linux_usart_load(DF_MB_HARDWARE_A20_A33);
	_err_add("port_core", &port_core_err_buf);//最后修改剔除err
}

