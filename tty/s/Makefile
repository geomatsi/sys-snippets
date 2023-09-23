#
#
#

CCFLAGS = -Wall -Wextra -Werror -std=gnu11 -g3 -ggdb
LDFLAGS = -g3 -ggdb

all: snake

snake: snake.o ncurs.o ui.h
	$(CC) $(LDFLAGS) -lncurses $^ -o $@

tests: t1 t2 t3

t1: t1.o
	$(CC) $(LDFLAGS) $^ -o $@

t2: t2.o
	$(CC) $(LDFLAGS) -lncurses $^ -o $@

t3: t3.o
	$(CC) $(LDFLAGS) -lncurses $^ -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.core
	rm -rf snake
	rm -rf t1 t2 t3 t4

.PHONY: all clean tests
