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
				//printf("lat=%f\n", loc->latitude);
				break;
			case 1:
				loc->longitude = strtod(line, NULL);
				break;
			case 2:
				loc->accuracy = strtof(line, NULL);
				//printf("acc=%f\n", loc->accuracy);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct gps_location gps;
	int exit = 0, i = 1;
	FILE *gps_file = NULL;

	printf("turning into daemon\n");
	daemon_mode();

	//fprintf(gps_file, "%s", "40.80548105\n-73.96202195\n247.0\n");
	
	while (!exit) {
		//gps_file = fopen(GPS_LOCATION_FILE, "w");
		//if (!gps_file)
		//	printf("Failed to open GPS file\n");
		switch (i) {
			case 1:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.80548105\n-73.96202195\n247.0\n");
				fclose(gps_file);
				break;
			case 3:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.809934\n-73.960617\n120.4\n");
				fclose(gps_file);
				break;
			case 4:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.808935\n-73.960992\n34.9\n");
				fclose(gps_file);
				break;
			case 6:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.807217\n-73.961873\n198.3\n");
				fclose(gps_file);
				break;
			case 8:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.807826\n-73.963364\n77.6\n");
				fclose(gps_file);
				break;
			case 9:
				gps_file = fopen(GPS_LOCATION_FILE, "w");
				fprintf(gps_file, "%s", "40.806892\n-73.962806\n156.2\n");
				fclose(gps_file);
				break;
		}
		//fclose(gps_file);

		if (argc > 1 && !strcmp(argv[1], "-e")) {
			gps = gps_generator(i);
		} else {
			gps_file = fopen(GPS_LOCATION_FILE, "r");
			if (!gps_file) {
				printf("error: Failed to open GPS file!\n");
				return 0;
			}
			if (gps_reader(gps_file, &gps) < 0) {
				printf("error: Failed to read GPS data\n");
				fclose(gps_file);
				return 0;
			}
			
		}
		if (syscall(__NR_set_gps_location, &gps) < 0) {
			printf("error: %s\n", strerror(errno));
			return 0;
		}

		sleep(1);
		if (i++ == 10)
			i = 1;
	}
	return 0;
}
