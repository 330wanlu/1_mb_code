#define _mb_sql_c
#include"_mb_sql.h"
 
/************************************************
 * @brief    �ù̶����û������������ӹ̶������ݿ�
 * @param    mysql:���ݿ���
 * @return   �ɹ�����0,ʧ�ܷ���-1
 *                       History
 *           Modified by��
 *           Date��
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
 
/*��mysqlԭ�ͷſռ��api���з�װ�����ں�����ͳһ*/
void mysql_fun_free_res(MYSQL_RES* res)
{
    mysql_free_result(res);
}
 
/*��mysqlԭ�ر����ݿ����ӵ�api���з�װ�����ں�����ͳһ*/
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
 * @brief    ����mysql�ͻ���ָ��
 * @param    mysql��mysql���
 *           command�����ݿ�ͻ���ָ��
 * @return   �ɹ�����0,ʧ�ܷ���-1
 *                       History
 *           Modified by��
 *           Date��
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
 * @brief    �����ݿ��л�ȡ���������ת��Ϊ����
 * @param    mysql�����ݿ���
 *           command�����͸����ݿ��sql����
 * @return   �ɹ������������,���󷵻�-1
 *                       History
 *           Modified by��
 *           Date��
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
 * @brief    �����ݿ��л�ȡ�ַ������
 * @param    mysql:���ݿ���
 *           command:����Ҫ���mysql�ͻ���ָ��
 *           str:���ս�����ַ����ռ�ָ��
 * @return   �ɹ����ؿռ��׵�ַ,ʧ�ܷ��ؿ�
 *                       History
 *           Modified by��
 *           Date��
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
            if(row && (*row))  //���������������ݲ�Ϊ�գ�ȱ�����ݲ�Ϊ�յ��жϿ��ܻ���ɶδ���
            {
                strcpy(str,*row);
                mysql_fun_free_res(res);
                res = NULL;
				usleep(100);
                return 0;
            }
        
            mysql_fun_free_res(res);  //ȱ�ٸô����ܻ�����ڴ�й©
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
 * @brief    ��ȡ������н��������
 * @param    mysql�����ݿ���
 *           command�����ݿ�ָ��
 * @return   �ɹ����ؽ��������,ʧ�ܷ���-1
 *                       History
 *           Modified by��
 *           Date��
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











//��ȡ���ݿ��в��ҷ��������ļ�¼������
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
		out("�������ݿ�ɹ�\n");
		sprintf((char *)sql_cmd, "select %s from %s where %s = \"%s\"",select,mysql_par->tb_name,field,value);
		res = mysql_query(conn_ptr, (const char *)sql_cmd); //��ѯ���  
		if (res)
		{	out("SELECT error:%s\n", mysql_error(conn_ptr));
		}
		else
		{	res_ptr = mysql_store_result(conn_ptr);             //ȡ�������  
			if (res_ptr)
			{	row_count = mysql_num_rows(res_ptr);//���������Ŀ
				//printf("%lu Rows\n",row_count);   
				if (row_count <= 0)
				{	//out("rows <= 0\n");
					//out("û�в��ҵ����������ļ�¼\n");
					mysql_free_result(res_ptr);
					mysql_close(conn_ptr);
					return 0;
				}
				j = mysql_num_fields(res_ptr);//������ֶ���Ŀ
				//printf("%lu fields\n", j);
				while ((sqlrow = mysql_fetch_row(res_ptr)))
				{   //����ȡ����¼  
					for (i = 0; i < j; i++)
					{	//printf("%s\t", sqlrow[i]);              //��� 	
						id[k] = atoi(sqlrow[i]);
						k += 1;
						//out("��%d������,����Ϊ%s,atoi = %d,ת��������Ϊ��%d\n", i, sqlrow[i], atoi(sqlrow[i]),id[k-1]);
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



//��ȡ���ݿ��в��ҷ���������Ψһһ����¼�е��ֶ�
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
	out("׼���������ݿ�\n");
	conn_ptr = mysql_real_connect(conn_ptr, mysql_par->ip, mysql_par->user, mysql_par->passwd, mysql_par->db_name, 0, NULL, 0);
	if (conn_ptr)
	{
		out("�������ݿ�ɹ�\n");
		sprintf((char *)sql_cmd, "select %s from %s where %s = \"%s\"", select, mysql_par->tb_name, field, value);
		out("sql :%s\n",sql_cmd);
		res = mysql_query(conn_ptr, (const char *)sql_cmd); //��ѯ���  
		if (res)
		{
			out("SELECT error:%s\n", mysql_error(conn_ptr));
		}
		else
		{	
			res_ptr = mysql_store_result(conn_ptr);             //ȡ������� 
			if (res_ptr)
			{	
				row_count = mysql_num_rows(res_ptr);//���������Ŀ
				//printf("%lu Rows\n", row_count);
				if (row_count != 1)
				{
					out("rows != 1\n");
					out("û�в��ҵ����������ļ�¼\n");
					mysql_free_result(res_ptr);
					mysql_close(conn_ptr);
					return 0;
				}
				j = mysql_num_fields(res_ptr);//������ֶ���Ŀ
				//printf("%lu fields\n", j);
				while ((sqlrow = mysql_fetch_row(res_ptr)))
				{   //����ȡ����¼  
					for (i = 0; i < j; i++)
					{
						//printf("%s\n", sqlrow[i]);              //��� 
						memset(out_data, 0, sizeof(out_data));
						out("��Ʊ������ˮ�ţ�%s\n",sqlrow[i]);
						if(sqlrow[i] != NULL)
						{
							memcpy(out_data, sqlrow[i], strlen(sqlrow[i]));
						}
						out("��%d������,ת��������Ϊ��%s\n", i, out_data);

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


//�޸����ݿ��в��ҷ���������Ψһһ����¼�е��ֶ�
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
		out("�������ݿ�ɹ�\n");
		//sprintf(sql_cmd, "select %s from %s where %s = \"%s\"", select, mysql_par->tb_name, field, value);//�޸����  
		sprintf((char *)sql_cmd, "UPDATE %s SET %s = \"%s\" where %s = \"%s\"",mysql_par->tb_name, field, value,select,key);//�޸���� 
		out("���ݿ������䣺%s\n",sql_cmd);
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