#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define MAXLEN 100
#define LISTEN 5

void chld_handler(int);

int main(int argc,char ** argv)
{

	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN+1];
	int listenfd,sockfd;
	int rc;
	pid_t pid;

	int len = sizeof(client);

	struct sigaction act;
	bzero(&act,sizeof(act));
	act.sa_handler = chld_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGCHLD,&act,NULL);


	if (0 > (listenfd = socket(AF_INET,SOCK_STREAM,0))) {
		perror(">>> socket");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (0 > bind(listenfd,(struct sockaddr *) &server,sizeof(server))) {
		perror(">>> bind");
		exit(1);
	}

	if (0 > listen(listenfd,LISTEN)) {
		perror(">>> listen");
		exit(1);
	}

	for(;;){
		if (0 > (sockfd = accept(listenfd,(struct sockaddr *) NULL,NULL))) {
			if (errno == EINTR) {
				perror(">>> 'accept' was interrupted, accept again");
				continue;
			} else {
				perror(">>> exit - some other problem ");
				exit(1);
			}
		}

		if (0 > (pid = fork())) {
			perror(">>> fork");
			exit(1);
		}

		if (0 == pid) {
			/* child -> handles request */
			close(listenfd);

			if (0 > getpeername(sockfd,(struct sockaddr*)&client,&len)) {
				perror(">>> getpeername");
				exit(1);
			} else {
				printf(">>> open connection from addr %s port %d\n",
					inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
			}

			if (0 > handle_echo(sockfd)) {
				perror(">>> problem");
				exit(1);
			}

			printf(">>> closing connection for addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
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

	for(;;){
		int j = 0;
		char c, * ptr = line;	

		bzero(line,sizeof(line));

		for(;;){

			if (0 > (j = read(sock,&c,1))) {
				perror(">>> read");
				return -1;
			}

			if (0 == j) {
				return 0;
			}

			if (j > 0) {
				*ptr++ = c;

				if (c == '\n') {
					break;
				}
			}
		}

		if (0 > write(sock,line,MAXLEN)) {
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

	while (0 < (pid = waitpid(-1,&stat,WNOHANG))) {
		printf(">>> child %d terminated\n",pid);

		if (WIFEXITED(stat)) {
			printf(">>\texit status %d\n",WEXITSTATUS(stat));
		}

		if (WIFSIGNALED(stat)) {
			printf(">>\tterminated by signal %d\n",WTERMSIG(stat));
		}
	}

	return;
}
