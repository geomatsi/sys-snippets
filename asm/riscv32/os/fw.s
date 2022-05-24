	.globl  _start
_start:
	li	t0, 0
	li	t1, 0
	li	a0, 5
L1:	add	t1, t1, t0
	addi	t0, t0, 1
	bgeu	a0, t0, L1 
	mv	a0, t1
	ret
