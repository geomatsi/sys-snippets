// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define BUFSIZE 1024

int main(void)
{
	struct sockaddr_in client;
	struct sockaddr_in server;
	char buffer[BUFSIZE];
	int sock, ret, opt;
	socklen_t len;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror(">>> socket");
		exit(1);
	}

	opt = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, opt | O_NONBLOCK);

	memset(&server, 0x0, sizeof(server));
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(20001);
	server.sin_family = AF_INET;

	ret = bind(sock, (struct sockaddr *)&server, sizeof(server));
	if (ret < 0) {
		perror(">>> bind");
		exit(1);
	}

	while (1) {
		memset(buffer, 0x0, sizeof(buffer));
		memset(&client, 0x0, sizeof(client));

		ret = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&client, &len);
		if (ret < 0) {
			if (errno == EWOULDBLOCK) {
				fprintf(stdout, "non-blocking recvfrom: no data\n");
				sleep(1);
				continue;
			}

			perror(">>> recvfrom");
			exit(1);
		} else {
			fprintf(stdout, "Got %d bytes from (%s, %d) : %s\n",
				ret, inet_ntoa(client.sin_addr),
				ntohs(client.sin_port), buffer);

			fprintf(stdout, "Toggle blocking mode\n");
			opt = fcntl(sock, F_GETFL, 0);
			ret = fcntl(sock, F_SETFL, opt ^ O_NONBLOCK);
			if (ret < 0) {
				perror(">>> fcntl");
				exit(1);
			}
		}
	}
}
