#include "traceroute.h"

int	main(int argc, char *argv[])
{
	t_args	args;

	init_t_args(&args);
	if (parse_args(&args, argc, argv) || check_args(&args))
		return (1);
	return (0);
}
