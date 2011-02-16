#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAXLEN 100

int main(int argc,char ** argv)
{
	int sockfd;
	char message[MAXLEN+1];
	struct sockaddr_in server,local;
	int len = sizeof(local);

	if (argc != 2) {
		printf("Usage: %s <ip> \n",argv[0]);
		exit(0);
	}

	if (0 > (sockfd = socket(AF_INET,SOCK_STREAM,0))) {
		perror(">>> socket");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_port = htons(5555);
	server.sin_family = AF_INET;

	if (0 >= inet_pton(AF_INET,argv[1],&server.sin_addr)) {
		perror(">>> inet_pton");
		exit(1);
	}

	if (0 > connect(sockfd,(struct sockaddr *) &server,sizeof(server))) {
		perror(">>> connect");
		exit(1);
	}

	if (0 > getsockname(sockfd,(struct sockaddr*)&local,&len)) {
		perror(">>> getpeername");
		exit(1);
	} else {
		printf(">>> Local socket:  ADDR %s PORT %d\n",
			inet_ntop(AF_INET,&local.sin_addr,message,sizeof(message)),
			ntohs(local.sin_port));
	}

	int j = 0;

	for(j = 0; j < 5; j++){

		bzero(message,sizeof(message));
		printf("input line:\n");
		scanf("%s",message);
		message[strlen(message)] = '\n';

		if (0 > write(sockfd,message,MAXLEN)) {
			perror(">>> write");
			exit(1);
		} else {
			puts("write OK");
		}
	}
}
