	.abicalls
	.text
	.align	2

# fibonacci sequence for n > 0: 1, 1, 2, 3, ...
	.globl	fib
	.set	nomips16
	.set	nomicromips
	.ent	fib
	.type	fib, @function
fib:
	# push: $s0, $s1, $2
	addi $sp, -12
	sw $s0, 0($sp)
	sw $s1, 4($sp)
	sw $s2, 8($sp)
	# init
	li $s0, 1
	li $s1, 1
	li $t0, 3
	# special case: n = 1, 2
	slt $t1, $a0, $t0
	beq $t1, $0, calc
	j done
calc:
	# common case: n > 2
loop:
	addu $s2, $s1, $s0
	move $s0, $s1
	move $s1, $s2
	beq $t0, $a0, done
	addi $t0, $t0, 1
	j loop
done:
	move $v0, $s1
	# pop: $s0, $s1, $2
	lw $s0, 0($sp)
	lw $s1, 4($sp)
	lw $s2, 8($sp)
	addi $sp, 12
	jr $ra
	.end	fib
