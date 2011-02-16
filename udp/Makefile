#
# simple make file
#

all: udp_srv_nonblock udp_client msg_udp_srv msg_udp_client

msg_udp_srv: msg_udp_srv.o
	gcc -o msg_udp_srv msg_udp_srv.o

msg_udp_client: msg_udp_client.o
	gcc -o msg_udp_client msg_udp_client.o

udp_srv_nonblock: udp_srv_nonblock.o
	gcc -o udp_srv_nonblock udp_srv_nonblock.o

udp_srv_nonblock.o: udp_srv_nonblock.c
	gcc -g -c udp_srv_nonblock.c

udp_client: udp_client.o
	gcc -o udp_client udp_client.o

udp_client.o: udp_client.c
	gcc -g -c udp_client.c

clean: 
	rm -rf *.o udp_client udp_srv_nonblock msg_udp_srv msg_udp_client
