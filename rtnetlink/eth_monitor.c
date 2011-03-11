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


#define LOGE 	printf
#define LOGI 	printf

typedef struct _interface_info_t {
	unsigned int i;                            /* interface index        */
	char *name;                       /* name (eth0, eth1, ...) */
	struct _interface_info_t *next;
} interface_info_t;

interface_info_t *interfaces = NULL;
int total_int = 0;

#define NL_SOCK_INV      -1
#define RET_STR_SZ       4096
#define NL_POLL_MSG_SZ   8*1024
#define SYSFS_PATH_MAX   256

static const char SYSFS_CLASS_NET[]     = "/sys/class/net";
static int nl_socket_msg = NL_SOCK_INV;
static struct sockaddr_nl addr_msg;
static int nl_socket_poll = NL_SOCK_INV;
static struct sockaddr_nl addr_poll;
static int netlink_init_interfaces_list(void);

static interface_info_t *find_info_by_index(unsigned int index) {
	interface_info_t *info = interfaces;
	while( info) {
		if (info->i == index)
			return info;
		info = info->next;
	}
	return NULL;
}

static void free_int_list()
{
	interface_info_t *tmp = interfaces;
	while(tmp) {
		if (tmp->name) free(tmp->name);
		interfaces = tmp->next;
		free(tmp);
		tmp = interfaces;
		total_int--;
	}

	if (total_int != 0 )
	{
		LOGE("Wrong interface count found\n");
		total_int = 0;
	}
}

void waitForEvent(void)
{
	char *buff;
	struct nlmsghdr *nh;
	struct ifinfomsg *einfo;
	struct iovec iov;
	struct msghdr msg;
	interface_info_t *info;
	int len;

	/* ugly: update ifindex for eth0 */
	free_int_list();
	netlink_init_interfaces_list();

	fprintf(stdout, "Poll events from ethernet devices\n");
	/*
	 *wait on uevent netlink socket for the ethernet device
	 */
	buff = (char *)malloc(NL_POLL_MSG_SZ);
	if (!buff) {
		fprintf(stdout, "Allocate poll buffer failed\n");
		exit(-1);
	}

	iov.iov_base = buff;
	iov.iov_len = NL_POLL_MSG_SZ;
	msg.msg_name = (void *)&addr_msg;
	msg.msg_namelen =  sizeof(addr_msg);
	msg.msg_iov =  &iov;
	msg.msg_iovlen =  1;
	msg.msg_control =  NULL;
	msg.msg_controllen =  0;
	msg.msg_flags =  0;

	if((len = recvmsg(nl_socket_poll, &msg, 0))>= 0) {
		//fprintf(stdout, "recvmsg get data\n");
		for (nh = (struct nlmsghdr *) buff; NLMSG_OK (nh, len);
				nh = NLMSG_NEXT (nh, len))
		{

			if (nh->nlmsg_type == NLMSG_DONE){
				LOGE("Did not find useful eth interface information\n");
				exit(-1);
			}

			if (nh->nlmsg_type == NLMSG_ERROR){

				/* Do some error handling. */
				LOGE("Read device name failed\n");
				exit(-1);
			}

			//LOGI(" event :%d  found\n",nh->nlmsg_type);
			einfo = (struct ifinfomsg *)NLMSG_DATA(nh);
			//LOGI("the device flag :%X\n",einfo->ifi_flags);
			if (nh->nlmsg_type == RTM_DELLINK ||
					nh->nlmsg_type == RTM_NEWLINK ||
					nh->nlmsg_type == RTM_DELADDR ||
					nh->nlmsg_type == RTM_NEWADDR) {
				int type = nh->nlmsg_type;

				if (type == RTM_NEWLINK && (!(einfo->ifi_flags & IFF_LOWER_UP))) {
					type = RTM_DELLINK;
				}

				if ((info = find_info_by_index(((struct ifinfomsg*) NLMSG_DATA(nh))->ifi_index)) != NULL)
					fprintf(stdout, "---------- expected event: %s:%d\n",info->name,type);
			} else {
				int type = nh->nlmsg_type;
				if ((info = find_info_by_index(((struct ifinfomsg*) NLMSG_DATA(nh))->ifi_index)) != NULL)
					fprintf(stdout, "!!!!!!!!!! unexpected event: %s:%d\n",info->name,type);
			}

		}

		//fprintf(stdout, "Done parsing\n");
	}

	return;
}

static void add_int_to_list(interface_info_t *node) {
	/*
	 *Todo: Lock here!!!!
	 */
	node->next = interfaces;
	interfaces = node;
	total_int ++;
}

static int netlink_init_interfaces_list(void)
{
	int ret = -1;
	DIR  *netdir;
	struct dirent *de;
	char path[SYSFS_PATH_MAX];
	interface_info_t *intfinfo;
	int index;
	FILE *ifidx;
#define MAX_FGETS_LEN 4
	char idx[MAX_FGETS_LEN+1];

	snprintf(path, SYSFS_PATH_MAX,"%s/eth0/ifindex",SYSFS_CLASS_NET,de->d_name);

	if ((ifidx = fopen(path,"r")) != NULL ) {
		memset(idx,0,MAX_FGETS_LEN+1);
		if(fgets(idx,MAX_FGETS_LEN,ifidx) != NULL) {
			index = strtoimax(idx,NULL,10);
		} else {
			//fprintf(stdout, "Can not read %s\n",path);
			index = -1;
		}
	} else {
		//fprintf(stdout, "Can not open %s for read\n",path);
		index = -1;
	}

	/* make some room! */
	intfinfo = (interface_info_t *) malloc(sizeof(struct _interface_info_t));
	if (intfinfo == NULL) {
		fprintf(stdout, "malloc in netlink_init_interfaces_table\n");
		exit(-1);
	}

	/* copy the interface name (eth0, eth1, ...) */
	intfinfo->name = strndup((char *) "eth0", SYSFS_PATH_MAX);
	intfinfo->i = index;
	//fprintf(stdout, "interface %s:%d found\n",intfinfo->name,intfinfo->i);
	add_int_to_list(intfinfo);

	return 0;
}

int main(void)
{
	int ret = -1;

	memset(&addr_msg, 0, sizeof(struct sockaddr_nl));
	addr_msg.nl_family = AF_NETLINK;
	memset(&addr_poll, 0, sizeof(struct sockaddr_nl));
	addr_poll.nl_family = AF_NETLINK;
	addr_poll.nl_pid = 0;//getpid();
	addr_poll.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	/*
	 *Create connection to netlink socket
	 */

	nl_socket_msg = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
	if (nl_socket_msg <= 0) {
		LOGE("Can not create netlink msg socket\n");
		goto error;
	}
	if (bind(nl_socket_msg, (struct sockaddr *)(&addr_msg),
				sizeof(struct sockaddr_nl))) {
		LOGE("Can not bind to netlink msg socket\n");
		goto error;
	}

	nl_socket_poll = socket(AF_NETLINK,SOCK_RAW,NETLINK_ROUTE);
	if (nl_socket_poll <= 0) {
		LOGE("Can not create netlink poll socket\n");
		goto error;
	}

	errno = 0;
	if(bind(nl_socket_poll, (struct sockaddr *)(&addr_poll),
				sizeof(struct sockaddr_nl))) {
		LOGE("Can not bind to netlink poll socket,%s\n",strerror(errno));

		goto error;
	}

	if ((ret = netlink_init_interfaces_list()) < 0) {
		LOGE("Can not collect the interface list\n");
		goto error;
	}

	LOGE("%s exited with success\n",__FUNCTION__);

	while (1) {
		waitForEvent();
	}

	return ret;

error:
	LOGE("%s exited with error\n",__FUNCTION__);
	if (nl_socket_msg >0)
		close(nl_socket_msg);
	if (nl_socket_poll >0)
		close(nl_socket_poll);
	return ret;
}
