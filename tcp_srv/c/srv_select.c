// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define LISTEN 5
#define MAXLEN 100
#define MAXCLIENTS 2

static int handle_echo(int sock);
static void int_handler(int);

int main(int argc, char **argv)
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN + 1];
	int clients[MAXCLIENTS];
	struct sigaction act;
	struct timeval tv;
	socklen_t len;
	int listenfd;
	fd_set rset;
	int sockfd;
	int ret;
	int max;
	int i;

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

	len = sizeof(client);

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

	for (i = 0; i < MAXCLIENTS; i++)
		clients[i] = -1;

	for (;;) {
		FD_ZERO(&rset);
		FD_SET(listenfd, &rset);
		max = listenfd;

		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i] == -1)
				continue;

			FD_SET(clients[i], &rset);

			if (max < clients[i])
				max = clients[i];
		}

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		ret = select(max + 1, &rset, NULL, NULL, &tv);

		if (ret == 0) {
			printf(">>> select timeout\n");
			continue;
		}

		if (ret == -1) {
			if (errno == EINTR) {
				perror(">>> 'select' was interrupted, try again");
				continue;
			} else {
				perror(">>> select");
				exit(1);
			}
		}

		if (FD_ISSET(listenfd, &rset)) {
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

			for (i = 0; i < MAXCLIENTS; i++) {
				if (clients[i] == -1) {
					ret = getpeername(sockfd, (struct sockaddr *)&client, &len);
					if (ret < 0) {
						perror(">>> getpeername");
						exit(1);
					} else {
						printf(">>> open connection from addr %s port %d\n",
							inet_ntop(AF_INET, &client.sin_addr,
								message, sizeof(message)),
							ntohs(client.sin_port));
					}

					clients[i] = sockfd;
					break;
				}

				if (i == MAXCLIENTS - 1) {
					printf(">>> Clients queue is full - drop client\n");
					close(sockfd);
				}
			}
		}

		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i] == -1)
				continue;

			if (!FD_ISSET(clients[i], &rset))
				continue;

			ret = getpeername(clients[i], (struct sockaddr *)&client, &len);
			if (ret < 0) {
				perror(">>> getpeername");
				exit(1);
			}

			ret = handle_echo(clients[i]);
			if (ret < 0) {
				printf(">>> closing this connection: addr %s port %d\n",
				inet_ntop(AF_INET, &client.sin_addr, message, sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i]);
				clients[i] = -1;
			}

			if (ret == 0) {
				printf(">>> client gone,closing connection: addr %s port %d\n",
				inet_ntop(AF_INET, &client.sin_addr, message, sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i]);
				clients[i] = -1;
			}

			if (ret > 0) {
				printf(">>> session  OK: addr %s port %d\n",
				inet_ntop(AF_INET, &client.sin_addr, message, sizeof(message)),
					ntohs(client.sin_port));
			}
		}
	}

	close(listenfd);
}

int handle_echo(int sock)
{
	char line[MAXLEN];
	char *ptr;
	char c;
	int ret;

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
	printf(">>> SIGINT\n");
	exit(0);
}
