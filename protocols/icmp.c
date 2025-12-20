#include "traceroute.h"

#define SEQ 20025

bool g_flag = 0;

static unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if (len == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return (result);
}

static void	modify_target(struct sockaddr_in *p, in_addr_t ip)
{
	p->sin_port = htons(0);
	p->sin_family = AF_INET;
	p->sin_addr.s_addr = ip;
}

static void	modify_package(struct icmphdr *icmp, int seq)
{
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid() & 0xFFFF);
	icmp->un.echo.sequence = htons(seq);
}

int	icmp(t_args *args, in_addr_t ip)
{
	int					tos = args->tos;
	int					icmp_socket;
	struct sockaddr_in	target;
	int					start = args->first_hop_num;
	int					finish = args->max_hop_num;
	int					times = args->packet_nums;
	char				packet[sizeof(struct icmphdr) + sizeof(struct icmp_payload)];
	char				buf[BUF_SIZE];
	int					timeout_time = args->wait;
	fd_set				readfds;
	struct timeval		tv;
	bool				resolve = args->resolve;

	// SOCKET SETTINGS
	icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (icmp_socket == -1)
	{
		if (errno == EPERM)
			printf("sudo yetkisi gerekir\n");
		perror("imcp soket sorunu: ");
		return (-1);
	}
	if (setsockopt(icmp_socket, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) == -1)
	{
		perror("tos sorunu");
		close(icmp_socket);
		return (-1);
	}

	// MODIFY TARGET
	modify_target(&target, ip);

	// INITIALIZE BUFFER
	memset(&buf, 0, BUF_SIZE);
	
	for (int ttl = start; ttl <= finish; ttl++)
	{
		printf("%d: ", ttl);

		// ICMP PACKAGE SETTINGS
		memset(packet, 0, sizeof(packet));
		struct icmphdr *icmp = (struct icmphdr *)packet;
		modify_package(icmp, SEQ + ttl);
		struct icmp_payload *payl = (struct icmp_payload *) ((char *) icmp + sizeof(struct icmphdr));
		gettimeofday(&payl->tv, NULL);
	
		/* Checksum (son adım) */
		icmp->checksum = 0;
		icmp->checksum = checksum(packet, sizeof(packet));

		if (setsockopt(icmp_socket, IPPROTO_IP, IP_TTL, &ttl, (socklen_t) sizeof(ttl)) == -1)
		{
			perror("ttl sorunu");
			close(icmp_socket);
			return (-1);
		}

		// PRINT IP
		int	print_ip = 0;

		for (int try = times; try > 0; try--)
		{
			// RTT START TIME
			struct timeval starttime;
			gettimeofday(&starttime, NULL);

			// PACKET SEND
			if (sendto(icmp_socket, packet, sizeof(packet), 0, (struct sockaddr *) &target, sizeof(target)) < 0)
			{
				close (icmp_socket);
				perror("sendto error");
				return (-1);
			}

			while (1)
			{
				// TIMEOUT
				tv.tv_sec = timeout_time;
				tv.tv_usec = 0;

				// SELECT INITIALIZE
				FD_ZERO(&readfds);
				FD_SET(icmp_socket, &readfds);

				int	ret = select(icmp_socket + 1, &readfds, NULL, NULL, &tv);
				if (ret == -1)
				{
					perror("select error");
					close(icmp_socket);
					return (-1);
				}
				else if (ret == 0)
				{
					printf(" * ");
					break ;
				}
				else if (ret && FD_ISSET(icmp_socket, &readfds))
				{
					int	reclen = recvfrom(icmp_socket, buf, sizeof(buf), 0, NULL, NULL);
					if (reclen < 0)
					{
						close (icmp_socket);
						perror("recvfrom error");
						return (-1);
					}

					// SIZE KONTROL.
					if (reclen < (int)sizeof(struct iphdr))
						break ;
					struct iphdr *ip_hdr = (struct iphdr *)buf;
					int ip_header_len = ip_hdr->ihl * 4;
					if (reclen < ip_header_len + (int)sizeof(struct icmphdr))
						break ;
					struct icmphdr *icmp_r = (struct icmphdr *)(buf + ip_header_len);

					// END TIME.
					struct timeval endtime;
					gettimeofday(&endtime, NULL);

					if (icmp_r->type == ICMP_TIME_EXCEEDED)
					{
						if (reclen < ip_header_len + (int)sizeof(struct icmphdr) + (int)sizeof(struct iphdr))
							break ;
						struct iphdr *inner_ip = (struct iphdr *)(buf + ip_header_len + sizeof(struct icmphdr));
						int inner_ip_len = inner_ip->ihl * 4;
						if ((long unsigned int ) reclen < ip_header_len + sizeof(struct icmphdr) + inner_ip_len + (int)sizeof(struct icmphdr))
							break ;
						struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + inner_ip_len);

						if (inner_icmp->un.echo.id == htons(getpid() & 0xFFFF)
							&& inner_icmp->un.echo.sequence == htons(SEQ + ttl))
						{
							struct in_addr src_ip;
							src_ip.s_addr = ip_hdr->saddr;

							if (!print_ip)
							{
								printf(" %s ", inet_ntoa(src_ip));
								if (resolve)
								{
									char	*rres = reverse_resolver(src_ip.s_addr);
									if (rres)
									{
										printf("(%s)", rres);
										free(rres);
									}
								}
								printf(" -> ");
							}
							print_rtt(&starttime, &endtime);
							printf(" ");
							print_ip++;
							break ;
						}
					}
					else if (icmp_r->type == ICMP_ECHOREPLY)
					{
						if (icmp_r->un.echo.id != htons(getpid() & 0xFFFF))
							break ;
						if (icmp_r->un.echo.sequence != htons(SEQ + ttl))
							break ;
						if (ip_hdr->saddr != target.sin_addr.s_addr)
							break ;

						g_flag = 1;
						struct in_addr src_ip;
						src_ip.s_addr = ip_hdr->saddr;

						if (!print_ip)
						{
							printf(" %s ", inet_ntoa(src_ip));
							if (resolve)
							{
								char	*rres = reverse_resolver(src_ip.s_addr);
								if (rres)
								{
									printf("(%s)", rres);
									free(rres);
								}
							}
							printf(" -> ");
						}
						print_rtt(&starttime, &endtime);
						printf(" ");
						print_ip++;
						break ;
					}
				}
			}
		}
		printf("\n");

		if (g_flag)
		{
			close(icmp_socket);
			return (0);
		}
	}
	close (icmp_socket);
	return (1);
}
