// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define BUFSIZE 1024

static void usage(const char *name);

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	struct cmsghdr *cmptr;
	struct in_pktinfo *pkt;
	int sock, ret, optval;
	struct in_addr addr;
	struct iovec iov[1];
	struct msghdr msg;
	char buffer[BUFSIZE];
	char ctl[BUFSIZE];

	if (argc != 2)
		usage(argv[0]);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror(">>> socket");
		exit(1);
	}

	/* option makes recvmsg return control messages */
	optval = 1;
	ret = setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(optval));
	if (ret < 0) {
		perror(">>> setsockopt");
		exit(1);
	}

	ret = inet_pton(AF_INET, argv[1], &addr);
	if (ret <= 0) {
		if (ret == 0)
			usage(argv[0]);

		perror(">>> inet_pton");
		close(sock);
		exit(1);
	}

	memset(&server, 0x0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr = addr;
	server.sin_port = htons(20001);

	ret = bind(sock, (struct sockaddr *) &server, sizeof(server));
	if (ret < 0) {
		perror(">>> bind");
		exit(1);
	}

	while (1) {
		memset(&client, 0x0, sizeof(client));
		memset(buffer, 0x0, sizeof(buffer));
		memset(&msg, 0x0, sizeof(msg));
		memset(&ctl, 0x0, sizeof(ctl));

		/* buffer for received packet */
		iov[0].iov_base = buffer;
		iov[0].iov_len = sizeof(buffer);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		/* buffer for packet source address */
		msg.msg_name = (struct sockaddr *) &client;
		msg.msg_namelen = sizeof(client);

		/* buffer for control messages */
		msg.msg_control = (void *) &ctl;
		msg.msg_controllen = sizeof(ctl);
		msg.msg_flags = 0;

		ret = recvmsg(sock, &msg, 0);
		if (ret < 0) {
			perror(">>> recvmsg");
			exit(1);
		}

		if (msg.msg_flags & MSG_TRUNC)
			fprintf(stdout, "diagram was truncated\n");

		if (msg.msg_flags & MSG_CTRUNC)
			fprintf(stdout, "control data was truncated\n");

		/* process control messages */
		for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
			if (IPPROTO_IP == cmptr->cmsg_level && IP_PKTINFO == cmptr->cmsg_type) {
				pkt = (struct in_pktinfo *) CMSG_DATA(cmptr);
				fprintf(stdout, "ifindex[%d]  local_addr[%s]  dest_addr[%s]\n",
						pkt->ipi_ifindex, inet_ntoa(pkt->ipi_spec_dst),
						inet_ntoa(pkt->ipi_addr));
			} else {
				fprintf(stdout, "ctl msg of level %d and type %d\n",
						cmptr->cmsg_level, cmptr->cmsg_type);
			}
		}

		fprintf(stdout, "got %d bytes from (%s, %d) : %s\n",
			ret, inet_ntoa(client.sin_addr), ntohs(client.sin_port), buffer);
	}
}

void usage(const char *name)
{
	printf("use: %s <ip addr>\n", name);
	exit(0);
}
