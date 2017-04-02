#

CC=gcc
CFGLAGS=-Wall

all: fs_notify

fs_notify: fs_notify.o
	$(CC) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o
	rm -rf fs_notify

.PHONY: clean
