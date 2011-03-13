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


int get_ifindex_by_ifname(char * ifname)
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
