	.abicalls
	.text
	.align	2

# add 2 numbers
	.globl	addu
	.set	nomips16
	.set	nomicromips
	.ent	addu
	.type	addu, @function
addu:
	addu $v0, $a0, $a1
	jr $ra
	.end	addu
