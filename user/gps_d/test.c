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


int main()
{
	struct gps_location source, target;
	source.latitude = 10.2;
	source.longitude = 5.4;
	source.accuracy = 3.2;

	syscall(247, &source, &target);

	printf("lat: %f\nlog: %f\n, acc: %f\n", target.latitude, target.longitude, target.accuracy);
	return 0;
}
