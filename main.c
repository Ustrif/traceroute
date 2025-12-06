#include "traceroute.h"

int	main(int argc, char *argv[])
{
	t_args	args;

	init_t_args(&args);
	if (parse_args(&args, argc, argv))
		return (1);
	print_args(&args);
	return (0);
}
