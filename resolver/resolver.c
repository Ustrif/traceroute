#include "traceroute.h"

static in_addr_t resolver(char *hostname)
{
	struct addrinfo		hints;
	struct addrinfo		*rp;
	int					s;
	struct sockaddr_in	*ipv4;
	in_addr_t			result = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	s = getaddrinfo(hostname, NULL, &hints, &rp);
	if (s != 0)
	{
		printf("ip adresi bulunamadı: %s\n", gai_strerror(s));
		return (0);
	}
	ipv4 = (struct sockaddr_in *)rp->ai_addr;

	result = ipv4->sin_addr.s_addr;
	freeaddrinfo(rp);

	return (result);
}

in_addr_t get_ip_address(const char *host)
{
	in_addr_t	result;

	result = resolver(host);
	return (result);
}

