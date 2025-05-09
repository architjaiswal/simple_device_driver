// ASP DD Intro code
// Compile using: make -C /lib/modules/6.8.0-57-generic/build M=$PWD modules
// Load kernel module using: insmod jsr_driver.ko
// Make device nodes using: mknod /dev/JSR_cdevice c 101 0
// All the printk and pr_info messages can be seen using: sudo dmesg | tail -n 30

#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/version.h>


#define MYDEV_NAME "JSR_cdevice"

#define ramdisk_size (size_t) (16*PAGE_SIZE)

static char *ramdisk;
static dev_t first;
static unsigned int count = 1;
static int my_major = 101, my_minor = 0;
static struct cdev *my_cdev;


 static int mycdrv_open(struct inode *inode, struct file *file)
 {
    pr_info(" OPENING device: %s:\n\n", MYDEV_NAME);
    return 0;
 }

 static int mycdrv_release(struct inode *inode, struct file *file)
 {
    pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
    return 0;
 }


 static ssize_t mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
 {
    int nbytes;
    if ((lbuf + *ppos) > ramdisk_size) {
        pr_info("trying to read past end of device,"
        "aborting because this is just a stub!\n");
        return 0;
    }

    nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
    *ppos += nbytes;
    pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);

    return nbytes;
 }


 static ssize_t mycdrv_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos)
 {
    int nbytes;

    if ((lbuf + *ppos) > ramdisk_size) {
        pr_info("trying to read past end of device, aborting because this is just a stub!\n");
        return 0;
    }

    nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
    *ppos += nbytes;
    pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);

    return nbytes;
 }

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mycdrv_read,
    .write = mycdrv_write,
    .open = mycdrv_open,
    .release = mycdrv_release,
};


static int __init my_init(void)
 {
    ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
    first = MKDEV(my_major, my_minor);
    int check = register_chrdev_region(first, count, MYDEV_NAME);
    if (check < 0 ){
        printk(KERN_ERR "jsr_driver: register_chrdev_region failed\n");
        return check;
    }
    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &mycdrv_fops);
    cdev_add(my_cdev, first, count);
    pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
    return 0;
 }

module_init(my_init);

static void __exit my_exit(void)
{
   cdev_del(my_cdev);
   unregister_chrdev_region(first, count);
   pr_info("\ndevice unregistered\n");
   kfree(ramdisk);
}

module_exit(my_exit);

MODULE_LICENSE("GPL v2");
