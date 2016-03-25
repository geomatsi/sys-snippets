#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

/* */

enum tool_type {
	MCAST_NOTSET = 0x0,
	MCAST_SERVER = 0x1,
	MCAST_CLIENT = 0x2,
};

/* */

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

/* */

void dump_packet(char *b, int n)
{
	int p;

	for(p = 0; p < n; p++) {
		printf("0x%02x ", *(b + p));
		if ((p > 0) && ((p % 64) == 0))
			printf("\n");
	}

	printf("\n");
}

/* */

int mcast_server(int sd, bool dump, struct sockaddr_in6 *saddr, struct ipv6_mreq *mreq)
{
	int faults = 0;
	int rc = 0;

	char buf[256];
	ssize_t len;

	struct timeval tv;
	fd_set rset;

	/* bind to mcast addr and port */

	if (bind(sd, (struct sockaddr *)saddr, sizeof(*saddr))) {
		perror("bind");
		return -1;
	}

	/* join mcast group */

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)mreq, sizeof(*mreq))) {
		perror("setsockopt IPV6_ADD_MEMBERSHIP");
		return -1;
	}

	/* server */

	while (faults < 5) {

		FD_ZERO(&rset);
		FD_SET(sd, &rset);

		tv.tv_sec = 10;
		tv.tv_usec = 0;

		rc = select(sd + 1, &rset, NULL, NULL, &tv);

		if (rc == 0) {
			/* timeout */
			continue;
		}

		if (rc < 0) {
			if (errno == EINTR) {
				/* select interrupted */
				faults++;
				continue;
			} else {
				perror("select");
				break;
			}
		}

		if (FD_ISSET(sd, &rset)) {

			len = read(sd, buf, sizeof(buf));
			buf[len] = '\0';

			if (len < 0) {
				perror("read");
				rc = len;
				break;
			} else if (len == 0) {
				printf("read zero bytes...\n");
				faults++;
				continue;
			} else {
				if (dump)
					dump_packet(buf, len);
				else
					printf("RECV: %s", buf);
			}

		} else {
			printf("unexpected select result...\n");
			faults++;
			continue;
		}
	}

	return rc;
}

/* */

int mcast_client(int sd, char *msg, bool cont, bool join, struct sockaddr_in6 *saddr, struct ipv6_mreq *mreq)
{
	int faults = 0;
	int rc = 0;
	char *pkt;
	int cnt = 0;

	/* join mcast group */

	if (join) {
		if (setsockopt(sd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char *)mreq, sizeof(*mreq))) {
			perror("setsockopt IPV6_ADD_MEMBERSHIP");
			return -1;
		}
	}

	/* client */

	pkt = calloc(1, strlen(msg) + 10);

	while (faults < 5) {
		memset(pkt, 0x0, strlen(msg) + 10);
		snprintf(pkt, strlen(msg) + 10, "%s:%d\n", msg, cnt++);
		rc = sendto(sd, pkt, strlen(pkt) + 1, 0, (const struct sockaddr *)saddr, sizeof(*saddr));
		if (rc < 0) {
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

	return rc;
}

/* */

int main(int argc, char *argv[])
{
	struct sockaddr_in6 *saddr, *maddr;
	struct ipv6_mreq *mreq;
	int sd;

	enum tool_type mtype = MCAST_NOTSET;
	bool cont = false;
	bool dump = false;
	bool join = true;

	char *message = NULL;
	char *ifname = NULL;
	int ifidx = 0;
	char *addr = NULL;
	int port = 9999;
	int hops = 255;
	int loop = 1;

	/* parse command line arguments */

	int opt;
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
				dump = true;
				break;
			case '1':
				hops = atoi(optarg);
				break;
			case '2':
				loop = 0;
				break;
			case '3':
				cont = true;
				break;
			case '4':
				join = false;
				break;
			case 'h':
			default:
				mcast_usage(argv[0]);
				exit(0);
		}
	}

	/* sanity check */

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

	/* */

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

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop))) {
		perror("setsockopt SO_REUSEADDR");
		exit(-1);
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifidx, sizeof(ifidx))) {
		perror("setsockopt IPV6_MULTICAST_IF");
		exit(-1);
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops))) {
		perror("setsockopt IPV6_MULTICAST_HOPS");
		exit(-1);
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop))) {
		perror("setsockopt IPV6_MULTICAST_LOOP");
		exit(-1);
	}

	/* setup saddr and mreq */

	saddr = calloc(1, sizeof(*saddr));
	if (!saddr) {
		printf("can't allocate saddr...\n");
		exit(-1);
	}

	saddr->sin6_family = AF_INET6;
	saddr->sin6_port = htons(port);
	inet_pton(AF_INET6, addr, &saddr->sin6_addr);

	maddr = calloc(1, sizeof(*maddr));
	if (!maddr) {
		printf("can't allocate maddr...\n");
		exit(-1);
	}

	maddr->sin6_family = AF_INET6;
	maddr->sin6_port = htons(port);
	inet_pton(AF_INET6, addr, &maddr->sin6_addr);

	mreq = calloc(1, sizeof(*mreq));
	if (!mreq) {
		printf("can't allocate mreq...\n");
		exit(-1);
	}

	memcpy(&mreq->ipv6mr_multiaddr, &maddr->sin6_addr, sizeof(mreq->ipv6mr_multiaddr));
	mreq->ipv6mr_interface = ifidx;

	/* */

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
