#
# simple make file
#

LIB=

all: rx tx

rx: rx.o get_ifindex_by_ifname.o
	gcc -o $@ $^ ${LIB}

tx: tx.o get_ifindex_by_ifname.o
	gcc -o $@ $^ ${LIB}

%.o: %.c
	gcc -c -o $@ $^

clean:
	rm -rf *.o
	rm -rf rx tx
