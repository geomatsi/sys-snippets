#
# simple make file
#

CC=gcc

LIB= -lrt
CFLAGS= -W -Wall

all: exec_fd

exec_fd: exec_fd.o
	${CC} -o $@ $^ ${LIB}

exec_fd.o: exec_fd.c
	${CC} -c -o $@ $^ ${CFLAGS}

clean: 
	rm -rf exec_fd exec_fd.o
