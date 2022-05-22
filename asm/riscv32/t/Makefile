#
#
#

AS = /home/matsi/devel/tools/riscv32-ilp32d--glibc--bleeding-edge-2021.11-1/bin/riscv32-buildroot-linux-gnu-as
CC = /home/matsi/devel/tools/riscv32-ilp32d--glibc--bleeding-edge-2021.11-1/bin/riscv32-buildroot-linux-gnu-gcc
LD = /home/matsi/devel/tools/riscv32-ilp32d--glibc--bleeding-edge-2021.11-1/bin/riscv32-buildroot-linux-gnu-gcc

CCFLAGS = -Wall -Wextra -Werror -std=c11
ASFLAGS =
LDFLAGS = -static

all: qemu

qemu: test
	qemu-riscv32 $<

test: main.o test1.o
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o *.core test

.PHONY: all qemu clean
