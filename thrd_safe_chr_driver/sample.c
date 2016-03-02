#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
int main() {
ioctl(open("/dev/mycdrv0",O_RDWR),ASP_CHGACCDIR,"default");
return 0;
}
