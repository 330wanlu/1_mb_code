#define _ty_explain_c
#include "_ty_explain.h"
int ty_explain(struct	_ty_agree	*data,uint8 *buf,int *tail)
{	int i;
	int buf_len;
	for(;;)
	{	if(*tail<DF_SPEAK_LEN)
			return 0;
		out("����ͷ\n");
		for(i=0;i<(*tail-1);i++)
		{	if((buf[i]=='T')&&(buf[i+1]=='Y'))
				break;
		}
		if(i!=0)
		{	out("û���ҵ�ͷ\n");
			memcpy(buf,buf+i,(*tail-i));
			(*tail)=(*tail)-i;
			continue;
		}
		out("����Դͷ\n");
		if((buf[2]!=DF_FILE_MAST)&&(buf[2]!=DF_FILE_SLAVE))
		{	memcpy(buf,buf+2,(*tail-2));
			(*tail)=(*tail)-2;
			out("Դͷ����\n");
			continue;	
		}
		memcpy(&buf_len,buf+3,sizeof(buf_len));
		out("��֡�����Ϊ:%d\n",buf_len);
		if((buf_len<=DF_SPEAK_LEN)||(buf_len>DF_COM_MAX_LEN))
		{	out("���ݳ��ȴ���\n");
			memcpy(buf,buf+2,(*tail-2));
			(*tail)=(*tail)-2;
			continue;
		}
		if(buf_len>*tail)
		{	out("��֡����û����ȫ\n");
			return 0;
		}	
		(*tail)=(*tail)-buf_len;
		data->len=buf_len-DF_SPEAK_LEN;
		data->m_s=buf[2];
		memcpy(&data->ls,buf+3+sizeof(buf_len),sizeof(data->ls));
		memcpy(&data->order,buf+3+sizeof(buf_len)+sizeof(data->ls),sizeof(data->order));
		memcpy(data->buf,buf+DF_SPEAK_LEN,data->len);
		
			
		memcpy(buf,buf+buf_len,(*tail));
		out("\r\n�����Ժ�:\n");
		out("data->order=%d\n",data->order);
		out("data->ls=%d\n",data->ls);
		out("data->len=%d\n",data->len);
		out("data->m_s=%d\n",data->m_s);
		out("data->buf:\n");
		for(i=0;i<data->len;i++)
		{	out(" %2x",data->buf[i]);
		}
		out("\r\n");
		return data->len;
	}
}




