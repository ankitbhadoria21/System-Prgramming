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
open("/dev/mycdrv2",O_RDWR);
/*printf("%s\n",buf);
lseek(id,5000,SEEK_SET);
write(id,buf1,10);
memset(buf1,0,sizeof(buf1));
strcpy(buf,"regular");
ioctl(id,ASP_CHGACCDIR,buf);
read(id,buf1,10);
printf("%s\n",buf1);*/
return 0;
}
