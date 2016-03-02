#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
int main() {
char buf[20]="reverse";
char buf1[20]="this is buffer";
int id=open("/dev/mycdrv0",O_RDWR);
//buf will receive the old direction of writing in the driver
ioctl(id,ASP_CHGACCDIR,buf);
lseek(id,5000,SEEK_SET);
write(id,buf1,14);
return 0;
}
