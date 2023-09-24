unsigned int f(unsigned int n)
{
	if (n <= 1)
		return 1;

	return n * f(n - 1);
}
