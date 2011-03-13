#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>

#include <linux/if.h>
#include <linux/if_tun.h>


#define max(a,b) ((a)>(b) ? (a):(b))


int main(int argc, char *argv[])
{
    	int fd, sock, rc, max, flags, opt;
	struct sockaddr_in server, client;
    	struct ifreq ifr;
    	char buf[1600];
	fd_set fds;

	if (argc < 4) {
		printf("usage: bridge tap|tun src dest\n");
		exit(1);
	}

	if (strncmp(argv[1], "tun", 3) && strncmp(argv[1], "tap", 3)) {
		printf("usage: bridge tap|tun\n");
		exit(1);
	}

	if (!strncmp(argv[1], "tun", 3)) {
		flags = IFF_TUN | IFF_NO_PI;
	} else if (!strncmp(argv[1], "tap", 3)) {
		flags = IFF_TAP | IFF_NO_PI;
	} else {
		/* should not happen */
		assert(0);
	}

	/* configure tun for tun%d or tap%d */
	if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		exit(-1);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	sprintf(ifr.ifr_name, "%s%d", argv[1], 0);

	if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
		perror("ioctl TUNSETIFF");
		exit(-1);
	}

	if (ioctl(fd, TUNSETNOCSUM, 1) < 0) {
		perror("ioctl TUNSETNOSUM");
		exit(-1);
	}


	/* create udp connection */

	if( 0 > (sock = socket(AF_INET, SOCK_DGRAM, 0)) ){
		perror("socket");
		exit(-1);
	}

	/*
	opt = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, opt | O_NONBLOCK);
	*/

	bzero(&server, sizeof(server));	
	server.sin_addr.s_addr = inet_addr(argv[2]);
	server.sin_port = htons(20001);
	server.sin_family = AF_INET;

	if( 0 > bind(sock, (struct sockaddr *) &server, sizeof(server)) ){
	    perror("bind");
	    exit(-1);
	}

	bzero(&client, sizeof(client));	
	client.sin_addr.s_addr = inet_addr(argv[3]);
	client.sin_port = htons(20001);
	client.sin_family = AF_INET;

	/* go on */

	max = max(fd, sock) + 1;

	while(1){
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		FD_SET(sock, &fds);

		select(max, &fds, NULL, NULL, NULL);

		if( FD_ISSET(fd, &fds) ) {
			if (0 > (rc = read(fd, buf, sizeof(buf)))) {
				perror("read");
				break;
			}

			printf("<1> read %d bytes from tun0\n", rc);
			rc = sendto(sock, (void *)buf, rc, 0, (struct sockaddr *)(&client), sizeof(client));
			if (0 > rc) {
                		perror("sock sendto");
                		exit(-1);
            		}
			printf("<1> write %d bytes to sock\n", rc);
		}

		if( FD_ISSET(sock, &fds) ) {
			if (0 > (rc = read(sock, buf, sizeof(buf)))) {
				perror("read");
				break;
			}

			printf("<2> read %d bytes from sock\n", rc);
			if (0 > (rc = write(fd, buf, rc))) {
                		perror("sock write");
                		exit(-1);
            		}
			printf("<2> write %d bytes to tun0\n", rc);
		}
	}
}
