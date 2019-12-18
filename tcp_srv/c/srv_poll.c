// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define MAXCLIENTS 3
#define MAXLEN 100
#define LISTEN 5

static int handle_echo(int sock);
static void int_handler(int);

int main(int argc, char **argv)
{
	struct pollfd clients[MAXCLIENTS];
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN + 1];
	int i, ret, max, count;
	struct sigaction act;
	socklen_t len;
	int listenfd;
	int sockfd;

	memset(&act, 0x0, sizeof(act));
	act.sa_handler = int_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror(">>> socket");
		exit(1);
	}

	memset(&server, 0x0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(listenfd, (struct sockaddr *) &server, sizeof(server));
	if (ret < 0) {
		perror(">>> bind");
		exit(1);
	}

	ret = listen(listenfd, LISTEN);
	if (ret < 0) {
		perror(">>> listen");
		exit(1);
	}

	clients[0].events = POLLIN;
	clients[0].fd = listenfd;

	for (i = 1; i < MAXCLIENTS; i++)
		clients[i].fd = -1;

	len = sizeof(client);

	for (;;) {
		max = 0;

		for (i = 1; i < MAXCLIENTS; i++)
			if (clients[i].fd != -1)
				max = i;

		count = poll(clients, max + 1, 5000);
		if (count == 0) {
			printf(">>> poll timeout\n");
			continue;
		}

		if (count == -1) {
			if (errno == EINTR) {
				perror(">>> 'poll' was interrupted, try again");
				continue;
			} else {
				perror(">>> poll");
				exit(1);
			}
		}

		if (clients[0].revents & POLLIN) {
			sockfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
			if (sockfd < 0) {
				if (errno == EINTR) {
					perror(">>> 'accept' was interrupted, accept again");
					continue;
				} else {
					perror(">>> accept");
					exit(1);
				}
			}

			for (i = 1; i < MAXCLIENTS; i++) {
				if (clients[i].fd == -1) {
					ret = getpeername(sockfd, (struct sockaddr *)&client, &len);
					if (ret < 0) {
						perror(">>> getpeername");
						exit(1);
					}

					printf(">>> open connection from addr %s port %d\n",
						inet_ntop(AF_INET, &client.sin_addr, message,
						sizeof(message)), ntohs(client.sin_port));
					clients[i].events = POLLIN;
					clients[i].fd = sockfd;
					break;
				}

				if (i == MAXCLIENTS - 1) {
					printf(">>> Clients queue is full - drop client\n");
					close(sockfd);
				}
			}

			if (--count == 0)
				continue;
		}

		for (i = 1; i < MAXCLIENTS; i++) {
			if (clients[i].fd == -1)
				continue;

			if (!(clients[i].revents & (POLLIN | POLLERR)))
				continue;

			ret = getpeername(clients[i].fd, (struct sockaddr *)&client, &len);
			if (ret < 0) {
				perror(">>> getpeername");
				exit(1);
			}

			ret = handle_echo(clients[i].fd);
			if (ret < 0) {
				perror(">>> problem with socket");
				printf(">>> closing this connection: addr %s port %d\n",
					inet_ntop(AF_INET, &client.sin_addr, message,
						sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i].fd);
				clients[i].fd = -1;
			}

			if (ret == 0) {
				printf(">>> client gone,closing connection: addr %s port %d\n",
					inet_ntop(AF_INET, &client.sin_addr, message,
						sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i].fd);
				clients[i].fd = -1;
			}

			if (ret > 0) {
				printf(">>> session  OK: addr %s port %d\n",
					inet_ntop(AF_INET, &client.sin_addr, message,
						sizeof(message)),
					ntohs(client.sin_port));
			}

			if (--count == 0)
				break;
		}
	}

	close(listenfd);
}

int handle_echo(int sock)
{
	char line[MAXLEN];
	char *ptr;
	int ret;
	char c;

	memset(line, 0x0, sizeof(line));
	ptr = line;

	for (;;) {
		ret = read(sock, &c, 1);
		if (ret < 0) {
			perror(">>> read");
			return -1;
		}

		if (ret == 0)
			return 0;

		if (ret > 0) {
			*ptr++ = c;
			if (c == '\n')
				break;
		}
	}

	ret = write(sock, line, MAXLEN);
	if (ret < 0) {
		perror(">>> write");
		return -1;
	}

	return 1;
}

void int_handler(int sig)
{
	printf(">>>SIGINT\n");
	exit(0);
}
