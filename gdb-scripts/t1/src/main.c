#include <stdio.h>
#include <stdlib.h>

void stack(void);

int main(void)
{
	printf("hello world\n");
	for (int i = 0; i < 10; i++)
		stack();
	exit(0);
}
