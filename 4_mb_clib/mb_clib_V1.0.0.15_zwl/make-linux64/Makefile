#CROSS=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-
CROSS=/usr/bin/
CC=gcc
CXX=g++
AR=ar
SSTRP=strip
CFLAGS= -Wall -g -fPIC -c 
INCLUDES=-I/usr/local/include-linux64-tassl -L/usr/local/lib-linux64-tassl -DSYSTEM_32#这里的【*-linux64-*】从arm拷贝的，它指的是链接的头文件和库文件的首选路径【里面都是文件，一般和架构无关】
HEADPATH=../


#包含的文件
software=_algorithm.o _base64.o _code_s.o _confl.o _Date.o _des.o _utf82gbk.o _zlib.o cJSON.o test_malloc.o _t_file_c.o _file_pro.o _folder.o _log.o _timer.o _dec_file.o\
 mxml-node.o mxml-file.o mxml-attr.o mxml-entity.o mxml-get.o mxml-index.o mxml-private.o mxml-set.o mxml-string.o mxml-search.o ioapi.o unzip.o zip.o


OBJSBSE=$(software)

#跟随之前的架构，下面创建相关的【*-linux64-*】的文件库，用于存放相应的文件
 
LIBNAME=libmb_clib
OBJSLIB=$(app) 

LIBPATH=../lib-linux64/
EXEPATH=exe/
LDLIB += -L/usr/local/lib-arm-tassl


 
.PHONY : all									 
all : $(LIBNAME) 

$(LIBNAME) : $(OBJSBSE)
	$(CROSS)$(CXX) $(CXXFLAGS_SO) $(LDLIB) $(OBJSBSE) -shared  -o $(LIBNAME).so  $(EXTRA_OBJS_SO)  -L/usr/local/lib
	$(CROSS)$(SSTRP) $(LIBNAME).so
	cp $(LIBNAME).so $(LIBPATH)
	cp $(LIBNAME).so /usr/local/lib-linux64-tassl/
	cp $(HEADPATH)software/*.h /usr/local/include-linux64-tassl/mb_clib/
	cp $(HEADPATH)software/mxml/*.h /usr/local/include-linux64-tassl/mb_clib/mxml/
	cp $(HEADPATH)software/minizip/*.h /usr/local/include-linux64-tassl/mb_clib/minizip/
	cp -rf /usr/local/include-linux64-tassl/mb_clib ../include


#$(LIBNAME) : $(OBJSBSE)
#	$(CROSS)$(CXX) $(CXXFLAGS_SO) $(LDLIB) $(OBJSBSE) -shared  -o $(LIBNAME).so  $(EXTRA_OBJS_SO)  -L/usr/local/lib
#	$(CROSS)$(SSTRP) $(LIBNAME).so
#	cp $(LIBNAME).so $(LIBPATH)
#	cp $(LIBNAME).so /usr/local/lib-arm-tassl/
#	cp $(HEADPATH)software/*.h /usr/local/include-arm-tassl/mb_clib/
#	cp $(HEADPATH)software/mxml/*.h /usr/local/include-arm-tassl/mb_clib/mxml/
#	cp $(HEADPATH)software/minizip/*.h /usr/local/include-arm-tassl/mb_clib/minizip/
#	cp -rf /usr/local/include-arm-tassl/mb_clib ../include

#software/mxml包含的文件
mxml-node.o : $(HEADPATH)software/mxml/mxml-node.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-node.c 

mxml-attr.o : $(HEADPATH)software/mxml/mxml-attr.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-attr.c 

mxml-entity.o : $(HEADPATH)software/mxml/mxml-entity.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-entity.c 

mxml-file.o : $(HEADPATH)software/mxml/mxml-file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-file.c 

mxml-get.o : $(HEADPATH)software/mxml/mxml-get.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-get.c 

mxml-index.o : $(HEADPATH)software/mxml/mxml-index.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-index.c 

mxml-private.o : $(HEADPATH)software/mxml/mxml-private.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-private.c 

mxml-search.o : $(HEADPATH)software/mxml/mxml-search.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-search.c 

mxml-set.o : $(HEADPATH)software/mxml/mxml-set.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-set.c 

mxml-string.o : $(HEADPATH)software/mxml/mxml-string.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/mxml/mxml-string.c 

#software/minizip包含的文件
ioapi.o : $(HEADPATH)software/minizip/ioapi.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/minizip/ioapi.c 

unzip.o : $(HEADPATH)software/minizip/unzip.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/minizip/unzip.c 

zip.o : $(HEADPATH)software/minizip/zip.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/minizip/zip.c 


#software 包含的文件

_algorithm.o : $(HEADPATH)software/_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_algorithm.c

_app_file.o : $(HEADPATH)software/_app_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_app_file.c

_base64.o : $(HEADPATH)software/_base64.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_base64.c

_code_s.o : $(HEADPATH)software/_code_s.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_code_s.c

_confl.o : $(HEADPATH)software/_confl.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_confl.c

_Date.o : $(HEADPATH)software/_Date.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_Date.c

_des.o : $(HEADPATH)software/_des.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_des.c

_md5.o : $(HEADPATH)software/_md5.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_md5.c

_utf82gbk.o : $(HEADPATH)software/_utf82gbk.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_utf82gbk.c

_zlib.o : $(HEADPATH)software/_zlib.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_zlib.c

cJSON.o : $(HEADPATH)software/cJSON.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/cJSON.c

test_malloc.o : $(HEADPATH)software/test_malloc.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/test_malloc.c

_file_pro.o : $(HEADPATH)software/_file_pro.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_file_pro.c

_folder.o : $(HEADPATH)software/_folder.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_folder.c

_t_file_c.o : $(HEADPATH)software/_t_file_c.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_t_file_c.c

_log.o : $(HEADPATH)software/_log.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_log.c

_timer.o : $(HEADPATH)software/_timer.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_timer.c

_dec_file.o : $(HEADPATH)software/_dec_file.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)software/_dec_file.c

.PHONY : clean	
clean :
	-rm -f *.o *.a *.so *.d $(LIBPATH)$(LIBNAME).so

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d

