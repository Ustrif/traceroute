#include <traceroute.h>

static struct timeval get_rtt(struct timeval *start, struct timeval *end)
{
	struct timeval diff;

	diff.tv_sec = end->tv_sec - start->tv_sec;
	diff.tv_usec = end->tv_usec - start->tv_usec;

	if (diff.tv_usec < 0)
	{
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}
	return (diff);
}

void	print_rtt(struct timeval *start, struct timeval *end)
{
	struct timeval	tv;
	long long		time;

	time = 0;
	tv = get_rtt(start, end);
	time += tv.tv_sec * 1000000;
	time += tv.tv_usec;

	time /= 1000;
	printf("%lld ms", time);
	fflush(stdout);
	
	return ;
}
