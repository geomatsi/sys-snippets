// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUFSIZE 1024

int main(void)
{
	struct sockaddr_in dest;
	char buf[BUFSIZE];
	int sock, ret;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror(">>> socket");
		exit(1);
	}

	memset(&dest, 0x0, sizeof(dest));
	dest.sin_addr.s_addr = inet_addr("127.0.0.1");
	dest.sin_port = htons(20001);
	dest.sin_family = AF_INET;

	while (1) {
		memset(buf, 0x0, sizeof(buf));
		if (!fgets(buf, sizeof(buf) - 1, stdin))
			break;

		ret = sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *) &dest, sizeof(dest));
		if (ret < 0) {
			perror(">>> sendto");
			exit(1);
		}
	}

	close(sock);
	return 0;
}
