#include "traceroute.h"

static int	run(t_args *args, in_addr_t ip)
{
	int	result;

	result = 0;
	switch (args->protocol)
	{
		case ICMP:
			result = icmp(args, ip);
			break ;
		default:
			break;
	}
	if (result < 0)
		return (1);
	else if (result)
	{
		printf("\n* had not reached.\n");
		return (1);
	}

	printf("\n* Arrived! *\n");
	return (0);
}

int	main(int argc, char *argv[])
{
	t_args		args;
	in_addr_t	ip;

	init_t_args(&args);
	if (parse_args(&args, argc, argv) || check_args(&args))
		return (1);
	ip = get_ip_address(args.host);
	if (!ip)
		return (1);

	// INTRO
	// ekle.
	return (run(&args, ip));
}
