#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#define MAXLEN 100

void pipe_handler(int);

int main(int argc,char ** argv)
{
	int sockfd;
	char message[MAXLEN+1];
	struct sockaddr_in server,local;
	int len = sizeof(local);

	signal(SIGPIPE,pipe_handler);

	if (argc != 2) {
		printf("Usage: %s <ip> \n",argv[0]);
		exit(0);
	}

	if (0 > (sockfd = socket(AF_INET,SOCK_STREAM,0))) {
		perror(">>> socket");
		exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);

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

	if (0 > echo_handler(sockfd)) {
		perror(">>> problem");
		exit(1);
	}

	close(sockfd);
	exit(0);
}

int echo_handler(int sockfd)
{
	char send[MAXLEN];
	char recv[MAXLEN];

	while(1) {
		bzero(send,sizeof(send));
		bzero(recv,sizeof(recv));
		gets(send);	

		if (0 == strlen(send)) {
			return 0;
		} else {	
			/* send[strlen(send)] = '\n'; */
		}

		if (0 > write(sockfd,send,strlen(send))) {
			return -1;
		}

		sleep(1);
		char c[] = "end\n";
		if (0 > write(sockfd,c,strlen(c))) {
			return -1;	
		}

		printf(">>> reading from server: ");

		if (0 > read(sockfd,recv,MAXLEN)) {
			return -1;
		}

		puts(recv);
	}
}

void pipe_handler(int c)
{
	puts(">>> got SIGPIPE\n");
	exit(1);
}




