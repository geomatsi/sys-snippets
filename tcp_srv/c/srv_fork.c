// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define MAXLEN 100
#define LISTEN 5

static void chld_handler(int sig);
static int handle_echo(int sock);

int main(int argc, char **argv)
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN + 1];
	struct sigaction act;
	socklen_t len;
	int listenfd;
	int sockfd;
	pid_t pid;
	int ret;

	memset(&act, 0x0, sizeof(act));
	act.sa_handler = chld_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCHLD, &act, NULL);

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

	for (;;) {
		sockfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
		if (sockfd < 0) {
			if (errno == EINTR) {
				perror(">>> 'accept' was interrupted, accept again");
				continue;
			} else {
				perror(">>> exit - some other problem ");
				exit(1);
			}
		}

		pid = fork();
		if (pid < 0) {
			perror(">>> fork");
			exit(1);
		}

		if (pid == 0) {
			/* child -> handles request */
			close(listenfd);
			len = sizeof(client);

			ret = getpeername(sockfd, (struct sockaddr *)&client, &len);
			if (ret < 0) {
				perror(">>> getpeername");
				exit(1);
			} else {
				printf(">>> open connection from addr %s port %d\n",
					inet_ntop(AF_INET, &client.sin_addr, message,
						sizeof(message)),
					ntohs(client.sin_port));
			}

			ret = handle_echo(sockfd);
			if (ret < 0) {
				perror(">>> problem");
				exit(1);
			}

			printf(">>> closing connection for addr %s port %d\n",
				inet_ntop(AF_INET, &client.sin_addr, message, sizeof(message)),
				ntohs(client.sin_port));
			close(sockfd);
			exit(0);
		}

		/* parent */
		close(sockfd);
	}
}


int handle_echo(int sock)
{
	char line[MAXLEN];
	char *ptr;
	int ret;
	char c;

	for (;;) {

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
	}

	return 0;
}

void chld_handler(int signo)
{
	pid_t pid;
	int stat;

	while (0 < (pid = waitpid(-1, &stat, WNOHANG))) {
		printf(">>> child %d terminated\n", pid);

		if (WIFEXITED(stat))
			printf(">>exit status %d\n", WEXITSTATUS(stat));

		if (WIFSIGNALED(stat))
			printf(">>terminated by signal %d\n", WTERMSIG(stat));
	}
}
