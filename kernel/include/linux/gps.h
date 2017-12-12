#ifndef GPS_H_
#define GPS_H_

#include <linux/time.h>

//unsigned long long int double_long(double d);

struct gps_location {
	double	latitude;
	double	longitude;
	float	accuracy;
};

struct kernel_gps {
	struct	gps_location loc;
	struct	timespec timestamp;
};

struct inode_gps {
	__u64	latitude;
	__u64	longitude;
	__u32	accuracy;
	__u32	age;
};

#endif
