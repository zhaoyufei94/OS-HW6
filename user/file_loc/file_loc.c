/*
 * file_loc.c
 *
 * Columbia University
 * COMS W4118 Fall 2017
 * Homework 6 - Geo Tagged File System
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "file_loc.h"
 
#define _PATH_MAX 256
#define GMAPS_ZOOM_LEVEL 16

char *url_encode(const char *path)
{
	char *spath;
	char *real_path;
	char *c, *cend, *p, *pend;

	real_path = malloc(_PATH_MAX);
	if (!real_path)
		return NULL;
	if (!realpath(path, real_path))
		return NULL;

	spath = malloc(_PATH_MAX);
	if (!spath)
	{
		free(real_path);
		return NULL;
	}

	/* now srub the real path for some basic URL safety */
	c = real_path;
	cend = c + strlen(real_path);
	p = spath;
	pend = p + _PATH_MAX - 4;

	#define url_char(CHAR, HEX) \
		do { \
			case CHAR: \
				*p++ = '%'; \
				strncpy(p, #HEX, 2); \
				p += 2; \
				c++; \
				break; \
		} while (0)

	while (c < cend && p < pend) {
		switch (*c) {
		url_char('$', 24);
		url_char('&', 26);
		url_char('+', 2B);
		url_char(',', 2C);
		url_char('/', 2F);
		url_char(':', 3A);
		url_char(';', 3B);
		url_char('=', 3D);
		url_char('?', 3F);
		url_char('@', 40);
		url_char(' ', 20);
		url_char('"', 22);
		url_char('<', 3C);
		url_char('>', 3E);
		url_char('#', 23);
		url_char('%', 25);
		url_char('{', 7B);
		url_char('}', 7D);
		url_char('|', 7C);
		url_char('\\', 5C);
		url_char('^', 5E);
		url_char('~', 7E);
		url_char('[', 5B);
		url_char(']', 5D);
		url_char('`', 60);
		default:
			*p++ = *c++;
			break;
		}
	}

	*pend = 0; /* forced NULL termination */
	free(real_path);

	return spath;
}

/* make system call here */
static int do_file_loc(const char *path)
{
	/* print Google Maps URL based on path, return -1 on failure */
	struct gps_location loc;
	int age;
	char *spath;

	age = syscall(__NR_get_gps_location,path, &loc);
	if (age < 0)
		return -1;

	spath = url_encode(path);
	printf("%s:\n", path);
	printf("\tlatitude: %lf\n", loc.latitude);
	printf("\tlongitude: %lf\n", loc.longitude);
	printf("\taccuracy: %fm\n", loc.accuracy);
	printf("\tGoogle Maps:\n\t   http://maps.google.com/maps?iwloc=A&q=%lf,%lf%%20(File:%%20%s)&z=%d\n\n",
	       loc.latitude, loc.longitude, spath, GMAPS_ZOOM_LEVEL);
	free(spath);
	return 0;
}

static void usage(char **argv)
{
	printf("Usage: %s <file>\n", argv[0]);
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		usage(argv);


	if (do_file_loc(argv[1]) < 0) {
		printf("No GPS information:\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


