#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>

#include "gpsd.h"


#define SET_GPS 245
#define GPS_LOCATION_FILE "/data/media/0/gps_location.txt"

static struct gps_location gps_generator(int i)
{
	struct gps_location new;
	FILE *file = NULL;

	new.latitude = i;
	new.longitude = i;
	new.accuracy = i;

	file = fopen(GPS_LOCATION_FILE, "w");
	if (!file)
		printf("Failed to open GPS file\n");
	fprintf(file, "%d\n%d\n%d\n", i, i, i);
	fclose(file);

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

	gps_file = fopen(GPS_LOCATION_FILE, "w");
	if (!gps_file)
		printf("Failed to open GPS file\n");
	fprintf(gps_file, "%s", "40.80548105\n-73.96202195\n247.0\n");
	fclose(gps_file);

	while (!exit) {
		if (argc > 1 && !strcmp(argv[1], "-e")) {
			gps = gps_generator(i);
		} else {
			gps_file = fopen(GPS_LOCATION_FILE, "r");
			if (!gps_file) {
				printf("[%d]error: Failed to open GPS file!\n", i);
				fclose(gps_file);
				goto out;
			}
			if (gps_reader(gps_file, &gps) < 0) {
				printf("[%d]error: Failed to read GPS data\n", i);
				fclose(gps_file);
				goto out;
			}
			
		}

		if (syscall(SET_GPS, &gps) < 0)
			printf("[%d]error: %s\n", i, strerror(errno));
		else
		       printf("[%d]success: set GPS location\n", i);	
out:
		sleep(i);
		if (i++ == 10)
			exit = 1;
	}
	return 0;
}
