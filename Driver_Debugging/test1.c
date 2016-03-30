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
==>. Do an open in both the threads in MODE2, this will Increment count2
==>. Now do ioctl in both this will cause system hang because both will wait for a condition
where count2 is greater than since no process can exit this deadlock it will cause system hang
*/

pthread_t threads[ttl_thrds];

void *func1(void *arg)
{
int fd = open(device, O_RDWR);
printf("func1 open\n");

sleep(10);

int ret = ioctl(fd, E2_IOCMODE1, 1);
if (ret == -1) {
printf("Error in ioctl in func1\n");
}
close(fd);
printf("Exiting Func1\n");
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

int ret = ioctl(fd, E2_IOCMODE1, 1);
if (ret == -1) {
printf("Error in IOCTL\n");
}
close(fd);
printf("Exiting func2\n");
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
