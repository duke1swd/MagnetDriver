/*
 * Read an I/O pin.  Just to prove I can.
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define	MAXPIN	53

char *myname;
int pin;

static void
set_defaults()
{
	pin = 0;
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
	while ((c = getopt(argc, argv, "p:")) != EOF)
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

static int read_pin(unsigned int p) {
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

int
main(int argc, char **argv)
{
	int v;
	myname = *argv;

	grok_args(argc, argv);

	v = read_pin(pin);
	printf("pin %d is %d\n", pin, v);
	return 0;
}
