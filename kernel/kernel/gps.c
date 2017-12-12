#include <linux/gps.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/limits.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#ifndef R_OK
#define R_OK 4
#endif /* R_OK */


/* GPS lock: need to grab this lock before access kgps */
static DEFINE_RWLOCK(gps_lock);

/* kernel structure to store latest gps info  */
struct kernel_gps kgps = {
	.loc = {
		.latitude	= 0,
		.longitude	= 0,
		.accuracy	= 0

	},
	.timestamp = {
		.tv_sec		= 0,
		.tv_nsec	= 0
	}
};

static unsigned long long int double_long(double d)
{
	unsigned long long int ret;
	ret = *((unsigned long long int *)(&d));
	return ret;
}

static unsigned int float_int(float f)
{
	unsigned int ret;
	ret = *((unsigned int *)(&f));
	return ret;
}

static int check_valid(struct gps_location *u_gps)
{
	struct gps_location *k_gps = &kgps.loc;
	unsigned long long int ulat, klat;
	unsigned long long int ulog, klog;
	unsigned int uacc, kacc;

	ulat = double_long(u_gps->latitude);
	ulog = double_long(u_gps->longitude);
	uacc = float_int(u_gps->accuracy);

	if (ulat == 0 && ulog == 0 && uacc == 0)
		return -1;

	read_lock(&gps_lock);
	
	klat = double_long(k_gps->latitude);
	klog = double_long(k_gps->longitude);
	kacc = float_int(k_gps->accuracy);

	read_unlock(&gps_lock);

	if (ulat == klat && ulog == klog && uacc == kacc)
		return 0;

	printk("need update!\n");
	printk("lat: %llu -> %llu\n", klat, ulat);
	printk("log: %llu -> %llu\n", klog, ulog);
	printk("acc: %d -> %d\n", kacc, uacc);

	return 1;
}

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	struct gps_location *k_gps = &kgps.loc;
	struct gps_location u_gps;
	int update;

	if (current_uid() != 0 && current_euid() != 0)
		return -EACCES;

	if (copy_from_user(&u_gps, loc, sizeof(struct gps_location)) != 0)
		return -EFAULT;
	
	update = check_valid(&u_gps);
	/* user gives all zero input */
	if (update < 0)
		return -EINVAL;
		//printk("all zero input!\n");
	/* kernel gps info need to update */
	if (update == 1) {
		write_lock(&gps_lock);
		memcpy(k_gps, &u_gps, sizeof(struct gps_location));
		printk("update!\n");
		kgps.timestamp = current_kernel_time();
		printk("current time:\n");
		printk("%ld %ld\n\n", kgps.timestamp.tv_sec, kgps.timestamp.tv_nsec);
		write_unlock(&gps_lock);
	}

	return 0;
}

/* Using userspace argument is essential here
 * because sys_open and sys_access only accept them
 * otherwise they will always return -EFAULT: Bad address
 * Linux does this to limit userspace access kernel files
 * */
static int access(const char __user *path)
{
	int ret;

	if (!path)
		return -EINVAL;
	//printk("access: path = %s\n", path);
	ret = sys_open(path , O_DIRECTORY, O_RDONLY);
	if (ret < 0) {
		printk("access: cannot open as directory\n");
		if (ret == -20) {  /* Not a directory, then maybe a file*/
			ret = sys_access(path, R_OK);
			printk("access file: ret = %d\n", ret);
		}
	} else {
		printk("access: openable as directory\n");
		sys_close(ret);
	}
	return ret;
}


static int get_gps_info(const char *kpath, struct gps_location *loc)
{
	struct path kkpath = { .mnt = NULL, .dentry = NULL };
	struct inode *kinode;
	int age = 0;

	if (!kpath || !loc)
		return -EINVAL;

	if (kern_path(kpath, LOOKUP_FOLLOW | LOOKUP_AUTOMOUNT, &kkpath) < 0)
		return -EFAULT;
	kinode = kkpath.dentry->d_inode;

	if (!kinode)
		return -EFAULT;
	
	if (kinode->i_op->get_gps_location)
		age = kinode->i_op->get_gps_location(kinode, loc);
	path_put(&kkpath);

	return age;
}

SYSCALL_DEFINE2(get_gps_location, const char __user *, pathname, struct gps_location __user *, loc)
{
	struct gps_location kloc;
	char *kpath = NULL;
	int age = 0, copy = 0;
	int path_size = (PATH_MAX + 1) * sizeof(char);

	if (!pathname || !loc)
		return -EINVAL;
	kpath = kmalloc(path_size, GFP_KERNEL);
	if (!kpath)
		return -ENOMEM;

	age = (int) strncpy_from_user(kpath, pathname, path_size);
	printk("actual copy %d bytes from userspace\n", age);
	if (age < 0) {
		kfree(kpath);
		return -EFAULT;
	} else if (age == path_size) {
		kfree(kpath);
		return -ENAMETOOLONG;
	}
	printk("kpath = %s\n", kpath);

	age = access(pathname);
	if (age < 0) {
		kfree(kpath);
		return age;
	}
	

	age = get_gps_info(kpath, &kloc);
	if (age < 0) {
		kfree(kpath);
		return -ENODEV;
	}
	
	copy = copy_to_user(loc, &kloc, sizeof(struct gps_location));
	if (copy < 0) {
		kfree(kpath);
		return -EFAULT;
	}

	kfree(kpath);
	return age;
		
}
