#include <linux/gps.h>
#include <linux/syscalls.h>
#include <linux/time.h>

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

	if (ulat == 0 || ulog == 0 || uacc == 0)
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
