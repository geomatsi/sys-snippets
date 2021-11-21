#include <stdint.h>
#include <assert.h>
#include <stdio.h>

extern uint32_t addu(uint32_t, uint32_t);
extern uint32_t ones(uint32_t);
extern uint32_t fib(uint32_t);

int main(void)
{
	// test: sum
	assert(addu(0, 0) == 0);
	assert(addu(1, 2) == 3);
	assert(addu(0xfffffffe, 1) == 0xffffffff);
	assert(addu(0xffffffff, 1) == 0);

	// test: ones
	assert(ones(0) == 0);
	assert(ones(1) == 1);
	assert(ones(2) == 1);
	assert(ones(3) == 2);
	assert(ones(7) == 3);
	assert(ones(8) == 1);
	assert(ones(0xf) == 4);
	assert(ones(0xf0) == 4);
	assert(ones(0xff) == 8);
	assert(ones(0xffff) == 16);
	assert(ones(0xff00ff00) == 16);
	assert(ones(0xffffffff) == 32);

	// test: fib
	assert(fib(1) == 1);
	assert(fib(2) == 1);
	assert(fib(3) == 2);
	assert(fib(4) == 3);
	assert(fib(5) == 5);
	assert(fib(6) == 8);
	assert(fib(7) == 13);
	assert(fib(8) == 21);
	assert(fib(9) == 34);
	assert(fib(10) == 55);
	assert(fib(11) == 89);
}
