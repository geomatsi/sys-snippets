#
#
#

AS = /home/matsi/devel/tools/mips32--glibc--stable-2020.08-1/bin/mips-buildroot-linux-gnu-as
CC = /home/matsi/devel/tools/mips32--glibc--stable-2020.08-1/bin/mips-buildroot-linux-gnu-gcc
LD = /home/matsi/devel/tools/mips32--glibc--stable-2020.08-1/bin/mips-buildroot-linux-gnu-gcc

CCFLAGS = -Wall -Wextra -Werror -std=c11
ASFLAGS =
LDFLAGS = -static

all: qemu

qemu: test
	qemu-mips $<

test: main.o test1.o test2.o test3.o
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.core test

.PHONY: all qemu clean
