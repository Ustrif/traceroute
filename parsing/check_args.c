#include "traceroute.h"

int		check_args(t_args *args)
{
	if (!args)
		return (printf("memory error\n"), 1);
	if (args->port >= __INT_LEAST16_MAX__ * 2 || args->port <= 0)
		return (printf("port number error\n"), 1);
	if (args->first_hop_num > args->max_hop_num || args->first_hop_num == 0)
		return (printf("first hop number error\n"), 1);
	if (args->packet_nums > 5 || args->packet_nums < 1)
		return (printf("packet number error\n"), 1);
	if (args->wait > 20 && args->wait >= 1)
		return (printf("wait sec error\n"), 1);
	if (args->tos > __INT_LEAST8_MAX__ * 2)
		return (printf("tos value error\n"), 1);
	if (args->protocol == TCP && args->port == 33434)
		args->port = 80;
	return (0);
}
