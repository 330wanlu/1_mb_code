#定义使用的嵌入式编译器
CROSS=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-
#GCC编译器
CC=gcc 
AR=ar
SSTRP=strip
#飞思卡尔编译器
#CC=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc 
#AR=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-ar
#三星S5PV210编译器
#CC=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
#AR=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-ar
CFLAGS= -Wall -fPIC -g -c  
#可执行文件
INCS=-I/usr/local/release-include-lib/include-6-1-16
LIBS=-L/usr/local/release-include-lib/lib-arm-tassl-6-1-16
INCLUDES=$(INCS) -DWITH_TLS -DMQTT_CTL -DDF_2216_1

#包含的文件
software=_app_file.o _lib_file.o _mb_tools.o
linux_s=_lock.o _task.o _queue.o _t_pid.o  _usb_file.o 
manage=_err.o  _ma_id.o  _process_manage.o
file=_file.o _file_ac.o _mem_oper.o
core=_seria.o _socket_sev.o _switch_dev.o _ty_usb.o \
		_ty_pd.o _event_file.o _deploy.o _ty_usb_m.o _machine_infor.o _get_net_state.o _register.o _get_net_time.o _udp_sev.o \
		_tran_data.o _socket_client.o _vpn_set.o _lcd_state.o	_deal_cmd_newshare.o _m_server.o _invoice_transmit.o   _bluetooth.o
bsp=_usart.o 
linux_sys=_linux_ip.o _ftp_n.o _file_oper.o mqtt_client.o mqtt_share.o _mb_sql.o _http_reg.o
app=_main.o _load.o  _app.o

EXENAME=mb_usbshare.bin
MB_TYPE=MB_2216
MB_VERSION=1.1.6.1
RUNNAME=mb_usbshare_n

OBJS=$(software) $(linux_s) $(manage) $(file) $(core) $(bsp) $(linux_sys) $(app) 
HEADPATH=../
EXEPATH=exe/


 
.PHONY : all									 
#all : clean $(LIBNAME).so cleanobj $(LIBNAME).a $(EXENAME)
all : $(EXENAME)

RPATH = -Wl,-rpath='/lib/preloadable_libiconv.so'

LDFLAGS += -Wl,-Bdynamic -lpthread -lrt  -lmosquitto -lmpfr -lgmp -lssl -lmysqlclient -lcrypto -lz -ldl -lm -lmb_clib -lusb-1.0  -levent -lsqlite3 -liconv $(RPATH) $(LIBS)

$(EXENAME) : $(OBJS)  
	$(CROSS)$(CC) -std=gnu99 -o $(EXENAME) $(OBJS) $(LDFLAGS)
	$(CROSS)$(SSTRP) $(EXENAME)
	cp $(EXENAME) $(EXEPATH)
	mb_makebin $(EXENAME) $(MB_TYPE) $(MB_VERSION)
	cp $(RUNNAME) $(EXEPATH)
					   

#software包含的文件
_code_s.o : $(HEADPATH)software/_code_s.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_code_s.c

_xml.o : $(HEADPATH)software/_xml.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_xml.c
	
_utf82gbk.o : $(HEADPATH)software/_utf82gbk.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_utf82gbk.c

_algorithm.o : $(HEADPATH)software/_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_algorithm.c

_app_file.o : $(HEADPATH)software/_app_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_app_file.c

_des.o : $(HEADPATH)software/_des.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_des.c
	
_base64.o : $(HEADPATH)software/_base64.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_base64.c

_confl.o : $(HEADPATH)software/_confl.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_confl.c

cJSON.o : $(HEADPATH)software/cJSON.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/cJSON.c

_lib_file.o : $(HEADPATH)software/_lib_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_lib_file.c

_mb_tools.o : $(HEADPATH)software/_mb_tools.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_mb_tools.c

_zlib.o : $(HEADPATH)software/_zlib.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_zlib.c

_Date.o : $(HEADPATH)software/_Date.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_Date.c

#linux 包含的文件
_timer.o : $(HEADPATH)linux_s/_timer.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_timer.c
												   
_lock.o : $(HEADPATH)linux_s/_lock.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_lock.c

_task.o : $(HEADPATH)linux_s/_task.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_task.c
	
_queue.o : $(HEADPATH)linux_s/_queue.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_queue.c	

_t_pid.o : $(HEADPATH)linux_s/_t_pid.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_t_pid.c

_file_pro.o : $(HEADPATH)linux_s/_file_pro.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_file_pro.c

_t_file_c.o : $(HEADPATH)linux_s/_t_file_c.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_t_file_c.c

_usb_file.o : $(HEADPATH)linux_s/_usb_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_usb_file.c

_folder.o : $(HEADPATH)linux_s/_folder.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_s/_folder.c





#manage 包含的文件
_log.o : $(HEADPATH)manage/_log.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)manage/_log.c

_err.o : $(HEADPATH)manage/_err.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)manage/_err.c
	
_ma_id.o : $(HEADPATH)manage/_ma_id.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)manage/_ma_id.c	

_log_file.o : $(HEADPATH)manage/_log_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)manage/_log_file.c	

_process_manage.o : $(HEADPATH)manage/_process_manage.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)manage/_process_manage.c	
	

#file 包含的文件
_file_ac.o : $(HEADPATH)file/_file_ac.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)file/_file_ac.c
	
_file.o : $(HEADPATH)file/_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)file/_file.c

_mem_oper.o : $(HEADPATH)file/_mem_oper.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)file/_mem_oper.c

#core 包含的文件
_bluetooth.o : $(HEADPATH)core/_bluetooth.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_bluetooth.c
_seria.o : $(HEADPATH)core/_seria.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_seria.c

_stm32_file.o : $(HEADPATH)core/_stm32_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_stm32_file.c

_socket_m.o : $(HEADPATH)core/_socket_m.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_socket_m.c	
	
_port_core_note.o : $(HEADPATH)core/_port_core_note.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_port_core_note.c	

_socket_sev.o : $(HEADPATH)core/_socket_sev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_socket_sev.c	

_ty_usbip.o : $(HEADPATH)core/_ty_usbip.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_ty_usbip.c	

_switch_dev.o : $(HEADPATH)core/_switch_dev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_switch_dev.c	

_ty_usb.o : $(HEADPATH)core/_ty_usb.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_ty_usb.c	

_ty_pd.o : $(HEADPATH)core/_ty_pd.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_ty_pd.c	

_event_file.o : $(HEADPATH)core/_event_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_event_file.c	

_deploy.o : $(HEADPATH)core/_deploy.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_deploy.c	

_invoice_transmit.o : $(HEADPATH)core/_invoice_transmit.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_invoice_transmit.c	
	
_ty_usb_m.o : $(HEADPATH)core/_ty_usb_m.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_ty_usb_m.c	
	
_machine_infor.o : $(HEADPATH)core/_machine_infor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_machine_infor.c		

_get_net_state.o: $(HEADPATH)core/_get_net_state.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_get_net_state.c		

_socket_client.o: $(HEADPATH)core/_socket_client.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_socket_client.c
	
_register.o: $(HEADPATH)core/_register.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_register.c	

_get_net_time.o: $(HEADPATH)core/_get_net_time.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_get_net_time.c	

_udp_sev.o: $(HEADPATH)core/_udp_sev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_udp_sev.c	

_tran_data.o: $(HEADPATH)core/_tran_data.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_tran_data.c	

_vpn_set.o: $(HEADPATH)core/_vpn_set.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_vpn_set.c	

_device_manage.o: $(HEADPATH)core/_device_manage.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_device_manage.c	

_lcd_state.o: $(HEADPATH)core/_lcd_state.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_lcd_state.c	

_deal_cmd_newshare.o: $(HEADPATH)core/_deal_cmd_newshare.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_deal_cmd_newshare.c

_m_server.o: $(HEADPATH)core/_m_server.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)core/_m_server.c

#bsp包含的文件
	
_usart.o : $(HEADPATH)bsp/_usart.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)bsp/_usart.c	

#linux_sys 包含文件
_linux_ip.o : $(HEADPATH)linux_sys/_linux_ip.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_linux_ip.c		

_ftp_n.o : $(HEADPATH)linux_sys/_ftp_n.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_ftp_n.c		

_file_oper.o : $(HEADPATH)linux_sys/_file_oper.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_file_oper.c		

_usb_names.o : $(HEADPATH)linux_sys/_usb_names.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_usb_names.c

mqtt_client.o : $(HEADPATH)linux_sys/mqtt_client.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/mqtt_client.c

mqtt_share.o : $(HEADPATH)linux_sys/mqtt_share.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/mqtt_share.c

_mb_sql.o : $(HEADPATH)linux_sys/_mb_sql.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_mb_sql.c

_http_reg.o: $(HEADPATH)linux_sys/_http_reg.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_http_reg.c

#app包含的文件
_main.o : _main.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _main.c
	
_load.o : _load.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _load.c	
	
_app.o : _app.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _app.c	









.PHONY : clean	
clean :
	-rm -f *.o $(EXENAME) $(RUNNAME) *.a *.so *.d $(EXEPATH)$(EXENAME) $(EXEPATH)$(RUNNAME)

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d
