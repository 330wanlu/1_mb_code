#include "../inc/ty_usb.h"

static int ty_usb_open(struct ty_file	*file);
static int ty_usb_ctl(void *data,int fd,int cm,va_list args);
static int ty_usb_close(void *data,int fd);
static int get_busid_exist(char *busid);
static int _set_busid(struct _ty_usb_fd *id,va_list args);
static int _get_busid(struct _ty_usb_fd *id,va_list args);
static int _set_port_n(struct _ty_usb_fd *id,va_list args);
static int _get_port_n(struct _ty_usb_fd *id,va_list args);
static int _set_enum_timer(struct _ty_usb_fd *id,va_list args);
static int _get_enum_timer(struct _ty_usb_fd *id,va_list args);
static int _open_port(struct _ty_usb_fd *id,va_list args);
static int _open_port_power(struct _ty_usb_fd *id,va_list args);
static int _close_port_power(struct _ty_usb_fd *id,va_list args);
static int _get_port_exist(struct _ty_usb_fd *id,va_list args);
static int _close_port(struct _ty_usb_fd *id,va_list args);
static int _close_port_power_just(struct _ty_usb_fd *id,va_list args);
static int get_busid(struct _ty_usb_fd *id,va_list args);
static int get_port_number(struct _ty_usb_fd *id,va_list args);
static int juge_port(struct _ty_usb_fd *id,va_list args);
static int get_port_maid(struct _ty_usb_fd *id,va_list args);
static int get_port_pcb(struct _ty_usb_fd *id,va_list args);
static int _get_busid2port(struct _ty_usb_fd *id,va_list args);
static int _get_register_whether(struct _ty_usb_fd *id,va_list args);
static int _get_tryout_whether(struct _ty_usb_fd *id,va_list args);
static int _get_machine_id(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_off_gren_off(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_on_gren_off(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_off_gren_on(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_on_gren_twi(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_on_gren_on(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_twi_gren_on(struct _ty_usb_fd *id, va_list args);
static int _set_led_read_off_green_twi(struct _ty_usb_fd *id, va_list args);
static int _set_led_read_twi_green_off(struct _ty_usb_fd *id, va_list args);
static int _set_led_red_twi_gren_twi_1s(struct _ty_usb_fd *id, va_list args);
static int get_port(struct _ty_usb  *stream,int port);
static int get_port_busid(struct _ty_usb  *stream,int port,char *busid);
static int get_usb_file(char *busid,int enum_timer,int mach_type);
static int get_busid_over_timer(const char *busid,int enum_timer);
static int get_port(struct _ty_usb  *stream,int port);
static int wait_usb_file_no(char *busid,int enum_timer);
static int _ctl_port_no_dev_led(struct _ty_usb_fd *id,va_list args);
static int _ctl_port_dev_local_led(struct _ty_usb_fd *id,va_list args);
static int _ctl_port_dev_share_led(struct _ty_usb_fd *id,va_list args);
static int _get_public_praviate_key(struct _ty_usb_fd *id, va_list args);
static int _get_all_port_status(struct _ty_usb_fd *id, va_list args);
static int _set_fan(struct _ty_usb_fd *id, va_list args);
static int _get_all_ad_status(struct _ty_usb_fd *id, va_list args);
static int _set_led_double_light_yellow(struct _ty_usb_fd *id, va_list args);
static int _ctl_dev_restore(struct _ty_usb_fd *id, va_list args);
static int _ctl_bluetooth_mode(struct _ty_usb_fd *id, va_list args);
static int _ctl_bluetooth_power(struct _ty_usb_fd *id, va_list args);
static int _ctl_dev_conn(struct _ty_usb_fd *id, va_list args);

static const struct _file_fuc	ty_usb_fuc=
{
	.open=ty_usb_open,
	.read=NULL,
	.write=NULL,
	.ctl=ty_usb_ctl,
	.close=ty_usb_close,
	.del=NULL
};

static const struct _ty_usb_ctl_fu ctl_fun[]=
{
	{   DF_TY_USB_CM_GET_BUSID,             _get_busid          },
	{   DF_TY_USB_CM_SET_BUSID,             _set_busid          },
	{   DF_TY_USB_CM_SET_PORT_N,            _set_port_n         },
	{   DF_TY_USB_CM_GET_PORT_N,            _get_port_n         },
	{   DF_TY_USB_CM_SET_ENUM_TIMER,        _set_enum_timer     },
	{   DF_TY_USB_CM_GET_ENUM_TIMER,        _get_enum_timer     },
	{   DF_TY_USB_CM_OPEN_PORT,             _open_port          },
	{   DF_TY_USB_CM_PORT_BUSID,            get_busid           },
	{   DF_TY_USB_CM_PORT_NUMBER,           get_port_number     },
	{   DF_TY_USB_CM_CLOSE_PORT,            _close_port         },
	{   DF_TY_USB_CM_JUGE_PORT,             juge_port           },
	{   DF_TY_USB_CM_GET_PORT_MA_ID,        get_port_maid       },
	{   DF_TY_USB_CM_GET_PORT_PCB_ID,       get_port_pcb        },
	{   DF_TY_USB_CM_GET_BUSID_PORT,        _get_busid2port     },
	{	DF_TY_USB_CM_GET_REGISTER_WHETHER,	_get_register_whether},
	{	DF_TY_USB_CM_OPEN_PORT_POWER,		_open_port_power	},
	{	DF_TY_USB_CM_CLOSE_PORT_POWER,		_close_port_power	},
	{	DF_TY_USB_CM_GET_PORT_FILE_EXIST,	_get_port_exist		},
	{	DF_TY_USB_CM_GET_TRYOUT_WHETHER,	_get_tryout_whether	},
	{	DF_TY_USB_CM_PORT_SEV_LED_NO,		_ctl_port_no_dev_led},
	{	DF_TY_USB_CM_PORT_SEV_LED_LOCAL,	_ctl_port_dev_local_led},
	{	DF_TY_USB_CM_PORT_SEV_LED_SHARE,	_ctl_port_dev_share_led},
	{	DF_TY_USB_CM_PORT_CLOSE_PORT_POWER,	_close_port_power_just},
	{	DF_TY_USB_CM_GET_MACHINE_ID,		_get_machine_id		},
	{	DF_TY_USB_CM_PORT_RED_OFF_GREN_OFF,	_set_led_red_off_gren_off},
	{	DF_TY_USB_CM_PORT_RED_ON_GREN_OFF,	_set_led_red_on_gren_off},
	{	DF_TY_USB_CM_PORT_RED_OFF_GREN_ON,	_set_led_red_off_gren_on},
	{	DF_TY_USB_CM_PORT_RED_ON_GREN_TWI,	_set_led_red_on_gren_twi},
	{	DF_TY_USB_CM_PORT_RED_ON_GREN_ON,	_set_led_red_on_gren_on},
	{	DF_TY_USB_CM_PORT_RED_TWI_GREN_ON,	_set_led_red_twi_gren_on},
	{	DF_TU_USB_CM_PORT_GET_KEY_VALUE,	_get_public_praviate_key },
	{	DF_TY_USB_CM_PORT_RED_TWI_GREN_TWI_1S,_set_led_red_twi_gren_twi_1s},
	{ 	DF_TY_USB_CM_PORT_RED_OFF_GREN_TWI, _set_led_read_off_green_twi },
	{	DF_TY_USB_CM_PORT_RED_TWI_GREN_OFF,_set_led_read_twi_green_off},
	{ 	DF_TY_USB_CM_PORT_GET_ALL_PORT_STATUS, _get_all_port_status },
	{ 	DF_TY_USB_CM_PORT_SET_FAN, _set_fan },
	{	DF_TY_USB_CM_GET_AD_STATUS	,_get_all_ad_status	},
	{	DF_TY_USB_CM_PORT_DOUBLE_LIGHT_YELLOW,	_set_led_double_light_yellow},
	{	DF_TY_USB_CM_RESTORE,	_ctl_dev_restore		},
	{	DF_TY_USB_CM_CTL_BLUETOOTH_MODE,	_ctl_bluetooth_mode},
	{	DF_TY_USB_CM_BLUETOOTH_POWER,		_ctl_bluetooth_power},
	{	DF_TY_USB_CM_CONN		,			_ctl_dev_conn},
};

const char *usb_port_busid[] =
{
	"1.4",
	"1.3",
	"1.2",
	"1.1",
	"1.5",
	"1.6",
	"1.7",
	"2.4",
	"2.3",
	"2.2",
	"2.1",
	"2.5",
	"2.6",
	"2.7",
	"3.4",
	"3.3",
	"3.2",
	"3.1",
	"3.5",
	"3.6",
	"3.7",
	"4.4",
	"4.3",
	"4.2",
	"4.1",
	"4.5",
	"4.6",
	"4.7",
	"5.4",
	"5.3",
	"5.2",
	"5.1",
	"5.5",
	"5.6",
	"5.7",
	"6.4",
	"6.3",
	"6.2",
	"6.1",
	"6.5",
	"6.6",
	"6.7",
	"7.4",
	"7.3",
	"7.2",
	"7.1",
	"7.5",
	"7.6",
	"7.7"
};

const char *usb_port_busid_a33[]=
{
	"1.7",
	"1.6",
	"1.5",
	"1.1",
	"1.2",
	"1.3",
	"1.4",
	"2.7",
	"2.6",
	"2.5",
	"2.1",
	"2.2",
	"2.3",
	"2.4",
	"3.7",
	"3.6",
	"3.5",
	"3.1",
	"3.2",
	"3.3",
	"3.4",
	"4.7",
	"4.6",
	"4.5",
	"4.1",
	"4.2",
	"4.3",
	"4.4",
	"5.7",
	"5.6",
	"5.5",
	"5.1",
	"5.2",
	"5.3",
	"5.4",
	"6.7",
	"6.6",
	"6.5",
	"6.1",
	"6.2",
	"6.3",
	"6.4",
	"7.7",
	"7.6",
	"7.5",
	"7.1",
	"7.2",
	"7.3",
	"7.4"
};

static const char *busid_68[]=
{
	"1-1.1.1.4",
	"1-1.1.1.3",
	"1-1.1.1.2",
	"1-1.1.1.1",
	"1-1.1.1.5",
	"1-1.1.1.6",
	"1-1.1.1.7",
	"1-1.1.2.4",
	"1-1.1.2.3",
	"1-1.1.2.2",
	"1-1.1.2.1",
	"1-1.1.2.5",
	"1-1.1.2.6",
	"1-1.1.2.7",
	"1-1.1.3.4",
	"1-1.1.3.3",
	"1-1.1.3.2",
	"1-1.1.3.1",
	"1-1.1.3.5",
	"1-1.1.3.6",
	"1-1.1.3.7",
	"1-1.1.4.4",
	"1-1.1.4.3",
	"1-1.1.4.2",
	"1-1.1.4.1",
	"1-1.1.4.5",
	"1-1.1.4.6",
	"1-1.1.4.7",
	"1-1.1.5.4",
	"1-1.1.5.3",
	"1-1.1.5.2",
	"1-1.1.5.1",
	"1-1.1.5.5",
	"1-1.1.5.6",
	"1-1.2.1.4",
	"1-1.2.1.3",
	"1-1.2.1.2",
	"1-1.2.1.1",
	"1-1.2.1.5",
	"1-1.2.1.6",
	"1-1.2.1.7",
	"1-1.2.2.4",
	"1-1.2.2.3",
	"1-1.2.2.2",
	"1-1.2.2.1",
	"1-1.2.2.5",
	"1-1.2.2.6",
	"1-1.2.2.7",
	"1-1.2.3.4",
	"1-1.2.3.3",
	"1-1.2.3.2",
	"1-1.2.3.1",
	"1-1.2.3.5",
	"1-1.2.3.6",
	"1-1.2.3.7",
	"1-1.2.4.4",
	"1-1.2.4.3",
	"1-1.2.4.2",
	"1-1.2.4.1",
	"1-1.2.4.5",
	"1-1.2.4.6",
	"1-1.2.4.7",
	"1-1.2.5.4",
	"1-1.2.5.3",
	"1-1.2.5.2",
	"1-1.2.5.1",
	"1-1.2.5.5",
	"1-1.2.5.6"
};

static const char* busid_100[] =
{
	"1-1.1.1.7",//1
	"1-1.1.1.6",//2
	"1-1.1.1.5",//3
	"1-1.1.1.1",//4
	"1-1.1.1.2",//5
	"1-1.1.1.3",//6
	"1-1.1.1.4",//7
	"1-1.1.2.7",//8
	"1-1.1.2.6",//9
	"1-1.1.2.5",//10
	"1-1.1.2.1",//11
	"1-1.1.2.2",//12
	"1-1.1.2.3",//13
	"1-1.1.2.4",//14
	"1-1.1.3.7",//15
	"1-1.1.3.6",//16
	"1-1.1.3.5",//17
	"1-1.1.3.1",//18
	"1-1.1.3.2",//19
	"1-1.1.3.3",//20
	"1-1.2.1.7",//1
	"1-1.2.1.6",//2
	"1-1.2.1.5",//3
	"1-1.2.1.1",//4
	"1-1.2.1.2",//5
	"1-1.2.1.3",//6
	"1-1.2.1.4",//7
	"1-1.2.2.7",//8
	"1-1.2.2.6",//9
	"1-1.2.2.5",//10
	"1-1.2.2.1",//11
	"1-1.2.2.2",//12
	"1-1.2.2.3",//13
	"1-1.2.2.4",//14
	"1-1.2.3.7",//15
	"1-1.2.3.6",//16
	"1-1.2.3.5",//17
	"1-1.2.3.1",//18
	"1-1.2.3.2",//19
	"1-1.2.3.3",//20
	"1-1.3.1.7",//1
	"1-1.3.1.6",//2
	"1-1.3.1.5",//3
	"1-1.3.1.1",//4
	"1-1.3.1.2",//5
	"1-1.3.1.3",//6
	"1-1.3.1.4",//7
	"1-1.3.2.7",//8
	"1-1.3.2.6",//9
	"1-1.3.2.5",//10
	"1-1.3.2.1",//11
	"1-1.3.2.2",//12
	"1-1.3.2.3",//13
	"1-1.3.2.4",//14
	"1-1.3.3.7",//15
	"1-1.3.3.6",//16
	"1-1.3.3.5",//17
	"1-1.3.3.1",//18
	"1-1.3.3.2",//19
	"1-1.3.3.3",//20
	"1-1.4.1.7",//1
	"1-1.4.1.6",//2
	"1-1.4.1.5",//3
	"1-1.4.1.1",//4
	"1-1.4.1.2",//5
	"1-1.4.1.3",//6
	"1-1.4.1.4",//7
	"1-1.4.2.7",//8
	"1-1.4.2.6",//9
	"1-1.4.2.5",//10
	"1-1.4.2.1",//11
	"1-1.4.2.2",//12
	"1-1.4.2.3",//13
	"1-1.4.2.4",//14
	"1-1.4.3.7",//15
	"1-1.4.3.6",//16
	"1-1.4.3.5",//17
	"1-1.4.3.1",//18
	"1-1.4.3.2",//19
	"1-1.4.3.3",//20
	"1-1.5.1.7",//1
	"1-1.5.1.6",//2
	"1-1.5.1.5",//3
	"1-1.5.1.1",//4
	"1-1.5.1.2",//5
	"1-1.5.1.3",//6
	"1-1.5.1.4",//7
	"1-1.5.2.7",//8
	"1-1.5.2.6",//9
	"1-1.5.2.5",//10
	"1-1.5.2.1",//11
	"1-1.5.2.2",//12
	"1-1.5.2.3",//13
	"1-1.5.2.4",//14
	"1-1.5.3.7",//15
	"1-1.5.3.6",//16
	"1-1.5.3.5",//17
	"1-1.5.3.1",//18
	"1-1.5.3.2",//19
	"1-1.5.3.3" //20
};

static const char* busid_120[] =
{
	"1-1.1.1.1.7",			"1-1.1.1.1.6",			"1-1.1.1.1.5",			"1-1.1.1.1.1",			"1-1.1.1.1.2",
	"1-1.1.1.1.3",			"1-1.1.1.1.4",			"1-1.1.1.2.7",			"1-1.1.1.2.6",			"1-1.1.1.2.5",
	"1-1.1.1.2.1",			"1-1.1.1.2.2",			"1-1.1.1.2.3",			"1-1.1.1.2.4",			"1-1.1.1.3.7",
	"1-1.1.1.3.6",			"1-1.1.1.3.5",			"1-1.1.1.3.1",			"1-1.1.1.3.2",			"1-1.1.1.3.3",
	"1-1.1.1.4.7",			"1-1.1.1.4.6",			"1-1.1.1.4.5",			"1-1.1.1.4.1",			"1-1.1.2.1.7",
	"1-1.1.2.1.6",			"1-1.1.2.1.5",			"1-1.1.2.1.1",			"1-1.1.2.1.2",			"1-1.1.2.1.3",
	"1-1.1.2.1.4",			"1-1.1.2.2.7",			"1-1.1.2.2.6",			"1-1.1.2.2.5",			"1-1.1.2.2.1",
	"1-1.1.2.2.2",			"1-1.1.2.2.3",			"1-1.1.2.2.4",			"1-1.1.2.3.7",			"1-1.1.2.3.6",
	"1-1.1.2.3.5",			"1-1.1.2.3.1",			"1-1.1.2.3.2",			"1-1.1.2.3.3",			"1-1.1.2.4.7",
	"1-1.1.2.4.6",			"1-1.1.2.4.5",			"1-1.1.2.4.1",			"1-1.1.3.1.7",			"1-1.1.3.1.6",
	"1-1.1.3.1.5",			"1-1.1.3.1.1",			"1-1.1.3.1.2",			"1-1.1.3.1.3",			"1-1.1.3.1.4",
	"1-1.1.3.2.7",			"1-1.1.3.2.6",			"1-1.1.3.2.5",			"1-1.1.3.2.1",			"1-1.1.3.2.2",
	"1-1.1.3.2.3",			"1-1.1.3.2.4",			"1-1.1.3.3.7",			"1-1.1.3.3.6",			"1-1.1.3.3.5",
	"1-1.1.3.3.1",			"1-1.1.3.3.2",			"1-1.1.3.3.3",			"1-1.1.3.4.7",			"1-1.1.3.4.6",
	"1-1.1.3.4.5",			"1-1.1.3.4.1",			"1-1.1.4.1.7",			"1-1.1.4.1.6",			"1-1.1.4.1.5",
	"1-1.1.4.1.1",			"1-1.1.4.1.2",			"1-1.1.4.1.3",			"1-1.1.4.1.4",			"1-1.1.4.2.7",
	"1-1.1.4.2.6",			"1-1.1.4.2.5",			"1-1.1.4.2.1",			"1-1.1.4.2.2",			"1-1.1.4.2.3",
	"1-1.1.4.2.4",			"1-1.1.4.3.7",			"1-1.1.4.3.6",			"1-1.1.4.3.5",			"1-1.1.4.3.1",
	"1-1.1.4.3.2",			"1-1.1.4.3.3",			"1-1.1.4.4.7",			"1-1.1.4.4.6",			"1-1.1.4.4.5",
	"1-1.1.4.4.1",			"1-1.1.5.1.7",			"1-1.1.5.1.6",			"1-1.1.5.1.5",			"1-1.1.5.1.1",
	"1-1.1.5.1.2",			"1-1.1.5.1.3",			"1-1.1.5.1.4",			"1-1.1.5.2.7",			"1-1.1.5.2.6",
	"1-1.1.5.2.5",			"1-1.1.5.2.1",			"1-1.1.5.2.2",			"1-1.1.5.2.3",			"1-1.1.5.2.4",
	"1-1.1.5.3.7",			"1-1.1.5.3.6",			"1-1.1.5.3.5",			"1-1.1.5.3.1",			"1-1.1.5.3.2",
	"1-1.1.5.3.3",			"1-1.1.5.4.7",			"1-1.1.5.4.6",			"1-1.1.5.4.5",			"1-1.1.5.4.1"
};

static const char* busid_240[] =
{
	"1-1.1.1.1.7",			"1-1.1.1.1.6",			"1-1.1.1.1.5",			"1-1.1.1.1.1",			"1-1.1.1.1.2",
	"1-1.1.1.1.3",			"1-1.1.1.1.4",			"1-1.1.1.2.7",			"1-1.1.1.2.6",			"1-1.1.1.2.5",
	"1-1.1.1.2.1",			"1-1.1.1.2.2",			"1-1.1.1.2.3",			"1-1.1.1.2.4",			"1-1.1.1.3.7",
	"1-1.1.1.3.6",			"1-1.1.1.3.5",			"1-1.1.1.3.1",			"1-1.1.1.3.2",			"1-1.1.1.3.3",
	"1-1.1.1.4.7",			"1-1.1.1.4.6",			"1-1.1.1.4.5",			"1-1.1.1.4.1",			"1-1.1.2.1.7",
	"1-1.1.2.1.6",			"1-1.1.2.1.5",			"1-1.1.2.1.1",			"1-1.1.2.1.2",			"1-1.1.2.1.3",
	"1-1.1.2.1.4",			"1-1.1.2.2.7",			"1-1.1.2.2.6",			"1-1.1.2.2.5",			"1-1.1.2.2.1",
	"1-1.1.2.2.2",			"1-1.1.2.2.3",			"1-1.1.2.2.4",			"1-1.1.2.3.7",			"1-1.1.2.3.6",
	"1-1.1.2.3.5",			"1-1.1.2.3.1",			"1-1.1.2.3.2",			"1-1.1.2.3.3",			"1-1.1.2.4.7",
	"1-1.1.2.4.6",			"1-1.1.2.4.5",			"1-1.1.2.4.1",			"1-1.1.3.1.7",			"1-1.1.3.1.6",
	"1-1.1.3.1.5",			"1-1.1.3.1.1",			"1-1.1.3.1.2",			"1-1.1.3.1.3",			"1-1.1.3.1.4",
	"1-1.1.3.2.7",			"1-1.1.3.2.6",			"1-1.1.3.2.5",			"1-1.1.3.2.1",			"1-1.1.3.2.2",
	"1-1.1.3.2.3",			"1-1.1.3.2.4",			"1-1.1.3.3.7",			"1-1.1.3.3.6",			"1-1.1.3.3.5",
	"1-1.1.3.3.1",			"1-1.1.3.3.2",			"1-1.1.3.3.3",			"1-1.1.3.4.7",			"1-1.1.3.4.6",
	"1-1.1.3.4.5",			"1-1.1.3.4.1",			"1-1.1.4.1.7",			"1-1.1.4.1.6",			"1-1.1.4.1.5",
	"1-1.1.4.1.1",			"1-1.1.4.1.2",			"1-1.1.4.1.3",			"1-1.1.4.1.4",			"1-1.1.4.2.7",
	"1-1.1.4.2.6",			"1-1.1.4.2.5",			"1-1.1.4.2.1",			"1-1.1.4.2.2",			"1-1.1.4.2.3",
	"1-1.1.4.2.4",			"1-1.1.4.3.7",			"1-1.1.4.3.6",			"1-1.1.4.3.5",			"1-1.1.4.3.1",
	"1-1.1.4.3.2",			"1-1.1.4.3.3",			"1-1.1.4.4.7",			"1-1.1.4.4.6",			"1-1.1.4.4.5",
	"1-1.1.4.4.1",			"1-1.1.5.1.7",			"1-1.1.5.1.6",			"1-1.1.5.1.5",			"1-1.1.5.1.1",
	"1-1.1.5.1.2",			"1-1.1.5.1.3",			"1-1.1.5.1.4",			"1-1.1.5.2.7",			"1-1.1.5.2.6",
	"1-1.1.5.2.5",			"1-1.1.5.2.1",			"1-1.1.5.2.2",			"1-1.1.5.2.3",			"1-1.1.5.2.4",
	"1-1.1.5.3.7",			"1-1.1.5.3.6",			"1-1.1.5.3.5",			"1-1.1.5.3.1",			"1-1.1.5.3.2",
	"1-1.1.5.3.3",			"1-1.1.5.4.7",			"1-1.1.5.4.6",			"1-1.1.5.4.5",			"1-1.1.5.4.1",
	"1-1.2.1.1.7",			"1-1.2.1.1.6",			"1-1.2.1.1.5",			"1-1.2.1.1.1",			"1-1.2.1.1.2",
	"1-1.2.1.1.3",			"1-1.2.1.1.4",			"1-1.2.1.2.7",			"1-1.2.1.2.6",			"1-1.2.1.2.5",
	"1-1.2.1.2.1",			"1-1.2.1.2.2",			"1-1.2.1.2.3",			"1-1.2.1.2.4",			"1-1.2.1.3.7",
	"1-1.2.1.3.6",			"1-1.2.1.3.5",			"1-1.2.1.3.1",			"1-1.2.1.3.2",			"1-1.2.1.3.3",
	"1-1.2.1.4.7",			"1-1.2.1.4.6",			"1-1.2.1.4.5",			"1-1.2.1.4.1",			"1-1.2.2.1.7",
	"1-1.2.2.1.6",			"1-1.2.2.1.5",			"1-1.2.2.1.1",			"1-1.2.2.1.2",			"1-1.2.2.1.3",
	"1-1.2.2.1.4",			"1-1.2.2.2.7",			"1-1.2.2.2.6",			"1-1.2.2.2.5",			"1-1.2.2.2.1",
	"1-1.2.2.2.2",			"1-1.2.2.2.3",			"1-1.2.2.2.4",			"1-1.2.2.3.7",			"1-1.2.2.3.6",
	"1-1.2.2.3.5",			"1-1.2.2.3.1",			"1-1.2.2.3.2",			"1-1.2.2.3.3",			"1-1.2.2.4.7",
	"1-1.2.2.4.6",			"1-1.2.2.4.5",			"1-1.2.2.4.1",			"1-1.2.3.1.7",			"1-1.2.3.1.6",
	"1-1.2.3.1.5",			"1-1.2.3.1.1",			"1-1.2.3.1.2",			"1-1.2.3.1.3",			"1-1.2.3.1.4",
	"1-1.2.3.2.7",			"1-1.2.3.2.6",			"1-1.2.3.2.5",			"1-1.2.3.2.1",			"1-1.2.3.2.2",
	"1-1.2.3.2.3",			"1-1.2.3.2.4",			"1-1.2.3.3.7",			"1-1.2.3.3.6",			"1-1.2.3.3.5",
	"1-1.2.3.3.1",			"1-1.2.3.3.2",			"1-1.2.3.3.3",			"1-1.2.3.4.7",			"1-1.2.3.4.6",
	"1-1.2.3.4.5",			"1-1.2.3.4.1",			"1-1.2.4.1.7",			"1-1.2.4.1.6",			"1-1.2.4.1.5",
	"1-1.2.4.1.1",			"1-1.2.4.1.2",			"1-1.2.4.1.3",			"1-1.2.4.1.4",			"1-1.2.4.2.7",
	"1-1.2.4.2.6",			"1-1.2.4.2.5",			"1-1.2.4.2.1",			"1-1.2.4.2.2",			"1-1.2.4.2.3",
	"1-1.2.4.2.4",			"1-1.2.4.3.7",			"1-1.2.4.3.6",			"1-1.2.4.3.5",			"1-1.2.4.3.1",
	"1-1.2.4.3.2",			"1-1.2.4.3.3",			"1-1.2.4.4.7",			"1-1.2.4.4.6",			"1-1.2.4.4.5",
	"1-1.2.4.4.1",			"1-1.2.5.1.7",			"1-1.2.5.1.6",			"1-1.2.5.1.5",			"1-1.2.5.1.1",
	"1-1.2.5.1.2",			"1-1.2.5.1.3",			"1-1.2.5.1.4",			"1-1.2.5.2.7",			"1-1.2.5.2.6",
	"1-1.2.5.2.5",			"1-1.2.5.2.1",			"1-1.2.5.2.2",			"1-1.2.5.2.3",			"1-1.2.5.2.4",
	"1-1.2.5.3.7",			"1-1.2.5.3.6",			"1-1.2.5.3.5",			"1-1.2.5.3.1",			"1-1.2.5.3.2",
	"1-1.2.5.3.3",			"1-1.2.5.4.7",			"1-1.2.5.4.6",			"1-1.2.5.4.5",			"1-1.2.5.4.1"
};

static const char* busid_20[] =
{
	"1-1.1.7",
	"1-1.1.6",
	"1-1.1.5",
	"1-1.2.4",
	"1-1.2.7",
	"1-1.2.6",
	"1-1.2.5",
	"1-1.3.4",
	"1-1.3.7",
	"1-1.3.6",
	"1-1.1.4",
	"1-1.1.2",
	"1-1.1.1",
	"1-1.1.3",
	"1-1.2.3",
	"1-1.2.2",
	"1-1.2.1",
	"1-1.3.3",
	"1-1.3.2",
	"1-1.3.1"
};

static const char* busid_60_2306[] =
{
	"1-1.1.1.4",
	"1-1.1.1.7",
	"1-1.1.1.6",
	"1-1.1.1.5",
	"1-1.1.2.7",
	"1-1.1.2.6",
	"1-1.1.2.5",
	"1-1.1.3.4",
	"1-1.1.3.7",
	"1-1.1.3.6",
	"1-1.1.3.5",
	"1-1.1.4.7",
	"1-1.1.4.6",
	"1-1.1.4.5",
	"1-1.2.5.7",
	"1-1.1.1.3",
	"1-1.1.1.2",
	"1-1.1.1.1",
	"1-1.1.2.4",
	"1-1.1.2.3",
	"1-1.1.2.2",
	"1-1.1.2.1",
	"1-1.1.3.3",
	"1-1.1.3.2",
	"1-1.1.3.1",
	"1-1.1.4.4",
	"1-1.1.4.3",
	"1-1.1.4.2",
	"1-1.1.4.1",
	"1-1.2.5.6",
	"1-1.2.1.4",
	"1-1.2.1.7",
	"1-1.2.1.6",
	"1-1.2.1.5",
	"1-1.2.2.7",
	"1-1.2.2.6",
	"1-1.2.2.5",
	"1-1.2.3.4",
	"1-1.2.3.7",
	"1-1.2.3.6",
	"1-1.2.3.5",
	"1-1.2.4.7",
	"1-1.2.4.6",
	"1-1.2.4.5",
	"1-1.2.5.5",
	"1-1.2.1.3",
	"1-1.2.1.2",
	"1-1.2.1.1",
	"1-1.2.2.4",
	"1-1.2.2.3",
	"1-1.2.2.2",
	"1-1.2.2.1",
	"1-1.2.3.3",
	"1-1.2.3.2",
	"1-1.2.3.1",
	"1-1.2.4.4",
	"1-1.2.4.3",
	"1-1.2.4.2",
	"1-1.2.4.1",
	"1-1.2.5.1"
};

static const char* busid_60[] = 
{
	"1-1.1.1.4",
	"1-1.1.1.3",
	"1-1.1.1.2",
	"1-1.1.1.1",
	"1-1.1.1.5",
	"1-1.1.1.6",
	"1-1.1.1.7",
	"1-1.1.2.4",
	"1-1.1.2.3",
	"1-1.1.2.2",
	"1-1.1.2.1",
	"1-1.1.2.5",
	"1-1.1.2.6",
	"1-1.1.2.7",
	"1-1.1.3.4",
	"1-1.1.3.3",
	"1-1.1.3.2",
	"1-1.1.3.1",
	"1-1.1.3.5",
	"1-1.1.3.6",
	"1-1.1.3.7",
	"1-1.1.4.4",
	"1-1.1.4.3",
	"1-1.1.4.2",
	"1-1.1.4.1",
	"1-1.1.4.5",
	"1-1.1.4.6",
	"1-1.1.4.7",
	"1-1.1.5.4",
	"1-1.1.5.3",
	"1-1.2.1.4",
	"1-1.2.1.3",
	"1-1.2.1.2",
	"1-1.2.1.1",
	"1-1.2.1.5",
	"1-1.2.1.6",
	"1-1.2.1.7",
	"1-1.2.2.4",
	"1-1.2.2.3",
	"1-1.2.2.2",
	"1-1.2.2.1",
	"1-1.2.2.5",
	"1-1.2.2.6",
	"1-1.2.2.7",
	"1-1.2.3.4",
	"1-1.2.3.3",
	"1-1.2.3.2",
	"1-1.2.3.1",
	"1-1.2.3.5",
	"1-1.2.3.6",
	"1-1.2.3.7",
	"1-1.2.4.4",
	"1-1.2.4.3",
	"1-1.2.4.2",
	"1-1.2.4.1",
	"1-1.2.4.5",
	"1-1.2.4.6",
	"1-1.2.4.7",
	"1-1.2.5.4",
	"1-1.2.5.3"
};

int ty_usb_add(char *switch_name,int type)
{
	struct _ty_usb *stream;
	int result;
	stream=malloc(sizeof(struct _ty_usb));
	if(stream==NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _ty_usb));
    memcpy(stream->switch_name,switch_name,strlen(switch_name));
	//ty_usb_out("��ϵͳ����ʱ,ֱ�Ӵ�����HUB\n");
	stream->hub=DF_TY_USB_HUB_OPEN;//���˷��汾���������Ű汾����
	stream->type=type;
	result = ty_file_add(DF_DEV_TY_USB, stream, "ty_usb", (struct _file_fuc *)&ty_usb_fuc);
	if(result<0)
		free(stream);
	return result;
}

static int ty_usb_open(struct ty_file	*file)
{
	struct _ty_usb  *stream;
    int result,j;
	stream=file->pro_data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
    //ty_usb_out("��ty_usb_openģ��\n");
    if(stream->state==0)
    {   //ty_usb_out("��һ��ʹ����Ҫ����,usb_dev_class\n");
        stream->lock=_lock_open("ty_usb_open.c",NULL);
        if(stream->lock<0)
        {   ty_usb_out("������ʧ��\n");
            return stream->lock;
        }
        //ty_usb_out("�򿪿���ģ��\n");
        stream->switch_fd=ty_open(stream->switch_name,0,0,NULL);
        if(stream->switch_fd<0)
        {   ty_usb_out("�򿪿���ʧ��\n");
            _lock_close(stream->lock);
            return stream->switch_fd;
        }
       // ty_usb_out("��ȡ�豸����Ϣ\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_INFOR,&stream->data.stm32_infor))<0)
        {   ty_usb_out("��ȡ�豸��Ϣʧ��\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //ty_usb_out("��ȡ�豸��id��Ϣ\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_ID,&stream->data.self_id))<0)
        {   ty_usb_out("��ȡ�豸idʧ��\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //ty_usb_out("��ȡ�����id\n");
		uint8 gg_id[6];
		memset(gg_id,0,sizeof(gg_id));
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_CLASS,DF_SWITCH_CM_CLASS_ID,stream->data.dev.id,1))<0)
        {   ty_usb_out("��ȡ����idʧ��\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        //ty_usb_out("��ȡ���������汾\n");
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_DEV_SOFT_EDIT,stream->data.dev.soft))<0)
        {   ty_usb_out("��ȡ��������汾ʧ��\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;  
        }
        struct _switch_dev_pcb   pcb_id[sizeof(stream->data.dev.pcb)/sizeof(stream->data.dev.pcb[0])];
        //ty_usb_out("��ȡ������Ϣ\n");
        memset(pcb_id,0,sizeof(pcb_id));
        if((result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_GET_PCB,stream->data.dev.id,
            pcb_id,sizeof(pcb_id)/sizeof(pcb_id[0])))<0)
        {   ty_usb_out("��ȡ����Ⱥ������Ϣʧ��\n");
            ty_close(stream->switch_fd);
            _lock_close(stream->lock);
            return result;
        }
        stream->data.dev.n=result;
		if(stream->hub==DF_TY_USB_HUB_OPEN)
		{	ty_usb_out("��hub\n");
			ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN_HUB);
		}
		for(j=0,stream->data.dev.usb_port_n=0;j<result;j++)
        {   memcpy(stream->data.dev.pcb[j].id,pcb_id[j].id,sizeof(stream->data.dev.pcb[0].id));
            stream->data.dev.pcb[j].n=pcb_id[j].n;
            stream->data.dev.usb_port_n+=stream->data.dev.pcb[j].n;       
        }
		if (stream->data.dev.usb_port_n> DF_TY_USB_PORT_N_MAX)
		{
			logout(INFO, "SYSTEM", "USB", "�˿�������ȡʧ��,��ȡ���Ķ˿�����Ϊ%d\r\n", stream->data.dev.usb_port_n);
			//return -1;
		}
        for(j=0;j<stream->data.dev.usb_port_n;j++)
        {   
			stream->data.dev.port_lock[j]=_lock_open("d",NULL);
			if (stream->data.dev.port_lock[j] < 0)
			{
				logout(INFO, "SYSTEM", "USB", "ty_usb�˿ڻ�������ʧ��\r\n");
				return -1;
			}
        }
    }
    for(j=0;j<sizeof(stream->fd)/sizeof(stream->fd[0]);j++)
	{	if(stream->fd[j].state==0)
			break;
	}
	if(j==sizeof(stream->fd)/sizeof(stream->fd[0]))
	{	return DF_ERR_PORT_CORE_SLAVE_SPACE;
	}
	memset(&stream->fd[j],0,sizeof(stream->fd[0]));
	stream->fd[j].dev=stream; 
	stream->fd[j].state=1; 
	stream->fd[j].mem_fd=&file->fd[j];
	stream->state++;	
    ty_usb_out("��ty_usb�ɹ�:j=%d\n",j);
    //if(ty_pd==0)
      //  ty_pd_add_file("ty_usb");
	return j+1;	   
}

static int ty_usb_ctl(void *data,int fd,int cm,va_list args)
{
	struct _ty_usb  *stream;
    struct _ty_usb_fd   *id;
    int i;
    stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
    for(i=0;i<sizeof(ctl_fun)/sizeof(ctl_fun[0]);i++)
    {   if(cm==ctl_fun[i].cm)
            return ctl_fun[i].ctl(id,args);
    }
    return DF_ERR_PORT_CORE_CM;    
}

static int ty_usb_close(void *data,int fd)
{
	struct _ty_usb  *stream;
    struct _ty_usb_fd   *id;
    int j;
	stream=data;
	if((stream==NULL))	
		return DF_ERR_PORT_CORE_FD_NULL;
	if((fd==0)||(fd>sizeof(stream->fd)/sizeof(stream->fd[0])))
		return DF_ERR_PORT_CORE_FD_Z;
	fd--;
	id=&stream->fd[fd];
	if(id->state==0)
		return DF_ERR_PORT_CORE_SLAVE_DEV;
	id->state=0;
	stream->state--;
	memset(id,0,sizeof(stream->fd[0]));
    if(stream->state==0)			//ȫ���ر���
	{   ty_close(stream->switch_fd);
        for(j=0;j<stream->data.dev.usb_port_n;j++)
        {   _lock_close(stream->data.dev.port_lock[j]);
        }
        _lock_close(stream->data.dev.port_lock[j]);
	}
    return 0;   
}

static int _get_all_port_status(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_ALL_PORT_STATUS, all_info);
	return result;
}

static int _set_fan(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_CONTRL_FAN, all_info);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _ctl_dev_restore(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_RESTORE, NULL);
	if (result<0)
	{
		return result;
	}
	return 0;
}

static int _get_all_ad_status(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 *all_info;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	all_info = va_arg(args, uint8 *);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_AD_STATUS, all_info);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _set_led_double_light_yellow(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_DOUBLE_LIGHT_YELLOW, port);
	if (result<0)
	{
		return result;
	}
	return result;

}

static int _ctl_bluetooth_mode(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 * control;
	control = va_arg(args, uint8 *);
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_BLUETOOTH_MODE, control);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _ctl_bluetooth_power(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	uint8 * control;
	control = va_arg(args, uint8 *);
	stream = id->dev;
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_BLUETOOTH_POWER, control);
	if (result < 0)
	{
		return result;

	}
	return result;
}

static int _ctl_dev_conn(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	stream = id->dev;
	return ty_ctl(stream->switch_fd, DF_SWITCH_CM_CONN, NULL);

}

static int _set_busid(struct _ty_usb_fd *id,va_list args)
{
	char *busid;
    ty_usb_out("��������·��busid\n");
    busid=va_arg(args,char *);
    _lock_set(id->dev->lock);
    memcpy(id->dev->data.dev.busid,busid,strlen(busid));
    _lock_un(id->dev->lock);
    return 0;
}

static int _get_busid(struct _ty_usb_fd *id,va_list args)
{
	char *busid;
    ty_usb_out("��ȡ����·��busid\n");
    busid=va_arg(args,char *);
    if(busid==NULL)
        return -1;
    _lock_set(id->dev->lock);
    memcpy(busid,id->dev->data.dev.busid,strlen(id->dev->data.dev.busid));
    _lock_un(id->dev->lock);
    return strlen(id->dev->data.dev.busid);    
}

static int _set_port_n(struct _ty_usb_fd *id,va_list args)
{
	ty_usb_out("���ö˿�����˳��\n");
    _lock_set(id->dev->lock);
    id->dev->port_n=va_arg(args,int);
    _lock_un(id->dev->lock);
    return id->dev->port_n;
}

static int _get_port_n(struct _ty_usb_fd *id,va_list args)
{
	int port_n;
    ty_usb_out("��ȡ�˿�����˳��\n");
    _lock_set(id->dev->lock);
    port_n=id->dev->port_n;
    _lock_un(id->dev->lock);
    return port_n;
}

static int _set_enum_timer(struct _ty_usb_fd *id,va_list args)
{
	ty_usb_out("�������ö��ʱ��\n");
    _lock_set(id->dev->lock);
    id->dev->enum_timer=va_arg(args,int);
    ty_usb_out("���ö��ʱ��Ϊ:%d\n",id->dev->enum_timer);
    _lock_un(id->dev->lock);
    return id->dev->enum_timer;
}

static int _get_enum_timer(struct _ty_usb_fd *id,va_list args)
{
	int enum_timer;
    ty_usb_out("��ȡ���ö��ʱ��\n");
    _lock_set(id->dev->lock);
    enum_timer=id->dev->enum_timer;
    _lock_un(id->dev->lock);
    return enum_timer;
}

static int _get_busid2port(struct _ty_usb_fd *id,va_list args)
{
	char *busid;
    struct _ty_usb  *stream;
    char port_busid[DF_TY_USB_BUSID];
    int n,i;
    ty_usb_out("ͨ��busid��ȡ����Ӧ��port\n");
    busid=va_arg(args,char *);
    stream=id->dev;
    n=stream->data.dev.usb_port_n;
    for(i=0;i<n;i++)
    {   memset(port_busid,0,sizeof(port_busid));
        get_port_busid(stream,i+1,port_busid);
        if(memcmp(busid,port_busid,strlen(port_busid))==0)
            return i+1;
    }
    return -1;
}

static int _get_machine_id(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	char * ter_id;
	stream = id->dev;
	ter_id = va_arg(args, char *);
	bcd2asc(stream->data.self_id.id, (uint8 *)ter_id, sizeof(stream->data.self_id.id));
	return 0;
}

static int _get_register_whether(struct _ty_usb_fd *id,va_list args)
{
    struct _ty_usb  *stream;
	uint8 buf[12];
	uint8 idbuf[12];
	//int i;
	long int iddata;
	//char port_busid[DF_TY_USB_BUSID];
    memset(buf,0,sizeof(buf));
	buf[0]=0x30;
	buf[1]=0x36;
	buf[2]=0x38;
	stream=id->dev;
	bcd2asc(stream->data.self_id.id,idbuf,sizeof(stream->data.self_id.id));
	/*ty_usb_out("������ţ�");
	for(i=0;i<12;i++)
	{
		ty_usb_out("%02x ",idbuf[i]);
	}
	ty_usb_out("\n");*/
	if(memcmp(idbuf,buf,3)!=0)
	{
		ty_usb_out("δ���û�����Ż������Ŵ���\n");
		return 0;
	}
	
	iddata=(idbuf[6]&0x0f)*10000+(idbuf[7]&0x0f)*1000+(idbuf[8]&0x0f)*100+(idbuf[9]&0x0f)*10+(idbuf[10]&0x0f)*1;
	//ty_usb_out("ID��%d\n",iddata);
	if(iddata<0x64)
	{	
		return 2;
	}
    return 1;
}

static int _get_tryout_whether(struct _ty_usb_fd *id,va_list args)
{
	uint8 buf[8];
	//uint8 sbuf[4];
	uint8 getcbuf[8];
	uint8 tryouttime[8];
	uint8 today[8];
	int result;
	//int i;
	struct day_t day1,day2;
	//buf[0]=0x32;buf[1]=0x30;buf[2]=0x31;buf[3]=0x36;
	//buf[4]=0x31;buf[5]=0x32;buf[6]=0x31;buf[7]=0x33;
	//asc2bcd(buf,sbuf,8);
	//result = _t_file_c_write("/etc/gettry.conf",sbuf,4);
	//sync();
	memset(buf,0,sizeof(buf));
	if(read_file("/etc/gettry.conf",(char *)buf,sizeof(buf))<0)
	{	ty_usb_out("��ȡ�ļ�:[%s]ʧ��\n","/etc/gettry.conf");
		return -1;
	}
	//ty_usb_out("��ȡ�����ݣ�");
	//for(i=0;i<4;i++)
	//{	
	//	ty_usb_out("%02x ",buf[i]);
	//}
	//ty_usb_out("\n");
	bcd2asc(buf,tryouttime,4);

	//ty_usb_out("����ʱ�䣺");
	//for(i=0;i<8;i++)
	//{	
	//	ty_usb_out("%02x ",tryouttime[i]);
	//}
	//ty_usb_out("\n");
	day1.year=tryouttime[0]*1000+tryouttime[1]*100+tryouttime[2]*10+tryouttime[3];
	day1.month=tryouttime[4]*10+tryouttime[5];
	day1.day=tryouttime[6]*10+tryouttime[7];

	clock_get(getcbuf);
	bcd2asc(getcbuf,today,4);
	//ty_usb_out("��ǰʱ�䣺");
	//for(i=0;i<8;i++)
	//{	
	//	ty_usb_out("%02x ",today[i]);
	//}
	//ty_usb_out("\n");
	day2.year=today[0]*1000+today[1]*100+today[2]*10+today[3];
	day2.month=today[4]*10+today[5];
	day2.day=today[6]*10+today[7];

	result = dmax(&day2,&day1);
	//ty_usb_out("_get_tryout_whether result =%d\n",result);
	if(result>0)
	{
		ty_usb_out("���������ѹ�\n");
		return -1;
	}
	return 0;
}

static int _ctl_port_no_dev_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
	//ty_usb_out("��ȥ_ctl_port_no_dev_led\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_NO_DEV,port);
    if(result<0)
    {   ty_usb_out("_ctl_port_no_dev_led:STM32 ctl port led no dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}

static int _ctl_port_dev_local_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
	//ty_usb_out("��ȥ_ctl_port_dev_local_led\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_DEV_LOCAL,port);
    if(result<0)
    {   ty_usb_out("_ctl_port_dev_local_led:STM32 ctl port led local dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}

static int _ctl_port_dev_share_led(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
	 stream=id->dev;
    port=va_arg(args,int);
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_PORT_DEV_SHARE,port);
    if(result<0)
    {   ty_usb_out("_ctl_port_dev_share_led:STM32 ctl port led share dev error\n");
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    return 0;   
}


static int _set_led_red_off_gren_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_OFF, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_off_gren_off:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_on_gren_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_OFF, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_on_gren_off:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_off_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_ON, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_off_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_on_gren_twi(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_TWI, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_on_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_ON_GREN_ON, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_on_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_twi_gren_on(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_ON, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_twi_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_red_twi_gren_twi_1s(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_TWI_1S, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_twi_gren_on:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_read_off_green_twi(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_OFF_GREN_TWI, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _set_led_read_twi_green_off(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	stream = id->dev;
	port = va_arg(args, int);
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_PORT_RED_TWI_GREN_OFF, port);
	if (result<0)
	{
		ty_usb_out("_set_led_red_on_gren_twi:STM32 ctl port led share dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}

static int _get_public_praviate_key(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int result;
	unsigned char *key_s;
	stream = id->dev;
	key_s = va_arg(args, unsigned char *);
	ty_usb_out("��ȥ_get_public_praviate_key\n");
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_GET_KEY_VALUE, key_s);
	if (result<0)
	{
		ty_usb_out("_ctl_port_dev_local_led:STM32 ctl port led local dev error\n");
		return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
	}
	return 0;
}



static int _open_port_power(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
    //char bus[4];
    //char busid[DF_TY_USB_BUSID];
    stream=id->dev;

    port=va_arg(args,int);

    _lock_set(stream->data.dev.port_lock[port-1]);
    ty_usb_out("_open_port_power:Send code to STM32\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN,stream->data.dev.id,port);
    if(result<0)
    {   ty_usb_out("_open_port_power:STM32 open port error\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    //stream->data.dev.port_state[port-1]=1;
    _lock_un(stream->data.dev.port_lock[port-1]);
    ty_usb_out("_open_port_power:�˿ڴ򿪳ɹ�\n");
    return 0;       
}

static int _close_port_power(struct _ty_usb_fd *id, va_list args)
{
	struct _ty_usb  *stream;
	int port, result;
	//char bus[4];
	//char busid[DF_TY_USB_BUSID];
	stream = id->dev;
	port = va_arg(args, int);
	_lock_set(stream->data.dev.port_lock[port - 1]);
	ty_usb_out("_close_port_power:Send code to STM32\n");
	result = ty_ctl(stream->switch_fd, DF_SWITCH_CM_CLOSE, stream->data.dev.id, port);
	if (result<0)
	{
		ty_usb_out("_close_port_power:STM32 close port error\n");
		_lock_un(stream->data.dev.port_lock[port - 1]);
		return DF_ERR_PORT_CORE_TY_USB_STM32_CLOSE;
	}
	//stream->data.dev.port_state[port-1]=1;
	_lock_un(stream->data.dev.port_lock[port - 1]);
	ty_usb_out("_close_port_power:�˿ڹرճɹ�\n");
	return 0;
}

static int _get_port_exist(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result;
    char bus[4];
    char busid[DF_TY_USB_BUSID];
    stream=id->dev;
	
    port=va_arg(args,int);
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    memset(busid,0,sizeof(busid));
	
    get_port_busid(stream,port,busid);
	//printf("_get_port_exist busid=%s\r\n",busid);
	//ty_usb_out("���busid %s\n",busid);
	//result = get_busid_over_timer(busid,1);
	//printf("")
    result=get_busid_exist(busid);			//wang 2022/11/24 Ŀǰ���ּ�����̨�ǽڵ�������ָú�������ֵ����1���������������ż����
	if(result<0)
    {   
        return result;
    }
    return 0;

}

int my_get_port_exit(int port,_so_note *app_note)
{
	 char busid[DF_TY_USB_BUSID] = {0x00};
	char tmp_busid[32] = {0x00},busid_hub[32] = {0x00},busid_hub_dev[32] = {0x00};
	//printf("my_get_busid app_note->machine_type = %d\n",app_note->machine_type);
	if(DF_TY_MACHINE_TYPE_MB2303 == app_note->machine_type)
	{
		memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
		if(_usb_file_exist(busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
	}
	else if(DF_TY_MACHINE_TYPE_MB2306 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_60_2306[port - 1], strlen(busid_60_2306[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
			if(_usb_file_exist(busid)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			else
			{
				return 0;
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2212 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_120[port - 1], strlen(busid_120[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
			if(_usb_file_exist(busid)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			else
			{
				return 0;
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2202 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_20[port - 1], strlen(busid_20[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
			if(_usb_file_exist(busid)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			else
			{
				return 0;
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2110 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
			if(_usb_file_exist(busid)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			else
			{
				return 0;
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2108 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_68[port - 1], strlen(busid_68[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
			if(_usb_file_exist(busid)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			else
			{
				return 0;
			}
			
		}
	}
	return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
}

int my_get_busid(int port,char *busid,_so_note *app_note)
{
	//memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
	char tmp_busid[32] = {0x00},busid_hub[32] = {0x00},busid_hub_dev[32] = {0x00};
	printf("my_get_busid app_note->machine_type = %d\n",app_note->machine_type);
	if(DF_TY_MACHINE_TYPE_MB2303 == app_note->machine_type)
	{
		memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
	}
	else if(DF_TY_MACHINE_TYPE_MB2202 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_20[port - 1], strlen(busid_20[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		printf("my_get_busid busid_hub = %s,busid_hub_dev = %s\n",busid_hub,busid_hub_dev);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				memcpy(busid,busid_hub,strlen(busid_hub));
			}
			else
			{
				memcpy(busid,busid_hub_dev,strlen(busid_hub_dev));
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2212 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_120[port - 1], strlen(busid_120[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		printf("my_get_busid busid_hub = %s,busid_hub_dev = %s\n",busid_hub,busid_hub_dev);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				memcpy(busid,busid_hub,strlen(busid_hub));
			}
			else
			{
				memcpy(busid,busid_hub_dev,strlen(busid_hub_dev));
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2306 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_60_2306[port - 1], strlen(busid_60_2306[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		printf("my_get_busid busid_hub = %s,busid_hub_dev = %s\n",busid_hub,busid_hub_dev);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				memcpy(busid,busid_hub,strlen(busid_hub));
			}
			else
			{
				memcpy(busid,busid_hub_dev,strlen(busid_hub_dev));
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2110 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		printf("my_get_busid busid_hub = %s,busid_hub_dev = %s\n",busid_hub,busid_hub_dev);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				memcpy(busid,busid_hub,strlen(busid_hub));
			}
			else
			{
				memcpy(busid,busid_hub_dev,strlen(busid_hub_dev));
			}
			
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2108 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_68[port - 1], strlen(busid_68[port - 1]));
		sprintf(busid_hub,"%s.1",tmp_busid);
		sprintf(busid_hub_dev,"%s.2",tmp_busid);
		printf("my_get_busid busid_hub = %s,busid_hub_dev = %s\n",busid_hub,busid_hub_dev);
		if(_usb_file_exist(busid_hub)<0)   //�����ڶ���hub
		{
			memcpy(busid,tmp_busid,strlen(tmp_busid));
		}
		else							 //���ڶ���hub
		{
			//�жϻ�е�����Ƿ����豸
			if(_usb_file_exist(busid_hub_dev)<0)
			{
				memcpy(busid,busid_hub,strlen(busid_hub));
			}
			else
			{
				memcpy(busid,busid_hub_dev,strlen(busid_hub_dev));
			}
			
		}
	}
	return 0;
}

int my_get_busid_only(int port,char *busid,_so_note *app_note)
{
	//memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
	// char tmp_busid[32] = {0x00},busid_hub[32] = {0x00},busid_hub_dev[32] = {0x00};
	// printf("my_get_busid app_note->machine_type = %d\n",app_note->machine_type);
	if(DF_TY_MACHINE_TYPE_MB2303 == app_note->machine_type)
	{
		memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
	}
	else if(DF_TY_MACHINE_TYPE_MB2202 == app_note->machine_type)
	{
		memcpy(busid, busid_20[port - 1], strlen(busid_20[port - 1]));
		
	}
	else if(DF_TY_MACHINE_TYPE_MB2212 == app_note->machine_type)
	{
		memcpy(busid, busid_120[port - 1], strlen(busid_120[port - 1]));
	}
	else if(DF_TY_MACHINE_TYPE_MB2306 == app_note->machine_type)
	{
		memcpy(busid, busid_60_2306[port - 1], strlen(busid_60_2306[port - 1]));
	}
	else if(DF_TY_MACHINE_TYPE_MB2110 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
	}
	else if(DF_TY_MACHINE_TYPE_MB2108 == app_note->machine_type)
	{
		memcpy(busid, busid_68[port - 1], strlen(busid_68[port - 1]));
	}
	return 0;
}


static int get_busid_exist(char *busid)
{
	//printf("get_busid_exist = %s\n",busid);
	if(_usb_file_exist(busid)<0)
	{
        return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
	}
    return 0;      
}

//port��1��ʼ
int my_judge_exit_dev_robotarm(int port,_so_note *app_note,int dev_or_roborarm)  //�ǻ�е�ۻ���usb�豸��0��usb�豸��1�ǻ�е��
{
	char busid[32] = {0x00},tmp_busid[32] = {0x00},busid_roborarm[32] = {0x00},busid_dev[32] = {0x00};
	if(DF_TY_MACHINE_TYPE_MB2303 == app_note->machine_type)
	{
		//if(0 == dev_or_roborarm)   //�豸
			memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
		// else					   //��е��
		// 	memcpy(busid, busid_60[port - 2], strlen(busid_60[port - 2]));
		if(_usb_file_exist(busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
	}
	else if(DF_TY_MACHINE_TYPE_MB2306 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_60_2306[port - 1], strlen(busid_60_2306[port - 1]));
		sprintf(busid_roborarm,"%s.1",tmp_busid);
		sprintf(busid_dev,"%s.2",tmp_busid);
		if(0 == dev_or_roborarm)   //�豸
		{
			if(_usb_file_exist(busid_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else
		{
			if(_usb_file_exist(busid_roborarm)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2202 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_20[port - 1], strlen(busid_20[port - 1]));
		sprintf(busid_roborarm,"%s.1",tmp_busid);
		sprintf(busid_dev,"%s.2",tmp_busid);
		if(0 == dev_or_roborarm)   //�豸
		{
			if(_usb_file_exist(busid_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else
		{
			if(_usb_file_exist(busid_roborarm)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2212 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_120[port - 1], strlen(busid_120[port - 1]));
		sprintf(busid_roborarm,"%s.1",tmp_busid);
		sprintf(busid_dev,"%s.2",tmp_busid);
		if(0 == dev_or_roborarm)   //�豸
		{
			if(_usb_file_exist(busid_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else
		{
			if(_usb_file_exist(busid_roborarm)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2110 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		sprintf(busid_roborarm,"%s.1",tmp_busid);
		sprintf(busid_dev,"%s.2",tmp_busid);
		if(0 == dev_or_roborarm)   //�豸
		{
			if(_usb_file_exist(busid_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else
		{
			if(_usb_file_exist(busid_roborarm)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
	}
	else if(DF_TY_MACHINE_TYPE_MB2108 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_68[port - 1], strlen(busid_68[port - 1]));
		sprintf(busid_roborarm,"%s.1",tmp_busid);
		sprintf(busid_dev,"%s.2",tmp_busid);
		if(0 == dev_or_roborarm)   //�豸
		{
			if(_usb_file_exist(busid_dev)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
		else
		{
			if(_usb_file_exist(busid_roborarm)<0)
			{
				return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
			}
			return 0;
		}
	}
	return 0;
}

int my_judge_exit_dev(int port,_so_note *app_note)
{
	char busid[32] = {0x00},tmp_busid[32] = {0x00},busid_roborarm[32] = {0x00},busid_dev[32] = {0x00};
	if(DF_TY_MACHINE_TYPE_MB2303 == app_note->machine_type)
	{
		memcpy(busid, busid_60[port - 1], strlen(busid_60[port - 1]));
		if(_usb_file_exist(busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
	}
	else if(DF_TY_MACHINE_TYPE_MB2110 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		if(_usb_file_exist(tmp_busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
		
	}
	else if(DF_TY_MACHINE_TYPE_MB2306 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_60_2306[port - 1], strlen(busid_60_2306[port - 1]));
		if(_usb_file_exist(tmp_busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
		
	}
	else if(DF_TY_MACHINE_TYPE_MB2202 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_20[port - 1], strlen(busid_20[port - 1]));
		if(_usb_file_exist(tmp_busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
		
	}
	else if(DF_TY_MACHINE_TYPE_MB2212 == app_note->machine_type)
	{
		// memcpy(busid, busid_100[port - 1], strlen(busid_100[port - 1]));
		memcpy(tmp_busid, busid_120[port - 1], strlen(busid_120[port - 1]));
		if(_usb_file_exist(tmp_busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
		
	}
	else if(DF_TY_MACHINE_TYPE_MB2108 == app_note->machine_type)
	{
		memcpy(tmp_busid, busid_68[port - 1], strlen(busid_68[port - 1]));
		if(_usb_file_exist(tmp_busid)<0)
		{
			return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;
		}
		return 0;
		
	}
	return 0;
}


static int _open_port(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result,enum_timer;
    char bus[4];
    char busid[DF_TY_USB_BUSID];
    stream=id->dev;
    ty_usb_out("��usb�˿�==================\n");
    if(strlen(stream->data.dev.busid)<3)
    {   ty_usb_out("û�п��õ�busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   ty_usb_out("�����˿�����\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    _lock_set(stream->lock);
    if(_usb_file_get_bus_dev_n(bus)>DF_TY_BUS_DEV_NUMBER)
    {   ty_usb_out("�Ѿ�������USBö�ٵ�����\n");
        _lock_un(stream->lock);
        return DF_ERR_PORT_CORE_TY_USB_BUS_NUMBER;
    }
    _lock_un(stream->lock);
    ty_usb_out("��ȡʵ������˿�\n");
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    ty_usb_out("��ȡ�˿ڵ�busid\n");
    _lock_set(stream->lock);
    memset(busid,0,sizeof(busid));
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    ty_usb_out("��ȡ���ö��ʱ��\n");
    _lock_set(stream->lock);
    enum_timer=stream->enum_timer;
    _lock_un(stream->lock);
    ty_usb_out("�˿�:%d,busid=%s,enum_timer=%d\n",port,busid,enum_timer);
    _lock_set(stream->data.dev.port_lock[port-1]);
    if(id->dev->data.dev.port_state[port-1]!=0)
    {   ty_usb_out("�ö˿ڶ˿��Ѿ���\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return port;
    }
    ty_usb_out("���������õ�Ƭ���򿪶˿�\n");
    result=ty_ctl(stream->switch_fd,DF_SWITCH_CM_OPEN,stream->data.dev.id,port);
    if(result<0)
    {   ty_usb_out("�����򿪶˿�ʧ��\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return DF_ERR_PORT_CORE_TY_USB_STM32_OPEN;
    }
    ty_usb_out("��ȡusb���ļ�ϵͳ�Ƿ��\n");
    result=get_usb_file(busid,enum_timer,stream->type);
    if(result<0)
    {   ty_usb_out("�˿��Ѿ���û�з����豸\n");
        //ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
        _lock_un(stream->data.dev.port_lock[port-1]);
        return result;
    }
    stream->data.dev.port_state[port-1]=1;
    _lock_un(stream->data.dev.port_lock[port-1]);
    ty_usb_out("�˿ڴ򿪳ɹ�\n");
    return 0;       
}


static int _close_port_power_just(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result; 
    stream=id->dev;
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   ty_usb_out("�����˿�����\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    ty_usb_out("��ȡʵ������˿�\n");
    port=get_port(id->dev,port);    
    result = ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
    id->dev->data.dev.port_state[port-1]=0;
	//usleep(500000);
    return result;   
}

static int _close_port(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,result,enum_timer;
    char bus[4];
    char busid[DF_TY_USB_BUSID];    
    stream=id->dev;
    if(strlen(stream->data.dev.busid)<3)
    {   ty_usb_out("û�п��õ�busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    port=va_arg(args,int);
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   ty_usb_out("�����˿�����\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    ty_usb_out("��ȡʵ������˿�\n");
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    ty_usb_out("��ȡ�˿ڵ�busid\n");
    _lock_set(stream->lock);
    memset(busid,0,sizeof(busid));
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    ty_usb_out("��ȡ���ö��ʱ��\n");
    _lock_set(stream->lock);
    enum_timer=stream->enum_timer;
    _lock_un(stream->lock);
    ty_usb_out("�˿�:%d,busid=%s,enum_timer=%d\n",port,busid,enum_timer);
    _lock_set(stream->data.dev.port_lock[port-1]);
    if(id->dev->data.dev.port_state[port-1]==0)
    {   ty_usb_out("�ö˿ڶ˿��Ѿ��ر�\n");
        _lock_un(stream->data.dev.port_lock[port-1]);
        return port;
    }
    ty_ctl(stream->switch_fd,DF_SWITCH_CM_CLOSE,stream->data.dev.id,port);
    id->dev->data.dev.port_state[port-1]=0;
    ty_usb_out("�жϸ��豸�Ƿ����ļ�ϵͳ����ʧ\n");
    result=wait_usb_file_no(busid,enum_timer);
    if(result<0)
    {   _lock_un(stream->data.dev.port_lock[port-1]);
        return result;    
    }
	//if(stream->hub!=DF_TY_USB_HUB_OPEN)
	//{	ty_usb_out("�жϸ��豸�ر�ʱ,hub�Ƿ�Ҳ�ر�\n");
	//    if(juge_sub_hub(stream,port)>=0)
	//    {   char busid_hub[DF_TY_USB_BUSID];
	//        memset(busid_hub,0,sizeof(busid_hub));
	//        memcpy(busid_hub,busid,strlen(busid)-2);
	//        ty_usb_out("�жϸ��豸��HUB�Ƿ����\n");
	//        result=wait_usb_file_no(busid_hub,enum_timer);
	//        if(result<0)
	//        {   _lock_un(stream->data.dev.port_lock[port-1]);
	//            return result; 
	//        }
	//    }
	//    ty_usb_out("�жϸ�PCB�ر�ʱ,PCB��HUB�Ƿ�ر�\n");
	//    if(juge_pcb_hub(stream,port)>=0)
	//    {   char busid_pcb[DF_TY_USB_BUSID];
	//        memset(busid_pcb,0,sizeof(busid_pcb));
	//        memcpy(busid_pcb,busid,strlen(busid)-4);
	//        ty_usb_out("�жϸ��豸��PCB HUB�Ƿ����\n");
	//        result=wait_usb_file_no(busid_pcb,enum_timer);
	//        if(result<0)
	//        {   _lock_un(stream->data.dev.port_lock[port-1]);
	//            return result; 
	//        }
	//    }
	//}	
    _lock_un(stream->data.dev.port_lock[port-1]);    
	usleep(500000);
    return result;   
}

//static int juge_sub_hub(struct _ty_usb  *stream,int port)
//{   int i;
//    ty_usb_out("�ж��Ƿ���HUBҪ�ж�\n");
//    port--;
//    port=port/7;
//    port=port*7;
//    for(i=0;i<7;i++)
//    {   if(stream->data.dev.port_state[i+port]!=0)
//            return -1;
//    }
//    return 0;
//}
//
//static int juge_pcb_hub(struct _ty_usb  *stream,int port)
//{   int i;
//    ty_usb_out("�ж��Ƿ���PCBҪ�ж�\n");
//    port--;
//    port=port/35;
//    port=port*35;
//    for(i=0;i<35;i++)
//    {   if(stream->data.dev.port_state[i+port]!=0)
//            return -1;
//    }
//    return 0;  
//}

static int get_busid(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port;
    char bus[4];
    char *busid;
    ty_usb_out("��ȡ�豸��busid\n");
    stream=id->dev;
    port=va_arg(args,int);
    busid=va_arg(args,char *);
    if(strlen(stream->data.dev.busid)<3)
    {   ty_usb_out("û�п��õ�busid\n");
        return DF_ERR_PORT_CORE_TY_USB_MAIN_BUSID;
    }
    if((port==0)||(port>stream->data.dev.usb_port_n))    
    {   ty_usb_out("�����˿�����\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
    }
    memset(bus,0,sizeof(bus));
    memcpy(bus,stream->data.dev.busid,3);
    ty_usb_out("��ȡʵ������˿� port=%d\n",port);
    _lock_set(stream->lock);
    port=get_port(id->dev,port);
    _lock_un(stream->lock);
    ty_usb_out("��ȡ�˿ڵ�busid,port=%d\n",port);
    _lock_set(stream->lock);
    get_port_busid(stream,port,busid);
    _lock_un(stream->lock);
    return 0;   
}

static int get_port_number(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int n;
    //ty_usb_out("��ȡ�豸������\n");
    stream=id->dev;
    _lock_set(stream->lock);
    n=stream->data.dev.usb_port_n;
    _lock_un(stream->lock);
    //ty_usb_out("�豸����:%d\n",n);
    return n;   
}

static int juge_port(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream; 
    int port;
	//ty_usb_out("juge_port ��ʼ\n");
    stream=id->dev;
    port=va_arg(args,int);
    if((port>stream->data.dev.usb_port_n)||(port==0))
        return DF_ERR_PORT_CORE_TY_USB_PORT_OVER;
	//ty_usb_out("juge_port ����\n");
    return 0;       
}

static int get_port_maid(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port;
    uint8 *maid;
    port=va_arg(args,int);
    maid=va_arg(args,uint8 *);
    stream=id->dev;
    if(id==NULL)
        return -1;
    memcpy(maid,stream->data.dev.id,sizeof(stream->data.dev.id));
    return port;    
}

static int get_port_pcb(struct _ty_usb_fd *id,va_list args)
{
	struct _ty_usb  *stream;
    int port,i,n;
    uint8 *subid;
    port=va_arg(args,int);
    subid=va_arg(args,uint8 *);
    stream=id->dev;
    if(id==NULL)
        return -1;
    port--;
    for(i=0,n=0;i<stream->data.dev.n;i++,n+=stream->data.dev.pcb[i].n)
    {   if(port<n+stream->data.dev.pcb[i].n)
            break;
        continue;
    }
    port-=n;
    memcpy(subid,stream->data.dev.pcb[i].id,sizeof(stream->data.dev.pcb[0].id));
    port++;
    return port;
}

//static int pd_oper(struct _ty_usb_fd *id,va_list args)
//{   int i;
//    struct _ty_usb  *stream;
//    stream=id->dev;
//    ty_usb_out("�����ػ��߳�\n");
//    _lock_set(stream->lock);
//    _lock_un(stream->lock);
//    for(i=0;i<stream->data.dev.usb_port_n;i++)
//    {   _lock_set(stream->data.dev.port_lock[i]);
//        _lock_un(stream->data.dev.port_lock[i]);
//        
//    }
//    return 1;
//}

/*======================================��������====================================================*/
static int get_port_busid(struct _ty_usb  *stream,int port,char *busid)
{
	int i,n,len;
    // ty_usb_out("��ȡ��Ӧ�˿ڵ�busid\n");
    port--;

    for(i=0,n=0;i<stream->data.dev.n;i++,n+=stream->data.dev.pcb[i].n)
    {   if(port<n+stream->data.dev.pcb[i].n)
            break;
        continue;
    }
    memcpy(busid,stream->data.dev.busid,strlen(stream->data.dev.busid));
    len=strlen(busid);
    busid[len++]='.';
	//printf("stream->type = %d\n",stream->type);
	if((stream->type == 1) || (stream->type == 4)||(stream->type == 6))
	{
		busid[len++]='1'+i;
		busid[len++]='.';//��˰�̻���
	}
	else if(stream->type == 3)
	{
		if(port==0)
		{	sprintf(busid,"2-1.4");
		}
		else if(port==1)
		{	sprintf(busid,"2-1.3");
		}
		else if(port==2)
		{	sprintf(busid,"2-1.2");
		}
		else if(port==3)
		{	sprintf(busid,"2-1.1");
		}
		else if(port==4)
		{	sprintf(busid,"2-1.5");
		}
		else if(port==5)
		{	sprintf(busid,"2-1.6");
		}
		else if(port==6)
		{	sprintf(busid,"2-1.7.4");
		}
		else if(port==7)
		{	sprintf(busid,"2-1.7.3");
		}
		//ty_usb_out("busid=[%s]\n",busid);
		return 0;
	}
	else if(stream->type == 5)
	{
		if(port == 0)
		{	sprintf(busid,"1-1.1");
		}
		else if(port == 1)
		{	sprintf(busid,"1-1.2");
		}
		return 0;
	}
	else if (stream->type == 7)
	{
		
		if (port == 0)
		{
			sprintf(busid, "1-1.1");
		}
		else if (port == 1)
		{
			sprintf(busid, "1-1.2");
		}
		else if (port == 2)
		{
			sprintf(busid, "1-1.3");
		}
		else if (port == 3)
		{
			sprintf(busid, "1-1.4");
		}
		return 0;
	}
	else if (stream->type == 8)
	{
		memcpy(busid, busid_20[port], strlen(busid_20[port]));
		return 0;
	}
	else if (stream->type == 9)
	{
		memcpy(busid, busid_120[port], strlen(busid_120[port]));
		return 0;
	}
	else if (stream->type == 10)
	{
		memcpy(busid, busid_240[port], strlen(busid_240[port]));
		return 0;
	}
	else if (stream->type == 11)
	{
		memcpy(busid, busid_60[port], strlen(busid_60[port]));
		return 0;
	}
	else if (stream->type == 12)
	{
		memcpy(busid, busid_60_2306[port], strlen(busid_60_2306[port]));
		return 0;
	}
	//#if MB1601_OR_MB1602
	////ty_usb_out("��ΪMB1601����һ��hub\n");
 //   busid[len++]='1'+i;
 //   busid[len++]='.';//��˰�̻���
	//#endif
	if (stream->type == 4)
	{	//ty_usb_out("MB1603C(1910 2110)���Ͷ˿���HUB�����з�ʽ��ͬ\n");
		memcpy(busid, busid_100[port], strlen(busid_100[port]));
		return 0;
	}
    port-=n;

	//if(stream->type == 4)
	//{	//ty_usb_out("MB1603C���Ͷ˿���HUB�����з�ʽ��ͬ\n");
	//	memcpy(busid+len,usb_port_busid_a33[port],strlen(usb_port_busid_a33[port]));
	//	return 0;
	//}
    memcpy(busid+len,usb_port_busid[port],strlen(usb_port_busid[port]));
    //ty_usb_out("busid=[%s]\n",busid);
    return 0;
}

static int wait_usb_file_no(char *busid,int enum_timer)
{
	int i;
	char buisdd[20];
	sprintf(buisdd,"%s:1.0",busid);
    enum_timer=enum_timer/100;    
    for(i=0;i<enum_timer;i++,usleep(100000))
    {   if(_usb_file_ls_busid(buisdd)==0)
            return 0; 
    }
    ty_usb_out("�ڵ�λʱ���ڻ�δ�ȵ����豸��ʧ\n",enum_timer*100);
    return DF_ERR_PORT_CORE_TY_USB_CLOSE;  
}

static int get_usb_file(char *busid,int enum_timer,int mach_type)
{
	char main_hub[DF_TY_USB_BUSID];
    char pcb_hub[DF_TY_USB_BUSID];
    char port_hub[DF_TY_USB_BUSID];
	if (mach_type == 3 || mach_type == 5 || mach_type == 7 || mach_type == 8)
	{
		return get_busid_over_timer(busid,enum_timer);
	}


    memset(port_hub,0,sizeof(port_hub));
    memset(pcb_hub,0,sizeof(pcb_hub));
    memset(main_hub,0,sizeof(main_hub));
    memcpy(port_hub,busid,strlen(busid)-2);
    memcpy(pcb_hub,busid,strlen(port_hub)-2);
    memcpy(main_hub,busid,strlen(pcb_hub)-2);

	if(mach_type == 1)
	{
	//#if MB1601_OR_MB1602
		ty_usb_out("��Ϊmb1601������HUB\n");
		if(get_busid_over_timer(main_hub,enum_timer)<0)
		{   ty_usb_out("��λʱ����û���ҵ���HUBϵͳҪ��Ҫ��������\n");
			return DF_ERR_PORT_CORE_TY_USB_MAIN_HUB;
		}
	}
	//#endif
    if(get_busid_over_timer(pcb_hub,enum_timer)<0)
    {   ty_usb_out("û���ҵ������HUB\n");
        return DF_ERR_PORT_CORE_TY_USB_PCB_HUB;
    }
    if(get_busid_over_timer(port_hub,enum_timer)<0)
    {   ty_usb_out("û���ҵ��˿ڵ�HUB\n");
        return DF_ERR_PORT_CORE_TY_USB_PORT_NO_ENUM;
    }
    return get_busid_over_timer(busid,9000);
}

static int get_busid_over_timer(const char *busid,int enum_timer)
{
	struct _usb_file_dev    usb_file;
    int i;
    enum_timer=enum_timer/100;    
    for(i=0;i<enum_timer;i++,usleep(100000))
    {   memset(&usb_file,0,sizeof(usb_file));
        if(_usb_file_get_infor(busid,&usb_file)<0)
            continue;
        return 0;    
    }
    ty_usb_out("��[%d]ʱ����δ�ҵ��豸\n",enum_timer*100);
    return DF_ERR_PORT_CORE_TY_USB_NO_FIND_DEV;   
}

static int get_port(struct _ty_usb  *stream,int port)
{
	int sub_s,result,i;
	//printf("stream->port_n = %d\n",stream->port_n);
    if(stream->port_n==0)
        return port;
    sub_s=(port-1);
    result=(sub_s%35);
    i=sub_s-result;
    port=i+(35-result);
    return port;
}

int vid_pid_jude_plate(uint16 vid, uint16 pid)
{
	//ty_usb_m_out("vid %04x,pid %04x\n", vid, pid);
	if ((vid == 0x101d) && (pid == 0x0003))
	{
		return DEVICE_AISINO_LOCAL;//���п�����˰��ukey DEVICE_CNTAX
	} 
	else if ((vid == 0x1432) && (pid == 0x07dc))
	{
		return DEVICE_NISEC_LOCAL;
	}
	else if ((vid == 0x0680) && (pid == 0x1901))
	{	
		return DEVICE_MENGBAI_LOCAL;
	}
	else if ((vid == 0x0680) && (pid == 0x2008))
	{
		return DEVICE_MENGBAI2_LOCAL;
	}
	else if ((vid == 0x14d6) && (pid == 0xa002))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x3254) && (pid == 0xff10))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x287f) && (pid == 0x00d4))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x1677) && (pid == 0x6101))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x1ea8) && (pid == 0xb00b))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0xd449) && (pid == 0x0001))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x2012) && (pid == 0x2010))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x163c) && (pid == 0x5480))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x1432) && (pid == 0x07d0))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x2ec9) && (pid == 0x1001))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x101d) && (pid == 0x0005))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x3a59) && (pid == 0x4458))
	{
		return DEVICE_CNTAX_LOCAL;
	}
	else if ((vid == 0x1432) && (pid == 0x07e2))
	{
		return DEVICE_SKP_KEY_LOCAL;
	}
	else if ((vid == ROBOT_ARM_VID) && (pid == ROBOT_ARM_PID))
	{
		return DEVICE_ROBOT_ARM_LOCAL;
	}
	return 0;
}


