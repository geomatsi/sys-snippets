#
#
#

CROSS_COMPILE = /home/matsi/devel/tools/riscv32-ilp32d--glibc--bleeding-edge-2021.11-1/bin/riscv32-buildroot-linux-gnu-

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld

CCFLAGS = -Wall -Wextra -Werror -std=c11
ASFLAGS =
LDFLAGS = --nostdlib

all: firmware

run: firmware
	qemu-system-riscv32 -M virt -nographic -bios none -kernel $<.elf

dbg: firmware
	qemu-system-riscv32 -M virt -nographic -bios none -kernel $<.elf -S -s

firmware: fw.ld fw.o
	$(LD) $(LDFLAGS) -T$< $(filter-out $<, $^) -o $@.elf
	$(CP) -O binary $@.elf $@.bin

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf *.o firmware.*

.PHONY: all run clean
