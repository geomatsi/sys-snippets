#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>

#define LISTEN 5
#define MAXLEN 100
#define MAXCLIENTS 2

int listenfd;
void int_handler(int);

int main(int argc,char ** argv){
	
	struct sockaddr_in server;
	struct sockaddr_in client;
	char message[MAXLEN+1];
	int clients[MAXCLIENTS];
	int sockfd;
	struct timeval tv;
	struct sigaction act;
	fd_set rset;
	int i,rc,len,max;
	
	len = sizeof(client);

		
	bzero(&act,sizeof(act));
	act.sa_handler = int_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT,&act,NULL);
	

	
	if( (listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
			perror(">>> socket");
			exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(10000);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr *) &server,sizeof(server)) < 0){
		perror(">>> bind");
		exit(1);
	}

	if( listen(listenfd,LISTEN) < 0){
		perror(">>> listen");
		exit(1);
	}

	for(i = 0; i < MAXCLIENTS; i++) clients[i] = -1;
	
	for(;;){

		FD_ZERO(&rset);
		FD_SET(listenfd,&rset);
		max = listenfd;
		for(i = 0; i < MAXCLIENTS; i++){
			if (clients[i] == -1) continue;
			FD_SET(clients[i],&rset);
			if(max < clients[i]) max = clients[i];
		}
		
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		
		rc = select(max + 1,&rset,NULL,NULL,&tv);
		
		if(rc == 0){
			printf(">>> select timeout\n");			
			continue;
		}

		if(rc == -1){
			if(errno == EINTR){
				perror(">>> 'select' was interrupted, try again");
				continue;
			}else{
				perror(">>> exit - some other problem");
				exit(1);
			}
		}
		
		if (FD_ISSET(listenfd,&rset)){		
			if( (sockfd = accept(listenfd,(struct sockaddr *) NULL,NULL)) < 0){
				if(errno == EINTR){
					perror(">>> 'accept' was interrupted, accept again");
					continue;
				}else{
					perror(">>> exit - some other problem ");
					exit(1);
				}
			}
			
			for(i = 0; i < MAXCLIENTS; i++){
				if(clients[i] == -1){
					if( getpeername(sockfd,(struct sockaddr*)&client,&len) < 0){
						perror(">>> getpeername");
						exit(1);
					}else{
						printf(">>> open connection from addr %s port %d\n",
						inet_ntop(AF_INET,&client.sin_addr,message,
								sizeof(message)),ntohs(client.sin_port));
					}
					clients[i] = sockfd;
					break;
				}

				if(i == MAXCLIENTS - 1){
					printf(">>> Clients queue is full - drop client\n");
					close(sockfd);
				}
			}
		}
		
		for(i = 0; i < MAXCLIENTS; i++){
			if (clients[i] == -1) continue;
			if (! FD_ISSET(clients[i],&rset)) continue;
			
			rc = handle_echo(clients[i]);
			
			if( getpeername(clients[i],(struct sockaddr*)&client,&len) < 0){
				perror(">>> getpeername");
				exit(1);
			}

			if(rc < 0){
				perror(">>> problem with socket");
				printf(">>> closing this connection: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i]);
				clients[i] = -1;
			}
			
			if(rc == 0){
				printf(">>> client gone,closing connection: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i]);
				clients[i] = -1;
			}
			
			if(rc > 0){
				printf(">>> session  OK: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
			}

		}
	}
	
	close(listenfd);
}


int handle_echo(int sock){
	
	char line[MAXLEN];
	char c, * ptr = line;	
	int j = 0;
		
	bzero(line,sizeof(line));
		
	for(;;){
		if( (j = read(sock,&c,1)) < 0){
			perror(">>> read");
			return -1;
		}

		if(j == 0){
			return 0;
		}

		if(j > 0){
			*ptr++ = c;
			if(c == '\n'){
				break;
			}	
		}
	}
			
	if( write(sock,line,MAXLEN) < 0){
		perror(">>> write");
		return -1;
	}

	return 1;
}

void int_handler(int i){
	puts(">>>got SIGINT - exiting\n");
	exit(0);
}


