#
# simple make file
#

all: usbreset

usbreset: usbreset.o
	gcc -o $@ $^

%.o: %.c
	gcc -c -o $@ $^

clean:
	rm -rf *.o
	rm -rf usbreset
