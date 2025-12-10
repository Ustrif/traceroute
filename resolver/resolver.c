#include "traceroute.h"

static char	*resolver(char *hostname)
{
	struct addrinfo		hints;
	struct addrinfo		*rp;
	int					s;
	char				ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in	*ipv4;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	s = getaddrinfo(hostname, NULL, &hints, &rp);
	if (s != 0)
	{
		printf("ip adresi bulunamadı: %s\n", gai_strerror(s));
		return (NULL);
	}
	ipv4 = (struct sockaddr_in *)rp->ai_addr;
	inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
	freeaddrinfo(rp);

	return (strdup(ipstr));
}

char	*get_ip_address(t_args *args)
{
	char	*result;

	result = resolver(args->host);
	return (result);
}

