#定义使用的嵌入式编译器
CROSS=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-
CC=gcc
CXX=g++
AR=ar
SSTRP=
SSTRP=strip
CFLAGS= -Wall -g -fPIC -c 
#可执行文件
INCLUDES= -I/usr/local/include-newmqtt -L/usr/local/lib-mtk-newmqtt



#包含的文件
software=_code_s.o _app_file.o cJSON.o _base64.o
#linux_sys=_file_oper.o
app=_main.o _tools_boot.o _http_boot.o

EXENAME=mb_boot
OBJS=$(software) $(app) 
HEADPATH=../
#LIBPATH=../lib/
EXEPATH=../exe/
#LDLIB+= -L../lib
#INLIB= 

LDFLAGS += -lcrypto -lm -lpthread -L/usr/local/lib-newmqtt
 
.PHONY : all									 
all : $(EXENAME)

$(EXENAME) : $(OBJS)  
	$(CROSS)$(CC) -std=gnu99 -o $(EXENAME) $(OBJS) $(LDFLAGS)
#	$(CROSS)$(SSTRP) $(EXENAME)
	cp $(EXENAME) $(EXEPATH)					   

#software包含的文件
_code_s.o : $(HEADPATH)software/_code_s.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_code_s.c

_algorithm.o : $(HEADPATH)software/_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_algorithm.c

_app_file.o : $(HEADPATH)software/_app_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_app_file.c
	
	
cJSON.o : $(HEADPATH)software/cJSON.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/cJSON.c
_base64.o : $(HEADPATH)software/_base64.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_base64.c

#app包含的文件
_main.o : _main.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _main.c
	
_tools_boot.o : _tools_boot.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _tools_boot.c

_http_boot.o : _http_boot.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _http_boot.c
.PHONY : clean	
clean :
	-rm -f *.o $(EXENAME) *.a *.so *.d $(EXEPATH)$(EXENAME)

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d
