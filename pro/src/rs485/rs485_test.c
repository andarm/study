#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <getopt.h>
#include <string.h>
#define FALSE 1
#define TRUE 0

char *recchr="We received:\"";

void print_usage();

void ASCII2Hex(char *pcFrom, char *pcTo, char cLen);
int strToHex(char *ch,char *hex,int *len);
char valueToHexCh(const int value);
int speed_arr[] = { 
	B921600, B460800, B230400, B115200, B57600, B38400, B19200, 
	B9600, B4800, B2400, B1200, B300, 
};

int name_arr[] = {
	921600, 460800, 230400, 115200, 57600,38400,  19200,  
	9600,  4800,  2400,  1200,  300,  
};

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);

	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i])	{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0)
				perror("tcsetattr fd1");
				return;
		}
		tcflush(fd,TCIOFLUSH);
  	 }

	if (i == 12){
		printf("\tSorry, please set the correct baud rate!\n\n");
		print_usage(stderr, 1);
	}
}
/*
	*@brief   设cnet置串口数据位，停止位和效验位
	*@param  fd     类型  int  打开的串口文件句柄*
	*@param  databits 类型  int 数据位   取值 为 7 或者8*
	*@param  stopbits 类型  int 停止位   取值为 1 或者2*
	*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE ;
	switch (databits) /*设置数据位数*/ {
	case 7:
		options.c_cflag |= CS7;
	break;
	case 8:
		options.c_cflag |= CS8;
	break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}
	
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
	break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
		options.c_iflag |= INPCK;             /* Disnable parity checking */
	break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* 转换为偶效验*/ 
		options.c_iflag |= INPCK;       /* Disnable parity checking */
	break;
	case 'S':	
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
	break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
 	/* 设置停止位*/  
  	switch (stopbits) {
   	case 1:
    	options.c_cflag &= ~CSTOPB;
  	break;
 	case 2:
  		options.c_cflag |= CSTOPB;
  	break;
 	default:
  		fprintf(stderr,"Unsupported stop bits\n");
  		return (FALSE);
 	}
  	/* Set input parity option */
  	if (parity != 'n')
	{
    		options.c_iflag |= INPCK;
  	}
	options.c_cc[VTIME] =0; // 15 seconds
    	options.c_cc[VMIN] = 0;

	//options.c_lflag &= ~(ECHO | ICANON);
     
   //  options.c_lflag &= ~(ECHO | ICANON | ISIG);
   //  options.c_oflag &= ~OPOST;
 //    options.c_iflag &= ~(ICRNL | IGNCR);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
      	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        options.c_oflag &= ~(ONLCR | OCRNL);

  	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
  	if (tcsetattr(fd,TCSANOW,&options) != 0) {
    	perror("SetupSerial 3");
  		return (FALSE);
 	}
	return (TRUE);
}

/**
	*@breif 打开串口
*/
int OpenDev(char *Dev)
{
	int fd = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
 	if (-1 == fd) { /*设置数据位数*/
   		perror("Can't Open Serial Port");
   		return -1;
	} else
		return fd;
}


/* The name of this program */
const char * program_name;

/* Prints usage information for this program to STREAM (typically
 * stdout or stderr), and exit the program with EXIT_CODE. Does not
 * return.
 */

void print_usage (FILE *stream, int exit_code)
{
    fprintf(stream, "Usage: %s option [ dev... ] \n", program_name);
    fprintf(stream,
            "\t-h  --help     Display this usage information.\n"
            "\t-d  --device   The device ttyS[0-3] or ttySCMA[0-1]\n"
	    "\t-b  --baudrate Set the baud rate you can select\n" 
	    "\t               [230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300]\n"
            "\t-s  --string   Write the device data\n");
    exit(exit_code);
}

int open_port(int fd , char port[],int baud,int stopbits,int parity)
{
	int i ;
	int val_baud;
	fd = open(port, O_RDWR); //djf  delete the value  |O_NONBLOCK 

	if (fd<0) {
		printf("open serialport failed!\n");
		return -1;
	} 
	for(i=0;i<sizeof(speed_arr)/sizeof(int);i++)
	{
				
		if(baud == name_arr[i])
		{
			val_baud = i ;
			printf("baud is;%d\n",name_arr[i]);
			break;
		}
	}

	struct termios tio; 

	tcgetattr(fd, &tio);
	bzero(&tio, sizeof(tio));
	// these must be set first, don't know why
	tio.c_cflag |= CLOCAL;
	tio.c_cflag |= CREAD;
	tio.c_cflag &= ~CSIZE;
	// baud rate 
	if (cfsetispeed(&tio, speed_arr[val_baud]) != 0)    {
		printf("cfsetispeed failed\n");
		return -1;
	}
	switch(parity)
	{
	case 'n':
	case 'N':
		// parity NONE
		tio.c_cflag &= ~PARENB;
        	tio.c_iflag &= ~INPCK;
        	tio.c_iflag |= IGNBRK;
		break;
	case 'o':
	case 'O':
		tio.c_cflag |= (PARODD | PARENB);
        	tio.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
        	tio.c_iflag |= INPCK;
		break;
	default:
		printf("parity error.....\n");
		break;

	}
	// data bit 8
	tio.c_cflag |= CS8;
	switch(stopbits)
	{
	// stop bit 1
	case 1:
		tio.c_cflag &= ~CSTOPB;
		break;
	case 2:	
		tio.c_cflag |= CSTOPB;
		break;
	default:
		printf("stopbits error\n");
		break;
	
	}// others
	tio.c_cc[VTIME] = 0;
	tio.c_cc[VMIN]  = 0;
	// flush settings
	if(tcflush(fd, TCIOFLUSH) != 0) //djf chage TCIFLUSH  to TCIOFLUSH 20150408
	{
		printf("open serialport: Flushing %s ERROR!\n\n", port);
		return -1;
	} 

	if(tcsetattr(fd, TCSANOW, &tio) != 0)
	{
	
		printf("open serialport: Setting %s ERROR!\n\n", port);
		return -1;
	}
	return fd;
	return 0;
}



/*
	*@breif  main()
 */
int main(int argc, char *argv[])
{
	int  fd, next_option, havearg = 0;
	char *device;
	int i=0,j=0;
	int nread;			/* Read the counts of data */
	char buff[512];		/* Recvice data buffer */
	pid_t pid;
//	char xmit[] = {0xDC,0x03,0x01,0x01,0x00,0x01,0xC6,0xBB}; /* Default send data */ 
	char xmit[] = {0x01,0x03,0x9c,0x42,0x00,0x01,0x0A,0x4E}; /* Default send data */ 
	int speed ;
	const char *const short_options = "hd:s:b:";

	const struct option long_options[] = {
		{ "help",   0, NULL, 'h'},
		{ "device", 1, NULL, 'd'},
		{ "string", 1, NULL, 's'},
		{ "baudrate", 1, NULL, 'b'},
		{ NULL,     0, NULL, 0  }
	};
	
	program_name = argv[0];
#if 0 
	do {
		next_option = getopt_long (argc, argv, short_options, long_options, NULL);
		switch (next_option) {
			case 'h':
				print_usage (stdout, 0);
			case 'd':
				device = optarg;
				havearg = 1;
				break;
			case 'b':
				speed = atoi(optarg);
				break;

			case 's':
				*xmit = optarg;
				havearg = 1;
				break;
			case -1:
				if (havearg)  break;
			case '?':
				print_usage (stderr, 1);
			default:
				abort ();
		}
	}while(next_option != -1);
//	sleep(1);
	fd = OpenDev(device);

	if (fd > 0) {
		set_speed(fd, speed);
	} else {
		fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
		exit(1);
	}

	if (set_Parity(fd,8,1,'O')== FALSE)
	{
		fprintf(stderr, "Set Parity Error\n");
		close(fd);
		exit(1);
	}
#endif
	printf("argc=%d\n",argc);
	if(argc<3)
	{
		printf("*****************************************************\n");
		printf("***please input test mode: argv1=/dev/ttyO2 **********\n");
		printf("***argv2=9600****************************************\n");
		printf("***argv3=data (12345678)***********************************\n");
		printf("***argv4=0,1,3stopbit*******************************\n");
		printf("***argv5=n  parity('n','o')************************\n");
		printf("*****************************************************\n");
		return -1;
	}
	fd = 0 ;
/***********************************************************
*input hanle
*argv1=dev
*argv2=9600
*argv3=data
*argv4=stopbit
*argv5=parity
*
*
*************************************************************/
	int temp_baud=atoi(argv[2]);
	char stopbit=1,parity='n';
	if(argv[4])
	{
		printf("setting stopbit:argv[4]=%d\n",argv[4]);
	stopbit=atoi(argv[4]);
	}
	if(argv[5])
	{
		printf("seting parity\n");
//		parity = argv[5];
	}
	printf("stopbit=%d,parity=%c\n",stopbit,parity);
	// $1=fd $2=dev $3=baud $4=stopbits $5=parity 
	fd = open_port(fd,argv[1],temp_baud,stopbit,parity);
	printf("show UART:%s\n",argv[0]);
	
	pid = fork();	

	if (pid < 0) 
	{ 
		fprintf(stderr, "Error in fork!\n"); 
	} 
	else if (pid == 0)
	{//father fork
	#if 1 	
		while(1)
		{
			printf("%s SEND: %d\n",argv[1],strlen(argv[3]));
		int len=0;
			
			strToHex(argv[3],buff,&len);
			//printf("buff size =%d\n",len);
			int ret;
			ret = write(fd,buff,len);
			printf("ret:%d\n",ret);
			//write(fd,argv[3],strlen(argv[3]));
			sleep(1);
			i++;
		}
		exit(0);
	#endif 

	}
	else
	{
	
		while(1) 
		{
			nread = read(fd, buff, sizeof(buff));
			if (nread > 0) {
				buff[nread] = '\0';
				printf("%s RECV %d total\n", device, nread);
				printf("%s RECV: %s\n", device, buff);
				printf("receiving......\n");
#if 1 
				for(j=0;j<nread;j++)
				{
//					printf("j=%d\n",j);
					printf("%02X  ",buff[j]);
				}
				printf("\n");
#endif 
			}
			usleep(50000);
		}	
	}
	close(fd);
	exit(0);
}
void ASCII2Hex(char *pcFrom, char *pcTo, char  cLen)
{
	

	char i;



	for(i = 0; i < cLen; i ++)

	{

	if((*pcFrom >= '0') && (*pcFrom <= '9'))

	{

	*pcTo =  (*pcFrom-'0');

	}

	else if((*pcFrom >= 'a') && (*pcFrom <= 'f'))

	{

	*pcTo =  (*pcFrom-'a') + 10;

	}

	else if((*pcFrom >= 'A') && (*pcFrom <= 'F'))

	{

	*pcTo =  (*pcFrom-'A') + 10;

	}


	pcFrom ++;

	pcTo ++;

	}

	
}

int strToHex(char *ch, char *hex,int *len)
{
	char high,low;
	char tmp = 0;
	printf("input data is:%s\n",ch);
	if(ch == NULL || hex == NULL){
		return -1;
	}

//	if(strlen(ch) == 0){
//		return -2;
//	}

	while(*ch){
		high = *ch++;
		low = *ch;
		#if 1 
		if(low>='0' && low<='9')
		{
			low = low-'0';
		}
		if(low>='A' && low<='F')
		{
			low = low-'A'+10;
		}
		if(low>='a' && low<='f')
		{
			low = low-87;
		}
		if(high>='0' && high<='9')
		{
			high = high-'0';
		}
		if(high>='A' && high<='F')
		{
			high = high-'A'+10;
		}
		if(high>='a' && high<='f')
		{
			high = high-87;
		}
		#endif
			
//		printf("h=%x,L=%x\n",high,low);
		tmp = (high<<4);
		tmp |= low;
		
		*hex++=tmp;
		*ch++;
		*len+=1;
		//printf("temp:%x\n",*hex);
	}
	*hex = '\0';
//	printf("hex size:%d\n",strlen(hex));
	return 0;
}
char valueToHexCh(const int value)
{
	char result = '\0';
	if(value >= 0 && value <= 9){
		result = (char)(value + 48); //48为ascii编码的‘0’字符编码值
	}
	else if(value >= 10 && value <= 15){
		result = (char)(value - 10 + 65); //减去10则找出其在16进制的偏移量，65为ascii的'A'的字符编码值
	}
	else{
		;
	}

	return result;
}
