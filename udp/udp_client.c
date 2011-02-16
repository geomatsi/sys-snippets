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
	struct sockaddr_in dest;
	int sock, rc, opt, len;
	char buffer[BUFSIZE];

	// init part: create and bind udp socket
	
	if (0 > (sock = socket(AF_INET, SOCK_DGRAM, 0))) {
		perror("ERROR# can't create UDP socket");
		exit(-1);
	}
	
	bzero(&dest, sizeof(dest));
	
	dest.sin_addr.s_addr = inet_addr("127.0.0.1");
	dest.sin_port = htons(20001);
	dest.sin_family = AF_INET;
	
	// main loop: get data - send data
	
	while (1) {
	
		bzero(buffer, sizeof(buffer));

		if (NULL == fgets(buffer, BUFSIZE, stdin)) {
			break;
		}

		rc = sendto(sock, buffer, BUFSIZE, 0, (struct sockaddr *) &dest, sizeof(dest));	    

		if (rc < 0) {
			perror("sendto");
		}
	}
}

