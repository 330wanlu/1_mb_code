#define _bsp_note_c
#include "_bsp_note.h"
void _bsp_load(void)
{	_linux_usart_load();
	_linux_spi_load();
	_linux_io_load();
	_linux_i2c_load();
	_linux_bsp_s_load();
	_soft_stm32_load();
	_linux_rc531_load();
	_linux_lv3070_load();
}


