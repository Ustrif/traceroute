#ifndef __traceroute__
#define __traceroute__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/select.h>

#define BUF_SIZE 4096

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

typedef struct	s_pro
{
	t_args	*args;
	char	ip[16];
}	t_program;

struct icmp_payload
{
	struct timeval 	tv;
};

void	init_t_args(t_args	*arg);
int		parse_args(t_args *args, int argc, char **argv);
int		check_args(t_args *args);
char	*get_ip_address(t_args *args);
void	init_program(t_program *p, t_args *args, char *ip);
void	print_rtt(struct timeval *start, struct timeval *end);
char	*reverse_resolver(in_addr_t ip);

#endif // __traceroute__