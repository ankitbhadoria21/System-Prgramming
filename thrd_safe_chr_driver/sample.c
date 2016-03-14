#include<stdio.h>
#include"head.h"
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
#define MAX 100

int main() {
int i;
char dev_name[MAX],buf[50*MAX],read_buf[50*MAX];
char dir[MAX];
int id[MAX];
int ind=0,opt,opr,direction,whence,to_read;
long long off;
while(1) {
printf("Enter the name of the device to open or type \"skip\" to skip this step or \"quit\" to quit\n");
scanf("%s",dev_name);
if(!strcmp(dev_name,"quit")) break;
if(strcmp(dev_name,"skip")) {
printf("opening %s\n",dev_name);
id[ind++]=open(dev_name,O_RDWR);
if(id[ind-1]==-1) {
printf("device not opened\n");
ind--;
continue;
}
}
printf("Enter Device number to use, devices till now have number from 0 to %d\n",ind-1);
if(ind==0) continue;
scanf("%d",&opt);
printf("0: Do lseek\n1: Do write\n2: Do Read\n3: Reverse Direction\n");
scanf("%d",&opr);
switch(opr) {
case 0:
printf("Enter offset to lseek to\n");
scanf("%lld",&off);
printf("Enter the location from which lseek needs to be done.\n0:SEEK_SET\n1:SEEK_CUR\n2:SEEK_END\n");
scanf("%d",&whence);
lseek(id[opt],off,whence);
break;
case 1:
printf("Input Data to write\n");
scanf("%s",buf);
write(id[opt],buf,sizeof(buf));
break;
case 2:
printf("Enter no of bytes to read\n");
scanf("%d",&to_read);
read(id[opt],read_buf,to_read);
printf("Data Read is %s\n", read_buf);
break;
case 3:
printf("Input\n0:for reverse direction\n1:for regular direction\n");
scanf("%d",&direction);
if(direction==0) {
strcpy(dir,"reverse");
ioctl(id[opt],ASP_CHGACCDIR,dir);
printf("old direction is %s\n",dir);
}
else {
strcpy(dir,"regular");
ioctl(id[opt],ASP_CHGACCDIR,dir);
printf("old direction is %s\n",dir);
}
break;
}

}
for(i=0;i<ind;++i)
close(id[ind]);
return 0;
}
