#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
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
	struct sockaddr_in client;
	struct cmsghdr * cmptr;
	struct in_addr addr;
	struct iovec iov[1];
	struct msghdr msg;

	int sock, rc, opt, len;
	char buffer[BUFSIZE];
	char ctl[BUFSIZE];

	int optval = 1;

	/* check command line parameters */

	if (argc != 2) {
		printf("usage: %s <ip addr>\n", argv[0]);
		exit(0);
	}

        /* create udp socket */
	
        if (0 > (sock = socket(AF_INET, SOCK_DGRAM, 0))) {
                perror("socket");
                exit(-1);
        }

	/* set socket option making recvmsg return control messages */

	if (0 > setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(optval))) {
		perror("setsockopt");
		exit(-1);
	}

        /* bind udp socket */

	if (0 >= inet_pton(AF_INET, argv[1], &addr)) {
		perror("inet_pton");
		close(sock);
		exit(-1);
	}

        bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr = addr;
        server.sin_port = htons(20001);

	if (0 > bind(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("bind");
		exit(-1);
	}
	
	/* recv loop for udp server */

	while (1) {

		bzero(buffer, sizeof(buffer));
		bzero(&client, sizeof(client));
		bzero(&msg, sizeof(msg));
		bzero(&ctl, sizeof(ctl));
		
		/* specify buffer to store received packet */

		iov[0].iov_base = buffer;
		iov[0].iov_len = sizeof(buffer);
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		
		/* specify buffer to store packet source address */

		msg.msg_name = (struct sockaddr *) &client;
		msg.msg_namelen = sizeof(client);
		
		/* specify buffer to store control messages */

		msg.msg_control = (void *) &ctl;
		msg.msg_controllen = sizeof(ctl);
		msg.msg_flags = 0;

		if (0 > (rc = recvmsg(sock, &msg, 0))) {
			perror("recvmsg\n");
			exit(-1);
		}			

		fprintf(stdout, "got %d bytes from (%s, %d) : %s\n", 
			rc, inet_ntoa(client.sin_addr), ntohs(client.sin_port), buffer);

		if (msg.msg_flags & MSG_TRUNC) {
			fprintf(stdout, "diagram was truncated\n");
		}
		
		if (msg.msg_flags & MSG_CTRUNC) {
			fprintf(stdout, "control data was truncated\n");
		}

		/* process control messages */

		for (cmptr = CMSG_FIRSTHDR(&msg); cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
			if (IPPROTO_IP == cmptr->cmsg_level && IP_PKTINFO == cmptr->cmsg_type) {
				struct in_pktinfo *pkt = (struct in_pktinfo *) CMSG_DATA(cmptr);
				fprintf(stdout, "ifindex = %d  local addr = %s  packet dest addr = %s\n",
					pkt->ipi_ifindex, inet_ntoa(pkt->ipi_spec_dst), inet_ntoa(pkt->ipi_addr));
			} else {
				fprintf(stdout, "ctl msg of level %d and type %d\n",
					cmptr->cmsg_level, cmptr->cmsg_type);
			}
		}
	}
}

