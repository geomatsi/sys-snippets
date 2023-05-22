#
#
#

CCFLAGS = -Wall -Wextra -Werror -std=gnu11 -g3 -ggdb
LDFLAGS = -g3 -ggdb

all: tetris1 tetris2

tetris1: tetris.o term.o ui.h
	$(CC) $(LDFLAGS) $^ -o $@

tetris2: tetris.o ncurs.o ui.h
	$(CC) $(LDFLAGS) -lncurses $^ -o $@

tests: t1 t2

t1: t1.o
	$(CC) $(LDFLAGS) $^ -o $@

t2: t2.o
	$(CC) $(LDFLAGS) -lncurses $^ -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.core
	rm -rf tetris1 tetris2 tetris3
	rm -rf t1 t2 t3

.PHONY: all clean tests
