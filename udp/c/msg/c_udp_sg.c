// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 1024

static void usage(const char *name);

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct in_addr addr;
	struct iovec iov[1];
	struct msghdr msg;
	char buf[BUFSIZE];
	int sock, ret;

	if (argc != 2)
		usage(argv[0]);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror(">>> socket");
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

	while (1) {
		memset(buf, 0x0, sizeof(buf));
		memset(&msg, 0x0, sizeof(msg));

		if (!fgets(buf, sizeof(buf) - 1, stdin))
			break;

		iov[0].iov_base = buf;
		iov[0].iov_len = strlen(buf);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		msg.msg_name = (void *)&server;
		msg.msg_namelen = sizeof(server);
		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		ret = sendmsg(sock, &msg, 0);
		if (ret < 0) {
			perror(">>> recvmsg");
			exit(1);
		}
	}

	close(sock);
}

void usage(const char *name)
{
	printf("use: %s <ip addr>\n", name);
	exit(0);
}
