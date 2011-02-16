#include <sys/ioctl.h>
#include <net/if.h>

#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	int sock, rc;
	struct ifreq eth;

	bzero(&eth, sizeof(eth));

	if (argc < 2) {
		sprintf(eth.ifr_name, "eth0");
	} else {
		snprintf(eth.ifr_name, sizeof(eth) - 1, argv[1]);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (0 > (rc  = ioctl(sock, SIOCGIFINDEX, &eth))) {
		perror("ioctl");
		exit(-1);
	}

	printf(">> index[%s] = %d\n", eth.ifr_name, eth.ifr_ifindex);
	return 0;
}
