#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME  "/dev/di_dev"

#define u16 unsigned short 
#define SIZE  3
int main(int argc, char * argv[])
{
    int i, n, fd;
    int cmd,arg;
    char ad_val[10]={0};
	u16 tempareture= 0,humidity=0;
	u16 buf[SIZE]={0};
	
	printf("test for DI\n");
	
     while(1)
    {
        fd = open(DEVICE_NAME,O_RDWR);
        if (fd < 0)
        {
        printf("can't open \n");
        exit(1);
        }
		printf("reading now \n");
        if(read(fd,buf,sizeof(buf))<0)
        {
            perror("read. error..\n");
        }
		for(i=0;i<sizeof(buf)/2;i++)
		{
			printf("ad_val[%d]=%d\n",i,(__u16)buf[i]);
		}

        close(fd);
        sleep(1);
    }
    return 0;
}
