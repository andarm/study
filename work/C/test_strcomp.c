#include <stdio.h> 
#include <string.h> 

/*********************************************************************
 * 描述： 
 *      strstr(p1,p2) 返回值 当p1 中查找不到p2 内容时，返回NULL。 
 *      如果能够查找到则就会从查找到的第一位置显示。 
 *      如； p1 = 123 ，p2= 3 输出： 3  
 * 
 * 
 * ******************************************************************/

int main()
{
    char *p="123";
    char *pt="4";
    char  *ret =  strstr(p,pt);
    printf("ret=%s\n",ret);
}