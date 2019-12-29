// SPDX-License-Identifier: GPL-2.0

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#include "netdev_utils.h"

#define BUFSIZE 2048

int main(int argc, char *argv[])
{
	struct sockaddr_ll addr;
	uint8_t buf[BUFSIZE];
	socklen_t len;
	int sock;
	int ret;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	/* usage: ./raw_sock_recv [ifname]
	 * - listen to all interfaces unless interface 'ifname' is specified
	 */
	if (argc >= 2) {
		memset(&addr, 0x0, sizeof(addr));

		addr.sll_family   = PF_PACKET;
		addr.sll_protocol = htons(ETH_P_IP);
		addr.sll_ifindex  = get_ifindex_by_ifname(argv[1]);
		if (addr.sll_ifindex < 0)
			return -1;

		ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
		if (ret < 0) {
			perror("bind");
			goto out;
		}
	}

	while (1) {
		memset(&addr, 0x0, sizeof(addr));
		memset(buf, 0x0, sizeof(buf));
		len  = sizeof(addr);

		ret = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &len);
		if (ret < 0) {
			perror("recvfrom");
			goto out;
		}

		printf("recv %d bytes from %s (%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx):\n",
			ret, get_ifname_by_ifindex(addr.sll_ifindex),
			addr.sll_addr[0],
			addr.sll_addr[1],
			addr.sll_addr[2],
			addr.sll_addr[3],
			addr.sll_addr[4],
			addr.sll_addr[5]);

		hexdump(buf, len);
	}

out:
	close(sock);
	return ret;
}
