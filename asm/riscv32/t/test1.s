	.text
	.align	2

# rv32i: sum of the first n numbers
	.globl  sum
	.type	sum, @function
sum:
	li	t0, 0
	li	t1, 0
L1:	add	t1, t1, t0
	addi	t0, t0, 1
	bgeu	a0, t0, L1 
	mv	a0, t1
	ret

# rv32im: sum of the first n squares
	.globl  sum2
	.type	sum2, @function
sum2:
	li	t0, 0
	li	t1, 0
L2:	mul	t2, t0, t0
	add	t1, t1, t2
	addi	t0, t0, 1
	bgeu	a0, t0, L2
	mv	a0, t1
	ret
