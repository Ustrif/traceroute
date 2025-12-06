#include <traceroute.h>

void	init_t_args(t_args	*arg)
{
	arg->host = NULL;
	arg->port = 33434;
	arg->first_hop_num = 1;
	arg->protocol = UDP;
	arg->max_hop_num = 30;
	arg->packet_nums = 3;
	arg->resolve = true;
	arg->tos = 0;
	arg->wait = 3;
}
