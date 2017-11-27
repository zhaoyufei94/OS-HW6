#ifndef _GPS_D_H
#define _GPS_D_H

#define GPS_LOCATION_FILE "/data/media/0/gps_location.txt"
#define __NR_set_gps_location 245
struct gps_location {
	double latitude;
	double longitude;
	float accuracy; /*in meters*/
};
#endif
