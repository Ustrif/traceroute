#ifndef __traceroute__
#define __traceroute__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

typedef enum	s_proto
{
	UDP,
	ICMP,
	TCP
}	e_protocol;

typedef struct s_args
{
	char		*host;
	int			port;
	int			first_hop_num;
	e_protocol	protocol;
	int			max_hop_num;
	int			packet_nums;
	bool		resolve;
	int			tos;
	int			wait;
}	t_args;

void	init_t_args(t_args	*arg);
int		parse_args(t_args *args, char **argv);


#endif // __traceroute__