#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "head.h"
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#define MAX_LENGTH SIZE
#define SIZE 16*PAGE_SIZE

struct asp_mycdrv {
struct list_head list;
struct cdev cd;
char *ramdisk;
struct semaphore sem;
int devNo;
};

static struct asp_mycdrv cdrv;
static int drv_num=-1;
static char *temp=NULL;
static ssize_t read_my(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *,const char *, size_t, loff_t *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
static loff_t my_llseek(struct file *filp, loff_t off, int whence);
struct file_operations fops={.unlocked_ioctl=my_ioctl,.read=read_my,.llseek=my_llseek,.write=my_write};
struct class *cl;
static int noOfDevices=3;
static int direction=0;
static int offset=0;

void strrev(char *str) {
int start=0,end=strlen(str);
char tmp;
while(start<end) {
tmp=str[start];
str[start]=str[end];
str[end]=tmp;
start++;
end--;
}

}

//noOfDevice can be passed as an argument while loading the module
module_param(noOfDevices,int,S_IRUSR | S_IWUSR);

//initializing function module_init not required with this function
// had it been a different function module_init would be required
int init_module(void)
{
	int i;
	sema_init(&(cdrv.sem),1);
	drv_num=register_chrdev(0,"Sample",&fops);
	cdrv.devNo=drv_num;
	if(drv_num!=-1){
	cdrv.ramdisk=kmalloc(SIZE,GFP_KERNEL);
	if(!cdrv.ramdisk) {
	printk(KERN_DEBUG"Memory Allocation Failed for Ramdisk\n");
	return -1;
	}
	temp=kmalloc(SIZE,GFP_KERNEL);
	if(!temp) {
	printk(KERN_DEBUG"Memory Allocation Failed for temp\n");
	return -1;
	}
	cl=class_create(THIS_MODULE,"chrdrv");
	register_chrdev_region(drv_num,noOfDevices,"Sample");
	for(i=0;i<noOfDevices;++i)
	device_create(cl,NULL,MKDEV(drv_num,i),NULL,"mycdrv%i",i);
	cdev_init(&(cdrv.cd),&fops);
	for(i=0;i<noOfDevices;++i)
	cdev_add(&(cdrv.cd),MKDEV(drv_num,i),1);
	}
	else printk(KERN_DEBUG"Major Number Not Assigned\n");
	printk(KERN_DEBUG"Done Intializing\n");
    	return 0;
}

//Cleanup Function module_exit not needed for this functiona
void cleanup_module(void)
{
	int i;
	kfree(cdrv.ramdisk);
	kfree(temp);
	cdev_del(&(cdrv.cd));
	for(i=0;i<noOfDevices;++i)
	device_destroy(cl,MKDEV(cdrv.devNo,i));
	class_destroy(cl);
	unregister_chrdev_region(drv_num,noOfDevices);
	unregister_chrdev(cdrv.devNo,"Sample");
	printk(KERN_DEBUG"Done Cleaning up\n");
}

loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
	switch(whence) {

	case SEEK_SET:
        if(off>SIZE) offset=SIZE;
	else
	offset=off;

	case SEEK_CUR:
	if(off>SIZE) offset=SIZE;
	else {
	offset+=off;
	if(offset>SIZE) offset=SIZE;
	}
	break;
	}
	printk(KERN_DEBUG"offset alligned to nearest page boundary at %d",offset);
	return offset;
}

static ssize_t read_my(struct file *a, char *b, size_t c, loff_t *d)
{
	int i,count=c;
	down(&cdrv.sem);
	if(!direction) {
	for(i=0;i<count && *d+i < SIZE && cdrv.ramdisk[*d+i];++i) {
	temp[i]=cdrv.ramdisk[*d+i];
	}
	temp[i]='\0';
	copy_to_user(b,temp,c);
	}
	else {
	for(i=0;i < count && *d+count-1-i >= 0 && cdrv.ramdisk[*d+count-1-i];++i) {
        temp[i]=cdrv.ramdisk[*d+count-1-i];
        }
        temp[i]='\0';
        copy_to_user(b,temp,c);
	}
	up(&cdrv.sem);
	return 0;
}

static ssize_t my_write(struct file *a,const char *b, size_t c, loff_t *e)
{
	int i,count=c,*d=&offset;
	down(&cdrv.sem);
	copy_from_user(temp,b,c);
	printk(KERN_DEBUG"buffer recieved is - %s\n",temp);
	if(!direction) {
	for(i=0;i<count && *d+i < SIZE-1 && temp[i];++i) {
	cdrv.ramdisk[*d+i]=temp[i];
	}
	cdrv.ramdisk[*d+i]='\0';
	printk(KERN_DEBUG"ramdisk content from %d is - %s\n",*d,cdrv.ramdisk+*d);
	}
	else {
	for(i=0;i < count && *d+count-1-i >= 0 && temp[i];++i) {
	cdrv.ramdisk[*d+count-1-i]=temp[i];
	}
  	cdrv.ramdisk[*d+count]='\0';
	printk(KERN_DEBUG"ramdisk content from %d is - %s\n",*d+count-i,cdrv.ramdisk+*d+count-i);
	}
	up(&cdrv.sem);
	return 0;
}

static long my_ioctl(struct file *b, unsigned int num, unsigned long param)
{
	char old_dir[20];
	printk(KERN_DEBUG"In Ioctl\n");
	switch(num) {
	case MY_READ:
	read_my(b,(char*)param,MAX_LENGTH,0);
	break;
	case MY_WRITE:
	my_write(b,(char*)param,MAX_LENGTH,0);
	break;
	case ASP_CHGACCDIR:
	down(&cdrv.sem);
	direction?strcpy(old_dir,"reverse"):strcpy(old_dir,"regular");
	if(!strcmp((char*)param,"reverse")) {
	direction=1;
	printk(KERN_DEBUG"change Direction to %d\n",direction);
	copy_to_user((char*)param,old_dir,sizeof(old_dir));
	up(&(cdrv.sem));
	}
	else {
	direction=0;
	printk(KERN_DEBUG"change Direction to %d\n",direction);
	copy_to_user((char*)param,old_dir,sizeof(old_dir));
	up(&(cdrv.sem));
	}
	break;
	}
	return 0;
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankit Bhadoria");

