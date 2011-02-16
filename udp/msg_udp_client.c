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

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct in_addr addr;
	struct iovec iov[1];
	struct msghdr msg;

	int sock, rc, opt, len;
	char buffer[BUFSIZE];

	/* check command line parameters */

	if (argc != 2) {
		printf("usage: %s <ip addr>\n", argv[0]);
		exit(0);
	}

        /* create and bind udp socket */
	
        if (0 > (sock = socket(AF_INET, SOCK_DGRAM, 0))) {
                perror("socket");
                exit(-1);
        }

	if (0 >= inet_pton(AF_INET, argv[1], &addr)) {
		perror("inet_pton");
		close(sock);
		exit(-1);
	}

        bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr = addr;
        server.sin_port = htons(20001);

	/* recv loop for udp server */

	while (1) {
		bzero(buffer, sizeof(buffer));
		bzero(&msg, sizeof(msg));
		
		if (NULL == fgets(buffer, BUFSIZE, stdin)) {
			break;
		}

		iov[0].iov_base = buffer;
		iov[0].iov_len = strlen(buffer);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		msg.msg_name = (void *) &server;
		msg.msg_namelen = sizeof(server);
		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		if (0 > (rc = sendmsg(sock, &msg, 0))) {
			perror("recvmsg\n");
			exit(-1);
		}			

		fprintf(stdout, "sent %d bytes\n", rc);

	}

	close(sock);
	return 0;
}

