#include <iostream>
#include "SocketCAN.h"
void handle_sig(int num)
{
    exit(1);
}

int main(char argc,char**argv)
{

    CSocketCAN  sockcan; 
    signal(SIGINT,handle_sig);
    if(argc<1)
    {

    printf("please input can0 can1 can2 can3\n");
    return 1;
    }

    char can_name[5]={0}; 
    if(argv[1]!=NULL )
    {
	memcpy(can_name,argv[1],sizeof(can_name));
    }
    printf("input:%s\n",can_name);
    int ret = sockcan.Open(can_name);
    if(!ret)
    {
	printf("打开can0失败\n");
    }
    printf("open success\n");
    sockcan.Init(NULL);
    while(1)
    {
	sleep(1000);
    }

    return 0 ;
}
