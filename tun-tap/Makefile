#
# simple make file
#

all: tunnel

tunnel: tunnel.o
	gcc -o $@ $^

tunnel.o: tunnel.c
	gcc -c -o $@ $^

clean: 
	rm -rf tunnel tunnel.o
