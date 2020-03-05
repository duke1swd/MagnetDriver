/*
 * Read an I/O pin.  Just to prove I can.
 * Now with wiringpi
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>

#define	MAXPIN	53

char *myname;
int pin;
int timetest;
int timing_loops;
int lib_mode;
int scan_mode;

static void
set_defaults()
{
	pin = 0;
	timetest = 0;
	lib_mode = 0;
	scan_mode = 0;
	timing_loops = 100000;
}

static void
usage()
{
	set_defaults();
	fprintf(stderr, "Usage: %s <options>\n",
		myname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-p <pin number (%d)>\n", pin);
	fprintf(stderr, "\t-l <run using libgpiod (%s)>\n",
		lib_mode? "yes": "no");
	fprintf(stderr, "\t-s <scan the pins for 1s (%s)>\n",
		scan_mode? "yes": "no");
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
	while ((c = getopt(argc, argv, "sltp:")) != EOF)
	switch (c) {
		case 's':
			scan_mode++;
			break;
		case 'l':
			lib_mode++;
			break;
		case 't':
			timetest++;
			break;
		case 'p':
			pin = atoi(optarg);
			break;
		default:
			usage();
	}

	if (scan_mode && timetest) {
		fprintf(stderr, "%s: scan mode and timing loop mode are mutually exclusive\n");
		errors++;
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

	if (lib_mode) {
		r = gpiod_ctxless_get_value("0", p, 0, "me");

		if (r < 0) {
			fprintf(stderr, "%s: error on get value\n",
				myname);
			perror("gpiod_ctxless_get_value");
			exit(1);
		}
	} else
		r = digitalRead(p);

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
	printf("usec/loop = %.2f\n", (double)totalusec / (double)timing_loops);
}

static void
scan_loop()
{
	int p;
	int v;
	int first;

	for (p = 0; p < MAXPIN; p++)
		pinMode(p, INPUT);

	for (;;) {
		first = 1;
		for (p = 0; p < MAXPIN; p++) {
			v = read_pin(p);
			if (v) {
				if (!first)
					printf(", ");
				first = 0;
				printf("%d", p);
			}
		}
		if (!first)
			printf("\n");
		else
			printf("None\n");
		sleep(1);
	}
}

int
main(int argc, char **argv)
{
	int v;
	myname = *argv;

	grok_args(argc, argv);

	if (!lib_mode) {
		wiringPiSetup();
		pinMode(pin, INPUT);
	}

	if (timetest)
		timing_loop();
	else if (scan_mode)
		scan_loop();
	else
		status_loop();

	return 0;
}
