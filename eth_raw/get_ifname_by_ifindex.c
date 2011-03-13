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

char * get_ifname_by_ifindex(int ifindex)
{
	int sock, rc;
	struct ifreq eth;

	bzero(&eth, sizeof(eth));
	eth.ifr_ifindex = ifindex;
	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (0 > (rc  = ioctl(sock, SIOCGIFNAME, &eth))) {
		perror("ioctl SIOCGIFNAME");
		return strdup("none");
	}

	close(sock);

	return strdup(eth.ifr_name);
}
