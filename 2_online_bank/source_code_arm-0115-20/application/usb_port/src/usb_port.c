#include "../inc/usb_port.h"

USB_PORT_CLASS usb_port;

int add_needed_read_list_node(int port_num);
int delete_needed_read_list_node_with_portnum(int port_num);
int add_reading_list_node(int port_num);
int delete_reading_list_node_with_portnum(int port_num);
void deal_read_plate_base_tax_info(void *arg);
READ_TAX_NAME_LIST *get_reading_list_node_with_portnum(int port_num);

void pack_report_m_server_status_data(void *arg);
int fun_get_ter_base_data_json(cJSON *data_layer);
void pack_report_json_data(void *arg);
void pack_report_m_server_heart_data(void *arg);
void tigger_report_port_status(void);
void pack_report_plate_info(void *arg);
void pack_report_other_info_new_mqtt(void *arg);
void pack_report_ca_info_new_mqtt(void *arg);
void pack_report_m_server_status_data_report(void *arg);

static int usbport_socket_read(int sock, uint8 *buf, int buf_len);

int fun_get_ter_all_data_json(cJSON *data_layer);
int fun_get_ter_all_data_json_without_port(cJSON *data_layer);
int fun_get_ter_all_data_json_without_tax(cJSON *data_layer);

int set_ignore_judge_flag(int port_num,enum IGNORE_FLAG flag);
enum IGNORE_FLAG get_ignore_judge_flag(int port_num);
int get_cert_json_file(unsigned char *key_zlib_json, char **cert_json);
int fun_deal_usb_port_power(int port_num, PORT_STATE_ENUM port_state);

void* fun_upload_auth_cert_file(void *args);
void *fun_query_auth_file(void *args);
int check_port_auth_info(KEY_AUTH_STATE *stream);

void *check_port_with_num(void *args);

void *check_port_status(void *args);

void *start_robot_arm(void *args);
void *update_robot_arm(void *args);
void *get_robot_arm_update_state(void *args);

//����������غ�������
void* load_start(void* args);
void* load_stop(void* args);

//���˿ڲ����غ�������
void *get_port_ad_status(void *arg);
void *start_step(void *arg);
void *recheck_step(void *arg);
void *conform_step(void *arg);
void *extract_init(void *arg);

int start_get_port_thread(void);

//�̻߳ص�����
int get_port_status_thread(void *arg, int o_timer);
int usbport_exist_recheck_thread(void *arg, int o_timer);
int port_check_thread(void *arg, int o_timer);
int read_tax_name_thread(void *arg, int o_timer);
int report_thread(void *arg, int o_timer);
int attach_thread(void *arg, int o_timer);
int port_led_manage_thread(void *arg, int timer);
int upgrade_strategy_thread(void *arg, int timer);

int get_arm_with_dev(void);

int get_port_used_state(int port_num);

USB_PORT_CLASS *get_usb_port_class(void)
{
	return &usb_port;
}

enum USBPORT_EXIST_STATE get_port_exit(int port)
{
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port|| usb_port_numb < port)
		return USB_PORT_PARAMETER_ERROR;
	return usb_port.judge_usbport_exist.port_exist_state[port].now_port_state;
}

void *get_machine_type(void *args)
{
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	while(0 >= usb_port.module->machine_fd) //�ȴ�ģ��׼�����
	{
		sleep(1);
	}
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	if(0 == memcmp(ter_info.ter_id,"068330",6))  //����е�ۻ���
	{
		usb_port.machine_type = TYPE_2303;
	}
	else if(0 == memcmp(ter_info.ter_id,"068161",6))  //2110  100�ڻ���
	{
		usb_port.machine_type = TYPE_2110;
	}
	else if(0 == memcmp(ter_info.ter_id,"068181",6))  //2108  68�ڻ���
	{
		usb_port.machine_type = TYPE_2108;
	}
	else if(0 == memcmp(ter_info.ter_id,"068171",6))  //2202  68�ڻ���
	{
		usb_port.machine_type = TYPE_2202;
	}
	else if(0 == memcmp(ter_info.ter_id,"068360",6))  //2306  60�ڻ���
	{
		usb_port.machine_type = TYPE_2306;
	}
	else if(0 == memcmp(ter_info.ter_id,"068191",6))  //2212  120�ڻ���
	{
		usb_port.machine_type = TYPE_2212;
	}
	else										//������е�ۻ���
	{
		usb_port.machine_type = TYPE_2303;
	}
	//printf("########################get_machine_type usb_port.machine_type = %d\n",usb_port.machine_type);
	return (void*)(&usb_port.machine_type);
}

void *check_with_robot_arm(void *args)
{
	int result = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		result = 1;
	}
	// printf("########################check_with_robot_arm result = %d\n",result);
	return (void *)(&result);
}

/*********************************************************************
-   Function : usb_port_init
-   Description��usb����ģ���ʼ����ע���ȡ�˿�״̬�̣߳���������߳�
-   Input  :��ȡ�˿������豸��Ŀǰ������adֵ���жϣ�
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int usb_port_init(MODULE *module,_so_note *app_note)
{
	int result = USB_PORT_NO_ERROR,i = 0,try_count = 0;
	
	if(NULL == module || NULL == app_note)
	{
		return USB_PORT_PARAMETER_ERROR;
	}
	memset(&usb_port,0x00,sizeof(USB_PORT_CLASS));
	usb_port.get_port_status.fun_get_port_ad_status = get_port_ad_status;
	usb_port.judge_usbport_exist.judge_exist_fun.fun_start_step = start_step;
	usb_port.judge_usbport_exist.judge_exist_fun.fun_recheck_step = recheck_step;
	usb_port.judge_usbport_exist.judge_exist_fun.fun_conform_step = conform_step;
	usb_port.judge_usbport_exist.judge_exist_fun.fun_init = extract_init;
	usb_port.fun_get_ter_all_data_json = fun_get_ter_all_data_json;
	usb_port.fun_get_ter_all_data_json_without_port = fun_get_ter_all_data_json_without_port;
	usb_port.fun_get_ter_all_data_json_without_tax = fun_get_ter_all_data_json_without_tax;
	usb_port.read_tax_name.need_read_list_lock = _lock_open("need_read_list_lock",NULL);
	usb_port.read_tax_name.reading_list_lock = _lock_open("reading_list_lock",NULL);
	usb_port.data_lock = _lock_open("data_lock",NULL);

	usb_port.read_tax_name.ca_read_lock = _lock_open("ca_read_lock",NULL);


	usb_port.fun_get_machine_type = get_machine_type;  //��ȡ�����ͺ�
	usb_port.fun_check_with_robot_arm = check_with_robot_arm;
	usb_port.module = module;
	usb_port.app_note = app_note;
	//���ݻ�����ţ�������ʲô�ͺŵĻ���
	get_machine_type(NULL);

	if(usb_port.read_tax_name.need_read_list_lock < 0 || usb_port.read_tax_name.reading_list_lock < 0)
	{
		return USB_SYS_ERROR;
	}

	usb_port.online_bank_action.mechanical_arm_action.class = get_robot_arm_class();
	usb_port.online_bank_action.mechanical_arm_action.arm_start = start_robot_arm;
	usb_port.online_bank_action.mechanical_arm_action.arm_update = update_robot_arm;

	usb_port.read_tax_name.need_read_tax_name_list_head = NULL;
	usb_port.read_tax_name.reading_tax_name_list_head = NULL;
	usb_port.timer_fd=timer_open();

	//usb_port.online_bank_action.mechanical_arm_action.class->fun_port_state_buff_init(usb_port.get_port_status.port_status_buff);
	
	usb_port.load_action.fun_load_start = load_start;
	// printf("usb_port.load_action.fun_load_start = %x,load_start=%x,usb_port=%x\n",usb_port.load_action.fun_load_start,load_start,&usb_port.load_action);
	usb_port.load_action.fun_load_stop = load_stop;

	usb_port.power_action.fun_power = fun_deal_usb_port_power;

	usb_port.key_auth_state.upload_auth_cert_file = fun_upload_auth_cert_file;
	usb_port.key_auth_state.get_auth_cert_file = fun_query_auth_file;

	usb_port.fun_check_port_status = check_port_status;
	usb_port.fun_check_port_with_num = check_port_with_num;

	while(!module->ty_usb_fd)
	{
		usb_port_out("�ȴ�ty_usb׼������\n");
		usleep(1*1000*1000);
	}

	while(1)
	{
		result = ty_ctl(module->ty_usb_fd, DF_TU_USB_CM_PORT_GET_KEY_VALUE, usb_port.key_auth_state.key_s);
		if(result < 0)
		{
			try_count++;
			usb_port_out("����Ƭ����Կʧ��\n");
			if(try_count > 5)
			{
				break;
			}
		}
		else
		{
			break;
		}
		logout(INFO, "SYSTEM", "��Կ��ȡ", "��Ƭ����Կ��ȡʧ��,�������¶�ȡ\r\n");
		usleep(1*1000*1000);
	}
	usb_port.key_auth_state.keyfile_load_err = result;
	get_cert_json_file(usb_port.key_auth_state.key_s,&usb_port.key_auth_state.cert_json);
	check_port_auth_info(&usb_port.key_auth_state);

	//usb_port.report_action.report_data.last_report_time_o = get_time_sec();//������
	usb_port.report_action.report_data.last_report_time_h = get_time_sec();//��̨����
	usb_port.report_action.report_data.last_report_time_s = get_time_sec();//������Ϣ
	
	usb_port.usb_port_numb = ty_ctl(module->ty_usb_fd, DF_TY_USB_CM_PORT_NUMBER);
	printf("usb_port.usb_port_numb = %d\n",usb_port.usb_port_numb);

	power_saving_init(module,usb_port.machine_type,usb_port.usb_port_numb);

	for(i = 0;i<usb_port.usb_port_numb;++i)  //��Դ���Ʒ�ʽ��ʼ��
	{
		usb_port.get_dev_basic_action.usb_dev_basic_data[i].port = i + 1;
		usb_port.get_dev_basic_action.usb_dev_basic_data[i].port_info.port = i + 1;
		usb_port.online_bank_action.mechanical_arm_action.arm_data[i].port = i + 1;
		usb_port.online_bank_action.mechanical_arm_action.arm_data[i].port_info.port = i + 1;
		if(usb_port.machine_type == TYPE_2303)
		{
			usb_port.power_action.power_data[i].power_control_type = 1;
		}
		else
		{
			usb_port.power_action.power_data[i].power_control_type = 0;
		}
	}

	

	//��е��ģ���ʼ��
	//if(usb_port.machine_type == 1)
	{
		robot_arm_model_init(module,usb_port.usb_port_numb);
		usb_port.online_bank_action.mechanical_arm_action.class = get_robot_arm_class();
	}
	
	

	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port.usb_port_numb = usb_port.usb_port_numb/2;
	}
	
	//�Բ�ģ��
	test_model_init(usb_port.usb_port_numb);

	if(1 != (*((int *)check_with_robot_arm(NULL))))  //���Ǵ���е�۵Ļ���
	{
		usb_port_out("�������ж˿�\n");
		for(i = 0;i<usb_port.usb_port_numb;++i)
		{
			result = power_saving_contrl_port(i,OPEN_STATE,PORT_CONFPRM_PRIORITY,usb_port.power_action.power_data[i].power_control_type);
			if(result != USB_PORT_NO_ERROR)
			{
				usb_port_out("�򿪶˿�=%d,result=%d\n",i,result);
			}
			result = power_saving_contrl_port(i,CLOSE_STATE,PORT_CONFPRM_PRIORITY,usb_port.power_action.power_data[i].power_control_type);
			if(result != USB_PORT_NO_ERROR)
			{
				usb_port_out("�رն˿�=%d,result=%d\n",i,result);
			}
		}
	}

	//�����������
	//1��ִ��get_port_status����ȡ�˿�״̬�������򵥴���
	usb_port.task_fd.get_port_status_fd = _scheduled_task_open("get_port_status",get_port_status_thread,NULL,100,NULL);
	//2���˿ڼ���̣߳�����get_port_status������process_step�������̻߳����ļ�ϵͳ�е�usb�豸�����������get_port_status��һ�£��򰴷ǽڵ紦��
	usb_port.task_fd.usbport_exist_recheck_fd = _scheduled_task_open("usbport_exist_recheck",usbport_exist_recheck_thread,NULL,100,NULL);
	//3���˿ڼ�����߳�
	usb_port.task_fd.port_check_fd = _scheduled_task_open("port_check",port_check_thread,NULL,50,NULL);
	//4��������˰���������ȡ���У�������Ƴ������У�����ȴ�
	usb_port.task_fd.read_tax_name_fd = _scheduled_task_open("read_tax_name",read_tax_name_thread,NULL,100,NULL);
	//5���ư��������
	usb_port.task_fd.led_manage_fd = _scheduled_task_open("port_led_manage",port_led_manage_thread,NULL,200,NULL);
	//6�������ϱ�����
	usb_port.task_fd.report_fd = _scheduled_task_open("report",report_thread,NULL,100,NULL);
	//7��������е�۲�������һ����ѯһ��
	//if(usb_port.machine_type == 1)
		usb_port.task_fd.upgrade_strategy_fd = _scheduled_task_open("upgrade_strategy",upgrade_strategy_thread,NULL,1000,NULL);
	// //6����������
	// usb_port.task_fd.attach_fd = _scheduled_task_open("attach",attach_thread,NULL,100,NULL);

	
	// sleep(60);   //�򵥴���ȴ�һ���ӣ�ȷ�����л�е�۳�ʼ���ɹ�
	// get_test_model_calss()->fun_start_test();

	//������ȡһ�����л�е���ϵ��豸�������
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //����е�۵Ļ���
		get_arm_with_dev();

	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : get_port_status_thread
-   Description����ȡ�˿������豸״̬
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void get_arm_with_dev_thread(void *arg)
{
	int result = 0;
	ARM_WITH_DEV *arm_with_dev = (ARM_WITH_DEV *)arg;
	arm_with_dev->arm_with_dev_result = 0;
	if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[arm_with_dev->port].port_info.arm_version))
	{
		result = *((int *)(usb_port.online_bank_action.mechanical_arm_action.arm_start((void *)(&arm_with_dev->port))));
		printf("get_arm_with_dev_thread port = %d,result=%d\n",arm_with_dev->port,result);
		if(result == 0)
			arm_with_dev->arm_with_dev_result = 1;
		usb_port.online_bank_action.mechanical_arm_action.arm_data[arm_with_dev->port].up_report = 1;
		usb_port.online_bank_action.mechanical_arm_action.arm_data[arm_with_dev->port].last_act = 1;
		_scheduled_task_close_self(arm_with_dev->arm_with_dev_fd);
	}
	
}

int get_arm_with_dev(void)
{
	int i = 0;
	for(i = 0;i<usb_port.usb_port_numb;++i)
	{
		//if(0 == i%2)
		{
			usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[i].port = i;
			usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[i].arm_with_dev_fd = _scheduled_task_open("get_arm_with_dev",get_arm_with_dev_thread,(void *)(&(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[i])),100,NULL);
		}
	}
	return 0;
}

/*********************************************************************
-   Function : get_port_status_thread
-   Description����ȡ�˿������豸״̬
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int get_port_status_thread(void *arg, int o_timer)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,i = 0,usb_port_numb = 0;
	int power_state_judge = 0;
	//char port_status_buff[usb_port.usb_port_numb] = {0x00};
	int time_now = 0;
	enum USBPORT_EXIST_STATE tmp_state = WITHOUT_DEV;
	//usb_port_out("enter get_port_status_thread\n");
	//printf("usb_port.load_action.fun_load_start = %x,load_start=%x,usb_port=%x\n",usb_port.load_action.fun_load_start,load_start,&usb_port.load_action);
	// if(NULL == usb_port.get_port_status.fun_get_port_ad_status)
	// {
	// 	return USB_PORT_NULL_FUN;
	// }
	// memcpy(port_status_buff,usb_port.get_port_status.port_status_buff,sizeof(usb_port.get_port_status.port_status_buff));
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	time_now = get_time_sec();

	 result = *((int *)usb_port.get_port_status.fun_get_port_ad_status((void *)usb_port.get_port_status.port_status_buff));
	 //printf("get_port_status_thread result = %d\n",result);
	 if(USB_PORT_NO_ERROR == result) //fun_get_port_status�޴���
	 {
		//printf("usb_port.get_port_status.port_status_buff\n");
		for(port_num = 0;port_num<usb_port_numb;++port_num)
		{
			if(usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.vid == 0x1ea7 && usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.pid == 0x0064)
		    {
				continue;
			}
			if(IGNORE == get_ignore_judge_flag(port_num))
			{
				continue;
			}
			//  if(port_num == 1)
		    //  	printf("port = %d,old_state = %d,now_state = %d ,port_state_enum= %d,time_now=%ld,power_off_time=%ld,process_step=%d\n",port_num,usb_port.get_port_status.last_port_status_buff[port_num],usb_port.get_port_status.port_status_buff[port_num],(get_port_priority_usedstate(port_num))->port_state_enum,time_now,usb_port.power_action.power_data[port_num].power_off_time,usb_port.judge_usbport_exist.port_exist_state[port_num].process_step);
			//&& CLOSE_STATE == (get_port_priority_usedstate(port_num))->port_state_enum
			//
			if(usb_port.get_port_status.port_status_buff[port_num] != usb_port.get_port_status.last_port_status_buff[port_num] && CLOSE_STATE == (get_port_priority_usedstate(port_num))->port_state_enum &&(time_now>usb_port.power_action.power_data[port_num].power_off_time)&&(abs(time_now - usb_port.power_action.power_data[port_num].power_off_time) > 1))
			{
				printf("port = %d,old_state = %d,now_state = %d \n",port_num,usb_port.get_port_status.last_port_status_buff[port_num],usb_port.get_port_status.port_status_buff[port_num]);
				//usb_port.get_port_status.last_port_status_buff[port_num] = usb_port.get_port_status.port_status_buff[port_num];
				tmp_state = (enum USBPORT_EXIST_STATE)usb_port.get_port_status.port_status_buff[port_num];
				printf("usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = %d\n",usb_port.judge_usbport_exist.port_exist_state[port_num].process_step);
				if(WAIT_STEP == usb_port.judge_usbport_exist.port_exist_state[port_num].process_step)//�˶˿ڴ��ڿ���״̬����������ȷ�Ͻ׶Σ����ⵥƬ������
				{
					usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = START_STEP;
					usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time = get_time_sec();   //��ʼʱ��
					usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state = tmp_state;
				}
				else	//�˶˿ڴ��ڷǿ���״̬
				{
					if(CONFROM_STEP == usb_port.judge_usbport_exist.port_exist_state[port_num].process_step) //�˶˿��Ѵ���ȷ��״̬
					{
						if(tmp_state != usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state) //֮ǰȷ�ϵ�״̬���ּ��״̬��һ�£���Ҫ����ȷ�ϴ���
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = START_STEP;
							usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state = tmp_state;
						}
					}
					else
					{
						if(tmp_state != usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state) //�ּ��״̬��֮ǰ��Ҫȷ�ϵ�Ŀ��״̬��һ��
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state = tmp_state;
						}
					}
				}
			}
		}
	}
	//usb_port_out("exit get_port_status_thread\n");
	return result;
}

/*********************************************************************
-   Function : usbport_exist_recheck_thread
-   Description���˿ڲ�����¼���߳�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int usbport_exist_recheck_thread(void *arg, int o_timer)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,time_now = 0,usb_port_numb = 0;
	int effective_port = 0;
	
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	//usb_port_out("enter usbport_exist_recheck_thread\n");
	for(port_num = 0;port_num<usb_port_numb;++port_num)
	{
		if(usb_port.machine_type)   //���û�е�۵Ļ���
			effective_port = port_num;
		else
			effective_port = port_num;
		if(WAIT_STEP != usb_port.judge_usbport_exist.port_exist_state[port_num].process_step)
		{
			switch(usb_port.judge_usbport_exist.port_exist_state[port_num].process_step)
			{
				case START_STEP:	//��ʼ����
					
					result = *((int *)(usb_port.judge_usbport_exist.judge_exist_fun.fun_start_step((void *)(&port_num))));
					printf("lbc start result = %d,port_num = %d\n",result,port_num);
					if(USB_PORT_NO_ERROR == result)
					{
						//printf("lbc start\n");
						usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time = get_time_sec();
						usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = RECHECK_STEP;
					}
					else
					{
						time_now = get_time_sec();
						if(time_now - usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time > 2)  //�ݶ�2��,2�뻹�ڿ�ʼ�׶�
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time = 0;
							usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
							usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state = WITHOUT_DEV;
						}
					}
				break;

				case RECHECK_STEP:	//���¼�鲽��
					
					time_now = get_time_sec();
					result = *((int *)(usb_port.judge_usbport_exist.judge_exist_fun.fun_recheck_step((void *)(&port_num))));
					// printf("lbc recheck = %d,result = %d\n",port_num,result);
					if(result == WITHOUT_DEV && WITH_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state) //ȷ��Ŀ�������豸��������豸�����ܴ����ļ�ϵͳ�л�δ������Ӧ�ļ�
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_recheck_failed_count = 0;
						usb_port.judge_usbport_exist.port_exist_state[port_num].with_dev_recheck_failed_count++;
						//if(usb_port.judge_usbport_exist.port_exist_state[port_num].with_dev_recheck_failed_count > 1000)  //ȷ�����豸
						if(time_now - usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time > DF_TY_USB_CHECK_WITH_DEV)  //�ݶ�5��
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].with_dev_recheck_failed_count = 0;
							if(WITHOUT_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state) //ȷ��Ŀ��������豸
							{
								usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = DEV_EXTRACT;
								usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
								usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = WITHOUT_DEV;
								usb_port.get_port_status.port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
								usb_port.get_port_status.last_port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
								printf("lbc new dev extract111 = %d\n",port_num);
							}
							else //ȷ��Ŀ��Ϊ���豸�����Ǽ�����豸
							{
								//printf("lbc test 222222 = %d\n",port_num);
								usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
							}
						}
					}
					else if(result == WITHOUT_DEV && WITHOUT_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].target_port_state)
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].with_dev_recheck_failed_count = 0;
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_recheck_failed_count++;
						//if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_recheck_failed_count>50)
						if(time_now - usb_port.judge_usbport_exist.port_exist_state[port_num].start_step_time > DF_TY_USB_CHECK_WITHOUT_DEV)  //�ݶ�1��
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_recheck_failed_count = 0;
							usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = DEV_EXTRACT;
							//usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = WITHOUT_DEV;
							usb_port.get_port_status.port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
							usb_port.get_port_status.last_port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
							usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
							printf("lbc new dev extract222 = %d\n",port_num);
						}
					}
					else if(result == WITH_DEV)
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_recheck_failed_count = 0;
						usb_port.judge_usbport_exist.port_exist_state[port_num].with_dev_recheck_failed_count = 0;
						if(usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state != WITH_DEV)
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = DEV_INSERT;
							
							usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = WITH_DEV;
							usb_port.get_port_status.port_status_buff[port_num] = (int)WITH_DEV; //��Ϊ���豸
							usb_port.get_port_status.last_port_status_buff[port_num] = (int)WITH_DEV; //��Ϊ���豸
							power_saving_contrl_port(port_num,OPEN_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
							printf("lbc new dev insert = %d\n",port_num);
						}
						usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
						usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
						power_saving_contrl_port(port_num,CLOSE_STATE,PORT_CONFPRM_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
						// usb_port.judge_usbport_exist.port_exist_state[port_num].recheck_result = (enum USBPORT_EXIST_STATE)result;
						// if(usb_port.judge_usbport_exist.port_exist_state[port_num].recheck_result != usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state)
						// {
						// 	if(WITHOUT_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state)  //�жϳ����豸���뻹���豸�γ�
						// 	{
						// 		usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = DEV_INSERT;
						// 		printf("lbc new dev insert\n");
						// 	}
						// 	else
						// 	{
						// 		usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = DEV_EXTRACT;
						// 		printf("lbc old dev extract\n");
						// 	}
						// 	usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = usb_port.judge_usbport_exist.port_exist_state[port_num].recheck_result;
						// }
						// usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
					}
					// result = *((int *)(usb_port.judge_usbport_exist.judge_exist_fun.fun_end_step((void *)(&port_num)))); //��������
					// if(USB_PORT_NO_ERROR == result)
					// {
					// 	usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = CONFROM_STEP;
					// }
				break;

				case CONFROM_STEP:  //ȷ�ϲ���
					//��ʱ��ִ��
				break;

				default:
				break;
			}
		}
	}
	//usb_port_out("exit usbport_exist_recheck_thread\n");
	return USB_PORT_NO_ERROR;
}

int check_attach(int port_num)
{
	int result = USB_PORT_NO_ERROR,usb_port_numb = 0;
	long time_now = 0;
	LOAD_DATA load_data;
	DEV_BASIC_DATA *state = NULL;
	//usb_port_out("enter check_attach\n");
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = port_num / 2;
	}
	else
	{
		usb_port_numb = port_num;
	}
	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[usb_port_numb];
	time_now = get_time_sec();
	
	if(IGNORE == get_ignore_judge_flag(port_num))
	{
		state->port_info.last_open_time = get_time_sec();
	}

	if(ATTACHED == usb_port.load_action.load_data[usb_port_numb].usbip_state)
	{
		//printf("!!!!!!!!!!!!!!!time_now = %d,state->port_info.last_open_time=%d,%d\r\n",time_now,state->port_info.last_open_time,time_now - state->port_info.last_open_time);
		if ((time_now - state->port_info.last_open_time) > DF_TY_USB_DATA_MAX_TIMER_S)
		{
			if(0 > ty_ctl(usb_port.module->tran_fd, DF_CHECK_PORT_LINE_STATUS, port_num + 1))
			{
				//usb_port_out("check_attach close_port_dettach\n");
				//close_port_dettach(state->port);
				load_data.port = port_num;
				usb_port.load_action.fun_load_stop((void *)(&load_data));
				return USB_PORT_NO_ERROR;
			}
		}
	}
	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : port_check_thread
-   Description���˿ڼ���߳�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int port_check_thread(void *arg, int o_timer)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,dev_port = 0,check_arm_version_flag = 0; //����е���Ƿ�����������־
	int usb_port_numb = 0,effect_port = 0;
	// static char first_flag = 1;
	// if(1 == first_flag)
	// {
	// 	usleep();
	// }
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}

	for(port_num = 0;port_num<usb_port_numb;++port_num)
	{
		// if(port_num == 31)
		// {
		// 	printf("port_state_enum=%d,process_step=%d,arm_version=%s\n",(get_port_priority_usedstate(port_num))->port_state_enum,usb_port.judge_usbport_exist.port_exist_state[port_num].process_step,usb_port.get_dev_basic_action.usb_dev_basic_data[(port_num-1)>0?(port_num-1):0].port_info.arm_version);
		// }

		if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
		{
			effect_port = port_num/2;
			if(strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num/2].port_info.arm_version) > 0)
				check_arm_version_flag = 1;
			else
				check_arm_version_flag = 0;
		}
		else
		{
			effect_port = port_num;
			check_arm_version_flag = 1;
		}
		if(1 != (*((int *)check_with_robot_arm(NULL))))  //���Ǵ���е�۵Ļ���
		{
			if(strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[effect_port].port_info.arm_version) > 0)
			{
				if(0 > my_judge_exit_dev_robotarm(port_num + 1,usb_port.app_note,1))   //��е���Ѿ�������  without_robot_arm_count
				{
					usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count++;
					if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count > 50) //ʧ�ܴ���
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count = 0;
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
						// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
						// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
						// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
						usb_port.power_action.power_data[port_num].power_control_type = 0;  //��е�ۿ��Ƶ�Դ�ķ�ʽ
						usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
						power_saving_contrl_port(port_num,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);

						printf("robot arm port = %d,pull out\n",port_num);
						memset(&usb_port.online_bank_action.mechanical_arm_action.arm_data[effect_port],0x00,sizeof(DEV_BASIC_DATA));  //��ջ�е��
						usb_port.online_bank_action.mechanical_arm_action.class->fun_dev_deinit(effect_port);
						ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_CLOSE_PORT_POWER,port_num + 1);//���ڷ����û�е�۵Ļ���ǿ��һ��ִ���µ����
						// power_saving_port_data_init(port_num);
						// if(0 < usb_port.read_tax_name.read_queue_num)
						// 	usb_port.read_tax_name.read_queue_num--;
					}
				}
				else
				{
					usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count = 0;
				}
			}
		}
		else
		{
			if(port_num % 2 == 0)  //�ǻ�е��
			{
				if(strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[effect_port].port_info.arm_version) > 0)
				{
					if(0 > my_judge_exit_dev_robotarm(port_num + 1,usb_port.app_note,1))   //��е���Ѿ�������  without_robot_arm_count
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count++;
						if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count > 50) //ʧ�ܴ���
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count = 0;
							usb_port.get_port_status.port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
							usb_port.get_port_status.last_port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
							usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = WAIT_STEP; //��Ϊ�ȴ�
							usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = WITHOUT_DEV; //��ǰ״̬��Ϊ���豸
							usb_port.read_tax_name.read_state[port_num] = WAIT_READ;
							power_saving_contrl_port(port_num,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
							dev_port = port_num + 1;
							usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&dev_port));
							printf("robot arm port = %d,pull out\n",port_num);
							memset(&usb_port.online_bank_action.mechanical_arm_action.arm_data[effect_port],0x00,sizeof(DEV_BASIC_DATA));  //��ջ�е��
							usb_port.online_bank_action.mechanical_arm_action.class->fun_dev_deinit(effect_port);
						}
					}
					else
					{
						usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count = 0;
					}
				}
			}
		}
	

		if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum && RECHECK_STEP != usb_port.judge_usbport_exist.port_exist_state[port_num].process_step && START_STEP != usb_port.judge_usbport_exist.port_exist_state[port_num].process_step && check_arm_version_flag == 1)  //�˿ڴ��ڴ�״̬����Ҫ�����豸���ڵļ��
		{
			//if(0 > ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port_num + 1)) //���豸
			    if(1 != (*((int *)check_with_robot_arm(NULL))))  //���Ǵ���е�۵Ļ���
				{
					//usb_port.judge_usbport_exist.port_exist_state[port_num].without_robot_arm_count = 0;
					if(strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num].port_info.arm_version) > 0)
					{
						if(0 > my_judge_exit_dev_robotarm(port_num + 1,usb_port.app_note,0))    //��е�۴��ڣ��ټ���豸�Ƿ����
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count++;
							if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 50)
							{
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								printf("lbc test 111111 port_num=%d\n",port_num);
								if(IGNORE == get_ignore_judge_flag(port_num))
								{
									printf("1111111get_ignore_judge_flag=%d\n",port_num);
									return (void *)(&result);
								}
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
								usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								
							}
						}
						else
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
						}
					}
					else
					{
						if(0 > my_judge_exit_dev(port_num + 1,usb_port.app_note))
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count++;
							if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 50)
							{
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								printf("lbc test 111222 port_num=%d\n",port_num);
								if(IGNORE == get_ignore_judge_flag(port_num))
								{
									printf("1111111get_ignore_judge_flag=%d\n",port_num);
									return (void *)(&result);
								}
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
								usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								
							}
						}
						else
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
						}
						
					}
			
				}
				else
				{
					if(0 > my_judge_exit_dev_robotarm(port_num + 1,usb_port.app_note,0))
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count++;
							if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 50)
							{
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								printf("lbc test 111333 port_num=%d\n",port_num);
								if(IGNORE == get_ignore_judge_flag(port_num))
								{
									printf("1111111get_ignore_judge_flag=%d\n",port_num);
									return (void *)(&result);
								}
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
								// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
								usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
								usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
								
							}
						}
						else
						{
							usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
						}
				}
			
			// else
			// {
			// 	if(0 > my_judge_exit_dev_robotarm(port_num + 1,usb_port.app_note,0))
			// 	{
			// 		usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count++;
			// 		if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 50)
			// 		{
			// 			usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 			printf("lbc test 111111 port_num=%d\n",port_num);
			// 			if(IGNORE == get_ignore_judge_flag(port_num))
			// 			{
			// 				printf("1111111get_ignore_judge_flag=%d\n",port_num);
			// 				return (void *)(&result);
			// 			}
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
			// 			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			// 			usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 		}
			// 	}
			// 	else
			// 	{
			// 		usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 	}
			// }



			// if(0 > my_get_port_exit(port_num + 1,usb_port.app_note))
			// {
			// 	usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count++;
			// 	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�ǻ�е�۵Ļ���
			// 	{
			// 		if(!usb_port.online_bank_action.mechanical_arm_action.class->fun_judge_robot_arm(port_num))
			// 		{
			// 			if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 500) //�˿ڴ��ڴ�״̬������ǻ�е�ۣ�ʱ��ӳ�Ϊ5s
			// 			{
			// 				printf("lbc test 111111\n");
			// 				usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			// 				usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 			}
			// 		}
			// 		else if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 30) //�˿ڴ��ڴ�״̬��500ms�����豸����Ϊ���豸�������豸�γ�����
			// 		{
			// 			printf("lbc test 222111 port_num=%d\n",port_num);
			// 			if(IGNORE == get_ignore_judge_flag(port_num))
			// 			{
			// 				printf("1111111get_ignore_judge_flag=%d\n",port_num);
			// 				return (void *)(&result);
			// 			}
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
			// 			//logout(INFO, "system", "�˿ڼ��", "��⵽�˿ڡ�%d�����γ�\r\n",port_num + 1);
			// 			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			// 			usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 		}
			// 	}
			// 	else
			// 	{
			// 		if(usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count > 30) //�˿ڴ��ڴ�״̬��500ms�����豸����Ϊ���豸�������豸�γ�����
			// 		{
			// 			printf("lbc test 111111 port_num=%d\n",port_num);
			// 			if(IGNORE == get_ignore_judge_flag(port_num))
			// 			{
			// 				printf("1111111get_ignore_judge_flag=%d\n",port_num);
			// 				return (void *)(&result);
			// 			}
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
			// 			usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
			// 			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			// 			usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// 		}
			// 	}
			// }
			// else //�൱���������
			// {
			// 	usb_port.judge_usbport_exist.port_exist_state[port_num].without_dev_count = 0;
			// }
		
		}
		if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
		{
			if(port_num%2 == 1)
				check_attach(port_num);
		}
		else
		{
			check_attach(port_num);
		}
		if(NO_ACTON == usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action) //û��ִ�ж�����������
		{
			continue;
		}
		else if(DEV_INSERT == usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action) //���µ��豸����,׼����ȡ����˰��
		{
			logout(INFO, "system", "�˿ڼ��", "��⵽�˿ڡ�%d������\r\n",port_num + 1);
			usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = NO_ACTON;
			if(usb_port.read_tax_name.read_state[port_num] == WAIT_READ || usb_port.read_tax_name.read_state[port_num] == READ_FAILED) //δ��ȡ�������ϴζ�ȡʧ��
			{
				//printf("port = %d ,need to read\n",port_num);
				usb_port.read_tax_name.read_state[port_num] = NEED_READ; //��ȡ״̬��Ϊ��Ҫ��ȡ
				add_needed_read_list_node(port_num); //���˶˿���ӵ�����ȡ�б�
			}
		}
		else if(DEV_EXTRACT == usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action) //���豸�γ�������������ʹ�ã�������Դ���ͷ�
		{
			//logout(INFO, "system", "�˿ڼ��", "��⵽�˿ڡ�%d�����γ�\r\n",port_num + 1);
			usb_port.judge_usbport_exist.port_exist_state[port_num].usbport_action = NO_ACTON;
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.ca_read = 0;
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.fail_num = 0;
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].port_info.usb_app = 0;
			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].up_report = 0;
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].off_report = 1;
			// usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].last_act = 2;
		}
	}
	//usb_port_out("exit port_check_thread\n");
	return USB_PORT_NO_ERROR;
}


/*********************************************************************
-   Function : attach_thread
-   Description����������
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int attach_thread(void *arg, int o_timer)
{
	//task_m
	return USB_PORT_NO_ERROR;
}




/*********************************************************************
-   Function : report_thread
-   Description���ϱ�����,�������������ݴ���ϱ��������,
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
//��ʱ�ϱ��˿���Ϣ�߳�
// void tigger_report_port_status(void)
// {
// 	DEV_BASIC_DATA *state = NULL;
// 	int port = 0;
// 	if (usb_port.report_action.report_data.report_ter_flag == 0)
// 	{
// 		//usb_port_out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�_1\n");
// 		return;
// 	}
// 	for (port = 1; port <= usb_port.usb_port_numb; port++)
// 	{
// 		state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1];
// 		if ((state->off_report == 0) && (state->up_report == 0))
// 		{
// 			continue;
// 		}
// 		usleep(5000);
						
// 		if (((state->up_report == 1) && (state->port_info.usb_app == DEVICE_AISINO || state->port_info.usb_app == DEVICE_NISEC || state->port_info.usb_app == DEVICE_MENGBAI || state->port_info.usb_app == DEVICE_MENGBAI2 || state->port_info.usb_app >= 101)) || \
// 			((state->off_report == 1) && (state->port_last_info.usb_app == DEVICE_AISINO || state->port_last_info.usb_app == DEVICE_NISEC || state->port_last_info.usb_app == DEVICE_MENGBAI || state->port_last_info.usb_app == DEVICE_MENGBAI2 || state->port_last_info.usb_app >= 101)))
// 		{
// 			if (usb_port.report_action.report_data.report_p_used != 0)
// 			{
// 				//usb_port_out("�˿�״̬�ϴ���\n");
// 				continue;
// 			}
// 			usb_port.report_action.report_data.report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
// 			usb_port_out("��%d�Ŷ˿ڴ���һ��˰�̸��»������ϱ�\n", state->port);
// 			logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�˰�̸��»������ϱ�\r\n", port);
// 			_queue_add("0", pack_report_plate_info, state, 0);
// 			continue;
// 		}
// 		else if ((state->off_report == 1) && (state->port_last_info.usb_app == 0 || state->port_last_info.usb_app == 5))
// 		{
// 			if (usb_port.report_action.report_data.report_p_used != 0)
// 			{
// 				//usb_port_out("�˿�״̬�ϴ���\n");
// 				continue;
// 			}
// 			usb_port.report_action.report_data.report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
// 			usb_port_out("����һ������USB���»������ϱ�\n");
// 			logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�����USB���»������ϱ�\r\n", port);
// 			_queue_add("0", pack_report_other_info_new_mqtt, state, 0);
// 			continue;
// 		}
// 		else if (state->up_report == 1 || state->off_report == 1)
// 		{
// 			if (usb_port.report_action.report_data.report_p_used != 0)
// 			{
// 				//usb_port_out("�˿�״̬�ϴ��У�%d\n", port);
// 				continue;
// 			}
// 			if ((state->port_info.usb_app == 4))
// 			{
// 				usb_port_out("����һ��[%d]�˿�״̬CA���»������ϱ�--\n", port);
// 				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬CA���»������ϱ�\r\n", port);
// 				//state->up_report = 0;
// 				usb_port.report_action.report_data.report_p_used = 1;
// 				_queue_add("0", pack_report_ca_info_new_mqtt, state, 0);
// 			}
// 			else
// 			{
// 				usb_port_out("����һ��[%d]�˿�״̬�������»������ϱ�--\n", port);
// 				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬�������»������ϱ�\r\n", port);
// 				usb_port.report_action.report_data.report_p_used = 1;
// 				_queue_add("0", pack_report_other_info_new_mqtt, state, 0);
// 			}
// 			continue;
// 		}
// 	}
// }


int report_thread(void *arg, int o_timer)
{
	//usb_port_out("enter report_thread\n");
	int time_now = 0;
	struct _net_status net_status;
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	if (net_status.nettime_ok != 1)
	{
		// usb_port_out("��������δ����,�Ժ������ϱ��ն�״̬��net_status.nettime_ok=%d\n",net_status.nettime_ok);
		return USB_COMMON_ERROR;
	}
	// usb_port_out("report_thread 1111111111111111\n");
	time_now = get_time_sec();
	// usb_port_out("report_thread 2222222222222222\n");
	if (time_now - usb_port.report_action.report_data.last_report_time_o > DF_TY_USB_MQTT_REPORT_STATUS-1)//����15��ᴥ��һ���ϱ�
	{
		if (usb_port.report_action.report_data.report_ter_flag != 1)
		{
			if (usb_port.report_action.report_data.report_o_used == 0)
			{
				if (_delay_task_add("report_ter", pack_report_json_data, NULL, 0)<0)
				{
					usb_port_out("��������ʧ��\n");
				}
			}
		}
		//����Ѿ��ϱ�M������Ϣ�ɹ����ڽ����ϰ汾���������ϱ�,�����˴���Ŀ��Ϊ���ݸߵ�ֱ��MQTT�Խ�
	}
	if (time_now - usb_port.report_action.report_data.last_report_time_h > DF_TY_USB_MQTT_M_SERVER_HEART_REPORT-1)//����60��ᴥ��һ���ϱ�
	{
		if (usb_port.report_action.report_data.report_h_used == 0)
		{
			if (_delay_task_add("report_heat", pack_report_m_server_heart_data, NULL, 0)<0)
			{
				usb_port_out("��������ʧ��\n");
			}
		}
	}
	if (time_now - usb_port.report_action.report_data.last_report_time_s > DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT - 1)
	{
		if ((usb_port.report_action.report_data.report_s_used == 0) && (usb_port.report_action.report_data.report_ter_flag == 0))
		{
			//usb_port_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
			usb_port.report_action.report_data.report_s_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			if (_delay_task_add("report_M", pack_report_m_server_status_data, NULL, 0)<0)
			{
				usb_port_out("��������ʧ��\n");
			}
		}
	}
	if (time_now - usb_port.report_action.report_data.last_report_time_s_report > DF_TY_USB_MQTT_M_SERVER_STATUS_REPORT - 1)
	{
		if ((usb_port.report_action.report_data.report_s_used == 0) && (usb_port.report_action.report_data.report_ter_flag_report == 0))
		{
			//usb_port_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
			usb_port.report_action.report_data.report_s_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			if (_delay_task_add("report_M_M", pack_report_m_server_status_data_report, NULL, 0)<0)
			{
				usb_port_out("��������ʧ��\n");
			}
		}
	}
	tigger_report_port_status();
	//usb_port_out("exit report_thread\n");
	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : read_tax_name_thread
-   Description��������˰�����񣬲��ö�����ʽ����Ҫ��ȡ����˰�ŵĶ��У���ȡ��ɺ�Ӷ����Ƴ�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int read_tax_name_thread(void *arg, int o_timer)
{
	int result = USB_PORT_NO_ERROR;
	char name[64] = {0x00};
	//usb_port_out("!!!!!!!!!!!!!!!!!!!!enter read_tax_name_thread\n");

	if(1 != usb_port.read_tax_name.first_start)
	{
		sleep(20);
		usb_port.read_tax_name.first_start = 1;
		return USB_PORT_NO_ERROR;
	}

	if(NULL != usb_port.read_tax_name.need_read_tax_name_list_head) //����Ҫ��ȡ����˰�ŵĶ˿�
	{
		//printf("!!!!!read_tax_name_thread  port = %d,read_state = %d\n",usb_port.read_tax_name.need_read_tax_name_list_head->port_num,usb_port.read_tax_name.read_state[usb_port.read_tax_name.need_read_tax_name_list_head->port_num]);
		//if(NEED_READ == usb_port.read_tax_name.read_state[usb_port.read_tax_name.need_read_tax_name_list_head->port_num]) //�˽ڵ��ȡ״̬Ϊ��Ҫ��ȡ
		{
			if(READ_QUENE_MAX_NUM > usb_port.read_tax_name.read_queue_num) //δ�������ͬʱ��ȡ�����ֵ
			{
				add_reading_list_node(usb_port.read_tax_name.need_read_tax_name_list_head->port_num); //�Ӵ���ȡ�б���ӵ����ڶ�ȡ�б�
				
				memset(name,0x00,sizeof(name));
				sprintf(name,"read_tax_name_%d",usb_port.read_tax_name.need_read_tax_name_list_head->port_num);
				
				if(NULL != get_reading_list_node_with_portnum(usb_port.read_tax_name.need_read_tax_name_list_head->port_num))
				{
					result = _delay_task_add(name,deal_read_plate_base_tax_info,(void *)(&(get_reading_list_node_with_portnum(usb_port.read_tax_name.need_read_tax_name_list_head->port_num)->port_num)),100);
					if(result < 0)
					{
						usb_port_out("������ȡ����˰������ʧ��\n");
					}
					else
					{
						delete_needed_read_list_node_with_portnum(usb_port.read_tax_name.need_read_tax_name_list_head->port_num); //�Ӵ���ȡ�б���ɾ��
						//usb_port.read_tax_name.read_state[usb_port.read_tax_name.need_read_tax_name_list_head->port_num] = READING;
						usb_port.read_tax_name.read_queue_num++;
					}
				}
			}
			else
			{
				printf("!!!!!!!!!!!!!!!!!read_tax_name_thread usb_port.read_tax_name.read_queue_num = %d\n",usb_port.read_tax_name.read_queue_num);
			}
		}
	}
	//usb_port_out("exit read_tax_name_thread\n");
	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : add_needed_read_list_node
-   Description������ȡ�б�����һ���ڵ�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int add_needed_read_list_node(int port_num)
{
	READ_TAX_NAME_LIST *node = usb_port.read_tax_name.need_read_tax_name_list_head;
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	_lock_set(usb_port.read_tax_name.need_read_list_lock);
	if(NULL == usb_port.read_tax_name.need_read_tax_name_list_head) //��һ���ڵ�
	{
		usb_port.read_tax_name.need_read_tax_name_list_head = (READ_TAX_NAME_LIST *)malloc(sizeof(READ_TAX_NAME_LIST));
		if(NULL == usb_port.read_tax_name.need_read_tax_name_list_head)
		{
			_lock_un(usb_port.read_tax_name.need_read_list_lock);
			return USB_SYS_ERROR;
		}
		usb_port.read_tax_name.need_read_tax_name_list_head->next = NULL;
		usb_port.read_tax_name.need_read_tax_name_list_head->port_num = port_num;
		usb_port.read_tax_name.need_read_tax_name_list_head->read_state = NEED_READ;
	}
	else
	{
		while(NULL != node->next)
		{
			node = node->next;
		}
		node->next = (READ_TAX_NAME_LIST *)malloc(sizeof(READ_TAX_NAME_LIST));
		if(NULL == node->next)
		{
			_lock_un(usb_port.read_tax_name.need_read_list_lock);
			return USB_SYS_ERROR;
		}
		node = node->next;
		node->next = NULL;
		node->port_num = port_num;
		node->read_state = NEED_READ;
	}
	_lock_un(usb_port.read_tax_name.need_read_list_lock);
	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : delete_needed_read_list_node_with_portnum
-   Description������ȡ�б�ɾ����port_num��Ӧ�Ľڵ�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int delete_needed_read_list_node_with_portnum(int port_num)
{
	READ_TAX_NAME_LIST *node = usb_port.read_tax_name.need_read_tax_name_list_head;
	READ_TAX_NAME_LIST *prev_node = NULL; //��¼��һ���ڵ�
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	if(NULL == node)
	{
		return USB_SYS_ERROR;
	}
	_lock_set(usb_port.read_tax_name.need_read_list_lock);
	while(NULL != node)
	{
		//printf("node->port_num=%d,port_num=%d\n",node->port_num,port_num);
		if(node->port_num == port_num)
		{
			break;
		}
		prev_node = node;
		node = node->next;
	}
	if(NULL == node)
	{
		_lock_un(usb_port.read_tax_name.need_read_list_lock);
		return USB_PARAMETER_NO_MATCH;
	}
	if(node == usb_port.read_tax_name.need_read_tax_name_list_head) //�׽ڵ�
	{
		usb_port.read_tax_name.need_read_tax_name_list_head = usb_port.read_tax_name.need_read_tax_name_list_head->next;
		//printf("free(node) 1\n");
		free(node);
		//printf("free(node) 3\n");
	}
	else
	{
		prev_node->next = node->next;
		//printf("free(node) 2\n");
		free(node);
	}
	_lock_un(usb_port.read_tax_name.need_read_list_lock);
	return USB_PORT_NO_ERROR;
}

/*********************************************************************
-   Function : add_reading_list_node
-   Description�����ڶ�ȡ�б�����һ���ڵ�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int add_reading_list_node(int port_num)
{
	READ_TAX_NAME_LIST *node = usb_port.read_tax_name.reading_tax_name_list_head;
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	_lock_set(usb_port.read_tax_name.reading_list_lock);
	if(NULL == usb_port.read_tax_name.reading_tax_name_list_head) //��һ���ڵ�
	{
		usb_port.read_tax_name.reading_tax_name_list_head = (READ_TAX_NAME_LIST *)malloc(sizeof(READ_TAX_NAME_LIST));
		if(NULL == usb_port.read_tax_name.reading_tax_name_list_head)
		{
			_lock_un(usb_port.read_tax_name.reading_list_lock);
			return USB_SYS_ERROR;
		}
		usb_port.read_tax_name.reading_tax_name_list_head->next = NULL;
		usb_port.read_tax_name.reading_tax_name_list_head->port_num = port_num;
		usb_port.read_tax_name.reading_tax_name_list_head->read_state = NEED_READ;
	}
	else
	{
		while(NULL != node->next)
		{
			node = node->next;
		}
		node->next = (READ_TAX_NAME_LIST *)malloc(sizeof(READ_TAX_NAME_LIST));
		if(NULL == node->next)
		{
			_lock_un(usb_port.read_tax_name.reading_list_lock);
			return USB_SYS_ERROR;
		}
		node = node->next;
		node->next = NULL;
		node->port_num = port_num;
		node->read_state = NEED_READ;
	}
	_lock_un(usb_port.read_tax_name.reading_list_lock);
	return USB_PORT_NO_ERROR;
}

READ_TAX_NAME_LIST *get_reading_list_node_with_portnum(int port_num)
{
	READ_TAX_NAME_LIST *node = usb_port.read_tax_name.reading_tax_name_list_head;
	_lock_set(usb_port.read_tax_name.reading_list_lock);
	while(NULL != node)
	{
		//printf("get_reading_list_node_with_portnum node->port_num=%d,port_num=%d\n",node->port_num,port_num);
		if(node->port_num == port_num)
		{
			break;
		}
		node = node->next;
	}
	_lock_un(usb_port.read_tax_name.reading_list_lock);
	return node;
}

/*********************************************************************
-   Function : delete_reading_list_node_with_portnum
-   Description�����ڶ�ȡ�б�ɾ����port_num��Ӧ�Ľڵ�
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
int delete_reading_list_node_with_portnum(int port_num)
{
	READ_TAX_NAME_LIST *node = usb_port.read_tax_name.reading_tax_name_list_head;
	READ_TAX_NAME_LIST *prev_node = NULL; //��¼��һ���ڵ�
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	if(NULL == node)
	{
		return USB_SYS_ERROR;
	}
	_lock_set(usb_port.read_tax_name.reading_list_lock);
	while(NULL != node)
	{
		if(node->port_num == port_num)
		{
			break;
		}
		prev_node = node;
		node = node->next;
	}
	if(NULL == node)
	{
		_lock_un(usb_port.read_tax_name.reading_list_lock);
		return USB_PARAMETER_NO_MATCH;
	}
	if(node == usb_port.read_tax_name.reading_tax_name_list_head) //�׽ڵ�
	{
		usb_port.read_tax_name.reading_tax_name_list_head = usb_port.read_tax_name.reading_tax_name_list_head->next;
		free(node);
	}
	else
	{
		prev_node->next = node->next;
		free(node);
	}
	_lock_un(usb_port.read_tax_name.reading_list_lock);
	return USB_PORT_NO_ERROR;
}

static int usbport_socket_read(int sock, uint8 *buf, int buf_len)
{
	int i, result;
	for (i = 0; i<buf_len; i += result)
	{
		result = read(sock, buf + i, buf_len - i);
		if (result <= 0)
		{
			tran_data_out("usbport_socket_read read data time tran_data_out or err \n");
			return result;
		}
	}
	return i;
}

static int base_3des_data(uint8 *in_data,uint8 *out_data,int src_len,unsigned char *key_s)
{	
uint8 key[16];
uint8 buf[1024];
uint8 in_buf[1024];
int  base_len;
int  buf_len;
int i;
int j;
int m;
memset(key,0,sizeof(key));
sprintf((char *)key,"Liu+Zhou%s",key_s);
//printf("key = %s\n",key);
base_len =src_len;
memset(in_buf,0,sizeof(in_buf));
memcpy(in_buf,in_data,base_len);
//printf("base64 data len%d\n",base_len);
//decode_string(in_data,buf);
//base64_dec(buf,in_data,base_len);
//printf("get dest \ndata\n");
//for(i=0;i<base_len;i++)
//{
//	printf("%02x ",in_buf[i]);
//}
//printf("\n");
memset(buf,0,sizeof(buf));
for(i=0,j=0;i<=base_len;i=i+4,j=j+3)
{
	decode((char *)in_buf+i,4,(char *)buf+j);
	/*for(m=0;m<4;m++)
	{
	printf("%02x ",(unsigned char)in_buf[i+m]);
	}
	printf("\n");
	printf("3des:");
	for(n=0;n<3;n++)
	{
	printf("%02x ",(unsigned char)buf[j+n]);
	}
	printf("\n");*/
}
//printf("ȥ��base64��λ\n");

buf_len=(base_len/4)*3;
int yu=buf_len%8;
buf_len = buf_len -yu;	
//printf("base64 len %d\ndata\n",buf_len);
//for(i=0;i<buf_len;i++)
//{
//	printf("%02X ",buf[i]);
//}
//printf("\n");
//

des3_out(buf,buf_len,key);
//printf("�����Ĳ�1\n");
for(m=0;m<8;m++)
{
	if(buf[buf_len-1-m]==0x01)
		buf[buf_len-1-m]=0;
	else
		break;
}
//printf("end \ndata\n");
//for(i=0;i<buf_len;i++)
//{
//	printf("%02X ",buf[i]);
//}
//printf("\n");
//memset(out_data,0,sizeof(out_data));
memcpy(out_data,buf,buf_len);
return buf_len;	
}
//����������˰��Ӧ��JSON����
static int analysis_json_data(char *inbuf,char *key,int *valueInt,char *valueString)
{	
	cJSON *item;  
	cJSON *root=cJSON_Parse(inbuf);  
	if (!root)  
	{   //ty_usb_m_out("Error before: [%s]\n",cJSON_GetErrorPtr());  
	return -1;  
	}  
	else  
	{	cJSON *object=cJSON_GetObjectItem(root,"data");  
	if(object==NULL)  
	{	printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
	cJSON_Delete(root);  
	return -1;  
	}    
	if(object!=NULL)  
	{	//ty_usb_m_out("��ȡЭ��汾\n");
		item=cJSON_GetObjectItem(object,key);  
		if(item!=NULL)  
		{	//ty_usb_m_out("cJSON_GetObjectItem: type=%d, string is %s, valuestring=%s\n",item->type,item->string,item->valuestring);
			if(item->type == 3)
			{
				*valueInt = item->valueint;
			}
			else if(item->type == 4)
			{
				memcpy(valueString,item->valuestring,strlen(item->valuestring));  
			}                
		}  
		else
		{
			cJSON_Delete(root);
			return -1;
		}
	}    
	cJSON_Delete(root); 
	return 0;
	}  
	return -1;
}

int unpack_ca_data(int fd, uint8 *out_buf, int *out_len, uint16 cmd)
{
	uint8 edit[2];
	uint8 len_buf[2];
	uint8 order[2];
	int buf_len;
	uint16 order_c;
	//usb_port_out("��ȡ�汾��Ϣ\n");
	memset(edit, 0, sizeof(edit));
	memset(len_buf, 0, sizeof(len_buf));
	memset(order, 0, sizeof(order));
	//memset(out_buf,0,sizeof(out_buf));
	if (usbport_socket_read(fd, edit, sizeof(edit))<0)
	{
		usb_port_out("��ȡ�汾��Ϣ����\n");
		return -1;
	}
	if ((edit[0] != 0x01) && (edit[1] != 0x10))
	{
		usb_port_out("��ȡ���İ汾��Ϣ����%02x %02x\n", edit[0], edit[1]);
		return -2;
	}
	//usb_port_out("�汾��Ϣ�ɹ� %02x %02x\n",edit[0],edit[1]);
	//usb_port_out("��ȡ������Ϣ\n");
	if (usbport_socket_read(fd, len_buf, sizeof(len_buf))<0)
	{
		usb_port_out("������Ϣ��ȡʧ��\n");
		return -1;
	}
	buf_len = len_buf[0] * 256 + len_buf[1];
	//usb_port_out("��֡����:%d\n",buf_len);
	if (buf_len<6)
	{
		usb_port_out("����С��6����\n");
		return -3;
	}
	if (buf_len - 6>(*out_len))
	{
		usb_port_out("���ȹ�����%d\n", buf_len - 6);
		return -4;
	}
	if (usbport_socket_read(fd, order, sizeof(order))<0)
	{
		usb_port_out("��ȡ������ʧ��\n");
		return -1;
	}
	order_c = order[0] * 256 + order[1];
	if (order_c != cmd)
	{	//usb_port_out("�����ֲ�һ�»�Ϊ�������\n");
		return -1;
	}
	if (usbport_socket_read(fd, out_buf, buf_len - 6)<0)
	{
		usb_port_out("��ȡ����ʧ��\n");
		return -1;
	}
	(*out_len) = (buf_len - 6);
	return (order[0] * 256 + order[1]);
}

int read_ukey_ca_name_1(DEV_BASIC_DATA   *state)
{
	int socketfd;
	uint8 inbuf[50];
	uint8 outbuf[2048];
	int out_len;
	//int i;
	char name_tmp[2048];
	char json_data[2048];
	char key[8];
	int valueInt;
	char valueString[500];
	int result;
	char so_name[200];
	memset(so_name, 0, sizeof(so_name));
	memset(name_tmp, 0, sizeof(name_tmp));
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));
	/*if (vid_pid_jude_plate(state->port_info.vid, state->port_info.pid)>0)
	return -1;*/
	sprintf(so_name, "/lib/lib%04x_%04x.so", state->port_info.vid, state->port_info.pid);
	if (access(so_name, 0) != 0)
	{
		usb_port_out("�Ҳ���ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
		return CA_READ_ERR_NOT_FOUND_SO;
	}
	usb_port_out("�ҵ�ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
	inbuf[0] = 0x01; inbuf[1] = 0x10;
	inbuf[2] = 0x00; inbuf[3] = 0x26;
	inbuf[4] = 0x05; inbuf[5] = 0x01;
	memcpy(inbuf + 6, state->port_info.busid, sizeof(state->port_info.busid));

	socketfd = socket_tcp_open_send_recv_timeout("127.0.0.1", 10003, inbuf, 38);
	if (socketfd < 0)
	{
		return -1;
	}
	out_len = sizeof(outbuf);
	usb_port_out("������ɣ��ȴ�����\n");
	result = unpack_ca_data(socketfd, outbuf, &out_len, 0x1501);
	if (result < 0)
	{
		//usb_port_out("��ȡ����ʧ�ܻ�ʱ\n");
		close(socketfd);
		//usb_port_out("�ڴ˴�����CA�����Ƿ�����\n");
		return -2;
	}
	usb_port_out("������CA������Ϣ����Ϊ%d,����Ϊ%s\n",out_len,outbuf);
	close(socketfd);
	memcpy(name_tmp, outbuf, out_len);
	memset(key, 0, sizeof(key));
	sprintf(key, "%04x%04x", state->port_info.vid, state->port_info.pid);
	memset(json_data, 0, sizeof(json_data));

	//������
	base_3des_data((uint8 *)name_tmp, (uint8 *)json_data, strlen(name_tmp), (unsigned char *)key);
	usb_port_out("MAIN ���ܺ������:\n%s\n",json_data);

	//_lock_set(state->lock);
	memset(valueString, 0, sizeof(valueString));

	//������
	result = analysis_json_data(json_data, "result", &valueInt, valueString);
	if (result <0)
	{
		//_lock_un(state->lock);
		usb_port_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��1\n");
		// state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -3;
	}
	usb_port_out("result = %d\n", valueInt);
	state->port_info.ca_ok = valueInt;

	//������
	result = analysis_json_data(json_data, "encording", &valueInt, valueString);
	if (result <0)
	{
		//_lock_un(state->lock);
		usb_port_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��2\n");
		// state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -4;
	}
	usb_port_out("encording = %d\n", valueInt);
	state->port_info.encording = valueInt;
	memset(valueString, 0, sizeof(valueString));

	//������
	result = analysis_json_data(json_data, "cert_num", &valueInt, valueString);
	if (result <0)
	{
		//_lock_un(state->lock);
		usb_port_out("read_ukey_ca_name_1 ��ȡִ�н��ʧ��3\n");
		// state->port_info.ca_read = 1;//��Ǵ˶˿���Ҫ���ж�CA֤����Ϣ����
		return -5;
	}
	usb_port_out("cert_num = %s\n", valueString);

	memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
	if (valueString[strlen(valueString) - 1] > 0x7A)//new change by whl 2021-02-25
	{
		valueString[strlen(valueString) - 1] = '\0';
	}
	memcpy(state->port_info.ca_serial, valueString, strlen(valueString));
	char name_tmp1[500];
	memset(name_tmp1, 0, sizeof(name_tmp1));
	if (state->port_info.encording == 1)
	{
		usb_port_out("utf8����\n");
		memset(valueString, 0, sizeof(valueString));

		//������
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		usb_port_out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		utf82gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	else if (state->port_info.encording == 0)
	{
		usb_port_out("unicode����\n");
		memset(valueString, 0, sizeof(valueString));

		//������
		analysis_json_data(json_data, "cert_name", &valueInt, valueString);
		usb_port_out("cert_name = %s\n",valueString);
		memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
		unicode2gbk((uint8 *)valueString, (uint8 *)name_tmp1);
	}
	memcpy(state->port_info.ca_name, name_tmp1, strlen(name_tmp1));
	//usb_port_out("ca_name = %s\n",name_tmp1);
	usb_port_out("ca_name = %s\n", state->port_info.ca_name);

	result = analysis_json_data(json_data,"ukey_type",&valueInt,valueString);
	if(result <0||valueInt==0)
	{	usb_port_out("��ȡukey����ʧ��\n");
		state->port_info.plate_type = 0;
	}
	else
	{	usb_port_out("ukey_type = %d\n",valueInt);
		state->port_info.plate_type = valueInt;
	}

	//������
	result = analysis_json_data(json_data, "extension", &valueInt, valueString);
	if (result <0)
	{
		usb_port_out("��ȡ�����̺�ʧ��\n");
		state->port_info.extension = 0;
	}
	else
	{
		usb_port_out("extension = %d\n", valueInt);
		state->port_info.extension = valueInt;
	}

	memset(valueString, 0, sizeof(valueString));
	
	//������
	result = analysis_json_data(json_data, "SN", &valueInt, valueString);
	if (result <0)
	{
		usb_port_out("��ȡ˰�̱��ʧ��\n");
		memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
	}
	else
	{
		usb_port_out("SN = %s\n", valueString);
		memcpy(state->port_info.sn, valueString, strlen(valueString));
	}
	//_lock_un(state->lock);
	return 0;
}

/*********************************************************************
-   Function : deal_read_plate_base_tax_info
-   Description����ȡ����˰�ź���
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void deal_read_plate_base_tax_info(void *arg)
{
	char tmp_busid[DF_TY_USB_BUSID] = {0x00};
	char so_name[128] = {0x00};
	uint16 tmp_vid = 0;
	uint16 tmp_pid = 0;
	int result = USB_PORT_NO_ERROR,usb_type = 0;
	int port_num = *((int *)arg);
	int effective_port = 0; //��Ч��ֵ�����������е�ۺ�usb�豸
	DEV_BASIC_DATA *state = NULL;
	struct _plate_infos plate_infos;
	int busnum = 0,devnum = 0,left_num = 0,usb_port_numb = 0;
	char sn[16] = {0x00};
	struct _careader_process careader_process;
	struct _usb_file_dev    dev;
	memset(&careader_process, 0, sizeof(struct _careader_process));
	memset(&dev,0x00,sizeof(struct _usb_file_dev));
	usb_port_out("enter deal_read_plate_base_tax_info port_num = %d!\n",port_num);
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num || usb_port_numb < port_num)
	{
		if(0 < usb_port.read_tax_name.read_queue_num)
			usb_port.read_tax_name.read_queue_num--;
		usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
		delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
		{
		if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
			power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
		}
		usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
		return;
	}
	set_ignore_judge_flag(port_num,IGNORE); //���ú��԰��豸���
	if(OPEN_STATE != (get_port_priority_usedstate(port_num))->port_state_enum)
	{
		usb_port_out("11111111111111111 port_num = %d,need to open power\n",port_num);
		result = power_saving_contrl_port(port_num,OPEN_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
	}
	// if(usb_port.machine_type != TYPE_2303)
	// {
	// 	sleep(8);								//�Ӹ�4����ʱ����ֹ��е���ϵ���  state->port_info.busid
	// }
	result = my_get_busid(port_num + 1,tmp_busid,usb_port.app_note);  
	usb_port_out("port_num = %d!��ȡ�豸busid = %s,result = %d\n",port_num,tmp_busid,result);
	if (result < 0)
	{
		usb_port_out("��ȡ�豸busid��Ϣʧ��\n");
		if(0 < usb_port.read_tax_name.read_queue_num)
			usb_port.read_tax_name.read_queue_num--;
		set_ignore_judge_flag(port_num,NO_IGNORE);
		usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
		delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
		if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
		{
			power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
		}
		usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
		set_ignore_judge_flag(port_num,NO_IGNORE);
		return;
	}
	result = _usb_file_get_infor(tmp_busid, &dev);
	if (result < 0)
	{
		usb_port_out("��ȡ�豸dev��Ϣʧ��\n");
		if(0 < usb_port.read_tax_name.read_queue_num)
			usb_port.read_tax_name.read_queue_num--;
		set_ignore_judge_flag(port_num,NO_IGNORE);
		usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
		delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
		if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
		{
			power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
		}
		usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
		set_ignore_judge_flag(port_num,NO_IGNORE);
		return;
	}
	if((dev.idVendor == 0x1ea7 && dev.idProduct == 0x0064))   //��ɻ�е�ۣ��ȴ�ȫ���˻����
	{
		sleep(4);
		usb_port_out("�ȴ���е�ۻ���\n");
	}
	if((dev.idVendor == ROBOT_ARM_HUB_VID_OLD && dev.idProduct == ROBOT_ARM_HUB_PID_OLD) || (dev.idVendor == ROBOT_ARM_HUB_VID_NEW && dev.idProduct == ROBOT_ARM_HUB_PID_NEW))   //�ж��ǻ�е�۵�hub
	{
		sleep(10);  //�ȴ�10�룬�Ȼ�е��ö�����
		memset(tmp_busid,0x00,sizeof(tmp_busid));
		result = my_get_busid(port_num + 1,tmp_busid,usb_port.app_note);  
		usb_port_out("��е�� port_num = %d!��ȡ�豸busid = %s,result = %d\n",port_num,tmp_busid,result);
		if (result < 0)
		{
			usb_port_out("��ȡ�豸busid��Ϣʧ��\n");
			if(0 < usb_port.read_tax_name.read_queue_num)
				usb_port.read_tax_name.read_queue_num--;
			set_ignore_judge_flag(port_num,NO_IGNORE);
			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
			if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
			{
				power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
			}
			usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
			set_ignore_judge_flag(port_num,NO_IGNORE);
			return;
		}
		memset(&dev,0x00,sizeof(struct _usb_file_dev));
		result = _usb_file_get_infor(tmp_busid, &dev);
		if (result < 0)
		{
			usb_port_out("��ȡ�豸dev��Ϣʧ��\n");
			if(0 < usb_port.read_tax_name.read_queue_num)
				usb_port.read_tax_name.read_queue_num--;
			set_ignore_judge_flag(port_num,NO_IGNORE);
			usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
			delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
			if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
			{
				power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.machine_type);
			}
			usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
			set_ignore_judge_flag(port_num,NO_IGNORE);
			return;
		}
	}
	usb_type = vid_pid_jude_plate(dev.idVendor, dev.idProduct);   //���жϲ��ϵ��豸���ǻ�е�ۻ���usb�豸
	if(1 == usb_port.machine_type)   //���û�е�۵Ļ���
			effective_port = port_num/2;
		else
			effective_port = port_num;
	printf("usb_port.machine_type = %d,port_num = %d!effective_port = %d,usb_type = %d\n",usb_port.machine_type,port_num,effective_port,usb_type);
	memcpy(usb_port.judge_usbport_exist.port_exist_state[port_num].busid,tmp_busid,strlen(tmp_busid));  //���busid����Ҫ���˿ڼ����
	if(usb_type == DEVICE_ROBOT_ARM_LOCAL)  //ʶ���ǻ�е��
	{
		state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[effective_port];
		memcpy(state->port_info.busid,tmp_busid,strlen(tmp_busid));
		// state->port = effective_port + 1;
		state->port_info.subclass = dev.interface[0].bInterfaceSubClass;
		state->port_info.dev_type = dev.interface[0].bInterfaceClass;
		state->port_info.protocol_ver = 2;
		state->port_info.vid = dev.idVendor;
		state->port_info.pid = dev.idProduct;
		state->port_info.bnuminterface = dev.bNumInterfaces;
		state->insert_time = get_time_sec();
		state->h_plate.nBusID = get_busid_busnum(state->port_info.busid);
		state->h_plate.nDevID = get_busid_devnum(state->port_info.busid);
		usb_port.power_action.power_data[port_num].power_control_type = 1;  //��е�ۿ��Ƶ�Դ�ķ�ʽ

		usb_port_out("�˿ڡ�%d�����ǻ�е��\n", state->port);
		state->port_info.usb_app = DEVICE_ROBOT_ARM_LOCAL;
		// state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		// state->last_act = 1;
		delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
		//��¼�˶˿ڵ�busid
		usb_port_out("robot arm port_num = %d@@@@@@@@@@@@@@@\n",effective_port);
		usb_port.online_bank_action.mechanical_arm_action.class->fun_set_busid(effective_port,state->port_info.busid);
		
		usb_port.read_tax_name.read_state[port_num] = READ_FAILED;
		usb_port.online_bank_action.mechanical_arm_action.class->fun_dev_init(effective_port,state->port_info.busid);
		usb_port_out("robot arm port_num = %d#################\n",effective_port);
		if(0 > usb_port.online_bank_action.mechanical_arm_action.class->fun_get_version(effective_port,state->port_info.arm_version))
		{
			usb_port_out("fun_get_version failed\n");
			usb_port.read_tax_name.read_state[port_num] = NEED_READ; //��ȡ״̬��Ϊ��Ҫ��ȡ
			add_needed_read_list_node(port_num); //���˶˿���ӵ�����ȡ�б�
		}
		else
		{
			usb_port_out("robot arm port_num = %d,arm version = %s\n",effective_port,state->port_info.arm_version);
			strcpy(usb_port.upgrade_strategy.upgrade_strategy_struct[(effective_port)<=(DF_TY_USB_PORT_N_MAX)?(effective_port):(DF_TY_USB_PORT_N_MAX)].old_version,state->port_info.arm_version);
			usb_port.online_bank_action.mechanical_arm_action.class->fun_set_power(effective_port,POWER_CLOES_STATE);
			usb_port_out("robot arm port_num = %d!!!!!!!!!!!!!!!!!!!!\n",effective_port);
			if(1 != usb_port.machine_type)   //�����û�е�۵Ļ���
			{
				result = usb_port.online_bank_action.mechanical_arm_action.class->fun_arm_action(effective_port);
				if(result == 0)
					result = 1;
				else if(result == -1)
					result = 0;
				else
					result = 0;
				usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[effective_port].arm_with_dev_result = result;  //״̬���µ�����

				usb_port.get_port_status.last_port_status_buff[effective_port] = 0;
				usb_port.judge_usbport_exist.port_exist_state[effective_port].process_step = WAIT_STEP;
				usb_port.judge_usbport_exist.port_exist_state[effective_port].now_port_state = WITHOUT_DEV;
				power_saving_port_data_init(port_num);
			}
		}
		if(0 < usb_port.read_tax_name.read_queue_num)
			usb_port.read_tax_name.read_queue_num--;
		
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
		
		set_ignore_judge_flag(port_num,NO_IGNORE);
		return;
	}
	//��ca�豸��ȡ
	else
	{
		state = &usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port];
		
		memcpy(state->port_info.busid,tmp_busid,strlen(tmp_busid));
		state->port = effective_port + 1;
		state->port_info.subclass = dev.interface[0].bInterfaceSubClass;
		state->port_info.dev_type = dev.interface[0].bInterfaceClass;
		state->port_info.protocol_ver = 2;
		state->port_info.vid = dev.idVendor;
		state->port_info.pid = dev.idProduct;
		state->port_info.bnuminterface = dev.bNumInterfaces;
		state->insert_time = get_time_sec();
		state->h_plate.nBusID = get_busid_busnum(state->port_info.busid);
		state->h_plate.nDevID = get_busid_devnum(state->port_info.busid);
		if((usb_type == DEVICE_AISINO_LOCAL) || (usb_type == DEVICE_NISEC_LOCAL) || (usb_type == DEVICE_MENGBAI_LOCAL) || (usb_type == DEVICE_MENGBAI2_LOCAL) || (usb_type == DEVICE_SKP_KEY_LOCAL) || (usb_type == DEVICE_CNTAX_LOCAL))
		{
			if(state->retry_forbidden) //������󣬽�ֹ��ȡ
			{
				state->port_info.usb_app = 5;
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				if(0 < usb_port.read_tax_name.read_queue_num)
					usb_port.read_tax_name.read_queue_num--;
				usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
				delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
				if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
				{
					power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
				}
				usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
				set_ignore_judge_flag(port_num,NO_IGNORE);
				return;
			}
			
			if(OPEN_STATE != (get_port_priority_usedstate(port_num))->port_state_enum)
			{
				if ((result = power_saving_contrl_port(port_num,OPEN_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type)))
				{
					usb_port_out("�˿�%d,�޷���ȡ֤������,�������%d\n",state->port,result);
					if(0 < usb_port.read_tax_name.read_queue_num)
						usb_port.read_tax_name.read_queue_num--;
					set_ignore_judge_flag(port_num,NO_IGNORE);
					return;
				}
			}
			memset(&plate_infos, 0, sizeof(struct _plate_infos));
			state->h_plate.usb_type = usb_type;
			memset(state->h_plate.busid, 0, sizeof(state->h_plate.busid));
			strcpy(state->h_plate.busid, state->port_info.busid);
			//printf("lbc----------------sn = %x\n",sn);
			result = function_common_get_basic_tax_info_local(state->port_info.busid, usb_type, (char*)plate_infos.plate_basic_info.ca_name, (char*)plate_infos.plate_basic_info.ca_number, sn, (int*)(&plate_infos.plate_basic_info.cert_passwd_right), &left_num);
			if (result < 0)
			{
				usb_port_out("��%d��˰�̶�ȡ����˰��ʧ��,left_num = %d\n", state->port,left_num);
				state->usb_err = 1;//��¼USB���ֹ�����
				if(0 < left_num)   //��Ϊ��������µĶ�ȡʧ�ܣ����ٽ��ж�ȡ
				{
					state->retry_forbidden = 1;
					state->port_info.usb_app = 5;
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					if(0 < usb_port.read_tax_name.read_queue_num)
						usb_port.read_tax_name.read_queue_num--;
					usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
					delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
					power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
					usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
					set_ignore_judge_flag(port_num,NO_IGNORE);
					return;
					//update_dev_passwd_err_port(state->port, left_num);
				}
				delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
				if ((result = power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type)))
				{
					usb_port_out("�˿�%d,�ر�ʧ��,�������%d\n",state->port,result);
				}
				usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
				usb_port.read_tax_name.read_state[port_num] = READ_FAILED;
				if(0 < usb_port.read_tax_name.read_queue_num)
					usb_port.read_tax_name.read_queue_num--;
				set_ignore_judge_flag(port_num,NO_IGNORE);
				state->port_info.fail_num++;
				if(3 < state->port_info.fail_num)   //ͬһ���˿�ʧ�����Σ����ٶ�ȡ
				{
					state->port_info.fail_num = 0;
					state->port_info.usb_app = 5;
					// state->port_info.ca_read = 1;   //�˶˿ڲ��ٶ�ȡ
					usb_port_out("˰�̶�ȡ����˰�Ŷ�ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", port_num + 1);
					state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
					state->last_act = 1;
					state->read_success_flag = 1;
				}
				else
				{
					usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
				}

				
				// usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
				// set_ignore_judge_flag(port_num,NO_IGNORE);
				return;
			}
			usb_port_out("effective_port = %d,��%d��˰�̶�ȡ����˰�ųɹ�\n", effective_port,state->port);
			//_lock_set(usb_port.data_lock);
			state->usb_err = 0;//�����ɹ����USB�����¼
			memset(state->port_info.ca_name, 0, sizeof(state->port_info.ca_name));
			memcpy(state->port_info.ca_name, plate_infos.plate_basic_info.ca_name, strlen((const char *)plate_infos.plate_basic_info.ca_name));
			memset(state->port_info.ca_serial, 0, sizeof(state->port_info.ca_serial));
			memcpy(state->port_info.ca_serial, plate_infos.plate_basic_info.ca_number, strlen((const char*)plate_infos.plate_basic_info.ca_number));
			memset(state->port_info.sn, 0, sizeof(state->port_info.sn));
			memcpy(state->port_info.sn, sn, strlen(sn));
			//usb_port_out("11111111\n");
			//state->port_info.usb_app = usb_type; 		//wang 2022��10��17�� Ĭ�϶���4����ca�ϱ�		
			state->nongye_report = result;				//wang 11/18
			state->port_info.usb_app = 4;
			
			state->port_info.ca_ok = plate_infos.plate_basic_info.result;
			state->port_info.extension = plate_infos.plate_basic_info.extension;
			state->port_info.offinv_num_exit = plate_infos.plate_tax_info.off_inv_num;
			memset(state->port_info.plate_time, 0, sizeof(state->port_info.plate_time));
			memcpy(state->port_info.plate_time, plate_infos.plate_tax_info.plate_time, strlen((char *)plate_infos.plate_tax_info.plate_time));
			sprintf(state->port_info.port_str,"���˿ںţ�%d,�̺ţ�%s,���ƣ�%s��",state->port,state->port_info.sn,state->port_info.ca_name);
			if (strlen((char *)state->port_info.plate_time) != 0)
			{
				//usb_port_out("222222\n");
				long plate_time = get_second_from_date_time_turn((char *)state->port_info.plate_time);
				//usb_port_out("3333333\n");
				long time_now = get_time_sec();
				usb_port_out("˰��ʱ���Ϊ%ld,��ǰϵͳʱ���Ϊ%ld\n", plate_time, time_now);
				state->port_info.plate_sys_time = plate_time - time_now;
				usb_port_out("˰���뵱ǰϵͳʱ���Ϊ%d\n", state->port_info.plate_sys_time);
			}
			usb_port.read_tax_name.read_state[port_num] = READ_COMPLATED;
			//usb_port_out("4444444\n");
			result = delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
			usb_port_out("delete_reading_list_node_with_portnum result = %d\n",result);
			if ((result = power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type)))
			{
				usb_port_out("�˿�%d,�ر�ʧ��,�������%d\n",state->port,result);
			}
			usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
			//usb_port_out("55555555 result = %d\n",result);
			//_lock_set(state->lock);
			//usb_port_out("%d�Ŷ˿ڵĽ�˰��ȫ��˰����Ϣ��ȡ���,���ѳ����忨�߳�\n", state->port);
			//��Ҫ�ϱ�����ʽ��Ҫ�޸�
			state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
			state->last_act = 1;
			state->read_success_flag = 1;
			logout(INFO, "system", "������Ϣ��ȡ", "%s,˰�ţ�%s,�ֻ��ţ�%d,˰�����ͣ�%d  tax_read:%d\r\n", state->port_info.port_str, state->port_info.ca_serial, state->port_info.extension, state->port_info.usb_app, state->port_info.tax_read);
			
			printf("effective_port = %d,vid = %.04x,pid = %.04x\n",effective_port,usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.vid,usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.pid);
			set_ignore_judge_flag(port_num,NO_IGNORE);
		}
		else
		{
#ifdef TEST_MODEL
			if(state->port_info.vid == TEST_DEV_VID && state->port_info.pid == TEST_DEV_PID)  //���Բ��豸
			{
				usb_port_out("�˿ڡ�%d�������Բ��豸�����رյ�Դ\n", state->port);
				get_test_model_calss()->fun_set_busid(port_num,state->port_info.busid);
				state->port_info.usb_app = 5;
				state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
				state->last_act = 1;
				//get_test_model_calss()->fun_dev_init(port_num,state->port_info.busid);
				if(0 < usb_port.read_tax_name.read_queue_num)
						usb_port.read_tax_name.read_queue_num--;
				set_ignore_judge_flag(port_num,NO_IGNORE);
				return;
			}
#endif
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			//��ȡ��˰�̵���˰�����ƺ�˰��
			if (atoi(careader_process.process_run) != 1)//CA����δ����
			{
					sleep(8); //usb_port_out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
					usb_port_out("�ȴ���������,�����������˰�ű�־,�´μ�����ȡ\n");
					set_ignore_judge_flag(port_num,NO_IGNORE);
					usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
					delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
			}
			else//�����Ѿ�����
			{
				// if(state->port_info.ca_read != 1)  //��־ca���ܷ������ȡ
				{
					sprintf(so_name, "/lib/lib%04x_%04x.so", state->port_info.vid, state->port_info.pid);
					if(access(so_name, 0) != 0)   //�Ҳ�����Ӧ�Ķ�̬��
					{
						usb_port_out("�Ҳ���ƥ��ca/˰�̿�:lib%04x_%04x.so!\n", state->port_info.vid, state->port_info.pid);
						state->port_info.fail_num = 0;
						state->port_info.usb_app = 5;
						state->port_info.ca_read = 1;   //�˶˿ڲ��ٶ�ȡ
						usb_port.read_tax_name.read_state[port_num] = READ_FAILED;
						usb_port_out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", port_num + 1);
						state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
						state->last_act = 1;
						state->read_success_flag = 1;
					}
					else
					{
						_lock_set(usb_port.read_tax_name.ca_read_lock);
						result = read_ukey_ca_name_1(state);
						_lock_un(usb_port.read_tax_name.ca_read_lock);
						if (result < 0)
						{
							usb_port_out("read_ukey_ca_name_1 falied\n");
							//state->port_info.usb_app = 5;

							state->port_info.fail_num++;
							if(result == CA_READ_ERR_NOT_FOUND_SO)    //û�м��ݹ����豸������ΪĬ�϶�ȡ����
							{
								state->read_success_flag = 1;
								state->port_info.fail_num = 4;
							}
							if(3 < state->port_info.fail_num)   //ͬһ���˿�ʧ�����Σ����ٶ�ȡ
							{
								state->port_info.fail_num = 0;
								state->port_info.usb_app = 5;
								state->port_info.ca_read = 1;   //�˶˿ڲ��ٶ�ȡ
								usb_port.read_tax_name.read_state[port_num] = READ_FAILED;
								usb_port_out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", port_num + 1);
								state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
								state->last_act = 1;

								state->read_success_flag = 1;
							}
							else
							{	
								usb_port_out("state->port_info.fail_num = %d\n",state->port_info.fail_num);
								set_ignore_judge_flag(port_num,NO_IGNORE);
								usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
								delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
								state->port_info.usb_app = 5;
								state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
								state->last_act = 1;
							}
						}
						else
						{
							usb_port_out("֤���ȡ�ɹ����ϱ���%d���˿�ca�豸\n", port_num + 1);
							state->port_info.usb_app = 4;	
							state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
							state->last_act = 1;
							// state->port_info.ca_read = 0;
							printf("state->port_info.ca_name = %s,state->port_info.ca_serial = %s\n",state->port_info.ca_name,state->port_info.ca_serial);
							memset(tmp_busid,0x00,sizeof(tmp_busid));
							memcpy(tmp_busid,state->port_info.busid,strlen(state->port_info.busid));
							tmp_vid = state->port_info.vid;
							tmp_pid = state->port_info.pid;
							
							state->port_info.usb_app = 4;
							state->port_info.vid = tmp_vid;
							state->port_info.pid = tmp_pid;
							memcpy(state->port_info.busid,tmp_busid,strlen(tmp_busid));
							usb_port.read_tax_name.read_state[port_num] = READ_COMPLATED;
							delete_reading_list_node_with_portnum(port_num); //�����ڶ�ȡ�б���ɾ��
							if(0 == strlen(state->port_info.ca_name) && 0 == strlen(state->port_info.ca_serial))  //û����Ч������˰�ţ����¶�ȡ
							{
								
								state->port_info.fail_num++;
								if(3 < state->port_info.fail_num)   //ͬһ���˿�ʧ�����Σ����ٶ�ȡ
								{
									state->port_info.fail_num = 0;
									state->port_info.usb_app = 5;
									state->port_info.ca_read = 1;   //�˶˿ڲ��ٶ�ȡ
									usb_port.read_tax_name.read_state[port_num] = READ_FAILED;
									usb_port_out("֤���ȡʧ�ܣ��ϱ���%d���˿�����usb�豸\n", port_num + 1);
									state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
									state->last_act = 1;
									state->read_success_flag = 1;
								}
								else
								{
									set_ignore_judge_flag(port_num,NO_IGNORE);
									usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
								}
							}
							else
							{
								state->port_info.fail_num = 0;
								state->read_success_flag = 1;
							}
						}
					}
				}
			}
			if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)
			{
				if ((result = power_saving_contrl_port(port_num,CLOSE_STATE,READ_COMPANY_TAX_PRIORITY,usb_port.power_action.power_data[effective_port].power_control_type)))
				{
					usb_port_out("�˿�%d,�ر�ʧ��,�������%d\n",state->port,result);
				}
			}
			set_ignore_judge_flag(port_num,NO_IGNORE);
		}
	}
	
	if(0 < usb_port.read_tax_name.read_queue_num)
		usb_port.read_tax_name.read_queue_num--;
	set_ignore_judge_flag(port_num,NO_IGNORE);
	return;
}

/*********************************************************************
-   Function : pack_report_m_server_heart_data
-   Description�������������
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_m_server_heart_data(void *arg)
{
	char *g_buf = NULL;
	struct _ter_info ter_info;

	//printf("enetr pack_report_m_server_heart_data\n");

	cJSON *json = cJSON_CreateObject();
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	
	usb_port.report_action.report_data.report_h_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
	cJSON_AddStringToObject(json, "ter_id",ter_info.ter_id);
	g_buf = cJSON_PrintUnformatted(json);
	//printf("pack_report_m_server_heart_data = %s\n",g_buf);
	_m_pub_no_answer("mb_equ_ter_heart_up", g_buf);
	usb_port.report_action.report_data.last_report_time_h = get_time_sec();
	usb_port.report_action.report_data.report_h_used = 0;
	free(g_buf);
	cJSON_Delete(json);


	// printf("pub_data.status = %d\n",my_get_mqtt_parm()->pub_data.status);

	if(1 != (my_get_mqtt_parm()->sub_data.cfgp.sub_connect_ok) || 4 == (my_get_mqtt_parm()->pub_data.status))    //mq�����˶Ͽ�����Ҫ�����ϱ�������Ϣ
	{
		usb_port.report_action.report_data.report_ter_flag = 0;
	}
	return ;
}

/*********************************************************************
-   Function : pack_report_json_data
-   Description����������ն���Ϣ
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_json_data(void *arg)
{
	char *g_buf = NULL;
	char tmp[50] = { 0 };
	int port_count = 0,sp_type = 0,usb_port_numb = 0;
	char source_topic[50] = {0x00};
	char sp_type_s[20] = {0};
	struct _net_status net_status;

	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	//printf("enter pack_report_json_data\n");

	cJSON *dir1, *dir2, *dir3, *dir4,*p_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	
	usb_port.report_action.report_data.report_o_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����

	memset(source_topic, 0, sizeof(source_topic));
	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", DF_MQTT_SYNC_REPORT_TER_STATUS);
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", "123456789");
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	cJSON_AddStringToObject(dir3, "ter_id", ter_info.ter_id);
	cJSON_AddStringToObject(dir3, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(dir3, "ter_mac", net_status.mac);
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", usb_port.usb_port_numb);
	cJSON_AddStringToObject(dir3, "usb_port_num", tmp);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", DF_TY_USB_MQTT_REPORT_STATUS);
	cJSON_AddStringToObject(dir3, "report_time", tmp);

	cJSON_AddItemToObject(dir3, "usb_port_info", dir4 = cJSON_CreateArray());
	for (port_count = 0; port_count < usb_port.usb_port_numb; port_count++)
	{
		sp_type = usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.usb_app;
		cJSON_AddItemToObject(dir4, "dira", p_layer = cJSON_CreateObject());
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", port_count + 1);
		cJSON_AddStringToObject(p_layer, "usb_port", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", (int)(get_port_priority_usedstate(port_count))->port_state_enum);
		cJSON_AddStringToObject(p_layer, "port_power", tmp);
		
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.judge_usbport_exist.port_exist_state[port_count].now_port_state);
		cJSON_AddStringToObject(p_layer, "dev_exists", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.dev_type);
		cJSON_AddStringToObject(p_layer, "usb_type", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.vid);
		cJSON_AddStringToObject(p_layer, "usb_vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.pid);
		cJSON_AddStringToObject(p_layer, "usb_pid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.ca_ok);
		cJSON_AddStringToObject(p_layer, "ca_support", tmp);
		if (usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.ca_ok != 0)
		{
			cJSON_AddStringToObject(p_layer, "ca_name", (char *)usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.ca_name);
			
			memset(tmp, 0, sizeof(tmp));
			memcpy(tmp, usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.ca_serial, sizeof(usb_port.get_dev_basic_action.usb_dev_basic_data[port_count].port_info.ca_serial));
			cJSON_AddStringToObject(p_layer, "ca_number", (char *)tmp);
			if (sp_type == DEVICE_AISINO)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "1");
			}
			else if (sp_type == DEVICE_NISEC)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "2");
			}
			else if (sp_type == DEVICE_MENGBAI)
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "3");
			}
			else if (sp_type == DEVICE_CNTAX)
			{
				sprintf(sp_type_s, "%d", sp_type);
				cJSON_AddStringToObject(p_layer, "ca_type", sp_type_s);
			}
			else
			{
				cJSON_AddStringToObject(p_layer, "ca_type", "0");
			}
		}

	}
	g_buf = cJSON_Print(json);
	str_replace(g_buf, "\\\\", "\\");
	_m_pub_no_answer("mb_fpkjxt_kpfw", g_buf);			//mqtt pub ����:mb_fpkjxt_kpfw
	usb_port.report_action.report_data.last_report_time_o = get_time_sec();
	usb_port.report_action.report_data.report_o_used = 0;
	free(g_buf);
	cJSON_Delete(json);
	return ;
}

int fun_get_ter_base_data_json(cJSON *data_layer)
{
	char tmp_s[100] = { 0 };
	int external_delay_us = 0;
	float external_delay_ms = 0.0;
	struct _ter_info ter_info;
	struct _net_status net_status;
	cJSON *dir4;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	memset(&net_status, 0, sizeof(struct _net_status));
	ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);

	cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
	// memset(tmp_s, 0, sizeof(tmp_s));
	// sprintf(tmp_s, "%d", usb_port.usb_port_numb/2);
	// cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
	cJSON_AddStringToObject(data_layer, "internet_ip", net_status.internet_ip);
	cJSON_AddStringToObject(data_layer, "longitude", net_status.longitude);
	cJSON_AddStringToObject(data_layer, "latitude", net_status.latitude);
	cJSON_AddStringToObject(data_layer, "area_info", net_status.area_info);

	memset(tmp_s,0x00,sizeof(tmp_s));
	sprintf(tmp_s,"%.02f ms",usb_port.intranet_delay_ms);
	cJSON_AddStringToObject(data_layer, "intranet_delay", tmp_s);
	memset(tmp_s,0x00,sizeof(tmp_s));

	external_delay_us = get_net_delay_external("103.27.4.61");
	
	external_delay_ms = external_delay_us;
	external_delay_ms = external_delay_ms/1000;
	if(external_delay_ms < 0)
		usb_port.external_delay_ms = abs(external_delay_ms);
	else
		usb_port.external_delay_ms = external_delay_ms;
	printf("lbc external_delay_us = %d,external_delay_ms = %f\n",external_delay_us,external_delay_ms);
	sprintf(tmp_s,"%.02f ms",usb_port.external_delay_ms);
	cJSON_AddStringToObject(data_layer, "external_delay", tmp_s);

	//lbc test
	//fun_get_ter_all_data_json_jude_need_port(data_layer, 1, 1);

	cJSON_AddStringToObject(data_layer, "dns", net_status.dns);
	cJSON_AddStringToObject(data_layer, "ip_type", net_status.ip_type);			
	cJSON_AddStringToObject(data_layer, "netmask", net_status.netmask);
	cJSON_AddStringToObject(data_layer, "gateway", net_status.gateway);

	cJSON_AddStringToObject(data_layer, "product_code", ter_info.product_code);
	cJSON_AddStringToObject(data_layer, "product_name", ter_info.product_name);
	cJSON_AddStringToObject(data_layer, "product_rename", ter_info.product_rename);
	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	cJSON_AddStringToObject(data_layer, "cpu_id", ter_info.cpu_id);
	cJSON_AddStringToObject(data_layer, "ter_ip", net_status.ip_address);
	cJSON_AddStringToObject(data_layer, "ter_mac", net_status.mac);
	memset(tmp_s, 0, sizeof(tmp_s));
	if(usb_port.machine_type == 1)
	{
		sprintf(tmp_s, "%d", usb_port.usb_port_numb);
		cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
		cJSON_AddStringToObject(data_layer, "robotarm_port_num", tmp_s);
	}
	else
	{
		sprintf(tmp_s, "%d", usb_port.usb_port_numb);
		cJSON_AddStringToObject(data_layer, "usb_port_num", tmp_s);
	}
	cJSON_AddStringToObject(data_layer, "ter_version", ter_info.ter_version);
	cJSON_AddStringToObject(data_layer, "ter_version_detail", ter_info.ter_version_detail);
	if (usb_port.key_auth_state.keyfile_load_err == 1)
		cJSON_AddStringToObject(data_layer, "key_file_load", "0");
	else
		cJSON_AddStringToObject(data_layer, "key_file_load", "1");
	cJSON_AddStringToObject(data_layer, "tax_so_load", ter_info.tax_so_load);
	cJSON_AddStringToObject(data_layer, "auth_file_load", ter_info.auth_file_load);
	cJSON_AddStringToObject(data_layer, "tax_so_ver", ter_info.tax_ver);
	cJSON_AddStringToObject(data_layer, "mbc_so_ver", ter_info.mbc_ver);
	cJSON_AddStringToObject(data_layer, "ter_type", ter_info.ter_type);
	cJSON_AddStringToObject(data_layer, "internet_ip", net_status.internet_ip);
	cJSON_AddStringToObject(data_layer, "longitude", net_status.longitude);
	cJSON_AddStringToObject(data_layer, "latitude", net_status.latitude);
	cJSON_AddStringToObject(data_layer, "area_info", net_status.area_info);
	cJSON_AddStringToObject(data_layer, "total_space", ter_info.total_space);
	cJSON_AddStringToObject(data_layer, "left_space", ter_info.left_space);
	cJSON_AddStringToObject(data_layer, "kernel_ver", ter_info.kernel_ver);


	memset(tmp_s, 0, sizeof(tmp_s));
	sprintf(tmp_s, "%ld", ter_info.boot_size);
	cJSON_AddStringToObject(data_layer, "boot_size", tmp_s);

	cJSON_AddStringToObject(data_layer, "short_circuit_ports", "");
	cJSON_AddStringToObject(data_layer, "open_circuit_ports", "");
	cJSON_AddStringToObject(data_layer, "ip_conflict", "0");


	cJSON_AddStringToObject(data_layer, "need_auth_aisino", "1");
	cJSON_AddStringToObject(data_layer, "need_auth_nisec", "1");

	cJSON_AddStringToObject(data_layer, "need_auth_cntax", "1");
	cJSON_AddStringToObject(data_layer, "need_auth_mengbai", "0");

	// char robotarm_with_dev_ports[500] = { 0 };
	// char robotarm_status[500] = { 0 };
	// int port_num = usb_port.usb_port_numb,port = 0;
	// int err_num = 0;
	// char without_dev_port_num[10] = { 0 };
	// char without_dev_ports[500] = { 0 };
	// if(usb_port.machine_type == 1)
	// {
	// 	port_num = port_num * 2;
	// }
	//usb_port_out("���ն�һ����%d��USB�˿�\n");
	// for (port = 1; port<= port_num; port++)
	// {
	// 	if(usb_port.machine_type == 1)
	// 	{
	// 		if(port%2 == 1)
	// 		{
	// 			if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[port/2].arm_with_dev_result == 1)
	// 			//if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[port - 1].arm_with_dev_result == 1)
	// 			{
	// 				sprintf(robotarm_with_dev_ports + strlen(robotarm_with_dev_ports), "1;");
	// 			}
	// 			else
	// 			{
	// 				sprintf(robotarm_with_dev_ports + strlen(robotarm_with_dev_ports), "0;");
	// 			}
	// 			if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port/2].port_info.arm_version))
	// 			{
	// 				sprintf(robotarm_status + strlen(robotarm_status),"0;");
	// 			}
	// 			else
	// 			{
	// 				sprintf(robotarm_status + strlen(robotarm_status),"1;");
	// 			}
	// 		}
	// 		else
	// 		{
	// 			if (usb_port.judge_usbport_exist.port_exist_state[port - 1].now_port_state != WITH_DEV)
	// 			{
	// 				//if(port%2 == 0)
	// 				{
	// 					//usb_port_out("%d�Ŷ˿�û���豸\n", port_count + 1);
	// 					sprintf(without_dev_ports + strlen(without_dev_ports), "%d;", port/2);
	// 					err_num = err_num + 1;
	// 				}
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (usb_port.judge_usbport_exist.port_exist_state[port - 1].now_port_state != WITH_DEV)
	// 		{
				
	// 			{
	// 				//usb_port_out("%d�Ŷ˿�û���豸\n", port_count + 1);
	// 				sprintf(without_dev_ports + strlen(without_dev_ports), "%d;", port);
	// 				err_num = err_num + 1;
	// 			}
	// 		}
	// 	}
		
	// }
	
	// if(usb_port.machine_type == 1)
	// {
	// 	cJSON_AddStringToObject(data_layer, "robotarm_with_dev_ports", robotarm_with_dev_ports);
	// 	cJSON_AddStringToObject(data_layer, "robotarm_status", robotarm_status);
	// }
	// sprintf(without_dev_port_num, "%d", err_num);
	// cJSON_AddStringToObject(data_layer, "without_dev_port_num", without_dev_port_num);
	// cJSON_AddStringToObject(data_layer, "without_dev_ports", without_dev_ports);

	cJSON_AddItemToObject(data_layer, "plate_function", dir4 = cJSON_CreateArray());

	return 0;
}

void pack_report_m_server_status_data_report(void *arg)
{
	char *g_buf = NULL;
	char source_topic[50] = { 0 };
	int result;
	char g_data[5000] = {0};
	char random[50] = {0};
	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	struct _ter_info ter_info;
	
	//printf("*********************************enetr pack_report_m_server_status_data***********************************\n");

	cJSON *json = cJSON_CreateObject();

	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	memset(source_topic, 0, sizeof(source_topic));
	get_radom_serial_number("zdsbztxx", random);

	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbztxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	fun_get_ter_base_data_json(dir3);

	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	
	//usb_port_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
	result = _m_pub_need_answer(random, "mb_equ_ter_status_sel", g_buf, g_data);
	usb_port.report_action.report_data.last_report_time_s_report = get_time_sec();
	if (result < 0)
	{		
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,��������Ӧ��ʱ\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,������Ӧ����ʧ��\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End ;
	}
	else
	{
		//logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,�ɹ�\r\n");
		usb_port.report_action.report_data.report_ter_flag_report = 1;
		// usb_port_out("�ϱ�M�����ն�״̬����,������Ӧ����ɹ�\n");
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	usb_port.report_action.report_data.report_s_used = 0;
	return ;
}


/*********************************************************************
-   Function : pack_report_m_server_status_data
-   Description���ϴ���Ϣ��m����
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_m_server_status_data(void *arg)
{
	char *g_buf = NULL;
	char source_topic[50] = { 0 };
	int result,i = 0;
	char g_data[5000] = {0};
	char random[50] = {0};
	cJSON *dir1, *dir2, *dir3;		//���ʹ��
	struct _ter_info ter_info;
	
	//printf("*********************************enetr pack_report_m_server_status_data***********************************\n");

	cJSON *json = cJSON_CreateObject();

	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	memset(source_topic, 0, sizeof(source_topic));
	get_radom_serial_number("zdsbztxx", random);

	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);

	cJSON_AddItemToObject(json, "head", dir1 = cJSON_CreateObject());
	cJSON_AddStringToObject(dir1, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(dir1, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(dir1, "cmd", "zdsbztxx");
	cJSON_AddStringToObject(dir1, "source_topic", source_topic);
	cJSON_AddStringToObject(dir1, "random", random);
	cJSON_AddStringToObject(dir1, "result", "s");
	cJSON_AddItemToObject(json, "data", dir2 = cJSON_CreateArray());

	cJSON_AddItemToObject(dir2, "dira", dir3 = cJSON_CreateObject());

	fun_get_ter_base_data_json(dir3);

	g_buf = cJSON_PrintUnformatted(json);
	str_replace(g_buf, "\\\\", "\\");
	
	//usb_port_out("����һ��M�����ն˻�����Ϣ�ϱ�\n");
	result = _m_pub_need_answer(random, "mb_equ_ter_status_up", g_buf, g_data);
	usb_port.report_action.report_data.last_report_time_s = get_time_sec();
	if (result < 0)
	{		
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,��������Ӧ��ʱ\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End;
	}
	else if (result == 0)
	{
		logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,������Ӧ����ʧ��\r\n");
		free(g_buf);
		cJSON_Delete(json);
		goto End ;
	}
	else
	{
		//logout(INFO, "SYSTEM", "M�������ͬ��", "ͬ���ն���Ϣ,�ɹ�\r\n");
		usb_port.report_action.report_data.report_ter_flag = 1;
		usb_port_out("�ϱ�M�����ն�״̬����,������Ӧ����ɹ�\n");
		for(i = 0;i<usb_port.usb_port_numb;++i)
		{
			if(1 == usb_port.machine_type)   //���û�е�۵Ļ���
			{
				//if(i%2 == 0)   //��е��
				{
					if(strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))
					{
						logout(INFO, "SYSTEM", "M�������ͬ��", "�˿�:%d,��Ҫ�����ϱ�\r\n",i);
						usb_port.online_bank_action.mechanical_arm_action.arm_data[i].up_report = 1;//�˿�״̬������Ҫ�ϱ�
						usb_port.online_bank_action.mechanical_arm_action.arm_data[i].last_act = 1;
					}
				}
				//else		  //usb�豸
				{
					if(WITH_DEV == get_port_exit(i))   //���豸
					{
						logout(INFO, "SYSTEM", "M�������ͬ��", "�˿�:%d,��Ҫ�����ϱ�\r\n",i);
						usb_port.get_dev_basic_action.usb_dev_basic_data[i].up_report = 1;//�˿�״̬������Ҫ�ϱ�
						usb_port.get_dev_basic_action.usb_dev_basic_data[i].last_act = 1;
					}
				}
			}
			else
			{
				if(WITH_DEV == get_port_exit(i))   //���豸
				{
					logout(INFO, "SYSTEM", "M�������ͬ��", "�˿�:%d,��Ҫ�����ϱ�\r\n",i);
					usb_port.get_dev_basic_action.usb_dev_basic_data[i].up_report = 1;//�˿�״̬������Ҫ�ϱ�
					usb_port.get_dev_basic_action.usb_dev_basic_data[i].last_act = 1;
				}
			}
		}
	}
	free(g_buf);
	cJSON_Delete(json);

End:
	usb_port.report_action.report_data.report_s_used = 0;
	return ;
}

/*********************************************************************
-   Function : tigger_report_port_status
-   Description���ϴ���Ϣ��m����
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void tigger_report_port_status(void)
{
	DEV_BASIC_DATA *state = NULL;
	DEV_BASIC_DATA *robot_arm_state = NULL;
	int port = 0;
	if (usb_port.report_action.report_data.report_ter_flag == 0)
	{
		//usb_port_out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�_1\n");
		return;
	}
	for (port = 1; port <= usb_port.usb_port_numb; port++)
	{
		if (usb_port.report_action.report_data.report_ter_flag == 0)
		{
			//usb_port_out("�ն�״̬δ�ϴ��ɹ������ж˿��ϱ�\n");  
			return;
		}
		state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1];
		robot_arm_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[port - 1];
		// if(port == 1)
		// {
		// 	printf("tigger_report_port_status state->off_report = %d,state->up_report = %d,robot_arm_state->off_report = %d,robot_arm_state->up_report = %d\n",state->off_report,state->up_report,robot_arm_state->off_report,robot_arm_state->up_report);
		// }
		if (((state->off_report == 0) && (state->up_report == 0))&&(robot_arm_state->off_report == 0 && robot_arm_state->up_report == 0))
		{
			continue;
		}
		// usb_port_out("1111111111111\n");
		usleep(5000);
		// if (robot_arm_state->up_report == 1 || robot_arm_state->off_report == 1)
		// {
		// 	//usb_port_out("usb_port.report_action.report_data.report_p_used : %d\n", usb_port.report_action.report_data.report_p_used);
		// 	if (usb_port.report_action.report_data.report_p_used != 0)
		// 	{
		// 		continue;
		// 	}
		// 	// usb_port_out("����һ��[%d]�˿�״̬�������»������ϱ�--\n", port);
		// 	logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ż�е��״̬���»������ϱ�\r\n", port);
		// 	usb_port.report_action.report_data.report_p_used = 1;
		// 	_delay_task_add("report_other", pack_report_other_info_new_mqtt, robot_arm_state, 0);
		// 	continue;
		// }
		if (((state->up_report == 1) && (state->port_info.usb_app == DEVICE_AISINO || state->port_info.usb_app == DEVICE_NISEC || state->port_info.usb_app == DEVICE_MENGBAI || state->port_info.usb_app == DEVICE_MENGBAI2 || (state->port_info.usb_app >= 101 && state->port_info.usb_app != DEVICE_ROBOT_ARM_LOCAL))) || \
			((state->off_report == 1) && (state->port_last_info.usb_app == DEVICE_AISINO || state->port_last_info.usb_app == DEVICE_NISEC || state->port_last_info.usb_app == DEVICE_MENGBAI || state->port_last_info.usb_app == DEVICE_MENGBAI2 || (state->port_last_info.usb_app >= 101 && state->port_last_info.usb_app != DEVICE_ROBOT_ARM_LOCAL))))
		{
			if (usb_port.report_action.report_data.report_p_used != 0)
			{
				//usb_port_out("�˿�״̬�ϴ���\n");
				continue;
			}
			usb_port.report_action.report_data.report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
			usb_port_out("��%d�Ŷ˿ڴ���һ��˰�̸��»������ϱ�\n", state->port);
			logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�˰�̸��»������ϱ�\r\n", port);
			_delay_task_add("report_plate_info", pack_report_plate_info, state, 0);
			continue;
		}
		// else if ((state->off_report == 1 || robot_arm_state->off_report == 1) && (state->port_last_info.usb_app == 0 || state->port_last_info.usb_app == 5 || state->port_last_info.usb_app == DEVICE_ROBOT_ARM_LOCAL))
		// {
		// 	// usb_port_out("2222222222222222\n");
		// 	if (usb_port.report_action.report_data.report_p_used != 0)
		// 	{
		// 		usb_port_out("�˿�״̬�ϴ���\n");
		// 		continue;
		// 	}
		// 	usb_port.report_action.report_data.report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
		// 	usb_port_out("����һ������USB���»������ϱ�\n");
		// 	logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�����USB���»������ϱ�\r\n", port);
		// 	_delay_task_add("report_other", pack_report_other_info_new_mqtt, state, 0);
		// 	continue;
		// }
		else if ((state->up_report == 1 || state->off_report == 1) || (robot_arm_state->up_report == 1 || robot_arm_state->off_report == 1))
		{
			// usb_port_out("333333333333333\n");
			//usb_port_out("usb_port.report_action.report_data.report_p_used : %d\n", usb_port.report_action.report_data.report_p_used);
			if (usb_port.report_action.report_data.report_p_used != 0 || state->port_info.port <= 0)
			{
				continue;
			}
			if ((state->port_info.usb_app == 4))
			{
				usb_port_out("����һ��[%d]�˿�״̬CA���»������ϱ�--\n", port);
				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬CA���»������ϱ�\r\n", port);
				//state->up_report = 0;
				usb_port.report_action.report_data.report_p_used = 1;
				_delay_task_add("report_ca", pack_report_ca_info_new_mqtt, state, 0);
			}
			else
			{
				usb_port_out("����һ��[%d]�˿�״̬�������»������ϱ�--\n", port);
				logout(INFO, "MQTT", "JSON_UPDATE", "����һ��[%d]�Ŷ˿�״̬�������»������ϱ�\r\n", port);
				usb_port.report_action.report_data.report_p_used = 1;
				_delay_task_add("report_other", pack_report_other_info_new_mqtt, state, 0);
			}
			continue;
		}
	}
}

int pack_report_plate_info_json_new_mqtt(char **g_buf, int off_up_flag, char *random,DEV_BASIC_DATA *state)
{
	struct _ter_info ter_info;
	char tmp[20] = { 0 };
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);


	cJSON *json = cJSON_CreateObject();


	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json, "client_ip", "");
	}

	if (off_up_flag == 1)
	{
		//�����ϱ�
		cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(json, "port_num", tmp);
		cJSON_AddStringToObject(json, "busid", state->port_info.busid);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		cJSON_AddStringToObject(json, "name", (char *)state->port_last_info.ca_name);
		cJSON_AddStringToObject(json, "number", (char *)state->port_last_info.ca_serial);
		cJSON_AddStringToObject(json, "plate_num", (char *)state->port_last_info.sn);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.extension);
		cJSON_AddStringToObject(json, "plate_extension", tmp);
		cJSON_AddStringToObject(json, "offline", "0");
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->power);
		cJSON_AddStringToObject(json, "port_power", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(json, "ad_status", tmp);


		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);


		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(json, "usb_share", "0");
		cJSON_AddStringToObject(json, "usb_share_mode", "0");
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(json, "user", state->port_info.user);
		//printf("pack_report_plate_info_json online = 0 , state->port_info.client = %d\r\n",state->port_info.client);
	}
	else
	{
		//�����ϱ�
		//wang 11/10
		usb_port.report_action.report_data.report_counts++;
		state->now_report_counts = usb_port.report_action.report_data.report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
		cJSON_AddStringToObject(json, "offline", "1");
		
		fun_get_port_data_json(json, state->port,2);			//wang 11/3 �޸� is_commonֵ��ֹ�����ն˲�ѯ�Ͷ���ն˲�ѯ�ֶ��ظ�

	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json);

	str_replace(tmp_json, "\\\\", "\\");
	cJSON_Delete(json);

	*g_buf = tmp_json;
	//ty_usb_m_out("����һ��˰����Ϣ�ϱ�,�˿ںţ�%d\n",state->port);
	//ty_usb_m_out("g_buf = %s\n", *g_buf);
	return 0;
}

/*********************************************************************
-   Function : pack_report_plate_info
-   Description���ϴ�˰����Ϣ
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_plate_info(void *arg)
{
	char *g_buf = NULL;
	char source_topic[50] = { 0 };
	int result = 0,off_up_flag = 0;;
	char random[50] = { 0 };
	char tmp[20] = {0};
	char *g_new_buf=NULL;
	char *g_pub_buf=NULL;
	char *tmp_json;
	char *json_zlib = NULL;
	int json_zlib_len = 0;
	cJSON *port_array, *port_layer ,*json ,*root ,*head_layer,*data_array,*data_layer;		//���ʹ��
	DEV_BASIC_DATA *state = (DEV_BASIC_DATA *)arg;
	DEV_BASIC_DATA *robot_arm_state = NULL,*report_state = NULL;
	struct _ter_info ter_info;

	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	
	memset(source_topic, 0, sizeof(source_topic));

	robot_arm_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];

	if ((state->off_report == 0) && (state->up_report == 0))
	{
		report_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];
	}

	if ((report_state->off_report == 1) && (report_state->up_report == 1))
	{
		if (report_state->last_act == 1)//������������Ȳ��������ϴ�
		{
			if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
			{
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
			}
			off_up_flag = 1;//���������ϴ�
		}	
		if (report_state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if((report_state->off_report == 1) && (report_state->up_report == 0))
	{
		off_up_flag = 1;//���������ϴ�
		if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
		{
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
		}
	}
		
	else if ((report_state->off_report == 0) && (report_state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	get_radom_serial_number("zdsbspxx", random);

	pack_report_plate_info_json_new_mqtt(&g_new_buf, off_up_flag,random,state);

	_lock_set(usb_port.data_lock);

	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		if(state->port == 0)
		{
			cJSON_Delete(json);
			goto End;
		}
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "plate");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);		
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);

		if(strlen(robot_arm_state->port_info.arm_version) > 0)
		{
			cJSON_AddStringToObject(port_layer, "robot_arm_version", robot_arm_state->port_info.arm_version);
			cJSON_AddStringToObject(port_layer, "robotarm_status", "0");
			if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[state->port-1].arm_with_dev_result == 1)
				cJSON_AddStringToObject(port_layer, "robotarm_with_dev_ports", "1");
			else
				cJSON_AddStringToObject(port_layer, "robotarm_with_dev_ports", "0");
		}

		free(g_new_buf);				
	}

	
	tmp_json = cJSON_PrintUnformatted(json);
	//usb_port_out("�ϱ�˰�̷��ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json);

	json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_pub_buf = cJSON_PrintUnformatted(root);
	//usb_port_out("�ϱ�˰�̷������ݣ�%s\n",g_pub_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_pub_buf);
	free(g_pub_buf);
	cJSON_Delete(root);

	//������
	//pack_report_plate_info_json(&g_buf, off_up_flag,random,state);
	pack_report_plate_info_json_new_mqtt(&g_buf, off_up_flag,random,state);
	result = _m_pub_no_answer("mb_equ_plate_status_up", g_buf);
	if (result < 0)
	{
		free(g_buf);
		sleep(5);
		goto End;
	}
	else
	{
		if (off_up_flag ==  1)
			state->off_report = 0;
		else
			state->up_report = 0;
		//logout(INFO, "MQTT", "JSON_UPDATE", "[%d]�Ŷ˿�˰�̸��»������ϱ��ɹ�\r\n", state->port);
		//usb_port_out("�ϴ���%d�ſڣ�%s��˰����Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	
	//usb_port_out("�����������\n");
	free(g_buf);

End:
	_lock_un(usb_port.data_lock);
	usb_port.report_action.report_data.report_p_used = 0;
	return;
}

/*********************************************************************
-   Function : pack_report_other_info_new_mqtt
-   Description���ϴ�˰����Ϣ
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_other_info_new_mqtt(void *arg)
{
	DEV_BASIC_DATA *state = (DEV_BASIC_DATA *)arg;
	DEV_BASIC_DATA *robot_arm_state = NULL,*report_state = NULL;
	char *g_buf;
	char *g_new_buf;
	char tmp[20] = { 0 };
	char source_topic[50] = { 0 };
	int result;
	char random[50] = { 0 };
	int off_up_flag = 0;
	struct _ter_info ter_info;

	memset(source_topic, 0, sizeof(source_topic));


	_lock_set(usb_port.data_lock);
	robot_arm_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];

	if ((state->off_report == 0) && (state->up_report == 0))
	{
		report_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];
	}
	else
	{
		report_state = (DEV_BASIC_DATA *)arg;
	}

	if ((report_state->off_report == 1) && (report_state->up_report == 1))
	{
		if (report_state->last_act == 1)//������������Ȳ��������ϴ�
		{
			if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
			{
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
			}
			off_up_flag = 1;//���������ϴ�
		}	
		if (report_state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if ((report_state->off_report == 1) && (report_state->up_report == 0))
	{
		if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
		{
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
		}
		off_up_flag = 1;//���������ϴ�
	}	
	else if ((report_state->off_report == 0) && (report_state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);

	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json, "port_num", tmp);

	if(strlen(robot_arm_state->port_info.arm_version) > 0)
	{
		cJSON_AddStringToObject(json, "robot_arm_version", robot_arm_state->port_info.arm_version);
		cJSON_AddStringToObject(json, "robotarm_status", "0");
		if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[state->port-1].arm_with_dev_result == 1)
			cJSON_AddStringToObject(json, "robotarm_with_dev_ports", "1");
		else
			cJSON_AddStringToObject(json, "robotarm_with_dev_ports", "0");
	}

	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json, "client_ip", "");
		// cJSON_AddStringToObject(json, "robotarm_with_dev_ports", "0");
		// cJSON_AddStringToObject(json, "robotarm_status", "0");
		// cJSON_AddStringToObject(json, "port_num", "1");
		// cJSON_AddStringToObject(json, "robot_arm_version", "MB_ROBOT_ARM_1.0");
	}
	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_last_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		cJSON_AddStringToObject(json, "offline", "0");

		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		//wang 2022��10��17�� ���json����ȡ
		cJSON_AddStringToObject(json, "usb_share", "0");
		cJSON_AddStringToObject(json, "usb_share_mode", "0");
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(json, "user", state->port_info.user);

		memset(tmp, 0, sizeof(tmp));
		if(usb_port.machine_type == 1)
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate((state->port*2)>0?((state->port*2) - 1):0))->port_state_enum);
		else
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate(state->port>0?(state->port - 1):0))->port_state_enum);
		cJSON_AddStringToObject(json, "port_power", tmp);
	}
	else
	{
		//�����ϱ�
		//wang 11/10
		usb_port.report_action.report_data.report_counts += 1;
		state->now_report_counts = usb_port.report_action.report_data.report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json, "report_serial", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(json, "usb_app", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json, "pid", tmp);
		cJSON_AddStringToObject(json, "offline", "1");

		//wang 2022��10��17�� ���json����ȡ
		//if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		//printf("!!!!!!!!!!!!!!!lbc state->port = %d\n",state->port);
		if(usb_port.load_action.load_data[state->port - 1].usbip_state == ATTACHED)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(json, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", usb_port.load_action.load_data[state->port - 1].net_type);
		cJSON_AddStringToObject(json, "usb_share_mode", tmp);
		cJSON_AddStringToObject(json, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(json, "user", state->port_info.user);
		memset(tmp, 0, sizeof(tmp));
		if(usb_port.machine_type == 1)
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate((state->port*2)>0?((state->port*2) - 1):0))->port_state_enum);
		else
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate(state->port>0?(state->port - 1):0))->port_state_enum);
		cJSON_AddStringToObject(json, "port_power", tmp);
	}

	g_new_buf = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);
	
	cJSON *port_array, *port_layer;				//���ʹ��
	cJSON *json_port_data = cJSON_CreateObject();
	cJSON_AddItemToObject(json_port_data, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		if(state->port == 0)
		{
			cJSON_Delete(json_port_data);
			goto End;
		}
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "other");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);			//014b
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);	
		free(g_new_buf);				
	}

	char *tmp_json;
	tmp_json = cJSON_PrintUnformatted(json_port_data);
	//usb_port_out("�ϱ�����usb���ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json_port_data);

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_buf = cJSON_PrintUnformatted(root);
	//usb_port_out("�ϱ�����usb�������ݣ�%s\n",g_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_buf);
	if (result < 0)
	{
		usb_port_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��,����ʱ��Ϊ%d\n", 5);
		free(g_buf);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (off_up_flag == 1)
			report_state->off_report = 0;
		else
			report_state->up_report = 0;
		usb_port_out("�ϴ���%d�ſڡ�����USB��Ϣ������Ӧ����ɹ�\n\n\n", state->port);
	}
	//sleep(5);
	//usb_port_out("�����������\n");
	free(g_buf);
	cJSON_Delete(root);

End:
	_lock_un(usb_port.data_lock);
	usb_port.report_action.report_data.report_p_used = 0;
	return;
}

/*********************************************************************
-   Function : pack_report_ca_info_new_mqtt
-   Description���ϴ�˰����Ϣ
-   Input  :
-   Output :
-   Return :0 ���� ���� ������Ӧ�Ĵ�����
-   Other :
***********************************************************************/
void pack_report_ca_info_new_mqtt(void *arg)
{
	DEV_BASIC_DATA *state = (DEV_BASIC_DATA *)arg;
	DEV_BASIC_DATA *robot_arm_state = NULL,*report_state = NULL;
	char *g_pub_buf;
	char *g_new_buf;
	char tmp[20];
	char source_topic[50];
	int result;
	char random[50] = { 0 };

	usb_port.report_action.report_data.report_p_used = 1;//��ʶ����ʹ�øĶ���,��������ٴ�����
	memset(source_topic, 0, sizeof(source_topic));
	int off_up_flag = 0;
	// printf("pack_report_ca_info_new_mqtt state->port = %d\n",state->port);
	robot_arm_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];
	if ((state->off_report == 0) && (state->up_report == 0))
	{
		report_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1];
	}
	else
	{
		report_state = (DEV_BASIC_DATA *)arg;
	}
    // printf("pack_report_ca_info_new_mqtt 123\n");
	if ((report_state->off_report == 1) && (report_state->up_report == 1))
	{
		if (report_state->last_act == 1)//������������Ȳ��������ϴ�
		{
			if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
			{
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
				usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
			}
			off_up_flag = 1;//���������ϴ�
		}	
		if (report_state->last_act == 2)//������������Ȳ��������ϴ�
			off_up_flag = 2;
	}
	else if((report_state->off_report == 1) && (report_state->up_report == 0))
	{
		if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].port_info.arm_version))
		{
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].up_report = 1;
			usb_port.online_bank_action.mechanical_arm_action.arm_data[state->port - 1].last_act = 1;
		}
		off_up_flag = 1;//���������ϴ�
	}	
	else if ((report_state->off_report == 0) && (report_state->up_report == 1))
		off_up_flag = 2;//���������ϴ�
	else
	{
		goto End;
	}
	// printf("pack_report_ca_info_new_mqtt 234\n");
	struct _ter_info ter_info;
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	// printf("pack_report_ca_info_new_mqtt 11111\n");
	_lock_set(usb_port.data_lock);

	get_radom_serial_number("zdsbcaxx", random);
	cJSON *json_data = cJSON_CreateObject();

	cJSON_AddStringToObject(json_data, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port);
	cJSON_AddStringToObject(json_data, "port_num", tmp);

	// printf("pack_report_ca_info_new_mqtt 22222\n");
	// if ((result = jude_port_allow_attach(state)) < 0)
	// {
	// 	cJSON_AddStringToObject(json_data, "tax_auth","0");
	// }
	// else
	{
		cJSON_AddStringToObject(json_data, "tax_auth","1");
	}

	cJSON_AddStringToObject(json_data, "tax_auth", tmp);
	cJSON_AddStringToObject(json_data, "start_time", usb_port.key_auth_state.start_time);
	cJSON_AddStringToObject(json_data, "end_time", usb_port.key_auth_state.end_time);
	// printf("pack_report_ca_info_new_mqtt 333333\n");
	if(strlen(robot_arm_state->port_info.arm_version) > 0)
	{
		cJSON_AddStringToObject(json_data, "robot_arm_version", robot_arm_state->port_info.arm_version);
		cJSON_AddStringToObject(json_data, "robotarm_status", "0");
		if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[state->port-1].arm_with_dev_result == 1)
			cJSON_AddStringToObject(json_data, "robotarm_with_dev_ports", "1");
		else
			cJSON_AddStringToObject(json_data, "robotarm_with_dev_ports", "0");
	}
	// printf("pack_report_ca_info_new_mqtt 44444\n");

	//wang 11/17 ����̨�ϱ� client_ip
	if (state->port_info.ip != 0x000000)
	{
		char ip_str[50] = { 0 };
		ip2ip_asc(state->port_info.ip, ip_str);
		cJSON_AddStringToObject(json_data, "client_ip", ip_str);
	}
	else
	{
		cJSON_AddStringToObject(json_data, "client_ip", "");
	}

	if (off_up_flag == 1)
	{
		//�����ϱ�
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port_last_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(json_data, "usb_app", tmp);
		//#endif

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.vid);
		cJSON_AddStringToObject(json_data, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_last_info.pid);
		cJSON_AddStringToObject(json_data, "pid", tmp);
		#ifndef DF_SUPPORT_CANAME				//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��
		if (strlen((char *)state->port_last_info.ca_name) == 0)
		{
			cJSON_AddStringToObject(json_data, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "name", (char *)state->port_last_info.ca_name);
		if (strlen((char *)state->port_last_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json_data, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "number", (char *)state->port_last_info.ca_serial);
		if (strlen((char *)state->port_last_info.sn) == 0)
		{
			cJSON_AddStringToObject(json_data, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "serial", (char *)state->port_last_info.sn);
		#endif
		memset(tmp, 0, sizeof(tmp));	
		if(usb_port.machine_type == 1)
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate((state->port*2)>0?((state->port*2) - 1):0))->port_state_enum);
		else
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate(state->port>0?(state->port - 1):0))->port_state_enum);
		cJSON_AddStringToObject(json_data, "port_power", tmp);
		//printf("step1\n");

		//wang 11/10
		state->now_report_counts = 0;
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json_data, "report_serial", tmp);

		cJSON_AddStringToObject(json_data, "usb_share", "0");
		cJSON_AddStringToObject(json_data, "usb_share_mode", "0");
		cJSON_AddStringToObject(json_data, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json_data, "offline", "0");
		cJSON_AddStringToObject(json_data, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(json_data, "user", state->port_info.user);
		//printf("step2\n");
	}
	else
	{
		//�����ϱ�

		//wang 11/10
		usb_port.report_action.report_data.report_counts += 1;
		state->now_report_counts = usb_port.report_action.report_data.report_counts;

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->now_report_counts);
		cJSON_AddStringToObject(json_data, "report_serial", tmp);

		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", state->port_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(json_data, "usb_app", tmp);
		//#endif
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(json_data, "vid", tmp);
		memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(json_data, "pid", tmp);
		#ifndef DF_SUPPORT_CANAME				//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��
		if (strlen((char *)state->port_info.ca_name) == 0)
		{
			//usb_port_out("#####################ca_name :%s###########\n",)
			cJSON_AddStringToObject(json_data, "name", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "name", (char *)state->port_info.ca_name);
		if (strlen((char *)state->port_info.ca_serial) == 0)
		{
			cJSON_AddStringToObject(json_data, "number", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "number", (char *)state->port_info.ca_serial);
		if (strlen((char *)state->port_info.sn) == 0)
		{
			cJSON_AddStringToObject(json_data, "serial", "Unknown");
		}
		else
			cJSON_AddStringToObject(json_data, "serial", (char *)state->port_info.sn);
		#endif
		// printf("lbc---------------------state->port_info.port_used=%d state->port_info.system_used=%d\n",state->port_info.port_used,state->port_info.system_used);
		// if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		//printf("!!!!!!!!!!!!!!!lbc state->port = %d\n",state->port);
		if(usb_port.load_action.load_data[state->port - 1].usbip_state == ATTACHED)
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "1");
		}
		else
		{
			memset(tmp, 0, sizeof(tmp)); sprintf(tmp, "0");
		}
		cJSON_AddStringToObject(json_data, "usb_share", tmp);
		memset(tmp, 0, sizeof(tmp)); 
		sprintf(tmp, "%d", usb_port.load_action.load_data[state->port - 1].net_type);
		cJSON_AddStringToObject(json_data, "usb_share_mode", tmp);
		cJSON_AddStringToObject(json_data, "client_id", state->port_info.client);
		cJSON_AddStringToObject(json_data, "offline", "1");
		cJSON_AddStringToObject(json_data, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(json_data, "user", state->port_info.user);
		memset(tmp, 0, sizeof(tmp));
		if(usb_port.machine_type == 1)
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate((state->port*2)>0?((state->port*2) - 1):0))->port_state_enum);
		else
			sprintf(tmp, "%d", (int)(get_port_priority_usedstate(state->port>0?(state->port - 1):0))->port_state_enum);
		cJSON_AddStringToObject(json_data, "port_power", tmp);
	}

	g_new_buf = cJSON_PrintUnformatted(json_data);
	cJSON_Delete(json_data);

	cJSON *port_array, *port_layer;		//���ʹ��
	cJSON *json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "port_data", port_array = cJSON_CreateArray());

	if(g_new_buf != NULL)
	{
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		if(state->port == 0)
		{
			cJSON_Delete(json);
			goto End;
		}
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%d", state->port);
		cJSON_AddStringToObject(port_layer, "port_num", tmp);
		cJSON_AddStringToObject(port_layer, "port_type", "ca");
		cJSON_AddStringToObject(port_layer, "port_off_on", "on");
		memset(tmp, 0, sizeof(tmp));	
		sprintf(tmp, "%04x", state->now_report_counts);			//014b
		cJSON_AddStringToObject(port_layer, "report_serial", tmp);
		cJSON_AddStringToObject(port_layer, "port_data", g_new_buf);	
		free(g_new_buf);				
	}

	char *tmp_json;

	tmp_json = cJSON_PrintUnformatted(json);
	//usb_port_out("�ϱ�ca���ͽ�ѹǰ���ݣ�%s\n",tmp_json);
	cJSON_Delete(json);

	char *json_zlib;
	int json_zlib_len = strlen(tmp_json) + 1024;
	json_zlib = (char *)malloc(json_zlib_len);
	memset(json_zlib, 0, json_zlib_len);
	asc_compress_base64((const unsigned char *)tmp_json, strlen(tmp_json), (unsigned char *)json_zlib, &json_zlib_len);
	free(tmp_json);

	cJSON *head_layer, *data_array, *data_layer;		//���ʹ��
	cJSON *root = cJSON_CreateObject();
	sprintf(source_topic, "mb_fpkjxt_%s", ter_info.ter_id);
	get_radom_serial_number("zdsbdkxx", random);
	cJSON_AddItemToObject(root, "head", head_layer = cJSON_CreateObject());
	cJSON_AddStringToObject(head_layer, "protocol", DF_MQTT_SYNC_PROTOCOL_VER);
	cJSON_AddStringToObject(head_layer, "code_type", DF_MQTT_SYNC_CODE_REPORT);
	cJSON_AddStringToObject(head_layer, "cmd", "zdsbspxx");
	cJSON_AddStringToObject(head_layer, "source_topic", source_topic);
	cJSON_AddStringToObject(head_layer, "random", random);
	cJSON_AddStringToObject(head_layer, "result", "s");
	cJSON_AddItemToObject(root, "data", data_array = cJSON_CreateArray());

	cJSON_AddItemToObject(data_array, "dira", data_layer = cJSON_CreateObject());

	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);
	memset(tmp, 0, sizeof(tmp));	sprintf(tmp, "%d", usb_port.usb_port_numb);
	cJSON_AddStringToObject(data_layer, "usb_port_num", tmp);
	cJSON_AddStringToObject(data_layer, "use_compress", "1");
	cJSON_AddStringToObject(data_layer, "compress_type", "zlib");
	cJSON_AddStringToObject(data_layer, "compress_data", json_zlib);

	free(json_zlib);

	g_pub_buf = cJSON_PrintUnformatted(root);
	//usb_port_out("�ϱ�ca�������ݣ�%s\n",g_pub_buf);
	result = _m_pub_no_answer("mb_equ_ports_status_up", g_pub_buf);
	if (result < 0)
	{
		usb_port_out("���������ճ�ʱʧ��,���Ӵ����������������ʱ��\n");
		free(g_pub_buf);
		cJSON_Delete(root);
		goto End;
	}
	else
	{
		if (off_up_flag == 1)
			report_state->off_report = 0;
		else
			report_state->up_report = 0;
		usb_port_out("�ϴ���%d�ſڣ�%s��CA��Ϣ������Ӧ����ɹ�\n\n\n", state->port, state->port_info.ca_name);
	}
	sleep(1);				//wang 11/24
	//usb_port_out("�����������\n");
	free(g_pub_buf);
	cJSON_Delete(root);

End:
	usb_port_out("����ca�ϱ�����1\n");
	_lock_un(usb_port.data_lock);
	usb_port.report_action.report_data.report_p_used = 0;
	return;
}

int jude_port_allow_attach(void)
{
	char date_now[100] = { 0 };
	timer_read_y_m_d_h_m_s(date_now);
	
	// printf("jude_port_allow_attach date_now = %s stream->end_time = %s\n",date_now,usb_port.key_auth_state.end_time);
	//if(strlen(usb_port.key_auth_state.end_time) > 0)
	if(usb_port.key_auth_state.keyfile_load_err < 0 && (access(AUTH_CERT_FILE_PATH, 0) >= 0))
	{
		logout(INFO, "SYSTEM", "��Ȩ�ж�", "��Ƭ����Կ��ȡʧ�ܣ���Ȩ�ļ����ڣ��ж�ͨ��\r\n");
		return 0;
	}

	{
		if (memcmp(date_now,usb_port.key_auth_state.end_time,10) > 0)
		{
			//out("�˿���Ȩ����\n");
			return -2;
		}
	}
	return 0;
}

void* load_start(void* args)
{
	int result = USB_PORT_NO_ERROR,time = 0;
	int effective_port = 0,usb_port_numb;
	LOAD_DATA *load_data = (LOAD_DATA *)args;
	DEV_BASIC_DATA *state = NULL;
	char ip_str[50] = {0x00};
	if(NULL == load_data)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > load_data->port|| usb_port_numb < load_data->port)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	
	if ((result = jude_port_allow_attach()) < 0)
	{
		printf("jude_port_allow_attach failed result = %d\n",result);
		return (void *)(&result);
	}

	if(WITH_DEV != usb_port.judge_usbport_exist.port_exist_state[load_data->port].now_port_state)   //�˶˿����豸
	{
		result = USB_COMMON_ERROR;
		return (void *)(&result);
	}

	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		effective_port = load_data->port/2;
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(load_data->port))   //�ǻ�е��
		{
			usb_port_out("�˿�%d�ǻ�е�ۣ��ݲ�֧�ּ���\n",load_data->port+1);
			result = USB_COMMON_ERROR;
			return (void *)(&result);
		}
	}
	else
	{
		effective_port = load_data->port;
	}
	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port];

	//�򿪶˿�
	set_ignore_judge_flag(load_data->port,IGNORE); //���ú��԰��豸���

	if(usb_port.load_action.load_data[effective_port].usbip_state == ATTACHED)//�Ѿ�����
	{
		usb_port_out("[open_port_ip]�Ѿ�����\n");
		set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
		result = USB_COMMON_ERROR;
		return (void *)(&result);
	}
	//����ɻ�е�۽��м��أ�״̬�������ı�
	if(usb_port.machine_type != TYPE_2303 && (usb_port.get_dev_basic_action.usb_dev_basic_data[load_data->port].port_info.vid == 0x1ea7 && usb_port.get_dev_basic_action.usb_dev_basic_data[load_data->port].port_info.pid == 0x0064)) //��2303
	{
		printf("111 no attach,restart!!!!!!!!!!!!!!!!\n");
	}
	else
	{
		usb_port.load_action.load_data[effective_port].usbip_state = ATTACHED;
		usb_port.load_action.load_data[effective_port].net_type = load_data->net_type;
			state->port_info.app_fd = timer_read(usb_port.timer_fd);
		state->port_info.ip = load_data->ip;
		usb_port.load_action.load_data[effective_port].usbip_state = ATTACHED;
		//printf("usb_port.load_action.load_data[%d].usbip_state = %d\n",effective_port,usb_port.load_action.load_data[effective_port].usbip_state);
		usb_port.load_action.load_data[effective_port].net_type = load_data->net_type;
		memset(usb_port.load_action.load_data[effective_port].oper_id,0x00,sizeof(usb_port.load_action.load_data[effective_port].oper_id));
		memcpy(usb_port.load_action.load_data[effective_port].oper_id,load_data->oper_id, strlen(load_data->oper_id));
		memset(usb_port.load_action.load_data[effective_port].user,0x00,sizeof(usb_port.load_action.load_data[effective_port].user));
		memcpy(usb_port.load_action.load_data[effective_port].user,load_data->user, strlen(load_data->user));
		ip2ip_asc(load_data->ip, ip_str);
		if (strlen(load_data->user) != 0)
		{
			memset(state->port_info.client,0x00,sizeof(state->port_info.client));
			memcpy(state->port_info.client, load_data->user, strlen(load_data->user));
			memset(state->port_info.user,0x00,sizeof(state->port_info.user));
			memcpy(state->port_info.user, load_data->user, strlen(load_data->user));
		}
		if (strlen(load_data->oper_id) != 0)
		{
			memset(state->port_info.oper_id,0x00,sizeof(state->port_info.oper_id));
			memcpy(state->port_info.oper_id, load_data->oper_id, strlen(load_data->oper_id));
		}
			
		*(load_data->port_fd) = state->port_info.app_fd;
		state->port_info.last_open_time = get_time_sec();
		state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
		state->last_act = 1;
	}

	if(usb_port.machine_type != TYPE_2303) //��2303
	{
		//��ɻ�е�ۣ������أ�ֱ�������˿�
		if(usb_port.get_dev_basic_action.usb_dev_basic_data[load_data->port].port_info.vid == 0x1ea7 && usb_port.get_dev_basic_action.usb_dev_basic_data[load_data->port].port_info.pid == 0x0064)
		{
			printf("no attach,restart!!!!!!!!!!!!!!!!\n");
			// usb_port.power_action.fun_power(load_data->port,RETART_ACTION);
			// sleep(2);
			// usb_port.power_action.fun_power(load_data->port,CLOSE_STATE);
			power_saving_contrl_port(load_data->port,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[load_data->port].power_control_type);
			// usb_port.power_action.power_data[load_data->port].power_off_time = get_time_sec();
			usleep(1*1000*1000);
			set_ignore_judge_flag(load_data->port,IGNORE); //���ú��԰��豸���
			power_saving_contrl_port(load_data->port,OPEN_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[load_data->port].power_control_type);
			usleep(4*1000*1000);
			power_saving_contrl_port(load_data->port,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[load_data->port].power_control_type);
			usleep(1*1000*1000);
			usb_port.judge_usbport_exist.port_exist_state[load_data->port].without_dev_count = 0;
			set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
			result = -1;
			
			return (void *)(&result);
		}
	}

	result = power_saving_contrl_port(load_data->port,OPEN_STATE,ATTACH_PRIORITY,usb_port.power_action.power_data[load_data->port].power_control_type);
	if(POWER_SAVING_NO_ERROR != result)
	{
		usb_port_out("�򿪶˿�%dʧ��,������� %d\n",load_data->port,result);
		set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
		return (void *)(&result);
	}
	state->port_info.last_open_time = get_time_sec();
	
	if (0 != my_get_port_exit(load_data->port + 1,usb_port.app_note))  //WITH_DEV == get_port_exit(load_data->port)
	{
		usb_port_out("[open_port_ip]û�з��ָ��豸����೬ʱ�ȴ�7��,load_data->port=%d\n",load_data->port);
		for (time = 0; time < 7000; time++)
		{
			if(OPEN_STATE == (get_port_priority_usedstate(load_data->port))->port_state_enum)
			{
				if (0 == my_get_port_exit(load_data->port + 1,usb_port.app_note))
				{
					sleep(1);
					break;
				}
				usleep(1000);
			}
			else
			{
				result = USB_COMMON_ERROR;
				// set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
				return (void *)(&result);
			}
		}
		if (time == 7000)
		{
			usb_port_out("[open_port_ip]�ö˿��Ѿ���,û�з��ָ��豸\n");
			result = USB_COMMON_ERROR;
			set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
			return (void *)(&result);
		}
	}
	
	//usleep(4*1000*1000);
	usleep(1000*1000);
	// state->port_info.app_fd = timer_read(usb_port.timer_fd);
	// state->port_info.ip = load_data->ip;
	// usb_port.load_action.load_data[effective_port].usbip_state = ATTACHED;
	// printf("usb_port.load_action.load_data[%d].usbip_state = %d\n",effective_port,usb_port.load_action.load_data[effective_port].usbip_state);
	// usb_port.load_action.load_data[effective_port].net_type = load_data->net_type;
	// ip2ip_asc(load_data->ip, ip_str);
	// if (strlen(load_data->client) != 0)
	// 	memcpy(state->port_info.client, load_data->client, strlen(load_data->client));
	// *(load_data->port_fd) = state->port_info.app_fd;
	// state->port_info.last_open_time = get_time_sec();

	if(usb_port.machine_type == 1)
		logout(INFO, "usbshare", "attach", "10001��·����USB�˿�,�˿ںţ�%d,ʹ����IP��%s\r\n",(load_data->port + 1)/2, ip_str);
	else
		logout(INFO, "usbshare", "attach", "10001��·����USB�˿�,�˿ںţ�%d,ʹ����IP��%s\r\n",load_data->port + 1, ip_str);
	
	printf("usb_port.get_dev_basic_action.usb_dev_basic_data[load_data->port].port_info.last_open_time = %d\n",usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.last_open_time);
	// state->up_report = 1;//�˿�״̬������Ҫ�ϱ�
	// state->last_act = 1;

	set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
	return (void *)(&result);
}

void* load_stop(void* args)
{
	LOAD_DATA *load_data = (LOAD_DATA *)args;
	DEV_BASIC_DATA *state = NULL;
	int result = USB_PORT_NO_ERROR;
	int effective_port = 0,usb_port_numb = 0;
	if(NULL == load_data)
	{
		usb_port_out("load_stop,NULL == load_data\n");
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
		
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > load_data->port|| usb_port_numb < load_data->port)
	{
		usb_port_out("load_data->port = %d,usb_port_numb = %d\n",load_data->port,usb_port_numb);
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
		
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(load_data->port))   //�ǻ�е��
		{
			usb_port_out("load_stop,��%d��is robotarm\n", load_data->port);
			result = USB_COMMON_ERROR;
			return (void *)(&result);
		}
	}
	if(usb_port.machine_type == 1)
	{
		effective_port = load_data->port/2;
		usb_port_out("close_port_socket����ر�����,�˿ںš�%d��\n", (load_data->port + 1)/2);
	}
	else
	{
		effective_port = load_data->port;
		usb_port_out("close_port_socket����ر�����,�˿ںš�%d��\n", load_data->port + 1);
	}
	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port];
	
	if(usb_port.load_action.load_data[effective_port].usbip_state == UNATTACHED)  //�����ͷ�
	{
		//printf("-------------------UNATTACHED--------------------------\n");
		result = USB_PORT_NO_ERROR;
		return (void *)(&result);
	}

	set_ignore_judge_flag(load_data->port,IGNORE); //���ú��԰��豸���

	state->port_info.tran_line_closed_flag = 0;
	
	usb_port.load_action.load_data[effective_port].usbip_state = UNATTACHED;
	usb_port.load_action.load_data[effective_port].net_type = INTRANET;
	state->port_info.close_port_flag = 1;
	state->port_info.usbip_state = 0;
	state->port_info.check_num = 0;
	state->port_info.port_status = 0x00;//�˿�״̬Ϊ�򿪹�,�����ѹر�   
	state->port_info.used_time = 0;
	memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	state->port_info.app_fd = 0;
	state->port_info.ip = 0;
	state->port_info.last_open_time = 0;
	state->port_info.system_used = 0;
	memset(state->port_info.client, 0, sizeof(state->port_info.client));
	state->port_info.usb_share_mode = 0;
	state->port_info.used_level = 0;
	state->up_report = 1;
	state->last_act = 1;

	ty_ctl(usb_port.module->tran_fd, DF_CLOSE_LINE_TRAN_TASK, load_data->port + 1,&state->port_info.tran_line_closed_flag);

	usb_port.power_action.power_data[load_data->port].power_off_time = get_time_sec();
	if(OPEN_STATE == get_port_priority_usedstate(load_data->port)->port_state_enum)  //����Ǵ�״̬������Ҫ�ر�
	{
		result = power_saving_contrl_port(load_data->port,CLOSE_STATE,ATTACH_PRIORITY,usb_port.power_action.power_data[load_data->port].power_control_type);
		if(POWER_SAVING_NO_ERROR != result)
		{
			usb_port_out("�رն˿�%dʧ��,������� %d\n",load_data->port,result);
			//return (void *)(&result);
		}
	}
	// set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
	usb_port.power_action.power_data[load_data->port].power_off_time = get_time_sec();
	if(usb_port.machine_type == 1)
		usb_port_out("�ر�ת����·,�رն˿�=%d\n", (load_data->port + 1)/2);
	else
		usb_port_out("�ر�ת����·,�رն˿�=%d\n", load_data->port + 1);
	// state->port_info.tran_line_closed_flag = 0;
	// ty_ctl(usb_port.module->tran_fd, DF_CLOSE_LINE_TRAN_TASK, load_data->port + 1,&state->port_info.tran_line_closed_flag);
	// usb_port.load_action.load_data[effective_port].usbip_state = UNATTACHED;
	// usb_port.load_action.load_data[effective_port].net_type = INTRANET;
	// state->port_info.close_port_flag = 1;
	// state->port_info.usbip_state = 0;
	// state->port_info.check_num = 0;
	// state->port_info.port_status = 0x00;//�˿�״̬Ϊ�򿪹�,�����ѹر�   
	// state->port_info.used_time = 0;
	// memset(state->port_info.used_info, 0, sizeof(state->port_info.used_info));
	// state->port_info.app_fd = 0;
	// state->port_info.ip = 0;
	// state->port_info.last_open_time = 0;
	// state->port_info.system_used = 0;
	// memset(state->port_info.client, 0, sizeof(state->port_info.client));
	// state->port_info.usb_share_mode = 0;
	// state->port_info.used_level = 0;
	// state->up_report = 1;
	// state->last_act = 1;
	if(usb_port.machine_type == 1)
	{
		usb_port_out("[close_port_socket]�ر�%d�˿ڳɹ�\n", (load_data->port + 1)/2);
		logout(INFO, "10001", "dettach", "%s,ǿ���ͷ�USB�˿�,�˿ںţ�%d\r\n", state->port_info.port_str, (load_data->port + 1)/2);
	}
	else
	{
		usb_port_out("[close_port_socket]�ر�%d�˿ڳɹ�\n", load_data->port + 1);
		logout(INFO, "10001", "dettach", "%s,ǿ���ͷ�USB�˿�,�˿ںţ�%d\r\n", state->port_info.port_str, load_data->port + 1);
	}
	usb_port.power_action.power_data[load_data->port].power_off_time = get_time_sec();
	set_ignore_judge_flag(load_data->port,NO_IGNORE); //����Ϊ�����԰��豸���
	
	return (void *)(&result);
}

//��ȡadֵ�Ͷ˿�״̬
void *get_port_ad_status(void *arg)
{
	int result = USB_PORT_NO_ERROR,i = 0,ret = 0,port_data = 0,port_total = 0,usb_port_numb = 0;
	DEV_BASIC_DATA *state = NULL;
	uint8 *all_port_status_ptr = (uint8 *)arg;
	uint8 all_port_status[4096] = { 0 };
	uint8 ad_status[4096] = {0};
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		if(NULL == usb_port.module || NULL == all_port_status_ptr)
		{
			result = USB_PORT_PARAMETER_ERROR;
			return (void *)(&result);
		}
		for(i = 0;i < usb_port_numb;++i)
		{
			// state = &usb_port.get_dev_basic_action.usb_dev_basic_data[i];
			if(0 != usb_port.online_bank_action.mechanical_arm_action.class->fun_judge_robot_arm(i))   //���ǻ�е��
			{
				if(0 == strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[i/2].port_info.arm_version))
				{
					continue;
				}
				ret = (get_robot_arm_class())->fun_get_port_state(i/2,&(usb_port.get_dev_basic_action.usb_dev_basic_data[i/2].port_info.ad_status));
				if(ret >= 0)
				{
					all_port_status_ptr[i] = ret;
				}
				else
				{
					all_port_status_ptr[i] = 0;
				}
			}
			else
				all_port_status_ptr[i] = 1;   //��е��Ĭ�ϳ�����
		}
	}
	else
	{
		result = ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_GET_AD_STATUS, ad_status);
		//usb_port_out("111111result =%d\n", result);
		if (result != 2 * usb_port.usb_port_numb)
		{
			usb_port_out("��ȡ����ad���س��ȴ���:result =%d\n", result);
			result = USB_PORT_PARAMETER_ERROR;
			return (void *)(&result);
		}
		for (i = 0; i < usb_port_numb; i++)
		{
			state = &usb_port.get_dev_basic_action.usb_dev_basic_data[i];
			state->port_info.ad_status = ad_status[i*2]*256+ad_status[i*2+1];
		}
		result = ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS, all_port_status);
		//usb_port_out("222222result =%d\n", result);
		if (result < 0 || result < 2)
		{
			usb_port_out("DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS err!\n");
			result = USB_PORT_PARAMETER_ERROR;
			return (void *)(&result);
		}
		if((result - 2)%3 != 0)  //������3�ı���
		{
			usb_port_out("DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS err111!\n");
			result = USB_PORT_PARAMETER_ERROR;
			return (void *)(&result);
		}
		port_total = all_port_status[0] << 8 | all_port_status[1];
		//usb_port_out("port_total = %d\n",port_total);
		for(i= 0;i<port_total;++i)
		{
			port_data = all_port_status[i*3 + 2] << 8 | all_port_status[i*3 + 2 + 1];
			// usb_port_out("port_data = %d,state = %d\n",port_data,all_port_status[i*3 + 2 + 2]);
			if(port_data > 0 && port_data <= usb_port.usb_port_numb)
			{
				all_port_status_ptr[port_data - 1] = all_port_status[i*3 + 2 + 2];
			}
		}

		for(i = 0;i < usb_port.usb_port_numb;++i)  //usb_port.usb_port_numb
		{
			state = &usb_port.get_dev_basic_action.usb_dev_basic_data[i];
			// if(i == 0)
			// {
			// 	printf("i = %d,arm_version = %s\n",i,usb_port.online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version);
			// }
			if(0 == strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[i].port_info.arm_version))
			{
				continue;
			}
			ret = (get_robot_arm_class())->fun_get_port_state(i,&(usb_port.get_dev_basic_action.usb_dev_basic_data[i].port_info.ad_status));
			//printf("!!!!!!!!!!!!!!!!!!!!!!!!get_port_ad_status i = %d,ret = %d\n",i,ret);
			if(ret >= 0)
			{
				all_port_status_ptr[i] = ret;
			}
			else
			{
				all_port_status_ptr[i] = 0;
			}
			
		}
	}
	//printf("@@@@@@@@@@@@@exit get_port_ad_status\n");
	result = USB_PORT_NO_ERROR;
	return (void *)(&result);
}

int get_port_ad_status_thread(void *arg, int o_timer)
{
	int result = USB_PORT_NO_ERROR,i = 0,ret = 0,usb_port_numb = 0;//r_len = 0,count = 0,port_total_num = 0,port_num = 0; //�˿����� �� �˿���
	DEV_BASIC_DATA *state = NULL;
	int port = *((int *)arg);
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	//usb_port_out("get_port_ad_status_thread,port = %d\n",port);
	if(1 > port || usb_port_numb < port)
	{
		return -1;
	}
	port = port - 1;
	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port];
		
	if(port%2 == 1) //����Ϊ�̣����ж˿�״̬��ȡ
	{
		//usb_port_out("enter get port state\n");
		if(0 == strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port - 1].port_info.arm_version))
		{
			//usb_port_out("arm is not ready\n");
			return result;
		}
		// ret = (get_robot_arm_class())->fun_get_port_ad(port);
		// if(ret >= 0)
		// {
		// 	state->port_info.ad_status = ret;
		// }
		// else
		// {
		// 	state->port_info.ad_status = 0;
		// }
		//ret = (get_robot_arm_class())->fun_get_port_state(port);
		// if(port == 1)
		usb_port_out("port = %d,ad_value = %d,ret= %d\n",port,state->port_info.ad_status,ret);
		if(ret >= 0)
		{
			usb_port.get_port_status.port_status_buff[port] = ret;
		}
		else
		{
			usb_port.get_port_status.port_status_buff[port] = 0;
		}
	}
	return result;
}

// void *get_port_ad_status_thread(void *arg)
// {
// 	int result = USB_PORT_NO_ERROR,i = 0,ret = 0;//r_len = 0,count = 0,port_total_num = 0,port_num = 0; //�˿����� �� �˿���
// 	DEV_BASIC_DATA *state = NULL;
// 	int port = *((int *)arg);
// 	usb_port_out("get_port_ad_status_thread,port = %d\n",port);
// 	if(1 > port || usb_port.usb_port_numb < port)
// 	{
// 		result = -1;
// 		return (void *)(&result);
// 	}
// 	port = port - 1;
// 	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port];
	
// 	while(1)
// 	{
// 		if(port%2 == 1) //����Ϊ�̣����ж˿�״̬��ȡ
// 		{
// 			//usb_port_out("enter get port state\n");
// 			if(0 == strlen(usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1].port_info.arm_version))
// 			{
// 				//usb_port_out("arm is not ready\n");
// 				usleep(1000*500);
// 				continue;
// 			}
// 			// ret = (get_robot_arm_class())->fun_get_port_ad(port);
// 			// if(ret >= 0)
// 			// {
// 			// 	state->port_info.ad_status = ret;
// 			// }
// 			// else
// 			// {
// 			// 	state->port_info.ad_status = 0;
// 			// }
// 			ret = (get_robot_arm_class())->fun_get_port_state(port);
// 			if(port == 1)
// 				usb_port_out("port = %d,ad_value = %d,ret= %d\n",port,state->port_info.ad_status,ret);
// 			if(ret >= 0)
// 			{
// 				usb_port.get_port_status.port_status_buff[port] = ret;
// 			}
// 			else
// 			{
// 				usb_port.get_port_status.port_status_buff[port] = 0;
// 			}
// 		}
// 		usleep(1000*500);
// 	}
// 	return (void *)(&result);
// }

int start_get_port_thread(void)
{
	int i = 0;
	// int fd = 0;
	// size_t stack_size = 0; //��ջ��С����
    // pthread_attr_t attr; //�߳����Խṹ�����
	//  //��ʼ���߳�����
    // int ret = pthread_attr_init(&attr);
    // if(ret != 0)
    // {
    //     perror("pthread_attr_init");
    //     return -1;
    // }
    // stack_size = 500*1024; //��ջ��С����
    // ret = pthread_attr_setstacksize(&attr, stack_size);//�����̶߳�ջ��С
    // if(ret != 0)
    // {
    //     perror("pthread_attr_getstacksize");
    //     return -1;
    // }
	for(i= 0;i<usb_port.usb_port_numb;++i)
	{
		usb_port.get_dev_basic_action.usb_dev_basic_data[i].port_info.port = i + 1;
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(i))   //�ǻ�е��
		{
			usb_port.get_port_status.port_status_buff[i] = 1;  //Ĭ�����豸
		}
		else
		{
			//pthread_create(&fd,&attr,get_port_ad_status_thread,(void *)(&(usb_port.get_dev_basic_action.usb_dev_basic_data[i].port_info.port)));
			
			_scheduled_task_open_set_size("get_port_thread",get_port_ad_status_thread,(void *)(&(usb_port.get_dev_basic_action.usb_dev_basic_data[i].port_info.port)),500,NULL,TASK_SIZE_500K);
			
		}
	}
	return 0;
}

//��ʼ�׶�
void *start_step(void *arg)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,usb_port_numb = 0;
	int effective_port = 0;
	PORT_STATE *port_state;
	port_num = *((int *)(arg));
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	//printf("port_num = %d\n",port_num);
	if(0 > port_num|| usb_port_numb < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	port_state = get_port_priority_usedstate(port_num);
	//printf("port_state->port_state_enum = %d,port_num = %d\n",port_state->port_state_enum,port_num);
	//���Ǵ�״̬�����ж˿ڵ�Դ�Ĵ�
	// if(usb_port.machine_type == 1)
	// {
	// 	effective_port = port_num/2;
	// }
	// else
	// {
	// 	effective_port = port_num;
	// }
	effective_port = port_num;
	//if(OPEN_STATE != port_state->port_state_enum)
	{
		printf("power_control_type = %d\n",usb_port.power_action.power_data[effective_port].power_control_type);
		result = power_saving_contrl_port(port_num,OPEN_STATE,PORT_CONFPRM_PRIORITY,usb_port.power_action.power_data[effective_port].power_control_type);
		//printf("result = %d\n",result);
		if(POWER_SAVING_NO_ERROR == result)
		{
			result = USB_PORT_NO_ERROR;
		}
		else
		{
			result = USB_COMMON_ERROR;
		}
	}
	return (void *)(&result);
}

//����ȷ�Ͻ׶�
void *recheck_step(void *arg)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	port_num = *((int *)(arg));
	if(0 > port_num|| usb_port_numb < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	//result = ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_GET_PORT_FILE_EXIST, port_num + 1);
	result = my_get_port_exit(port_num + 1,usb_port.app_note);
	if(0 > result)
	{
		result = WITHOUT_DEV;
	}
	else
	{
		result = WITH_DEV;
	}
	return (void *)(&result);
}

//ȷ�Ͻ׶�
void *conform_step(void *arg)
{
	int result = USB_PORT_NO_ERROR;
	return (void *)(&result);
}

int set_ignore_judge_flag(int port_num,enum IGNORE_FLAG flag)
{
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num|| usb_port_numb < port_num)
	{
		return USB_PORT_PARAMETER_ERROR;
	}
	usb_port.judge_usbport_exist.port_exist_state[port_num].ignore_judge_flag = flag;
	return USB_PORT_NO_ERROR;
}

enum IGNORE_FLAG get_ignore_judge_flag(int port_num)
{
	return usb_port.judge_usbport_exist.port_exist_state[port_num].ignore_judge_flag;
}

//�豸�γ������,���˿�δ�µ�����µ磬һЩ״̬λ����λ
void *extract_init(void *arg)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,ca_read = 0,ca_read_failed_num = 0;
	int effective_port = 0;
	unsigned short usb_app = 0;
	char busid[DF_TY_USB_BUSID]= {0x00};
	LOAD_DATA load_data;
	int usb_port_numb = 0;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}

	port_num = *((int *)(arg));

	if(0 > port_num|| usb_port_numb < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}

	if(IGNORE == get_ignore_judge_flag(port_num))
	{
		usb_port_out("get_ignore_judge_flag=%d\n",port_num);
		return (void *)(&result);
	}

	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		effective_port = port_num/2;
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(port_num))   //�ǻ�е��
		{
			return (void *)(&result);
		}
	}
	else
	{
		effective_port = port_num;
	}
	

	if(usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.usb_app == DEVICE_ROBOT_ARM_LOCAL) //��е�ۣ���ִ�аγ�����
	{
		return (void *)(&result);
	}

	printf("priority=%d,port_state_enum=%d\n",(get_port_priority_usedstate(port_num))->priority,(get_port_priority_usedstate(port_num))->port_state_enum);

	if(usb_port.load_action.load_data[effective_port].usbip_state == ATTACHED)//�Ѿ�����,��Ҫ�ر�
	{
		usb_port_out("�ر��Ѿ����ص���·,�˿ڣ�%d\n",port_num);
		load_data.port = port_num;
		usb_port.load_action.fun_load_stop((void *)(&load_data));
	}

	if(OPEN_STATE == (get_port_priority_usedstate(port_num))->port_state_enum)  //�˿ڴ��ڴ�״̬����Ҫ���йر�
	{
		//if(ATTACH_PRIORITY != (get_port_priority_usedstate(port_num))->priority)
		{
			
			result = power_saving_contrl_port(port_num,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[effective_port].power_control_type);
			usb_port_out("�˿ڣ�%d��Դ�Ѵ򿪣���Ҫ�ر�,result = %d\n",port_num,result);
			usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
			if(POWER_SAVING_NO_ERROR == result)
			{
				result = USB_PORT_NO_ERROR;
			}
			else
			{
				result = USB_COMMON_ERROR;
				return (void *)(&result);
			}
		}
		// else
		// {
		// 	result = USB_COMMON_ERROR;
		// 	return (void *)(&result);
		// }
	}
	
	printf("lbc extract_init,port_num=%d\n",port_num);
	usb_port.get_port_status.port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
	usb_port.get_port_status.last_port_status_buff[port_num] = (int)WITHOUT_DEV; //��Ϊ���豸
	usb_port.judge_usbport_exist.port_exist_state[port_num].process_step = WAIT_STEP; //��Ϊ�ȴ�
	
	usb_port.read_tax_name.read_state[port_num] = WAIT_READ;

	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(port_num))   //�ǻ�е��
		{
			return (void *)(&result);
		}
	}
	
	_lock_set(usb_port.data_lock);
	ca_read = usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.ca_read;
	ca_read_failed_num = usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.fail_num;
	usb_app = usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.usb_app;
	memset(&(usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info), 0, sizeof(struct _usb_port_infor));
	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.ca_read = ca_read;
	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.fail_num = ca_read_failed_num;
	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.port = effective_port + 1;
	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port = effective_port + 1;

	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].retry_forbidden = 0;
	usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].read_success_flag = 0;

	// usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].port_info.usb_app = usb_app;
	if(usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state == WITH_DEV)   //�˶˿�Ŀǰ���豸���Ž��ж˿ڵİγ�����
	{
		usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state = WITHOUT_DEV; //��ǰ״̬��Ϊ���豸
		logout(INFO, "system", "�˿ڼ��", "��⵽�˿ڡ�%d�����γ�\r\n",port_num + 1);
		usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].up_report = 0;
		usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].off_report = 1;
		usb_port.get_dev_basic_action.usb_dev_basic_data[effective_port].last_act = 2;
	}
	_lock_un(usb_port.data_lock);
	return (void *)(&result);
}

int fun_get_port_data_json(cJSON *data_layer,int port,int is_common)
{
	char tmp_s[1024] = {0};
	DEV_BASIC_DATA *state;
	state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1];
	memset(tmp_s, 0, sizeof(tmp_s));	
	sprintf(tmp_s, "%d", state->port);
	cJSON_AddStringToObject(data_layer, "port_num", tmp_s);
	if(is_common == 2)
	{
		cJSON_AddStringToObject(data_layer, "busid", state->port_info.busid);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", usb_port.judge_usbport_exist.port_exist_state[port - 1].now_port_state);
		cJSON_AddStringToObject(data_layer, "dev_exists", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.usb_app);
		cJSON_AddStringToObject(data_layer, "usb_app", tmp_s);
	}
	cJSON_AddStringToObject(data_layer, "name", (char *)state->port_info.ca_name);
	cJSON_AddStringToObject(data_layer, "number", (char *)state->port_info.ca_serial);
	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.dev_type);
		cJSON_AddStringToObject(data_layer, "usb_type", tmp_s);
	}

	cJSON_AddStringToObject(data_layer, "plate_num", (char *)state->port_info.sn);
	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.extension);
	cJSON_AddStringToObject(data_layer, "plate_extension", tmp_s);

	memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.plate_infos.plate_basic_info.plate_test);
	cJSON_AddStringToObject(data_layer, "plate_train", tmp_s);


	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(data_layer, "usb_vid", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(data_layer, "usb_pid", tmp_s);
	}

	if(is_common == 2)
	{
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", (int)(get_port_priority_usedstate(port - 1))->port_state_enum);
		cJSON_AddStringToObject(data_layer, "port_power", tmp_s);
		
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port_info.ad_status);
		cJSON_AddStringToObject(data_layer, "ad_status", tmp_s);

		//if (state->port_info.port_used == 1 && state->port_info.system_used == 0)
		//printf("!!!!!!!!!!!!!!!lbc state->port = %d\n",state->port);
		if(usb_port.load_action.load_data[state->port - 1].usbip_state == ATTACHED)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "1");
		}
		else
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "0");
		}

		cJSON_AddStringToObject(data_layer, "usb_share", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); 
		sprintf(tmp_s, "%d", usb_port.load_action.load_data[state->port - 1].net_type);
		cJSON_AddStringToObject(data_layer, "usb_share_mode", tmp_s);
		cJSON_AddStringToObject(data_layer, "client_id", state->port_info.client);
		cJSON_AddStringToObject(data_layer, "oper_id", state->port_info.oper_id);
		cJSON_AddStringToObject(data_layer, "user", state->port_info.user);

		if (state->port_info.ip != 0x000000)
		{
			char ip_str[50] = { 0 };
			ip2ip_asc(state->port_info.ip, ip_str);
			cJSON_AddStringToObject(data_layer, "client_ip", ip_str);
		}
		else
		{
			cJSON_AddStringToObject(data_layer, "client_ip", "");
		}
	}
	//wang 11/3 ��Ӷ���ʼʱ��ͽ���ʱ���ֶ��ϴ�
 	// cJSON_AddStringToObject(data_layer, "start_time", stream->start_time);
 	// cJSON_AddStringToObject(data_layer, "end_time", stream->end_time);

	return 0;
}



int fun_get_ter_all_data_json_jude_need_port(cJSON *data_layer,int need_port_info,int need_tax_info)
{
	char tmp_s[100] = { 0 };
	struct _ter_info ter_info;
	struct _mode_support mode_support;
	int external_delay_us = 0;
	float external_delay_ms = 0.0;
	
	memset(&ter_info, 0, sizeof(struct _ter_info));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_INFOMATION, &ter_info);
	
	memset(&mode_support, 0, sizeof(struct _mode_support));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_SUPPORT, &mode_support);


	cJSON_AddStringToObject(data_layer, "product_code", ter_info.product_code);
	cJSON_AddStringToObject(data_layer, "product_name", ter_info.product_name);
	cJSON_AddStringToObject(data_layer, "product_rename", ter_info.product_rename);
	cJSON_AddStringToObject(data_layer, "ter_id", ter_info.ter_id);	
	cJSON_AddStringToObject(data_layer, "ter_version", ter_info.ter_version);
	cJSON_AddStringToObject(data_layer, "ter_version_detail", ter_info.ter_version_detail);
	cJSON_AddStringToObject(data_layer, "ter_type", ter_info.ter_type);

	

	if (need_tax_info == 1)
	{
		cJSON_AddStringToObject(data_layer, "cpu_id", ter_info.cpu_id);
		cJSON_AddStringToObject(data_layer, "product_date", ter_info.product_date);
		cJSON_AddStringToObject(data_layer, "total_space", ter_info.total_space);
		cJSON_AddStringToObject(data_layer, "left_space", ter_info.left_space);
		cJSON_AddStringToObject(data_layer, "ram", ter_info.ram);
		cJSON_AddStringToObject(data_layer, "mem_per", ter_info.mem_per);
		cJSON_AddStringToObject(data_layer, "cpu_per", ter_info.cpu_per);
		cJSON_AddStringToObject(data_layer, "uptime", ter_info.uptime);
		cJSON_AddStringToObject(data_layer, "kernel_ver", ter_info.kernel_ver);
		// if (stream->keyfile_load_err == 1)
		// 	cJSON_AddStringToObject(data_layer, "key_file_load", "0");
		// else
			cJSON_AddStringToObject(data_layer, "key_file_load", "1");
		cJSON_AddStringToObject(data_layer, "tax_so_load", ter_info.tax_so_load);
		cJSON_AddStringToObject(data_layer, "auth_file_load", ter_info.auth_file_load);
		cJSON_AddStringToObject(data_layer, "tax_so_ver", ter_info.tax_ver);
		cJSON_AddStringToObject(data_layer, "mbc_so_ver", ter_info.mbc_ver);
		cJSON_AddStringToObject(data_layer, "need_auth_aisino", "1");
		cJSON_AddStringToObject(data_layer, "need_auth_nisec", "1");
#ifdef UKEY_AUTH
		cJSON_AddStringToObject(data_layer, "need_auth_cntax", "0");
#else
		cJSON_AddStringToObject(data_layer, "need_auth_cntax", "1");
#endif
		cJSON_AddStringToObject(data_layer, "need_auth_mengbai", "0");

		memset(tmp_s, 0, sizeof(tmp_s));
		sprintf(tmp_s, "%ld", ter_info.boot_size);
		cJSON_AddStringToObject(data_layer, "boot_size", tmp_s);

		//usb_port_out("�ն˻�����Ϣ������\n");
	}
	cJSON *manage_layer;
	cJSON_AddItemToObject(data_layer, "module_manage", manage_layer = cJSON_CreateObject());
	// cJSON_AddStringToObject(manage_layer, "eth_support", mode_support.eth_support);
	if(usb_port.machine_type == TYPE_2202 || usb_port.machine_type == TYPE_2212)
		cJSON_AddStringToObject(manage_layer, "wifi_support", "1");
	else
		cJSON_AddStringToObject(manage_layer, "wifi_support", "0");
	// cJSON_AddStringToObject(manage_layer, "4g_support", mode_support.g4_support);
	// cJSON_AddStringToObject(manage_layer, "mysql_support", mode_support.mysql_support);
	// cJSON_AddStringToObject(manage_layer, "sqlite_support", mode_support.sqlite_support);
	// cJSON_AddStringToObject(manage_layer, "ftp_support", mode_support.ftp_support);
	// cJSON_AddStringToObject(manage_layer, "ssh_support", mode_support.ssh_support);
	// cJSON_AddStringToObject(manage_layer, "ntpdate_support", mode_support.ntpdate_support);
	// cJSON_AddStringToObject(manage_layer, "tf_card_support", mode_support.ftp_support);
	// cJSON_AddStringToObject(manage_layer, "event_log_support ", mode_support.event_log_support);
	// cJSON_AddStringToObject(manage_layer, "usbshare_support", mode_support.usbshare_support);
	// cJSON_AddStringToObject(manage_layer, "device_manage_support", mode_support.device_manage_support);
	// cJSON_AddStringToObject(manage_layer, "mqtt_support", mode_support.mqtt_support);
	// cJSON_AddStringToObject(manage_layer, "tax_invoice_support", "0");
	// cJSON_AddStringToObject(manage_layer, "extend_space_support ", mode_support.extend_space_support);
	// cJSON_AddStringToObject(manage_layer, "usb_port_support", mode_support.usb_port_support);
	// cJSON_AddStringToObject(manage_layer, "careader_support", mode_support.careader_support);
	// cJSON_AddStringToObject(manage_layer, "userload_support", mode_support.userload_support);

	// cJSON_AddStringToObject(manage_layer, "ble_support", "1");
	// cJSON_AddStringToObject(manage_layer, "power_support", "1");
	// cJSON_AddStringToObject(manage_layer, "lcd_support", "0");
	// cJSON_AddStringToObject(manage_layer, "fan_support", "0");
		//usb_port_out("�ն�ģ��֧��������\n");
	
	struct _mode_enable mode_enable;
	memset(&mode_enable, 0, sizeof(struct _mode_enable));
	ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_MODE_ENABLE, &mode_enable);

	if (atoi(mode_support.eth_support) != 0)
	{
		struct _net_status net_status;
		memset(&net_status, 0, sizeof(struct _net_status));
		ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
		cJSON *eth_layer;
		cJSON_AddItemToObject(data_layer, "eth_manage", eth_layer = cJSON_CreateObject());
		if (strcmp(net_status.eth_wlan, "eth0") == 0)
		{					
			memset(tmp_s,0x00,sizeof(tmp_s));
			sprintf(tmp_s,"%.02f ms",usb_port.intranet_delay_ms);
			cJSON_AddStringToObject(eth_layer, "intranet_delay", tmp_s);
			external_delay_us = get_net_delay_external("103.27.4.61");
			external_delay_ms = external_delay_us;
			external_delay_ms = external_delay_ms/1000;
			if(external_delay_ms < 0)
				usb_port.external_delay_ms = abs(external_delay_ms);
			else
				usb_port.external_delay_ms = external_delay_ms;

			memset(tmp_s,0x00,sizeof(tmp_s));
			sprintf(tmp_s,"%.02f ms",usb_port.external_delay_ms);
			cJSON_AddStringToObject(eth_layer, "external_delay", tmp_s);
			// cJSON_AddStringToObject(eth_layer, "internet", net_status.internet);
			cJSON_AddStringToObject(eth_layer, "ip_type", net_status.ip_type);			
			cJSON_AddStringToObject(eth_layer, "ter_mac", net_status.mac);
			cJSON_AddStringToObject(eth_layer, "ip_address", net_status.ip_address);
			cJSON_AddStringToObject(eth_layer, "netmask", net_status.netmask);
			cJSON_AddStringToObject(eth_layer, "gateway", net_status.gateway);
			cJSON_AddStringToObject(eth_layer, "dns", net_status.dns);			
			if (need_tax_info == 1)
			{				
				cJSON_AddStringToObject(eth_layer, "eth_enable", mode_enable.eth_enable);
				cJSON_AddStringToObject(eth_layer, "first_net", net_status.first_net);
				cJSON_AddStringToObject(eth_layer, "eth_type", net_status.mac);
				cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "0");
			}
		}
		else
		{						
			cJSON_AddStringToObject(eth_layer, "internet", "");
			cJSON_AddStringToObject(eth_layer, "ip_type","");			
			cJSON_AddStringToObject(eth_layer, "ter_mac", "");
			cJSON_AddStringToObject(eth_layer, "ip_address", "");
			cJSON_AddStringToObject(eth_layer, "netmask", "");
			cJSON_AddStringToObject(eth_layer, "gateway", "");
			cJSON_AddStringToObject(eth_layer, "dns", "");
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "eth_enable", "0");
				cJSON_AddStringToObject(eth_layer, "first_net", "0");
				cJSON_AddStringToObject(eth_layer, "eth_type", "");
				cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "");
			}
		}
	}
	//usb_port_out("�ն����ڹ���������\n");
	if (atoi(mode_support.wifi_support) != 0)
	{
		struct _net_status net_status;
		memset(&net_status, 0, sizeof(struct _net_status));
		ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
		cJSON *eth_layer;
		cJSON_AddItemToObject(data_layer, "wifi_manage", eth_layer = cJSON_CreateObject());
		if (strcmp(net_status.eth_wlan, "wlan0") == 0)
		{
					
			cJSON_AddStringToObject(eth_layer, "internet", net_status.internet);
			cJSON_AddStringToObject(eth_layer, "ip_type", net_status.ip_type);
			cJSON_AddStringToObject(eth_layer, "ter_mac", net_status.mac);
			cJSON_AddStringToObject(eth_layer, "ip_address", net_status.ip_address);
			cJSON_AddStringToObject(eth_layer, "netmask", net_status.netmask);
			cJSON_AddStringToObject(eth_layer, "gateway", net_status.gateway);
			cJSON_AddStringToObject(eth_layer, "dns", net_status.dns);
			cJSON_AddStringToObject(eth_layer, "connected_ssid", net_status.wifi_ssid);
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "wifi_enable", mode_enable.eth_enable);
				cJSON_AddStringToObject(eth_layer, "first_net", net_status.first_net);
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "0");
			}
		}
		else
		{
				
			cJSON_AddStringToObject(eth_layer, "internet", "");
			cJSON_AddStringToObject(eth_layer, "ip_type", "");
			cJSON_AddStringToObject(eth_layer, "ter_mac", "");
			cJSON_AddStringToObject(eth_layer, "ip_address", "");
			cJSON_AddStringToObject(eth_layer, "netmask", "");
			cJSON_AddStringToObject(eth_layer, "gateway", "");
			cJSON_AddStringToObject(eth_layer, "dns", "");
			cJSON_AddStringToObject(eth_layer, "connected_ssid", "");
			if (need_tax_info == 1)
			{
				cJSON_AddStringToObject(eth_layer, "wifi_enable", "0");
				cJSON_AddStringToObject(eth_layer, "first_net", "0");
				cJSON_AddStringToObject(eth_layer, "signal_intensity", "");
				cJSON_AddStringToObject(eth_layer, "ip_conflict", "");
			}
		}
	}
	if (atoi(mode_support.g4_support) != 0)
	{
		;
	}
	if (need_port_info == 0)
	{
		return 0;
	}
	if (need_tax_info == 1)
	{

		if (atoi(mode_support.mysql_support) != 0)
		{
			struct _mysql_process mysql_process;
			memset(&mysql_process, 0, sizeof(struct _mysql_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_MYSQL_PROCESS, &mysql_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "mysql_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "mysql_enable", mode_enable.mysql_enable);
			cJSON_AddStringToObject(layer, "process_run", mysql_process.process_run);
			cJSON_AddStringToObject(layer, "port", mysql_process.port);
			cJSON_AddStringToObject(layer, "load_name", mysql_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", mysql_process.load_passwd);
			cJSON_AddStringToObject(layer, "data_path", mysql_process.data_path);
		}




		//usb_port_out("�ն�mysql����������\n");
		if (atoi(mode_support.extend_space_support) != 0)
		{
			struct _extend_space_process extend_space_process;
			memset(&extend_space_process, 0, sizeof(struct _extend_space_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_EXTEND_SPACE_PROCESS, &extend_space_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "extend_space_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "extend_space_enable", mode_enable.extend_space_enable);
			cJSON_AddStringToObject(layer, "space_size", extend_space_process.space_size);
			cJSON_AddStringToObject(layer, "mount", extend_space_process.mount);
			cJSON_AddStringToObject(layer, "mount_dir", extend_space_process.mount_dir);
		}
		//usb_port_out("�ն���չ�ڴ����������\n");
		if (atoi(mode_support.ftp_support) != 0)
		{
			struct _ftp_process ftp_process;
			memset(&ftp_process, 0, sizeof(struct _ftp_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_FTP_PROCESS, &ftp_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ftp_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ftp_enable", mode_enable.ftp_enable);
			cJSON_AddStringToObject(layer, "process_run", ftp_process.process_run);
			cJSON_AddStringToObject(layer, "port", ftp_process.port);
			cJSON_AddStringToObject(layer, "load_name", ftp_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", ftp_process.load_passwd);
			cJSON_AddStringToObject(layer, "default_path", ftp_process.default_path);
		}
		//usb_port_out("�ն�ftp����������\n");
		if (atoi(mode_support.ssh_support) != 0)
		{
			struct _ssh_process ssh_process;
			memset(&ssh_process, 0, sizeof(struct _ssh_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_SSH_PROCESS, &ssh_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ssh_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ssh_enable", mode_enable.ssh_enable);
			cJSON_AddStringToObject(layer, "process_run", ssh_process.process_run);
			cJSON_AddStringToObject(layer, "port", ssh_process.port);
			cJSON_AddStringToObject(layer, "load_name", ssh_process.load_name);
			cJSON_AddStringToObject(layer, "load_passwd", ssh_process.load_passwd);
		}
		//usb_port_out("�ն�ssh����������\n");
		if (atoi(mode_support.ntpdate_support) != 0)
		{
			struct _ntpdate_process ntpdate_process;
			memset(&ntpdate_process, 0, sizeof(struct _ntpdate_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_NTPDATE_PROCESS, &ntpdate_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "ntpdate_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "ntpdate_enable", mode_enable.ntpdate_enable);
			cJSON_AddStringToObject(layer, "process_run", ntpdate_process.process_run);
			cJSON_AddStringToObject(layer, "server", ntpdate_process.server);
			cJSON_AddStringToObject(layer, "time_now", ntpdate_process.time_now);
		}
		//usb_port_out("�ն�ntpdate����������\n");
		if (atoi(mode_support.tf_card_support) != 0)
		{
			struct _tfcard_process tfcard_process;
			memset(&tfcard_process, 0, sizeof(struct _tfcard_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_TFCARD_PROCESS, &tfcard_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tf_card_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tf_card_enable", mode_enable.tf_card_enable);
			cJSON_AddStringToObject(layer, "insert", tfcard_process.insert);
			cJSON_AddStringToObject(layer, "mount", tfcard_process.mount);
			cJSON_AddStringToObject(layer, "status", tfcard_process.status);
		}
		//usb_port_out("�ն�tfcard����������\n");
		if (atoi(mode_support.event_log_support) != 0)
		{

			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "event_log_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "event_log_enable", mode_enable.event_log_enable);
			cJSON_AddStringToObject(layer, "event_log_num", "0");
			cJSON_AddStringToObject(layer, "event_log", "");
		}
		//usb_port_out("�ն��¼���־����������\n");
		if (atoi(mode_support.usbshare_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "usbshare_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "usbshare_enable", mode_enable.usbshare_enable);
			cJSON_AddStringToObject(layer, "port", "3240");
			cJSON_AddStringToObject(layer, "stop_use", "0");
		}
		//usb_port_out("�ն�usbshare����������\n");
		if (atoi(mode_support.device_manage_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "device_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "device_manage_enable", mode_enable.device_manage_enable);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
		}
		//usb_port_out("�ն��豸�������������\n");
		if (atoi(mode_support.mqtt_support) != 0)
		{
			char mqtt_state[10] = { 0 };
			char mqtt_server[50] = { 0 };
			int mqtt_port = 0;
			char user_name[50] = { 0 };
			char user_passwd[50] = { 0 };
			cJSON *layer;
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_MQTT_USER_MEM, mqtt_server, &mqtt_port, user_name, user_passwd);

			cJSON_AddItemToObject(data_layer, "mqtt_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "mqtt_enable", mode_enable.mqtt_enable);
			cJSON_AddStringToObject(layer, "server_addr", mqtt_server);
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", mqtt_port);
			cJSON_AddStringToObject(layer, "server_port", tmp_s);
			cJSON_AddStringToObject(layer, "now_use_net", "0");
			cJSON_AddStringToObject(layer, "cert_path", "/etc");
			sprintf(mqtt_state, "1" );//lcd_print_info.mqtt_state);
			cJSON_AddStringToObject(layer, "connect_status", mqtt_state);
		}
		//usb_port_out("�ն�mqtt����������\n");
		if (atoi(mode_support.tax_invoice_support) != 0)
		{
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "tax_invoice_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "tax_invoice_enable", mode_enable.tax_invoice_enable);
		}
		//usb_port_out("�ն�˰��Ʊ����������\n");
		if (atoi(mode_support.careader_support) != 0)
		{
			struct _careader_process careader_process;
			memset(&careader_process, 0, sizeof(struct _careader_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_CAREADER_PROCESS, &careader_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "careader_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "careader_enable", mode_enable.careader_enable);
			cJSON_AddStringToObject(layer, "process_run", careader_process.process_run);
			cJSON_AddStringToObject(layer, "err_describe", careader_process.err_describe);
		}
		//usb_port_out("�ն�CA���̹���������\n");
		if (atoi(mode_support.userload_support) != 0)
		{
			struct _userload_process userload_process;
			memset(&userload_process, 0, sizeof(struct _userload_process));
			ty_ctl(usb_port.module->machine_fd, DF_MACHINE_INFOR_CM_GET_USERLOAD_PROCESS, &userload_process);
			cJSON *layer;
			cJSON_AddItemToObject(data_layer, "userload_manage", layer = cJSON_CreateObject());
			cJSON_AddStringToObject(layer, "userload_enable", mode_enable.userload_enable);
			cJSON_AddStringToObject(layer, "process_run", userload_process.process_run);
			cJSON_AddStringToObject(layer, "err_describe", userload_process.err_describe);
		}
	}
	//usb_port_out("�ն��û���¼���̹���������\n");

	int port,exit_port = 0;
	int err_num = 0;
	char usb_port_num[10] = { 0 };
	char without_dev_port_num[10] = { 0 };
	char without_dev_ports[500] = { 0 };
	char robotarm_with_dev_ports[500] = { 0 };

	int port_num = usb_port.usb_port_numb,robot_arm_num = 0;

	DEV_BASIC_DATA *state;

	//��е�����ݴ��
	cJSON *rebot_arm_layer;
	cJSON *rebot_arm_port_array;		 //���ʹ��

	// printf("111111111111\n");
	cJSON_AddItemToObject(data_layer, "robotarm_manage", rebot_arm_layer = cJSON_CreateObject());
	cJSON_AddItemToObject(rebot_arm_layer, "robotarm_info", rebot_arm_port_array = cJSON_CreateArray());
	if(usb_port.machine_type == 1)
	{
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port_num);
		cJSON_AddStringToObject(rebot_arm_layer, "robotarm_num",tmp_s);
		// printf("2222222222222\n");
		for (port = 1; port<= port_num; port++)
		{
			state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[port - 1];
			cJSON *robot_arm_port_layer;
			cJSON_AddItemToObject(rebot_arm_port_array, "dira", robot_arm_port_layer = cJSON_CreateObject());
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port);
			cJSON_AddStringToObject(robot_arm_port_layer, "port_num", tmp_s);
			cJSON_AddStringToObject(robot_arm_port_layer, "robot_arm_version", state->port_info.arm_version);
			if(strlen(state->port_info.arm_version))
				cJSON_AddStringToObject(robot_arm_port_layer, "robotarm_status", "0");
			else
				cJSON_AddStringToObject(robot_arm_port_layer, "robotarm_status", "1");
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[(port - 1)].arm_with_dev_result);
			cJSON_AddStringToObject(robot_arm_port_layer, "robotarm_with_dev_ports", tmp_s);
		}
	}
	else
	{
		for (port = 1; port<= port_num; port++)
		{
			state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[port - 1];
			if(strlen(state->port_info.arm_version))
			{
				robot_arm_num++;
				cJSON *robot_arm_port_layer;
				cJSON_AddItemToObject(rebot_arm_port_array, "dira", robot_arm_port_layer = cJSON_CreateObject());
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", port);
				cJSON_AddStringToObject(robot_arm_port_layer, "port_num", tmp_s);
				cJSON_AddStringToObject(robot_arm_port_layer, "robot_arm_version", state->port_info.arm_version);
				cJSON_AddStringToObject(robot_arm_port_layer, "robotarm_status", "0");
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[(port - 1)].arm_with_dev_result);
				cJSON_AddStringToObject(robot_arm_port_layer, "robotarm_with_dev_ports", tmp_s);
			}
		}
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", robot_arm_num);
		cJSON_AddStringToObject(rebot_arm_layer, "robotarm_num",tmp_s);
	}
	// printf("3333333333333\n");
	cJSON *usblayer;
	

	
	cJSON_AddItemToObject(data_layer, "usb_port_manage", usblayer = cJSON_CreateObject());
	
	//usb_port_out("���ն�һ����%d��USB�˿�\n");
	for (port = 1; port<= port_num; port++)
	{
		if(usb_port.machine_type == 1)
		{
			if (usb_port.judge_usbport_exist.port_exist_state[port*2 - 1].now_port_state != WITH_DEV)
			{
				//if(port%2 == 0)
				{
					//usb_port_out("%d�Ŷ˿�û���豸\n", port_count + 1);
					sprintf(without_dev_ports + strlen(without_dev_ports), "%d;", port);
					err_num = err_num + 1;
				}
			}
			//if(port%2 == 1)
			// {
			// 	if(usb_port.online_bank_action.mechanical_arm_action.arm_with_dev[port - 1].arm_with_dev_result == 1)
			// 	{
			// 		sprintf(robotarm_with_dev_ports + strlen(robotarm_with_dev_ports), "%d;", );
			// 	}
			// 	else
			// 	{

			// 	}
			// }
		}
		else
		{
			if (usb_port.judge_usbport_exist.port_exist_state[port - 1].now_port_state != WITH_DEV)
			{
				sprintf(without_dev_ports + strlen(without_dev_ports), "%d;", port);
				err_num = err_num + 1;
			}
		}
		
	}
	if(usb_port.machine_type == 1)
		sprintf(usb_port_num, "%d", port_num);
	else
		sprintf(usb_port_num, "%d", port_num);
	sprintf(without_dev_port_num, "%d", err_num);
	cJSON_AddStringToObject(usblayer, "usb_port_num", usb_port_num);
	cJSON_AddStringToObject(usblayer, "without_dev_port_num", without_dev_port_num);
	cJSON_AddStringToObject(usblayer, "without_dev_ports", without_dev_ports);
	// cJSON_AddStringToObject(usblayer, "robotarm_with_dev_ports", robotarm_with_dev_ports);


	// cJSON_AddStringToObject(usblayer, "short_circuit_ports", "");
	// cJSON_AddStringToObject(usblayer, "open_circuit_ports", "");



	cJSON *port_array;		 //���ʹ��
	
	
	cJSON_AddItemToObject(usblayer, "usb_port_info", port_array = cJSON_CreateArray());
	
	for (port = 1; port <= port_num; port++)
	{	
		state = &usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1];
		if(usb_port.machine_type == 1)
		{
			exit_port = port*2 - 1;
		}
		else
		{
			exit_port = port - 1;
		}
		if ((need_tax_info == 0) && (usb_port.judge_usbport_exist.port_exist_state[exit_port].now_port_state == WITHOUT_DEV))  //
		{
			continue;
		}

		// if(usb_port.machine_type == 1)
		// {
		// 	if(port%2 == 1)
		// 		continue;
		// }

		cJSON *port_layer;
		cJSON_AddItemToObject(port_array, "dira", port_layer = cJSON_CreateObject());
		if(usb_port.machine_type == 1)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); 
			sprintf(tmp_s, "%d", port);
		}
		else
		{
			memset(tmp_s, 0, sizeof(tmp_s)); 
			sprintf(tmp_s, "%d", port);
		}
		cJSON_AddStringToObject(port_layer, "usb_port", tmp_s);
		// cJSON_AddStringToObject(port_layer, "busid", state->port_info.busid);		
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", usb_port.judge_usbport_exist.port_exist_state[exit_port].now_port_state);
		cJSON_AddStringToObject(port_layer, "dev_exists", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", (int)(get_port_priority_usedstate(exit_port))->port_state_enum);
		cJSON_AddStringToObject(port_layer, "port_power", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.ad_status);
		// cJSON_AddStringToObject(port_layer, "ad_status", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", 0);
		// cJSON_AddStringToObject(port_layer, "dev_abnormal", tmp_s);
		// if(strlen(state->port_info.arm_version) > 0)
		// 	cJSON_AddStringToObject(port_layer, "robot_arm_version", state->port_info.arm_version);
		if (need_tax_info == 1)
		{
			if (state->port_info.dev_type == 3)
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Hid");
			}
			else if (state->port_info.dev_type == 8)
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Masstore");
			}
			else
			{
				memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "Unkown");
			}
			cJSON_AddStringToObject(port_layer, "usb_type", tmp_s);
		}
		// printf("lbc ----- vid = %d,pid = %d\n",usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1].port_info.vid,usb_port.get_dev_basic_action.usb_dev_basic_data[port - 1].port_info.pid);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%04x", state->port_info.vid);
		cJSON_AddStringToObject(port_layer, "usb_vid", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%04x", state->port_info.pid);
		cJSON_AddStringToObject(port_layer, "usb_pid", tmp_s);


		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", state->port_info.usb_app);
		//#ifndef DF_SUPPORT_CANAME				//wang 12/7 ����CA ���ϱ�usb_app
		cJSON_AddStringToObject(port_layer, "usb_app", tmp_s);
		//#endif
		if (usb_port.load_action.load_data[port - 1].usbip_state == ATTACHED)
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "1");
			cJSON_AddStringToObject(port_layer, "oper_id", usb_port.load_action.load_data[port - 1].oper_id);
			cJSON_AddStringToObject(port_layer, "user", usb_port.load_action.load_data[port - 1].user);
			
		}
		else
		{
			memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "0");
		}
		cJSON_AddStringToObject(port_layer, "usb_share", tmp_s);
		memset(tmp_s, 0, sizeof(tmp_s)); sprintf(tmp_s, "%d", (int)usb_port.load_action.load_data[port - 1].net_type);
		cJSON_AddStringToObject(port_layer, "usb_share_mode", tmp_s);
		// cJSON_AddStringToObject(port_layer, "client_id", state->port_info.client);
		if (state->port_info.ip != 0x000000)
		{
			char ip_str[50] = { 0 };
			ip2ip_asc(state->port_info.ip, ip_str);
			cJSON_AddStringToObject(port_layer, "client_ip", ip_str);
		}
		else
		{
			cJSON_AddStringToObject(port_layer, "client_ip", "");
		}

		if (usb_port.judge_usbport_exist.port_exist_state[exit_port].now_port_state != WITHOUT_DEV)
		{
			if (state->port_info.usb_app == 4)
			{
				#ifndef DF_SUPPORT_CANAME							//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��				
				cJSON *ca_layer;
				cJSON_AddItemToObject(port_layer, "ca_infos", ca_layer = cJSON_CreateObject());
				cJSON_AddStringToObject(ca_layer, "name", (char *)state->port_info.ca_name);
				cJSON_AddStringToObject(ca_layer, "number", (char *)state->port_info.ca_serial);
				// cJSON_AddStringToObject(ca_layer, "cert_file_name", "");
				// cJSON_AddStringToObject(ca_layer, "cert_file_data", "");		
				#endif		
			}
			else if ((state->port_info.usb_app == 1) || (state->port_info.usb_app == 2) || (state->port_info.usb_app == 3) || state->port_info.usb_app >= 101)
			{
				cJSON *plate_layer;
				cJSON_AddItemToObject(port_layer, "plate_infos", plate_layer = cJSON_CreateObject());
				if (need_tax_info == 0)
				{
					memset(tmp_s, 0, sizeof(tmp_s));	sprintf(tmp_s, "%d", state->port);
					cJSON_AddStringToObject(plate_layer, "port_num", tmp_s);
					cJSON_AddStringToObject(plate_layer, "busid", state->port_info.busid);
					memset(tmp_s, 0, sizeof(tmp_s));				//wang usb_app��ʾ101
					sprintf(tmp_s, "%d", state->port_info.usb_app);
					cJSON_AddStringToObject(plate_layer, "usb_app", tmp_s);

					#ifndef DF_SUPPORT_CANAME						//wang 11/18 ����CA ���ϱ�CA���ƺ�˰��	
					cJSON_AddStringToObject(plate_layer, "name", (char *)state->port_info.ca_name);
					cJSON_AddStringToObject(plate_layer, "number", (char *)state->port_info.ca_serial);
					cJSON_AddStringToObject(plate_layer, "plate_num", (char *)state->port_info.sn);
					#endif	
				}
				else
				{				
					fun_get_port_data_json(plate_layer, port, 1);
				}
			}
		}
	}
	return 0;

}

int fun_get_ter_all_data_json(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 1,1);
	return 0;
}

int fun_get_ter_all_data_json_without_port(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 0,0);
	return 0;
}

int fun_get_ter_all_data_json_without_tax(cJSON *data_layer)
{
	fun_get_ter_all_data_json_jude_need_port(data_layer, 1,0);
	return 0;
}

//�������ƹ���
int port_led_manage_thread(void *arg, int timer)
{
	DEV_BASIC_DATA *state = NULL,*robot_arm_state = NULL;
	int i = 0,now_port_led_status = 0;
	int allow_attach_flag = -1,read_flag = 0;
	//stream->timer = timer;

	// struct _net_status net_status;
	// memset(&net_status, 0, sizeof(struct _net_status));
	// ty_ctl(usb_port.module->get_net_fd, DF_GET_NET_STATE_CM_GET_NET_STATUS, &net_status);
	// if (net_status.nettime_ok != 1)
	// {
	// 	ty_ctl(usb_port.module->ty_usb_fd, now_port_led_status, i);
	// }
	if(jude_port_allow_attach() < 0)
	{
		allow_attach_flag = -1;
	}
	else
	{
		allow_attach_flag = 0;
	}
	//allow_attach_flag = -1;
	if(usb_port.machine_type == TYPE_2108)
	{
		// printf("port_led_manage_thread usb_port.machine_type = %d\n",usb_port.machine_type);
		for (i = 1; i <= usb_port.usb_port_numb; i++)
		{
			state = &usb_port.get_dev_basic_action.usb_dev_basic_data[i - 1];
			robot_arm_state = &usb_port.online_bank_action.mechanical_arm_action.arm_data[i - 1];
			read_flag = 0;
			now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
			if(1 == state->retry_forbidden) //������󣬺��̵ƽ�����˸
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S;
			}
			else
			{
				if(1 == state->read_success_flag)   //��ȡ�ɹ�
				{
					read_flag = read_flag | 0x01;
				}
				if(0 < strlen(robot_arm_state->port_info.arm_version))
				{
					read_flag = read_flag | 0x02;
				}
				if(read_flag == 0x01)  //�����
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_ON_GREN_OFF;
				}
				else if(read_flag == 0x02)  //�̵���
				{
					now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_ON;
				}
				else if(read_flag == 0x03)  //�Ƶ���
				{
					now_port_led_status = DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW;
				}
			}
			//  printf("port_led_manage_thread i=%d,read_flag = %d,now_port_led_status=%d,robot_arm_state->port_info.arm_version=%s\n",i,read_flag,now_port_led_status,robot_arm_state->port_info.arm_version);
			if (state->port_led_status != now_port_led_status)
			{
				if (ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, i) < 0)
				{
					usleep(2000);
					ty_ctl(usb_port.module->ty_usb_fd, DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF, i);
				}
				if (ty_ctl(usb_port.module->ty_usb_fd, now_port_led_status, i) < 0)
				{
					usleep(2000);
					ty_ctl(usb_port.module->ty_usb_fd, now_port_led_status, i);
				}
				state->port_led_status = now_port_led_status;
			}
		}
	}
	else
	{
		for (i = 1; i <= usb_port.usb_port_numb; i++)
		{
			state = &usb_port.get_dev_basic_action.usb_dev_basic_data[i - 1];
			if(WITH_DEV == usb_port.judge_usbport_exist.port_exist_state[i - 1].now_port_state)
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_ON_GREN_OFF;
			}
			else
			{
				now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_ON;
				if(usb_port.machine_type != 1)   //�����û�е�ۻ����д��ж�
				{
					if(allow_attach_flag < 0)
					{
						now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF;
					}
					else
					{
						now_port_led_status = DF_TY_USB_CM_PORT_RED_OFF_GREN_ON;
					}
				}
			}
			if (state->port_led_status != now_port_led_status)
			{
				if (ty_ctl(usb_port.module->ty_usb_fd, now_port_led_status, i) < 0)
				{
					usleep(2000);
					ty_ctl(usb_port.module->ty_usb_fd, now_port_led_status, i);
				}
				state->port_led_status = now_port_led_status;
			}
		}
	}
	return USB_PORT_NO_ERROR;
}


//��е�����������߳�
int upgrade_strategy_thread(void *arg, int timer)
{
	int result = -1,count = 0,i = 0,len = 0,need_update_port = 0;
	int port_array[ROBOT_ARM_NUM] = {0x00};
	char tmp_file_data[1024] = {0x00},tmp_data[32] = {0x00};
	char *p = NULL;
	//printf("enter upgrade_strategy_thread\n");
	result = access(ROBOT_ARM_UPDATE_FLAG_FILE_PATH, 0);
	if(result < 0)
	{
		return result;
	}
	result = access(ROBOT_ARM_UPDATE_VERSION_FILE_PATH, 0);
	if(result < 0)
	{
		return result;
	}
	//if(result >= 0)
	{
		read_file(ROBOT_ARM_UPDATE_FLAG_FILE_PATH,tmp_file_data,sizeof(tmp_file_data));
		memset(usb_port.upgrade_strategy.file_version,0x00,sizeof(usb_port.upgrade_strategy.file_version));
		read_file(ROBOT_ARM_UPDATE_VERSION_FILE_PATH,usb_port.upgrade_strategy.file_version,sizeof(usb_port.upgrade_strategy.file_version));
		// printf("upgrade_strategy_thread tmp_file_data = %s,usb_port.upgrade_strategy.file_version = %s\n",tmp_file_data,usb_port.upgrade_strategy.file_version);
		for (p = strtok(tmp_file_data, ",");p != NULL;p=strtok(NULL,","))
		{
			port_array[count++] = atoi(p);
		}
		for(i = 0;i<count;++i)
		{
			if(port_array[i] >=0 && port_array[i] < ROBOT_ARM_NUM)   //�˿ںŵ��ȺϷ�
			{
				usb_port.upgrade_strategy.need_update_port_num[i<=(ROBOT_ARM_NUM-1)?i:(ROBOT_ARM_NUM-1)] = port_array[i];
				// printf("port_array[%d] = %d\n",i,port_array[i]);
				// printf("usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version = %s\n",usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version);
				if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version))  //�����л�е��
				{
					if(0 < strlen(usb_port.upgrade_strategy.file_version))  //�����ļ��İ汾��Ҳ����
					{
						len = strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version) > strlen(usb_port.upgrade_strategy.file_version)?strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version):strlen(usb_port.upgrade_strategy.file_version);
						// printf("1111111111111111111\n");
						if(0 == memcmp(usb_port.online_bank_action.mechanical_arm_action.arm_data[(port_array[i] >= 1?(port_array[i] - 1):0)].port_info.arm_version,usb_port.upgrade_strategy.file_version,len)) //�汾��һ�£�����Ҫ�������б���ɾ���˶˿�
						{
							// printf("222222222222222\n");
							usb_port.upgrade_strategy.need_update_port_num[i<=(ROBOT_ARM_NUM-1)?i:(ROBOT_ARM_NUM-1)] = 0;
							usb_port.upgrade_strategy.upgrade_strategy_struct[port_array[i]].update_state = UPGRADE_STRATEGY_UPDATE_SUCCESS;
						}
						else  //�汾�Ų�һ�£���Ҫ����
						{
							// printf("33333333333333333\n");
							if(0 == get_port_used_state(port_array[i] + 1))
							{
								// printf("44444444444444444\n");
								if(usb_port.upgrade_strategy.upgrade_strategy_struct[port_array[i]].update_state != UPGRADE_STRATEGY_UPDATING)
								{
									// printf("55555555555555555\n");
									usb_port.online_bank_action.mechanical_arm_action.arm_update((void *)(&port_array[i])); //��е������
									usb_port.upgrade_strategy.upgrade_strategy_struct[port_array[i]].update_state = UPGRADE_STRATEGY_UPDATING;
								}
							}
							else
							{
								printf("upgrade_strategy_thread get_port_used_state != 0,port_array[i] = %d\n",port_array[i]);
							}
						}
					}
				}
			}
		}
		memset(tmp_file_data,0x00,sizeof(tmp_file_data));
		delete_file(ROBOT_ARM_UPDATE_FLAG_FILE_PATH);  //ɾ���ļ�
		
		for(i = 0;i<count;++i)
		{
			if(usb_port.upgrade_strategy.need_update_port_num[i] > 0)
			{
				memset(tmp_data,0x00,sizeof(tmp_data));
				sprintf(tmp_data,"%d,",usb_port.upgrade_strategy.need_update_port_num[i]);
				strcat(tmp_file_data,tmp_data);
				need_update_port++;
			}
		}
		if(need_update_port > 0)
			write_file(ROBOT_ARM_UPDATE_FLAG_FILE_PATH,tmp_file_data,strlen(tmp_file_data));
	}
	return USB_PORT_NO_ERROR;
}

//��ȡ��Կ�ļ�
int get_cert_json_file(unsigned char *key_zlib_json, char **cert_json)
{
	int result = 0;
	char enc_cert[1024 * 1024] = { 0 };
	int enc_cert_len;
	//�жϼ�����Ȩ�ļ��Ƿ����   
	result = access(AUTH_CERT_FILE_PATH, 0);
	if (result < 0)
	{
		usb_port_out("��Ȩ�ļ�������,�޷�ʹ��˰����ع���\n");
		return -1;
	}
	//��ȡ��Ȩ�ļ�
	enc_cert_len = _t_file_c_read(AUTH_CERT_FILE_PATH, enc_cert, 1024 * 1024);
	if (enc_cert_len < 10)
	{
		usb_port_out("��Ȩ�ļ���ȡʧ��,�ļ��쳣\n");
		return -2;
	}
	//����������Ȩ�ļ�
	int cert_len;
	result = mb_dec_file_function(key_zlib_json, enc_cert, enc_cert_len, cert_json, &cert_len);
	if (result != 0)
	{
		printf("lbc mb_dec_file_function failed result = %d\n",result);
		return result;
	}
	usb_port_out("get_cert_json_file cert_len=%d\n",cert_len);
	return result;
}

// int fun_deal_usb_port_power(int port_num, PORT_STATE_ENUM port_state)
// {
// 	printf("enter fun_deal_usb_port_power,port_num=%d\n",port_num);
// 	int result = USB_PORT_NO_ERROR;
// 	int effective_port = 0,usb_port_numb = 0;
// 	LOAD_DATA load_data;
// 	if(usb_port.machine_type == TYPE_2303)
// 	{
// 		usb_port_numb = usb_port.usb_port_numb * 2;
// 	}
// 	else
// 	{
// 		usb_port_numb = usb_port.usb_port_numb;
// 	}
// 	if(0 > port_num|| usb_port_numb < port_num)
// 		return USB_PORT_PARAMETER_ERROR;
// 	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
// 	{
// 		effective_port = port_num/2;
// 		if(0 == get_robot_arm_class()->fun_judge_robot_arm(port_num))   //�ǻ�е��
// 		{
// 			return -10;
// 		}
// 	}
// 	else
// 	{
// 		effective_port = port_num;
// 	}
// 	if(RETART_ACTION != port_state)
// 	{
// 		if(CLOSE_STATE == port_state && ATTACHED == usb_port.load_action.load_data[effective_port].usbip_state)
// 		{
// 			usb_port_out("fun_deal_usb_port_power 11111\n");
// 			load_data.port = port_num;
// 			usb_port.load_action.fun_load_stop((void *)(&load_data));
// 		}
// 		else if(OPEN_STATE == port_state)  //��Դ��״̬�£�����̻߳���������豸��⣬�˶�ʱ�䲻���
// 		{
// 			set_ignore_judge_flag(port_num,IGNORE); //���ú��԰��豸���
// 			result = power_saving_contrl_port(port_num,port_state,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
// 			usb_port_out("fun_deal_usb_port_power 22222,result=%d\n",result);
// 			usleep(4*1000*1000);
// 			set_ignore_judge_flag(port_num,NO_IGNORE); //����Ϊ�����԰��豸���
// 		}
// 		else if(CLOSE_STATE == port_state)
// 		{
// 			printf("power_control_type = %d\n",usb_port.power_action.power_data[port_num].power_control_type);
// 			result = power_saving_contrl_port(port_num,port_state,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
// 			usb_port_out("fun_deal_usb_port_power 121212,result=%d\n",result);
// 		}
// 	}
// 	else
// 	{

// 		if(ATTACHED == usb_port.load_action.load_data[effective_port].usbip_state)
// 		{
// 			load_data.port = port_num;
// 			usb_port.load_action.fun_load_stop((void *)(&load_data));
// 		}
// 		set_ignore_judge_flag(port_num,IGNORE); //���ú��԰��豸���

// 		result = power_saving_contrl_port(port_num,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
// 		usb_port_out("fun_deal_usb_port_power 3333,result=%d\n",result);
// 		usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
// 		usleep(3*1000*1000);
		
// 		result = power_saving_contrl_port(port_num,OPEN_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
// 		usb_port_out("fun_deal_usb_port_power 44444,result=%d\n",result);
// 		usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
// 		usleep(3*1000*1000);
// 		set_ignore_judge_flag(port_num,NO_IGNORE); //����Ϊ�����԰��豸���
// 	}
// 	printf("exit fun_deal_usb_port_power\n");
// 	return result;
// }


int fun_deal_usb_port_power(int port_num, PORT_STATE_ENUM port_state)
{
	printf("enter fun_deal_usb_port_power,port_num=%d\n",port_num);
	int result = USB_PORT_NO_ERROR;
	int effective_port = 0,usb_port_numb = 0;
	LOAD_DATA load_data;
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num|| usb_port_numb < port_num)
		return USB_PORT_PARAMETER_ERROR;
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		effective_port = port_num/2;
		if(0 == get_robot_arm_class()->fun_judge_robot_arm(port_num))   //�ǻ�е��
		{
			return -10;
		}
	}
	else
	{
		effective_port = port_num;
	}
	if(RETART_ACTION != port_state)
	{
		if(CLOSE_STATE == port_state && ATTACHED == usb_port.load_action.load_data[effective_port].usbip_state)
		{
			usb_port_out("fun_deal_usb_port_power 11111\n");
			load_data.port = port_num;
			usb_port.load_action.fun_load_stop((void *)(&load_data));
		}
		else if(OPEN_STATE == port_state)  //��Դ��״̬�£�����̻߳���������豸��⣬�˶�ʱ�䲻���
		{
			set_ignore_judge_flag(port_num,IGNORE); //���ú��԰��豸���
			result = power_saving_contrl_port(port_num,port_state,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
			usb_port_out("fun_deal_usb_port_power 22222,result=%d\n",result);
			usleep(4*1000*1000);
			set_ignore_judge_flag(port_num,NO_IGNORE); //����Ϊ�����԰��豸���
		}
		else if(CLOSE_STATE == port_state)
		{
			printf("power_control_type = %d\n",usb_port.power_action.power_data[port_num].power_control_type);
			result = power_saving_contrl_port(port_num,port_state,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
			usb_port_out("fun_deal_usb_port_power 121212,result=%d\n",result);
		}
	}
	else
	{
		if(ATTACHED == usb_port.load_action.load_data[effective_port].usbip_state)
		{
			load_data.port = port_num;
			usb_port.load_action.fun_load_stop((void *)(&load_data));
		}
		result = power_saving_contrl_port(port_num,CLOSE_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
		usb_port_out("fun_deal_usb_port_power 3333,result=%d\n",result);
		usb_port.power_action.power_data[port_num].power_off_time = get_time_sec();
		usleep(3*1000*1000);
		set_ignore_judge_flag(port_num,IGNORE); //���ú��԰��豸���
		result = power_saving_contrl_port(port_num,OPEN_STATE,HIGHEST_PRIORITY,usb_port.power_action.power_data[port_num].power_control_type);
		usb_port_out("fun_deal_usb_port_power 44444,result=%d\n",result);
		set_ignore_judge_flag(port_num,NO_IGNORE); //����Ϊ�����԰��豸���
		usb_port.judge_usbport_exist.judge_exist_fun.fun_init((void *)(&port_num));
		usleep(4*1000*1000);
		// set_ignore_judge_flag(port_num,NO_IGNORE); //����Ϊ�����԰��豸���
	}
	printf("exit fun_deal_usb_port_power\n");
	return result;
}

int check_port_auth_info(KEY_AUTH_STATE *stream)
{
	char auth[10] = { 0 };
	cJSON *arrayItem, *item;
	usb_port_out("##############check_port_auth_info#################\n");
	if (stream->cert_json == NULL)
	{
		usb_port_out("��Ȩ�ļ�Ϊ��\n");
		return -1;
	}

	char version[129] = { 0 };
	char serial_num[129] = { 0 };
	char name[129] = { 0 };

	usb_port_out("��Ȩ�ļ�����:%s\n", stream->cert_json);

	cJSON *root = cJSON_Parse(stream->cert_json);
	if (!root)
	{
		usb_port_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		return -1;
	}
	item = cJSON_GetObjectItem(root, "version");
	if (get_json_value_can_not_null(item, version, 0, 128) < 0) {
		usb_port_out("������Ȩ�ļ��汾��ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "serial_num");
	if (get_json_value_can_not_null(item, serial_num, 0, 128) < 0) {
		usb_port_out("������Ȩ�ļ����к�ʧ��\n");
		goto End;
	}

	item = cJSON_GetObjectItem(root, "name");
	if (get_json_value_can_not_null(item, name, 0, 128) < 0) {
		usb_port_out("������Ȩ�ļ�����ʧ��\n");
		goto End;
	}

	cJSON *object_share = cJSON_GetObjectItem(root, "usbshare");
	if (object_share == NULL)
	{
		usb_port_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	int size = 0;
	size = cJSON_GetArraySize(object_share);
	if (size < 1)
	{
		usb_port_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}

	arrayItem = cJSON_GetArrayItem(object_share, 0);
	if (arrayItem == NULL)
	{
		usb_port_out("Error before: [%s]\n", cJSON_GetErrorPtr());
		goto End;
	}
	memset(auth, 0, sizeof(auth));
	item = cJSON_GetObjectItem(arrayItem, "auth");
	if (get_json_value_can_not_null(item, auth, 0, 10) < 0) {
		usb_port_out("����usbshare authʧ��\n");
		goto End;
	}
	usb_port_out("��ȡ����usbshare auth Ϊ:%s\n", auth);
	if (memcmp(auth, "whole", 5) != 0)
	{
		cJSON_Delete(root);
		return -1;
	}
	char start_time[40] = { 0 };
	item = cJSON_GetObjectItem(arrayItem, "start_time");

	if (get_json_value_can_not_null(item, start_time, 0, 32) < 0) {
		usb_port_out("����usbshare start_timeʧ��\n");
		goto End;
	}
	strcpy(stream->start_time, start_time);

	char end_time[40] = { 0 };

	item = cJSON_GetObjectItem(arrayItem, "end_time");

	if (get_json_value_can_not_null(item, end_time, 0, 32) < 0) {
		usb_port_out("����usbshare end_timeʧ��\n");
		goto End;
	}

	strcpy(stream->end_time, end_time);

	usb_port_out("��ȡ����start_timeΪ:%s\n", stream->start_time);

	usb_port_out("��ȡ����end_timeΪ:%s\n", stream->end_time);
End:
	cJSON_Delete(root);
	return 0;
}

void *fun_upload_auth_cert_file(void *args)
{
	int i = 0;
	get_cert_json_file(usb_port.key_auth_state.key_s, &usb_port.key_auth_state.cert_json);
	check_port_auth_info(&(usb_port.key_auth_state));
	//���ж˿������ϱ�
	return USB_PORT_NO_ERROR;
}

void *fun_query_auth_file(void *args)
{
	int result = USB_PORT_NO_ERROR;
	if (usb_port.key_auth_state.cert_json == NULL)
	{
		result = USB_PORT_NULL_FUN;
		usb_port_out("��Ȩ�ļ�Ϊ��\n");
		return (void *)(&result);
	}
	*(char **)args = usb_port.key_auth_state.cert_json;
	return (void *)(&result);
}

int get_port_used_state(int port_num)
{
	//printf("get_port_used_state port_num=%d,usb_port.load_action.load_data[(port_num - 1) >= 0?(port_num - 1):0].usbip_state = %d\n",port_num,usb_port.load_action.load_data[(port_num - 1) >= 0?(port_num - 1):0].usbip_state);
	return usb_port.load_action.load_data[(port_num - 1) >= 0?(port_num - 1):0].usbip_state;
}

void *check_port_status(void *args)
{
	int result = USB_PORT_NO_ERROR,i = 0;
	for(i = 0;i < usb_port.usb_port_numb;++i)
	{
		if(ATTACHED == usb_port.load_action.load_data[i].usbip_state)
		{
			result = USB_COMMON_ERROR;
			return (void *)(&result);
		}
	}
	return (void *)(&result);
}

//����һ�λ�е��
void *start_robot_arm(void *args)
{
	char version[32] = {0x00};
	int result = USB_PORT_NO_ERROR,port_num = 0,usb_port_numb = 0;
	port_num = *((int *)args);
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	printf("start_robot_arm port_num = %d\n",port_num);
	if(0 > port_num|| (usb_port_numb) < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	// if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
		result = usb_port.online_bank_action.mechanical_arm_action.class->fun_arm_action(port_num);
	// else
	// 	result = USB_COMMON_ERROR;
	return (void *)(&result);
}

//����һ�λ�е������
void *update_robot_arm(void *args)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,usb_port_numb = 0;
	port_num = *((int *)args) - 1;
	// printf("update_robot_arm ,port= %d\n",port_num);
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num|| (usb_port_numb) < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
		result = usb_port.online_bank_action.mechanical_arm_action.class->fun_start_update(port_num,usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num].port_info.arm_version,&(usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].up_report));
	else
	{
		if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num].port_info.arm_version))
		{
			result = usb_port.online_bank_action.mechanical_arm_action.class->fun_start_update(port_num,usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num].port_info.arm_version,&(usb_port.get_dev_basic_action.usb_dev_basic_data[port_num].up_report));
		}
		else
		{
			result = USB_COMMON_ERROR;
		}
	}
	// printf("update_robot_arm ,port= %d,result = %d\n",port_num,result);
	return (void *)(&result);
}

// void *get_robot_arm_update_state(void *args)
// {
// 	int result = USB_PORT_NO_ERROR,port_num = 0;
// 	port_num = *((int *)args) - 1;
// 	printf("v ,port= %d\n",port_num);
// }

void *check_port_with_num(void *args)
{
	int result = USB_PORT_NO_ERROR,port_num = 0,usb_port_numb = 0;
	port_num = *((int *)args);
	if(usb_port.machine_type == TYPE_2303)
	{
		usb_port_numb = usb_port.usb_port_numb * 2;
	}
	else
	{
		usb_port_numb = usb_port.usb_port_numb;
	}
	if(0 > port_num|| (usb_port_numb) < port_num)
	{
		result = USB_PORT_PARAMETER_ERROR;
		return (void *)(&result);
	}
	if(1 == (*((int *)check_with_robot_arm(NULL))))  //�Ǵ���е�۵Ļ���
	{
		if(0 == usb_port.online_bank_action.mechanical_arm_action.class->fun_judge_robot_arm(port_num))  //�ǻ�е��
		{
			if(0 < strlen(usb_port.online_bank_action.mechanical_arm_action.arm_data[port_num].port_info.arm_version))
			{
				result = USB_PORT_NO_ERROR;
			}
			else
			{
				result = USB_COMMON_ERROR;
			}
		}
		else
		{
			if(WITH_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state)
			{
				result = USB_PORT_NO_ERROR;
			}
			else
			{
				result = USB_COMMON_ERROR;
			}
		}
	}
	else
	{
		if(WITH_DEV == usb_port.judge_usbport_exist.port_exist_state[port_num].now_port_state)
		{
			result = USB_PORT_NO_ERROR;
		}
		else
		{
			result = USB_COMMON_ERROR;
		}
	}
	return (void *)(&result);
}
