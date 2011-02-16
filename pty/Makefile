#
# simple make file
#

LIB=-lutil 

all: fork_pty open_pty minicom_tty

minicom_tty: minicom_tty.o
	gcc -o $@ $^ ${LIB}

minicom_tty.o: minicom_tty.c
	gcc -c -o $@ $^

fork_pty: fork_pty.o
	gcc -o $@ $^ ${LIB}

fork_pty.o: fork_pty.c
	gcc -c -o $@ $^

open_pty: open_pty.o
	gcc -o $@ $^ ${LIB}

open_pty.o: open_pty.c
	gcc -c -o $@ $^

clean: 
	rm -rf *.o
	rm -rf fork_pty open_pty minicom_tty
