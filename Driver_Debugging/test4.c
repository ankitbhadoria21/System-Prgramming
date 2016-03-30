#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>

#define ttl_thrds 2

char *device = "/dev/a5";

/*
Scanerio Considered in this flle:-
** Important Point to notice: Since the values for MODE can be changed by previous succesfull ioctl calls
** and MODE value is critical in acheiving the deadlock, so Desired Deadlock Behaviour can be acheived by reinsering the module.
1. Do an open in both the threads in mode1
2. then do an pthread_exit on thread1, since thread2 was waiting on sem2 which would be upped by thread1 in release operation
but as it is killed before it could release the semaphore, it would cause a deadlock.
*/

pthread_t threads[ttl_thrds];

void *func1(void *arg)
{
int fd = open(device, O_RDWR);
printf("func1 open\n");

sleep(4);

//Exit pthread1 before it could close
pthread_exit((void*)NULL);

close(fd);
printf("Exit func1\n");
}

void *func2(void *arg)
{
//Opening this device device will cause a deadlock

int fd = open(device, O_RDWR);
if(fd == -1) {
printf("Error in Open\n");
}
printf("func2 open\n");
sleep(10);

close(fd);
printf("Exit func2\n");
}

int main( ) 
{
pthread_create(&(threads[0]), NULL, func1, NULL);
sleep(5);
pthread_create(&(threads[1]), NULL, func2, NULL);
int i;
for (i=0; i<ttl_thrds; i++)
pthread_join(threads[i], NULL);

return 0;
}
