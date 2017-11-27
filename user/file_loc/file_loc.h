#ifndef _FILE_LOC_H
#define _FILE_LOC_H

#define __NR_get_gps_location 246
struct gps_location {
	double latitude;
	double longitude;
	float accuracy; /*in meters*/
};
#endif
