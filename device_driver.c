
#include<linux/module.h>
#include<linux/string.h>
#include<linux/fs.h>
#include<asm/uaccess.h>
#include<linux/miscdevice.h>
#include<linux/semaphore.h>
#include<linux/slab.h>
#include<linux/moduleparam.h>

struct semaphore empty;    	 /* counts number of empty buffer slots */
struct semaphore full;  	 /* counts number of full buffer slots */
struct semaphore mutex;      	 /* mutex for critical region  */

MODULE_LICENSE("GPL");		/* This avoids kernel taint warning */
MODULE_DESCRIPTION("Producer Consumer Using Device");
MODULE_AUTHOR("Parag Chaudhari, Pravin Nagare");

char *deviceBuffer;
static int times = 0;
static int write_count = 0;
int buffSize = 0;
module_param(buffSize, int, S_IRUSR | S_IWUSR);
static struct miscdevice my_dev;

/* Called when open system call is done on mypipe */
static int mypipe_open(struct inode *inode,struct file *fp)
{	
	printk(KERN_ALERT "MyPipe Opened %d times\n", ++times);
	return 0;
}

/* Called when read system call is done on mypipe */
static ssize_t mypipe_read(struct file *filp,char *buff,size_t len,loff_t *off)
{
	unsigned int retByCopy=0,bytes_read=0, buff_counter, shift;
	for(buff_counter=0; buff_counter < len; buff_counter++)
	{
		if( down_interruptible(&full) < 0 ){				/* Decreament full count */
			printk(KERN_ALERT "User exit manually");		/* If interrupted by user */
			return -1;
		}
		if( down_interruptible(&mutex) < 0) {				/* Enter critical region */
			printk(KERN_ALERT "User exit manually");		/* If interrupted by user */
			return -1;
		}
			retByCopy = copy_to_user(buff+buff_counter, deviceBuffer, 1);		/* Copy from device buffer to user buffer */
			if(retByCopy < 0){
				printk(KERN_ALERT "Error in copy_to_user");
				return retByCopy;
			}
			bytes_read++;						/*Total number of bytes read from device buffer */
			printk(KERN_ALERT "READ from deviceBuffer:%c mesg[0]:%c buff_counter:%d len:%zu write_count:%d bytes_read:%d\n",buff[buff_counter],deviceBuffer[0],buff_counter, len,write_count, bytes_read);
			for(shift=0;shift < write_count-1; shift++)				/* Shift buffer left by one */
			{
				deviceBuffer[shift]=deviceBuffer[shift+1];
			}
			write_count--;								
		up(&mutex);						/* Leave critical region */
		up(&empty);						/* Increament count of empty slots */
	}
	return bytes_read;						/* Return total number of bytes read from device buffer */
}

/* Called when write system call is done on mypipe */
 ssize_t mypipe_write(struct file *filp,const char *buff,size_t len,loff_t *off)
{
	unsigned int ret=0,bytes_written=0;
	int buff_counter=0;
	while(buff_counter < len)
	{
		if( down_interruptible(&empty) < 0){			/* Decreament empty count */
			printk(KERN_ALERT "User exit manually");	/* If interrupted by user */
			return -1;
		}
						
		if( down_interruptible(&mutex) < 0){				/* Enter critical region */
			printk(KERN_ALERT "User exit manually");		/* If interrupted by user */
			return -1;
		}
			ret = copy_from_user(deviceBuffer+write_count, buff+buff_counter, 1); 	/* Copy from user buffer to device buffer */
			if(ret < 0){
				printk(KERN_ALERT "Error in copy_from_user");
				return ret;
			}

			bytes_written++;
			printk(KERN_ALERT "write in module deviceBuffer:%c, buff_counter:%d, len:%zu, write_count:%d bytesWritten:%d\n",deviceBuffer[write_count],buff_counter,len,write_count+1, bytes_written);
			write_count++;
			buff_counter++;	
		up(&mutex);						/* Leave critical region */
		up(&full);						/* Increament count of full slots */
	}
    	return bytes_written;					/* Return total number of bytes write to device buffer */
}

/* Called when close system call is done on mypipe */
static int mypipe_release(struct inode *inode,struct file *fil)
{
	printk(KERN_ALERT "Device Closed worked\n");	
	return 0;
}

static struct file_operations fops = 			/* Structure containing callbacks */
{
	.owner = THIS_MODULE,
	.open = mypipe_open,				/* Address of mypipe_open*/
	.release = mypipe_release,			/* Address of mypipe_release*/
	.write = mypipe_write,				/* Address of mypipe_write*/
	.read = mypipe_read				/* Address of mypipe_read*/
};

/* Called when module is loaded */
int __init init_module()
{
    int checkRegisterStatus;
   
    my_dev.minor = MISC_DYNAMIC_MINOR;
    my_dev.name = "mypipe";
    my_dev.fops = &fops;
    checkRegisterStatus = misc_register(&my_dev);
    printk("misc_register status: %d\n",checkRegisterStatus);
	
    sema_init(&full, 0);
    sema_init(&empty, buffSize);
    sema_init(&mutex, 1);

    if (checkRegisterStatus) 
	return checkRegisterStatus;   		//Return if mypipe device is not registered 
    printk("Minor Number %i\n", my_dev.minor);

    deviceBuffer = kmalloc(buffSize, GFP_KERNEL);
    if (!deviceBuffer){
	printk(KERN_ALERT "Error: Not able to allocate memory using kmalloc\n");
    }

    return 0;
}

/* Called when module is unloaded */
void __exit cleanup_module()
{
	int retval;
	printk(KERN_ALERT "Mypipe: Device removed\n");
	kfree(deviceBuffer);				/* Free buffer */
	retval=misc_deregister(&my_dev);		/* Deregister mypipe device */
}

