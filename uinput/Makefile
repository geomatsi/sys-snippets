#
# simple make file
#

all: rx tx

rx: rx.o
	gcc -o $@ $^ ${LIB}

tx: tx.o
	gcc -o $@ $^ ${LIB}

%.o: %.c
	${CC} -c -o $@ $^ -I .

clean:
	rm -rf *.o
	rm -rf rx tx
