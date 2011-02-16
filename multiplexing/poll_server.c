#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>
#include<sys/poll.h>

#define LISTEN 5
#define MAXLEN 100
#define MAXCLIENTS 3

int listenfd;
void int_handler(int);

int main(int argc,char ** argv){
	
	struct sockaddr_in server;
	struct sockaddr_in client;
	struct sigaction act;
	struct pollfd clients[MAXCLIENTS];
	
	int sockfd;
	int i,rc,max;
	
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
	
	clients[0].fd = listenfd;
	clients[0].events = POLLIN;
	for(i = 1; i < MAXCLIENTS; i++) clients[i].fd = -1;
	
	int len = sizeof(client);
	char message[MAXLEN+1];
	
	for(;;){

		max = 0;
		for(i = 1; i < MAXCLIENTS; i++){
			if (clients[i].fd != -1) max = i;
		}
		
		rc = poll(clients,max+1,5000);
		
		if(rc == 0){
			printf(">>> poll timeout\n");			
			continue;
		}

		if(rc == -1){
			if(errno == EINTR){
				perror(">>> 'poll' was interrupted, try again");
				continue;
			}else{
				perror(">>> exit - some other problem");
				exit(1);
			}
		}
		
		if (clients[0].revents & POLLIN){		
			if( (sockfd = accept(listenfd,(struct sockaddr *) NULL,NULL)) < 0){
				if(errno == EINTR){
					perror(">>> 'accept' was interrupted, accept again");
					continue;
				}else{
					perror(">>> exit - some other problem ");
					exit(1);
				}
			}
			
			for(i = 1; i < MAXCLIENTS; i++){
				if(clients[i].fd == -1){
					if( getpeername(sockfd,(struct sockaddr*)&client,&len) < 0){
						perror(">>> getpeername");
						exit(1);
					}else{
						printf(">>> open connection from addr %s port %d\n",
						inet_ntop(AF_INET,&client.sin_addr,message,
								sizeof(message)),ntohs(client.sin_port));
					}
					clients[i].fd = sockfd;
					clients[i].events = POLLIN;
					break;
				}

				if(i == MAXCLIENTS - 1){
					printf(">>> Clients queue is full - drop client\n");
					close(sockfd);
				}
			}
			
			if(--rc == 0) continue;
		}
		
		for(i = 1; i < MAXCLIENTS; i++){
			if (clients[i].fd == -1) continue;
			if (!(clients[i].revents & (POLLIN | POLLERR))) continue;
			
			int ret = handle_echo(clients[i].fd);
			
			if( getpeername(clients[i].fd,(struct sockaddr*)&client,&len) < 0){
				perror(">>> getpeername");
				exit(1);
			}

			if(ret < 0){
				perror(">>> problem with socket");
				printf(">>> closing this connection: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i].fd);
				clients[i].fd = -1;
			}
			
			if(ret == 0){
				printf(">>> client gone,closing connection: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
				close(clients[i].fd);
				clients[i].fd = -1;
			}
			
			if(ret > 0){
				printf(">>> session  OK: addr %s port %d\n",
				inet_ntop(AF_INET,&client.sin_addr,message,sizeof(message)),
					ntohs(client.sin_port));
			}

			if(--rc == 0) break;
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


