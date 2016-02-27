#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME  "/dev/ai_dev"

int main(int argc, char * argv[])
{
    int i, n, fd;
    int cmd,arg;
    int ret; 
    unsigned short  ad_val[10];
    while(1)
    {
        fd = open(DEVICE_NAME,O_RDWR);
        if (fd < 0)
        {
        printf("can't open \n");
        exit(1);
        }
        ret = read(fd,ad_val,6) ;


        if(ret <0)
        {
            perror("read...\n");
        }
	for(i=0;i<ret/2;i++)
	{
	    printf("ch =%d Val=%d V\n",i,ad_val[i]);
	}
//        printf("ad0 =%0.2f V,ad1=%0.2f V\n",(float)ad_val[0]/10000,(float)ad_val[1]/10000);

        close(fd);
        sleep(1);
    }
    return 0;
}
