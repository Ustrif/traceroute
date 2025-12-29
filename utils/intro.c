#include "traceroute.h"

void	print_intro(t_args* args)
{
	printf("*****");
	printf(" ./traceroute to %s ", args->host);
	printf("*****\n\n");
}
