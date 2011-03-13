#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <net/if.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#define BUFSIZE 1024

/* */

int rx(void);
void display(unsigned char *buffer, int name);

/* */

char eth[10];

/* */

int main(int argc, char * argv[])
{

	if (argc < 2)
		sprintf(eth, "eth0");
	else
		snprintf(eth, sizeof(eth) - 1, argv[1]);


	while (1) {
		if (0 > rx())
			break;
	}

	return 0;
}

/*  Rx */
int rx(void)
{

	int sock, bytes, len;
	struct sockaddr_ll addr;
	unsigned char buf[BUFSIZE];

	bzero(buf, sizeof(buf));
	bzero(&addr, sizeof(addr));

	addr.sll_family   = PF_PACKET;
	addr.sll_protocol = htons(ETH_P_IP);
	addr.sll_ifindex  = get_ifindex_by_ifname(eth);

	if (0 > (sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))) {
		perror("socket");
		return -1;
	}

	/*
	if( fcntl(sock, F_SETFL, O_NONBLOCK) != 0 ){
	    perror("nonblocking I/O");
	}

	if( bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
	    perror("bind");
	    close(sock);
	    return -1;
	}
	*/

	if (0 > (bytes = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &len))) {
		printf(">>> got %d bytes\n", bytes);
		display(buf, bytes);
		printf("sll_ifindex: %d\n", addr.sll_ifindex);
		/*
		printf("sll_addr: %02x%02x%02x%02x%02x%02x\n", 
			addr.sll_addr[1], addr.sll_addr[2],addr.sll_addr[3],addr.sll_addr[4],addr.sll_addr[5]);
		*/
		printf("sll_addr: %02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n", addr.sll_addr);
	} else {
		perror("recvfrom");
		close(sock);
		return -1;
	}

	close(sock);
	return bytes;
}

void display(unsigned char * buffer, int len)
{
	int idx;

	for(idx = 1; idx <= len; idx++){
		printf("%02x%c", buffer[idx - 1], ( idx % 10 ) ? ' ' : '\n');
	}

	printf("\n");
}

