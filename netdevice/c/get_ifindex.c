// SPDX-License-Identifier: GPL-2.0

#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct ifreq eth;
	int sock;
	int ret;

	memset(&eth, 0x0, sizeof(eth));

	if (argc < 2)
		sprintf(eth.ifr_name, "eth0");
	else
		snprintf(eth.ifr_name, sizeof(eth) - 1, argv[1]);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(-1);
	}

	ret = ioctl(sock, SIOCGIFINDEX, &eth);
	if (ret < 0) {
		perror("ioctl");
		exit(-1);
	}

	printf("ifindex[%s] = %d\n", eth.ifr_name, eth.ifr_ifindex);
	return 0;
}
