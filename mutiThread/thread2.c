#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *thread_add(void *arg);
void *thread_sub(void *arg);

int main()
{
    int res;
    pthread_t a_thread,b_thread;
    void *thread_result;
    int num=0;
    res = pthread_create(&a_thread, NULL, thread_add,NULL);
    if (res != 0)
    {
        perror("Thread creation failed!");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&b_thread, NULL, thread_sub, NULL);
    if (res != 0)
    {
        perror("Thread creation failed!");
        exit(EXIT_FAILURE);
    }    
    res = pthread_join(a_thread, &thread_result);
    if (res != 0)
    {
        perror("Thread join failed!\n");
        exit(EXIT_FAILURE);
    }

    printf("Thread joined, it returned %s\n", (char *)thread_result);
    printf("Message is now %s\n", message);

    exit(EXIT_FAILURE);
}

void *thread_add(void *arg)
{
    int i=0,temp;
    for(;i<500;i++)
    {
        temp=num+1;
        num=temp;
        printf("add+1,result= %d\n",num);
    }
    return ((void*)0);
}
void *thread_sub(void *arg)
{
    int i=0,temp;
    for(;i<500;i++)
    {
        temp=num-1;
        num=temp;
        printf("add+1,result= %d\n",num);
    }
    return ((void*)0);
