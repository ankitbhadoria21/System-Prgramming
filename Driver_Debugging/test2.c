#include <stdio.h>
#include "head.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define ttl_thrds 2

/*
** Important Point to notice: Since the values for MODE can be changed by previous succesfull ioctl calls
** and MODE value is critical in acheiving the deadlock, so Desired Deadlock Behaviour can be acheived by reinsering the module.
==>. Do an open to a device on thread1 and then on thread do both with MODE1
==>. now do an ioctl to change mode to MODE2 in thread1 system will hang
*/

char *device = "/dev/a5";

pthread_t threads[ttl_thrds];

void *func1(void *arg)
{
int fd = open(device, O_RDWR);
printf("func1 open\n");

sleep(6);

int ret = ioctl(fd, E2_IOCMODE2, 2);
if (ret == -1) {
printf("Error in ioctl in func1\n");
}
close(fd);
printf("Exit func2\n");
}

void *func2(void *arg)
{
//Opening this device device will cause a deadlock

int fd = open(device, O_RDWR);
if(fd == -1) {
printf("Error in Open\n");
}
printf("func2 open\n");
close(fd);
printf("Exit fun2\n");
}

int main()
{
pthread_create(&(threads[0]), NULL, func1, NULL);
sleep(5);
pthread_create(&(threads[1]), NULL, func2, NULL);
int i;
for (i=0; i<ttl_thrds; i++)
pthread_join(threads[i], NULL);

return 0;
}
