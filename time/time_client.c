#include<sys/socket.h>
#include<netinet/in.h>

#define MAXLEN 100

int main(int argc,char ** argv){

	int sockfd;
	char message[MAXLEN+1];
	struct sockaddr_in server,local;
	int len = sizeof(local);
	
	if(argc != 2){
			printf("Usage: %s <ip> \n",argv[0]);
			exit(0);
	}

	if( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
			perror(">>> socket");
			exit(1);
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(5555);
	
	if(inet_pton(AF_INET,argv[1],&server.sin_addr) <= 0){
		perror(">>> inet_pton");
		exit(1);
	}

	if(connect(sockfd,(struct sockaddr *) &server,sizeof(server)) < 0){
		perror(">>> connect");
		exit(1);
	}
	
	if( getsockname(sockfd,(struct sockaddr*)&local,&len) < 0){
		perror(">>> getpeername");
		exit(1);
	}else{
		printf(">>> Local socket:  ADDR %s PORT %d\n",
			inet_ntop(AF_INET,&local.sin_addr,message,sizeof(message)),
				ntohs(local.sin_port));
	}

	int n = 0;
	char * ptr = message;
	
	for(;;){
		int i = read(sockfd,ptr,MAXLEN);
		if(i < 0){
			perror(">>> read");
			exit(1);
		}
		
		if(i > 0){
			n += i;
			ptr += i;
		}

		if(i == 0){
			close(sockfd);
			puts(message);
			exit(0);
		}						
	}
}






