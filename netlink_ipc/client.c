#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/netlink.h>

#define NETLINK_BPLNIC  25
#define SERVER_NL_ID 1000

int main(int argc, char * argv[])
{   
    
    char data[100] = "CLIENT TALK";

    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr * nlh = NULL;
    uint8_t * ptr;
    int sock_fd, fd;
    int tmp, rc, i;
    char ch;

    int data_size = sizeof(data);

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_BPLNIC);

    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    //  src address
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();                 //  self pid
    src_addr.nl_groups = 0;                     //  not in mcast groups

    if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        return -1;
    }

    //  dest address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = SERVER_NL_ID; 
    dest_addr.nl_groups = 0;                    // unicast

    //  allocate packet
    nlh = (struct nlmsghdr *) calloc(NLMSG_SPACE(data_size), 1);

    //  header
    nlh->nlmsg_len = NLMSG_SPACE(data_size);
    nlh->nlmsg_pid = getpid(); 
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    
    memcpy(NLMSG_DATA(nlh), (void *) data, data_size);

    printf("client %d sends message to server\n", getpid());

    if (sendto(sock_fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        close(sock_fd);
        return -1;
    }

    //  recv
    memset(nlh, 0, NLMSG_SPACE(data_size));
    nlh->nlmsg_len = NLMSG_SPACE(data_size);

    if (recvfrom(sock_fd, nlh, nlh->nlmsg_len, 0, NULL, NULL) < 0) {
        perror("recvfrom");
        close(sock_fd);
        return -1;
    }

    ptr = (uint8_t *) NLMSG_DATA(nlh);
    printf("client got message from server %d\n", nlh->nlmsg_pid); 
    printf("message payload: %s\n", ptr);
    
    /*
    for(i = 0; i < NLMSG_PAYLOAD(nlh,0); i++){
        printf("%02x ", *ptr++);
    }

    printf("\n");
    */

    close(sock_fd);
    return 0;
}

