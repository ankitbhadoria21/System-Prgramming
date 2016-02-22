#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>

int main()
{
char write_buf[]="Calling IOCTL Write\n";
char read_buf[100];
ioctl(open("/dev/mydrv0",O_RDWR),MY_WRITE,write_buf);
ioctl(open("/dev/mydrv0",O_RDWR),MY_READ,read_buf);
printf("received Message: %s\n",read_buf);
return 0;
}
