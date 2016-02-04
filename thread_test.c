#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* fun(void* a)
{
    printf("the number is:%d\n",(int)a);
}

int main(void)
{
    pthread_t t;
    int no;
    void* rt;
    printf("input the number:");
    scanf("%d",&no);
    printf("input the:");
    pthread_create(&t,NULL,fun,(void*)no);
    pthread_join(t,&rt);
    return 0;
}

