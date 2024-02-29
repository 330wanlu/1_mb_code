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
 	switch (databits) /*设置数据位数*/
 	{   
 		case 7:  
  			newtio.c_cflag |= CS7; //7位数据位
  			break;
 		case 8:     
  			newtio.c_cflag |= CS8; //8位数据位
  			break;   
 		default:    
  			newtio.c_cflag |= CS8;
  		break;    
 	}
 	switch (parity) //设置校验
 	{   
 		case 'n':
 		case 'N':    
  			newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
  			newtio.c_iflag &= ~INPCK;     /* Enable parity checking */ 
  			break;  
 		case 'o':   
 		case 'O':     
  			newtio.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
  			newtio.c_iflag |= INPCK;             /* Disnable parity checking */ 
  			break;  
 		case 'e':  
 		case 'E':   
  			newtio.c_cflag |= PARENB;     /* Enable parity */    
  			newtio.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
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
 	switch (stopbits)//设置停止位
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
    tcflush(fd, TCIOFLUSH);  //接收发送全部清空 
 	if (tcsetattr(fd,TCSANOW,&newtio) != 0)   
 	{ 
  		perror("com tcsetattr fail");  
  		return -1;  
 	}  
 	return 0;
}
