/*
 * Check endianness of a machine
 *  - This is a pretty basic program which initializes an int and typecasts it
 * to char to verify endianness
 */
 
#include <stdio.h>
int main() 
{
	unsigned int i = 0x0102;
	char *c = (char*)&i;
	printf("sizeof(int):%ld\n", sizeof(int));
	if (*c == 0x02) {    
		printf("Little endian\n");
	} else if (*c == 0x01) {
		printf("Big endian\n");
	} else {
		printf("Only God Knows!!\n");
	}
	return 0;
}
