#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "head.h"
#include <asm/uaccess.h>
#define MAX_LENGTH 100

static struct cdev cd;
static int drv_num=-1;
static ssize_t read_my(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *, char *, size_t, loff_t *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
struct file_operations fops={.unlocked_ioctl=my_ioctl,.read=read_my,.write=my_write};
struct class *cl;

//initializing function module_init not required with this function
// had it been a different function module_init would be required
int init_module(void) 
{
	drv_num=register_chrdev(0,"Sample",&fops);
	if(drv_num!=-1){
	cl=class_create(THIS_MODULE,"chrdrv");
	device_create(cl,NULL,MKDEV(drv_num,0),NULL,"mydrv%i",0);
	cdev_init(&cd,&fops);
	cdev_add(&cd,MKDEV(drv_num,0),1);
	}
	else printk(KERN_DEBUG"Major Number Not Assigned\n");
    	return 0;
}

//Cleanup Function module_exit not needed for this functiona
void cleanup_module(void) 
{
	cdev_del(&cd);
	device_destroy(cl,drv_num);
	class_destroy(cl);
	unregister_chrdev(drv_num,"Sample");

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
	}
	return 0;
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankit Bhadoria");
