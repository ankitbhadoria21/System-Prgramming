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
static ssize_t read_my(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *, char *, size_t, loff_t *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
struct file_operations fops={.unlocked_ioctl=my_ioctl,.read=read_my,.write=my_write};
struct class *cl;
int noOfDevices=3;
int direction=0;

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
	cdev_del(&(cdrv.cd));
	device_destroy(cl,drv_num);
	class_destroy(cl);
	unregister_chrdev_region(drv_num,noOfDevices);
	unregister_chrdev(cdrv.devNo,"Sample");

}

loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
    loff_t newpos;
    switch(whence) {
      case 0: /* SEEK_SET */
        newpos = off;
        break;

      case 1: /* SEEK_CUR */
        newpos = filp->f_pos + off;
        break;
/*
      case 2: 
        newpos = dev->size + off;
        break;
*/
      default: 
        return -EINVAL;
    }
    if (newpos < 0) return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}

static ssize_t read_my(struct file *a, char *b, size_t c, loff_t *d)
{
	char str[MAX_LENGTH];
	strcpy(str,"IOCTL Read Done\n");
	copy_to_user(b,str,MAX_LENGTH);
	return 0;
}

static ssize_t my_write(struct file *a, char *b, size_t c, loff_t *d)
{
	char str[MAX_LENGTH];
	copy_from_user(str,b,c);
	printk(KERN_INFO"Message Received from UserSpace: %s\n",str);
	return 0;
}

static long my_ioctl(struct file *b, unsigned int num, unsigned long param)
{
	switch(num) {
	case MY_READ:
	read_my(b,(char*)param,sizeof((char*)param),0);
	break;
	case MY_WRITE:
	my_write(b,(char*)param,MAX_LENGTH,0);
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

