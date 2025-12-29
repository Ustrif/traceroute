#include "traceroute.h"

char	*reverse_resolver(in_addr_t ip)
{
	struct sockaddr_in	temp_addr;
	socklen_t			len;
	char				buf[NI_MAXHOST];
	char				*result;

	buf[0] = '\0';

	memset(&temp_addr, 0, sizeof(temp_addr));
	temp_addr.sin_family = AF_INET;
	temp_addr.sin_addr.s_addr = ip;

	len = sizeof(struct sockaddr_in);

	int	res = getnameinfo((struct sockaddr *) &temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD);
	if (res != 0)
	{
		return (NULL);
	}
	if (strlen(buf) > 1 && strcmp(buf, "_gateway") == 0)
	{
		return (NULL);
	}
	result = strdup(buf);
	return (result);
}
