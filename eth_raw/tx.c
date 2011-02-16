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

#define BUFSIZE 63

/* */

int tx(void);
int get_ifindex(char *);
void display(unsigned char *, int);

/* */

int main(void)
{
	int rc = tx();
	printf(">>> sent %d bytes\n", rc);
	return 0;
}


/*  Tx */
int tx(void)
{
	int sock, bytes;
	unsigned char buf[BUFSIZE];

	struct sockaddr_ll addr;
	struct ethhdr * eth_hdr; 

	unsigned char dst_mac[6] = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};
	unsigned char src_mac[6] = {0xee, 0xee, 0xee, 0xee, 0xee, 0xee};

	bzero(&buf, sizeof(buf));
	bzero(&addr, sizeof(addr));

	eth_hdr = (struct ethhdr* ) buf;
	memcpy((void*)(eth_hdr->h_dest), (void*)dst_mac, ETH_ALEN);
	memcpy((void*)(eth_hdr->h_source), (void*)src_mac, ETH_ALEN);
	eth_hdr->h_proto = 0x00;

	addr.sll_family   = PF_PACKET;   
	addr.sll_halen    = ETH_ALEN;     
	addr.sll_ifindex  = get_ifindex("eth0");
	addr.sll_addr[0]  = 0xbb;     
	addr.sll_addr[1]  = 0xbb;     
	addr.sll_addr[2]  = 0xbb;
	addr.sll_addr[3]  = 0xbb;
	addr.sll_addr[4]  = 0xbb;
	addr.sll_addr[5]  = 0xbb;
	addr.sll_addr[6]  = 0xbb;     /*not used*/
	addr.sll_addr[7]  = 0xbb;     /*not used*/

	printf("ifndex = %d\n", addr.sll_ifindex);

	if (0 > (sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))) {
		perror("socket");
		return;
	}

	/*
	if( fcntl(sock, F_SETFL, O_NONBLOCK) != 0 ){
	    perror("nonblocking I/O");
	}
	*/

	bytes = sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, sizeof(addr));

	if (bytes) {
		display(buf, bytes);
	} else {
		perror("sendmsg");
		close(sock);
		return -1;
	}

	close(sock);
	return bytes;
}

void display(unsigned char * buffer, int len)
{
	int idx;

	printf("\n");

	for(idx = 1; idx <= len; idx++){
		printf("%02x%c", buffer[idx - 1], ( idx % 10 ) ? ' ' : '\n');
	}

	printf("\n");
}

int get_ifindex(char * ifname)
{
	int sock, rc;
	struct ifreq eth;

	bzero(&eth, sizeof(eth));
	sprintf(eth.ifr_name, ifname); 
	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (0 > (rc  = ioctl(sock, SIOCGIFINDEX, &eth))) {
		perror("ioctl");
		return -1;
	}

	close(sock);
	return eth.ifr_ifindex;
}
