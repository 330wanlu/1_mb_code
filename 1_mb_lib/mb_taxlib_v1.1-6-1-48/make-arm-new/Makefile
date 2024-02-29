CROSS=/usr/local/arm/4.6.3/bin/arm-linux-gnueabi-
CC=gcc
CXX=g++
AR=ar
SSTRP=strip
CFLAGS= -Wall -g -fPIC -c 
INCLUDES=-I/usr/local/include-arm-libusb -L/usr/local/lib-arm-libusb -DARM_A33 -DDF_UK_NEED_AUTH
HEADPATH=.


#包含的文件
tax_aisino_module = aisino_algorithm.o aisino_common.o aisino_getinvoice.o aisino_getmonitor.o aisino_makeinvoice.o aisino_report_clear.o aisino_uploadinvoice.o aisino_netinvoice.o
tax_aisino_usb = aisino_usbio.o
tax_aisino = aisino_call.o $(tax_aisino_module) $(tax_aisino_usb)
tax_authority_module = ta_algorithm.o ta_confirm.o ta_vendor.o ta_vendor_common.o ta_xml_func.o ta_xml_iface.o 
tax_authority_net = httpsclt.o transfer.o
tax_authority = ta_xml.o ta_iface.o  $(tax_authority_module) $(tax_authority_net)
tax_common = character.o debug.o fpxx_json.o function.o importinv.o mbdiff.o usb_device.o 
tax_mengbai_usb = hid_io.o
tax_mengbai = mengbai_call.o $(tax_mengbai_usb)
tax_nisec_module = nisec_algorithm.o nisec_common.o nisec_getinvoice.o nisec_getmonitor.o nisec_makeinvoice.o nisec_netinvoice.o nisec_report_clear.o nisec_uploadinvoice.o
tax_nisec_usb = nisec_usbio.o
tax_nisec = nisec_call.o $(tax_nisec_module) $(tax_nisec_usb)
tax_swukey_module = cntax_algorithm.o cntax_common.o cntax_getinvoice.o cntax_getmonitor.o cntax_makeinvoice.o cntax_netinvoice.o cntax_report_clear.o cntax_uploadinvoice.o
tax_swukey = cntax_call.o $(tax_swukey_module)


#wang 03 07
tax_etax =  etax_bridge.o etax_call.o etax_jsvmp.o shanghai_call.o jiangsu_call.o shanxi_call.o henan_call.o hubei_call.o sichuan_call.o qingdao_call.o chongqing_call.o beijing_call.o anhui_call.o guangxi_call.o hunan_call.o \
			xinjiang_call.o guangdong_call.o jiangxi_call.o ningbo_call.o hainan_call.o fujian_call.o guizhou_call.o gansu_call.o shandong_call.o hebei_call.o zhejiang_call.o shaanxi_call.o
#etax_auth_server.o etax_confirm.o 

tax= interface_main.o $(tax_aisino) $(tax_authority) $(tax_common) $(tax_mengbai) $(tax_nisec) $(tax_swukey)  $(tax_etax)




OBJSBSE=$(tax) 


 
LIBNAME=libmb_taxlib
#OBJSLIB=$(app) 

HEADPATH=../
LIBPATH=../lib-arm-new/
OPTINCPATH=/usr/local/include-arm-tassl/
OPTLIBPATH=/usr/local/lib-arm-tassl/

 
.PHONY : all									 
all : $(LIBNAME) 



$(LIBNAME) : $(OBJSBSE)
	$(CROSS)$(CXX) $(CXXFLAGS_SO)  $(OBJSBSE) -shared  -o $(LIBNAME).so  $(EXTRA_OBJS_SO) -lmb_clib -lssl -lmpfr -lgmp -lcrypto -levent -levent_openssl -levent_pthreads $(INCLUDES)
	$(CROSS)$(SSTRP) $(LIBNAME).so
	cp $(LIBNAME).so $(LIBPATH)
	cp $(LIBNAME).so $(OPTLIBPATH)
	cp $(HEADPATH)tax/*.h $(OPTINCPATH)mb_taxlib/

	cp $(HEADPATH)tax/aisino/*.h $(OPTINCPATH)mb_taxlib/aisino/
	cp $(HEADPATH)tax/aisino/module/*.h $(OPTINCPATH)mb_taxlib/aisino/module/
	cp $(HEADPATH)tax/aisino/usb/*.h $(OPTINCPATH)mb_taxlib/aisino/usb/

	cp $(HEADPATH)tax/authority/*.h $(OPTINCPATH)mb_taxlib/authority/
	cp $(HEADPATH)tax/authority/module/*.h $(OPTINCPATH)mb_taxlib/authority/module
	cp $(HEADPATH)tax/authority/net/*.h $(OPTINCPATH)mb_taxlib/authority/net

	cp $(HEADPATH)tax/common/*.h $(OPTINCPATH)mb_taxlib/common/
	
	cp $(HEADPATH)tax/mengbai/*.h $(OPTINCPATH)mb_taxlib/mengbai/
	cp $(HEADPATH)tax/mengbai/usb/*.h $(OPTINCPATH)mb_taxlib/mengbai/usb/
	
	cp $(HEADPATH)tax/nisec/*.h $(OPTINCPATH)mb_taxlib/nisec/
	cp $(HEADPATH)tax/nisec/module/*.h $(OPTINCPATH)mb_taxlib/nisec/module/
	cp $(HEADPATH)tax/nisec/usb/*.h $(OPTINCPATH)mb_taxlib/nisec/usb/

	cp $(HEADPATH)tax/cntax/*.h $(OPTINCPATH)mb_taxlib/cntax/
	cp $(HEADPATH)tax/cntax/module/*.h $(OPTINCPATH)mb_taxlib/cntax/module/

#tax包含的文件

#etax包含的文件
#wang 
#etax_auth_server.o etax_bridge.o etax_call.o etax_confirm.o
#etax_auth_server.o : $(HEADPATH)tax/etax/etax_auth_server.c
#	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/etax_auth_server.c			

etax_bridge.o : $(HEADPATH)tax/etax/etax_bridge.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/etax_bridge.c		

etax_call.o : $(HEADPATH)tax/etax/etax_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/etax_call.c		

etax_jsvmp.o : $(HEADPATH)tax/etax/etax_jsvmp.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/etax_jsvmp.c	

shanghai_call.o : $(HEADPATH)tax/etax/shanghai_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/shanghai_call.c	   

jiangsu_call.o : $(HEADPATH)tax/etax/jiangsu_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/jiangsu_call.c	  

shanxi_call.o : $(HEADPATH)tax/etax/shanxi_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/shanxi_call.c	

henan_call.o : $(HEADPATH)tax/etax/henan_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/henan_call.c

hubei_call.o : $(HEADPATH)tax/etax/hubei_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/hubei_call.c

sichuan_call.o : $(HEADPATH)tax/etax/sichuan_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/sichuan_call.c

qingdao_call.o : $(HEADPATH)tax/etax/qingdao_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/qingdao_call.c		
	  
chongqing_call.o : $(HEADPATH)tax/etax/chongqing_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/chongqing_call.c

beijing_call.o : $(HEADPATH)tax/etax/beijing_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/beijing_call.c

anhui_call.o : $(HEADPATH)tax/etax/anhui_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/anhui_call.c

guangxi_call.o : $(HEADPATH)tax/etax/guangxi_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/guangxi_call.c

hunan_call.o : $(HEADPATH)tax/etax/hunan_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/hunan_call.c

xinjiang_call.o : $(HEADPATH)tax/etax/xinjiang_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/xinjiang_call.c

guangdong_call.o : $(HEADPATH)tax/etax/guangdong_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/guangdong_call.c

jiangxi_call.o : $(HEADPATH)tax/etax/jiangxi_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/jiangxi_call.c

ningbo_call.o : $(HEADPATH)tax/etax/ningbo_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/ningbo_call.c

hainan_call.o : $(HEADPATH)tax/etax/hainan_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/hainan_call.c

fujian_call.o : $(HEADPATH)tax/etax/fujian_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/fujian_call.c

guizhou_call.o : $(HEADPATH)tax/etax/guizhou_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/guizhou_call.c

gansu_call.o : $(HEADPATH)tax/etax/gansu_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/gansu_call.c

shandong_call.o : $(HEADPATH)tax/etax/shandong_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/shandong_call.c

hebei_call.o : $(HEADPATH)tax/etax/hebei_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/hebei_call.c

zhejiang_call.o : $(HEADPATH)tax/etax/zhejiang_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/zhejiang_call.c

shaanxi_call.o : $(HEADPATH)tax/etax/shaanxi_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/etax/shaanxi_call.c

#tax/aisino/auxiliary包含的文件

#tax/aisino/module包含的文件
aisino_algorithm.o : $(HEADPATH)tax/aisino/module/aisino_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_algorithm.c
aisino_common.o : $(HEADPATH)tax/aisino/module/aisino_common.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_common.c
aisino_getinvoice.o : $(HEADPATH)tax/aisino/module/aisino_getinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_getinvoice.c
aisino_getmonitor.o : $(HEADPATH)tax/aisino/module/aisino_getmonitor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_getmonitor.c
aisino_makeinvoice.o : $(HEADPATH)tax/aisino/module/aisino_makeinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_makeinvoice.c
aisino_report_clear.o : $(HEADPATH)tax/aisino/module/aisino_report_clear.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_report_clear.c
aisino_uploadinvoice.o : $(HEADPATH)tax/aisino/module/aisino_uploadinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_uploadinvoice.c
aisino_netinvoice.o : $(HEADPATH)tax/aisino/module/aisino_netinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/module/aisino_netinvoice.c

#tax/aisino/usb包含的文件
aisino_usbio.o : $(HEADPATH)tax/aisino/usb/aisino_usbio.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/usb/aisino_usbio.c

#tax/aisino包含的文件
aisino_call.o : $(HEADPATH)tax/aisino/aisino_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/aisino/aisino_call.c

#tax/authority/module包含的文件
ta_algorithm.o : $(HEADPATH)tax/authority/module/ta_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_algorithm.c
ta_confirm.o : $(HEADPATH)tax/authority/module/ta_confirm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_confirm.c
ta_vendor.o : $(HEADPATH)tax/authority/module/ta_vendor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_vendor.c
ta_vendor_common.o : $(HEADPATH)tax/authority/module/ta_vendor_common.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_vendor_common.c
ta_xml_func.o : $(HEADPATH)tax/authority/module/ta_xml_func.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_xml_func.c
ta_xml_iface.o : $(HEADPATH)tax/authority/module/ta_xml_iface.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/module/ta_xml_iface.c

#tax/authority/net包含的文件
httpsclt.o : $(HEADPATH)tax/authority/net/httpsclt.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/net/httpsclt.c
transfer.o : $(HEADPATH)tax/authority/net/transfer.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/net/transfer.c

#tax/authority包含的文件
ta_xml.o : $(HEADPATH)tax/authority/ta_xml.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/ta_xml.c
ta_iface.o : $(HEADPATH)tax/authority/ta_iface.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/authority/ta_iface.c

#tax/common包含的文件
character.o: $(HEADPATH)tax/common/character.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/character.c
debug.o: $(HEADPATH)tax/common/debug.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/debug.c
fpxx_json.o: $(HEADPATH)tax/common/fpxx_json.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/fpxx_json.c
function.o: $(HEADPATH)tax/common/function.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/function.c
importinv.o : $(HEADPATH)tax/common/importinv.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/importinv.c
mbdiff.o : $(HEADPATH)tax/common/mbdiff.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/mbdiff.c
usb_device.o: $(HEADPATH)tax/common/usb_device.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/common/usb_device.c

#tax/mengbai/usb包含的文件
hid_io.o : $(HEADPATH)tax/mengbai/usb/hid_io.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/mengbai/usb/hid_io.c	

#tax/mengbai包含的文件
mengbai_call.o : $(HEADPATH)tax/mengbai/mengbai_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/mengbai/mengbai_call.c

#tax/nisec/module包含的文件
nisec_algorithm.o : $(HEADPATH)tax/nisec/module/nisec_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_algorithm.c
nisec_common.o : $(HEADPATH)tax/nisec/module/nisec_common.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_common.c
nisec_getinvoice.o : $(HEADPATH)tax/nisec/module/nisec_getinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_getinvoice.c
nisec_getmonitor.o : $(HEADPATH)tax/nisec/module/nisec_getmonitor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_getmonitor.c
nisec_makeinvoice.o : $(HEADPATH)tax/nisec/module/nisec_makeinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_makeinvoice.c
nisec_netinvoice.o : $(HEADPATH)tax/nisec/module/nisec_netinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_netinvoice.c
nisec_report_clear.o : $(HEADPATH)tax/nisec/module/nisec_report_clear.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_report_clear.c
nisec_uploadinvoice.o : $(HEADPATH)tax/nisec/module/nisec_uploadinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/module/nisec_uploadinvoice.c

#tax/nisec/usb包含的文件
nisec_usbio.o: $(HEADPATH)tax/nisec/usb/nisec_usbio.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/usb/nisec_usbio.c

#tax/nisec包含的文件
nisec_call.o : $(HEADPATH)tax/nisec/nisec_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/nisec/nisec_call.c
	
#tax/cntax/module包含的文件
cntax_algorithm.o : $(HEADPATH)tax/cntax/module/cntax_algorithm.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_algorithm.c

cntax_common.o : $(HEADPATH)tax/cntax/module/cntax_common.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_common.c

cntax_getinvoice.o : $(HEADPATH)tax/cntax/module/cntax_getinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_getinvoice.c

cntax_getmonitor.o : $(HEADPATH)tax/cntax/module/cntax_getmonitor.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_getmonitor.c 

cntax_makeinvoice.o : $(HEADPATH)tax/cntax/module/cntax_makeinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_makeinvoice.c 

cntax_netinvoice.o : $(HEADPATH)tax/cntax/module/cntax_netinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_netinvoice.c 

cntax_report_clear.o : $(HEADPATH)tax/cntax/module/cntax_report_clear.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_report_clear.c 

cntax_uploadinvoice.o : $(HEADPATH)tax/cntax/module/cntax_uploadinvoice.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/module/cntax_uploadinvoice.c 

#tax/cntax包含的文件
cntax_call.o : $(HEADPATH)tax/cntax/cntax_call.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/cntax/cntax_call.c

#tax包含的文件
interface_main.o : $(HEADPATH)tax/interface_main.c
	$(CROSS)$(CC) $(INCLUDES) $(CFLAGS) $(HEADPATH)tax/interface_main.c				   












.PHONY : clean	
clean :
	-rm -f *.o *.a *.so *.d $(LIBPATH)$(LIBNAME).so

.PHONY : cleanobj
cleanobj :
	-rm -f *.o *.d

