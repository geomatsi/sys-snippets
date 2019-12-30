// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <arpa/inet.h>
//#include <netinet/in.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
//#include <fcntl.h>

enum tool_type {
	MCAST_NOTSET = 0x0,
	MCAST_SERVER = 0x1,
	MCAST_CLIENT = 0x2,
};

void mcast_usage(char *name)
{
	printf("Usage: %s [-h] -i <interface> -a <address> -p <port>\n", name);
	printf("\t-h, --help\t\t\tthis help message\n");
	printf("\t-c, --client\t\t\tmulticast test client\n");
	printf("\t-s, --server\t\t\tmulticast test server\n");
	printf("\t-m, --message\t\t\tmessage for multicast client\n");
	printf("\t-i, --intf <interface>\t\tbind to network interface\n");
	printf("\t-a, --addr <address>\t\tmulticast address\n");
	printf("\t-p, --port <port>\t\tport, default is 9999\n");
	printf("\t--hops = <hops>\t\t\tmulticast packet lifetime\n");
	printf("\t--noloop\t\t\tdisable loopback for outgoing multicast diagrams\n");
	printf("\t--nojoin\t\t\tprevent client from joining multicast group\n");
	printf("\t--cont\t\t\tmulticast client continous mode: no need to press enter to send next packet\n");
	printf("\t--hexdump\t\t\thexdump packet mode for multicast server\n");
	printf("\nMinimal client example: %s -c -i eth0 -a ff02::5:6 -p 12345 -m 'test message'\n\n", name);
	printf("\nMinimal server example: %s -s -i eth0 -a ff02::5:6 -p 12345\n\n", name);
}

void dump_packet(char *b, int n)
{
	int p;

	for (p = 0; p < n; p++) {
		printf("0x%02x ", *(b + p));
		if ((p > 0) && ((p % 64) == 0))
			printf("\n");
	}

	printf("\n");
}

int mcast_server(int sd, int dump, struct sockaddr_in6 *saddr, struct ipv6_mreq *mreq)
{
	struct timeval tv;
	char buffer[256];
	int faults = 0;
	ssize_t len;
	fd_set rset;
	int ret;

	ret = bind(sd, (struct sockaddr *)saddr, sizeof(*saddr));
	if (ret < 0) {
		perror("bind");
		return -1;
	}

	ret = setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)mreq, sizeof(*mreq));
	if (ret < 0) {
		perror("setsockopt IPV6_ADD_MEMBERSHIP");
		return -1;
	}

	while (faults < 5) {
		FD_ZERO(&rset);
		FD_SET(sd, &rset);
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		ret = select(sd + 1, &rset, NULL, NULL, &tv);
		if (ret == 0) {
			/* timeout */
			continue;
		}

		if (ret < 0) {
			if (errno == EINTR) {
				/* select interrupted */
				faults++;
				continue;
			}

			perror("select");
			break;
		}

		if (FD_ISSET(sd, &rset)) {
			len = read(sd, buffer, sizeof(buffer));
			buffer[len] = '\0';

			if (len < 0) {
				perror("read");
				ret = len;
				break;
			} else if (len == 0) {
				printf("read zero bytes...\n");
				faults++;
				continue;
			} else {
				if (dump)
					dump_packet(buffer, len);
				else
					printf("RECV: %s", buffer);
			}
		} else {
			printf("unexpected select result...\n");
			faults++;
			continue;
		}
	}

	return ret;
}

int mcast_client(int sd, char *msg, int cont, int join,
		 struct sockaddr_in6 *saddr, struct ipv6_mreq *mreq)
{
	int faults = 0;
	int cnt = 0;
	char *pkt;
	int ret;

	if (join) {
		ret = setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
				 (char *)mreq, sizeof(*mreq));
		if (ret < 0) {
			perror("setsockopt IPV6_ADD_MEMBERSHIP");
			return -1;
		}
	}

	pkt = calloc(1, strlen(msg) + 10);

	while (faults < 5) {
		memset(pkt, 0x0, strlen(msg) + 10);
		snprintf(pkt, strlen(msg) + 10, "%s:%d\n", msg, cnt++);
		ret = sendto(sd, pkt, strlen(pkt) + 1, 0,
			     (const struct sockaddr *)saddr, sizeof(*saddr));
		if (ret < 0) {
			perror("sendto");
			faults++;
		}

		if (cont) {
			sleep(1);
		} else {
			printf("press enter to send next packet...\n");
			getchar();
		}
	}

	free(pkt);
	return ret;
}

int main(int argc, char *argv[])
{
	enum tool_type mtype = MCAST_NOTSET;
	struct sockaddr_in6 *saddr, *maddr;
	struct ipv6_mreq *mreq;
	char *message = NULL;
	char *ifname = NULL;
	char *addr = NULL;
	int port = 9999;
	int hops = 255;
	int ifidx = 0;
	int loop = 1;
	int cont = 0;
	int dump = 0;
	int join = 1;
	int ret;
	int opt;
	int sd;

	const char opts[] = "a:p:i:m:sch";
	const struct option longopts[] = {
		{"help", no_argument, NULL, 'h'},
		{"addr", required_argument, NULL, 'a'},
		{"port", required_argument, NULL, 'p'},
		{"intf", required_argument, NULL, 'i'},
		{"client", no_argument, NULL, 'c'},
		{"server", no_argument, NULL, 's'},
		{"message", required_argument, NULL, 'm'},
		{"hexdump", no_argument, NULL, '0'},
		{"hops", required_argument, NULL, '1'},
		{"noloop", no_argument, NULL, '2'},
		{"cont", no_argument, NULL, '3'},
		{"nojoin", no_argument, NULL, '4'},
		{NULL,}
	};

	while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
		switch (opt) {
		case 'c':
			mtype = MCAST_CLIENT;
			break;
		case 's':
			mtype = MCAST_SERVER;
			break;
		case 'a':
			addr = strdup(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'i':
			ifname = strdup(optarg);
			break;
		case 'm':
			message = strdup(optarg);
			break;
		case '0':
			dump = 1;
			break;
		case '1':
			hops = atoi(optarg);
			break;
		case '2':
			loop = 0;
			break;
		case '3':
			cont = 1;
			break;
		case '4':
			join = 0;
			break;
		case 'h':
		default:
			mcast_usage(argv[0]);
			exit(0);
		}
	}

	switch (mtype) {
	case MCAST_SERVER:
		if (!port || !addr || !ifname) {
			mcast_usage(argv[0]);
			exit(0);
		}

		printf("server: addr[%s] ifname[%s] port[%d] dump[%d]\n",
		       addr, ifname, port, dump);
		break;
	case MCAST_CLIENT:
		if (!port || !addr || !ifname || !message) {
			mcast_usage(argv[0]);
			exit(0);
		}

		printf("client: addr[%s] ifname[%s] port[%d] hops[%d] loop[%d] message[%s] cont[%d] join[%d]\n",
		       addr, ifname, port, hops, loop, message, cont, join);
		break;
	default:
		printf("should be client or server...\n");
		mcast_usage(argv[0]);
		exit(0);
		break;

	}

	sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		perror("socket");
		exit(-1);
	}

	ifidx = if_nametoindex(ifname);
	if (ifidx == 0) {
		perror("if_nametoindex");
		exit(-1);
	}

	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop));
	if (ret < 0) {
		perror("setsockopt SO_REUSEADDR");
		exit(-1);
	}

	ret = setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifidx, sizeof(ifidx));
	if (ret < 0) {
		perror("setsockopt IPV6_MULTICAST_IF");
		exit(-1);
	}

	ret = setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops));
	if (ret < 0) {
		perror("setsockopt IPV6_MULTICAST_HOPS");
		exit(-1);
	}

	ret = setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop));
	if (ret < 0) {
		perror("setsockopt IPV6_MULTICAST_LOOP");
		exit(-1);
	}

	saddr = calloc(1, sizeof(*saddr));
	if (!saddr) {
		perror("calloc saddr");
		exit(-1);
	}

	saddr->sin6_family = AF_INET6;
	saddr->sin6_port = htons(port);
	inet_pton(AF_INET6, addr, &saddr->sin6_addr);

	maddr = calloc(1, sizeof(*maddr));
	if (!maddr) {
		perror("calloc maddr");
		exit(-1);
	}

	maddr->sin6_family = AF_INET6;
	maddr->sin6_port = htons(port);
	inet_pton(AF_INET6, addr, &maddr->sin6_addr);

	mreq = calloc(1, sizeof(*mreq));
	if (!mreq) {
		perror("calloc mreq");
		exit(-1);
	}

	memcpy(&mreq->ipv6mr_multiaddr, &maddr->sin6_addr, sizeof(mreq->ipv6mr_multiaddr));
	mreq->ipv6mr_interface = ifidx;

	switch (mtype) {
	case MCAST_SERVER:
		mcast_server(sd, dump, saddr, mreq);
		break;
	case MCAST_CLIENT:
		mcast_client(sd, message, cont, join, maddr, mreq);
		break;
	default:
		printf("undefined tool type...\n");
		break;

	}

	free(saddr);
	free(maddr);
	free(mreq);
	close(sd);

	return 0;
}
