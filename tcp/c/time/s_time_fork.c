// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define MAXLEN 100
#define LISTEN 5

int main(void)
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN + 1];
	socklen_t len;
	time_t ticks;
	int listenfd;
	int sockfd;
	pid_t pid;
	int ret;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror(">>> socket");
		exit(1);
	}

	memset(&server, 0x0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(5555);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(listenfd, (struct sockaddr *)&server, sizeof(server));
	if (ret < 0) {
		perror(">>> bind");
		exit(1);
	}

	ret = listen(listenfd, LISTEN);
	if (ret < 0) {
		perror(">>> listen");
		exit(1);
	}

	for (;;) {
		sockfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
		if (sockfd < 0) {
			perror(">>> accept");
			exit(1);
		}

		pid = fork();
		if (pid < 0) {
			perror(">>> fork");
			exit(1);
		}

		if (pid == 0) {
			len = sizeof(client);
			ret = getpeername(sockfd, (struct sockaddr *)&client, &len);
			if (ret < 0) {
				perror(">>> getpeername");
				exit(1);
			}

			printf("connection from addr %s port %d\n",
				inet_ntop(AF_INET, &client.sin_addr, message, sizeof(message)),
				ntohs(client.sin_port));

			ticks = time(NULL);
			snprintf(message, sizeof(message), "%.24s\er\en", ctime(&ticks));
			ret = write(sockfd, message, strlen(message));
			if (ret < 0) {
				perror(">>> write");
				exit(-1);
			}

			printf("reported time: %s\n", message);
			close(sockfd);
			exit(0);
		}

		close(sockfd);
	}
}
