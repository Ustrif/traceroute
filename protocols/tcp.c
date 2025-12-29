#include "traceroute.h"

//#include <linux/tcp.h> // KALDIR.

/*
GELEN PAKET İNCELEMESİ ŞART.

*/
extern bool g_flag;

struct pseudo_header
{
	uint32_t	source_address;
	uint32_t	dest_address;
	uint8_t		placeholder;
	uint8_t		protocol;
	uint16_t	tcp_length;
};

static void	modify_target(struct sockaddr_in *p, in_addr_t ip, int port)
{
	p->sin_port = htons(port);
	p->sin_family = AF_INET;
	p->sin_addr.s_addr = ip;
}

static uint32_t get_local_ip()
{
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return (0);

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr("8.8.8.8");
	serv.sin_port = htons(53);

	connect(sock, (const struct sockaddr*)&serv, sizeof(serv));

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	getsockname(sock, (struct sockaddr*)&name, &namelen);

	close(sock);
	return (name.sin_addr.s_addr);
}

static void compute_tcp_checksum(struct iphdr *ip, struct tcphdr *tcp, struct timeval *tv)
{
	struct pseudo_header psh;

	const size_t payload_len = sizeof(struct timeval);
	const size_t tcp_len = sizeof(struct tcphdr) + payload_len;
	const size_t psize = sizeof(struct pseudo_header) + tcp_len;

	psh.source_address = ip->saddr;
	psh.dest_address = ip->daddr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons((uint16_t)tcp_len);

	unsigned char *pseudogram = malloc(psize);
	if (!pseudogram)
	{
		perror("compute_tcp_checksum: malloc failed");
		return ;
	}

	memset(pseudogram, 0, psize);
	memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcp, sizeof(struct tcphdr));
    if (tv)
	{
		memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct tcphdr), tv, payload_len);
	}
	tcp->check = checksum((unsigned short *)pseudogram, (int)psize);
	free(pseudogram);
}

int	tcp(t_args *args, in_addr_t ip)
{
	int					tos = args->tos;
	int					tcp_socket;
	int					icmp_socket;
	struct sockaddr_in	target;
	int					start = args->first_hop_num;
	int					finish = args->max_hop_num;
	int					times = args->packet_nums;
	int					timeout_time = args->wait;
	fd_set				readfds;
	struct timeval		tv;
	char				packet[sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(tv)];
	bool				resolve = args->resolve;
	int					port = args->port;
	in_addr_t			localip;
	char				buf[BUF_SIZE];

	// LOCAL IP
	localip = get_local_ip();
	if (!localip)
	{
		printf("no local ip\n");
		return (-1);
	}
	
	// SOCKET SETTINGS
	icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (icmp_socket == -1)
	{
		if (errno == EPERM)
			printf("sudo yetkisi gerekir\n");
		perror("imcp soket sorunu: ");
		return (-1);
	}
	tcp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (tcp_socket == -1)
	{
		if (errno == EPERM)
			printf("sudo yetkisi gerekir\n");
		perror("imcp soket sorunu: ");
		close(icmp_socket);
		return (-1);
	}

	const int one = 1;
	if (setsockopt(tcp_socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) == -1)
	{
		perror("IP-HDRINC sorunu");
		close(tcp_socket);
		close(icmp_socket);
		return (-1);
	} 

	// MODIFY TARGET
	modify_target(&target, ip, port);
	
	for (int ttl = start; ttl <= finish; ttl++)
	{
		printf("%d: ", ttl);

		// IP Packet Settings
		memset(packet, 0, sizeof(packet));
		struct iphdr *ip_package = (struct iphdr *) packet;
		ip_package->ttl = ttl;
		ip_package->tos = tos & 0xFF;
		ip_package->frag_off = 0;
		ip_package->version = 4;
		ip_package->tot_len = htons(sizeof(packet));
		ip_package->ihl = 5;
		ip_package->id = htons(ttl);
		ip_package->protocol = IPPROTO_TCP;
		ip_package->check = 0;
		ip_package->daddr = ip;
		ip_package->saddr = localip;

		// CHECKSUM
		ip_package->check = checksum((unsigned short *)packet, ip_package->ihl * 4);

		// TCP Packet Settings
		struct tcphdr *tcp_package = (struct tcphdr *)(packet + sizeof(struct iphdr));
		tcp_package->source = htons(SEQ + ttl);
		tcp_package->dest = htons(port);
		tcp_package->seq = htonl(ttl);
		tcp_package->ack_seq = 0;
		tcp_package->doff = 5;

		// FLAGS
		tcp_package->syn = 1;
		tcp_package->fin = 0;
		tcp_package->rst = 0;
		tcp_package->psh = 0;
		tcp_package->ack = 0;
		tcp_package->urg = 0;

		tcp_package->window = htons(BUF_SIZE);
		tcp_package->check = 0;

		// CHECKSUM
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		compute_tcp_checksum(ip_package, tcp_package, &tv);

		// PRINT IP
		int	print_ip = 0;

		for (int try = times; try > 0; try--)
		{
			// RTT START TIME
			struct timeval starttime;
			gettimeofday(&starttime, NULL);

			// PACKET SEND
			if (sendto(tcp_socket, packet, sizeof(packet), 0, (struct sockaddr *) &target, sizeof(target)) < 0)
			{
				close (tcp_socket);
				close(icmp_socket);
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
				FD_SET(tcp_socket, &readfds);
				FD_SET(icmp_socket, &readfds);

				// BIG INT FOR SELECT
				int	big_socket_num = tcp_socket > icmp_socket ? tcp_socket : icmp_socket;

				int	ret = select(big_socket_num + 1, &readfds, NULL, NULL, &tv);
				if (ret == -1)
				{
					perror("select error");
					close(tcp_socket);
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
						close (tcp_socket);
						close (icmp_socket);
						perror("recvfrom error in icmp");
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
						if ((long unsigned int ) reclen < ip_header_len + sizeof(struct icmphdr) + inner_ip_len + (int)sizeof(struct tcphdr))
							break ;
						struct tcphdr *inner_tcp = (struct tcphdr *)((char *)inner_ip + inner_ip_len);

						if (ip_hdr->id == ip_package->id
							|| inner_tcp->source == tcp_package->source)
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
				}
				else if (ret && FD_ISSET(tcp_socket, &readfds))
				{
					int	reclen = recvfrom(tcp_socket, buf, sizeof(buf), 0, NULL, NULL);
					if (reclen < 0)
					{
						close (tcp_socket);
						close (icmp_socket);
						perror("recvfrom error in tcp");
						return (-1);
					}

					// SIZE KONTROL.
					if (reclen < (int)sizeof(struct iphdr))
						break ;
					struct iphdr *ip_hdr = (struct iphdr *)buf;
					int ip_header_len = ip_hdr->ihl * 4;
					if (reclen < ip_header_len + (int)sizeof(struct tcphdr))
						break ;
					struct tcphdr *tcp_r = (struct tcphdr *)(buf + ip_header_len);

					// END TIME.
					struct timeval endtime;
					gettimeofday(&endtime, NULL);

					if (tcp_r->rst || tcp_r->ack)
					{
						if (ip_hdr->id == ip_package->id
							|| tcp_r->source == tcp_package->source)
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
				}
			}
		}
		printf("\n");

		if (g_flag)
		{
			close(tcp_socket);
			close(icmp_socket);
			return (0);
		}
	}
	close (tcp_socket);
	close (icmp_socket);
	return (1);
}
