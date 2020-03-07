/*
 * Read an I/O pin.  Just to prove I can.
 * Now with wiringpi
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <pthread.h>

#define	MAXPIN	53

char *myname;
int pin;

static void
set_defaults()
{
	pin = 29;
}

static void
usage()
{
	set_defaults();
	fprintf(stderr, "Usage: %s <options>\n",
		myname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-p <pin number (%d)>\n", pin);
	exit(1);
}

static void
grok_args(int argc, char **argv) {
	int c;
	int nargs;
	int errors;

	errors = 0;
	set_defaults();
	while ((c = getopt(argc, argv, "slp:")) != EOF)
	switch (c) {
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

static pthread_t counter_thread;
static volatile int count;

static void *
counter_routine(void *arg)
{
	int h, old;

	old = 0;
	count = 0;

	for (;;) {
		h = digitalRead(pin);
		if (h && !old)
			count++;
		old = h;
	}
}

static void
spawn_counter()
{
	int r;

	r = pthread_create(&counter_thread,
		NULL,
		counter_routine,
		(void *)0);
	if (r != 0) {
		fprintf(stderr, "%s: cannot spawn counter thread: %d\n",
			myname, r);
		exit(1);
	}
}

int
main(int argc, char **argv)
{
	int v;
	myname = *argv;

	grok_args(argc, argv);

	wiringPiSetup();
	pinMode(pin, INPUT);

	spawn_counter();
	sleep(1);
	printf("Counter = %d\n", count);

	return 0;
}
