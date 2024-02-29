#define ty_print_x_c
#include "_ty_print_x.h"
int ty_print_xml(char *asc,struct _page	*page)					//解析一页数据
{	return xml_read(&page_table,asc,page);
}
static void line_write(void *asc,void *page_arg,uint16 number)			//申请line
{	_line_asc	*line_asc;
	struct _page	*page;
	struct _line	*line;
	struct _line	**line_val;
	if(page_arg==NULL)
		return ;
	out("======================line_write=======================\r\n");
	line_asc=asc;
	page=(struct _page	*)page_arg;
	for(line=page->line,line_val=&page->line;line!=NULL;
	line_val=&line->next,line=line->next);
	line=mem_malloc(sizeof(struct _line));
	if(line==NULL)
	{	out("mem line err\r\n");
		return ;
	}
	memset(line,0,sizeof(struct _line));
	line->line_space=asc2algorism(line_asc->line_space,strlen(line_asc->line_space)); 
	out("line_val=%d\r\n",line_val);
	
	*line_val=line;	
	out("line->line_space=%d\r\n",line->line_space);
	out("++++++++++++++++++++++line over+++++++++++++++++++\r\n");
	return ;	
}

static void page_write(void *asc,void *page_arg,uint16 number)
{	_page_head	*page_asc;
	struct _page	*page;
	page_asc=asc;
	page=page_arg;
	out("==========================page write============================\r\n");
	out("page_head:width=%s\r\n",page_asc->width);
	out("page_head:length=%s\r\n",page_asc->length);
	out("page_head:font=%s\r\n",page_asc->font);
	out("page_head:line_space=%s\r\n",page_asc->line_space);
	page->font=asc2algorism(page_asc->font,strlen(page_asc->font));
	page->line_space=asc2algorism(page_asc->line_space,strlen(page_asc->line_space));
	out("page_write suc\r\n");
	return ;
}

static void start_movement_write(void *asc,void *page_arg,uint16 number)
{	_paper_asc	*movement_asc;
	struct _movement	*data;
	struct _page	*page;
	movement_asc=asc;
	page=page_arg;
	data=&page->start;
	data->feed_paper=asc2algorism(movement_asc->feed_paper,strlen(movement_asc->feed_paper));
	data->back_paper=asc2algorism(movement_asc->back_paper,strlen(movement_asc->back_paper));
	data->black_paper=asc2algorism(movement_asc->black_paper,strlen(movement_asc->black_paper));
	out("start_movement_write over\r\n");
	return ;
}

static void end_movement_write(void *asc,void *page_arg,uint16 number)
{	_paper_asc	*movement_asc;
	struct _movement	*data;
	struct _page	*page;
	movement_asc=asc;
	page=page_arg;
	data=&page->end;
	data->feed_paper=asc2algorism(movement_asc->feed_paper,strlen(movement_asc->feed_paper));
	data->back_paper=asc2algorism(movement_asc->back_paper,strlen(movement_asc->back_paper));
	data->black_paper=asc2algorism(movement_asc->black_paper,strlen(movement_asc->black_paper));
	out("end_movement_write over\r\n");
	return ;
}



static void label_write(void *asc,void *page_arg,uint16 number)
{	_lable_asc	*lable_asc;
	struct _page	*page;
	struct _line	*line;
	struct _lable	*lable;
	struct _lable	**lable_val;
	int code_len;
	lable_asc=asc;
	page=page_arg;
	out("【lable_asc】code:%s\r\n\r\n",lable_asc->code);
	
	out("=========================label write===========================\r\n");
	for(line=page->line;line->next!=NULL;line=line->next);		//找到当前有效的line
	for(lable=line->l_data,lable_val=&line->l_data;lable!=NULL;
	lable_val=&lable->next,lable=lable->next);
	lable=mem_malloc(sizeof(struct _lable));
	if(lable==NULL)
	{	out("mem lable err\r\n");
		return ;
	}
	memset(lable,0,sizeof(struct _lable));
	lable->x=asc2algorism(lable_asc->x,strlen(lable_asc->x));
	out("lable->x=%d\r\n",lable->x);
	lable->y=asc2algorism(lable_asc->y,strlen(lable_asc->y));
	out("lable->y=%d\r\n",lable->y);
	lable->font=asc2algorism(lable_asc->font,strlen(lable_asc->font));
	out("lable->font=%d\r\n",lable->font);
	code_len=strlen(lable_asc->code);
	lable->code=mem_malloc(code_len+1);
	if(lable->code==NULL)
	{	out("mem lable->code\r\n");
		return ;
	}
	memcpy(lable->code,lable_asc->code,code_len);
	lable->code[code_len]=0;
	out("lable->code=%s\r\n",lable->code);
	out("lable_val=%d\r\n",lable_val);
	*lable_val=lable;
	out("lable over\r\n");
	return ;
}

static void pic_write(void *asc,void *page_arg,uint16 number)		//PIC
{	_pic_asc	*pic_asc;
	struct _page	*page;
	struct _line	*line;
	struct _pic		*pic;
	struct _pic		**pic_val;
	int code_len,i;
	pic_asc=asc;
	page=page_arg;
	for(line=page->line;line->next!=NULL;line=line->next);		//找到当前有效的line
	for(pic=line->p_data,pic_val=&line->p_data;pic!=NULL;
	pic_val=&pic->next,pic=pic->next);
	pic=mem_malloc(sizeof(struct _pic));
	if(pic==NULL)
	{	out("mem pic err\r\n");
		return ;
	}
	memset(pic,0,sizeof(struct _pic));
	pic->x=asc2algorism(pic_asc->x,strlen(pic_asc->x));
	out("pic->x=%d\r\n",pic->x);
	pic->y=asc2algorism(pic_asc->y,strlen(pic_asc->y));
	out("pic->y=%d\r\n",pic->y);
	pic->length=asc2algorism(pic_asc->length,strlen(pic_asc->length));
	out("pic->length=%d\r\n",pic->length);
	pic->width=asc2algorism(pic_asc->width,strlen(pic_asc->width));
	out("pic->width=%d\r\n",pic->width);
	code_len=strlen(pic_asc->code);
	out("code_len=%d\r\n",code_len);
	*pic_val=pic;
	if(j_asc1hex(pic_asc->code,code_len)==0)
	{	pic->code=mem_malloc(code_len/2);
		if(pic->code==NULL)
		{	out("mem pic code err\r\n");
			return ;
		}
		memset(pic->code,0,code_len/2);
		asc2hexbuf(pic_asc->code,pic->code,code_len);
		for(i=0;i<code_len/2;i++)
			out("%x\r\n",pic->code[i]);
		return ;
	}
	out("hex_buf err\r\n");
	return ;
}


