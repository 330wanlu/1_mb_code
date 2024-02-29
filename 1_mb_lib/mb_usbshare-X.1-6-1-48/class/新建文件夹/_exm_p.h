#ifndef EXM_P_DEFINE
	#define EXM_P_DEFINE
	/*=======================================立即数定义===============================================*/
	#define ZZ(A,B)												(uint16 )(((A	*)NULL)->B)	
	#define page_head(B)										ZZ(_page_head,B)
	#define line_asc(B)											ZZ(_line_asc,B)
	#define lable_asc(B)										ZZ(_lable_asc,B)
	#define pic_asc(B)											ZZ(_pic_asc,B)
	#define paper_asc(B)										ZZ(_paper_asc,B)

	#define BUF_IN(buf)			(uint16 *)buf,sizeof(buf)/sizeof(uint16)
	#define xml_exp(head,s_head,str,hex_buf,next,end,pro,write)	\
				   {	head,s_head,											\
						{	str,BUF_IN(hex_buf),(struct st_xml_exp_table	*)next	},					\
						end,pro,write	}
	#define xml_exp_NULL(head,s_head,str,next,end,pro,write)	\
				   {	head,s_head,											\
						{	str,NULL,NULL,(struct st_xml_exp_table	*)next	},					\
						end,pro,write	}
	/*=======================================结构体定义=============================================*/
	typedef struct
	{	char feed_paper[100];				//纸宽
		char back_paper[100];				//纸长
		char black_paper[100];				//字体大小
	}_paper_asc;
	
	typedef struct
	{	char width[100];				//纸宽
		char length[100];				//纸长
		char font[100];				//字体大小
		char line_space[100];			//行间距
	}_page_head;
	
	typedef struct
	{	char line_space[100];			//行间距
	}_line_asc;
	
	typedef struct
	{	char x[100];
		char y[100];
		char code[1024];
		char font[100];
	}_lable_asc;
	
	typedef struct
	{	char x[100];
		char y[100];
		char code[20000];
		char length[100];
		char width[100];
	}_pic_asc;
	
	
	
	
	/*==================================BUF 使用定义==============================================*/
	static const uint16 page_buf[]=
	{	page_head(width),
		page_head(length),
		page_head(font),
		page_head(line_space)
	};
	
	static const uint16 line_asc_buf[]=
	{	line_asc(line_space)
		
	};
	
	
	static const uint16 lable_asc_buf[]=
	{	lable_asc(x),
		lable_asc(y),
		lable_asc(code),
		lable_asc(font)
	};
	
	static const uint16 pic_asc_buf[]=
	{	pic_asc(x),
		pic_asc(y),
		pic_asc(code),
		pic_asc(length),
		pic_asc(width)
	};
		
	static const uint16 paper_asc_buf[]=
	{	paper_asc(feed_paper),
		paper_asc(back_paper),
		paper_asc(black_paper)
	};
	
	/*======================================xml 定义==============================================*/
		/*++++++++++++++++++++++++++++++++lable pic 定义++++++++++++++++++++++++++++++++++++*/
		static const struct st_xml_exp	   col_xml[]=
		{	xml_exp("<label_list COUNT='\r\n'>",
						"<map>",
							"<x>\r\n</x>"
							"<y>\r\n</y>"
							"<code>\r\n</code>"
							"<font>\r\n</font>"
						"</map>",lable_asc_buf,NULL,
					"</label_list>",1,label_write	),	
			xml_exp(	"<picture_list COUNT='\r\n'>",
							"<map>",
								"<x>\r\n</x>"
								"<y>\r\n</y>"
								"<code>\r\n</code>"
								"<length>\r\n</length>"
								"<width>\r\n</width>"
							"</map>",pic_asc_buf,NULL,
						"</picture_list>",1,pic_write	)
		};
		
		static const struct st_xml_exp_table	col_table=
		{	(struct st_xml_exp	*)col_xml,sizeof(col_xml)/sizeof(struct st_xml_exp)	
		};
		
		
	static const struct st_xml_exp	page_xml[]=
	{	xml_exp("<property>",NULL,
					"<width>\r\n</width>"
					"<length>\r\n</length>"
					"<font>\r\n</font>"
					"<line_space>\r\n</line_space>",page_buf,NULL,
				"</property>",0,page_write),
		xml_exp("<print_start>",NULL,
					"<feed_paper>\r\n</feed_paper>"
					"<back_paper>\r\n</back_paper>"
					"<black_paper>\r\n</black_paper>",paper_asc_buf,NULL,
				"</print_start>",0,start_movement_write),
		xml_exp("<print_end>",NULL,
					"<feed_paper>\r\n</feed_paper>"
					"<back_paper>\r\n</back_paper>"
					"<black_paper>\r\n</black_paper>",paper_asc_buf,NULL,
				"</print_end>",0,end_movement_write),
		
		
		xml_exp("<line COUNT=\'\r\n\'>",
					"<line_map>",
						"<line_space>\r\n</line_space>"
					"</line_map>",line_asc_buf,&col_table,
				"</line>",1,line_write)
	};
	
	static const struct st_xml_exp_table	page_table=
	{	(struct st_xml_exp	*)page_xml,sizeof(page_xml)/sizeof(struct st_xml_exp)	
		
	};
		
#endif	
	