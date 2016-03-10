#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "head.h"
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/list.h>

#define MAX 255
#define MAX_LENGTH SIZE
#define SIZE 16*PAGE_SIZE

struct asp_mycdrv {
struct list_head list;
struct cdev cd;
char *ramdisk;
struct semaphore sem;
int devNo;
int dir;
};

static struct asp_mycdrv *cdrv=NULL;
static struct semaphore read_sem;
static struct list_head head;
static int drv_num=-1;
static char *ramdisk=NULL;
static char *mem[MAX]={NULL};
static char *temp=NULL;
static int dev_open(struct inode *in, struct file *fi);
static int dev_close(struct inode *in, struct file *fi);
static ssize_t read_my(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *,const char *, size_t, loff_t *);
static long my_ioctl(struct file *, unsigned int, unsigned long);
static loff_t my_llseek(struct file *filp, loff_t off, int whence);
struct file_operations fops={.owner=THIS_MODULE,.open=dev_open,.unlocked_ioctl=my_ioctl,.read=read_my,\
.llseek=my_llseek,.write=my_write,.release=dev_close};
struct class *cl;
static int noOfDevices=3;
static int *direction=NULL,k=0;

//noOfDevice can be passed as an argument while loading the module
module_param(noOfDevices,int,S_IWUSR|S_IRUGO);

int dev_open (struct inode *in, struct file *fi) {
	struct list_head *tmp=NULL;

	down(&read_sem);
	list_for_each(tmp,&head) {
	cdrv=list_entry(tmp,struct asp_mycdrv,list);

	if((MAJOR((cdrv->cd).dev)==imajor(in)) && (MINOR((cdrv->cd).dev)==iminor(in))) {
	printk(KERN_DEBUG"major %d minor %d\n",MAJOR((cdrv->cd).dev),MINOR((cdrv->cd).dev));
	direction=&(cdrv->dir);
	ramdisk=cdrv->ramdisk;
	temp=mem[MINOR((cdrv->cd).dev)];
	printk(KERN_DEBUG"Initial direction %d\n",cdrv->dir);
	break;
	}
	}
	if(!cdrv) {
	printk(KERN_DEBUG"Device not Found\n");
	return -1;
	}
	return 0;
}

int dev_close (struct inode *in, struct file *fi){
	up(&read_sem);
	return 0;
}

//initializing function module_init not required with this function
// had it been a different function module_init would be required
int init_module(void)
{
	int i,j;
	struct asp_mycdrv *temp1=NULL;
	struct list_head *tmp=NULL;
	INIT_LIST_HEAD(&head);

	for(j=0;j<noOfDevices;++j) {
	temp1=kmalloc(sizeof(struct asp_mycdrv),GFP_KERNEL);
	list_add(&(temp1->list),&head);
	}

	i=0;
	list_for_each(tmp,&head) {
	temp1=list_entry(tmp,struct asp_mycdrv,list);
	sema_init(&(temp1->sem),1);
	}
	sema_init(&read_sem,1);
	drv_num=register_chrdev(0,"Sample",&fops);
	if(drv_num==-1) {
	printk(KERN_DEBUG"Major number not assigned\n");
	return -1;
	}
	register_chrdev_region(drv_num,noOfDevices,"Sample");
	j=0;

	list_for_each(tmp,&head) {
        temp1=list_entry(tmp,struct asp_mycdrv,list);
	temp1->devNo=j;
	(temp1->cd).dev=MKDEV(drv_num,j);
	temp1->dir=0;
	if(drv_num!=-1){
	temp1->ramdisk=kmalloc(SIZE,GFP_KERNEL);
	if(!(temp1->ramdisk)) {
	printk(KERN_DEBUG"Memory Allocation Failed for Ramdisk\n");
	return -1;
	}
	}
	j++;
	}

	//driver specific memory
	for(i=0;i<noOfDevices;++i) {
	mem[i]=kmalloc(SIZE,GFP_KERNEL);
	if(!mem[i]) {
	for(k=0;k<i;++k) kfree(mem[k]);
	printk(KERN_DEBUG"Memory Allocation Failed for temp\n");
	return -1;
	}
	}

	cl=class_create(THIS_MODULE,"chrdrv");
	for(i=0;i<noOfDevices;++i)
	device_create(cl,NULL,MKDEV(drv_num,i),NULL,"mycdrv%i",i);

	list_for_each(tmp,&head) {
	temp1=list_entry(tmp,struct asp_mycdrv,list);
	cdev_init(&(temp1->cd),&fops);
	}

	i=0;
        list_for_each(tmp,&head) {
        temp1=list_entry(tmp,struct asp_mycdrv,list);
	cdev_add(&(temp1->cd),MKDEV(drv_num,i),1);
	i++;
	}
	printk(KERN_DEBUG"Done Intializing\n");
    	return 0;
}

//Cleanup Function module_exit not needed for this functiona
void cleanup_module(void)
{
	int i;
	struct asp_mycdrv *temp1;
	struct list_head *tmp,*next;
        //free ramdisk memory
	list_for_each(tmp,&head) {
        temp1=list_entry(tmp,struct asp_mycdrv,list);
	if(temp1 && temp1->ramdisk)
	kfree(temp1->ramdisk);
	}

	for(i=0;i<noOfDevices;++i)
	kfree(mem[i]);

        list_for_each(tmp,&head) {
        temp1=list_entry(tmp,struct asp_mycdrv,list);
	cdev_del(&(temp1->cd));
	}

	for(i=0;i<noOfDevices;++i)
	device_destroy(cl,MKDEV(drv_num,i));

	class_destroy(cl);
	unregister_chrdev_region(drv_num,noOfDevices);
	unregister_chrdev(drv_num,"Sample");

        list_for_each_safe(tmp,next,&head) {
        temp1=list_entry(tmp,struct asp_mycdrv,list);
	kfree(temp1);
	}

	printk(KERN_DEBUG"Done Cleaning up\n");

}

loff_t my_llseek(struct file *filp, loff_t off, int whence)
{
	switch(whence) {

	case SEEK_SET:
        if(off>SIZE) off=SIZE;
	else if(off<0) off=0;
	break;

	case SEEK_CUR:
	if(off>SIZE) off=SIZE;
	else if(off<0) off=0;
	else {
	filp->f_pos += off;
	if(filp->f_pos>SIZE) filp->f_pos=SIZE;
	}
	break;
	filp->f_pos = off;

	case SEEK_END:
	if(SIZE-off>=0)
	filp->f_pos=SIZE-off;
	else filp->f_pos=0;
	}
	return off;
}

static ssize_t read_my(struct file *a, char *b, size_t c, loff_t *d)
{
	struct semaphore *sem1=(struct semaphore*)&(cdrv->sem);
	int i,count=c;
	down(sem1);
	if(!(*direction)) {
	for(i=0;i<count && *d+i < SIZE && cdrv->ramdisk[*d+i];++i) {
	temp[i]=cdrv->ramdisk[*d+i];
	}
	temp[i]='\0';
	printk(KERN_DEBUG"buffer to write is -%s",temp);
	copy_to_user(b,temp,c);
	}
	else {
	for(i=0;i < count && *d+count-1-i >= 0 && cdrv->ramdisk[*d+count-1-i];++i) {
        temp[i]=cdrv->ramdisk[*d+count-1-i];
        }
        temp[i]='\0';
	printk(KERN_DEBUG"buffer to write is -%s",temp);
        copy_to_user(b,temp,c);
	}
	up(sem1);
	return 0;
}

static ssize_t my_write(struct file *a,const char *b, size_t c, loff_t *d)
{
	struct semaphore *sem1=(struct semaphore*)&(cdrv->sem);
	int i,count=c;
	down(sem1);
	copy_from_user(temp,b,c);
	printk(KERN_DEBUG"buffer recieved is - %s\n",temp);
	if(!(*direction)) {
	for(i=0;i<count && *d+i < SIZE-1 && temp[i];++i) {
	cdrv->ramdisk[*d+i]=temp[i];
	}
	cdrv->ramdisk[*d+i]='\0';
	a->f_pos+=i+1;
	printk(KERN_DEBUG"ramdisk content at offset %lld is - %s\n",*d,cdrv->ramdisk+*d);
	}
	else {
	for(i=0;i < count && *d+count-1-i >= 0 && temp[i];++i) {
	cdrv->ramdisk[*d+count-1-i]=temp[i];
	}
  	cdrv->ramdisk[*d+count]='\0';
	a->f_pos+=count+1;
	printk(KERN_DEBUG"ramdisk content at offset %lld is - %s\n",*d+count-i,cdrv->ramdisk+*d+count-i);
	}
	up(sem1);
	return 0;
}

static long my_ioctl(struct file *b, unsigned int num, unsigned long param)
{
	char old_dir[20];
	struct semaphore *sem1=(struct semaphore*)&(cdrv->sem);
	switch(num) {
	case MY_READ:
	read_my(b,(char*)param,num,0);
	break;
	case MY_WRITE:
	my_write(b,(char*)param,num,0);
	break;
	case ASP_CHGACCDIR:
	down(sem1);
	*direction?strcpy(old_dir,"reverse"):strcpy(old_dir,"regular");
	if(!strcmp((char*)param,"reverse")) {
	*direction=1;
	printk(KERN_DEBUG"change Direction to %d\n",*direction);
	copy_to_user((char*)param,old_dir,sizeof(old_dir));
	up(sem1);
	}
	else {
	*direction=0;
	printk(KERN_DEBUG"change Direction to %d\n",*direction);
	copy_to_user((char*)param,old_dir,sizeof(old_dir));
	up(sem1);
	}
	break;
	}
	return 0;
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ankit Bhadoria");
