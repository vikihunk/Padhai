#include <stdio.h>

void *ptr;

int sum (int a, int b) {
	return a+b;
}

//int (*sum_ptr)(int,int);
int main()
{
	ptr = &sum;

	int a = (((int (*)(int,int))(ptr))(2,3));
	printf("%d\n", a);
	int b = 10;
	ptr = &b;
	printf("%d\n", *(int *)ptr);
}
