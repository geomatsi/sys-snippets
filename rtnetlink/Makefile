#
# simple make file
#

LIB=

all: eth_monitor

eth_monitor: eth_monitor.o
	gcc -o $@ $^ ${LIB}

eth_monitor.o: eth_monitor.c
	gcc -c -o $@ $^

clean:
	rm -rf *.o
	rm -rf eth_monitor
