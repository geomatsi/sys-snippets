#include <asm/types.h>

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>


#define NL_POLL_MSG_SZ   8*1024
#define SYSFS_PATH_MAX   256
#define NL_SOCK_INV      -1
#define RET_STR_SZ       4096

/* */

typedef struct _interface_info_t {
	unsigned int i;
	char *name;
} interface_info_t;

/* */

static const char SYSFS_CLASS_NET[]     = "/sys/class/net";
static const char IFNAME[]     = "eth4";

static int nl_socket_msg = NL_SOCK_INV;
static int nl_socket_poll = NL_SOCK_INV;

static struct sockaddr_nl addr_msg;
static struct sockaddr_nl addr_poll;

interface_info_t *ifdata = NULL;

/* */

static void waitForEvent(void);
char * get_ifname(int);

/* */

int main(void)
{
	memset(&addr_msg, 0, sizeof(struct sockaddr_nl));
	addr_msg.nl_family = AF_NETLINK;

	memset(&addr_poll, 0, sizeof(struct sockaddr_nl));
	addr_poll.nl_family = AF_NETLINK;
	addr_poll.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
	addr_poll.nl_pid = 0;

	/* Create netlink sockets */

	nl_socket_msg = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (nl_socket_msg <= 0) {
		perror("create netlink msg socket");
		goto error;
	}

	if (bind(nl_socket_msg, (struct sockaddr *)(&addr_msg), sizeof(struct sockaddr_nl))) {
		perror("bind netlink msg socket");
		goto error;
	}

	nl_socket_poll = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (nl_socket_poll <= 0) {
		perror("create netlink poll socket");
		goto error;
	}

	if(bind(nl_socket_poll, (struct sockaddr *)(&addr_poll), sizeof(struct sockaddr_nl))) {
		perror("bind netlink poll socket");
		goto error;
	}

	/* init data structure for trackable interface */

	ifdata = (interface_info_t *) malloc(sizeof(interface_info_t));
	if (ifdata == NULL) {
		printf("no memory to allocate ifdata\n");
		goto error;
	}

	ifdata->name = strndup((char *) IFNAME, SYSFS_PATH_MAX);
	ifdata->i = -1;

	while (1) {
		waitForEvent();
	}

	return 0;

error:
	if (nl_socket_msg > 0)
		close(nl_socket_msg);

	if (nl_socket_poll > 0)
		close(nl_socket_poll);

	return -1;
}

void waitForEvent(void)
{
	struct ifinfomsg *einfo;
	struct nlmsghdr *nh;
	struct msghdr msg;
	struct iovec iov;
	char *buff;
	int len;

	/* wait on uevent netlink socket for the ethernet device */

	buff = (char *) malloc(NL_POLL_MSG_SZ);

	if (!buff) {
		printf("Allocate poll buffer failed\n");
		exit(-1);
	}

	iov.iov_len = NL_POLL_MSG_SZ;
	iov.iov_base = buff;
	msg.msg_name = (void *)(&addr_msg);
	msg.msg_namelen =  sizeof(addr_msg);
	msg.msg_iov =  &iov;
	msg.msg_control =  NULL;
	msg.msg_controllen =  0;
	msg.msg_iovlen =  1;
	msg.msg_flags =  0;

	if ((len = recvmsg(nl_socket_poll, &msg, 0)) >= 0) {

		for (nh = (struct nlmsghdr *) buff; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len))
		{
			if (nh->nlmsg_type == NLMSG_DONE) {
				printf("Did not find useful eth interface information\n");
				exit(-1);
			}

			if (nh->nlmsg_type == NLMSG_ERROR) {
				printf("Read device name failed\n");
				exit(-1);
			}

			einfo = (struct ifinfomsg *) NLMSG_DATA(nh);

#if 1

			{
				int index = ((struct ifinfomsg *) NLMSG_DATA(nh))->ifi_index;
				int type = nh->nlmsg_type;

				printf("event [%s, %d] -> %d]\n", get_ifname(index), index, type);
			}

#else

			if (nh->nlmsg_type == RTM_DELLINK ||
					nh->nlmsg_type == RTM_NEWLINK ||
					nh->nlmsg_type == RTM_DELADDR ||
					nh->nlmsg_type == RTM_NEWADDR) {

				int index = ((struct ifinfomsg *) NLMSG_DATA(nh))->ifi_index;
				int type = nh->nlmsg_type;

				if (type == RTM_NEWLINK && (!(einfo->ifi_flags & IFF_LOWER_UP))) {
					type = RTM_DELLINK;
				}

				printf("event [%s, %d] -> %d]\n", get_ifname(index), index, type);
			}
#endif
		}
	}

	return;
}

char * get_ifname(int ifindex)
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
