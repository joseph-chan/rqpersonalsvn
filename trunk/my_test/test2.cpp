#include <stdarg.h>
#include <stdio.h>

	int
add_em_up (int count,va_list args)
{
	va_list ap;
	int i, sum;

	//va_start (ap, count);         /* Initialize the argument list. */

	sum = 0;
	for (i = 0; i < count; i++)
	{
		sum += va_arg (args, int);    /* Get the next argument value. */
		printf("%d\n",sum);
	}
	//va_end (ap);                  /* Clean up. */
	return sum;
}

int testf(int count, ...)
{
	int a;
	va_list args;

	va_start(args,count);
	a=add_em_up(count, args);
	va_end(args);
	return a;
}

	int
main (void)
{
	/* This call prints 16. */
	printf ("%d\n", testf (3, 1, 2, 3));

	/* This call prints 55. */
	printf ("%d\n", testf (10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));

	printf ("%d\n", testf (10, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10));
	return 0;
}
