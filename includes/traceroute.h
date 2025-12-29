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
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <linux/errqueue.h>

#define BUF_SIZE 4096
#define SEQ 20025

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

struct icmp_payload
{
	struct timeval 	tv;
};

void			init_t_args(t_args	*arg);
int				parse_args(t_args *args, int argc, char **argv);
int				check_args(t_args *args);
in_addr_t		get_ip_address(char *host);
void			print_rtt(struct timeval *start, struct timeval *end);
char			*reverse_resolver(in_addr_t ip);
int				icmp(t_args *args, in_addr_t ip);
int				udp(t_args *args, in_addr_t ip);
int				tcp(t_args *args, in_addr_t ip);
unsigned short checksum(void *b, int len);
void			print_intro(t_args* args);

#endif // __traceroute__
