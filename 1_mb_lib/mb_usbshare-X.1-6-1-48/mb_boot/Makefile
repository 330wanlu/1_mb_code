#定义使用的嵌入式编译器
CROSS=
#定义使用的C编译器
CC=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-gcc
AR=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-ar
#CC=gcc 
#AR=ar
#CC=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-gcc 
#AR=/opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-fsl-linux-gnueabi-ar
#CC=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-gcc
#AR=/usr/local/arm/arm-2009q3/bin/arm-none-linux-gnueabi-ar
CFLAGS= -Wall -g -fPIC   -c 
#可执行文件
INCLUDES= 



#包含的文件
software=_code_s.o _algorithm.o _app_file.o
linux_sys=_file_oper.o
app=_main.o 

EXENAME=mb_1902
OBJS=$(software) $(app) $(linux_sys)
HEADPATH=../
LIBPATH=../lib/
EXEPATH=../exe/
LDLIB+= -L../lib
INLIB= 


 
.PHONY : all									 
#all : clean $(LIBNAME).so cleanobj $(LIBNAME).a $(EXENAME)
all : $(EXENAME)

$(EXENAME) : $(OBJS)  
	$(CROSS)$(CC) -std=gnu99 -o $(EXENAME) $(OBJS)
	cp $(EXENAME) $(EXEPATH)

#$(LIBNAME).a : $(OBJSLIB)
#	$(CROSS)$(AR) -rcv $(LIBNAME).a $(OBJSLIB)


#	$(CROSS)$(CC) -L. -I. -static -lc \
#	-o $(EXENAME) $(OBJS) $(LIBNAME).a	$(LIBICONV_DIR)/lib/libiconv.a						   

#software包含的文件
_code_s.o : $(HEADPATH)software/_code_s.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_code_s.c

_algorithm.o : $(HEADPATH)software/_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_algorithm.c

_app_file.o : $(HEADPATH)software/_app_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_app_file.c
	
_file_oper.o : $(HEADPATH)linux_sys/_file_oper.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)linux_sys/_file_oper.c
	
	


#app包含的文件
_main.o : _main.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) _main.c
	



.PHONY : clean	
clean :
	-rm -f *.o $(EXENAME) *.a *.so *.d $(EXEPATH)$(EXENAME)

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d
