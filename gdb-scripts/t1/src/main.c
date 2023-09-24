#include <stdio.h>
#include <stdlib.h>

unsigned int f(unsigned int);

int main(void)
{
	unsigned int m;

	for (unsigned int n = 0; n < 35; n += 5) {
		m = f(n);
		printf("%u! = %u\n", n, m);
	}

	exit(0);
}
