project('tcp', 'c',
  version : '0.2',
  default_options : ['c_std=gnu99', 'buildtype=debug'],
)

get_ifindex = ['get_ifindex.c', 'netdev_utils.c', 'netdev_utils.h']
executable('get_ifindex',  get_ifindex)

raw_sock_xmit = ['raw_sock_xmit.c', 'netdev_utils.c', 'netdev_utils.h']
executable('raw_sock_xmit',  raw_sock_xmit)

raw_sock_recv = ['raw_sock_recv.c', 'netdev_utils.c', 'netdev_utils.h']
executable('raw_sock_recv',  raw_sock_recv)
