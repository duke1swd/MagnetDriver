/*
 * Read an I/O pin.  Just to prove I can.
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>

char *myname;

static int pin(unsigned int p) {
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
	int p;
	myname = *argv;

	p = 77;

	v = pin(p);
	printf("pin %d is %d\n", p, v);
	return 0;
}
