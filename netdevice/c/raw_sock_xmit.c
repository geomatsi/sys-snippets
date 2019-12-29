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

#define BUFSIZE 32

int main(int argc, char *argv[])
{
	unsigned char dst_mac[6] = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};
	unsigned char src_mac[6] = {0xee, 0xee, 0xee, 0xee, 0xee, 0xee};
	uint8_t buf[BUFSIZE];
	struct sockaddr_ll addr;
	struct ethhdr *eth_hdr;
	char *ifname = "eth0";
	int bytes;
	int sock;

	if (argc >= 2)
		ifname = argv[1];

	memset(&addr, 0x0, sizeof(addr));
	memset(&buf, 0x0, sizeof(buf));

	eth_hdr = (struct ethhdr *)buf;
	memcpy((void *)(eth_hdr->h_source), (void *)src_mac, ETH_ALEN);
	memcpy((void *)(eth_hdr->h_dest), (void *)dst_mac, ETH_ALEN);
	eth_hdr->h_proto = 0x00;

	addr.sll_ifindex  = get_ifindex_by_ifname(ifname);
	if (addr.sll_ifindex < 0)
		return -1;

	addr.sll_family   = PF_PACKET;
	addr.sll_halen    = ETH_ALEN;
	addr.sll_addr[0]  = 0xbb;
	addr.sll_addr[1]  = 0xbb;
	addr.sll_addr[2]  = 0xbb;
	addr.sll_addr[3]  = 0xbb;
	addr.sll_addr[4]  = 0xbb;
	addr.sll_addr[5]  = 0xbb;
	addr.sll_addr[6]  = 0xbb;
	addr.sll_addr[7]  = 0xbb;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	bytes = sendto(sock, buf, sizeof(buf), 0, (struct sockaddr *) &addr, sizeof(addr));
	if (bytes < 0) {
		perror("sendto");
		goto out;
	}

	printf("sent %d bytes:\n", bytes);
	hexdump(buf, bytes);

out:
	close(sock);
	return 0;
}
