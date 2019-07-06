#include <stdio.h> 
#include <stdlib.h> 
#include <sys/time.h> 
#define ADC_DEVICE "/dev/adc" 
int main() 
{ 
	int ret; 
	unsigned int data; 
	printf("script begin!steven!\n");

	ret=open(ADC_DEVICE,0);
	if(ret<0) 
	{ 
		printf("Open adc fail\n");
		printf("the ret is : %d", ret); 
		return ret; 
	} 
	for(;;) 
	{ 
		read(ret,&data,sizeof(data)); 
		printf("show data: %d\n",data);
		sleep(1);

	} 
	printf("open succeed!steven!\n");
	close(ret); 
	return 0; 
}
