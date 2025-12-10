#include "traceroute.h"

int	main(int argc, char *argv[])
{
	t_args		args;
	t_program	pr;
	char		*ip;

	init_t_args(&args);
	if (parse_args(&args, argc, argv) || check_args(&args))
		return (1);
	ip = get_ip_address(&args);
	if (!ip)
		return (1);
	init_program(&pr, &args, ip);
	
	return (0);
}
