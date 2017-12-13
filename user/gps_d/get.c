#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "gpsd.h"

int main(int argc, char *argv[])
{
	struct gps_location gps;
	int age;
	const char *path;
	
	if (argc > 1) {
		path = argv[1];
		printf("path: %s\n", path);
	} else {
		return 0;
	}

	age = syscall(246, path, &gps);
	if (age < 0) {
		printf("error: %s\n", strerror(errno));
		return 0;
	} else {
		printf("lat: %f\n", gps.latitude);
		printf("log: %f\n", gps.longitude);
		printf("acc: %f\n", gps.accuracy);
		printf("age: %d\n", age);
		return 0;
	}
}
