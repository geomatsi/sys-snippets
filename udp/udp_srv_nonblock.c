#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define BUFSIZE 1024

int main(void)
{
	struct sockaddr_in client;
	struct sockaddr_in server;
	int sock, rc, opt, len;
	char buffer[BUFSIZE];

	// init part: create and bind udp socket
	
	if (0 > (sock = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("ERROR# can't create UDP socket");
		exit(-1);
	}
	
	opt = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, opt | O_NONBLOCK);
	
	bzero(&server, sizeof(server));
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(20001);
	server.sin_family = AF_INET;
	
	if (0 > bind(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("ERROR# can't bind UDP socket");
		exit(-1);
	}
	
	// main part: select loop for udp socket
	
	while (1) {
	
		bzero(buffer, sizeof(buffer));
		bzero(&client, sizeof(client));

		rc = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &client, &len);	    

		if (rc >= 0) {
			fprintf(stdout, "Got %d bytes from (%s, %d) : %s\n",
				rc, inet_ntoa(client.sin_addr), ntohs(client.sin_port), buffer);
			
			fprintf(stdout, "Toggle blocking mode\n");
			opt = fcntl(sock, F_GETFL, 0);
			fcntl(sock, F_SETFL, opt ^ O_NONBLOCK);
		} else {
		
			if (EWOULDBLOCK == errno) {
				fprintf(stdout, "nonblock - no data\n");
				sleep(1);
				continue;
			} else {
				perror("unexpected error");
				exit(-1);
			}
		}
	}
}

