#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>

#define MAXLEN 100
#define LISTEN 5

int main(int argc,char ** argv){
	
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN+1];
	int listenfd,sockfd;
	time_t ticks;
	int rc;
	pid_t pid;
	
	int len = sizeof(client);
			
	
	if( (listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
			perror(">>> socket");
			exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(5555);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr *) &server,sizeof(server)) < 0){
		perror(">>> bind");
		exit(1);
	}

	if( listen(listenfd,LISTEN) < 0){
		perror(">>> listen");
		exit(1);
	}

	for(;;){
		sockfd = accept(listenfd,(struct sockaddr *) NULL,NULL);
		
		if( (pid = fork()) < 0){
			perror(">>> fork");
			exit(1);
		}

		if( pid == 0){
			/* child -> handles request */
			if( getpeername(sockfd,(struct sockaddr*)&client,&len) < 0){
				perror(">>> getpeername");
				exit(1);
			}else{
				printf(">>> connection from addr %s port %d\n",
					inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
						ntohs(client.sin_port));
			}
			
			ticks = time(NULL);
			snprintf(message,sizeof(message),"%.24s\er\en",ctime(&ticks));
			if( (rc = write(sockfd,message,strlen(message))) < 0){
				perror(">>> write");
			}
		}
		/* parent */
		close(sockfd);
	}	
}






