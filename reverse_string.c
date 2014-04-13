#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_LEN	256

void swap(char *s, char *c) {
	*s = (*s ^ *c);
	*c = (*s ^ *c);
	*s = (*s ^ *c);
}
	
int main() {
	char *str;
	int len = 0;
	int i = 0;

	str = (char *)malloc(STR_LEN);
	memset(str, 0, STR_LEN);

	printf("Type in a string: ");
	gets(str);

	len = strlen(str);
	for(i = 0; i < len/2; i++) {
		swap(&str[i], &str[len-i-1]);
	}
	printf("Reversed string: %s\n", str);
	free(str);
}
