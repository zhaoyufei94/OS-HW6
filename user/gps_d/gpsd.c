#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "gpsd.h"


#define SET_GPS 245
#define GPS_FILE "/data/media/0/gps_location.txt"

static struct gps_location gps_generator(int i)
{
	struct gps_location new;
	new.latitude = 10.1 * i;
	new.longitude = 12.3 * i;
	new.accuracy = 3.2 * i;
	return new;
}

static int gps_reader(FILE *gps_file, struct gps_location *loc)
{
	char line[100];
	int num;

	if (!gps_file || !loc) {
		printf("invalid argument\n");
		return -1;
	}

	for (num = 0; num < 3; num++) {
		fgets(line, 100, gps_file);
		line[strlen(line) - 1] = '\0';

		switch (num) {
			case 0:
				loc->latitude = strtod(line, NULL);
				printf("lat=%f\n", loc->latitude);
				break;
			case 1:
				loc->longitude = strtod(line, NULL);
				break;
			case 2:
				loc->accuracy = strtof(line, NULL);
				printf("acc=%f\n", loc->accuracy);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct gps_location gps;
	int exit = 0, i = 1;
	FILE *gps_file = NULL;

	while (!exit) {
		if (argc > 1 && !strcmp(argv[1], "-e")) {
			gps = gps_generator(i++);
		} else {
			gps_file = fopen(GPS_LOCATION_FILE, "r");
			if (!gps_file) {
				printf("[%d]error: Failed to open GPS file!\n", i);
				continue;
			}
			if (gps_reader(gps_file, &gps) < 0) {
				printf("[%d]error: Failed to read GPS data\n", i);
				continue;
			}
			
		}

		if (syscall(__NR_set_gps_location, &gps) < 0)
			printf("[%d]error: %s\n", i, strerror(errno));
		else
		       printf("[%d]success: set GPS location\n", i);	
		sleep(1);
		if (i == 10)
			exit = 1;
	}
	return 0;
}
