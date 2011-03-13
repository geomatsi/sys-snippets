#
# simple make file
#

all: tunnel

tunnel: tunnel.o
	gcc -o $@ $^

%.o: %.c
	gcc -c -o $@ $^

clean:
	rm -rf tunnel *.o
