#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME  "/dev/am335x_test"

#define u16 unsigned short 
#define u8 unsigned char

#define SIZE  3

#define MAX_CHANEL  4

#define S3C_DO_NC	        8001
#define S3C_DO_NO		8002
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
	printf("test for DO\n");
	ST_S3C_DO_CFG st_do_cfg[3];
	
	static int flat = 0;

	for(i=0;i<MAX_CHANEL;i++)
	{
		st_do_cfg[i].ch =  i ;
		st_do_cfg[i].cmd = S3C_DO_NC;
		st_do_cfg[i].time_10ms =  10 ;
	}
	
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
			flat = 0 ;
			for(i=0;i<MAX_CHANEL;i++)
			{
				st_do_cfg[i].ch =  i ;
				st_do_cfg[i].cmd = S3C_DO_NC;
				st_do_cfg[i].time_10ms =  10 ;
				
				printf("S3C_DO_NC:%d\n",i);
				write(fd,(void *)&st_do_cfg[i],sizeof(ST_S3C_DO_CFG));
			}
				
		}
		else
		{
			flat = 1;
			for(i=0;i<MAX_CHANEL;i++)
			{
				st_do_cfg[i].ch =  i ;
				st_do_cfg[i].cmd = S3C_DO_NO;
				st_do_cfg[i].time_10ms =  10 ;
				
				printf("S3C_DO_NO:%d\n",i);
				write(fd,(void *)&st_do_cfg[i],sizeof(ST_S3C_DO_CFG));
			}
		}
        close(fd);
        sleep(1);
    }
    return 0;
}
