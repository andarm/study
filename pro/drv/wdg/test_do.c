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


#define S3C_DO_NC			    8003
#define S3C_DO_NO			    8004
#define S3C_DO_PULSE_NC		8003
#define S3C_DO_PULSE_NO		8004


	typedef struct
	{
		__u32 cmd;
		__u32 ch;
		__u32 time_10ms;
	} ST_S3C_DO_CFG, *LPST_S3C_DO_CFG;
	
int main(int argc, char * argv[])
{
    int i, n, fd;
    int cmd,arg;
    char ad_val[10]={0};
	u16 tempareture= 0,humidity=0;
	u16 buf[SIZE]={0};
	u8 w_buf[12]={0};
	u8 w2_buf[12]={0};
	printf("test for DO\n");
	LPST_S3C_DO_CFG pt0 = &w_buf[0];
	
	LPST_S3C_DO_CFG pt1 = &w2_buf[0];
	
	static int flat = 0;
	
	pt0->cmd = S3C_DO_NC;
	pt0->ch = 0 ;
	pt0->time_10ms =10;
	
	pt1->cmd = S3C_DO_NC;
	pt1->ch = 1 ;
	pt1->time_10ms =10;
	
     while(1)
    {
        fd = open(DEVICE_NAME,O_RDWR);
        if (fd < 0)
        {
			printf("can't open \n");
			exit(1);
        }
		printf("write now \n");
		if(flat)
		{
			printf("S3C_DO_NC\n");
			flat = 0 ;
				pt0->cmd = S3C_DO_NC;
				pt0->ch = 0 ;
				pt0->time_10ms =10;

				pt1->cmd = S3C_DO_NC;
				pt1->ch = 1 ;
				pt1->time_10ms =10;
				
			if(write(fd,pt0,sizeof(ST_S3C_DO_CFG))<0)
			{
				perror("write. error..\n");
			}
			if(write(fd,pt1,sizeof(ST_S3C_DO_CFG))<0)
			{
				perror("write. error..\n");
			}
			
		}
		else
		{
			printf("S3C_DO_NO\n");
			pt0->cmd = S3C_DO_NO;
			pt0->ch = 0 ;
			pt0->time_10ms =10;

			pt1->cmd = S3C_DO_NO;
			pt1->ch = 1 ;
			pt1->time_10ms =10;
			
			flat =1  ;
			if(write(fd,pt0,sizeof(ST_S3C_DO_CFG))<0)
			{
				perror("write. error..\n");
			}
			if(write(fd,pt1,sizeof(ST_S3C_DO_CFG))<0)
			{
				perror("write. error..\n");
			}
		}
        close(fd);
        sleep(1);
    }
    return 0;
}
