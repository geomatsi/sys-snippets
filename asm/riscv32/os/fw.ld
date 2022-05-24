MEMORY
{
	ram (rwx) : ORIGIN = 0x80000000, LENGTH = 512K
}

SECTIONS
{
	.text : { *(.text) } >ram
}
