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
==>. Do an open in MODE2 in both thread1 and thread2 which increment count2
==>. Do pthread_exit in thread1 which will kill it before it could decrement count2,
thread2 will keep on waiting as count2 is greater than 1
*/

pthread_t threads[ttl_thrds];

void *func1(void *arg)
{
int fd = open(device, O_RDWR);
printf("func1 open\n");

sleep(5);

//Exit pthread1 before it could close
pthread_exit(NULL);

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
sleep(15);

int ret = ioctl(fd, E2_IOCMODE1, 1);
if(ret == -1) {
printf("Error in IOCTL\n");
}
close(fd);
printf("Exit func2\n");
}

int main() 
{
int fd = open(device, O_RDWR);
if(fd == -1) {
printf("Error in Open\n");
}

int ret = ioctl(fd, E2_IOCMODE2, 2);
if (ret == -1) {
printf("Error in IOCTL\n");
}

close(fd);
pthread_create(&(threads[0]), NULL, func1, NULL);
sleep(5);
pthread_create(&(threads[1]), NULL, func2, NULL);
int i;
for (i=0; i<ttl_thrds; i++)
pthread_join(threads[i], NULL);

return 0;
}
