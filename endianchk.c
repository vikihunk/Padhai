/*
 * Check endianness of a machine
 *  - This is a pretty basic program which initializes an int and typecasts it
 * to char to verify endianness
 */
 
#include <stdio.h>
int main() 
{
	unsigned int i = 1;
	char *c = (char*)&i;
	printf("sizeof(int):%d\n", sizeof(int));
	if (*c)    
		printf("Little endian\n");
	else
		printf("Big endian\n");
	getchar();
	return 0;
}
