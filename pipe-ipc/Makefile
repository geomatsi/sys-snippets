#

all: pipe xmit

pipe: pipe.o
	${CC} -o $@ $^

xmit: xmit.o
	${CC} -o $@ $^

%.o: %.c
	${CC} -c -o $@ $^

clean:
	rm -rf xmit pipe
	rm -rf *.o
