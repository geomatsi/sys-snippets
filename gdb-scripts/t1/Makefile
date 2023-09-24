#
#
#

CROSS_COMPILE ?= riscv32-picolibc-elf-

CP = $(CROSS_COMPILE)objcopy
CC = $(CROSS_COMPILE)gcc

QEMU ?= qemu-system-riscv32
QEMU_ARGS ?= -semihosting-config enable=on -monitor none -serial none -nographic -machine virt -cpu rv32

CCFLAGS  = -march=rv32imac -mabi=ilp32
CCFLAGS += --specs=picolibc.specs --oslib=semihost
CCFLAGS += -Wall -Wextra -Werror
CCFLAGS += -ggdb -g

VPATH += src

all: qemu

VIRT1_SRC := main.c stack.c
VIRT1_LDS := ld/virt.ld

# QEMU RISC-V virt platform flash: [0x20000000, 0x24000000), however for full flash size QEMU complains:
# - qemu-system-riscv32: device requires 33554432 bytes, block backend provides 67108864 bytes
# So pad binary size to half flash size
virt1: $(VIRT1_LDS) $(VIRT1_SRC)
	@$(CC) $(CCFLAGS) -T$< $(filter-out $<, $^) -o $@.elf
	@$(CP) -O binary --gap-fill 0x00 --pad-to 0x22000000 $@.elf $@.bin

qemu: virt1
	@$(QEMU) $(QEMU_ARGS) -bios none -drive if=pflash,file=$<.bin,readonly=on,format=raw

gdb: virt1
	@$(QEMU) $(QEMU_ARGS) -bios none -drive if=pflash,file=$<.bin,readonly=on,format=raw -s -S

%.o: %.c

clean:
	rm -rf *.o *.elf *.bin

.PHONY: all run clean tags
