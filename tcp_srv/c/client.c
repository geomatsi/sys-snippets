// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define MAXLEN 100

static void pipe_handler(int sig);
static void usr_handler(int sig);
static int echo_handler(int sockfd);

int main(int argc, char **argv)
{
	struct sockaddr_in server;
	struct sockaddr_in local;
	char message[MAXLEN + 1];
	socklen_t len;
	int sockfd;
	int ret;

	signal(SIGPIPE, pipe_handler);
	signal(SIGUSR1, usr_handler);

	if (argc != 2) {
		printf("Usage: %s <ip>\n", argv[0]);
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror(">>> socket");
		exit(1);
	}

	memset(&server, 0x0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);

	len = sizeof(local);

	ret = inet_pton(AF_INET, argv[1], &server.sin_addr);
	if (ret <= 0) {
		perror(">>> inet_pton");
		exit(1);
	}

	ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
	if (ret < 0) {
		perror(">>> connect");
		exit(1);
	}

	ret = getsockname(sockfd, (struct sockaddr *)&local, &len);
	if (ret < 0) {
		perror(">>> getpeername");
		exit(1);
	}

	printf(">>> Local socket:  ADDR %s PORT %d\n",
			inet_ntop(AF_INET, &local.sin_addr, message, sizeof(message)),
			ntohs(local.sin_port));

	ret = echo_handler(sockfd);
	if (ret < 0) {
		perror(">>> echo_handler");
		exit(1);
	}

	close(sockfd);
	exit(0);
}

int echo_handler(int sockfd)
{
	char send[MAXLEN];
	char recv[MAXLEN];
	fd_set rset;
	int ret;

	while (1) {
		memset(send, 0x0, sizeof(send));
		memset(recv, 0x0, sizeof(recv));

		FD_SET(0, &rset);
		FD_SET(sockfd, &rset);

		ret = select(sockfd + 1, &rset, NULL, NULL, NULL);
		if (ret < 0) {
			if (errno == EINTR) {
				perror(">>> 'accept' was interrupted, accept again");
				continue;
			} else {
				perror(">>> select");
				exit(1);
			}
		}

		if (FD_ISSET(0, &rset)) {
			fgets(send, sizeof(send) - 1, stdin);

			// use CTRL-D to exit
			if (strlen(send) == 0)
				return 0;

			ret = write(sockfd, send, strlen(send));
			if (ret < 0)
				return -1;
		}

		if (FD_ISSET(sockfd, &rset)) {
			ret = read(sockfd, recv, MAXLEN);
			if (ret < 0)
				return -1;

			if (ret == 0) {
				printf(">>> server terminated prematurely\n");
				return 0;
			}

			printf(">>> reading from server: %s", recv);
		}
	}
}

void usr_handler(int sig)
{
	printf(">>> got SIGUSR1\n");
}

void pipe_handler(int sig)
{
	printf(">>> got SIGPIPE\n");
	exit(1);
}
