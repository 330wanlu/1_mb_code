#ifndef OL_BANK_INCLUDE_H
#define OL_BANK_INCLUDE_H

//linux系统相关头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>	
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <sys/vfs.h>
#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <iconv.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <openssl/sha.h>
#include <ctype.h>
#include <linux/rtc.h>
#include <stdarg.h>
#include <dirent.h>
#include <regex.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <strings.h>
#include <assert.h>
#include <netinet/tcp.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <termios.h> 
#include <linux/spi/spidev.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <getopt.h>

//第三方相关头文件
#include "third_pard/inc/mb_typedef.h"
#include "third_pard/inc/_algorithm.h"
#include "third_pard/inc/_base64.h"
#include "third_pard/inc/_code_s.h"
#include "third_pard/inc/_confl.h"
#include "third_pard/inc/_Date.h"
#include "third_pard/inc/_des.h"
#include "third_pard/inc/_utf82gbk.h"
#include "third_pard/inc/_zlib.h"
#include "third_pard/inc/cJSON.h"
#include "third_pard/inc/test_malloc.h"
#include "third_pard/inc/_folder.h"
#include "third_pard/inc/_file_pro.h"
#include "third_pard/inc/_t_file_c.h"
//#include "third_pard/inc/_timer.h"
#include "third_pard/inc/_log.h"
#include "third_pard/inc/_dec_file.h"
#include "third_pard/inc/mosquitto_internal.h"
#include "third_pard/inc/mosquitto_new.h"
#include "third_pard/inc/mosquitto.h"
#include "third_pard/inc/mqtt_protocol.h"
#include "third_pard/inc/tax_info.h"
// // #include "third_pard/inc/common.h"
// #include "third_pard/inc/define.h"
// #include "third_pard/inc/function.h"


// //app端头文件
// #include "sys_relevant/common/inc/linux_s.h"
// #include "sys_relevant/common/inc/lock.h"
// #include "sys_relevant/common/inc/mb_tools.h"
// #include "sys_relevant/common/inc/timer.h"

// #include "sys_relevant/file/inc/file_ac.h"
// #include "sys_relevant/file/inc/file_oper.h"
// #include "sys_relevant/file/inc/file.h"
// #include "sys_relevant/file/inc/mem_oper.h"
// #include "sys_relevant/file/inc/usb_file.h"

// #include "sys_relevant/net/inc/ftp_n.h"
// #include "sys_relevant/net/inc/http_reg.h"
// #include "sys_relevant/net/inc/http.h"
// #include "sys_relevant/net/inc/linux_ip.h"
// #include "sys_relevant/net/inc/m_server.h"
// #include "sys_relevant/net/inc/mqtt_client.h"
// #include "sys_relevant/net/inc/mqtt_share.h"
// #include "sys_relevant/net/inc/socket_client.h"
// #include "sys_relevant/net/inc/socket_sev.h"
// #include "sys_relevant/net/inc/udp_sev.h"

// #include "sys_relevant/serial/inc/switch_dev.h"

// #include "sys_relevant/serial/inc/seria.h"
// #include "sys_relevant/serial/inc/usart.h"

// #include "sys_relevant/task/inc/delay_task.h"
// #include "sys_relevant/task/inc/scheduled_task.h"

// #include "application/app/inc/module.h"
// #include "application/app/inc/process_manage.h"

// #include "application/imitate_usb/inc/imitate_usb.h"

// #include "application/power_saving/inc/power_saving.h"

// #include "application/protocol/inc/machine_infor.h"
// #include "application/protocol/inc/deal_cmd_newshare.h"
// #include "application/protocol/inc/deploy.h"
// #include "application/protocol/inc/get_net_state.h"
// #include "application/protocol/inc/get_net_time.h"

// #include "application/protocol/inc/register.h"
// #include "application/protocol/inc/tax_interface.h"

// #include "application/test_mode/inc/test.h"

// #include "application/update/inc/update.h"


// #include "application/usb_port/inc/ty_usb.h"
// #include "application/usb_port/inc/usb_port.h"
// #include "application/usb_port/inc/tran_data.h"





// #include "sys_relevant/serial/inc/bluetooth.h"
// #include "sys_relevant/serial/inc/lcd_state.h"



#endif
