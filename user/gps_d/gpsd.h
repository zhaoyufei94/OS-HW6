#ifndef _GPS_D_H
#define _GPS_D_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define GPS_LOCATION_FILE "/data/media/0/gps_location.txt"
#define __NR_set_gps_location 245
struct gps_location {
	double latitude;
	double longitude;
	float accuracy; /*in meters*/
};

void daemon_mode(void)
{
	pid_t pid, sid;

	pid = fork();
	if (pid == -1)
		goto error;

	if (pid > 0)
		exit(EXIT_SUCCESS);

	sid = setsid();

	if (sid < 0)
		goto error;

	if ((chdir("/")) < 0)
		goto error;

	return;
error:
	fprintf(stderr, "error: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
}


#endif
