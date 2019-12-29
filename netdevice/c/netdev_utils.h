#ifndef __NETDEV_UTILS_H__
#define __NETDEV_UTILS_H__

#include <stdint.h>

int get_ifindex_by_ifname(char *ifname);
char *get_ifname_by_ifindex(int ifindex);
void hexdump(const uint8_t *data, int len);

#endif /* __NETDEV_UTILS_H__ */
