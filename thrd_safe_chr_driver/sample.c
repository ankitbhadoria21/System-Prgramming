#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
int main() {
char buf[20]="reverse";
int id=open("/dev/mycdrv0",O_RDWR);
ioctl(id,ASP_CHGACCDIR,buf);
printf("%s\n",buf);
//lseek(id,5000,SEEK_CUR);
return 0;
}
