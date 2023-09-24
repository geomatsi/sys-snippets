/*
 * FLASH/RAM addrs selected according to QEMU RISC-V virt machine memory map
 * - see hw/riscv/virt.c in QEMU source tree
 */

__flash =      0x20000000;
__flash_size = 0x04000000;
__ram =        0x80000000;
__ram_size   = 0x00200000;
__stack_size = 4k;

INCLUDE picolibc.ld
