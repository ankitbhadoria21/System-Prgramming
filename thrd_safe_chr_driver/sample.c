#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
int main() {
char buf[20]="reverse";
char buf1[20]="this is buffer";
int id=open("/dev/mycdrv2",O_RDWR);
//buf will receive the old direction of writing in the driver
ioctl(id,ASP_CHGACCDIR,buf);
write(id,buf1,14);
id=open("/dev/mycdrv0",O_RDWR);
write(id,buf,5);
close(id);
return 0;
}
