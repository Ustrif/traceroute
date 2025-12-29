#include "traceroute.h"

static int	get_val(const char *argv)
{
	if (!argv)
		return (-1);
	for (int i = 0; argv[i]; i++)
	{
		if (!(argv[i] >= '0' && argv[i] <= '9'))
			return (-1);
	}
	long	number;

	number = atol(argv);
	if (number > INT_MAX)
		return (-1);
	return ((int) number);
}

static void	place_data(t_args *args, char where, int val)
{
	switch (where)
	{
		case 'p':
			args->port = val;
			break ;
		case 'f':
			args->first_hop_num = val;
			break ;
		case 'm':
			args->max_hop_num = val;
			break ;
		case 'q':
			args->packet_nums = val;
			break ;
		case 'n':
			args->resolve = false;
			break ;
		case 't':
			args->tos = val;
			break ;
		case 'w':
			args->wait = val;
			break ;
		case 'T':
			args->protocol = TCP;
			break ;
		case 'U':
			args->protocol = UDP;
			break ;
		case 'I':
			args->protocol = ICMP;
		default:
			break ;
	}
}

static int	handle_options(t_args *args, int argc, char **argv)
{
	const char	*opts[] = {"-p", "-f", "-m", "-q", "-w", "-t", "-n", "-T", "-I", "-U", NULL};
	int	val;

	val = 0;
	for (int i = 1; i != argc; i++)
	{
		for (int y = 0; y != 10; y++)
		{
			if (strcmp(argv[i], opts[y]) == 0 && y < 6)
			{
				val = get_val(argv[i + 1]);
				if (val <= -1)
				{
					printf("parameter error\n");
					return (1);
				}
				place_data(args, opts[y][1], val);
				break ;
			}
			else if (strcmp(argv[i], opts[y]) == 0 && y >= 6)
			{
				place_data(args, opts[y][1], 0);
				break ;
			}
		}
	}
	return (0);
}

static bool	is_valid_hostname(const char *host)
{
	int		len;
	bool	last_was_dot;
	char	c;

	len = strlen(host);
	last_was_dot = true;
	if (!len || len == 0 || len > 253)
		return (false);
    for (int i = 0; i < len; i++)
    {
        c = host[i];
		if (c == '.')
		{
			if (last_was_dot)
				return (false);
			last_was_dot = true;
			continue ;
		}
		if (!(isalnum(c) || c == '-'))
			return (false);
		last_was_dot = false;
	}
	return (!last_was_dot);
}

static int	add_web(t_args *args, char **argv)
{
	for (int i = 1; argv[i]; i++)
	{
		if (argv[i][0] == '-')
			continue ;
		if (strlen(argv[i]) >= 3 && strchr(argv[i], '.') && is_valid_hostname(argv[i]))
		{
			args->host = argv[i];
			break ;
		}
	}
	if (!args->host)
	{
		printf("no valid host\n");
		return (1);
	}
	return (0);
}

int	parse_args(t_args *args, int argc, char **argv)
{
	if (argc == 1)
	{
		printf("Usage:\ntraceroute [-p port 1-65535] [-f first_hop 1-30] [-n no-resolve-host]"); 
		printf(" [-m max_hop 1-30] [-q packet number 1-5]\n[-t tos 0-256] [-w wait 1-20] [protocols -T -U -I] x.x\n\n");
		printf("-p for port. 1-65535\n");
		printf("-f for first hop number 1-30 for default\n");
		printf("-n no resolve for hop hostname");
		printf("-m for max hop number\n");
		printf("-q for packet number per ttl\n");
		printf("-t for tos 0-256\n");
		printf("-w for wait secs between packets\n");
		printf("-T for TCP requires root\n");
		printf("-U for UDP default protocol\n");
		printf("-I for ICMP requires root\n\n");
		printf("Default usage:\ntraceroute x.x \nis equals\n");
		printf("traceroute x.x -p 33434 -f 1 -m 30 -q 3 -t 0 -w 3 -U\n\n");
		printf("coded by github.com/Ustrif .\n");
		return (1);
	}
	if (handle_options(args, argc, argv))
		return (1);
	if (add_web(args, argv))
		return (1);
	return (0);
}
