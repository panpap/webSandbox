#ifndef _IFKIT_H_
#define _IFKIT_H_

#include <time.h>

struct power_stats {
	double voltage;
	double current;
	double power;
	time_t time;
	unsigned samples;
	long double sum_power;
	double mean_power;
};

extern struct power_stats p_12v, p_5v, p_3_3v, p_misc;

/* initialize sensor interface */
int
ifkit_init(void);

/* clear power stats */
void
ifkit_clear(void);

#endif /* _IFKIT_H_ */
