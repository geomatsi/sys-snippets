// SPDX-License-Identifier: GPL-2.0

#include <stdio.h>

#include "netdev_utils.h"

int main(int argc, char *argv[])
{
	char *name = "eth0";
	int index;

	if (argc >= 2)
		name = argv[1];

	index = get_ifindex_by_ifname(name);
	if (index < 0) {
		printf("failed to get %s index\n", name);
		return -1;
	}

	printf("ifindex[%s] = %d\n", name, index);
	return 0;
}
