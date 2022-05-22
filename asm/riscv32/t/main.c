#include <stdint.h>
#include <assert.h>
#include <stdio.h>

extern uint32_t sum(uint32_t);
extern uint32_t sum2(uint32_t);

int main(void)
{
	// test: sum
	for (unsigned int i = 0; i < 1000; i++) {
		assert(sum(i) == (i * (i + 1) / 2));
	}

	// test: sum2
	for (unsigned int i = 0; i < 1000; i++) {
		assert(sum2(i) == (i * (i + 1) * (2 * i + 1) / 6));
	}
}
