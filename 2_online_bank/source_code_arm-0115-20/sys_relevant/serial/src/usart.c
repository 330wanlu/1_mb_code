#include "../inc/usart.h"

static int _usart_open(struct _seria	*data,void *o_arg);
static void _usart_close(void *bsp_data);
static int _usart_write(void *bsp_data,uint8 *buf,int buf_len);
static void _usart_hook(void *arg,int o_timer);
static  int _usart_set_bps(void *bsp_data,int bps);
int SetDevice(int fd, int baud,int databits,int stopbits,int parity);

static const struct _seria_bsp_fuc	us_fuc =
{
	.open=_usart_open,
	.close=_usart_close,
	.write=_usart_write,
	.bps=_usart_set_bps
};

void _linux_usart_load(int hard_type)
{
	if(hard_type == 2)
	{
		seria_add("seria0",&us_fuc,"/dev/ttyS1");
	}
	else if((hard_type == 1) || (hard_type == 3))
	{
		printf("hard_type = %d\n",hard_type);
		seria_add("seria0",&us_fuc,"/dev/ttyS1");
		seria_add("seria1",&us_fuc,"/dev/ttyS2");
	}
	else
	{
		seria_add("seria0",&us_fuc,"/dev/ttyS4");
		seria_add("seria1",&us_fuc,"/dev/ttyS3");
	}
}

static int _usart_open(struct _seria *data,void *o_arg)
{
	struct _usart *stream = NULL;
	int result = 0;
	stream = (struct _usart *)malloc(sizeof(struct _usart));
	if(stream == NULL)
		return DF_ERR_MEM_ERR;
	memset(stream,0,sizeof(struct _usart));
	usart_out("_usart open start\n");

	printf("_usart_open name = %s\n",o_arg);

	stream->fd = open(o_arg,O_RDWR|O_NOCTTY|O_NDELAY);
	printf("_usart_open stream->fd = %d\n",stream->fd);
	if(stream->fd<0)
	{
		usart_out("linux usart open err=%d\n",stream->fd);
		result = stream->fd;
		free(stream);
		return result;
	}
	if((result=SetDevice(stream->fd,9600,8,1,'n')) < 0)
	{
		usart_out("linux usart open bps err=%d\n",result);
		close(stream->fd);
		free(stream);
		return result;
	}
	usart_out("creat task\n");
	stream->task = _scheduled_task_open((char *)o_arg,_usart_hook,stream,1,NULL);
	if(stream->task < 0)
	{
		result = stream->task;
		usart_out("linux usart open task err=%d\n",stream->task);
		close(stream->fd);
		free(stream);
		return result;
	}
	stream->seria = data;
	stream->write_data = data->write;
	data->bsp_data = stream;
	usart_out("linux usart open suc \n");
	return 0;
}

static int _usart_set_bps(void *bsp_data,int bps)
{
	struct _usart *stream = NULL;
	int result = 0;
	stream = bsp_data;
	usart_out("_usart_set_bps bps=%d\n",bps);
	if((result=SetDevice(stream->fd,bps,8,1,'n')) < 0)
	{
		usart_out("_usart_set_bps  err=%d\n",result);
		return result;
	}
	usart_out("_usart_set_bps suc\n");
	return 0;
}

static void _usart_close(void *bsp_data)
{
	struct _usart *stream = NULL;
	stream = bsp_data;
	usart_out("_usart_close in \n");
	_scheduled_task_close(stream->task);
	close(stream->fd);
	free(stream);
	usart_out("_usart_close suc\n");
	return ;
}

static int _usart_write(void *bsp_data,uint8 *buf,int buf_len)
{
	struct _usart *stream = NULL;
	int result = 0,i = 0;
	stream = bsp_data;
	// usart_out("_usart_write buf_len=%d\n",buf_len);
	// printf("send data:\n");
	// for(i = 0;i<buf_len;++i)
	// {
	// 	printf("%02x ",buf[i]);
	// }
	// printf("\n");
	result=write(stream->fd,buf,buf_len);
	if(result<0)
		return result;
	return buf_len;
}

static void _usart_hook(void *arg,int o_timer)
{
	uint8 buf[512];
	struct _usart *stream = NULL;
	int len = 0, i = 0;
	stream = arg;
	//usart_out("_usart_hook oper\n");
	for(;stream->write_data == NULL;usleep(1000));
	//usart_out("_usart_hook in\n");
	if(stream->aa!=0xaa)
	{
		stream->aa=0xaa;
	}
	for(;;)
	{
		len = read(stream->fd,buf,sizeof(buf));
		if(len>0)
		{
			// printf("_usart_hook stream->fd = %d\n",stream->fd);
			// printf("uart recv data:\n");
			// for(i = 0;i<len;++i)
			// {
			// 	printf("%02x ",buf[i]);
			// }
			// printf("\n");
			stream->write_data(stream->seria,buf,len);
			continue;
		}
		if(len < 0)
		{	
			perror("_usart_hook len < 0\n");
		}
		break;
	}
}

int SetDevice(int fd, int baud,int databits,int stopbits,int parity)
{
 	int baudrate;
 	struct termios newtio;   
 	switch(baud)
 	{
 		case 300:
  			baudrate=B300;
  			break;
 		case 600:
  			baudrate=B600;
  			break;
 		case 1200:
  			baudrate=B1200;
  			break;
 		case 2400:
  			baudrate=B2400;
  			break;
 		case 4800:
  			baudrate=B4800;
  			break;
 		case 9600:
  			baudrate=B9600;
  			break;
 		case 19200:
  			baudrate=B19200;
  			break;
 		case 38400:
  			baudrate=B38400;
  			break;
		case 57600:
  			baudrate=B57600;
  			break;
		case 115200:
		 	baudrate=B115200;
  			break;
		case 230400:
		 	baudrate=B230400;
  			break;
 		default :
  			baudrate=B230400;  
  			break;
 	}
 	tcgetattr(fd,&newtio);     
 	bzero(&newtio,sizeof(newtio));   
   //setting   c_cflag 
 	newtio.c_cflag  &=~CSIZE;     
 	switch (databits) /*??????*/
 	{   
 		case 7:  
  			newtio.c_cflag |= CS7; //7????
  			break;
 		case 8:     
  			newtio.c_cflag |= CS8; //8????
  			break;   
 		default:    
  			newtio.c_cflag |= CS8;
  		break;    
 	}
 	switch (parity) //????
 	{   
 		case 'n':
 		case 'N':    
  			newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
  			newtio.c_iflag &= ~INPCK;     /* Enable parity checking */ 
  			break;  
 		case 'o':   
 		case 'O':     
  			newtio.c_cflag |= (PARODD | PARENB); /* ??????*/  
  			newtio.c_iflag |= INPCK;             /* Disnable parity checking */ 
  			break;  
 		case 'e':  
 		case 'E':   
  			newtio.c_cflag |= PARENB;     /* Enable parity */    
  			newtio.c_cflag &= ~PARODD;   /* ??????*/     
  			newtio.c_iflag |= INPCK;       /* Disnable parity checking */
  			break;
 		case 'S': 
 		case 's':  /*as no parity*/   
     		newtio.c_cflag &= ~PARENB;
  			newtio.c_cflag &= ~CSTOPB;
			break;  
 		default:   
  			newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
  			newtio.c_iflag &= ~INPCK;     /* Enable parity checking */ 
  			break;   
 	} 
 	switch (stopbits)//?????
 	{   
 		case 1:    
  			newtio.c_cflag &= ~CSTOPB;  //1
  			break;  
 		case 2:    
  			newtio.c_cflag |= CSTOPB;  //2
    		break;
 		default:  
  			newtio.c_cflag &= ~CSTOPB;  
  			break;  
 	} 
 	newtio.c_cc[VTIME] = 0;    
 	newtio.c_cc[VMIN] = 0; 
 	newtio.c_cflag |= (CLOCAL|CREAD);
 	newtio.c_oflag &= ~OPOST; 
 	newtio.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON); 
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);                     
    cfsetispeed(&newtio,baudrate);  
	cfsetospeed(&newtio,baudrate);    
    tcflush(fd, TCIOFLUSH);  //???????? 
 	if (tcsetattr(fd,TCSANOW,&newtio) != 0)   
 	{ 
  		perror("com tcsetattr fail");  
  		return -1;  
 	}  
 	return 0;
}
