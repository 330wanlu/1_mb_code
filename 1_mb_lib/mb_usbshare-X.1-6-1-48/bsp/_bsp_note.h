#ifndef _BSP_NOTE_DEFINE
	#define _BSP_NOTE_DEFINE
	#include "_usart.h"
	#ifdef _bsp_note_c
		#define e_bsp_note
	#else
		#define e_bsp_note			extern
	#endif
	
	e_bsp_note	void _bsp_load(void);
	
#endif
