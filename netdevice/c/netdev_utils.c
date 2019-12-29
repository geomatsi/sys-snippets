// SPDX-License-Identifier: GPL-2.0

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "netdev_utils.h"

int get_ifindex_by_ifname(char *ifname)
{
	struct ifreq eth;
	int sock;
	int ret;

	memset(&eth, 0x0, sizeof(eth));
	sprintf(eth.ifr_name, ifname);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	ret = ioctl(sock, SIOCGIFINDEX, &eth);
	if (ret < 0) {
		perror("ioctl SIOCGIFINDEX");
		goto out;
	}

	ret = eth.ifr_ifindex;
out:
	close(sock);
	return ret;
}

char *get_ifname_by_ifindex(int ifindex)
{
	struct ifreq eth;
	char *res;
	int sock;
	int ret;

	memset(&eth, 0x0, sizeof(eth));
	eth.ifr_ifindex = ifindex;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		return NULL;
	}

	ret = ioctl(sock, SIOCGIFNAME, &eth);
	if (ret < 0) {
		perror("ioctl SIOCGIFNAME");
		res = NULL;
		goto out;
	}

	res = strdup(eth.ifr_name);
out:
	close(sock);
	return res;
}

void hexdump(const uint8_t *data, int len)
{
	for (int i = 0; i < len; i++)
		printf("%02x%c", data[i], (i == 0 || i % 32) ? ' ' : '\n');

	printf("\n");
}
