#
#
#

CROSS_COMPILE ?= riscv64-unknown-linux-gnu-

CC = $(CROSS_COMPILE)gcc

CCFLAGS = -Wall -Wextra -Werror -std=c11

all: hwprobe

hwprobe: hwprobe.o
	${CC} -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o hwprobe

.PHONY: all clean
