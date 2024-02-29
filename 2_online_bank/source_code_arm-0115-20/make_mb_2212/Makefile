#定义使用的嵌入式编译器
CROSS=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-
#GCC编译器
CC=gcc
AR=ar
SSTRP=strip
CFLAGS= -Wall -fPIC -g -c
#可执行文件
INCS=-I/usr/local/release-include-lib/include-5-10-2 -I../ -I../third_pard/inc -I../sys_relevant/serial/inc -I../sys_relevant/common/inc -I../sys_relevant/file/inc \
-I../sys_relevant/net/inc  -I../sys_relevant/task/inc -I../application/app/inc -I../application/imitate_usb/inc -I../application/power_saving/inc \
-I../application/protocol/inc -I../application/test_mode/inc -I../application/update/inc -I../application/usb_port/inc

LIBS=-L/usr/local/release-include-lib/lib-arm-tassl-5-10-2 -L/usr/local/lib-arm-libusb
INCLUDES=$(INCS) -DWITH_TLS -DMQTT_CTL -DTEST_MODEL -DDF_SUPPORT_CA #-DLBC_TEST
#-DDF_1906_1 -DDF_SUPPORT_CANAME -DDF_SUPPORT_CA

#app相关路径
APP_PATH=../application/app/src/
IMITATE_USB_PATH=../application/imitate_usb/src/
POWER_SAVE_PATH=../application/power_saving/src/
PROTOCOL_PATH=../application/protocol/src/
TEST_MODE_PATH=../application/test_mode/src/
UPDATE_PATH=../application/update/src/
USBPORT_PATH=../application/usb_port/src/
MAIN_PATH=../application/

#sys相关路径
COMMON_PATH=../sys_relevant/common/src/
FILE_PATH=../sys_relevant/file/src/
NET_PATH=../sys_relevant/net/src/
SERIAL_PATH=../sys_relevant/serial/src/
TASK_PATH=../sys_relevant/task/src/

#包含的文件
app=process_manage.o module.o
imitate_usb=imitate_usb.o
power_save=power_saving.o
protocol=deal_cmd_newshare.o deploy.o get_net_state.o get_net_time.o machine_infor.o register.o tax_interface.o
test_mode=test.o
update=update.o
download=download.o
usb_port=usb_port.o ty_usb.o tran_data.o robot_arm.o
common=lock.o mb_tools.o timer.o
file=file_ac.o file_oper.o file.o mem_oper.o usb_file.o
net=ftp_n.o http_reg.o linux_ip.o m_server.o mqtt_client.o mqtt_share.o socket_client.o socket_sev.o udp_sev.o https_client.o ip_check.o
seria=bluetooth.o lcd_state.o seria.o switch_dev.o usart.o
task=delay_task.o scheduled_task.o

EXENAME=mb_usbshare.bin
MB_TYPE=MB_2212
MB_VERSION=4.0.0.3
RUNNAME=mb_usbshare_n

TYPE_VERSION=4-0-0-3
TYPE_PATH=2212

OBJS=$(imitate_usb) $(power_save) $(protocol) $(test_mode) $(update) $(download) $(usb_port) $(common) $(file) $(net) $(seria) $(task) $(app) main.o
HEADPATH=../
EXEPATH=exe/

.PHONY : all									 
#all : clean $(LIBNAME).so cleanobj $(LIBNAME).a $(EXENAME)
all : $(EXENAME)

RPATH = -Wl,-rpath='/lib/preloadable_libiconv.so'

#LDFLAGS += -Wl,-Bdynamic -lpthread -lrt  -lmosquitto -lmpfr -lgmp -lssl -lmysqlclient -lcrypto -lz -ldl -lm -lmb_clib -lusb-1.0  -levent -lsqlite3 -liconv $(RPATH) $(LIBS)

LDFLAGS += -Wl,-Bdynamic -lpthread -lrt  -lmosquitto  -lssl  -lcrypto -lz -ldl -lm -lmb_clib -lusb-1.0  -levent -levent_openssl  -liconv $(RPATH) $(LIBS)

$(EXENAME) : $(OBJS)
	$(CROSS)$(CC) -std=gnu99 -o $(EXENAME) $(EXEPATH)* $(LDFLAGS)
	$(CROSS)$(SSTRP) $(EXENAME)
#cp $(EXENAME) $(EXEPATH)
	mb_makebin $(EXENAME) $(MB_TYPE) $(MB_VERSION)
	cp $(RUNNAME) ../release_version/$(TYPE_PATH)
	rm ../release_version/pack/$(TYPE_VERSION)_$(TYPE_PATH).zip
	zip -q -j ../release_version/pack/$(TYPE_VERSION)_$(TYPE_PATH).zip ../release_version/$(TYPE_PATH)/*

process_manage.o : $(APP_PATH)process_manage.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(APP_PATH)process_manage.c -o $(EXEPATH)$@

module.o : $(APP_PATH)module.c ../opt.h
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(APP_PATH)module.c -o $(EXEPATH)$@

imitate_usb.o : $(IMITATE_USB_PATH)imitate_usb.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(IMITATE_USB_PATH)imitate_usb.c -o $(EXEPATH)$@

power_saving.o : $(POWER_SAVE_PATH)power_saving.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(POWER_SAVE_PATH)power_saving.c -o $(EXEPATH)$@

deal_cmd_newshare.o : $(PROTOCOL_PATH)deal_cmd_newshare.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)deal_cmd_newshare.c -o $(EXEPATH)$@

deploy.o : $(PROTOCOL_PATH)deploy.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)deploy.c -o $(EXEPATH)$@

get_net_state.o : $(PROTOCOL_PATH)get_net_state.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)get_net_state.c -o $(EXEPATH)$@

get_net_time.o : $(PROTOCOL_PATH)get_net_time.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)get_net_time.c -o $(EXEPATH)$@

machine_infor.o : $(PROTOCOL_PATH)machine_infor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)machine_infor.c -o $(EXEPATH)$@

register.o : $(PROTOCOL_PATH)register.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)register.c -o $(EXEPATH)$@

tax_interface.o : $(PROTOCOL_PATH)tax_interface.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(PROTOCOL_PATH)tax_interface.c -o $(EXEPATH)$@

test.o : $(TEST_MODE_PATH)test.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(TEST_MODE_PATH)test.c -o $(EXEPATH)$@

update.o : $(UPDATE_PATH)update.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(UPDATE_PATH)update.c -o $(EXEPATH)$@

download.o : $(UPDATE_PATH)download.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(UPDATE_PATH)download.c -o $(EXEPATH)$@

usb_port.o : $(USBPORT_PATH)usb_port.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(USBPORT_PATH)usb_port.c -o $(EXEPATH)$@

ty_usb.o : $(USBPORT_PATH)ty_usb.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(USBPORT_PATH)ty_usb.c -o $(EXEPATH)$@

tran_data.o : $(USBPORT_PATH)tran_data.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(USBPORT_PATH)tran_data.c -o $(EXEPATH)$@

robot_arm.o : $(USBPORT_PATH)robot_arm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(USBPORT_PATH)robot_arm.c -o $(EXEPATH)$@

lock.o : $(COMMON_PATH)lock.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(COMMON_PATH)lock.c -o $(EXEPATH)$@

mb_tools.o : $(COMMON_PATH)mb_tools.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(COMMON_PATH)mb_tools.c -o $(EXEPATH)$@

timer.o : $(COMMON_PATH)timer.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(COMMON_PATH)timer.c -o $(EXEPATH)$@

file_ac.o : $(FILE_PATH)file_ac.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(FILE_PATH)file_ac.c -o $(EXEPATH)$@

file_oper.o : $(FILE_PATH)file_oper.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(FILE_PATH)file_oper.c -o $(EXEPATH)$@

file.o : $(FILE_PATH)file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(FILE_PATH)file.c -o $(EXEPATH)$@

mem_oper.o : $(FILE_PATH)mem_oper.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(FILE_PATH)mem_oper.c -o $(EXEPATH)$@

usb_file.o : $(FILE_PATH)usb_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(FILE_PATH)usb_file.c -o $(EXEPATH)$@

ftp_n.o : $(NET_PATH)ftp_n.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)ftp_n.c -o $(EXEPATH)$@

http_reg.o : $(NET_PATH)http_reg.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)http_reg.c -o $(EXEPATH)$@
	
linux_ip.o : $(NET_PATH)linux_ip.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)linux_ip.c -o $(EXEPATH)$@
	
m_server.o : $(NET_PATH)m_server.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)m_server.c -o $(EXEPATH)$@
	
mqtt_client.o : $(NET_PATH)mqtt_client.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)mqtt_client.c -o $(EXEPATH)$@
	
mqtt_share.o : $(NET_PATH)mqtt_share.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)mqtt_share.c -o $(EXEPATH)$@
	
socket_client.o : $(NET_PATH)socket_client.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)socket_client.c -o $(EXEPATH)$@
	
socket_sev.o : $(NET_PATH)socket_sev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)socket_sev.c -o $(EXEPATH)$@
	
udp_sev.o : $(NET_PATH)udp_sev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)udp_sev.c -o $(EXEPATH)$@

https_client.o : $(NET_PATH)https_client.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)https_client.c -o $(EXEPATH)$@

ip_check.o : $(NET_PATH)ip_check.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(NET_PATH)ip_check.c -o $(EXEPATH)$@
	
bluetooth.o : $(SERIAL_PATH)bluetooth.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(SERIAL_PATH)bluetooth.c -o $(EXEPATH)$@
	
lcd_state.o : $(SERIAL_PATH)lcd_state.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(SERIAL_PATH)lcd_state.c -o $(EXEPATH)$@
	
seria.o : $(SERIAL_PATH)seria.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(SERIAL_PATH)seria.c -o $(EXEPATH)$@
	
switch_dev.o : $(SERIAL_PATH)switch_dev.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(SERIAL_PATH)switch_dev.c -o $(EXEPATH)$@

usart.o : $(SERIAL_PATH)usart.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(SERIAL_PATH)usart.c -o $(EXEPATH)$@
	
delay_task.o : $(TASK_PATH)delay_task.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(TASK_PATH)delay_task.c -o $(EXEPATH)$@
	
scheduled_task.o : $(TASK_PATH)scheduled_task.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(TASK_PATH)scheduled_task.c -o $(EXEPATH)$@

main.o : ../application/main.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) ../application/main.c -o $(EXEPATH)$@

.PHONY : clean	
clean :
	-rm -f $(EXEPATH)*.o $(EXENAME) $(RUNNAME) *.a *.so *.d $(EXEPATH)$(EXENAME) $(EXEPATH)$(RUNNAME)

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d
