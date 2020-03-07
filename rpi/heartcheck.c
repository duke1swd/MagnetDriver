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
int debug;

static void
set_defaults()
{
	pin = 29;
	debug = 0;
}

static void
usage()
{
	set_defaults();
	fprintf(stderr, "Usage: %s <options>\n",
		myname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-p <pin number (%d)>\n", pin);
	fprintf(stderr, "\t-d (set debugging mode)\n");
	exit(1);
}

static void
grok_args(int argc, char **argv) {
	int c;
	int nargs;
	int errors;

	errors = 0;
	set_defaults();
	while ((c = getopt(argc, argv, "dp:")) != EOF)
	switch (c) {
		case 'd':
			debug++;
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

static pthread_t counter_thread;
static volatile int count;

#define	MAX_STAMP	50

struct stamp_s {
	unsigned long delta_t;
	int v;
};

long long stamp_start;

int nstamp;
struct stamp_s stamps[MAX_STAMP];

static void
stamp_init()
{
	int r;
	struct timeval s;

	nstamp = 0;

	r = gettimeofday(&s, NULL);
	if (r != 0) {
		fprintf(stderr, "%s: failed to get stamp time\n", myname);
		exit(1);
	}

	stamp_start = (long long)s.tv_sec * (long long)1000000 + s.tv_usec;
}

static void
stamp(int e)
{
	int r;
	struct timeval stamp;
	long long dt;

	if (nstamp >= MAX_STAMP)
		return;

	r = gettimeofday(&stamp, NULL);
	if (r != 0) {
		fprintf(stderr, "%s: failed to get stamp time\n", myname);
		exit(1);
	}

	dt = (long long)stamp.tv_sec * (long long)1000000 + stamp.tv_usec;
	dt -= stamp_start;
	stamps[nstamp].v = e;
	stamps[nstamp].delta_t = dt;
	nstamp++;
}

static void *
counter_routine(void *arg)
{
	int h, old;

	old = 0;
	count = 0;

	stamp_init();

	for (;;) {
		h = digitalRead(pin);
		if (debug && h != old)
			stamp(h);

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

static void
print_stamps()
{
	int i;
	unsigned long last;

	if (nstamp) {
		printf("Time Stamps:\n");

		last = 0;
		for (i = 0; i < nstamp; i++) {
			printf("%6d (%d): %d\n",
				stamps[i].delta_t,
				stamps[i].delta_t - last,
				stamps[i].v);
			last = stamps[i].delta_t;
		}
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
	print_stamps();

	return 0;
}
