#
#
#

all: fbdev

fbdev: fbdev.o
	gcc -o $@ $^

fbdev.o: fbdev.c
	gcc -c -o $@ $^

clean: 
	rm -rf fbdev fbdev.o
