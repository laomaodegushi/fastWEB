#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


void print_msg(char *ptr);


int main()
{
pthread_t thread1, thread2;
int i,j;
char *msg1="do sth1\n";
char *msg2="do sth2\n";
pthread_create(&thread1,NULL, (void *)(&print_msg), (void *)msg1);
pthread_create(&thread2,NULL, (void *)(&print_msg), (void *)msg2);
sleep(1);
return 0;
}

void  print_msg(char *ptr)
{
int retval;
int id=pthread_self();
printf("Thread ID: %x\n",id);
  printf("%s",ptr);
pthread_exit(&retval);
}
