#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ifkit.h"
#include "hacks.h"

/* our flags */
int rtime;
int die;
int elapsed;

void
stats(void)
{
	printf("%f %f %f %f\n",
	    p_12v.mean_power, p_5v.mean_power,
	    p_3_3v.mean_power, p_misc.mean_power);

	ifkit_clear();

	if (rtime && ++elapsed >= rtime)
		die = 1;
}

void
usage(int ret)
{
	fprintf(stderr, "power [-h] [-t rtime]\n");
	exit(ret);
}


int
main(int argc, char *argv[])
{
	int ch;

	/* XXX: make output streams unbuffered */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	while ((ch = getopt(argc, argv, "t:h")) != -1) {
		switch (ch) {
		case 't':
			rtime = atoi(optarg);
			break;
		case 'h':
			usage(0);
		default:
			usage(1);
		}
	}
	argc -= optind;
	argv -= optind;

	ifkit_init();

	printf("# power_12v power_5v power_3_3v power_misc\n");

	/* set alarm */
	setalarmperiodic(stats, 100);

	while (!die)
		sleep(1);

	return 0;
}
