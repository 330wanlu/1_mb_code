#define _mb_sql_c
#include"_mb_sql.h"
 
/************************************************
 * @brief    用固定的用户名和密码连接固定的数据库
 * @param    mysql:数据库句柄
 * @return   成功返回0,失败返回-1
 *                       History
 *           Modified by：
 *           Date：
 *           Modification:
 ************************************************/
int mysql_fun_connect(MYSQL* mysql)
{
    if(!mysql_init(mysql))
    {	printf("init failed\n");
        //syslog(LOG_ERR,"file:%s,line:%d.Mysql_init error",__FILE__,__LINE__);
        return -1;
    }
 
    if(!mysql_real_connect(mysql,"localhost","root","123456","mac_db",0,NULL,0))
    {	printf("connect failed\n");
		printf("Insert error %d\n",mysql_errno(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.Connecte mysql error",__FILE__,__LINE__);
        return -1;
    }
 
    return 0;
}


int mb_mysql_fun_connect(MYSQL* mysql,char *ip,char *user,char *pwd,char *_db,int port)
{
    if(!mysql_init(mysql))
    {	printf("mb_mysql_fun_connect init failed\n");
       // syslog(LOG_ERR,"file:%s,line:%d.Mysql_init error",__FILE__,__LINE__);
        return -1;
    }
	//printf("ip = %s\nuser = %s\npwd = %s\n_db = %s\nport = %d\n",ip,user,pwd,_db,port);
    if(!mysql_real_connect(mysql,ip,user,pwd,_db,port,NULL,0))
    {	printf("mb_mysql_fun_connect connect failed\n");
		printf("mysql_real_connect error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.Connecte mysql error",__FILE__,__LINE__);
        return -1;
    }
	usleep(100);
    return 0;
}
 
/*对mysql原释放空间的api进行封装，便于函数名统一*/
void mysql_fun_free_res(MYSQL_RES* res)
{
    mysql_free_result(res);
}
 
/*对mysql原关闭数据库连接的api进行封装，便于函数名统一*/
void mysql_fun_close(MYSQL* mysql)
{
	if (mysql == NULL)
	{
		return ;
	}
    //syslog(LOG_DEBUG,"file:%s,line:%d.In the mysql_my_close",__FILE__,__LINE__);
	//fprintf(stderr, "mysql_fun_close error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
    mysql_close(mysql);
}
 
/***********************************************
 * @brief    发送mysql客户端指令
 * @param    mysql：mysql句柄
 *           command：数据库客户端指令
 * @return   成功返回0,失败返回-1
 *                       History
 *           Modified by：
 *           Date：
 *           Modification:
 **********************************************/
int mysql_fun_send(MYSQL* mysql,const char* command)
{
    int ret = 0;
	if (mysql == NULL)
	{
		return -1;
	}
    ret = mysql_query(mysql,command);
    if(ret)
    {	printf("mysql_fun_send error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.Mysql_query error:%s",__FILE__,__LINE__,mysql_error(mysql));
        return -1;
    }
	usleep(100);
    return 0;
}
 
/***********************************************************
 * @brief    从数据库中获取结果并将其转化为整数
 * @param    mysql：数据库句柄
 *           command：发送给数据库的sql命令
 * @return   成功返回整数结果,错误返回-1
 *                       History
 *           Modified by：
 *           Date：
 *           Modification:
 ***********************************************************/
int mysql_fun_get_int_res(MYSQL* mysql,const char* command)
{
    MYSQL_RES* res = NULL;
    MYSQL_ROW row;
    int ret = 0;
 
	if (mysql == NULL)
	{
		return -1;
	}
    ret = mysql_query(mysql,command);
    if(ret)
	{	printf("mysql_fun_get_int_res mysql_query error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.Mysql query error:%s",__FILE__,__LINE__,mysql_error(mysql));
	}
    else
    {
        res = mysql_store_result(mysql);
        if(res)
        {
            row = mysql_fetch_row(res);
            if(row && (*row))
            {
                mysql_fun_free_res(res);
                res = NULL;
				usleep(100);
                return ((int)atoi(*row));
            }
            
            mysql_fun_free_res(res);
            res = NULL;
        }
        if(mysql_errno(mysql))
		{	printf("mysql_fun_get_int_res mysql_errno error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
            //syslog(LOG_ERR,"file:%s,line:%d.Retrive error:%s",__FILE__,__LINE__,mysql_error(mysql));
		}
    }
 
    return -1;
}
 
/*****************************************************************
 * @brief    从数据库中获取字符串结果
 * @param    mysql:数据库句柄
 *           command:符合要求的mysql客户端指令
 *           str:接收结果的字符串空间指针
 * @return   成功返回空间首地址,失败返回空
 *                       History
 *           Modified by：
 *           Date：
 *           Modification:
 *****************************************************************/
int mysql_fun_get_char_res(MYSQL* mysql,char* command,char* str)
{
    MYSQL_RES* res = NULL;
    MYSQL_ROW row;
    int ret = 0;

	if (mysql == NULL)
	{
		return -1;
	} 
    ret = mysql_query(mysql,command);
    if(ret)
	{	printf("mysql_fun_get_char_res mysql_query error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.Mysql_query error",__FILE__,__LINE__);
	}
    else
    {
        res = mysql_store_result(mysql);
        if(res)
        {
            row = mysql_fetch_row(res);
            if(row && (*row))  //必须有内容且内容不为空，缺少内容不为空的判断可能会造成段错误
            {
                strcpy(str,*row);
                mysql_fun_free_res(res);
                res = NULL;
				usleep(100);
                return 0;
            }
        
            mysql_fun_free_res(res);  //缺少该处可能会造成内存泄漏
            res = NULL;
        }
        if(mysql_errno(mysql))
		{	printf("mysql_fun_get_char_res mysql_errno error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
            //syslog(LOG_ERR,"file:%s,line:%d.Retrive error:%s",__FILE__,__LINE__,mysql_error(mysql));
		}
    }
 
    return -1;
}
 
/****************************************************
 * @brief    获取结果集中结果的数量
 * @param    mysql：数据库句柄
 *           command：数据库指令
 * @return   成功返回结果集数量,失败返回-1
 *                       History
 *           Modified by：
 *           Date：
 *           Modification:
 ***************************************************/
int mysql_fun_get_num(MYSQL* mysql,char* command)
{
    MYSQL_RES* res = NULL;
    int ret        = 0;
    int num        = 0;
	if (mysql == NULL)
	{
		return -1;
	}
    ret = mysql_query(mysql,command);
    if(ret)
    {	printf("mysql_fun_get_num mysql_query error %d: %s\n",mysql_errno(mysql),  mysql_error(mysql));
        //syslog(LOG_ERR,"file:%s,line:%d.selected error:%s",__FILE__,__LINE__,mysql_error(mysql));
        return -1;
    }
    res = mysql_store_result(mysql);
    if(res)
    {
        num = (int)mysql_num_rows(res);
        mysql_fun_free_res(res);
        res = NULL;
    }
	usleep(100);
    return num;
}











//获取数据库中查找符合条件的记录号数组
int get_mysql_db_array(struct _mysql_par *mysql_par, char *select, char *field, char *value,int * id)
{
	MYSQL *conn_ptr;
	MYSQL_RES *res_ptr;
	MYSQL_ROW sqlrow;
	//MYSQL_FIELD *fd;
	unsigned char sql_cmd[200];
	int res, i, j,k=0;
	unsigned long row_count = 0;
	memset(sql_cmd, 0, sizeof(sql_cmd));
	conn_ptr = mysql_init(NULL);
	if (!conn_ptr)
	{	return -1;
	}
	conn_ptr = mysql_real_connect(conn_ptr,mysql_par->ip, mysql_par->user, mysql_par->passwd, mysql_par->db_name, 0, NULL, 0);
	if (conn_ptr)
	{
		out("连接数据库成功\n");
		sprintf((char *)sql_cmd, "select %s from %s where %s = \"%s\"",select,mysql_par->tb_name,field,value);
		res = mysql_query(conn_ptr, (const char *)sql_cmd); //查询语句  
		if (res)
		{	out("SELECT error:%s\n", mysql_error(conn_ptr));
		}
		else
		{	res_ptr = mysql_store_result(conn_ptr);             //取出结果集  
			if (res_ptr)
			{	row_count = mysql_num_rows(res_ptr);//结果集行数目
				//printf("%lu Rows\n",row_count);   
				if (row_count <= 0)
				{	//out("rows <= 0\n");
					//out("没有查找到符合条件的记录\n");
					mysql_free_result(res_ptr);
					mysql_close(conn_ptr);
					return 0;
				}
				j = mysql_num_fields(res_ptr);//结果集字段数目
				//printf("%lu fields\n", j);
				while ((sqlrow = mysql_fetch_row(res_ptr)))
				{   //依次取出记录  
					for (i = 0; i < j; i++)
					{	//printf("%s\t", sqlrow[i]);              //输出 	
						id[k] = atoi(sqlrow[i]);
						k += 1;
						//out("第%d条数据,内容为%s,atoi = %d,转换后内容为：%d\n", i, sqlrow[i], atoi(sqlrow[i]),id[k-1]);
						//memset(sql_passwd, 0, sizeof(sql_passwd));
						//memcpy(sql_passwd, sqlrow[i], strlen(sqlrow[i]));
					}       
				}
				if (mysql_errno(conn_ptr))
				{	//fprintf(stderr, "Retrive error:s\n", mysql_error(conn_ptr));
					printf("Retrive error:%s\n", mysql_error(conn_ptr));
				}
			}
			mysql_free_result(res_ptr);
		}
	}
	else
	{	out("Connection failed\n");
		mysql_close(conn_ptr);
		return -1;
	}
	mysql_close(conn_ptr);
	return row_count;
}



//获取数据库中查找符合条件的唯一一条记录中的字段
int get_mysql_db_data(struct _mysql_par *mysql_par, char *select, char *field, char *value, char *out_data)
{	out("get_mysql_db_data\n");
	MYSQL *conn_ptr;
	MYSQL_RES *res_ptr;
	MYSQL_ROW sqlrow;
	//MYSQL_FIELD *fd;
	unsigned char sql_cmd[200];
	int res, i, j = 0;
	unsigned long row_count = 0;
	memset(sql_cmd, 0, sizeof(sql_cmd));
	conn_ptr = mysql_init(NULL);
	if (!conn_ptr)
	{
		return -1;
	}
	out("准备连接数据库\n");
	conn_ptr = mysql_real_connect(conn_ptr, mysql_par->ip, mysql_par->user, mysql_par->passwd, mysql_par->db_name, 0, NULL, 0);
	if (conn_ptr)
	{
		out("连接数据库成功\n");
		sprintf((char *)sql_cmd, "select %s from %s where %s = \"%s\"", select, mysql_par->tb_name, field, value);
		out("sql :%s\n",sql_cmd);
		res = mysql_query(conn_ptr, (const char *)sql_cmd); //查询语句  
		if (res)
		{
			out("SELECT error:%s\n", mysql_error(conn_ptr));
		}
		else
		{	
			res_ptr = mysql_store_result(conn_ptr);             //取出结果集 
			if (res_ptr)
			{	
				row_count = mysql_num_rows(res_ptr);//结果集行数目
				//printf("%lu Rows\n", row_count);
				if (row_count != 1)
				{
					out("rows != 1\n");
					out("没有查找到符合条件的记录\n");
					mysql_free_result(res_ptr);
					mysql_close(conn_ptr);
					return 0;
				}
				j = mysql_num_fields(res_ptr);//结果集字段数目
				//printf("%lu fields\n", j);
				while ((sqlrow = mysql_fetch_row(res_ptr)))
				{   //依次取出记录  
					for (i = 0; i < j; i++)
					{
						//printf("%s\n", sqlrow[i]);              //输出 
						memset(out_data, 0, sizeof(out_data));
						out("发票请求流水号：%s\n",sqlrow[i]);
						if(sqlrow[i] != NULL)
						{
							memcpy(out_data, sqlrow[i], strlen(sqlrow[i]));
						}
						out("第%d条数据,转换后内容为：%s\n", i, out_data);

					}
				}
				if (mysql_errno(conn_ptr))
				{
					printf("Retrive error:%s\n", mysql_error(conn_ptr));
				}
			}
			mysql_free_result(res_ptr);
		}
	}
	else
	{
		out("Connection failed\n");
		mysql_close(conn_ptr);
		return -1;
	}
	mysql_close(conn_ptr);
	return row_count;
}


//修改数据库中查找符合条件的唯一一条记录中的字段
int wirte_mysql_db_data(struct _mysql_par *mysql_par,char *field, char *value,char *select,char *key)
{
	MYSQL *conn_ptr;
	//MYSQL_RES *res_ptr;
	//MYSQL_ROW sqlrow;
	//MYSQL_FIELD *fd;
	unsigned char sql_cmd[200];
	int res;
	unsigned long row_count = 0;
	memset(sql_cmd, 0, sizeof(sql_cmd));
	conn_ptr = mysql_init(NULL);
	if (!conn_ptr)
	{
		return -1;
	}
	conn_ptr = mysql_real_connect(conn_ptr, mysql_par->ip, mysql_par->user, mysql_par->passwd, mysql_par->db_name, 0, NULL, 0);
	if (conn_ptr)
	{
		out("连接数据库成功\n");
		//sprintf(sql_cmd, "select %s from %s where %s = \"%s\"", select, mysql_par->tb_name, field, value);//修改语句  
		sprintf((char *)sql_cmd, "UPDATE %s SET %s = \"%s\" where %s = \"%s\"",mysql_par->tb_name, field, value,select,key);//修改语句 
		out("数据库操作语句：%s\n",sql_cmd);
		res = mysql_query(conn_ptr, (const char *)sql_cmd);
		if (res)
		{
			out("SELECT error:%s\n", mysql_error(conn_ptr));
		}
	}
	else
	{
		out("Connection failed\n");
		mysql_close(conn_ptr);
		return -1;
	}
	mysql_close(conn_ptr);
	return row_count;
}