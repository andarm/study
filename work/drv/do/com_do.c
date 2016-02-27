#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME  "/dev/do_dev"

#define u16 unsigned short 
#define u8 unsigned char

#define SIZE  3

#define MAX_CHANEL  3

#define S3C_DO_NC			    8001
#define S3C_DO_NO			    8002
#define S3C_DO_PULSE_NC		8003
#define S3C_DO_PULSE_NO		8004


typedef struct
{
	__u32 cmd;
	__u32 ch;
	__u32 time_10ms;
} ST_S3C_DO_CFG, *LPST_S3C_DO_CFG;


ST_S3C_DO_CFG st_do_cfg[3];


void do_init(void)
{
    int i ;

    int fd;                                                                                                                                                                                          
    fd = open(DEVICE_NAME,O_RDWR);
    if(fd<0 )
    {
	printf("open:%s error\n",DEVICE_NAME);
    }




    for(i=0;i<MAX_CHANEL;i++)
    {
	st_do_cfg[i].ch =  i ;
	st_do_cfg[i].cmd = S3C_DO_NO; // normal Close
	st_do_cfg[i].time_10ms =  10 ;

	write(fd,(void *)&st_do_cfg[i],sizeof(ST_S3C_DO_CFG));   
    }

    
    close(fd);
    
}

void open_chanel(int ch)
{

   int fd; 
    fd = open(DEVICE_NAME,O_RDWR);
    if(fd<0 )
    {
	printf("open:%s error\n",DEVICE_NAME);
    }

    st_do_cfg[ch].cmd = S3C_DO_NO;


    write(fd,(void *)&st_do_cfg[ch],sizeof(ST_S3C_DO_CFG));

    close(fd);

}

void close_chanel(int ch)
{

   int fd; 
    fd = open(DEVICE_NAME,O_RDWR);
    if(fd<0 )
    {
	printf("open:%s error\n",DEVICE_NAME);
    }

    st_do_cfg[ch].cmd = S3C_DO_NC;


    write(fd,(void *)&st_do_cfg[ch],sizeof(ST_S3C_DO_CFG));

    close(fd);

}

	
int main(int argc, char * argv[])
{
    int i ; 
    static int flat = 0;
    do_init();	
    while(1)
    {
	printf("write now \n");
	close_chanel(1);

#if 0 
	if(flat)
	{
	    printf("S3C_DO_NC\n");
	    flat = 0 ;
	    for(i=0;i<MAX_CHANEL;i++)
	    {
		close_chanel(i);
	    }
	}
	else
	{
	    printf("S3C_DO_NO\n");
	    flat = 1;
	    for(i=0;i<MAX_CHANEL;i++)
	    {
		open_chanel(i);
	    }
	}
#endif 
	sleep(1);
    }
    return 0;
}
