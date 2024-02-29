CROSS=/usr/local/mips/0.9.33.2/bin/mipsel-openwrt-linux-
CC=gcc
CXX=g++
AR=ar
SSTRP=strip
CFLAGS= -Wall -g -fPIC -c 
INCLUDES=-I/usr/local/include-mips-tassl -L/usr/local/lib-mips-tassl -DSYSTEM_32
HEADPATH=../


#包含的文件
software=_algorithm.o _base64.o _code_s.o _confl.o _Date.o _des.o _utf82gbk.o _zlib.o cJSON.o test_malloc.o _t_file_c.o _file_pro.o _folder.o _log.o _timer.o _dec_file.o\
 mxml-node.o mxml-file.o mxml-attr.o mxml-entity.o mxml-get.o mxml-index.o mxml-private.o mxml-set.o mxml-string.o mxml-search.o ioapi.o unzip.o zip.o 


OBJSBSE=$(software)


 
LIBNAME=libmb_clib
OBJSLIB=$(app) 

LIBPATH=../lib-mips/
EXEPATH=exe/
LDLIB += -L/usr/local/lib-mips-tassl


 
.PHONY : all									 
all : $(LIBNAME) 



$(LIBNAME) : $(OBJSBSE)
	$(CROSS)$(CXX) $(CXXFLAGS_SO) $(LDLIB) $(OBJSBSE) -shared  -o $(LIBNAME).so  $(EXTRA_OBJS_SO)  
	$(CROSS)$(SSTRP) $(LIBNAME).so
	cp $(LIBNAME).so $(LIBPATH)
	cp $(LIBNAME).so /usr/local/lib-mips-tassl/
	cp $(HEADPATH)software/*.h /usr/local/include-mips-tassl/mb_clib/
	cp $(HEADPATH)software/mxml/*.h /usr/local/include-mips-tassl/mb_clib/mxml/


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

