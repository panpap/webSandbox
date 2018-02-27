#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE
#include <sys/time.h>

#include <sched.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#endif /* _WIN32 */

#include "hacks.h"

void (*_thishandler)(void);
#ifndef _WIN32
struct itimerval it;
#endif

#ifdef _WIN32
VOID STDCALL
_thisalarm(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser,
    DWORD_PTR dw1, DWORD_PTR dw2)
{
	_thishandler();
}
#else
void
_thisalarm(int sig)
{
	_thishandler();
}
#endif /* _WIN32 */

/* alarm once using handler and timeout */
void
setalarmonce(void (*handler)(void), unsigned msec)
{
	_thishandler = handler;
#ifdef _WIN32
	timeSetEvent(msec, msec, _thisalarm, 0,
	    TIME_ONESHOT | TIME_CALLBACK_FUNCTION);
#else
	signal(SIGALRM, _thisalarm);
	it.it_value.tv_sec = msec / 1000;
	it.it_value.tv_usec = (msec % 1000) * 1000;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &it, 0);
#endif /* _WIN32 */
}

/* alarm periodic using handler and timeout */
void
setalarmperiodic(void (*handler)(void), unsigned msec)
{
	_thishandler = handler;
#ifdef _WIN32
	timeSetEvent(msec, msec, _thisalarm, 0,
	    TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
#else
	signal(SIGALRM, _thisalarm);
	it.it_value.tv_sec = msec / 1000;
	it.it_value.tv_usec = (msec % 1000) * 1000;
	it.it_interval.tv_sec = it.it_value.tv_sec;
	it.it_interval.tv_usec = it.it_value.tv_usec;
	setitimer(ITIMER_REAL, &it, 0);
#endif /* _WIN32 */
}

/* set process affinity to one cpu */
int
setaffinity(int cpuid)
{
#ifdef _WIN32
	HANDLE process = GetCurrentProcess();
	DWORD_PTR mask = 1 << cpuid;
	BOOL ret = SetProcessAffinityMask(process, mask);
	return ret;
#else
	pid_t pid = getpid();
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);
	return sched_setaffinity(pid, sizeof(mask), &mask);
#endif /* _WIN32 */
}

/* go to sleep for sec seconds */
#ifdef _WIN32
void
sleep(unsigned sec)
{
	Sleep(sec * 1000);
}
#else
/* it is found in unistd.h */
#endif /* _WIN32 */

/* return counter in usecs */
unsigned long long
gettime(void)
{
#ifdef _WIN32
	unsigned long long counter;
	unsigned long long freq;
	QueryPerformanceCounter((PLARGE_INTEGER)&counter);
	QueryPerformanceFrequency((PLARGE_INTEGER)&freq);
	return (counter / ((double)freq / 1000000.0));
#else
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (tp.tv_sec * 1000000 + tp.tv_nsec / 1000.0);
#endif /* _WIN32 */
}

#ifdef _UTEST

#include <stdio.h>

unsigned long long cnt;
unsigned long long t, pt = 0;

void
stats(void)
{
	t = gettime();
	printf("cnt=%llu\n", cnt);
	if (pt)
		printf("dt=%llu\n", t - pt);
	pt = t;
	fflush(stdout);
}

int
main(void)
{
	setaffinity(0);

	setalarmperiodic(stats, 1000);

	while (1)
		cnt++;

	return 0;
}
#endif /* _UTEST */
