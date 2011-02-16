#
# simple make file
#

CC=gcc

CFLAGS= -W -Wall

all: daemon

daemon: daemon.o
	${CC} -o $@ $^ ${LIB}

daemon.o: daemon.c
	${CC} -c -o $@ $^ ${CFLAGS}

clean: 
	rm -rf *.o daemon
