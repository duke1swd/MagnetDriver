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

#include "pins.h"

#define	MAXPIN	53
#define	MAXECHO	15

char *myname;
int pin;
int echo_mode;
int echo_value;
int debug;

static void
set_defaults()
{
	pin = CLK;
	echo_mode = 0;
	echo_value = 0;
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
	fprintf(stderr, "\t-e <echo value (disabled)>\n");
	fprintf(stderr, "\t-d (set debugging mode)\n");
	exit(1);
}

static void
grok_args(int argc, char **argv) {
	int c;
	int nargs;
	int errors;
	int pin_set;

	errors = 0;
	pin_set = 0;
	set_defaults();
	while ((c = getopt(argc, argv, "e:dp:")) != EOF)
	switch (c) {
		case 'e':
			echo_mode++;
			echo_value = atoi(optarg);
			break;
		case 'd':
			debug++;
			break;
		case 'p':
			pin = atoi(optarg);
			pin_set++;
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

	if (echo_mode && (echo_value < 0 || echo_value > MAXECHO)) {
		fprintf(stderr, "%s: echo value (%d) must be in the range [0-%d]\n",
			myname,
			echo_value);
		errors++;
	}

	if (pin_set && echo_mode) {
		fprintf(stderr, "%s: pins cannot be adjusted with echo mode\n",
			myname);
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

static void
setup()
{
	wiringPiSetup();

	pinMode(pin, INPUT);

	if (echo_mode) {
		pinMode(OL0, OUTPUT);
		pinMode(OL1, OUTPUT);
		pinMode(OL2, OUTPUT);
		pinMode(OL3, OUTPUT);
		
		pinMode(IL0, INPUT);
		pinMode(IL1, INPUT);
		pinMode(IL2, INPUT);
		pinMode(IL3, INPUT);
		pinMode(IL4, INPUT);
		pinMode(IL5, INPUT);

		pinMode(SW1, INPUT);

		pinMode(CLK, INPUT);
	}
}

static void
heartbeat_test(){
	spawn_counter();
	sleep(1);
	printf("Counter = %d\n", count);
	print_stamps();
}

static void
set_output(int v)
{
	if (v & 0x1)
		digitalWrite(OL0, HIGH);
	else
		digitalWrite(OL0, LOW);

	if (v & 0x2)
		digitalWrite(OL1, HIGH);
	else
		digitalWrite(OL1, LOW);

	if (v & 0x4)
		digitalWrite(OL2, HIGH);
	else
		digitalWrite(OL2, LOW);

	if (v & 0x8)
		digitalWrite(OL3, HIGH);
	else
		digitalWrite(OL3, LOW);
}

static int
get_input()
{
	int v;

	v = 0;


	if (digitalRead(IL0))
		v |= 0x1;

	if (digitalRead(IL1))
		v |= 0x2;

	if (digitalRead(IL2))
		v |= 0x4;

	if (digitalRead(IL3))
		v |= 0x8;

	if (digitalRead(IL4))
		v |= 0x10;

	if (digitalRead(IL5))
		v |= 0x20;

	return v;
}

static void
echo_test()
{
	int v;

	printf("Setting value %d\n", echo_value);
	set_output(echo_value);
	sleep(1);
	v = get_input();
	printf("Got value %d\n", v);
}

int
main(int argc, char **argv)
{
	int v;
	myname = *argv;

	grok_args(argc, argv);
	setup();

	if (echo_mode)
		echo_test();
	else
		heartbeat_test();

	return 0;
}
