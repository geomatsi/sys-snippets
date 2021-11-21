	.abicalls
	.text
	.align	2

# count non-zero bits in binary representation of 32bit integer
	.globl	ones
	.set	nomips16
	.set	nomicromips
	.ent	ones
	.type	ones, @function
ones:
	li $v0, 0
loop:
	andi $t0, $a0, 1
	add $v0, $v0, $t0
	srl $a0, $a0, 1
	beq $a0, $0, done
	j loop
done:
	jr $ra
	.end	ones
