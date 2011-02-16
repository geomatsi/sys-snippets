#
# simple make file
#

CC=gcc

LIB= -lrt
CFLAGS= -W -Wall

all: event

event: event.o
	${CC} -o $@ $^ ${LIB}

event.o: event.c
	${CC} -c -o $@ $^ ${CFLAGS}

clean: 
	rm -rf event event.o
