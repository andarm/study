#include <stdio.h> 
#include <stdlib.h> 
#include <sys/time.h> 

#define DEV_DEVICE "/dev/am335x_key" 

int get_key_value(void)
{
	int ret; 
	unsigned short  data; 

	ret=open(DEV_DEVICE,0);
	if(ret<0) 
	{ 
		printf("open error the ret is : %d", ret); 
		return ret; 
	} 
	read(ret,&data,sizeof(data)); 
	close(ret); 
	return data;

}

int main() 
{ 
	int val;
	while(1)
	{

		val = get_key_value();
		printf("val:%d\n", val);
	}
	return 0; 
}
