#include "traceroute.h"

extern bool g_flag;

static void	modify_target(struct sockaddr_in *p, in_addr_t ip, int port)
{
	p->sin_port = htons(port);
	p->sin_family = AF_INET;
	p->sin_addr.s_addr = ip;
}

int	udp(t_args *args, in_addr_t ip)
{
	int					tos = args->tos;
	int					udp_socket;
	struct sockaddr_in	target;
	int					start = args->first_hop_num;
	int					finish = args->max_hop_num;
	int					times = args->packet_nums;
	int					timeout_time = args->wait;
	fd_set				readfds;
	struct timeval		tv;
	bool				resolve = args->resolve;
	int					port = args->port;
	char				packet[sizeof(tv)];

	// SOCKET SETTINGS
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udp_socket == -1)
	{
		if (errno == EPERM)
			printf("sudo yetkisi gerekir\n");
		perror("udp soket sorunu: ");
		return (-1);
	}

	if (setsockopt(udp_socket, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) == -1)
	{
		perror("tos sorunu");
		close(udp_socket);
		return (-1);
	}

	const int	recverrval = 1;
	if (setsockopt(udp_socket, IPPROTO_IP, IP_RECVERR, &recverrval, sizeof(recverrval)) == -1)
	{
		perror("recverror sorunu");
		close(udp_socket);
		return (-1);
	}

	for (int ttl = start; ttl <= finish; ttl++)
	{
		// MODIFY TARGET
		modify_target(&target, ip, port + ttl);

		// ttl number
		printf("%d: ", ttl);

		// UDP PACKAGE PAYLOAD
		gettimeofday(&tv, NULL);
		memcpy(packet, &tv, sizeof(tv));

		if (setsockopt(udp_socket, IPPROTO_IP, IP_TTL, &ttl, (socklen_t) sizeof(ttl)) == -1)
		{
			perror("ttl sorunu");
			close(udp_socket);
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
			if (sendto(udp_socket, packet, sizeof(packet), 0, (struct sockaddr *) &target, sizeof(target)) < 0)
			{
				close (udp_socket);
				perror("sendto error");
				return (-1);
			}

			// TIMEOUT
			tv.tv_sec = timeout_time;
			tv.tv_usec = 0;

			// SELECT INITIALIZE
			FD_ZERO(&readfds);
			FD_SET(udp_socket, &readfds);

			int	ret = select(udp_socket + 1, &readfds, NULL, NULL, &tv);
			if (ret == -1)
			{
				perror("select error");
				close(udp_socket);
				return (-1);
			}
			else if (ret == 0)
			{
				printf(" * ");
			}
			else if (ret && FD_ISSET(udp_socket, &readfds))
			{
				// MESSAGE HEADER
				struct msghdr		msg;
				char				control_buf[BUF_SIZE];
				struct iovec		iov;
				char				dummy_buf[1024];

				// IOV for recv
				iov.iov_base = dummy_buf;
				iov.iov_len = sizeof(dummy_buf);
				msg.msg_iov = &iov;
				msg.msg_iovlen = 1;

				// BUFFER SETTINGS
				memset(&msg, 0, sizeof(msg));
				msg.msg_control = control_buf;
				msg.msg_controllen = sizeof(control_buf);

				int	reclen = recvmsg(udp_socket, &msg, MSG_ERRQUEUE);
				if (reclen < 0)
				{
					close (udp_socket);
					perror("recvmsg error");
					return (-1);
				}

				// END TIME
				struct timeval	endtime;
				gettimeofday(&endtime, NULL);

				struct cmsghdr 				*cmsg = CMSG_FIRSTHDR(&msg);
				if (!cmsg)
					continue ;
				struct sock_extended_err	*ee = (struct sock_extended_err *)CMSG_DATA(cmsg);
				struct sockaddr_in			*rtr_exceed;

				if (ee->ee_origin == SO_EE_ORIGIN_ICMP)
				{
					if (ee->ee_type == ICMP_TIME_EXCEEDED)
					{
						if (!print_ip)
						{
							rtr_exceed = (struct sockaddr_in *)SO_EE_OFFENDER(ee);
							printf(" %s ", inet_ntoa(rtr_exceed->sin_addr));
							if (resolve)
							{
								char	*rres = reverse_resolver(rtr_exceed->sin_addr.s_addr);
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
					}
					else if (ee->ee_type == ICMP_DEST_UNREACH && ee->ee_code == ICMP_PORT_UNREACH)
					{
						g_flag = 1;

						if (!print_ip)
						{
							rtr_exceed = (struct sockaddr_in *)SO_EE_OFFENDER(ee);
							printf(" %s ", inet_ntoa(rtr_exceed->sin_addr));
							if (resolve)
							{
								char	*rres = reverse_resolver(rtr_exceed->sin_addr.s_addr);
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
					}
				}
			}
		}
		printf("\n");

		if (g_flag)
		{
			close(udp_socket);
			return (0);
		}
	}
	close (udp_socket);
	return (1);
}
