#!/bin/bash

start_setup() {
	sudo modprobe veth
	sudo ip link add eth0 type veth peer name eth1

	sudo ip addr add 192.168.10.100/24 dev eth0
	sudo ip link set dev eth0 up

	sudo ip netns add mcast-test
	sudo ip link set eth1 netns mcast-test
	sudo ip netns exec mcast-test /usr/bin/ip addr add 192.168.10.200/24 dev eth1
	sudo ip netns exec mcast-test /usr/bin/ip link set dev eth1 up

	sudo ip netns exec mcast-test /bin/bash
}

stop_setup() {
	sudo ip netns del mcast-test
	sudo rmmod veth
}

case "$1" in
start)
	start_setup
	;;
stop)
	stop_setup
	;;
*)
	echo "Usage: $0 {start|stop}"
	exit 1
	;;
esac

exit 0
