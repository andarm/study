#include <stdio.h>

int main()
{
    int i = 123456;
    char s[10];
    sprintf(s,"%d",i);

    printf("%s\n",s);

    return 0;
}
