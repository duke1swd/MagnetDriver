/*
 * Read an I/O pin.  Just to prove I can.
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>

#define	MAXPIN	53

char *myname;
int pin;
int timetest;
int timing_loops;

static void
set_defaults()
{
	pin = 0;
	timetest = 0;
	timing_loops = 10000;
}

static void
usage()
{
	set_defaults();
	fprintf(stderr, "Usage: %s <options>\n",
		myname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-p <pin number (%d)>\n", pin);
	fprintf(stderr, "\t-t <run a timing test (%s)>\n",
		timetest? "yes": "no");
	exit(1);
}

static void
grok_args(int argc, char **argv) {
	int c;
	int nargs;
	int errors;

	errors = 0;
	set_defaults();
	while ((c = getopt(argc, argv, "tp:")) != EOF)
	switch (c) {
		case 't':
			timetest++;
			break;
		case 'p':
			pin = atoi(optarg);
			break;
		default:
			usage();
	}

	nargs = argc - optind;

	if (pin < 0 || pin > MAXPIN) {
		fprintf(stderr, "%s: pin number (%d) must be "
				"in the range [0-%d]\n",
			myname, pin, MAXPIN);
		errors++;
	}

	if (nargs != 0) {
		fprintf(stderr, "%s: No positional arguments\n",
			myname);
		errors++;
	}

	if (errors)
		usage();
}

static int
read_pin(unsigned int p) {
	int r;

	r = gpiod_ctxless_get_value("0", p, 0, "me");

	if (r < 0) {
		fprintf(stderr, "%s: error on get value\n",
			myname);
		perror("gpiod_ctxless_get_value");
		exit(1);
	}

	return r;
}

static void
status_loop() {
	int v;

	for (;;) {
		v = read_pin(pin);
		printf("pin %d is %d\n", pin, v);
		sleep(1);
	}
}

static void
timing_loop()
{
	int r;
	int i;
	struct timeval start, end;
	long long totalusec;

	r = gettimeofday(&start, NULL);
	if (r != 0) {
		fprintf(stderr, "%s: failed to get start time\n", myname);
		exit(1);
	}

	for (i = 0; i < timing_loops; i++)
		(void)read_pin(pin);

	r = gettimeofday(&end, NULL);
	if (r != 0) {
		fprintf(stderr, "%s: failed to get start time\n", myname);
		exit(1);
	}

	totalusec = (long long)end.tv_sec * (long long)1000000;
	totalusec += end.tv_usec;
	totalusec -= (long long)start.tv_sec * (long long)1000000;
	totalusec -= start.tv_usec;
	printf("usec/loop = %.1f\n", (double)totalusec / (double)timing_loops);
}

int
main(int argc, char **argv)
{
	int v;
	myname = *argv;

	grok_args(argc, argv);

	if (timetest)
		timing_loop();
	else
		status_loop();

	return 0;
}
