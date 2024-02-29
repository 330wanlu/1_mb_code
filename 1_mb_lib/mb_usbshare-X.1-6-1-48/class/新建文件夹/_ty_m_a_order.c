#define _ty_m_a_order_c
#include "_ty_m_a_order.h"
int ty_m_a_order(int order,uint8 *buf,int buf_len,va_list args)
{	int i;
	out("ty_m_a_order:order=%d,buf=%d,buf_len=%d\n",order,buf,buf_len);
	for(i=0;i<sizeof(order_buf)/sizeof(struct ty_m_a_buf);i++)
	{	out("order_buf[i].order=%d\n",order_buf[i].order);
		if(order_buf[i].order==order)
			break;
	}
	out("order_buf[%d]\n",i);
	if(i==sizeof(order_buf)/sizeof(struct ty_m_a_buf))
		return -1;
	return order_buf[i].oper_buf(buf,buf_len,args);
}


static int ty_m_a_order_read_buf(uint8 *buf,int buf_len,va_list args)
{	int i;
	uint8 *r_buf;
	r_buf=va_arg(args,uint8 *);
	out("read_buf:r_buf=%d,buf_len=%d\n",r_buf,buf_len);
	memcpy(r_buf,buf,buf_len);
	for(i=0;i<buf_len;i++)
	{	out("%2x\n",r_buf[i]);
	}
	return buf_len;	
}

