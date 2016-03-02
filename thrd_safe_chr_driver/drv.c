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
#define MAX_LENGTH 100
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
static char *ramdisk=NULL;
static char *temp=NULL;
static ssize_t read_my(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *, char *, size_t, loff_t *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
static loff_t my_llseek(struct file *filp, loff_t off, int whence);
struct file_operations fops={.unlocked_ioctl=my_ioctl,.read=read_my,.llseek=my_llseek,.write=my_write};
struct class *cl;
static int noOfDevices=3;
static int direction=0;
static int count=10;
static loff_t offset=0;

module_param(noOfDevices,int,S_IRUSR | S_IWUSR);
module_param(count,int,S_IRUSR | S_IWUSR);

//initializing function module_init not required with this function
// had it been a different function module_init would be required
int init_module(void) 
{
	int i;
	sema_init(&(cdrv.sem),1);
	drv_num=register_chrdev(0,"Sample",&fops);
	cdrv.devNo=drv_num;
	if(drv_num!=-1){
	ramdisk=kmalloc(SIZE,GFP_KERNEL);
	if(!ramdisk) {
	printk(KERN_DEBUG"Memory Allocation Failed\n");
	return -1;
	}
	if(!temp) {
	printk(KERN_DEBUG"Memory Allocation Failed\n");
	return -1;
	}
	cl=class_create(THIS_MODULE,"chrdrv");
	register_chrdev_region(drv_num,noOfDevices,"Sample");
	for(i=0;i<noOfDevices;++i)
	device_create(cl,NULL,MKDEV(drv_num,i),NULL,"mydrv%i",i);
	cdev_init(&(cdrv.cd),&fops);
	for(i=0;i<noOfDevices;++i)
	cdev_add(&(cdrv.cd),MKDEV(drv_num,i),1);
	}
	else printk(KERN_DEBUG"Major Number Not Assigned\n");
    	return 0;
}

//Cleanup Function module_exit not needed for this functiona
void cleanup_module(void) 
{
	kfree(ramdisk);
	kfree(temp);
	cdev_del(&(cdrv.cd));
	device_destroy(cl,drv_num);
	class_destroy(cl);
	unregister_chrdev_region(drv_num,noOfDevices);
	unregister_chrdev(cdrv.devNo,"Sample");

}

loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
return 0;
}

static ssize_t read_my(struct file *a, char *b, size_t c, loff_t *d)
{
	int i;
	if(!direction) {
	for(i=0;i<count && *d+i < SIZE;++i) {
	temp[i]=ramdisk[*d+i];
	}
	temp[i]='\0';
	copy_to_user(b,temp,c);
	}
	else {
	int j;
	for(i=count,j=0;i>=0 && *d-i >= 0;--i,j++) {
        temp[j]=ramdisk[*d-i];
        }
        temp[j]='\0';
        copy_to_user(b,temp,c);
	}
	*d=0;
	return 0;
}

static ssize_t my_write(struct file *a, char *b, size_t c, loff_t *d)
{
	char str[MAX_LENGTH];
	copy_from_user(str,b,c);
	printk(KERN_INFO"Message Received from UserSpace: %s\n",str);
	*d=0;
	return 0;
}

static long my_ioctl(struct file *b, unsigned int num, unsigned long param)
{
	switch(num) {
	case MY_READ:
	read_my(b,(char*)param,MAX_LENGTH,&offset);
	break;
	case MY_WRITE:
	my_write(b,(char*)param,MAX_LENGTH,&offset);
	break;
	case ASP_CHGACCDIR:
	down(&cdrv.sem);
	if(!strcmp((char*)param,"reverse")) {
	direction=1;
	up(&(cdrv.sem));
	}
	else {
	direction=0;
	up(&(cdrv.sem));
	}
	break;
	}
	return 0;
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankit Bhadoria");

